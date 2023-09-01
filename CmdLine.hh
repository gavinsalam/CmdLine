///////////////////////////////////////////////////////////////////////////////
// File: CmdLine.hh                                                          //
// Part of the CmdLine library
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


#ifndef __CMDLINE__
#define __CMDLINE__

#include<string>
#include<sstream>
#include<map>
#include<vector>
#include<ctime>
#include<typeinfo> 

/// Class designed to deal with command-line arguments.
///
/// Basic usage:
///
/// \code
///
/// #include "CmdLine.hh"
/// 
/// int main(int argc, char** argv) {
///   CmdLine cmdline(argc,argv);
///   cmdline.help("Overall help for your program");
///   
///   // required argument, no help string
///   double x = cmdline.value<double>("-x");
/// 
///   // optional argument, with default value, and help string
///   double y = cmdline.value("-y",1.0).help("sets the value of y");
/// 
///   //
///   bool b_is_present = cmdline.present("-b").help("sets b_is_present to true");
/// 
///   // makes sure that all provided command-line options have been used
///   // (also triggers printout of help if -h was present)
///   cmdline.assert_all_options_used();
/// }
///
/// \endcode
class CmdLine {
 public :

  /// class to store help related to an option
  class OptionHelp {
  public:
    std::string option, default_value, help, argname="val";
    std::string type;
    std::vector<std::string> choices;
    std::vector<std::string> range_strings;
    bool required;
    bool takes_value;
    bool has_default;
    /// returns a short summary of the option (suitable for
    /// placing in the command-line summary
    std::string summary() const; 
    /// returns a longer description of the option (suitable for
    /// placing in the more extended help)
    std::string description() const;
    /// returns an attempt at a human readable typename
    std::string type_name() const;
    /// returns a string with a comma-separated list of choices
    std::string choice_list() const;
    /// returns the string with the allowed range
    std::string range_string() const;
  };

  /// class that contains the result of an option.
  /// Can be implicitly converted to type T, and can also be used
  /// to add help information to the option.
  template<class T>
  class Result {
  public:
    Result(const T & t) : _t(t), _opthelp(0), _is_present(true) {}
    Result(const T & t, OptionHelp * opthelp_ptr, bool is_present) : 
               _t(t), _opthelp(opthelp_ptr), _is_present(is_present) {}

    /// this allows for implicit conversion to type T in assignments
    operator T() const {return _t;}

    /// this allows the user to do the conversion manually
    T operator()() const {return _t;}

    /// return true if the argument was present on the command-line
    bool present() const {return _is_present;}

    /// for adding help to an option
    const Result & help(const std::string & help_string) const {
      opthelp().help = help_string;
      return *this;
    }

    /// for adding an argument name to an option
    const Result & argname(const std::string & argname_string) const {
      opthelp().argname = argname_string;
      return *this;
    }    

    /// @brief sets the allowed choices
    /// @param allowed_choices 
    /// @return the Result object
    const Result & choices(const std::vector<T> allowed_choices) const; 

    /// sets the allowed range: minval  <= arg <= maxval
    const Result & range(T minval, T maxval) const; 

    /// returns a reference to the option help, and throws an error if
    /// there is no help
    OptionHelp & opthelp() const;

    /// sets a pointer to the help instance for this argument.
    void set_opthelp(OptionHelp * opthelp) {_opthelp = opthelp;}

  private:
    T _t;
    mutable OptionHelp * _opthelp;
    bool _is_present;
  };
  
  CmdLine() {};
  /// initialise a CmdLine from a C-style array of command-line arguments
  CmdLine(const int argc, char** argv, bool enable_help = true, const std::string & file_option=_default_argfile_option );
  /// initialise a CmdLine from a C++ std::vector of arguments 
  CmdLine(const std::vector<std::string> & args, bool enable_help = true, const std::string & file_option=_default_argfile_option );

  /// Add an overall help string
  CmdLine & help(const std::string & help_str);
  
  /// true if the option is present
  Result<bool> present(const std::string & opt) const;

  /// returns the value of the argument converted to type Result<T>
  template<class T> Result<T> value(const std::string & opt) const;

