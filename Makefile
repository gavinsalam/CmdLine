all: libCmdLine.a example unit-tests

#CXXFLAGS=-g -std=c++11 -stdlib=libc++ -pedantic -Wall -O3 -fPIC -DPIC
CXXFLAGS=-D__CMDLINE_ABI_DEMANGLE__ -g -std=c++14 -pedantic -Wall -Wextra -Wsign-compare -Wshadow -O3 -fPIC -DPIC

libCmdLine.a: CmdLine.o
	ar rc libCmdLine.a CmdLine.o
	ranlib libCmdLine.a

example: libCmdLine.a example.o
	$(CXX) -o example example.o -L. -lCmdLine

unit-tests: libCmdLine.a unit-tests.o
	$(CXX) -o unit-tests unit-tests.o -L. -lCmdLine

check: unit-tests
	./unit-tests

dist:
	tarit.sh

clean:
	rm -f *.o

CmdLine.o: CmdLine.cc CmdLine.hh
example.o: CmdLine.hh 
unit-tests.o: CmdLine.hh
