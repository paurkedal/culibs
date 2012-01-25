# Copyright (C) 2007  Petter Urkedal
#
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

dnl CUAC_LIB_BDWGC
dnl     Check for the Boehm-Demers-Weiser conservative garbage collector
dnl     and the "disclaim" patch.
dnl  => AC_SUBST([BDWGC_CFLAGS], ...)
dnl  => AC_SUBST([BDWGC_LIBS], ...)
dnl  => AC_DEFINE([HAVE_GC_DISCLAIM], ...)
dnl  => have_gc_disclaim=true|false
dnl  => ... see AC_CHECK_HEADERS, AC_CHECK_FUNCS below


dnl ---- old docs, consider internal ---
dnl _CUAC_LIB_BDWGC_NONPKGCONFIG(VERSION, OKCB, FAILCB)
dnl
dnl If the Boehm-Demers-Weiser conservative garbage collector is found,
dnl     sets and SUBSTitutes BDWGC_LIBS and BDWGC_CFLAGS,
dnl     defines HAVE_BDWGC, and
dnl	either HAVE_GC_GC_H or HAVE_GC_H depending on whether the installation
dnl     uses as a gc/ prefix on headers.
dnl     calls OKCB
dnl else
dnl     if FAILCB is defined
dnl         calls it
dnl     else
dnl         fails with a message indicating that the collector is required
dnl VERSION is ignored, at least for the moment, but it's still good for
dnl documentation.
dnl ---- end old docs ---

dnl Consider this internal.
AC_DEFUN([_CUAC_LIB_BDWGC_NONPKGCONFIG],
  [
    tmp_found=false
    tmp_defines=
    BDWGC_CFLAGS=
    BDWGC_LDFLAGS=
    AC_ARG_WITH([boehm-gc-prefix],
	AS_HELP_STRING([--with-boehm-gc-prefix], [where to find Boehm GC]),
	  [
	    BDWGC_CFLAGS="-I $withval/include"
	    BDWGC_LDFLAGS="-L $withval/lib"
	  ])
    AC_CHECK_HEADERS([gc/gc.h],
	[tmp_found=true; tmp_defines="-DHAVE_GC_GC_H=1"], [],
	[$BDWGC_CFLAGS])
    AC_CHECK_HEADERS([gc.h], [tmp_found=true], [], [$BDWGC_CFLAGS])
    AC_MSG_CHECKING([for usable garbage collector])
    if $tmp_found; then
	tmp_found=false
	tmp_saved_cppflags="$CPPFLAGS"
	tmp_saved_libs="$LIBS"
	CPPFLAGS="$BDWGC_CFLAGS $tmp_defines"
	for tmp_libs in "-lgc" \
			"-lgc -lpthread" \
			"-lgc -ldl" \
			"-lgcj" \
			"-lgcj -lpthread"
	do
	    LIBS="$tmp_saved_libs $BDWGC_LDFLAGS $tmp_libs"
	    AC_LINK_IFELSE(
[AC_LANG_SOURCE([[
#ifdef HAVE_GC_GC_H
#include <gc/gc.h>
#else
#include <gc.h>
#endif
int main()
{
    return !GC_malloc(1);
}
]])],
		[
		    tmp_found=true
		    BDWGC_LIBS="$BDWGC_LDFLAGS $tmp_libs"
		    break
		], [],
		[$tmp_libs])
	done
	CPPFLAGS="$tmp_saved_cppflags"
	LIBS="$tmp_saved_libs"
    fi
    if $tmp_found; then
        AC_MSG_RESULT([$BDWGC_LIBS])
	AC_DEFINE(HAVE_BDWGC, 1, [Boehm-Demers-Weiser conservative garbage collector present?])
	$2
    else
	AC_MSG_RESULT([no])
	ifelse([$3], ,
	    [AC_MSG_ERROR([
    This package requires Boehm-Demers-Weiser conservative garbage
    collector.  To carry on, you may search your system for "libgc.*",
    "gc.h" or "gc_typed.h" and set the appropriate CPPFLAGS and LDFLAGS.
    Otherwise, see http://www.hpl.hp.com/personal/Hans_Boehm/gc/.])],
	    [$3])
    fi
    AC_SUBST(BDWGC_CFLAGS)
    AC_SUBST(BDWGC_LIBS)
  ])

AC_DEFUN([CUAC_LIB_BDWGC],
  [ PKG_CHECK_MODULES(BDWGC, bdw-gc, [], [_CUAC_LIB_BDWGC_NONPKGCONFIG])
    CUAC_WITH_SAVED_ENV([CPPFLAGS, LIBS],
      [ CPPFLAGS="$CPPFLAGS $BDWGC_CFLAGS"
	LIBS="$LIBS $BDWGC_LIBS"
	AC_CHECK_FUNC([GC_pthread_create], [],
	    [AC_MSG_ERROR(
		[libgc must be configured with multi-thread support.])])
	AC_CHECK_FUNC([GC_register_disclaim_proc],
	  [ AC_DEFINE([HAVE_GC_DISCLAIM], 1,
		      [Define if libgc has disclaim patch.])
	    have_gc_disclaim=true
	    AC_MSG_RESULT(yes)
	    AC_MSG_CHECKING([GC_register_disclaim_proc API])
	    AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
#include <gc/gc_disclaim.h>
int main()
{
    GC_register_disclaim_proc(0, NULL, 0);
}
]])],
	    [AC_MSG_RESULT([new])],
	    [AC_MSG_RESULT([old])
	     AC_DEFINE([GC_REGISTER_DISCLAIM_PROC_OLD_API], [],
		       [Define if GC_register_disclaim_proc has old API.])] )
	  ],
	  [ have_gc_disclaim=false
	    AC_MSG_RESULT(no) ])
	AC_CHECK_HEADERS([gc/gc.h gc/gc_local_alloc.h gc_local_alloc.h gc/gc_tiny_fl.h gc/gc_rnotify.h gc_rnotify.h])
	AC_CHECK_FUNCS([GC_generic_malloc_many GC_local_malloc GC_local_malloc_atomic GC_malloc_atomic_uncollectable])
      ])
  ])
