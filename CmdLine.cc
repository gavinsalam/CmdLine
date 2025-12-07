///////////////////////////////////////////////////////////////////////////////
// File: CmdLine.cc                                                          //
// Part of the CmdLine library                                               //
//                                                                           //
// Copyright (c) 2007-2023 Gavin Salam with contributions from               //
// Gregory Soyez and Rob Verheyen                                            //
//                                                                           //
// This program is free software; you can redistribute it and/or modify      //
// it under the terms of the GNU General Public License as published by      //
// the Free Software Foundation; either version 2 of the License, or         //
// (at your option) any later version.                                       //
//                                                                           //
// This program is distributed in the hope that it will be useful,           //
// but WITHOUT ANY WARRANTY; without even the implied warranty of            //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             //
// GNU General Public License for more details.                              //
//                                                                           //
// You should have received a copy of the GNU General Public License         //
// along with this program; if not, write to the Free Software               //
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////




#include "CmdLine.hh"
#include<string>
#include<sstream>
#include<iostream> // testing
#include<fstream>
#include<vector>
#include<cstddef> // for size_t
#include<cstdint> // for uint64_t
#include <sys/utsname.h> // for getting uname
#include <unistd.h> // for getting current path
#include <stdlib.h> // for getting the environment (including username)
#include <cstdio>
#include <algorithm>
#include <cctype>
#ifdef __CMDLINE_ABI_DEMANGLE__
#include <cxxabi.h>
#endif 

using namespace std;

string CmdLine::_default_argfile_option = "-argfile";

std::ostream & operator<<(std::ostream & ostr, CmdLine::OptKind optkind) {
  if      (optkind == CmdLine::OptKind::present) ostr << "present";
  else if (optkind == CmdLine::OptKind::required_value) ostr << "required_value";
  else if (optkind == CmdLine::OptKind::optional_value) ostr << "optional_value";
  else if (optkind == CmdLine::OptKind::value_with_default) ostr << "value_with_default";
  else if (optkind == CmdLine::OptKind::undefined) ostr << "undefined";
  else ostr << "UNRECOGNISED";
  return ostr;
}


// initialise the various structures that we shall
// use to access the command-line options;
//
// If an option appears several times, it is its LAST value
// that will be used in searching for option values (opposite of f90)
CmdLine::CmdLine (const int argc, char** argv, bool enable_help, const string & file_option) : 
    __help_enabled(enable_help), __argfile_option(file_option) {

  __arguments.resize(argc);
  for(int iarg = 0; iarg < argc; iarg++){
    __arguments[iarg] = argv[iarg];
  }
  this->init();
}

/// constructor from a vector of strings, one argument per string
CmdLine::CmdLine (const vector<string> & args, bool enable_help, const string & file_option) : 
    __help_enabled(enable_help), __argfile_option(file_option) {

  // some sanity checks
  if (args[0].size() == 0) throw Error("CmdLine constructor: args[0] is empy, but should contain a command name");
  if (args[0][0] == '-') throw Error("CmdLine constructor: args[0] = '" + args[0] + "' starts with a -, but should contain a command name");

  __arguments = args;
  this->init();
}

/// Add an overall help string
CmdLine & CmdLine::help(const std::string & help_str) {
  __overall_help_string = help_str;
  __help_enabled = true;
  return *this;
}