  /// returns the value of the option, or defval if the option is not present
  template<class T> Result<T> value(const std::string & opt, const T & defval) const;

  /// returns a Result<T> for the option; the result.present() should be queried to
  /// see if it was present before trying to use the value
  template<class T> Result<T> optional_value(const std::string & opt) const;

  /// returns the value of the argument, prefixed with prefix (NB: 
  /// require different function name to avoid confusion with 
  /// 2-arg template).
  template<class T> Result<T> value_prefix(const std::string & opt, const std::string & prefix) const;
  template<class T> Result<T> value(const std::string & opt, const T & defval, 
                                    const std::string & prefix) const;



  /// true if the option is present and corresponds to a value
  bool         present_and_set(const std::string & opt) const;

  /// when an option is missing but help has been asked for, we will
  /// still return a value, specified by this function, which can
  /// be specialised by the user if they want to extend it to
  /// further types (currently defaults to 0, except for strings
  /// where it defaults to the empty string).
  template<class T> T value_for_missing_option() const;
  
  /// return a reference to the std::vector of command-line arguments (0 is
  /// command).
  inline const std::vector<std::string> & arguments() const {return __arguments;}



  /// return the full command line
  std::string command_line() const;

  /// print the help std::string that has been deduced from all the options called
  void print_help() const;
  
  /// return true if all options have been asked for at some point or other
  bool all_options_used() const;

  /// gives an error if there are unused options
  void assert_all_options_used() const;

  /// return a time stamp (UTC) corresponding to now
  std::string time_stamp(bool utc = false) const;

  /// return a time stamp (UTC) corresponding to time of object construction
  std::string time_stamp_at_start(bool utc = false) const;

  /// return the elapsed time in seconds since the CmdLine object was
  /// created
  double time_elapsed_since_start() const;

  /// return output similar to that from uname -a on unix
  std::string unix_uname() const;

  /// return the username
  std::string unix_username() const;

  /// In C++17 we don't need this, we can instead use std::filesystem::current_path();
  /// But for compatibility with older system it is useful to have
  std::string current_path() const;

  /// enable/disable git info support (on by default)
  CmdLine & set_git_info_enabled(bool enable=true) {_git_info_enabled = enable; return *this;}
  bool git_info_enabled() const {return _git_info_enabled;}

  /// returns a string with basic info about the git
  std::string git_info() const;



  /// return a multiline header that contains
  /// - the command line
  /// - the current directory path
  /// - the start time
  /// - the user
  /// - the system name
  /// The header includes a final newline
  std::string header(const std::string & prefix = "# ") const;
  
  class Error;

  /// return the integer value corresponding to the given option
  [[deprecated]]
  int     int_val(const std::string & opt) const;
  /// return the integer value corresponding to the given option or default if option is absent
  [[deprecated]]
  int     int_val(const std::string & opt, const int & defval) const;

  /// return the double value corresponding to the given option
  [[deprecated]]
  double  double_val(const std::string & opt) const;
  /// return the double value corresponding to the given option or default if option is absent
  [[deprecated]]
  double  double_val(const std::string & opt, const double & defval) const;

  /// return the std::string value corresponding to the given option
  [[deprecated("use value<std::string>(opt) instead")]]
  std::string  string_val(const std::string & opt) const;

  /// return the std::string value corresponding to the given option or default if option is absent
  [[deprecated("use value<std::string>(opt, defval) instead")]]
  std::string  string_val(const std::string & opt, const std::string & defval) const;



 private:

  /// returns the stdout (and stderr) from the command
  std::string stdout_from_command(std::string cmd) const;

  std::string  internal_string_val(const std::string & opt) const;


  /// stores the command line arguments in a C++ friendly way
  std::vector<std::string> __arguments;

  /// a map of possible options found on the command line, referencing
  /// the index of the argument that might assign a value to that
  /// option (an option being anything starting with a dash)
  mutable std::map<std::string,int> __options;

  /// whether a given option has been requested
  mutable std::map<std::string,bool> __options_used;

  /// whether help functionality is enabled
  bool __help_enabled;
  /// whether the user has requested help with -h or --help
  bool __help_requested;
  /// whether the git info is included or not
  bool _git_info_enabled;
  
