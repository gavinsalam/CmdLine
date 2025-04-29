
Version 3.2.0: 2025-04-29
-------------------------

### new features

- when there are multiple forms of an option, instead of
  `any_value<T>({"-l,"--long"})`, one can now simply use
  `value<T>({"-l","--long"})` to get the same effect. 
  This is the preferred option.

- added `CmdLine::value_bool("-opt",true/false)` option, which can process
  -opt true/false (or yes/no, on/off, 1/0), as well as simply -opt and -no-opt
  and returns true/false accordingly.

- name demangling is enabled via cxxabi.h if CmdLine.cc is compiled with
  `-DCMDLINE_ENABLE_DEMANGLE`. This is useful for printing out type
  names in the help output.

- subsections now possible, with CmdLine::start_subsection(...)

- sections and subsections can have descriptions, which are printed out
  in the help text -- just add a second argument to the
  `start_[sub]section` call. 

- as well as overloading operator<< to obtain conversions to
  specific types, the user has the option of specialising the
  static template function `T CmdLine_string_to_value<T>(...)`,
  which can be used as a last-resort for types where overloading
  `operator<<` is not an option.

- this has been used to enable `cmdline.value<bool>` to handle
  various inputs (true/false, on/off, 1/0, yes/no)

- unit-tests.cc has now been added to check various behaviours;
  can be invoked with make check

- added markdown formatted help, printed out with `--markdown-help` (or
  `-markdown-help`), together with the `CmdLine::markdown_help_requested()` 
  enquiry function.

- `CmdLine::Result<T>::choices()` can now take a second argument with a
  vector of help strings, one per valid choice. These are then printed
  out in the various help formats

### other changes
- `CmdLine::section()` and `help_requested()` members, to return current section
  and help requested status
- `CmdLine::section(...)` as a shorthand for setting the section (similar subsection()...)
- small improvements in doxygen output

Version 3.1.0: 2023-09-07
-------------------------

### new features

- increased C++ standard from C++11 to C++14, to allow for deprecations

- added CmdLine::optional_value<T>("-opt"), for options that take a value
  but are optional and do not have a default. 

- added CmdLine::Result<T>::present() function to tell if an option was
  actually present on the command line (notably for use with
  optional_value results), ::has_value() to indicate if it has a value
  and value_or(...) to get the value or an alternative if it has no value.

- added CmdLine::start_section(..) CmdLine::end_section() to allow for
  basic sectioning of help text 

- added variants of the query functions prefixed by any_, which allow the
  user to supply multiple variants (or aliases) of an option name, e.g. 
  `cmdline.any_present({"-l","--long"})`

- added CmdLine::dump() to return a string with all options and their values,
  suitable for reading with the -argfile option

- added checks that options are not being re-registered multiple times
  with inconsistent kinds or defaults (produces a warning unless the new
  CmdLine:set_fussy() function has been called, in which case a
  CmdLine::Error is thrown)

- help description gets wrapped to fit in 80 columns, via the new
  CmdLine::wrap() static function


### other changes
- -argfile files now allow # as a comment character (in addition to //)

### deprecations

- all string_val, double_val, int_val functions are deprecated. 
  Use value<string>, value<double>, value<int> instead.

- present_or_set(...) is also deprecated. Use optional_value(...) instead
  and then check if the result is present().


version 3.0.0 [undated]
-----------------------

# new features
- basic handling of help