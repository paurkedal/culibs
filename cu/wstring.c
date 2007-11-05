/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2007  Petter Urkedal <urkedal@nbi.dk>
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

#include <cu/wstring.h>
#include <cuoo/oalloc.h>
#include <cuoo/intf.h>
#include <cu/size.h>
#ifdef CU_WCHAR_IS_STDC
#  include <wchar.h>
#endif

cu_wstring_t
cu_wstring_of_arr(cu_wchar_t const *src_arr, size_t src_len)
{
    cu_wstring_t s = cu_gnew(struct cu_wstring_s);
    cu_wchar_t *arr = cu_galloc(sizeof(cu_wchar_t)*src_len);
    memcpy(arr, src_arr, sizeof(cu_wchar_t)*src_len);
    s->arr = arr;
    s->len = src_len;
    return s;
}

cu_wstring_t
cu_wstring_of_arr_o(cu_wchar_t const *src_arr, size_t src_len)
{
    cu_wstring_t s = cuoo_onew(cu_wstring);
    cu_wchar_t *arr = cu_galloc(sizeof(cu_wchar_t)*src_len);
    memcpy(arr, src_arr, sizeof(cu_wchar_t)*src_len);
    s->arr = arr;
    s->len = src_len;
    return s;
}

cu_wstring_t
cu_wstring_of_arr_ref(cu_wchar_t const *arr, size_t len)
{
    cu_wstring_t s = cu_gnew(struct cu_wstring_s);
    s->arr = arr;
    s->len = len;
    return s;
}

cu_wstring_t
cu_wstring_of_arr_ref_o(cu_wchar_t const *arr, size_t len)
{
    cu_wstring_t s = cuoo_onew(cu_wstring);
    s->arr = arr;
    s->len = len;
    return s;
}

cu_wstring_t
cu_wstring_of_char_arr(char *arr, size_t len)
{
    iconv_t cd = cu_iconv_for_char_to_wchar();
    cu_wchar_t *buf = cu_salloc(len*sizeof(cu_wstring_t));
    char *inbuf = arr;
    size_t incap = len;
    char *outbuf = (char *)buf;
    size_t outcap = len*sizeof(cu_wstring_t);
    if (iconv(cd, &inbuf, &incap, &outbuf, &outcap)
	== (size_t)-1) {
	switch (errno) {
	    case E2BIG:
		cu_bugf("Unexpected error from iconv: %s", strerror(errno));
	    case EINVAL:
	    case EILSEQ:
		return NULL;
	}
    }
    cu_debug_assert(outcap % sizeof(cu_wstring_t) == 0);
    return cu_wstring_of_arr(buf, len - outcap/sizeof(cu_wstring_t));
}

cu_wstring_t
cu_wstring_of_char_str(char *str)
{
    return cu_wstring_of_char_arr(str, strlen(str));
}

cu_wstring_t
cu_wstring_cat2(cu_wstring_t sl, cu_wstring_t sr)
{
    cu_wchar_t *arr;
    if (sl->len == 0)
	return sr;
    if (sr->len == 0)
	return sl;
    arr = cu_galloc(sizeof(cu_wchar_t)*(sl->len + sr->len));
    memcpy(arr, sl->arr, sizeof(cu_wchar_t)*sl->len);
    memcpy(arr + sl->len, sr->arr, sizeof(cu_wchar_t)*sr->len);
    return cu_wstring_of_arr_ref(arr, sl->len + sr->len);
}

cu_wstring_t
cu_wstring_cat2_o(cu_wstring_t sl, cu_wstring_t sr)
{
    cu_wchar_t *arr;
    if (sl->len == 0)
	return sr;
    if (sr->len == 0)
	return sl;
    arr = cu_galloc(sizeof(cu_wchar_t)*(sl->len + sr->len));
    memcpy(arr, sl->arr, sizeof(cu_wchar_t)*sl->len);
    memcpy(arr + sl->len, sr->arr, sizeof(cu_wchar_t)*sr->len);
    return cu_wstring_of_arr_ref_o(arr, sl->len + sr->len);
}

cu_wstring_t
cu_wstring_cat3(cu_wstring_t s0, cu_wstring_t s1, cu_wstring_t s2)
{
    cu_wchar_t *arr, *p;
    arr = cu_galloc(sizeof(cu_wchar_t)*(s0->len + s1->len + s2->len));
    p = arr;
    memcpy(p, s0->arr, sizeof(cu_wchar_t)*s0->len); p += s0->len;
    memcpy(p, s1->arr, sizeof(cu_wchar_t)*s1->len); p += s1->len;
    memcpy(p, s2->arr, sizeof(cu_wchar_t)*s2->len); p += s2->len;
    return cu_wstring_of_arr_ref(arr, p - arr);
}

cu_wstring_t
cu_wstring_cat3_o(cu_wstring_t s0, cu_wstring_t s1, cu_wstring_t s2)
{
    cu_wchar_t *arr, *p;
    arr = cu_galloc(sizeof(cu_wchar_t)*(s0->len + s1->len + s2->len));
    p = arr;
    memcpy(p, s0->arr, sizeof(cu_wchar_t)*s0->len); p += s0->len;
    memcpy(p, s1->arr, sizeof(cu_wchar_t)*s1->len); p += s1->len;
    memcpy(p, s2->arr, sizeof(cu_wchar_t)*s2->len); p += s2->len;
    return cu_wstring_of_arr_ref_o(arr, p - arr);
}

cu_wstring_t
cu_wstring_slice(cu_wstring_t s, size_t i, size_t j)
{
    cu_debug_assert(i <= j);
    cu_debug_assert(j <= s->len);
    return cu_wstring_of_arr_ref(s->arr + i, j - i);
}

cu_wstring_t
cu_wstring_slice_o(cu_wstring_t s, size_t i, size_t j)
{
    cu_debug_assert(i <= j);
    cu_debug_assert(j <= s->len);
    return cu_wstring_of_arr_ref_o(s->arr + i, j - i);
}

int
cu_wstring_cmp(cu_wstring_t sl, cu_wstring_t sr)
{
    size_t n = cu_size_min(sl->len, sr->len);
#ifdef CU_WCHAR_IS_STDC
    int r;
    r = wcsncmp(sl->arr, sr->arr, n);
    if (r != 0)
	return r;
#else
    size_t i;
    for (i = 0; i < n; ++i) {
	if (sl->arr[i] < sr->arr[i])
	    return -1;
	if (sl->arr[i] > sr->arr[i])
	    return 1;
    }
#endif
    if (sl->len < sr->len)
	return -1;
    if (sl->len > sr->len)
	return 1;
    return 0;
}

cu_wstring_t
cu_wstring_image(cu_wstring_t s, cu_clop(f, cu_wchar_t, cu_wchar_t))
{
    size_t i;
    cu_wchar_t *arr = cu_galloc(sizeof(cu_wchar_t)*s->len);
    for (i = 0; i < s->len; ++i)
	arr[i] = cu_call(f, s->arr[i]);
    return cu_wstring_of_arr(arr, s->len);
}

cu_wstring_t
cu_wstring_image_o(cu_wstring_t s, cu_clop(f, cu_wchar_t, cu_wchar_t))
{
    size_t i;
    cu_wchar_t *arr = cu_galloc(sizeof(cu_wchar_t)*s->len);
    for (i = 0; i < s->len; ++i)
	arr[i] = cu_call(f, s->arr[i]);
    return cu_wstring_of_arr_o(arr, s->len);
}

cu_bool_t
cu_wstring_forall(cu_wstring_t s, cu_clop(f, cu_bool_t, cu_wchar_t))
{
    size_t i;
    for (i = 0; i < s->len; ++i)
	if (!cu_call(f, s->arr[i]))
	    return cu_false;
    return cu_true;
}

static void
wstring_print(void *o, FILE *out)
{
    fprintf(out, "#[cu_wstring @ %p]", o); /* TODO */
}

static cu_word_t
wstring_dispatch(cu_word_t intf, ...)
{
    switch (intf) {
	case CUOO_INTF_PRINT_FN:
	    return (cu_word_t)wstring_print;
	default:
	    return CUOO_IMPL_NONE;
    }
}

cuoo_stdtype_t cuP_wstring_type;
cu_wstring_t cuP_wstring_empty;

void
cuP_wstring_init()
{
    cuP_wstring_type = cuoo_stdtype_new(wstring_dispatch);
    cuP_wstring_empty = cuoo_onew(cu_wstring);
    cuP_wstring_empty->arr = NULL;
    cuP_wstring_empty->len = 0;
}
