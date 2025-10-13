#include "CmdLine.hh"
#include <cassert>
#include <iostream>
#include <list>

using namespace std;

int n_checks = 0;

/// return a vector of strings, split by spaces
vector<string> split_spaces(const string& s) {
  vector<string> result;
  result.push_back("dummy");
  stringstream ss(s);
  string item;
  while (getline(ss, item, ' ')) {
    // skip empty items, effectively ignoring multiple spaces
    if (item == "") continue;
    result.push_back(item);
  }
  return result;
}


/// first part of recursive print function for tuples (from stack overflow)
template<std::size_t I = 0, typename... Tp>
inline typename std::enable_if<I == sizeof...(Tp), void>::type
  print(const std::tuple<Tp...>& )
  { }

/// second part of recursive print function for tuples
template<std::size_t I = 0, typename... Tp>
inline typename std::enable_if<I < sizeof...(Tp), void>::type
  print(const std::tuple<Tp...>& t)
  {
    std::cerr << std::get<I>(t) << " ";
    print<I + 1, Tp...>(t);
  }

/// check to see that the of the function fn, passed the command line associated with options, 
/// gives the expected result
template<typename U, typename V> 
void check_pass(const int line_number, 
                const U & fn, 
                const string & options, 
                const V & expected_result,
                bool enable_help = true){
  n_checks++;
  CmdLine cmdline(split_spaces(options), enable_help);
  V result = fn(cmdline);
  if (result != expected_result) {
    cerr << "From line " << to_string(line_number) << ", failure with options: " << options << endl;
    cerr << "  Expected: ";
    print(expected_result);
    cerr << endl << "  Got: ";
    print(result);
    cerr << endl;
    //cerr << "Error: expected " << expected_result << " but got " << result << endl;
    throw runtime_error("CmdLine failure");
  }
  cmdline.assert_all_options_used();
}

template<typename U> 
void check_fail(const int line_number, const U & fn, const string & options) {

  n_checks++;
  try {
    CmdLine cmdline(split_spaces(options));
    auto result = fn(cmdline);
    cmdline.assert_all_options_used();

    cerr << "From line " << to_string(line_number) << ", unexpected success with options: " << options << endl;
    cerr << "  Expected failure, but got: ";
    print(result);
    cerr << endl;
    throw runtime_error("CmdLine passed when failure expected");

  } catch (const CmdLine::Error & e) {
    // expected
    return;
  }
}

#define CHECK_PASS(fn, options, expected) (check_pass(__LINE__, fn, options, expected))
#define CHECK_FAIL(fn, options)           (check_fail(__LINE__, fn, options))
#define CHECK_PASS_NOHELP(fn, options, expected) (check_pass(__LINE__, fn, options, expected, false))

