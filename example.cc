///////////////////////////////////////////////////////////////////////////////
// File: example.cc                                                          //
// Part of the CmdLine library                                               //
//                                                                           //
// Copyright (c) 2007-2019 Gavin Salam                                       //
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
#include<iostream>

using namespace std;

int main (int argc, char ** argv) {

  // construct the cmdline object
  bool enable_help = true;
  CmdLine cmdline(argc, argv, enable_help);
  cmdline.help("Small program to illustrate how the CmdLine library can be used.");

  auto ival = cmdline.value<int>("-i").argname("ival").choices({0,1,2})
              .help("required argument, illustrates obtaining an int from the command line"
              ", with a long help line to verify that it gets wrapped");
  

  //---------------------------------------------------------------------------
  cmdline.start_section("optional arguments, with defaults"); 
  // the value<T> template deduces the correct type from the
  // default value for the option (if present)
  auto dres = cmdline.value("-d",0.0).argname("dval").range(-1.0, 2.0)
              .help("optional argument, illustrates "
                    "obtaining a double from the command line");  
  bool d_present = dres.present();
  double dval = dres;

  // for options with a default character value, we usually want
  // a string result -- so this must be specified explicitly
  string sval = cmdline.value<string>("-s","default-string").argname("sval")
    .help("optional argument, illustrates obtaining a string from the command line");

  //---------------------------------------------------------------------------
  cmdline.start_section("optional arguments, no defaults"); 

  // optional argument, which if present, takes a value (user must
  // check whether it was present before using the value)
  auto ores = cmdline.optional_value<double>("-o").help("optional argument that takes value");

  //---------------------------------------------------------------------------
  cmdline.end_section();

  // optional flag, which if present, is true, otherwise false
  bool flag = cmdline.present("-f").help("illustrates a command-line flag");
  
  //---------------------------------------------------------------------------
  // make sure we've used all options that were provided on command-line.
  // If the user asked for help (-h or --help) then execution will stop here.
  cmdline.assert_all_options_used();

  // output a header with various info (command-line, path, time, system)
  cout << cmdline.header() ;
  // output the values
  cout << "ival = " << ival << endl;
  cout << "dval = " << dval << " (argument was " << (d_present ? "" : "not " ) << "present)" << endl;
  cout << "sval = " << sval << endl;
  if (ores.present()) cout << "oval = " << ores() << endl;
  else                cout << "oval = " << "not present" << endl; 
  cout << "flag = " << flag << endl;

  // this can be used to dump the state of all options
  // suitable for reading back in with the -argfile option
  // cout << cmdline.dump() << endl;

}
