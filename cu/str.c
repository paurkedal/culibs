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

#define _GNU_SOURCE 1

#include <cu/str.h>
#include <cu/memory.h>
#include <cu/debug.h>
#include <cu/thread.h>
#include <cu/diag.h>
#include <cu/idr.h>
#include <cu/init.h>

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#define SHARING(x) ((x)->cap = 0)

/* MT.  It is assumed that other threads do not modifying the string
 * when it is used.  Some of the non-mutating algorithms may still
 * reuse the capacity of a string, thus mutating it.  This is done
 * with cu_str_grab_cap, which is the only function which may modify
 * a shared string.  If the grab is successful the available storage
 * may be used.  Thus a function which is non-mutating for a string
 * argument, can only change that argument by setting cap to 0.
 *
 * It is questionable if this is feasible unless there is an atomic
 * fetch-and-clear operation for the processor. */
#if 1
size_t
cu_str_grab_cap(cu_str_t str)
{
    static cu_mutex_t mutex = CU_MUTEX_INITIALISER;
    size_t cap;
    cu_mutex_lock(&mutex);
    cap = str->cap;
    str->cap = 0;
    cu_mutex_unlock(&mutex);
    return cap;
}
#else
#define cu_str_grab_cap(str) 0
#endif

void
cu_str_set_capacity(cu_str_t str, size_t capacity)
{
    char *old_arr = str->arr;
    str->cap = capacity;
    str->arr = cu_galloc(sizeof(char)*capacity);
    memcpy(str->arr, old_arr, str->len);
}

cu_str_t
cu_str_new()
{
    cu_str_t str = cu_gnew(struct cu_str_s);
    str->arr = 0;
    str->len = 0;
    str->cap = 0;
    return str;
}
cu_str_t
cu_str_onew()
{
    cu_str_t str = cudyn_onew(cu_str);
    str->arr = 0;
    str->len = 0;
    str->cap = 0;
    return str;
}

void
cu_str_cct_charr(cu_str_t str, char const *ptr, size_t n)
{
    str->arr = cu_galloc(n + 1);
    str->len = n;
    str->cap = n + 1;
    memcpy(str->arr, ptr, n);
    str->arr[n] = 0;
}
cu_str_t
cu_str_new_charr(char const *ptr, size_t n)
{
    cu_str_t str = cu_gnew(struct cu_str_s);
    str->arr = cu_galloc(n + 1);
    str->len = n;
    str->cap = n + 1;
    memcpy(str->arr, ptr, n);
    str->arr[n] = 0;
    return str;
}
cu_str_t
cu_str_onew_charr(char const *ptr, size_t n)
{
    cu_str_t str = cudyn_onew(cu_str);
    str->arr = cu_galloc(n + 1);
    str->len = n;
    str->cap = n + 1;
    memcpy(str->arr, ptr, n);
    str->arr[n] = 0;
    return str;
}

cu_str_t
cu_str_new_cstr(char const *ptr)
{
    return cu_str_new_charr(ptr, strlen(ptr));
}

cu_str_t
cu_str_onew_cstr(char const *ptr)
{
    return cu_str_onew_charr(ptr, strlen(ptr));
}

void
cu_str_cct_cstr_static(cu_str_t str, char const *cstr)
{
    str->arr = (char*)cstr;
    str->len = strlen(cstr);
    str->cap = 0;
}

cu_str_t
cu_str_new_cstr_static(char const *cstr)
{
    cu_str_t str = cu_gnew(struct cu_str_s);
    str->arr = (char *)cstr;
    str->len = strlen(cstr);
    str->cap = 0;
    return str;
}
cu_str_t
cu_str_onew_cstr_static(char const *cstr)
{
    cu_str_t str = cudyn_onew(cu_str);
    str->arr = (char *)cstr;
    str->len = strlen(cstr);
    str->cap = 0;
    return str;
}

void
cu_str_cct_uninit(cu_str_t str, size_t n)
{
    str->arr = cu_galloc(n + 1);
    str->len = n;
    str->cap = n + 1;
}
cu_str_t
cu_str_new_uninit(size_t n)
{
    cu_str_t str = cu_gnew(struct cu_str_s);
    str->arr = cu_galloc(n + 1);
    str->len = n;
    str->cap = n + 1;
    return str;
}
cu_str_t
cu_str_onew_uninit(size_t n)
{
    cu_str_t str = cudyn_onew(cu_str);
    str->arr = cu_galloc(n + 1);
    str->len = n;
    str->cap = n + 1;
    return str;
}