  template<class T>
  OptionHelp OptionHelp_value_with_default(const std::string & option, const T & default_value,
                                     const std::string & help_string = "") const {
    OptionHelp help;
    help.option        = option;
    std::ostringstream defval_ostr;
    defval_ostr << default_value;
    help.default_value = defval_ostr.str();
    help.help          = help_string;
    help.type          = typeid(T).name();
    help.required      = false;
    help.takes_value   = true;
    help.has_default   = true;
    return help;
  }
  template<class T>
  OptionHelp OptionHelp_value_required(const std::string & option,
                                       const std::string & help_string = "") const {
    OptionHelp help;
    help.option        = option;
    help.default_value = "";
    help.help          = help_string;
    help.type          = typeid(T).name();
    help.required      = true;
    help.takes_value   = true;
    help.has_default   = false;
    return help;
  }
  template<class T>
  OptionHelp OptionHelp_optional_value(const std::string & option,
                                       const std::string & help_string = "") const {
    OptionHelp help;
    help.option        = option;
    help.default_value = "";
    help.help          = help_string;
    help.type          = typeid(T).name();
    help.required      = false;
    help.takes_value   = true;
    help.has_default   = false;
    return help;
  }
  OptionHelp OptionHelp_present(const std::string & option,
                                const std::string & help_string = "") const {
    OptionHelp help;
    help.option        = option;
    help.default_value = "";
    help.help          = help_string;
    help.type          = "";
    help.required      = false;
    help.takes_value   = false;
    help.has_default   = false;
    return help;
  }
  
  /// return a pointer to an existing opthelp is the option is present
  /// otherwise register the given opthelp and return a pointer to that
  /// (if help is disabled, return a null poiner)
  OptionHelp * opthelp_ptr(const OptionHelp & opthelp) const;

  /// a std::vector of the options queried (this may evolve)
  mutable std::vector<std::string> __options_queried;
  /// a map with help for each option that was queried
  mutable std::map<std::string, OptionHelp> __options_help;
  
  //std::string __progname;
  std::string __command_line;
  std::time_t __time_at_start;
  std::string __overall_help_string;

  /// default option to tell CmdLine to read arguments 
  /// from a file
  static std::string _default_argfile_option;
  std::string __argfile_option = _default_argfile_option;
  

  /// builds the internal structures needed to keep track of arguments and options
  void init();

  /// report failure of conversion
  void _report_conversion_failure(const std::string & opt, 
                                  const std::string & optstring) const;

  /// convert the time into a std::string (local by default -- utc if 
  /// utc=true).
  std::string _string_time(const time_t & time, bool utc) const;
};



//----------------------------------------------------------------------
/// class that deals with errors
class CmdLine::Error {
public:
  Error(const std::ostringstream & ostr);
  Error(const std::string & str);
  const char* what() throw() {return _message.c_str();}
  const std::string & message() throw() {return _message;}
  static void set_print_message(bool doprint) {_do_printout = doprint;}
private:
  std::string _message;
  static bool _do_printout;
};


template<class T>
CmdLine::OptionHelp & CmdLine::Result<T>::opthelp() const {
  if (_opthelp) {
    return *_opthelp;
  } else {
    throw CmdLine::Error("tried to access optionhelp for option where it does not exist\n"
                  "(e.g. because option help already set for an identical option earlier)");
  }
}

template<class T> inline T CmdLine::value_for_missing_option() const {return T(0);}
template<> inline std::string CmdLine::value_for_missing_option<std::string>() const {return "";}

/// returns the value of the argument, convertible to type T
template<class T> CmdLine::Result<T> CmdLine::value(const std::string & opt) const {
  // we create the result from the (more general) value_prefix
  // function, with an empty prefix
  return value_prefix<T>(opt,"");
}

