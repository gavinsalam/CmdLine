//STARTHEADER
// $Id: CmdLine.cc 293 2006-08-17 19:38:38Z salam $
//
// Copyright (c) 2005 Matteo Cacciari and Gavin Salam
//
//----------------------------------------------------------------------
// This file is part of a simple command-line handling environment
//
//  FastJet is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  The algorithms that underlie FastJet have required considerable
//  development and are described in hep-ph/0512210. If you use
//  FastJet as part of work towards a scientific publication, please
//  include a citation to the FastJet paper.
//
//  FastJet is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with FastJet; if not, write to the Free Software
//  Foundation, Inc.:
//      59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//----------------------------------------------------------------------
//ENDHEADER


#include "CmdLine.hh"
#include<string>
#include<sstream>
#include<iostream> // testing
#include<vector>
#include<cstddef> // for size_t
using namespace std;

// initialise the various structures that we shall
// use to access the command-line options;
//
// If an option appears several times, it is its LAST value
// that will be used in searching for option values (opposite of f90)
CmdLine::CmdLine (const int argc, char** argv) {
  __arguments.resize(argc);
  for(int iarg = 0; iarg < argc; iarg++){
    __arguments[iarg] = argv[iarg];
  }
  this->init();
}

/// constructor from a vector of strings, one argument per string
CmdLine::CmdLine (const vector<string> & args) {
  __arguments = args;
  this->init();
}

//----------------------------------------------------------------------
void CmdLine::init (){
  __command_line = "";
  for(size_t iarg = 0; iarg < __arguments.size(); iarg++){
    __command_line += __arguments[iarg];
    __command_line += " ";
  }
  
  // group things into options
  bool next_may_be_val = false;
  string currentopt;
  for(size_t iarg = 1; iarg < __arguments.size(); iarg++){
    // if expecting an option value, then take it (even if
    // it is actually next option...)
    if (next_may_be_val) {__options[currentopt] = iarg;}
    // now see if it might be an option itself
    string arg = __arguments[iarg];
    bool thisisopt = (arg.compare(0,1,"-") == 0);
    if (thisisopt) {
      // set option to a standard undefined value and say that 
      // we expect (possibly) a value on next round
      currentopt = arg;
      __options[currentopt] = -1;
      __options_used[currentopt] = false;
      next_may_be_val = true;}
    else {
      // otherwise throw away the argument for now...
      next_may_be_val = false;
      currentopt = "";
    }
  }
}

// indicates whether an option is present
bool CmdLine::present(const string & opt) {
  bool result = (__options.find(opt) != __options.end());
  if (result) __options_used[opt] = true;
  return result;
}

// indicates whether an option is present and has a value associated
bool CmdLine::present_and_set(const string & opt) {
  bool result = present(opt) && __options[opt] > 0;
  return result;
}


// return the string value corresponding to the specified option
string CmdLine::string_val(const string & opt) {
  if (!this->present_and_set(opt)) {
    cerr << "Error: Option "<<opt
	 <<" is needed but is not present_and_set"<<endl;
    exit(-1);
  }
  string arg = __arguments[__options[opt]];
  // this may itself look like an option -- if that is the case
  // declare the option to have been used
  if (arg.compare(0,1,"-") == 0) {__options_used[arg] = true;}
  return arg;
}

// as above, but if opt is not present_and_set, return default
string CmdLine::string_val(const string & opt, const string & defval) {
  if (this->present_and_set(opt)) {return string_val(opt);} 
  else {return defval;}
}

// Return the integer value corresponding to the specified option;
// Not too sure what happens if option is present_and_set but does not
// have string value...
int CmdLine::int_val(const string & opt) {
  int result;
  string optstring = string_val(opt);
  istringstream optstream(optstring);
  optstream >> result;
  if (optstream.fail()) {
    cerr << "Error: could not convert option ("<<opt<<") value ("
	 <<optstring<<") to int"<<endl; 
    exit(-1);}
  return result;
}

// as above, but if opt is not present_and_set, return default
int CmdLine::int_val(const string & opt, const int & defval) {
  if (this->present_and_set(opt)) {return int_val(opt);} 
  else {return defval;}
}


// Return the integer value corresponding to the specified option;
// Not too sure what happens if option is present_and_set but does not
// have string value...
double CmdLine::double_val(const string & opt) {
  double result;
  string optstring = string_val(opt);
  istringstream optstream(optstring);
  optstream >> result;
  if (optstream.fail()) {
    cerr << "Error: could not convert option ("<<opt<<") value ("
	 <<optstring<<") to double"<<endl; 
    exit(-1);}
  return result;
}

// as above, but if opt is not present_and_set, return default
double CmdLine::double_val(const string & opt, const double & defval) {
  if (this->present_and_set(opt)) {return double_val(opt);} 
  else {return defval;}
}


// return the full command line including the command itself
string CmdLine::command_line() {
  return __command_line;
}


// return true if all options have been asked for at some point or other
bool CmdLine::all_options_used() const {
  bool result = true;
  for(map<string,bool>::const_iterator opt = __options_used.begin();
      opt != __options_used.end(); opt++) {
    bool this_one = opt->second;
    if (! this_one) {cerr << "Option "<<opt->first<<" unused"<<endl;}
    result = result && this_one;
  }
  return result;
}