#if 0
cu_str_t
cu_str_new_copy(cu_str_t str0)
{
    /* XX MT */
    cu_str_t str = cu_gnew(struct cu_str_s);
    size_t n = str0->len;
    str->arr = cu_galloc(n + 1);
    str->len = n;
    str->cap = n + 1;
    memcpy(str->arr, str0->arr, n);
    str->arr[n] = 0;
    return str;
}
#else

cu_str_t
cu_str_new_copy(cu_str_t str0)
{
    cu_str_t str = cu_gnew(struct cu_str_s);
    str0->cap = 0;
    str->arr = str0->arr;
    str->len = str0->len;
    str->cap = 0;
    return str;
}
cu_str_t
cu_str_onew_copy(cu_str_t str0)
{
    cu_str_t str = cudyn_onew(cu_str);
    str0->cap = 0;
    str->arr = str0->arr;
    str->len = str0->len;
    str->cap = 0;
    return str;
}

#endif

void
cu_str_cct(cu_str_t str)
{
    str->arr = 0;
    str->len = 0;
    str->cap = 0;
}

void
cu_str_assign(cu_str_t dest, cu_str_t src)
{
    dest->arr = src->arr;
    dest->len = src->len;
    dest->cap = 0;
    SHARING(src);
}

void
cu_str_append_str(cu_str_t x, cu_str_t y)
{
    size_t newsz = x->len + y->len;
    if (newsz > x->cap) {
	char *old;
	if (newsz < x->cap*2)
	    x->cap *= 2;
	else
	    x->cap = newsz;
	old = x->arr;
	x->arr = cu_galloc(sizeof(char)*x->cap);
	memcpy(x->arr, old, x->len);
    }
    memcpy(x->arr + x->len, y->arr, y->len);
    x->len = newsz;
}

void
cu_str_cct_2str(cu_str_t str, cu_str_t x, cu_str_t y)
{
    size_t newsz = x->len + y->len;
    if (newsz > x->cap) {
	str->arr = cu_galloc(sizeof(char)*(newsz + 1));
	str->len = newsz;
	str->cap = newsz + 1;
	memcpy(str->arr, x->arr, x->len);
    }
    else {
	/* XX MT */
	str->arr = x->arr;
	str->len = newsz;
	str->cap = x->cap;
	x->cap = 0;
    }
    memcpy(str->arr + x->len, y->arr, y->len);
}
cu_str_t
cu_str_new_2str(cu_str_t x, cu_str_t y)
{
    cu_str_t str = cu_gnew(struct cu_str_s);
    cu_str_cct_2str(str, x, y);
    return str;
}
cu_str_t
cu_str_onew_2str(cu_str_t x, cu_str_t y)
{
    cu_str_t str = cudyn_onew(cu_str);
    cu_str_cct_2str(str, x, y);
    return str;
}

void
cu_str_cct_2charr(cu_str_t str,
		     char const *s0, size_t n0, char const *s1, size_t n1)
{
    size_t n = n0 + n1 + 1;
    str->arr = cu_galloc(n);
    str->cap = n;
    str->len = --n;
    memcpy(str->arr, s0, n0);
    memcpy(str->arr + n0, s1, n1);
}
cu_str_t
cu_str_new_2charr(char const *s0, size_t n0, char const *s1, size_t n1)
{
    cu_str_t str = cu_gnew(struct cu_str_s);
    cu_str_cct_2charr(str, s0, n0, s1, n1);
    return str;
}
cu_str_t
cu_str_onew_2charr(char const *s0, size_t n0, char const *s1, size_t n1)
{
    cu_str_t str = cudyn_onew(cu_str);
    cu_str_cct_2charr(str, s0, n0, s1, n1);
    return str;
}