/// returns the value of the argument converted to type T
template<class T> CmdLine::Result<T> CmdLine::value_prefix(const std::string & opt, const std::string & prefix) const {
  OptionHelp * opthelp = opthelp_ptr(OptionHelp_value_required<T>(opt, ""));

  T result;
  if (present_and_set(opt)) {
    std::string optstring = prefix+internal_string_val(opt);
    std::istringstream optstream(optstring);
    optstream >> result;
    if (optstream.fail()) _report_conversion_failure(opt, optstring);
  } else if (__help_requested) {
    result = value_for_missing_option<T>();
  } else {
    std::ostringstream ostr;
    ostr << "Option "<< opt
         <<" is needed but is not present_and_set"<< std::endl;
    throw(Error(ostr)); 
  }
  return Result<T>(result, opthelp, true);
}

/// for the std::string case, just copy the std::string...
template<> inline CmdLine::Result<std::string> CmdLine::value<std::string>(const std::string & opt) const {
  OptionHelp * opthelp = opthelp_ptr(OptionHelp_value_required<std::string>(opt, ""));

  std::string result;
  // following bit of code is largely repeated from value_prefix.
  // Consider folding into a separate routine?
  if (present_and_set(opt)) {
    result = internal_string_val(opt);
  } else if (__help_requested) {
     result = value_for_missing_option<std::string>();
  } else {
    std::ostringstream ostr;
    ostr << "Option "<<opt
         <<" is needed but is not present_and_set"<< std::endl;
    throw(Error(ostr)); 
  }
  return Result<std::string>(result, opthelp, true);
}



template<class T> CmdLine::Result<T> CmdLine::value(const std::string & opt, const T & defval) const {
  // construct help
  OptionHelp * opthelp = opthelp_ptr(OptionHelp_value_with_default(opt, defval, ""));

  // return value
  if (this->present_and_set(opt)) {
    auto result = value<T>(opt);
    result.set_opthelp(opthelp);
    return result;
  } else {
    return Result<T>(defval,opthelp,false);
  }
}

template<class T> CmdLine::Result<T> CmdLine::optional_value(const std::string & opt) const {
  // construct help
  OptionHelp * opthelp = opthelp_ptr(OptionHelp_optional_value<T>(opt));
  opthelp->default_value = "None";

  // return value
  if (this->present_and_set(opt)) {
    auto result = value<T>(opt);
    result.set_opthelp(opthelp);
    return result;
  } else {
    return Result<T>(value_for_missing_option<T>(), opthelp, false);
  }
}


template<class T> CmdLine::Result<T> CmdLine::value(const std::string & opt, const T & defval, 
                                   const std::string & prefix) const {
  if (this->present_and_set(opt)) {return value_prefix<T>(opt, prefix);} 
  else {
    OptionHelp * opthelp = opthelp_ptr(OptionHelp_value_with_default(opt, defval, ""));
    return Result<T>(defval,opthelp,false);
  }
}

template<class T>
std::ostream & operator<<(std::ostream & ostr, const CmdLine::Result<T> & result) {
  ostr << result();
  return ostr;
}

template<class T>
const CmdLine::Result<T> & CmdLine::Result<T>::choices(const std::vector<T> allowed_choices) const {
  // register the choices with the help module
  for (const auto & choice: allowed_choices) {
    std::ostringstream ostr;
    ostr << choice;
    _opthelp->choices.push_back(ostr.str());
  }

  // check the choice actually made is valid
  bool valid = false;
  for (const auto & choice: allowed_choices) {
    if (_t == choice) {valid = true; break;}
  }
  if (!valid) {
    std::ostringstream ostr;
    ostr << "For option " << _opthelp->option << ", invalid option value " 
        << _t << ". Allowed choices are: " << _opthelp->choice_list();
    throw Error(ostr.str());
  }
  return *this;
}

template<class T>
const CmdLine::Result<T> & CmdLine::Result<T>::range(T minval, T maxval) const {
  std::ostringstream minstr, maxstr;
  minstr << minval;
  maxstr << maxval;
  _opthelp->range_strings.push_back(minstr.str());
  _opthelp->range_strings.push_back(maxstr.str());
  if (_t < minval || _t > maxval) {
    std::ostringstream errstr;
    errstr << "For option " << _opthelp->option << ", option value " << _t 
           << " out of allowed range: " 
           << _opthelp->range_string();
    throw Error(errstr.str());
  }
  return *this;
}


#endif
