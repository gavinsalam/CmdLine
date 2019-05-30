README for the CmdLine directory
--------------------------------

A simple library to provde access to command line options.

type 

 make 

to build the library and the example program. To include it in your
own projects, you can simply copy the CmdLine.cc and CmdLine.hh
files. 

Many command-line libraries require you to declare the options you
want and then process the command line to access them. This one
instead gives you immediate access to the value of the argument.  One
advantage of such an approach is that it makes it easier to take a
first group of command-line arguments and use their values to adapt a
second set of command line arguments.

Help is to some extent automated, though as of May 2019 it does not
yet handle complex cases where one set of arguments depends on a
previous set. 

## Simple usage

```c++
#include "CmdLine.hh"

int main(int argc, char** argv) {
  CmdLine cmdline(argc,argv);
  cmdline.help("Overall help for your program");
  
  // required argument, no help string
  double x = cmdline.value<double>("-x");

  // optional argument, with default value for when the argument is
  // absent, and help string
  double y = cmdline.value("-y",1.0).help("sets the value of y");

  //
  bool b_is_present = cmdline.present("-b").help("sets b_is_present to true");

  // makes sure that all provided command-line options have been used
  // (also triggers printout of help if -h was present)
  cmdline.assert_all_options_used();
}
```


