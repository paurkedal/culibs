AC_INIT([culibs], [0.25], [urkedal@nbi.dk])
m4_pattern_forbid([^CUAC_])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([aclocal])
AC_CONFIG_SRCDIR([cu/clos.h])
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([nostdinc no-define dist-bzip2 subdir-objects foreign])
AM_CONFIG_HEADER([config_raw.h])
CUAC_PACKAGE_VERSION

AM_PROG_CC_C_O
AC_PROG_YACC
PKG_PROG_PKG_CONFIG

AC_ARG_ENABLE([cplusplus],
   [AC_HELP_STRING([--disable-cplusplus],
	[Disable test for C++ compatibility.])])
AC_PROG_CXX
AM_CONDITIONAL([enable_cplusplus], [test x"$enable_cplusplus" != xno])

AM_PROG_LIBTOOL

AC_ARG_ENABLE([experimental],
    [AC_HELP_STRING([--enable-experimental], [Include experimental features?])],
    [AC_DEFINE([ENABLE_EXPERIMENTAL], [1], [Include experimental features?])])
AM_CONDITIONAL([enable_experimental], [test "x$enable_experimental" = xyes])

AC_ARG_ENABLE([redundant],
    [AC_HELP_STRING([--enable-redundant],
	[Enable APIs which are considered redundant and subject to removal at]
	[any time.  If you find any of these useful, please contact the]
	[maintainer before using them in production code.])])
AM_CONDITIONAL([enable_redundant], [test x"$enable_redundant" = xyes])


# Decide what to Build
# ====================

# Modules
#
CUAC_MODULE([cu],	[],		[libcubase.la])
CUAC_MODULE([cuoo],	[],		[libcubase.la])
CUAC_MODULE([cucon],	[],		[libcubase.la])
CUAC_MODULE([cufo],	[cucon, cutext, cuos])
CUAC_MODULE([cuflow],	[cucon])
CUAC_MODULE([cugra],	[cucon])
CUAC_MODULE([cuos],	[cucon])
CUAC_MODULE([cutext],	[cucon])
CUAC_MODULE([custo],	[cucon])
CUAC_MODULE([cuex],	[cucon, cugra, cufo])
CUAC_MODULE_ALIAS([cubase], [cu, cucon, cuoo])
CUAC_MODULE_EQUIVALENCE([cu, cuoo, cucon])
CUAC_ARG_MODULES

# Documentation option.
#
AC_ARG_ENABLE([doc], [AC_HELP_STRING([--enable-doc], [Build documentation.])])
AM_CONDITIONAL([enable_doc], [test x$enable_doc = xyes])


# Compiler and Architecture Details
# =================================

CPPFLAGS="-D_GNU_SOURCE $CPPFLAGS"

AC_MSG_CHECKING([if the compiler supports static __inline__ functions])
AC_C_INLINE
AC_C_CHAR_UNSIGNED
AC_C_BIGENDIAN

AC_MSG_CHECKING([if compiler and library supports 64 bit integers])
AC_COMPILE_IFELSE(
  [AC_LANG_SOURCE([[
#include <stdint.h>
uint64_t test;
  ]])], [
    AC_MSG_RESULT([yes])
    AC_DEFINE([HAVE_UINT64_T], 1, [Define if stdint.h provides uint64_t.])
  ], [
    AC_MSG_RESULT([no])
  ])

CUAC_C_WIDTHS
CUAC_C_VA_LIST_IS_REF
CUAC_C_STACK_DIRECTION

CUAC_C_THREAD_KEYWORD

AC_ARG_ENABLE([risky-optimisations],
   [AC_HELP_STRING([--enable-risky-optimisations],
       [Enable optimisations which may cause incorrect code generation on some
	compilers.])],
   [AC_DEFINE([CUCONF_ENABLE_RISKY_OPTIMISATIONS], [1],
       [Define to enable optimisations which may cause incorrect code
	generation on some compilers.])])

# Check if it is valid to pass more arguments than the prototype in
# the definition specifies.
#
AH_TEMPLATE([REDUNDANT_ARG_IS_VALID],
    [Define to 1 if it is valid to call a function with more arguments than
     in the prototype of its definition.  Cf. REDUNDANT_ARG_FIRST.])
AH_TEMPLATE([REDUNDANT_ARG_FIRST],
    [Define to 1 if REDUNDANT_ARG_IS_VALID is defined and the redundant
     argument must be first in the argument list.])
AC_MSG_CHECKING([if redundant arguments are valid])
AC_TRY_RUN(
  [ int f(int x) { return x; }
    int main() { return ((int (*)(int, int))f)(1727139376, 0) != 1727139376; }
],
  [ AC_MSG_RESULT([yes, at end])
    AC_DEFINE(REDUNDANT_ARG_IS_VALID) ],
  [ AC_TRY_RUN(
      [ int f(int x) { return x; }
	int main() { return ((int (*)(int, int))f)(0, 1727139376) != 1727139376; }
],
      [	AC_MSG_RESULT([yes, at start])
	AC_DEFINE(REDUNDANT_ARG_IS_VALID)
	AC_DEFINE(REDUNDANT_ARG_FIRST) ],
      [	AC_MSG_RESULT([no]) ] )
  ])


