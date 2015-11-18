#!/bin/zsh

mkdir tmp || exit
if [[ ! -e releases ]] mkdir releases

tar zcf tmp/tmp.tgz *.cc *.hh *.sh *.pl Makefile [A-Z]*[A-Z]

cd tmp
tar zxf tmp.tgz
rm tmp.tgz
cd ..
mv tmp CmdLine
date=`date +%Y%m%d`
outfile=releases/CmdLine-$date.tgz
echo Creating $outfile  which contains
tar zcvf $outfile  CmdLine
rm -rf CmdLine
