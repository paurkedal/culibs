# Copyright (C) 2007  Petter Urkedal
#
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.


dnl CUAC_LIB_BUDDY(MIN_VERSION, ACTION_IF_FOUND, ACTION_IF_NOT_FOUND)
dnl
AC_DEFUN([CUAC_LIB_BUDDY],
  [ AC_MSG_CHECKING([for Buddy binary decisions diagrams])
    CUAC_WITH_ENVVAR([LIBS], [-lbdd],
      [ CUAC_WITH_ENVVAR([CPPFLAGS], [$CPPFLAGS $BUDDY_CPPFLAGS],
	  [ AC_RUN_IFELSE(
	      [ AC_LANG_SOURCE([[
#include <bdd.h>
#include <fdd.h>
#include <bvec.h>

int
main()
{
    bdd_init(2, 2);
    if (bdd_versionnum() < $1) {
	fprintf(stderr, "Buddy is version %d, need version $1.\n",
		bdd_versionnum());
	return 1;
    }
    return 0;
}
]])
	      ], [
		have_buddy=true
		AC_DEFINE([HAVE_BUDDY], 1,
			  [Define if Buddy library is present.])
		AC_MSG_RESULT([yes])
		BUDDY_LIBS="-lbdd"
		ifelse($2, [], :, $3)
	      ], [
		have_buddy=false
		AC_MSG_RESULT([no])
		ifelse($3, [], :, $3)
	      ])
	  ])
      ])
dnl AM_CONDITIONAL([have_buddy], [$have_buddy])
    AC_SUBST([BUDDY_CPPFLAGS])
    AC_SUBST([BUDDY_LIBS])
  ])

