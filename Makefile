# dummy Makefile which just invokes scons (a more modern build tool)

all: always
	scons

libCmdLine.a: always
	scons libCmdLine.a

example: always
	scons example

# dummy entry
always: