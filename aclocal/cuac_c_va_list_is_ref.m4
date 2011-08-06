# Copyright (C) 2007  Petter Urkedal
#
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

AC_DEFUN([CUAC_C_VA_LIST_IS_REF],
  [
    AC_MSG_CHECKING([if va_list has reference semantics])
    AC_RUN_IFELSE(
      [AC_LANG_SOURCE([[
#include <stdarg.h>
int f(va_list va)
{
    return va_arg(va, int);
}
int g(int i, ...)
{
    va_list va;
    va_start(va, i);
    if (f(va) == 11 && f(va) == 22)
	exit(0);
    else
	exit(1);
}
int main()
{
    g(0, 11, 22);
}
      ]])], [
	AC_MSG_RESULT([yes])
	AC_DEFINE([VA_LIST_IS_REF], 1,
	    [Define if va_list has reference semantics])
      ], [
	AC_RUN_IFELSE(
	  [AC_LANG_SOURCE([[
#include <stdarg.h>
typedef va_list *va_list_ref_t;
int f(va_list_ref_t va)
{
    return va_arg(*va, int);
}
int g(int i, ...)
{
    va_list va;
    va_start(va, i);
    if (f(&va) == 11 && f(&va) == 22)
	exit(0);
    else
	exit(1);
}
int main()
{
    g(0, 11, 22);
}
	  ]])], [
	    AC_MSG_RESULT([no])
	  ], [
	    AC_MSG_RESULT([error])
	    AC_MSG_ERROR([Could not construct a reference version of va_list])
	  ])
      ])
 ])

dnl vim: ft=config