//----------------------------------------------------------------------
void CmdLine::init (){
  // record time at start
  time(&__time_at_start);

  // this does not work...
  //__options_help[__argfile_option] = 
  //    OptionHelp_value_with_default<string>(__argfile_option, "filename", 
  //                          "if present, further arguments are read from the filename");

  // check first if a file option is passed
  for(size_t iarg = 0; iarg < __arguments.size(); iarg++) {
    const string & arg = __arguments[iarg];
    if (arg == __argfile_option) {
      // make sure a file is passed too
      bool found_file = true;
      ifstream file_in;
      if (iarg+1 == __arguments.size()) found_file = false;
      else {
        file_in.open(__arguments[iarg+1].c_str());
        found_file = file_in.good();
      }

      // error if no file found
      if (!found_file) {
        ostringstream ostr;
        ostr << "Option "<< __argfile_option
             <<" is passed but no file was found"<<endl;
        throw Error(ostr);
      }

      // remove the file options from the list of arguments
      __arguments.erase(__arguments.begin()+iarg, __arguments.begin()+iarg+2);

      string read_string = "";
      while (file_in >> read_string) {
        // skip the rest of the line if it's a comment;
        // allow both C++-style and shell-style comments
        if (read_string.find("//") != string::npos || read_string.find("#") != string::npos) {
          // read in the rest of this line, effectively discarding it
          getline(file_in, read_string);
        }
        else {
          __arguments.push_back(read_string);
        }
      }

      // start from the beginning of the argument list again again
      iarg = 0;
    }
  }

  // record whole command line so that it can be easily reused
  __command_line = "";
  for(size_t iarg = 0; iarg < __arguments.size(); iarg++){
    const string & arg = __arguments[iarg];
    // if an argument contains special characters, enclose it in
    // single quotes [NB: does not work if it contains a single quote
    // itself: treated below]
    if (arg.find(' ') != string::npos ||
        arg.find('|') != string::npos ||
        arg.find('<') != string::npos || 
        arg.find('>') != string::npos || 
        arg.find('"') != string::npos || 
        arg.find('#') != string::npos) {
      __command_line += "'"+arg+"'";
    } else if (arg.find("'") != string::npos) {
      // handle the case with single quotes in the argument
      // (NB: if there are single and double quotes, we are in trouble...)
      __command_line += '"'+arg+'"';
    } else {
      __command_line += arg;
    }
    __command_line += " ";
  }
  
  // group things into options
  bool next_may_be_val = false;
  string currentopt;
  __arguments_used.resize(__arguments.size(), false);
  __arguments_used[0] = true;
  for(size_t iarg = 1; iarg < __arguments.size(); iarg++){
    // if expecting an option value, then take it (even if
    // it is actually next option...)
    if (next_may_be_val) {__options[currentopt].second = iarg;}
    // now see if it might be an option itself
    string arg = __arguments[iarg];
    bool thisisopt = (arg.compare(0,1,"-") == 0);
    if (thisisopt) {
      // set option to a standard undefined value and say that 
      // we expect (possibly) a value on next round
      currentopt = arg;
      __options[currentopt] = make_pair(int(iarg),-1);
      __options_used[currentopt] = false;
      next_may_be_val = true;}
    else {
      // otherwise throw away the argument for now...
      next_may_be_val = false;
      currentopt = "";
    }
  }
  if (__help_enabled) {
    start_section("Options for getting help");
    __help_requested = any_present({"-h","-help","--help"}).help("prints this help message").no_dump();
    __markdown_help = any_present({"--markdown-help","-markdown-help"}).help("prints this help message in markdown format").no_dump();
    __help_requested |= __markdown_help;
    end_section();
  }

  // by default, enabe the git info
  set_git_info_enabled(true);
}

// indicates whether an option is present
CmdLine::Result<bool> CmdLine::any_present(const vector<string> & opts) const {
  OptionHelp * opthelp = opthelp_ptr(OptionHelp_present(opts));
  pair<int,int> result_pair = internal_present(opts);
  bool result = (result_pair.first > 0);
  Result<bool> res(result, opthelp, result);
  if (opthelp) opthelp->result_ptr = std::make_shared<Result<bool>>(res);
  return res;
}

//CmdLine::Result<bool> CmdLine::value_bool(const std::string & opt, const bool defval) const {
CmdLine::Result<bool> CmdLine::any_value_bool(const std::vector<std::string> & opts, const bool defval) const {
    OptionHelp * opthelp = opthelp_ptr(OptionHelp_value_with_default<bool>(opts, defval));
  pair<int,int> result_opt    = internal_present(opts);
  std::vector<std::string> no_opts;
  for (const auto & opt: opts) {no_opts.push_back("-no" + opt);}
  pair<int,int> result_no_opt = internal_present(no_opts);
  bool result;
  bool is_present = true;
  if (result_opt.first > 0) {
    if (result_no_opt.first > 0) {
      throw Error("boolean option " + __arguments[result_opt.first] 
            + " and negation " + __arguments[result_no_opt.first]  + " are both present");
    } else if (result_opt.second > 0) {
      const string & arg = __arguments[result_opt.second];
      // if next value starts with a - then it's an option, not a value
      if (arg[0] == '-') {
        result = true;
      } else  {
        result = internal_value<bool>(__arguments[result_opt.first]);
      }
    } else {
      result = true;
    }
  } else if (result_no_opt.first > 0) {
    result = false;
  } else {
    result = defval;
    is_present = false;
  }
  auto res = std::make_shared<Result<bool>>(result, opthelp, is_present);
  if (opthelp) opthelp->result_ptr = res;
  return *res;
}

// indicates whether an option is present (for internal use only -- does not set help)
pair<int,int> CmdLine::internal_present(const string & opt) const {
  bool result = (__options.find(opt) != __options.end());
  if (result) {
    __options_used[opt] = true;
    __arguments_used[__options[opt].first] = true;
    return __options[opt];
  } else {
    return make_pair(-1,-1);
  }
}

// indicates whether an option is present (for internal use only -- does not set help)
pair<int,int> CmdLine::internal_present(const vector<string> & opts) const {
  vector<string> opts_present;
  for (const auto & opt: opts) {
    bool opt_present = (__options.find(opt) != __options.end());
    if (opt_present) opts_present.push_back(opt);
  }

  if      (opts_present.size() == 0) return make_pair(-1,-1);
  else if (opts_present.size() == 1) {
    __options_used[opts_present[0]] = true;
    __arguments_used[__options[opts_present[0]].first] = true;
    return __options[opts_present[0]];
  } else {
    // options are supposed to be mutually exclusive, so eliminate
    // them all
    ostringstream ostr;
    ostr << "Options " << opts_present[0];
    for (size_t i = 1; i < opts_present.size()-1; i++) {
      ostr << ", " << opts_present[i];
    }
    ostr << " and " << opts_present[opts_present.size()-1] << " are mutually exclusive";
    throw Error(ostr);
  }
}


