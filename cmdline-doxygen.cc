///////////////////////////////////////////////////////////////////////////////
// File: cmdline-doxygen.cc                                                  //
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


// file whose sole purpose is to generate the doxygen mainpage

/// \mainpage CmdLine code documentation
///
/// The main classes are CmdLine and the result of an option, CmdLine::Result.
///
/// Include the CmdLine.hh and CmdLine.hh files in your project and then use it
/// as follows:
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
///   // (this actually returns a CmdLine::Result<double> object, on which
///   // varioius options can be set; it then then automatically gets converted
///   // to a double when assigned to y).
///   double y = cmdline.value("-y",1.0).help("sets the value of y");
/// 
///   // a flag, with help string
///   bool b_is_present = cmdline.present("-b").help("sets b_is_present to true");
/// 
///   // makes sure that all provided command-line options have been used
///   // (also triggers printout of help if -h was present)
///   cmdline.assert_all_options_used();
/// }
///
/// \endcode