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

static int
_ceil_log2(size_t size)
{
    int i = 0;
    while (size >>= 1)
	++i;
    return i;
}

static void
_emit_size(FILE *out, char const *name, size_t size)
{
    int l = _ceil_log2(size) + 3;
    fprintf(out,
	    "SIZEOF_%s=%d\n"
	    "WIDTHOF_%s=%d\n"
	    "P2WIDTHOF_%s=%d\n"
	    "LOG2_P2WIDTHOF_%s=%d\n",
	    name, size, name, size*8,
	    name, (1 << l), name, l);
}

int main()
{
    int i, j;
    FILE *out = fopen("config.sizeof", "w");
    if (!out)
        return 1;
    _emit_size(out, "CHAR", sizeof(char));
    _emit_size(out, "SHORT", sizeof(short));
    _emit_size(out, "INT", sizeof(int));
    _emit_size(out, "LONG", sizeof(long));
    _emit_size(out, "SIZE_T", sizeof(size_t));
    _emit_size(out, "VOID_P", sizeof(void *));
    _emit_size(out, "INTPTR_T", sizeof(intptr_t));
    _emit_size(out, "FNPTR_T", sizeof(void (*)()));
    _emit_size(out, "WCHAR_T", sizeof(wchar_t));
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
	AC_DEFINE_UNQUOTED(SIZEOF_SIZE_T, $SIZEOF_SIZE_T, [sizeof(size_t)])
	AC_DEFINE_UNQUOTED(SIZEOF_VOID_P, $SIZEOF_VOID_P, [sizeof(void *)])
	AC_DEFINE_UNQUOTED(SIZEOF_INTPTR_T, $SIZEOF_INTPTR_T, [sizeof(intptr_t)])
	AC_DEFINE_UNQUOTED(SIZEOF_FNPTR_T, $SIZEOF_FNPTR_T, [sizeof(void (*)())])
	AC_DEFINE_UNQUOTED(SIZEOF_WCHAR_T, $SIZEOF_WCHAR_T, [sizeof(wchar_t)])

	AC_DEFINE_UNQUOTED(WIDTHOF_CHAR, $WIDTHOF_CHAR, [8*sizeof(char)])
	AC_DEFINE_UNQUOTED(WIDTHOF_SHORT, $WIDTHOF_SHORT, [8*sizeof(short)])
	AC_DEFINE_UNQUOTED(WIDTHOF_INT, $WIDTHOF_INT, [8*sizeof(int)])
	AC_DEFINE_UNQUOTED(WIDTHOF_LONG, $WIDTHOF_LONG, [8*sizeof(long)])
	AC_DEFINE_UNQUOTED(WIDTHOF_SIZE_T, $WIDTHOF_SIZE_T, [8*sizeof(size_t)])
	AC_DEFINE_UNQUOTED(WIDTHOF_VOID_P, $WIDTHOF_VOID_P, [8*sizeof(void *)])
	AC_DEFINE_UNQUOTED(WIDTHOF_INTPTR_T, $WIDTHOF_INTPTR_T, [8*sizeof(intptr_t)])
	AC_DEFINE_UNQUOTED(WIDTHOF_FNPTR_T, $WIDTHOF_FNPTR_T, [8*sizeof(void (*)())])
	AC_DEFINE_UNQUOTED(WIDTHOF_WCHAR_T, $WIDTHOF_WCHAR_T, [8*sizeof(wchar_t)])

	AC_DEFINE_UNQUOTED(P2WIDTHOF_CHAR, $P2WIDTHOF_CHAR,
			   [8*sizeof(char) rounded down to a power of 2])
	AC_DEFINE_UNQUOTED(P2WIDTHOF_SHORT, $P2WIDTHOF_SHORT,
			   [8*sizeof(short) rounded down to a power of 2])
	AC_DEFINE_UNQUOTED(P2WIDTHOF_INT, $P2WIDTHOF_INT,
			   [8*sizeof(int) rounded down to a power of 2])
	AC_DEFINE_UNQUOTED(P2WIDTHOF_LONG, $P2WIDTHOF_LONG,
			   [8*sizeof(long) rounded down to a power of 2])
	AC_DEFINE_UNQUOTED(P2WIDTHOF_SIZE_T, $P2WIDTHOF_SIZE_T,
			   [8*sizeof(size_t) rounded down to a power of 2])
	AC_DEFINE_UNQUOTED(P2WIDTHOF_VOID_P, $P2WIDTHOF_VOID_P,
			   [8*sizeof(void *) rounded down to a power of 2])
	AC_DEFINE_UNQUOTED(P2WIDTHOF_INTPTR_T, $P2WIDTHOF_INTPTR_T,
			   [8*sizeof(intptr_t) rounded down to a power of 2])
	AC_DEFINE_UNQUOTED(P2WIDTHOF_FNPTR_T, $P2WIDTHOF_FNPTR_T,
			   [8*sizeof(void (*)()) rounded down to a power of 2])
	AC_DEFINE_UNQUOTED(P2WIDTHOF_WCHAR_T, $P2WIDTHOF_WCHAR_T,
			   [8*sizeof(wchar_t) rounded down to a power of 2])

	AC_DEFINE_UNQUOTED(LOG2_P2WIDTHOF_CHAR, $LOG2_P2WIDTHOF_CHAR,
			   [log2(P2WIDTHOF_CHAR)])
	AC_DEFINE_UNQUOTED(LOG2_P2WIDTHOF_SHORT, $LOG2_P2WIDTHOF_SHORT,
			   [log2(P2WIDTHOF_SHORT)])
	AC_DEFINE_UNQUOTED(LOG2_P2WIDTHOF_INT, $LOG2_P2WIDTHOF_INT,
			   [log2(P2WIDTHOF_INT)])
	AC_DEFINE_UNQUOTED(LOG2_P2WIDTHOF_LONG, $LOG2_P2WIDTHOF_LONG,
			   [log2(P2WIDTHOF_LONG)])
	AC_DEFINE_UNQUOTED(LOG2_P2WIDTHOF_SIZE_T, $LOG2_P2WIDTHOF_SIZE_T,
			   [log2(P2WIDTHOF_SIZE_T)])
	AC_DEFINE_UNQUOTED(LOG2_P2WIDTHOF_VOID_P, $LOG2_P2WIDTHOF_VOID_P,
			   [log2(P2WIDTHOF_VOID_P)])
	AC_DEFINE_UNQUOTED(LOG2_P2WIDTHOF_INTPTR_T, $LOG2_P2WIDTHOF_INTPTR_T,
			   [log2(P2WIDTHOF_INTPTR_T)])
	AC_DEFINE_UNQUOTED(LOG2_P2WIDTHOF_FNPTR_T, $LOG2_P2WIDTHOF_FNPTR_T,
			   [log2(P2WIDTHOF_FNPTR_T)])
	AC_DEFINE_UNQUOTED(LOG2_P2WIDTHOF_WCHAR_T, $LOG2_P2WIDTHOF_WCHAR_T,
			   [log2(P2WIDTHOF_WCHAR_T)])

	AC_DEFINE_UNQUOTED(LOG2_MAXALIGN, $LOG2_MAXALIGN, [log2 of the maximum alignment (for long and double)])
	AC_DEFINE_UNQUOTED(MAXALIGN, $MAXALIGN, [The maximum alignment (for long and double)])
      ], [
	AC_MSG_ERROR([Could not determine sizeof of basic types.])
      ])
  ])

