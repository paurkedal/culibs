# Copyright (C) 2007  Petter Urkedal
#
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

dnl CUAC_LIB_UNWIND(MIN_VERSION, IF_FOUND, IF_NOT_FOUND)
dnl
AC_DEFUN([CUAC_LIB_UNWIND], [
    AC_MSG_CHECKING([for libunwind])
    UNWIND_LIBS=
    case $host in
	i386-*|i486-*|i586-*|i686-*)
	    UNWIND_LIBS="-lunwind -lunwind-x86"
	    ;;
	*)
	    AC_MSG_WARN([May need to update cuac_lib_unwind.m4 with platform-specific -lunwind-ARCH to use libunwind.])
	    UNWIND_LIBS="-lunwind -lunwind-generic"
	    ;;
    esac
    _cuac_saved_libs="$LIBS"
    LIBS="$UNWIND_LIBS $LIBS"
    AC_RUN_IFELSE(
      [ AC_LANG_PROGRAM(
	  [[
#include <libunwind.h>
	  ]], [[
int err;
unw_context_t context;
unw_cursor_t cursor;
err = unw_getcontext(&context);
if (err < 0) return 1;
err = unw_init_local(&cursor, &context);
if (err < 0) return 1;
	  ]])
      ], [
	AC_DEFINE([HAVE_LIBUNWIND], 1, [Defined if libunwind is available.])
	AC_MSG_RESULT([yes])
	$2
      ], [
	UNWIND_LIBS=
        AC_MSG_RESULT([no])
	$3
      ])
    LIBS="$_cuac_saved_libs"
    AC_SUBST(UNWIND_LIBS)
  ])
