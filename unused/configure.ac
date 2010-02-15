#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
#
# OR, better still: autoreconf [--install] 
#
# NB: a rough tutorial is to be found at:
#     http://www-src.lip6.fr/homepages/Alexandre.Duret-Lutz/autotools.html

AC_PREREQ(2.61)
AC_INIT([CmdLine], [1.0], [salam@lpthe.jussieu.fr])
AM_INIT_AUTOMAKE([-Wall -Werror])

# not sure what the following does???
AC_CONFIG_SRCDIR([CmdLine.cc])

AC_CONFIG_HEADER([config.h])


# Checks for programs.
AC_PROG_CXX
AC_PROG_RANLIB
#AC_PROG_CC

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_STRFTIME
AC_CHECK_FUNCS([uname])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