// indicates whether an option is present and has a value associated
bool CmdLine::internal_present_and_set(const string & opt) const {
  pair<int,int> is_present = internal_present(opt);
  return (is_present.second > 0);

}


// return the string value corresponding to the specified option
string CmdLine::internal_string_val(const vector<string> & opts) const {
  pair<int,int> is_present = internal_present(opts);
  if (is_present.second < 0) {
    if (opts.size() == 1) {
      throw Error("Option " +opts[0]+ " requested but not present and set");
    } else {
      ostringstream ostr;
      ostr << "One of the options " << opts[0];
      for (size_t i = 1; i < opts.size()-1; i++) {
        ostr << ", " << opts[i];
      }
      ostr << " or " << opts[opts.size()-1] << " requested but none present and set";
      throw Error(ostr);
    }
  }
  string arg = __arguments[is_present.second];
  __arguments_used[is_present.second] = true;
  // this may itself look like an option -- if that is the case
  // declare the option to have been used
  if (arg.compare(0,1,"-") == 0) {__options_used[arg] = true;}
  return arg;
}

void CmdLine::end_section(const std::string & section_name) {
  if (__current_section != section_name) {
    std::ostringstream ostr;
    ostr << "Tried to end section '" << section_name 
          << "' but current section is '" << __current_section << "'";
    throw Error(ostr.str());
  }
  __current_section = "";
  __current_subsection = "";
}

  void CmdLine::start_subsection(const std::string & subsection_name, const std::string & description) {
    if (__current_section == "") throw Error("cannot start subsection '" + subsection_name + "' without being in a section");
    __current_subsection = subsection_name;
    if (description != "") {
      __section_descriptions[__section_key(__current_section, subsection_name)] = description;
    }
  }

void CmdLine::end_subsection(const std::string & subsection_name) {
  if (__current_subsection != subsection_name) {
    std::ostringstream ostr;
    ostr << "Tried to end subsection '" << subsection_name 
          << "' but current subsection is '" << __current_subsection << "'";
    throw Error(ostr.str());
  }
  __current_subsection = "";
}

// return true if all options have been asked for at some point or other
// and send diagnostic info to ostr
bool CmdLine::all_options_used(ostream & ostr) const {
  bool result = true;
  for (size_t iarg = 1; iarg < __arguments_used.size(); iarg++) {
    string arg = __arguments[iarg];
    bool this_one = __arguments_used[iarg];
    if (! this_one) {
      ostr << "\nArgument " << arg << " at position " << iarg << " unused/unrecognized";
      if (__options_used.count(arg) > 0 && __options_used[arg]) {
        ostr << "  (this could be because the same option already appeared";
        if (__options.count(arg) && __options[arg].first > 0) {
          ostr << " at position " << __options[arg].first << ")";
        } else {
          ostr << " elsewhere on the command line)";
        }
      }
      ostr << endl;
    }
    result &= this_one;
  }
  return result;
}

/// return a time stamp corresponding to now
string CmdLine::time_stamp(bool utc) const {
  time_t timenow;
  time(&timenow);
  return _string_time(timenow, utc);
}

/// return a time stamp corresponding to start time
string CmdLine::time_stamp_at_start(bool utc) const {
  return _string_time(__time_at_start, utc);
}

/// return the elapsed time in seconds since the CmdLine object was
/// created
double CmdLine::time_elapsed_since_start() const {
  time_t timenow;
  time(&timenow);
  return std::difftime(timenow, __time_at_start);
}


/// convert the time into a string (local by default -- utc if 
/// utc=true).
string CmdLine::_string_time(const time_t & time, bool utc) const {
  struct tm * timeinfo;
  if (utc) {
    timeinfo = gmtime(&time);
  } else {
    timeinfo = localtime(&time);
  }
  char timecstr[100];
  strftime (timecstr,100,"%Y-%m-%d %H:%M:%S (%Z)",timeinfo);
  //sprintf(timecstr,"%04d-%02d-%02d %02d:%02d:%02d",
  //        timeinfo->tm_year+1900,
  //        timeinfo->tm_mon+1,
  //        timeinfo->tm_mday,
  //        timeinfo->tm_hour,
  //        timeinfo->tm_min,
  //        timeinfo->tm_sec);
  //string timestr = timecstr;
  //if (utc) {
  //  timestr .= " (UTC)";
  //} else {
  //  timestr .= " (local)";
  //}
  return timecstr;
}

/// return a unix-style uname
string CmdLine::unix_uname() const {
  utsname utsbuf;
  int utsret = uname(&utsbuf);
  if (utsret != 0) {return "Error establishing uname";}
  ostringstream uname_result;
  uname_result << utsbuf.sysname << " " 
               << utsbuf.nodename << " "
               << utsbuf.release << " "
               << utsbuf.version << " "
               << utsbuf.machine;
  return uname_result.str();
}

string CmdLine::unix_username() const {
  char * logname;
  logname = getenv("LOGNAME");
  if (logname != nullptr) {return logname;}
  else {return "unknown-username";}
}

/// report failure of conversion
void CmdLine::_report_conversion_failure(const string & opt, 
                                         const string & optstring) const {
  ostringstream ostr;
  ostr << "could not convert option ("<<opt<<") value ("
       <<optstring<<") to requested type"<<endl; 
  throw Error(ostr);
}

void CmdLine::assert_all_options_used() const {
  // deal with the help part
  if (__help_enabled && __help_requested) {
    print_help(cout, __markdown_help);
    exit(0);
  }
  ostringstream ostr;
  if (! all_options_used(ostr)) {
    ostr <<"Unrecognised options on the command line" << endl;
    throw Error(ostr);
  }
}

string CmdLine::string_val(const string & opt) const {return value<std::string>(opt);}

// as above, but if opt is not present_and_set, return default
string CmdLine::string_val(const string & opt, const string & defval) const {
  return value<string>(opt,defval);
}

// Return the integer value corresponding to the specified option;
// Not too sure what happens if option is present_and_set but does not
// have string value...
int CmdLine::int_val(const string & opt) const { return value<int>(opt);}

// as above, but if opt is not present_and_set, return default
int CmdLine::int_val(const string & opt, const int & defval) const {
  return value<int>(opt,defval);
}


// Return the integer value corresponding to the specified option;
// Not too sure what happens if option is present_and_set but does not
// have string value...
double CmdLine::double_val(const string & opt) const {return value<double>(opt);}

// as above, but if opt is not present_and_set, return default
double CmdLine::double_val(const string & opt, const double & defval) const {
  return value<double>(opt,defval);
}

// return the full command line including the command itself
string CmdLine::command_line() const {
  return __command_line;
}



bool CmdLine::Error::_do_printout = true;
CmdLine::Error::Error(const std::ostringstream & ostr) : CmdLine::Error(ostr.str()) {}

  CmdLine::Error::Error(const std::string & str) 
  : std::runtime_error(str) {
  _message = what();
  if (_do_printout) cerr << tc::red << tc::bold 
                         << "CmdLine Error: " << tc::nobold << _message << tc::reset << endl;
}

string CmdLine::current_path() const {
  const size_t maxlen = 10000;
  char tmp[maxlen];
  char * result = getcwd(tmp,maxlen);
  if (result == nullptr) {
    return "error-getting-path";
  } else {
    return string(tmp);
  }
}

string CmdLine::header(const string & prefix) const {
  ostringstream ostr;
  ostr << prefix << "" << command_line() << endl;
  ostr << prefix << "from path: " << current_path() << endl;
  ostr << prefix << "started at: " << time_stamp_at_start() << endl;
  ostr << prefix << "by user: "    << unix_username() << endl;
  ostr << prefix << "running on: " << unix_uname() << endl;
  ostr << prefix << "git state (if any): " << git_info() << endl;
  return ostr.str();
}

/// return a pointer to an existing opthelp is the option is present
/// otherwise register the given opthelp and return a pointer to that
/// (if help is disabled, return a null poiner)
CmdLine::OptionHelp * CmdLine::opthelp_ptr(const CmdLine::OptionHelp & opthelp) const {
  if (!__help_enabled) return nullptr;

  OptionHelp * result;

  auto opthelp_iter = __options_help.find(opthelp.option);
  if (opthelp_iter == __options_help.end()) {
    __options_queried.push_back(opthelp.option);
    __options_help[opthelp.option] = opthelp;
    result = &__options_help[opthelp.option];
  } else {
    result = &opthelp_iter->second;
    // now create a lambda to help with checks that
    // - the option is not being redefined with a different kind
    // - the option is not being redefined with a different default value
    auto warn_or_fail = [&](const string & message) {
      if (fussy()) throw Error(message);
      else         cout << "********* CmdLine warning: " << message << endl;
    };
    if (result->kind != opthelp.kind) {
      ostringstream ostr;
      ostr << "Option " << opthelp.option << " has already been requested with kind '" 
           << result->kind << "' but is now being requested with kind '" << opthelp.kind << "'";
      warn_or_fail(ostr.str());
    }
    if (result->kind == OptKind::value_with_default && result->default_value != opthelp.default_value) {
      ostringstream ostr;
      ostr << "Option " << opthelp.option << " has already been requested with default value " 
           << result->default_value << " but is now being requested with default_value " << opthelp.default_value;
      warn_or_fail(ostr.str());      
    }
  }
  return result;

}

string CmdLine::OptionHelp::type_name() const {
  if      (type == typeid(int)   .name())   return "int"   ;
  else if (type == typeid(unsigned int).name()) return "unsigned int"   ;
  else if (type == typeid(uint64_t).name()) return "uint64_t"   ;
  else if (type == typeid(int64_t).name())  return "int64_t"   ;
  else if (type == typeid(double).name())   return "double";
  else if (type == typeid(string).name())   return "string";
  else if (type == typeid(bool  ).name())   return "bool";
  else return demangle(type);
}

string CmdLine::OptionHelp::demangle(const std::string & type_name) {
#ifdef __CMDLINE_ABI_DEMANGLE__
  int     status;
  char   *realname;

  realname = abi::__cxa_demangle(type_name.c_str(), NULL, NULL, &status);
  string realname_str;
  if (status == 0) {
    realname_str = realname;
  } else {
    realname_str = type_name;
  }
  std::free(realname);
  return realname_str;
#else // __CMDLINE_ABI_DEMANGLE__
  return type_name;
#endif // __CMDLINE_ABI_DEMANGLE__
}


