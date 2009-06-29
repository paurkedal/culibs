/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2002--2007  Petter Urkedal <urkedal@nbi.dk>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cu/fwd.h>
#include <cu/memory.h>
#include <cu/debug.h>
#include <cu/idr.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>

/* Expensive debugging. */
#define CU_CSTR_DEBUG 1

#define CU_STRUNIQ_COLLECTABLE 1

#define HASH(key) (((key) ^ ((key) >> 7)) - (((key) >> 11) ^ ((key) >> 19)))

#ifdef CU_STRUNIQ_COLLECTABLE
#  define cu_bitnot_ptr(ptr) ((void*)~(uintptr_t)(ptr))
#else
#  define cu_bitnot_ptr(ptr) ((void*)(ptr))
#endif

cu_bool_t
cu_streq_bitnot(char const *s0, char const *s1)
{
    s0 = cu_bitnot_ptr(s0);
    s1 = cu_bitnot_ptr(s1);
    return strcmp(s0, s1) == 0;
}

cu_hash_t
cu_strhash_bitnot(char const *s)
{
    cu_hash_t hc = 0;
    s = cu_bitnot_ptr(s);
    while (s[0]) {
	if (s[1] == 0) {
	    hc += *s;
	    break;
	}
	hc += s[0] + (s[1]*sizeof(char)) + HASH(hc);
	s += 2;
    }
    return hc;
}

char const *
cu_struniq(char const *cstr)
{
    return cu_idr_to_cstr(cu_idr_by_cstr(cstr));
}

char const *
cu_struniq_int(int i)
{
    char str[sizeof(int)*3 + 2];
    sprintf(str, "$%d", i);
    return cu_struniq(str);
}

char *
cu_wcstombs(wchar_t const *wcs)
{
    size_t n = wcstombs(NULL, wcs, 0) + 1;
    char* str;
    if (n == (size_t)(-1))
	return 0;
    str = (char*)cu_galloc(sizeof(char)*n);
    wcstombs(str, wcs, n);
    return str;
}

wchar_t *
cu_mbstowcs(char const *str)
{
    size_t n = mbstowcs(NULL, str, 0) + 1;
    wchar_t* wcs;
    if (n == (size_t)(-1))
	return 0;
    wcs = (wchar_t*)cu_galloc(sizeof(wchar_t)*n);
    mbstowcs(wcs, str, n);
    return wcs;
}

#define PRIME0 10000019
#define PRIME1 10000079
#define PRIME2 10000103
#define PRIME3 10000121

cu_hash_t
cu_cstr_hash(char const *s)
{
    cu_hash_t hc = PRIME0;
    while (*s)
	hc = hc*PRIME2 + *s++;
    return hc;
}

cu_clop_edef(cu_cstr_hash_clop, cu_hash_t, char const *s)
{
    cu_hash_t hc = PRIME0;
    while (*s)
	hc = hc*PRIME2 + *s++;
    return hc;
}

cu_hash_t
cu_cstr_hash_x(char const *s, cu_hash_t p)
{
    cu_hash_t hc = PRIME1;
    while (*s)
	hc = hc*p + *s++;
    return hc;
}

cu_bool_t
cu_cstr_eq(char const *x, char const *y)
{
    return !strcmp(x, y);
}

cu_clop_edef(cu_cstr_eq_clop, cu_bool_t, char const *x, char const *y)
{
    return !strcmp(x, y);
}

char *
cu_cstr_new_copy(char const *s)
{
    char *sp = cu_galloc(strlen(s) + 1);
    strcpy(sp, s);
    return sp;
}

char *
cu_cstr_toupper(char *s)
{
    char *r = s;
    while (*s) {
	*s = toupper(*s);
	++s;
    }
    return r;
}

char *
cu_cstr_tolower(char *s)
{
    char *r = s;
    while (*s) {
	*s = tolower(*s);
	++s;
    }
    return r;
}