# Libraries
# =========

# libatomic_ops
#
AC_CHECK_HEADER([atomic_ops.h], [],
  [ AC_MSG_ERROR([We'll need libatomic_ops from http://www.hpl.hp.com/research/linux/atomic_ops/index.php4]) ])

# libgc
#
CUAC_LIB_BDWGC

AC_ARG_ENABLE([hashcons],
    [AC_HELP_STRING([--enable-hashcons=disclaim|disclaim-old|disapp],
	[Select hash-consing implementation])],
    [AS_CASE([$enableval],
	[disclaim|disclaim-old],
	    [AS_IF([test $have_gc_disclaim != true],
		[AC_MSG_ERROR([To use --enable-hashcons=$enableval you must link against a libgc build with the disclaim-patch, available at http://www.eideticdew.org/culibs/bdwgc-rn.html])])],
	[disapp], [],
	[AC_MSG_ERROR([Invalid argument to --enable-hashcons.])])],
    [AS_IF([test $have_gc_disclaim], [enable_hashcons=disclaim],
	   [enable_hashcons=disapp])])
AS_CASE([$enable_hashcons],
    [disclaim],     [AC_DEFINE([CUCONF_ENABLE_HASHCONS_DISCLAIM], [1],
			       [Use the new disclaim-based hash-consing.])],
    [disclaim-old], [AC_DEFINE([CUCONF_ENABLE_HASHCONS_DISCLAIM_OLD], [1],
			       [Use the old disclaim-based hash-consing.])],
    [disapp],       [AC_DEFINE([CUCONF_ENABLE_HASHCONS_DISAPP], [1],
			       [Use disappearing links for hash-consing.])])
AS_IF([$enable_cuex && test $enable_hashcons = disapp],
      [AC_MSG_WARN([Hash-consing is slow without disclaim support. This is mainly an issue if you use expression trees from the cuex module.])])
AM_CONDITIONAL([enable_hashcons_disclaim],
	       [test $enable_hashcons = disclaim])
AM_CONDITIONAL([enable_hashcons_disclaim_old],
	       [test $enable_hashcons = disclaim-old])
AM_CONDITIONAL([enable_hashcons_disapp],
	       [test $enable_hashcons = disapp])

# other libraries
#
CUAC_LIB_RT
CUAC_LIB_UNWIND

# cufo-dependencies
#
if $enable_cufo; then
    MP_WITH_CURSES
    if test x"$CURSES_LIB" != x; then
	AC_DEFINE([CUCONF_ENABLE_TERM], 1,
		  [Define to enable cufo terminal target.])
    fi
    AC_SUBST([CURSES_LIB])
    AC_CHECK_HEADERS([langinfo.h])
    AC_CHECK_FUNCS([fileno isatty nl_langinfo])
fi
AM_CONDITIONAL([enable_cufo_term], [test x"$CURSES_LIB" != x])

# cuex-dependencies
#
if $enable_cuex; then
    CUAC_LIB_FFI([], [:], [:])
    CUAC_LIB_BUDDY([22])
fi
AM_CONDITIONAL([have_buddy], [$have_buddy])
AM_CONDITIONAL([have_libffi], [$have_libffi])


# Optional Features
# =================

AC_ARG_ENABLE([threads],
    [AC_HELP_STRING([--disable-threads], [Disable thread-safety.])])
if test x"$enable_threads" != xno; then
    AC_DEFINE([CUCONF_ENABLE_THREADS], 1, [Enable thread-safety.])
    PTHREAD_LIBS="-lpthread"
fi
AM_CONDITIONAL([enable_threads], [test x"$enable_threads" != xno])
AC_SUBST(PTHREAD_LIBS)

AC_ARG_ENABLE([debug],
    [AC_HELP_STRING([--disable-debug],
	[Do not include debug-checks for client calls.])])
if test x"$enable_debug" != xno; then
    AC_DEFINE([CUCONF_DEBUG_CLIENT], 1, [Enable debugging for client.])
fi

AC_ARG_ENABLE([debug-lib],
    [AC_HELP_STRING([--enable-debug-lib],
	[Enable debugging which is only relevant for the library itself.])])
if test x"$enable_debug_lib" = xyes; then
    AC_DEFINE([CUCONF_DEBUG_SELF], 1, [Debugging of libraries.])
fi

if test x"$enable_debug_lib" = xyes; then
    AC_MSG_CHECKING([if error-cheking mutices are supported by pthread library])
    CUAC_WITH_ENVVAR([LDFLAGS], [$LDFLAGS -lpthread],
      [ AC_RUN_IFELSE(
	  [ AC_LANG_PROGRAM(
	      [[
#define _GNU_SOURCE
#include <pthread.h>
]],
	      [[
pthread_mutex_t mutex_init = PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP;
pthread_mutexattr_t mutexattr;
pthread_mutexattr_init(&mutexattr);
pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_ERRORCHECK_NP);
return 0;
]])
	  ],
	  [ dnl The definition of these constants also implies enabling their
	    dnl use, so only define when building debugging version.
	    AC_DEFINE([CUCONF_ERRORCHECK_MUTEX_INITIALIZER],
		      [PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP],
		      [Error-checking mutex initializer.])
	    AC_DEFINE([CUCONF_MUTEX_ERRORCHECK],
		      [PTHREAD_MUTEX_ERRORCHECK_NP],
		      [Error-checking mutex type.])
	    dnl Don't put _GNU_SOURCE in conf.h, since it would be too invasive
	    dnl on client.
	    AC_MSG_RESULT([yes]) ],
	  [ AC_MSG_RESULT([no]) ])
      ])
fi

# Determine winding implementitaion
#
AC_ARG_ENABLE([wind-variant],
    AC_HELP_STRING(
	[--enable-wind-variant=setjmp|unw],
	[Which variant of the wind implementation to use.]),
    [	case $enableval in
	    setjmp)
		wind_variant=setjmp
		cuflow_wind_variant_macro=CUFLOW_WIND_VARIANT_SETJMP
		;;
	    unw)
		wind_variant=unw
		cuflow_wind_variant_macro=CUFLOW_WIND_VARIANT_UNW
		;;
	    *)
		AC_MSG_ERROR([Bad value for --enable-wind-variant.])
		;;
	esac
    ], [
	wind_variant=setjmp
	wind_variant_header=wind_setjmp.h
	wind_variant_source=wind_setjmp.c
	cuflow_wind_variant_macro=CUFLOW_WIND_VARIANT_SETJMP
    ])
AC_DEFINE_UNQUOTED([WIND_VARIANT], [$cuflow_wind_variant_macro],
		   [Wind variant.])
AM_CONDITIONAL([wind_variant_is_setjmp], [test $wind_variant = setjmp])
AM_CONDITIONAL([wind_variant_is_unw], [test $wind_variant = unw])
AC_SUBST(wind_variant)

AM_CFLAGS="-Wall"
AC_SUBST([AM_CFLAGS])

# libcutext
#
AC_ARG_ENABLE([wccat-method],
    [AC_HELP_STRING([--enable-wccat-method],
	[Choose how cutext_wchar_wccat is implemented: "switch" or "table".])],
    [], [enable_wccat_method="switch"])
AM_CONDITIONAL([enable_wccat_switch], [test $enable_wccat_method = switch])
if test $enable_wccat_method = switch; then
    AC_DEFINE([ENABLE_WCCAT_SWITCH], [1], [cutext_wchar_wccat as switch])
fi

# For developers
#
AC_ARG_VAR([TAGS_DIR], [Where to put ctags files.])
if test -n "$TAGS_DIR"; then
    tagsdir="$TAGS_DIR"
else
    tagsdir=.
fi
AC_SUBST([tagsdir])


# Produce the Output
# ==================

AC_CONFIG_FILES([
	Makefile
	doc/Doxyfile
	pkgconfig/cubase.pc pkgconfig/cubase-uninstalled.pc
	pkgconfig/cufo.pc   pkgconfig/cufo-uninstalled.pc
	pkgconfig/cugra.pc  pkgconfig/cugra-uninstalled.pc
	pkgconfig/cuflow.pc pkgconfig/cuflow-uninstalled.pc
	pkgconfig/cuos.pc   pkgconfig/cuos-uninstalled.pc
	pkgconfig/custo.pc  pkgconfig/custo-uninstalled.pc
	pkgconfig/cuex.pc   pkgconfig/cuex-uninstalled.pc
	pkgconfig/cutext.pc pkgconfig/cutext-uninstalled.pc
	culibs.spec cubase.spec ])
AC_CONFIG_COMMANDS([pkgconfig-links],
  [ ln -sf cubase-uninstalled.pc pkgconfig/cu-uninstalled.pc
    ln -sf cubase-uninstalled.pc pkgconfig/cucon-uninstalled.pc
    ln -sf cubase-uninstalled.pc pkgconfig/cuoo-uninstalled.pc
    ln -sf cudyn-uninstalled.pc pkgconfig/cuex-uninstalled.pc
  ])

CUAC_CONFIG_INSTALLDIRS([cu/conf_installdirs.c], [cuconfP_installdirs])

AC_CONFIG_COMMANDS([cu/conf_ac.h],
  [ test cu/conf_ac.h -nt config_raw.h || sed >cu/conf_ac.h <config_raw.h \
	-e 's/\(#define\|#undef\) */\1 CUCONF_/' \
	-e 's/CUCONF_\(cu_\|CUCONF_\)/\1/' ])

AC_OUTPUT