string CmdLine::OptionHelp::summary() const {
  ostringstream ostr;
  if (! required) ostr << "[";
  ostr << option;
  if (takes_value) ostr << " " << argname;
  if (! required) ostr << "]";
  return ostr.str();
}


string CmdLine::OptionHelp::description(const string & prefix, int wrap_column, bool markdown) const {
  ostringstream ostr;
  auto code = [&](const std::string & str) {
    if (markdown) return "`" + str + "`";
    else          return str;
  };
  auto bold_code = [&](const std::string & str) {
    if (markdown) return "**`" + str + "`**";
    else          return str;
  };
  auto italic_code = [&](const std::string & str) {
    if (markdown) return "*`" + str + "`*";
    else          return str;
  };

  ostr << prefix << bold_code(option);

  bool itemised_choices = false;

  if (takes_value) {
    ostr << " " << italic_code(argname) << " (" << type_name() << ")";
    if (has_default) ostr << ", default: " << code(default_value);
    if (choices.size() != 0) {
      string choice_list_str = choice_list(code);
      // some arbitrary limit on the length of the list of choices
      itemised_choices = choice_list_str.size() > 40 || choices_help.size() != 0;
      if (!itemised_choices) ostr << ", valid choices: {" << choice_list_str << "}";
    }
    if (range_strings.size() != 0) {
      ostr << ", allowed range: " << range_string() << "";
    }
  }
  ostr << "  \n";
  if (aliases.size() > 1) {
    ostr << prefix << "  aliases: ";
    for (unsigned i = 1; i < aliases.size(); i++) {
      ostr << code(aliases[i]);
      if (i+1 != aliases.size()) ostr << ", ";
    }
    ostr << "  \n";
  }
  if (help.size() > 0) {
    ostr << wrap(help, wrap_column, prefix + "  ");
  } 
  ostr << endl;

  // finish off with any itemised choices
  if (itemised_choices) {
    ostr << prefix << endl 
         << prefix << "  Valid choices: " << endl;
    bool has_choices_help = (choices_help.size() == choices.size());
    for (unsigned i = 0; i < choices.size(); i++)   {
      if (has_choices_help) {
        ostr << CmdLine::wrap(prefix + "  * " + code(choices[i]) + ": " + choices_help[i], wrap_column, 
                              prefix+"    ", false) << endl;
      } else {
        ostr << prefix+"  * " << code(choices[i]) << endl;
      }
      //if (i+1 != choices.size()) ostr << ", ";
    }
  }
  return ostr.str();
}

std::string CmdLine::wrap(const std::string & str, int wrap_column, 
                          const std::string & prefix, bool first_line_prefix) {
  // start by separating the string into tokens: words or spaces or new line characters
  vector<string> tokens;
  size_t last_i = 0;
  size_t i = 0;
  size_t n = str.size();
  while (i < n) {
    if (str[i] == ' ' || str[i] == '\n') {
      tokens.push_back(str.substr(last_i,i-last_i));
      tokens.push_back(str.substr(i,1));
      last_i = i+1;
    }
    i++;
  }
  if (last_i < n) tokens.push_back(str.substr(last_i,n-last_i));

  // then loop over the tokens, printing them out, and wrapping if need be
  ostringstream ostr;
  size_t line_len = 0;
  if (first_line_prefix) {
    ostr << prefix;
  }
  for (const auto & token: tokens) {
    if (token == "\n") {
      ostr << endl << prefix;
      line_len = prefix.size();
    } else {
      if (int(line_len + token.size()) < wrap_column) {
        ostr << token;
        line_len += token.size();
      } else if (token == " ") {
        ostr << endl << prefix;
        line_len = prefix.size();
      } else {
        ostr << endl << prefix << token;
        line_len = prefix.size() + token.size();
      }
    }
  }
  return ostr.str();
}

string CmdLine::OptionHelp::choice_list(const std::function<std::string(const std::string & str)> & code_formatter) const {
  ostringstream ostr;
  for (unsigned i = 0; i < choices.size(); i++)   {
    ostr << code_formatter(choices[i]);
    if (i+1 != choices.size()) ostr << ", ";
  }
  return ostr.str();
}

string CmdLine::OptionHelp::range_string() const {
  ostringstream ostr;
  if (range_strings.size() != 2) return "";
  ostr << range_strings[0] << " <= " << argname << " <= " << range_strings[1];
  return ostr.str();
}

