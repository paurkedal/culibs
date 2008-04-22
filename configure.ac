AC_INIT(culibs, 0.22, [urkedal@nbi.dk])
m4_pattern_forbid([^CUAC_])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([aclocal])
AC_CONFIG_SRCDIR([cu/clos.h])
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([nostdinc no-define dist-bzip2 subdir-objects foreign])
AM_CONFIG_HEADER([config_raw.h])

AM_PROG_CC_C_O
AC_PROG_CXX
AM_PROG_LIBTOOL
AC_PROG_YACC
PKG_PROG_PKG_CONFIG

AC_ARG_ENABLE([experimental],
  [ AC_HELP_STRING([--enable-experimental],
	[Include experimental features?]) ],
  [ AC_DEFINE([ENABLE_EXPERIMENTAL], [1], [Include experimental features?]) ])
AM_CONDITIONAL([enable_experimental], [test "x$enable_experimental" = xyes])


# Decide what to Build
# ====================

# Modules
#
CUAC_MODULE([cu],	[],		[libcubase.la])
CUAC_MODULE([cuoo],	[],		[libcubase.la])
CUAC_MODULE([cucon],	[],		[libcubase.la])
CUAC_MODULE([cufo],	[cucon, cutext])
CUAC_MODULE([cuflow],	[cucon])
CUAC_MODULE([cugra],	[cucon])
CUAC_MODULE([cuos],	[cucon])
CUAC_MODULE([cutext],	[cucon])
CUAC_MODULE([custo],	[cucon])
CUAC_MODULE([cuex],	[cucon, cugra])
CUAC_MODULE_ALIAS([cubase], [cu, cucon, cuoo])
CUAC_MODULE_EQUIVALENCE([cu, cuoo, cucon])
CUAC_ARG_MODULES

# Documentation option.
#
AC_ARG_ENABLE([doc],
  [ AC_HELP_STRING([--enable-doc], [Build documentation.]) ])
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
  [
#include <stdint.h>
uint64_t test;
  ], [
    AC_MSG_RESULT([yes])
    AC_DEFINE(HAVE_UINT64_T, 1, [Define if stdint.h provides uint64_t.])
  ], [
    AC_MSG_RESULT([no])
  ])

CUAC_C_WIDTHS
CUAC_C_VA_LIST_IS_REF
CUAC_C_STACK_DIRECTION

dnl CUAC_C_THREAD_KEYWORD
dnl 2007-05-04 __thread storage breaks things.  It's probably not added to the
dnl GC roots, and linking it to a global variable as done in tstate.h does not
dnl help since it's not dynamic memory.

# Check if it is valid to pass more arguments than the prototype in
# the definition specifies.
#
AH_TEMPLATE(REDUNDANT_ARG_IS_VALID,
    [Define to 1 if it is valid to call a function with more arguments than
     in the prototype of its definition.  Cf. REDUNDANT_ARG_FIRST.])
AH_TEMPLATE(REDUNDANT_ARG_FIRST,
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
AC_ARG_ENABLE([gc-disclaim],
  [ AC_HELP_STRING([--disable-gc-disclaim],
	[Disable use of GC disclaim patch, even if present.]) ])
if test x"$enable_gc_disclaim" != xno; then
    if $have_gc_disclaim; then
	AC_DEFINE([CUCONF_ENABLE_GC_DISCLAIM], 1,
		  [Define to enable use of disclaim GC patch.])
    else
	enable_gc_disclaim=no
	AC_MSG_WARN([Hash-consing will we very slow without the disclaim])
	AC_MSG_WARN([patch for the garbage collector.  The patch is available at])
	AC_MSG_WARN([http://www.eideticdew.org/culibs/bdwgc-rn.html.])
    fi
fi
AM_CONDITIONAL([enable_gc_disclaim], [test x"$enable_gc_disclaim" != xno])

# other libraries
#
CUAC_LIB_RT
CUAC_LIB_UNWIND

# cuex-dependencies
#
if $enable_cuex; then
    CUAC_LIB_FFI
    CUAC_LIB_BUDDY([22])
fi
AM_CONDITIONAL([have_buddy], [$have_buddy])


# Optional Features
# =================

AC_ARG_ENABLE([threads],
  [ AC_HELP_STRING([--disable-threads], [Disable thread-safety.]) ])
if test x"$enable_threads" != xno; then
    AC_DEFINE([CUCONF_ENABLE_THREADS], 1, [Enable thread-safety.])
    PTHREAD_LIBS="-lpthread"
fi
AM_CONDITIONAL([enable_threads], [test x"$enable_threads" != xno])
AC_SUBST(PTHREAD_LIBS)

AC_ARG_ENABLE([debug],
  [ AC_HELP_STRING([--disable-debug],
	[Do not include debug-checks for client calls.]) ])
if test x"$enable_debug" != xno; then
    AC_DEFINE([CUCONF_DEBUG_CLIENT], 1, [Enable debugging for client.])
fi

AC_ARG_ENABLE([debug-lib],
  [ AC_HELP_STRING([--enable-debug-lib],
	[Enable debugging which is only relevant for the library itself.]) ])
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

AC_ARG_ENABLE([debug-memory],
  [ AC_HELP_STRING([--enable-debug-memory],
	[Enable metadata on memory blocks, used for debugging.]) ])
if test x"$enable_debug_metadata" = xyes; then
    AC_DEFINE([CUCONF_DEBUG_MEMORY], 1, [Define to enable memory metadata.])
fi

# Determine winding implementitaion
#
AC_ARG_ENABLE(wind-variant,
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
AC_DEFINE_UNQUOTED(WIND_VARIANT, $cuflow_wind_variant_macro, [Wind variant.])
AM_CONDITIONAL([wind_variant_is_setjmp], [test $wind_variant = setjmp])
AM_CONDITIONAL([wind_variant_is_unw], [test $wind_variant = unw])
AC_SUBST(wind_variant)

AM_CFLAGS="-Wall"
AC_SUBST(AM_CFLAGS)

# libcutext
#
AC_ARG_ENABLE([wccat-method],
  [ AC_HELP_STRING([--enable-wccat-method],
	[Choose how cutext_wchar_wccat is implemented: "switch" or "table".]) ],
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
	Doxyfile
	pkgconfig/cubase.pc pkgconfig/cubase-uninstalled.pc
	pkgconfig/cufo.pc   pkgconfig/cufo-uninstalled.pc
	pkgconfig/cugra.pc  pkgconfig/cugra-uninstalled.pc
	pkgconfig/cuflow.pc pkgconfig/cuflow-uninstalled.pc
	pkgconfig/cuos.pc   pkgconfig/cuos-uninstalled.pc
	pkgconfig/custo.pc  pkgconfig/custo-uninstalled.pc
	pkgconfig/cuex.pc   pkgconfig/cuex-uninstalled.pc
	pkgconfig/cutext.pc pkgconfig/cutext-uninstalled.pc
	culibs.spec cubase.spec ])

AC_CONFIG_COMMANDS([cu/conf_ac.h],
  [ sed >cu/conf_ac.h <config_raw.h \
	-e 's/\(#define\|#undef\) */\1 CUCONF_/' \
	-e 's/CUCONF_\(cu_\|CUCONF_\)/\1/' ])

AC_OUTPUT