int main() {
  // make sure that intentional failures are quiet
  CmdLine::Error::set_print_message(false);

  //---------------------------------------------------------------------------
  // verify value_bool with a true default
  {
    auto cmd = [](CmdLine & cmdline){
      cmdline.help("test script");
      return make_tuple(
          cmdline.value<int>("-i"), 
          cmdline.value_bool({"-f","-future"}, true)
      );
    };
    CHECK_PASS(cmd, "-i 2",            make_tuple(2,true) );
    CHECK_PASS(cmd, "-i 2 -f",         make_tuple(2,true) );
    CHECK_PASS(cmd, "-f -i 2",         make_tuple(2,true) );
    CHECK_PASS(cmd, "-no-f -i 2",      make_tuple(2,false));
    CHECK_PASS(cmd, "-f off -i 2",     make_tuple(2,false));
    CHECK_PASS(cmd, "-f on -i 2",      make_tuple(2,true) );
    CHECK_PASS(cmd, "-future on -i 2", make_tuple(2,true) );
    CHECK_PASS(cmd, "-no-future -i 2", make_tuple(2,false));
    // should fail because -i is either absent or its associated value is absent
    CHECK_FAIL(cmd, "-f 3");
    CHECK_FAIL(cmd, "-f 3 -i");

    // should fail because 3 is not accepted as a boolean
    CHECK_FAIL(cmd, "-f 3 -i 2");
    CHECK_FAIL(cmd, "-f -2 -i 2");
    // should fail because we give the option twice
    CHECK_FAIL(cmd, "-i 2 -f false -no-f");    
    CHECK_FAIL(cmd, "-i 2 -f -no-f");    
    CHECK_FAIL(cmd, "-i 2 -f -future");    
    CHECK_FAIL(cmd, "-i 2 -no-f -no-future");    
    CHECK_FAIL(cmd, "-i 2 -f -no-future");    
  }

  {
    auto cmd = [](CmdLine & cmdline){
      return make_tuple(
          cmdline.value<int>("-i"), 
          cmdline.value_bool({"-f","-future"}, true)
      );
    };
    CHECK_PASS_NOHELP(cmd, "-i 2",            make_tuple(2,true) );
    CHECK_PASS_NOHELP(cmd, "-i 2 -f",         make_tuple(2,true) );
    CHECK_PASS_NOHELP(cmd, "-f -i 2",         make_tuple(2,true) );
    CHECK_PASS_NOHELP(cmd, "-no-f -i 2",      make_tuple(2,false));
    CHECK_PASS_NOHELP(cmd, "-f off -i 2",     make_tuple(2,false));
    CHECK_PASS_NOHELP(cmd, "-f on -i 2",      make_tuple(2,true) );
    CHECK_PASS_NOHELP(cmd, "-future on -i 2", make_tuple(2,true) );
    CHECK_PASS_NOHELP(cmd, "-no-future -i 2", make_tuple(2,false));
  }


  //---------------------------------------------------------------------------
  // verify value_bool with a false default
  {
    auto cmd = [](CmdLine & cmdline){
      return make_tuple(
          cmdline.value<int>("-i"), 
          cmdline.value_bool({"-f","-future"}, false)
      );
    };
    for (const string opt : {"-f","-future"}) {
      CHECK_PASS(cmd, "-i 2",            make_tuple(2,false));
      CHECK_PASS(cmd, "-i 2 " + opt,     make_tuple(2,true) );
      CHECK_PASS(cmd, "-i 2 -no" + opt,  make_tuple(2,false));

      CHECK_PASS(cmd, "-i 2 " + opt + " on",     make_tuple(2,true) );
      CHECK_PASS(cmd, "-i 2 " + opt + " yes",    make_tuple(2,true) );
      CHECK_PASS(cmd, "-i 2 " + opt + " true",   make_tuple(2,true) );
      CHECK_PASS(cmd, "-i 2 " + opt + " 1",      make_tuple(2,true) );
      CHECK_PASS(cmd, "-i 2 " + opt + " .true.", make_tuple(2,true) );

      CHECK_PASS(cmd, "-i 2 " + opt + " off",     make_tuple(2,false));
      CHECK_PASS(cmd, "-i 2 " + opt + " no",      make_tuple(2,false));
      CHECK_PASS(cmd, "-i 2 " + opt + " false",   make_tuple(2,false));
      CHECK_PASS(cmd, "-i 2 " + opt + " .false.", make_tuple(2,false));
      CHECK_PASS(cmd, "-i 2 " + opt + " 0",       make_tuple(2,false));
    }

    CHECK_PASS(cmd, "-i 2",    make_tuple(2,false));
    CHECK_PASS(cmd, "-i 2 -f", make_tuple(2,true));
    CHECK_PASS(cmd, "-i 2 -f 1", make_tuple(2,true));

    CHECK_PASS(cmd, "-f       -i 2", make_tuple(2,true));
    CHECK_PASS(cmd, "-f on    -i 2", make_tuple(2,true));
    CHECK_PASS(cmd, "-f yes   -i 2", make_tuple(2,true));
    CHECK_PASS(cmd, "-f true  -i 2", make_tuple(2,true));
    CHECK_PASS(cmd, "-f 1     -i 2", make_tuple(2,true));
    
    CHECK_PASS(cmd, "-no-f    -i 2", make_tuple(2,false));
    CHECK_PASS(cmd, "-f off   -i 2", make_tuple(2,false));
    CHECK_PASS(cmd, "-f no    -i 2", make_tuple(2,false));
    CHECK_PASS(cmd, "-f false -i 2", make_tuple(2,false));
    CHECK_PASS(cmd, "-f 0     -i 2", make_tuple(2,false));

    CHECK_PASS(cmd, "-f .true.  -i 2", make_tuple(2,true));
    CHECK_PASS(cmd, "-f .false. -i 2", make_tuple(2,false));
  }

  // check the default constructor compiles. We will assign
  // to it below to check that that works too.
  CmdLine::Result<double> double_result;

  //---------------------------------------------------------------------------
  // verify cases that take initialiser lists and vectors of options
  auto cmd = [&double_result](CmdLine & cmdline){
    vector<string> opts_d = {"-d","--double"};
    auto uu = cmdline.optional_value<int>({"-u","--uu"});
    double u = uu.present() ? uu.value() : 3;

    double_result = cmdline.value<double>(opts_d, 1.4);
    return make_tuple(      
      double_result,
      cmdline.value<int>({"-i","--int"}),
      u
    );
  };
  CHECK_PASS(cmd, "-d 2.3 -i 2",          make_tuple(2.3, 2, 3));
  CHECK_PASS(cmd, "--double 2.3 --int 2", make_tuple(2.3, 2, 3));
  CHECK_PASS(cmd, "--int 2",              make_tuple(1.4, 2, 3));
  CHECK_PASS(cmd, "--int 2 --uu 6",       make_tuple(1.4, 2, 6));
  CHECK_FAIL(cmd, "");



  cout << "All " << n_checks << " checks passed" << endl;
  return 0;

}