/// returns a vector of OptSection objects, each of which contains
/// a vector of options, as well as an indication of the name of the section
/// and the level of indentation
std::vector<CmdLine::OptSection> CmdLine::organised_options() const {
  if (!__help_enabled) throw Error("CmdLine::organised_options() called, but help disabled");

  vector<OptSection> opt_sections;
  opt_sections.push_back(OptSection("", 0));

  map<string,vector<const OptionHelp *> > opthelp_section_contents;
  vector<string> opthelp_sections;

  // First register each option that is not in any section
  for (const auto & opt: __options_queried) {
    const OptionHelp & opthelp = __options_help[opt];
    if (opthelp.section == "") {
      opt_sections.back().options.push_back(&opthelp);
    } else {
      // if an option is in a section, register it for later
      if (opthelp_section_contents.count(opthelp.section) == 0) {
        opthelp_sections.push_back(opthelp.section);
      }
      opthelp_section_contents[opthelp.section].push_back(&opthelp);
    }
  }

  // then loop over the sections, registering each one
  for (const auto & section: opthelp_sections) {
    opt_sections.push_back(OptSection(section, 1));
    opt_sections.back().section_key = __section_key(section, "");

    map<string,vector<const OptionHelp *> > opthelp_subsection_contents;
    vector<string> opthelp_subsections;
    for (const auto & opthelp: opthelp_section_contents[section]) {
      // print out those that are not in a subsection, registering the
      // subsection options for later
      if (opthelp->subsection == "") {
        opt_sections.back().options.push_back(opthelp);
      } else {
        // if an option is in a subsection, register it for later
        if (opthelp_subsection_contents.count(opthelp->subsection) == 0) {
          opthelp_subsections.push_back(opthelp->subsection);
        }
        opthelp_subsection_contents[opthelp->subsection].push_back(opthelp);
      }
    }

    // and then print out things that are in subsections
    for (const auto & subsection: opthelp_subsections) {
      opt_sections.push_back(OptSection(subsection, 2));
      opt_sections.back().section_key = __section_key(section, subsection);
      for (const auto & opthelp: opthelp_subsection_contents[subsection]) {
        opt_sections.back().options.push_back(opthelp);
      }
    }
  }

  return opt_sections;

}

void CmdLine::print_help(ostream & ostr, bool markdown) const {
  if (!__help_enabled) throw Error("CmdLine::print_help() called, but help disabled");

  if (markdown) {
    print_markdown(ostr);
    return;
  }
  // First print a summary
  ostr << "\nUsage: \n       " << __arguments[0];
  for (const auto & opt: __options_queried) {
    ostr << " " << __options_help[opt].summary();
  }
  ostr << endl << endl;

  if (__overall_help_string.size() != 0) {
    ostr << wrap(__overall_help_string);
    ostr << endl << endl;
  }

  ostr << "Detailed option help" << endl;
  ostr << "====================" << endl << endl;

  vector<OptSection> sections = organised_options();
  string prefix = "";
  for (const auto & section: sections) {
    // skip empty sections
    if (section.options.size() == 0) continue;

    // print a section header if appropriate
    if (section.level > 0) {      
      ostr << endl;
      ostr << section.name << endl;
      ostr << string(section.name.size(), section.level == 1 ? '-' : '.') << endl;
      auto description = __section_descriptions.find(section.section_key);
      if (description != __section_descriptions.end()) {
        ostr << wrap(description->second, 80, "", false) << endl;
      }
      ostr << endl;
    }

    // then print the options in that section (or subsection)
    for (const auto & opthelp: section.options) {
      ostr << opthelp->description(prefix) << endl;
    }
  }

}


//------------------------------------------------------------------------
void CmdLine::print_markdown(ostream & ostr) const {
  bool markdown = true;
  int wrap_column = 80;
  auto code = [&](const std::string & str) {
    if (markdown) return "`" + str + "`";
    else          return str;
  };

  // First print a summary
//  ostr << "\nUsage: \n       " << __arguments[0];
//  for (const auto & opt: __options_queried) {
//    ostr << " " << __options_help[opt].summary();
//  }
//  ostr << endl << endl;

  ostr << "# " << code(__arguments[0]) << ": Option help" << endl << endl;;

  ostr << "[//]: # (Generated by: " << command_line () << ")" << endl << endl;

  if (__overall_help_string.size() != 0) {
    ostr << wrap(__overall_help_string);
    ostr << endl << endl;
  }

  ostringstream body;
  ostringstream toc;

  toc  << "## Table of contents" << endl << endl;
  body << "# Detailed option help" << endl << endl;

  vector<OptSection> sections = organised_options();
  string prefix = "";
  for (int isec = 0; isec < int(sections.size()); isec++) {
    const auto & section = sections[isec];

    // skip empty sections
    if (section.options.size() == 0) continue;

    // print a section header if appropriate
    string section_name = section.level > 0 ? section.name : string("General options");
    int section_level = max(1,section.level);
    //cerr << section_name << " " << " " << section.level << " " << section_level << endl;

    // indent the section name according to its level
    toc << string(section_level * 2, ' ');
    toc << "- [" << section_name << "](#sec" << isec << ")" << endl;

    body << endl 
          << "<a id=\"sec" << isec << "\"></a>" 
          << endl;
    body << string(section_level+1, '#') << " ";
    body << section_name << endl;
    auto description = __section_descriptions.find(section.section_key);
    if (description != __section_descriptions.end()) {
      body << wrap(description->second, wrap_column, "", false) << endl;
    }
    body << endl;

    // then print the options in that section (or subsection)
    for (const auto & opthelp: section.options) {
      body << opthelp->description(prefix, wrap_column, markdown) << endl;
    }
  }

  ostr << toc.str() << endl << endl;
  ostr << body.str() << endl << endl;

}



