#include "CmdLine.hh"
#include<iostream>

using namespace std;

int main (int argc, char ** argv) {

  CmdLine cmdline(argc, argv);

  //double dval = cmdline.double_val("-d",1.0);
  double dval = cmdline.value("-d",0.0);
  cout << "dval = " <<dval << endl;
  string sval = cmdline.value<string>("-s","not there");
  cout << "sval = " <<sval << endl;
}