void
cu_str_cct_3charr(cu_str_t str,
		     char const *s0, size_t n0, char const *s1, size_t n1,
		     char const *s2, size_t n2)
{
    size_t n = n0 + n1 + n2 + 1;
    str->arr = cu_galloc(n);
    str->cap = n;
    str->len = --n;
    memcpy(str->arr, s0, n0);
    memcpy(str->arr + n0, s1, n1);
    memcpy(str->arr + n0 + n1, s2, n2);
}
cu_str_t
cu_str_new_3charr(char const *s0, size_t n0, char const *s1, size_t n1,
		     char const *s2, size_t n2)
{
    cu_str_t str = cu_gnew(struct cu_str_s);
    cu_str_cct_3charr(str, s0, n0, s1, n1, s2, n2);
    return str;
}
cu_str_t
cu_str_onew_3charr(char const *s0, size_t n0, char const *s1, size_t n1,
		      char const *s2, size_t n2)
{
    cu_str_t str = cudyn_onew(cu_str);
    cu_str_cct_3charr(str, s0, n0, s1, n1, s2, n2);
    return str;
}

void
cu_str_cct_str_cstr(cu_str_t str, cu_str_t x, char const *y)
{
    size_t n_y = strlen(y);
    size_t n_str = x->len + n_y;
    str->len = n_str;
    if (n_str > x->cap || n_str > (str->cap = cu_str_grab_cap(x))) {
	str->arr = cu_galloc(sizeof(char)*(n_str + 1));
	str->cap = n_str + 1;
	memcpy(str->arr, x->arr, x->len);
    }
    else
	str->arr = x->arr;
    memcpy(str->arr + x->len, y, n_y);
}
cu_str_t
cu_str_new_str_cstr(cu_str_t x, char const *y)
{
    cu_str_t str = cu_gnew(struct cu_str_s);
    cu_str_cct_str_cstr(str, x, y);
    return str;
}
cu_str_t
cu_str_onew_str_cstr(cu_str_t x, char const *y)
{
    cu_str_t str = cudyn_onew(cu_str);
    cu_str_cct_str_cstr(str, x, y);
    return str;
}

void
cu_str_cct_cstr_str(cu_str_t str, char const *x, cu_str_t y)
{
    size_t n_x = strlen(x);
    size_t n_new = n_x + y->len;
    str->arr = cu_galloc(sizeof(char)*(n_new + 1));
    str->len = n_new;
    str->cap = n_new + 1;
    memcpy(str->arr, x, n_x);
    memcpy(str->arr + n_x, y->arr, y->len);
}
cu_str_t
cu_str_new_cstr_str(char const *x, cu_str_t y)
{
    cu_str_t str = cu_gnew(struct cu_str_s);
    cu_str_cct_cstr_str(str, x, y);
    return str;
}
cu_str_t
cu_str_onew_cstr_str(char const *x, cu_str_t y)
{
    cu_str_t str = cudyn_onew(cu_str);
    cu_str_cct_cstr_str(str, x, y);
    return str;
}

cu_str_t
cu_str_new_2cstr(char const *x, char const *y)
{
    return cu_str_new_2charr(x, strlen(x), y, strlen(y));
}
cu_str_t
cu_str_onew_2cstr(char const *x, char const *y)
{
    return cu_str_onew_2charr(x, strlen(x), y, strlen(y));
}

void
cu_str_cct_str_char(cu_str_t str, cu_str_t x, char c)
{
    size_t n = x->len + 1;
    str->len = n;
    if (n > x->cap || n > (str->cap = cu_str_grab_cap(x))) {
	str->arr = cu_galloc(sizeof(char)*n);
	str->cap = n;
	memcpy(str->arr, x->arr, n - 1);
    }
    else
	str->arr = x->arr;
    str->arr[--n] = c;
}
cu_str_t
cu_str_new_str_char(cu_str_t x, char c)
{
    cu_str_t str = cu_gnew(struct cu_str_s);
    cu_str_cct_str_char(str, x, c);
    return str;
}
cu_str_t
cu_str_onew_str_char(cu_str_t x, char c)
{
    cu_str_t str = cudyn_onew(cu_str);
    cu_str_cct_str_char(str, x, c);
    return str;
}

void
cu_str_cct_vfmt(cu_str_t str, char const *fmt, va_list va)
{
    size_t size;
    char *buf;
    FILE *out = open_memstream(&buf, &size);
    cu_vfprintf(out, fmt, va);
    fclose(out);
    cu_str_cct_charr(str, buf, size);
    free(buf);
}
cu_str_t
cu_str_new_vfmt(char const *fmt, va_list va)
{
    cu_str_t str = cu_gnew(struct cu_str_s);
    cu_str_cct_vfmt(str, fmt, va);
    return str;
}
cu_str_t
cu_str_onew_vfmt(char const *fmt, va_list va)
{
    cu_str_t str = cudyn_onew(cu_str);
    cu_str_cct_vfmt(str, fmt, va);
    return str;
}

