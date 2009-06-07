# Copyright (C) 2007  Petter Urkedal
#
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

dnl CUAC_LIB_FFI(MIN_VERSION, ACTION_IF_FOUND, ACTION_IF_NOT_FOUND)
dnl ==> AC_SUBST([FFI_LIBS])
dnl ==> AC_DEFINE([HAVE_LIBFFI])
dnl
dnl Note: Version check is not implemented, pass an empty value.
dnl
AC_DEFUN([CUAC_LIB_FFI],
  [ AC_ARG_WITH([libffi],
	[AS_HELP_STRING([--without-libffi],
			[Don't link with libffi even if present.])],
	[], [with_libffi=check])
    have_libffi=false
    AS_IF([test x$with_libffi != xno],
      [ FFI_LIBS=
	have_ffi_h=false
	AC_CHECK_HEADERS([libffi/ffi.h ffi.h], [have_ffi_h=true])
	if $have_ffi_h; then
	    tmp_ldflags=
	    if test -d /usr/lib/libffi; then
		tmp_ldflags="-L/usr/lib/libffi"
	    fi
	    CUAC_WITH_ENVVAR([LDFLAGS], [$LDFLAGS $tmp_ldflags],
	      [ AC_CHECK_LIB([ffi], [ffi_prep_cif],
		  [ FFI_LIBS="$tmp_ldflags -lffi"
		    have_libffi=true ])
	      ])
	fi
	AC_MSG_CHECKING([for usable libffi])
	if $have_libffi; then
	    AC_DEFINE([HAVE_LIBFFI], 1, [Defined if libffi is available.])
	    AC_MSG_RESULT([yes])
	    ifelse($2, [], :, $2)
	else
	    AC_MSG_RESULT([no])
	    ifelse($3, [], [AC_MSG_ERROR([Can not find required libffi])], $3)
	fi
	AC_SUBST(FFI_LIBS)
      ])
  ])
