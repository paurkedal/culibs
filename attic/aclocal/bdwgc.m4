## -*- autoconf -*-
##
## MORE_LIB_BDWGC()
##
## Check for the Boehm-Demers-Weiser conservative garbage collector
## and provide
##
##	AC_SUBST(BDWGC_LIBS)
##	AC_SUBST(BDWGC_CPPFLAGS)
##	AM_CONDITIONAL(HAVE_BDWGC, ...)
##	AC_DEFINE(HAVE_BDWGC)
##	AC_DEFINE(BDWGC_INCLUDEDIR, ...)

AC_DEFUN([AM_LIB_BDWGC],
  [ BDWGC_INCLUDEDIR=""
    BDWGC_CPPFLAGS=""
    BDWGC_LIBS=""

    # libgc.la may or may not be installed and the include files are
    # not installed.  In addition libgc.la may or may not need -ldl,
    # and we do not have libtool here.  Thus the complication.
    AC_ARG_WITH(bdwgc,
	AC_HELP_STRING([--with-bdwgc=DIR],
		       [Path to the Boehm-Demers-Weiser garbage collector.]),
      [	if test -f $withval/include/gc.h; then
	    more_bdwgc_ldflags="-L$withval"
	    BDWGC_CPPFLAGS="-I$withval/include"
	    BDWGC_INCLUDEDIR="$withval/include"
	else
	    AC_MSG_RESULT(error)
	    AC_MSG_ERROR([Invalid path to the BDWGC.])
        fi
      ])

    AC_MSG_CHECKING(Boehm-Demers-Weiser GC)
    more_save_libs="$LIBS"
    more_save_cppflags="$CPPFLAGS"
    CPPFLAGS="$BDWGC_CPPFLAGS $CPPFLAGS"
    for libs in "-lgc" \
		"-lgc -ldl" \
		"$more_bdwgc_ldflags -lgc" \
		"$more_bdwgc_ldflags -lgc -ldl" \
		"-lgcj" \
		"-lgcj -lpthread"
    do
	LIBS="$libs $more_save_libs"
	AC_TRY_LINK([#include <gc.h>], [GC_malloc(1);],
		    [have_bdwgc=yes], [have_bdwgc=no])
	if test x$have_bdwgc = xyes; then
	    BDWGC_LIBS="$libs"
	    break
	fi
    done
    LIBS="$more_save_libs"
    CPPFLAGS="$more_save_cppflags"

    if test $have_bdwgc = yes; then
	AC_MSG_RESULT(yes)
	if test x$BDWGC_INCLUDEDIR != x; then
	    AC_DEFINE_UNQUOTED(BDWGC_INCLUDEDIR, $BDWGC_INCLUDEDIR)
	fi
	AC_DEFINE(HAVE_BDWGC)
    else
	AC_MSG_RESULT(no)
    fi

    AM_CONDITIONAL(HAVE_BDWGC, test $have_bdwgc = yes)
    AC_SUBST(BDWGC_LIBS)
    AC_SUBST(BDWGC_CPPFLAGS)
    AH_TEMPLATE(HAVE_BDWGC,
		[Define if the Boehm-Demers-Weiser collector is available.])
    AH_TEMPLATE(BDWGC_INCLUDEDIR,
		[The path where to find the headers of the BDW collector.])
  ])
