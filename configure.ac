#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
AC_INIT([ylib], [1.0], [yhcting77@gmail.com])
AC_CONFIG_SRCDIR([Makefile.sh])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([-Wall -Werror subdir-objects])

# Checks for programs.
AC_PROG_CC
AC_PROG_MAKE_SET

AC_GNU_SOURCE

AM_PROG_AR

# libtool is used for shared library
AC_PROG_LIBTOOL

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_C_BIGENDIAN

# Check size of primitive types
AC_CHECK_SIZEOF(char)
AC_CHECK_SIZEOF(short)
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(void*)
if test ${ac_cv_sizeof_char} != 1; then
    AC_MSG_ERROR([Size of char isn't 8 bit.])
fi
if test ${ac_cv_sizeof_short} != 2; then
    AC_MSG_ERROR([Size of short isn't 16 bit.])
fi
if test ${ac_cv_sizeof_int} != 4; then
    AC_MSG_ERROR([Size of int isn't 32 bit.])
fi
# some codes assumes below condition.
if test ${ac_cv_sizeof_long} != ${ac_cv_sizeof_voidp}; then
    AC_MSG_ERROR([Size of long isn't same with size of pointer.])
fi

# Checks for library functions.
AC_CHECK_FUNCS([memmove memset malloc memcpy],
    [],
    [AC_MSG_ERROR([sorry, these function is required. \
    		   But portability will be improved later!])])

AC_ARG_WITH(debug,
    [AS_HELP_STRING([--with-debug],
        [Build with debugging feature. This arguement should be enabled for testing.])],
    [AC_DEFINE([CONFIG_DEBUG],
        [1],
        [Debug is enabled.])],
    [with_debug=no])

AC_ARG_WITH(debug2,
    [AS_HELP_STRING([--with-debug2],
        [Debugging level 2 - more verbose debugging.])],
    [AC_DEFINE([CONFIG_DEBUG2],
        [1],
        [ More verbose debugging is enabled.])
     AC_DEFINE([CONFIG_DEBUG], [1])],
    [with_debug2=no])

AS_IF([test "x$with_debug" != xno -o "x$with_debug2" != xno ],
    [CFLAGS="-g -O0"],
    [CFLAGS="-O2"])
CFLAGS="${CFLAGS} -Wall -Werror"
AC_SUBST([CFLAGS])

AC_CONFIG_FILES([Makefile])

AC_OUTPUT

