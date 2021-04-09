#include "CmdLine.hh"
#include<iostream>

using namespace std;

int main (int argc, char ** argv) {
  // run with '-file file-example.cmnd' to load options listed there

  // construct the cmdline object
  bool enable_help = true;
  CmdLine cmdline(argc, argv, enable_help);
  cmdline.help("Small program to illustrate using a file with options.");

  auto ival = cmdline.value<int>("-i").argname("ival")
              .help("required argument, illustrates obtaining an int from the command line");
  
  // the value<T> template deduces the correct type from the
  // default value for the option (if present)
  double dval = cmdline.value("-d",0.0).argname("dval")
              .help("optional argument, illustrates "
                    "obtaining a double from the command line");

  // for options with a default character value, we usually want
  // a string result -- so this must be specified explicitly
  string sval = cmdline.value<string>("-s","default-string").argname("sval")
    .help("optional argument, illustrates obtaining a string from the command line");

  bool flag = cmdline.present("-f").help("illustrates a command-line flag");
  
  // make sure we've used all options that were provided on command-line.
  // If the user asked for help (-h or --help) then execution will stop here.
  cmdline.assert_all_options_used();

  // output a header with various info (command-line, path, time, system)
  cout << cmdline.header() ;
  // output the values
  cout << "ival = " << ival << endl;
  cout << "dval = " << dval << endl;
  cout << "sval = " << sval << endl;
  cout << "flag = " << flag << endl;

}