//------------------------------------------------------------------------
  /// return a std::string in argfile format that contains all
  /// options queried and, where relevant, their values
  /// - if an option is optional (no default), it is printed commented-out
  /// - if an option was not supplied but has a default, it is printed out with its default
  ///
  /// @param prefix is the string the precedes each description line (default is "# ")
  /// @param absence_prefix is the string that precedes each line for an option that was not present
  /// @param presence_prefix is the string that precedes each line for an option that was present
string CmdLine::dump(const string & prefix, const string & absence_prefix, const string & presence_prefix, bool compact) const {
  ostringstream ostr;

  ostr << prefix << "argfile for " << command_line() << endl;
  if (!compact) ostr << wrap(__overall_help_string, 80, prefix) << endl;
  if (!compact) ostr << prefix << "generated by CmdLine::dump() on " << time_stamp() << endl;

  auto print_option = [&](const OptionHelp & opthelp) {
    const ResultBase & res = *(opthelp.result_ptr);
    if (opthelp.kind == OptKind::present) {
      if (res.present()) ostr << opthelp.option << endl;
      else               ostr << absence_prefix << opthelp.option << endl;
    } else if (opthelp.kind == OptKind::optional_value) {
      if (res.present()) ostr << presence_prefix << opthelp.option << " " << res.value_as_string() << endl;
      else               ostr << absence_prefix << opthelp.option << " " << opthelp.argname << endl;
    } else {      
      ostr << presence_prefix << opthelp.option << " " << res.value_as_string() << endl;
    }
  };

  vector<OptSection> sections = organised_options();
  for (const auto & section: sections) {
    // print a section header if appropriate
    if (section.level > 0) {
      if (!compact) ostr << prefix << endl;
      if (!compact) ostr << prefix << string(section.name.size(), section.level == 1 ? '-' : '.') << endl;
      ostr << prefix << section.name << endl;
      if (!compact) ostr << prefix << string(section.name.size(), section.level == 1 ? '-' : '.') << endl;
    }

    // then print the options in that section (or subsection)
    for (const auto & opthelp: section.options) {
      if (opthelp->no_dump) continue;
      if (!compact) ostr << prefix << "\n" << opthelp->description(prefix);
      print_option(*opthelp);
    }
  }

  return ostr.str();
}


// //------------------------------------------------------------------------
// /// return a std::string in argfile format that contains all
// /// options queried and, where relevant, their values
// /// - if an option is optional (no default), it is printed commented-out
// /// - if an option was not supplied but has a default, it is printed out with its default
// ///
// /// @param prefix is the string the precedes each description line (default is "# ")
// /// @param absence_prefix is the string that precedes each line for an option that was not present
// string CmdLine::dump(const string & prefix, const string & absence_prefix) const {
//   ostringstream ostr;
//   map<string,vector<const OptionHelp *> > opthelp_section_contents;
//   vector<string> opthelp_sections;
// 
//   ostr << prefix << "argfile for " << command_line() << endl;
//   ostr << wrap(__overall_help_string, 80, prefix) << endl;
//   ostr << prefix << "generated by CmdLine::dump() on " << time_stamp() << endl;
// 
//   auto print_option = [&](const OptionHelp & opthelp) {
//     const ResultBase & res = *(opthelp.result_ptr);
//     if (opthelp.kind == OptKind::present) {
//       if (res.present()) ostr << opthelp.option << endl;
//       else               ostr << absence_prefix << opthelp.option << endl;
//     } else if (opthelp.kind == OptKind::optional_value) {
//       if (res.present()) ostr << opthelp.option << " " << res.value_as_string() << endl;
//       else               ostr << absence_prefix << opthelp.option << " " << opthelp.argname << endl;
//     } else {      
//       ostr << opthelp.option << " " << res.value_as_string() << endl;
//     }
//   };
// 
//   for (const auto & opt: __options_queried) {
//     const OptionHelp & opthelp = __options_help[opt];
//     if (opthelp.no_dump) continue;
//     if (opthelp.section == "") {
//       ostr << prefix << "\n" << opthelp.description(prefix);
//       print_option(opthelp);
//     } else {
//       // if an option is in a section, register it for later
//       if (opthelp_section_contents.find(opthelp.section) == opthelp_section_contents.end()) {
//         opthelp_sections.push_back(opthelp.section);
//       }
//       opthelp_section_contents[opthelp.section].push_back(&opthelp);
//     }
//   }
// 
//   // then print out the options that are in sections
//   for (const auto & section: opthelp_sections) {
//     ostr << prefix << endl;
//     ostr << prefix << string(section.size(),'-') << endl ;
//     ostr << prefix << section << endl;
//     ostr << prefix << string(section.size(),'-') << endl ;
//     for (const auto & opthelp: opthelp_section_contents[section]) {
//       ostr << prefix << "\n" << opthelp->description(prefix) << prefix << endl;
//       print_option(*opthelp);
//     }
//   }
//   
//   return ostr.str();
// }


