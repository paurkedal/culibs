# Copyright (C) 2007  Petter Urkedal
#
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

dnl AC_C_THREAD_KEYWORD
dnl     -- check if compiler supports the __thread keyword to declare
dnl        thread-local storage.
dnl
dnl AC_DEFINE([HAVE_THREAD_KEYWORD])

AC_DEFUN([CUAC_C_THREAD_KEYWORD],
  [ AC_CACHE_CHECK([for __thread keyword], [cuac_have_thread_keyword],
      [ AC_COMPILE_IFELSE(
	  [ AC_LANG_SOURCE([[__thread int i; static __thread int j;]]) ],
	  [ cuac_have_thread_keyword=true ],
	  [ cuac_have_thread_keyword=false ])
      ])
    if test $cuac_have_thread_keyword = true; then
	AC_DEFINE([HAVE_THREAD_KEYWORD], [1],
		  [Defined if C compiler supports the __thread keyword.])
    fi
  ])
