# Copyright (C) 2007  Petter Urkedal
#
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

dnl -*- autoconf -*- macro to locate libccf.

dnl CCF_WITH_ENVVAR(VAR, VALUE, BODY)
dnl ---------------------------------
dnl     Sets VAR to VALUE temporarily while running BODY.
AC_DEFUN([CUAC_WITH_ENVVAR],
  [ $1_save="${$1}"
    $1="$2"
    $3
    $1="${$1_save}"
  ])

dnl CUAC_WITH_SAVED_ENV([VAR0, VAR1, ...], [BODY])
dnl     Runs BODY and then restores the listed environment variables to
dnl     their value before entering of BODY.
AC_DEFUN([CUAC_WITH_SAVED_ENV],
  [ m4_foreach([_var], [$1], [__saved_[]_var="$_var";])
    $2
    m4_foreach([_var], [$1], [_var="$__saved_[]_var";])
  ])
