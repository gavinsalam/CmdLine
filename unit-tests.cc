#include "CmdLine.hh"
#include <cassert>
#include <iostream>

using namespace std;

/// return a vector of strings, split by spaces
vector<string> split_spaces(const string& s) {
  vector<string> result;
  result.push_back("dummy");
  stringstream ss(s);
  string item;
  while (getline(ss, item, ' ')) {
    result.push_back(item);
  }
  return result;
}


/// first part of recursive print function for tuples (from stack overflow)
template<std::size_t I = 0, typename... Tp>
inline typename std::enable_if<I == sizeof...(Tp), void>::type
  print(const std::tuple<Tp...>& t)
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
void check_pass(const int line_number, const U & fn, const string & options, const V & expected_result){
  CmdLine cmdline(split_spaces(options));
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
  try {
    CmdLine cmdline(split_spaces(options));
    auto result = fn(cmdline);
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

int main() {
  // make sure that intentional failures are quiet
  CmdLine::Error::set_print_message(false);

  //---------------------------------------------------------------------------
  // verify value_bool with a true default
  {
    auto cmd = [](CmdLine & cmdline){
      return make_tuple(
          cmdline.value<int>("-i"), 
          cmdline.value_bool("-f", true)
      );
    };
    CHECK_PASS(cmd, "-i 2",        make_tuple(2,true) );
    CHECK_PASS(cmd, "-i 2 -f",     make_tuple(2,true) );
    CHECK_PASS(cmd, "-f -i 2",     make_tuple(2,true) );
    CHECK_PASS(cmd, "-no-f -i 2",  make_tuple(2,false));
    CHECK_PASS(cmd, "-f off -i 2", make_tuple(2,false));
    CHECK_PASS(cmd, "-f on -i 2",  make_tuple(2,true) );
    // should fail because 3 is not accepted as a boolean
    CHECK_FAIL(cmd, "-f 3 -i 2");
  }

  //---------------------------------------------------------------------------
  // verify value_bool with a false default
  {
    auto cmd = [](CmdLine & cmdline){
      return make_tuple(
          cmdline.value<int>("-i"), 
          cmdline.value_bool("-f", false)
      );
    };
    CHECK_PASS(cmd, "-i 2",    make_tuple(2,false));
    CHECK_PASS(cmd, "-i 2 -f", make_tuple(2,true));
    CHECK_PASS(cmd, "-f -i 2", make_tuple(2,true));
    CHECK_PASS(cmd, "-no-f -i 2", make_tuple(2,false));
    CHECK_PASS(cmd, "-f off -i 2", make_tuple(2,false));
    CHECK_PASS(cmd, "-f on -i 2", make_tuple(2,true));
  }

}