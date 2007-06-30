#!/usr/bin/perl -w
#
# script to help conversion of enums to and from text, via istream and 
# ostream operators
#
# Usage:
# make-enum-IO.pl outfile.hh infile1.hh:Enum1[:Enum2[:..]] [...]
#
#

$usage="make-enum-IO.pl outfile.hh infile1.hh:Enum1[:Enum2[:..]] [...]";
$cmdline_error="Error: unsuitable command line\nUsage: $usage\n";
$cmdline = "$0 ".join(" ",@ARGV);
if ($#ARGV < 1) {
  print $cmdline_error; exit(-1);
}

$outfile=shift @ARGV;
($outDEF = "__".(uc $outfile)."__") =~ s/\./_/;

# collect list of things
@list=();
@files=();
while ($fileenum = shift @ARGV) {
  (@enum) = split(":",$fileenum);
  if ($#enum < 1) {  print $cmdline_error; exit(-1);}
  $file = shift @enum;
  push @files, $file;
  foreach $enum (@enum) {push @list, "$file:$enum"}
}

# first prepare the headers of the files
$outputcc = "// File generated automatically with the following command
// $cmdline
// to help with I/O of enums\n";
$outputhh = "#ifndef $outDEF
#define $outDEF
$outputcc

#include<iostream>\n\n\n";
$outputcc .= "
#include \"$outfile\"
#include <string>
";
foreach $file (@files) {$outputhh .= "#include \"$file\"\n";}

# now for each file get our list
foreach $fileenum (@list) {
  ($file,$enum) = split(":",$fileenum);
  open(FILE, "<$file") || die "Could not open $file";
  # find the enum
  $result="";
  while ($line = <FILE>) {
    $line =~ s/\/\/.*//; # remove comments
    chomp($line);
    if ($line =~ "enum $enum") {$result=$line}
    elsif ($result) {
      $result .= $line;
      if ($result =~ /\}\s*\;/) {last;}
    }
  }
  # now parse the enum -- reduce it to list of chars
  $result =~ s/.*\{//;
  $result =~ s/[}; ]//g;
  $result =~ s/=[^,]*//g;
  @vals=split(",",$result);

  # and write the corresponding output routines
  $outputcommon = "/// overloaded output for $enum
std::ostream & operator<<(std::ostream & ostr, $enum val)";
  $outputhh .= "$outputcommon;\n\n";
  $outputcc .= "$outputcommon {
  switch(val) {\n";
  foreach $val (@vals) {
    $outputcc .= "  case $val: ostr << \"$val\"; return ostr;\n";
  }
  $outputcc .= "  default: ostr << \"[unrecognized $enum]\"; return ostr;\n}\n\n";

  # and write the corresponding input routines
  $outputcommon = "/// overloaded input for $enum
std::istream & operator>>(std::istream & istr, $enum & val)";
  $outputhh .= "$outputcommon;\n\n";
  $outputcc .= "$outputcommon {
  std::string strval;
  istr >> strval;
";
  foreach $val (@vals) {
    $outputcc .= "  if (strval == \"$val\") {val = $val}\n  else";
  }
  $outputcc .= "  {std::cerr << \"Could not interpret \" << strval << \" as a $enum\\n\";
    exit(-1);}
  return istr;\n}\n\n";
  
}

# pur guards on
$outputhh .= "#endif // $outDEF\n";


if (! ($outfile =~ /\.hh$/)) {
  die "Error: Outfile should end in .hh but is $outfile\n";}
open (HH ,">$outfile") || die "Could not open $outfile for output";
print "Generating $outfile\n";
print HH $outputhh;

$outfile =~ s/\.hh$/.cc/;
open (CC ,">$outfile") || die "Could not open $outfile for output";
print "Generating $outfile\n";
print CC $outputcc;