cu_str_t
cu_str_new_fmt(char const *fmt, ...)
{
    cu_str_t str;
    va_list va;
    va_start(va, fmt);
    str = cu_str_new_vfmt(fmt, va);
    va_end(va);
    return str;
}

cu_str_t
cu_str_onew_fmt(char const *fmt, ...)
{
    cu_str_t str;
    va_list va;
    va_start(va, fmt);
    str = cu_str_onew_vfmt(fmt, va);
    va_end(va);
    return str;
}

void
cu_str_append_vfmt(cu_str_t dst, char const *fmt, va_list va)
{
    char *buf;
    size_t size;
    FILE *out = open_memstream(&buf, &size);
    cu_vfprintf(out, fmt, va);
    fclose(out);
    cu_str_append_charr(dst, buf, size);
    free(buf);
}

void
cu_str_append_fmt(cu_str_t dst, char const *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    cu_str_append_vfmt(dst, fmt, va);
    va_end(va);
}

void
cu_str_assign_substr(cu_str_t x, size_t pos, size_t len)
{
    x->arr += pos;
    x->len = len;
    x->cap -= pos;
}

cu_str_t
cu_str_substr(cu_str_t x, size_t pos, size_t len)
{
    cu_str_t str = cu_gnew(struct cu_str_s);
    cu_debug_assert(pos + len <= x->len);
    str->arr = x->arr + pos;
    str->len = len;
    str->cap = 0;
    SHARING(x);
    return str;
}

cu_str_t
cu_str_suffix(cu_str_t x, size_t pos)
{
    cu_str_t str = cu_gnew(struct cu_str_s);
    cu_debug_assert(pos <= x->len);
    str->arr = x->arr + pos;
    str->len = x->len - pos;
    str->cap = 0;
    SHARING(x);
    return str;
}

void
cu_str_append_char(cu_str_t dest, char ch)
{
    if (dest->cap <= dest->len) {
	char *old = dest->arr;
	dest->cap = 2*dest->len + 4;
	dest->arr = cu_galloc(sizeof(char)*dest->cap);
	memcpy(dest->arr, old, dest->len);
    }
    dest->arr[dest->len++] = ch;
}

void
cu_str_append_charr(cu_str_t dest, char const *cstr, size_t len)
{
    size_t newlen = dest->len + len;
    if (newlen > dest->cap) {
	char *old = dest->arr;
	dest->cap *= 2;
	if (newlen > dest->cap)
	    dest->cap = newlen;
	dest->arr = cu_galloc(sizeof(char)*dest->cap);
	memcpy(dest->arr, old, dest->len);
    }
    memcpy(dest->arr + dest->len, cstr, len);
    dest->len = newlen;
}

void
cu_str_append_cstr(cu_str_t dest, char const *cstr)
{
    cu_str_append_charr(dest, cstr, strlen(cstr));
}

void
cu_str_unappend(cu_str_t str, size_t char_count)
{
    if (char_count > str->len)
	cu_bugf("cu_str_unappend tried to erase past beginning of string.");
    str->len -= char_count;
}

void
cu_str_insert_substr(cu_str_t dst, cu_str_t src, size_t pos)
{
    /* XXX untested, esp for overlap handling */
    char *s = dst->arr + pos;
    cu_debug_assert(pos < dst->len);
    if (!dst->cap || src->len + dst->len > dst->cap) {
	cu_str_set_capacity(dst, src->len + dst->len + 1);
	s = dst->arr + pos;
    }
    memmove(s + src->len, s, dst->len - pos);
    memcpy(s, src->arr, src->len);
    dst->len += src->len;
}

void
cu_str_erase_substr(cu_str_t dst, size_t pos, size_t len)
{
    size_t endpos = pos + len;
    if (!dst->cap)
	cu_str_set_capacity(dst, dst->len - len + 1);
    cu_debug_assert(pos + len <= dst->len);
    memmove(dst->arr + pos, dst->arr + endpos, dst->len - endpos);
    dst->len -= len;
}

