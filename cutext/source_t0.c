/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2010  Petter Urkedal <paurkedal@eideticdew.org>
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

#include <cutext/source.h>
#include <cu/test.h>
#include <cu/memory.h>
#include <cu/size.h>
#include <cu/wstring.h>

static cutext_source_t
_make_cstr_source(size_t n)
{
    int i;
    char *buf = cu_galloc(n + 1);
    for (i = 0; i < n; ++i)
	buf[i] = 'a' + i % 26;
    buf[i] = 0;
    return cutext_source_new_cstr(buf);
}

static cutext_source_t
_make_wstr_source(size_t n)
{
    int i;
    cu_wchar_t *buf = cu_galloc((n + 1) * sizeof(cu_wchar_t));
    cu_wstring_t wstr;
    for (i = 0; i < n; ++i)
	buf[i] = 'a' + i % 26;
    buf[i] = 0;
    wstr = cu_wstring_of_arr(buf, n);
    return cutext_source_new_wstring(wstr);
}

static cutext_source_t
_make_wraw_source(size_t n)
{
    int i;
    cu_wchar_t *buf = cu_galloc((n + 1) * sizeof(cu_wchar_t));
    for (i = 0; i < n; ++i)
	buf[i] = 'a' + i % 26;
    buf[i] = 0;
    return cutext_source_new_mem(NULL, buf, n*sizeof(cu_wchar_t));
}

static void
_verify_read(cutext_source_t src, size_t n)
{
    int i = 0;
    char *buf = cu_galloc(n);
    while (i < n) {
	size_t j, m = lrand48() % n + 1;
	size_t k = cu_size_min(n - i, m);
	cu_test_assert(cutext_source_read(src, buf, m) == k);
	for (j = 0; j < k; ++j, ++i)
	    cu_test_assert(buf[j] == 'a' + i % 26);
    }
    cu_test_assert(cutext_source_read(src, buf, 1) == 0);
}

static void
_verify_look(cutext_source_t src, size_t n)
{
    int i = 0;
    while (i < n) {
	size_t j, m = lrand48() % n + 1;
	size_t k;
	char const *buf = cutext_source_look(src, m, &k);
	cu_test_assert(k == cu_size_min(n - i, m));
	for (j = 0; j < k; ++j, ++i)
	    cu_test_assert(buf[j] == 'a' + i % 26);
	cu_test_assert(cutext_source_read(src, NULL, m) == k);
    }
    cu_test_assert(cutext_source_read(src, NULL, 1) == 0);
}

static void
_test_cstr_source(size_t n)
{
    cutext_source_t src;

    src = _make_wstr_source(n);
    cu_test_assert_size_eq(cutext_source_count(src), n*sizeof(cu_wchar_t));

    src = cutext_source_stack_iconv("utf-8", _make_wstr_source(n));
    cu_test_assert_size_eq(cutext_source_count(src), n);

    _verify_read(_make_cstr_source(n), n);
    _verify_look(cutext_source_stack_buffer(_make_cstr_source(n)), n);
    _verify_read(cutext_source_stack_iconv("utf-8", _make_wstr_source(n)), n);
    _verify_read(cutext_source_stack_iconv("utf-8", _make_wraw_source(n)), n);
}

int
main()
{
    int n;
    cutext_init();
    for (n = 1; n < 7000; n = n*3/2 + 1)
	_test_cstr_source(n);
    return 2*!!cu_test_bug_count();
}
