#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
AC_INIT([ylib], [1.0], [yhcting77@gmail.com])
AC_CONFIG_SRCDIR([ylib.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([-Wall -Werror])

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
n	AC_MSG_ERROR([Size of short isn't 16 bit.])
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

AC_ARG_WITH(dynamic-mempool,
[  --with-dynamic-mempool
        Dynamic mempool(henceforth DMP) is used instead of static mempool(henceforth SMP)
        DMP is suitable for the case that,
          frequent memory alloc/free within certain arrange.
        DMP uses 'malloc' for each memory block.
        (cf. SMP uses 'malloc' for each memory block group.)
	So, at some moment, getting memory block may take long time.
        (expanding pool - 'malloc' for each expanded block.)
	But, DMP supports shrinking pool in short time.
	In summay,
	  SMP is "fast for allocating block - even in worst case (one malloc) -,
	    very slow for shrinking pool."
	  DMP is "slow for allocating block - expanding pool requires lots of malloc call -,
	    fast for shirinking pool.],
[AC_DEFINE([CONFIG_MEMPOOL_DYNAMIC],
	   [1],
	   [Dynamic mempool is used instead of static mempool.])],
[])

AC_ARG_WITH(debug,
[  --with-debug
        Build with debugging feature. This arguement should be enabled for testing.],
[AC_DEFINE([CONFIG_DEBUG],
	   [1],
	   [Debug is enabled.])],
[])

AC_ARG_WITH(debug2,
[  --with-debug2
        Debugging level 2 - more verbose debugging.],
[AC_DEFINE([CONFIG_DEBUG2],
	   [1],
	   [ More verbose debugging is enabled.])],
[])

AM_CFLAGS="-Wall -Werror"
AC_SUBST([AM_CFLAGS])

AC_CONFIG_FILES([Makefile])

AC_OUTPUT