char const *
cu_str_to_cstr(cu_str_t src)
{
    if (src->cap > src->len && cu_str_grab_cap(src)) {
	src->arr[src->len] = 0;
	src->cap = 0;
	return src->arr;
    }
    else {
	char *cstr = cu_galloc(sizeof(char)*(src->len + 1));
	memcpy(cstr, src->arr, src->len);
	cstr[src->len] = 0;
	return cstr;
    }
}

cu_str_t
cu_str_new_idr(cu_idr_t idr)
{
    return cu_str_new_cstr(cu_idr_to_cstr(idr));
}

cu_str_t
cu_str_onew_idr(cu_idr_t idr)
{
    return cu_str_onew_cstr(cu_idr_to_cstr(idr));
}

cu_idr_t
cu_str_to_idr(cu_str_t str)
{
    return cu_idr_by_charr(str->arr, str->len);
}

char const *(cu_str_charr)(cu_str_t str) { return cu_str_charr(str); }

int
cu_str_cmp(cu_str_t x, cu_str_t y)
{
    int i;
    if (x->len == y->len)
	return strncmp(x->arr, y->arr, x->len);
    else if (x->len < y->len) {
	i = strncmp(x->arr, y->arr, x->len);
	return i == 0? -1 : i;
    }
    else {
	i = strncmp(x->arr, y->arr, y->len);
	return i == 0? 1 : i;
    }
}

int
cu_str_cmp_charr(cu_str_t x, char const *y_arr, size_t y_size)
{
    if (x->len < y_size)
	return -1;
    if (x->len > y_size)
	return 1;
    return memcmp(x->arr, y_arr, y_size);
}

int
cu_str_cmp_cstr(cu_str_t x, char const *y)
{
    return cu_str_cmp_charr(x, y, strlen(y));
}

cu_bool_fast_t
cu_str_eq(cu_str_t x, cu_str_t y)
{
    if (x->len != y->len)
	return cu_false;
    return memcmp(x->arr, y->arr, x->len) == 0;
}

#define PRIME0 10000019
#define PRIME1 10000079
#define PRIME2 10000103
#define PRIME3 10000121

cu_hash_t
cu_str_hash(cu_str_t x)
{
    char const *s = x->arr;
    char const *se = s + x->len;
    cu_hash_t h = PRIME0;
    while (s < se)
	h = h*PRIME2 + *s++;
    return h;
}

int
cu_str_coll(cu_str_t x, cu_str_t y)
{
    char *x0;
    char *y0;
    if (x->cap > x->len)
	x0 = x->arr;
    else {
	x0 = cu_salloc(sizeof(char)*(x->len + 1));
	memcpy(x0, x->arr, x->len);
    }
    x0[x->len] = 0;

    if (y->cap > y->len)
	y0 = y->arr;
    else {
	y0 = cu_salloc(sizeof(char)*(y->len + 1));
	memcpy(y0, y->arr, y->len);
    }
    y0[y->len] = 0;

    if (cuP_locale_is_utf8)
	return strcoll(x0, y0);
    else
	return strcmp(x0, y0);
}

int
cu_str_coll_charr(cu_str_t x, char const *y_arr, size_t y_size)
{
    char *x0;
    char *y0;
    if (x->cap > x->len)
	x0 = x->arr;
    else {
	x0 = cu_salloc(sizeof(char)*(x->len + 1));
	memcpy(x0, x->arr, x->len);
    }
    x0[x->len] = 0;

    y0 = cu_salloc(sizeof(char)*(y_size + 1));
    memcpy(y0, y_arr, y_size);
    y0[y_size] = 0;
    if (cuP_locale_is_utf8)
	return strcoll(x0, y0);
    else
	return strcmp(x0, y0);
}

int
cu_str_coll_cstr(cu_str_t x, char const *y)
{
    char *x0;
    if (x->cap > x->len)
	x0 = x->arr;
    else {
	x0 = cu_salloc(sizeof(char)*(x->len + 1));
	memcpy(x0, x->arr, x->len);
    }
    x0[x->len] = 0;
    if (cuP_locale_is_utf8)
	return strcoll(x0, y);
    else
	return strcmp(x0, y);
}

