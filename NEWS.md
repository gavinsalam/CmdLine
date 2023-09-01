
Towards version 3.1.0: 2023-09
------------------------------

### new features

- increased C++ standard from C++11 to C++14, to allow for deprecations

- added CmdLine::optional_value<T>("-opt"), for options that take a value
  but are optional and do not have a default

- new CmdLine::Result<T>::present() function to tell if an option was
  actually present on the command line (e.g. notably for use with
  optional_value results) 

- added CmdLine::start_section(..) CmdLine::end_section() to allow for
  basic sectioning of help text 

### other changes
- -argfile files now allow # as a comment character (in addition to //)

### deprecations

- all string_val, double_val, int_val functions are deprecated. 
  Use value<string>, value<double>, value<int> instead.


version 3.0.0 [undated]
-----------------------

# new features
- basic handling of help