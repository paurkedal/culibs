# Copyright (C) 2007  Petter Urkedal
#
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

AC_DEFUN([CUAC_LIB_RT],
  [ AC_MSG_CHECKING([for librt time suppport])
    _cuac_saved_libs="$LIBS"
    LIBS="$LIBS -lrt"
    AC_RUN_IFELSE(
      [ AC_LANG_SOURCE([[
#include <time.h>
#include <stdio.h>
#include <inttypes.h>

int main()
{
    FILE *out;
    struct timespec t0, t1;
    if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t0) != 0)
	return 2;
    return 0;
}
]])
      ], [
	AC_MSG_RESULT([yes])
	AC_SUBST([LIBRT_LIBS], [-lrt])
	AC_DEFINE([HAVE_LIBRT], [1], [Defined if librt available.])
      ], [
	AC_MSG_RESULT([no])
      ])
    LIBS="$_cuac_saved_libs"
  ])

dnl vim: filetype=config