// From https://www.jeremymorgan.com/tutorials/c-programming/how-to-capture-the-output-of-a-linux-command-in-c/
string CmdLine::stdout_from_command(string cmd) const {

  string data;
  FILE * stream;
  const int max_buffer = 1024;
  char buffer[max_buffer];
  cmd.append(" 2>&1");
  
  stream = popen(cmd.c_str(), "r");
  if (stream) {
    while (!feof(stream))
      if (fgets(buffer, max_buffer, stream) != NULL) data.append(buffer);
    pclose(stream);
  }
  return data;
}

//
string CmdLine::git_info() const {
  if (!__git_info_enabled) return "unknown (disabled)";

  string log_line = stdout_from_command("git log --pretty='%H %d of %cd' --decorate=short -1");
  for (auto & c : log_line) {if (c == 0x0a || c == 0x0d) c = ';';}
  
  if (log_line.substr(0,6) == "fatal:") {
    log_line = "no git info";
  } else {
    // add info about potentially modified files
    string modifications;
    string status_output = stdout_from_command("git status --porcelain --untracked-files=no");
    for (auto & c : status_output) {if (c == 0x0a || c == 0x0d) c = ',';}
    log_line += "; ";
    log_line += status_output;
  }
  return log_line;
}

template<> std::string CmdLine_string_to_value<string>(const std::string & str) {return str;}


/// specialisation for bools, to allow for 1/0, on/off, true/false .true./.false., yes/no
template<> bool CmdLine_string_to_value<bool>(const std::string & str) {
  // get the lower-case string
  string lcstr = str;
  // taken from https://stackoverflow.com/questions/313970/how-to-convert-an-instance-of-stdstring-to-lower-case
  // adapted from https://notfaq.wordpress.com/2007/08/04/cc-convert-string-to-upperlower-case/
  std::transform(str.begin(), str.end(), lcstr.begin(),
    [](unsigned char c){ return std::tolower(c); });

  if (lcstr == "1" || lcstr == "yes" || lcstr == "on" || lcstr == "true" || lcstr == ".true.") return true;
  if (lcstr == "0" || lcstr == "no" || lcstr == "off" || lcstr == "false" || lcstr == ".false.") return false;
  throw CmdLine::ConversionFailure(str);
}

// all the terminal control strings

std::string CmdLine::tc::red = "\033[31m";
std::string CmdLine::tc::grn = "\033[32m";
std::string CmdLine::tc::yel = "\033[33m";
std::string CmdLine::tc::blu = "\033[34m";
std::string CmdLine::tc::mag = "\033[35m";
std::string CmdLine::tc::cyn = "\033[36m";
std::string CmdLine::tc::wht = "\033[37m";
std::string CmdLine::tc::blk = "\033[30m";
std::string CmdLine::tc::gry = "\033[90m";
std::string CmdLine::tc::org = "\033[91m";

std::string CmdLine::tc::red_bg = "\033[41m";
std::string CmdLine::tc::grn_bg = "\033[42m";
std::string CmdLine::tc::yel_bg = "\033[43m";
std::string CmdLine::tc::blu_bg = "\033[44m";
std::string CmdLine::tc::mag_bg = "\033[45m";
std::string CmdLine::tc::cyn_bg = "\033[46m";
std::string CmdLine::tc::wht_bg = "\033[47m";
std::string CmdLine::tc::blk_bg = "\033[40m";
std::string CmdLine::tc::gry_bg = "\033[100m";
std::string CmdLine::tc::org_bg = "\033[101m";

// versions with full names for those colors that have >3 letters
// both foreground and background
std::string CmdLine::tc::yellow  = "\033[33m";
std::string CmdLine::tc::blue    = "\033[34m";
std::string CmdLine::tc::magenta = "\033[35m";
std::string CmdLine::tc::cyan    = "\033[36m";
std::string CmdLine::tc::white   = "\033[37m";
std::string CmdLine::tc::black   = "\033[30m";
std::string CmdLine::tc::gray    = "\033[90m";
std::string CmdLine::tc::grey    = "\033[90m";
std::string CmdLine::tc::orange  = "\033[91m";

std::string CmdLine::tc::yellow_bg  = "\033[43m";
std::string CmdLine::tc::blue_bg    = "\033[44m";
std::string CmdLine::tc::magenta_bg = "\033[45m";
std::string CmdLine::tc::cyan_bg    = "\033[46m";
std::string CmdLine::tc::white_bg   = "\033[47m";
std::string CmdLine::tc::black_bg   = "\033[40m";
std::string CmdLine::tc::gray_bg    = "\033[100m";
std::string CmdLine::tc::grey_bg    = "\033[100m";
std::string CmdLine::tc::orange_bg  = "\033[101m";


std::string CmdLine::tc::bold = "\033[1m";
std::string CmdLine::tc::nobold = "\033[22m";
std::string CmdLine::tc::italics = "\033[3m";

std::string CmdLine::tc::underline = "\033[4m";
std::string CmdLine::tc::reverse = "\033[7m";
std::string CmdLine::tc::reset = "\033[0m";
std::string CmdLine::tc::clear = "\033[2J\033[H"; ///< clear screen and move cursor to home
std::string CmdLine::tc::clear_screen = "\033[2J";
std::string CmdLine::tc::clear_line = "\033[2K\r";

