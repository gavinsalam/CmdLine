all: libCmdLine.a example file_example

#CXXFLAGS=-g -std=c++11 -stdlib=libc++ -pedantic -Wall -O3 -fPIC -DPIC
CXXFLAGS=-g -std=c++11  -pedantic -Wall -O3 -fPIC -DPIC

libCmdLine.a: CmdLine.o
	ar rc libCmdLine.a CmdLine.o
	ranlib libCmdLine.a


example: libCmdLine.a example.o
	$(CXX) -o example example.o -L. -lCmdLine

file_example: libCmdLine.a file_example.o
	$(CXX) -o file_example file_example.o -L. -lCmdLine

dist:
	tarit.sh

clean:
	rm -f *.o

CmdLine.o: CmdLine.cc CmdLine.hh
example.o: CmdLine.hh 
