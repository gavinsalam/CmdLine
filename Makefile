all: libCmdLine.a example unit-tests

#CXXFLAGS=-g -std=c++11 -stdlib=libc++ -pedantic -Wall -O3 -fPIC -DPIC
CXXFLAGS=-D__CMDLINE_ABI_DEMANGLE__ -g -std=c++14 -pedantic -Wall -Wextra -Wsign-compare -Wshadow -O3 -fPIC -DPIC

## to enable coverage tests, on linux, uncomment the following lines
##
# CXXFLAGS += --coverage
# LDFLAGS += --coverage
## 
## and then run
## 
##   make clean; make; make check
##   lcov --directory . --capture --output-file coverage.info
##   genhtml coverage.info --output-directory coverage
##
## 

libCmdLine.a: CmdLine.o
	ar rc libCmdLine.a CmdLine.o
	ranlib libCmdLine.a

example: libCmdLine.a example.o
	$(CXX) $(LDFLAGS) -o example example.o -L. -lCmdLine

unit-tests: libCmdLine.a unit-tests.o
	$(CXX) $(LDFLAGS) -o unit-tests unit-tests.o -L. -lCmdLine

check: unit-tests example
	./unit-tests
	./example -i 2 > /dev/null
	./example -h > /dev/null

dist:
	tarit.sh

clean:
	rm -f *.o

distclean: clean
	rm -f unit-tests example libCmdLine.a

CmdLine.o: CmdLine.cc CmdLine.hh
example.o: CmdLine.hh 
unit-tests.o: CmdLine.hh
