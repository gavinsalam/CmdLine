[![Build Status](https://img.shields.io/github/actions/workflow/status/gavinsalam/CmdLine/CI.yml?label=build&logo=github&style=flat-square)](https://github.com/gavinsalam/CmdLine/actions/workflows/CI.yml)

README for the CmdLine directory
--------------------------------

A simple C++14 library to provide access to command line options.

type 

    make 

to build the library and the example program. To include it in your
own projects, you can simply copy the CmdLine.cc and CmdLine.hh
files. 

Many command-line libraries require you to declare all the options you
want and then subsequently process the command line to access them. This one
instead gives you immediate access to the value of the argument.  An
advantage of such an approach is that it avoids having to set the
variable in a different location from where it is declared or assigned.

Help is to some extent automated, thought it does not
yet handle complex cases where one set of arguments depends on a
previous set. 

## Simple usage

```c++
#include "CmdLine.hh"

int main(int argc, char** argv) {
  CmdLine cmdline(argc,argv);
  cmdline.help("Overall help for your program");
  
  // required argument, no help string provided
  double x = cmdline.value<double>("-x");

  // optional argument, with default value (1.0) for when the argument is
  // absent, and help string
  double y = cmdline.value("-y",1.0).help("sets the value of y");

  // optional argument that returns a bool; it can be used in several
  // ways, with
  // - "-b", "-b true", "-b yes", "-b on", "-b 1", all setting b_is_present to true
  // - the absence of any argument, as well as "-no-b", "-b no", etc.
  //   all setting b_is_present to false
  bool b = cmdline.value_bool("-b",false).help("sets b to true or false");

  // makes sure that all provided command-line options have been used
  // (also triggers printout of help if -h was present)
  cmdline.assert_all_options_used();
}
```

Run `example -h` to see an illustrative help message, or 
`example -markdown-help` to see the help message in markdown format.