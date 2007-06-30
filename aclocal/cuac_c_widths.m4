# Copyright (C) 2007  Petter Urkedal
#
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

AC_DEFUN([CUAC_C_WIDTHS],
  [ AC_MSG_NOTICE([Determining widths of basic types])
    AC_RUN_IFELSE(
      [
#include <stdio.h>
#include <wchar.h>
#include <stdint.h>
#include <stddef.h>

struct aligntest_s
{
    char c;
    union {
	long l;
	double x;
    } u;
};
int main()
{
    int i, j;
    FILE *out = fopen("config.sizeof", "w");
    if (!out)
        return 1;
    fprintf(out,
	    "SIZEOF_CHAR=%d\nWIDTHOF_CHAR=%d\n"
	    "SIZEOF_SHORT=%d\nWIDTHOF_SHORT=%d\n"
	    "SIZEOF_INT=%d\nWIDTHOF_INT=%d\n"
	    "SIZEOF_LONG=%d\nWIDTHOF_LONG=%d\n"
	    "SIZEOF_VOID_P=%d\nWIDTHOF_VOID_P=%d\n"
	    "SIZEOF_INTPTR_T=%d\nWIDTHOF_INTPTR_T=%d\n"
	    "SIZEOF_FNPTR_T=%d\nWIDTHOF_FNPTR_T=%d\n"
	    "SIZEOF_WCHAR_T=%d\nWIDTHOF_WCHAR_T=%d\n",
	    sizeof(char), sizeof(char)*8,
	    sizeof(short), sizeof(short)*8,
	    sizeof(int), sizeof(int)*8,
	    sizeof(long), sizeof(long)*8,
	    sizeof(void *), sizeof(void *)*8,
	    sizeof(intptr_t), sizeof(intptr_t)*8,
	    sizeof(void (*)()), sizeof(void (*)())*8,
	    sizeof(wchar_t), sizeof(wchar_t)*8);
    i = 0;
    j = offsetof(struct aligntest_s, u);
    fprintf(out, "MAXALIGN=%d\n", j);
    while (j /= 2) ++i;
    fprintf(out, "LOG2_MAXALIGN=%d\n", i);
    fclose(out);
    return 0;
}
      ], [
	source config.sizeof
	AC_MSG_NOTICE([$WIDTHOF_CHAR b char, $WIDTHOF_SHORT b short, $WIDTHOF_INT b int, $WIDTHOF_LONG b long, $WIDTHOF_INTPTR_T b intptr_t])
	AC_MSG_NOTICE([$WIDTHOF_VOID_P b pointers, $WIDTHOF_FNPTR_T b function pointers, $WIDTHOF_WCHAR_T b wchar_t])
	AC_DEFINE_UNQUOTED(SIZEOF_CHAR, $SIZEOF_CHAR, [sizeof(char)])
	AC_DEFINE_UNQUOTED(SIZEOF_SHORT, $SIZEOF_SHORT, [sizeof(short)])
	AC_DEFINE_UNQUOTED(SIZEOF_INT, $SIZEOF_INT, [sizeof(int)])
	AC_DEFINE_UNQUOTED(SIZEOF_LONG, $SIZEOF_LONG, [sizeof(long)])
	AC_DEFINE_UNQUOTED(SIZEOF_VOID_P, $SIZEOF_VOID_P, [sizeof(void *)])
	AC_DEFINE_UNQUOTED(SIZEOF_INTPTR_T, $SIZEOF_INTPTR_T, [sizeof(intptr_t)])
	AC_DEFINE_UNQUOTED(SIZEOF_FNPTR_T, $SIZEOF_FNPTR_T, [sizeof(void (*)())])
	AC_DEFINE_UNQUOTED(SIZEOF_WCHAR_T, $SIZEOF_WCHAR_T, [sizeof(wchar_t)])
	AC_DEFINE_UNQUOTED(WIDTHOF_CHAR, $WIDTHOF_CHAR, [8*sizeof(char)])
	AC_DEFINE_UNQUOTED(WIDTHOF_SHORT, $WIDTHOF_SHORT, [8*sizeof(short)])
	AC_DEFINE_UNQUOTED(WIDTHOF_INT, $WIDTHOF_INT, [8*sizeof(int)])
	AC_DEFINE_UNQUOTED(WIDTHOF_LONG, $WIDTHOF_LONG, [8*sizeof(long)])
	AC_DEFINE_UNQUOTED(WIDTHOF_VOID_P, $WIDTHOF_VOID_P, [8*sizeof(void *)])
	AC_DEFINE_UNQUOTED(WIDTHOF_INTPTR_T, $WIDTHOF_INTPTR_T, [8*sizeof(intptr_t)])
	AC_DEFINE_UNQUOTED(WIDTHOF_FNPTR_T, $WIDTHOF_FNPTR_T, [8*sizeof(void (*)())])
	AC_DEFINE_UNQUOTED(WIDTHOF_WCHAR_T, $WIDTHOF_WCHAR_T, [8*sizeof(wchar_t)])
	AC_DEFINE_UNQUOTED(LOG2_MAXALIGN, $LOG2_MAXALIGN, [log2 of the maximum alignment (for long and double)])
	AC_DEFINE_UNQUOTED(MAXALIGN, $MAXALIGN, [The maximum alignment (for long and double)])
      ], [
	AC_MSG_ERROR([Could not determine sizeof of basic types.])
      ])
  ])

