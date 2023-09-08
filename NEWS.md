
Towards Version 3.1.1
---------------------

### other changes
- CmdLine::section() and help_requested() members to return current section
  and help requested status

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