size_t
cu_str_chr(cu_str_t str, char ch)
{
    size_t n = str->len;
    size_t i;
    for (i = 0; i < n; ++i) {
	if (str->arr[i] == ch)
	    return i;
    }
    return cu_str_index_none;
}

size_t
cu_str_rchr(cu_str_t str, char ch)
{
    size_t i = str->len;
    while (i > 0) {
	if (str->arr[--i] == ch)
	    return i;
    }
    return cu_str_index_none;
}

size_t
cu_str_chr_in_substr(cu_str_t str, size_t pos, size_t len, char ch)
{
    size_t end = pos + len;
    assert(pos <= end && end <= str->len);
    while (pos < end) {
	if (str->arr[pos] == ch)
	    return pos;
	++pos;
    }
    return cu_str_index_none;
}

size_t
cu_str_rchr_in_substr(cu_str_t str, size_t pos, size_t len, char ch)
{
    size_t end = pos + len;
    assert(pos <= end && end <= str->len);
    while (pos < end) {
	--end;
	if (str->arr[end] == ch)
	    return end;
    }
    return cu_str_index_none;
}

cu_bool_t
cu_str_subeq_charr(cu_str_t str, size_t pos, char const *arr, size_t len)
{
    if (pos + len > cu_str_size(str))
	return cu_false;
    return !strncmp(str->arr + pos, arr, len);
}

cu_bool_t
cu_str_subeq(cu_str_t str, size_t pos, cu_str_t substr)
{
    return cu_str_subeq_charr(str, pos, substr->arr, substr->len);
}

cu_bool_t
cu_str_subeq_cstr(cu_str_t str, size_t pos, char const *cstr)
{
    return cu_str_subeq_charr(str, pos, cstr, strlen(cstr));
}

void
cu_str_chop_space(cu_str_t str)
{
    char *s = str->arr;
    int n = str->len;
    while (n > 0 && isspace(*s)) {
	++s;
	--n;
    }
    do --n; while (n > 0 && isspace(s[n]));
    ++n;
    str->arr = s;
    str->cap -= str->len - n;
    str->len = n;
}

cu_bool_t
cu_str_next_space_delimited(cu_str_t str, size_t *pos, size_t *len)
{
    char const *s = str->arr;
    char const *e = s + str->len;
    char const *sp;
    s += *pos + *len;
    while (s < e && isspace(*s))
	++s;
    if (s == e)
	return cu_false;
    *pos = s - str->arr;
    sp = s;
    while (s < e && !isspace(*s))
	++s;
    *len = s - sp;
    return cu_true;
}

cu_str_t
cu_str_quote(cu_str_t str)
{
    /* FIXME. Proper handling of UTF-8, should probably convert to UCS
     * chars to detect non-ascii control characters. */
    cu_str_t res = cu_str_new_charr("\"", 1);
    char const *s = str->arr;
    char const *s_end = s + str->len;
    while (s < s_end) {
	switch (*s) {
	    case '\n': cu_str_append_charr(res, "\\n", 2); break;
	    case '\t': cu_str_append_charr(res, "\\t", 2); break;
	    case '\v': cu_str_append_charr(res, "\\v", 2); break;
	    case '\f': cu_str_append_charr(res, "\\f", 2); break;
	    case '\b': cu_str_append_charr(res, "\\b", 2); break;
	    case '\r': cu_str_append_charr(res, "\\r", 2); break;
	    case '\\': cu_str_append_charr(res, "\\\\", 2); break;
	    case '"': cu_str_append_charr(res, "\\\"", 2); break;
	    default:
		cu_str_append_char(res, *s);
		break;
	}
	++s;
    }
    cu_str_append_char(res, '"');
    return res;
}

cu_clop_edef(cu_str_eq_clop, cu_bool_t, cu_str_t x, cu_str_t y)
{ return cu_str_eq(x, y); }
cu_clop_edef(cu_str_hash_clop, cu_hash_t, cu_str_t x)
{ return cu_str_hash(x); }
cu_clop_edef(cu_str_cmp_clop, int, cu_str_t x, cu_str_t y)
{ return cu_str_cmp(x, y); }
cu_clop_edef(cu_str_coll_clop, int, cu_str_t x, cu_str_t y)
{ return cu_str_coll(x, y); }

cudyn_stdtype_t cuP_str_type;

void
cuP_str_init(void)
{
    cuP_str_type = cudyn_stdtype_new();
}
