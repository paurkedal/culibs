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

#include <cutext/lsource.h>
#include <cutext/wccat.h>
#include <cutext/wctype.h>
#include <cu/str.h>
#include <cu/memory.h>
#include <string.h>

void
cutext_lsource_init(cutext_lsource_t lsrc, cutext_source_t src,
		    cu_locbound_t locb)
{
    src = cutext_source_stack_iconv(cu_wchar_encoding, src);
    if (!cutext_source_can_look(src))
	src = cutext_source_stack_buffer(src);

    cutext_bufsource_init(&lsrc->bsrc, src);
    lsrc->indentation = -1;
    cu_locbound_init_copy(&lsrc->locb, locb);
}

cu_bool_t
cutext_lsource_init_fopen(cutext_lsource_t lsrc,
			  char const *enc, cu_str_t path)
{
    cutext_source_t src = cutext_source_fopen(enc, cu_str_to_cstr(path));
    struct cu_locbound locb;
    if (!src)
	return cu_false;
    cu_locbound_init(&locb, cu_locorigin_new(path, 8), 1, 0);
    cutext_lsource_init(lsrc, src, &locb);
    return cu_true;
}

cutext_lsource_t
cutext_lsource_new(cutext_source_t src, cu_locbound_t locb)
{
    cutext_lsource_t lsrc = cu_gnew(struct cutext_lsource);
    cutext_lsource_init(lsrc, src, locb);
    return lsrc;
}

cutext_lsource_t
cutext_lsource_new_fopen(char const *enc, cu_str_t path)
{
    cutext_lsource_t lsrc = cu_gnew(struct cutext_lsource);
    if (cutext_lsource_init_fopen(lsrc, enc, path))
	return lsrc;
    else
	return 0;
}

static void
_pass_wchar(cutext_lsource_t lsrc, cu_wchar_t ch)
{
    if (ch == '\n') {
	cu_locbound_put_newline(&lsrc->locb);
	lsrc->indentation = -1;
    }
    else {
	if (lsrc->indentation == -1 && !cutext_iswspace(ch))
	    lsrc->indentation = cu_locbound_column(&lsrc->locb);
	cutext_locbound_put_wchar(&lsrc->locb, ch);
    }
}

size_t
cutext_lsource_read(cutext_lsource_t lsrc, cu_wchar_t *buf, size_t n)
{
    size_t size, i;
    if (!buf)
	buf = cu_salloc(n*sizeof(cu_wchar_t));
    size = cutext_bufsource_read(&lsrc->bsrc, buf, n*sizeof(cu_wchar_t));
    n = size / sizeof(cu_wchar_t);
    for (i = 0; i < n; ++i)
	_pass_wchar(lsrc, buf[i]);
    return n;
}

size_t
cutext_lsource_skip(cutext_lsource_t lsrc, size_t n)
{
    size_t size, sizep, i;
    cu_wchar_t const *arr;
    arr = cutext_bufsource_look(&lsrc->bsrc, n*sizeof(cu_wchar_t), &size);
    sizep = cutext_bufsource_read(&lsrc->bsrc, NULL, size);
    cu_debug_assert(size == sizep);
    n = size / sizeof(cu_wchar_t);
    for (i = 0; i < n; ++i)
	_pass_wchar(lsrc, arr[i]);
    return n;
}

cu_wchar_t const *
cutext_lsource_look(cutext_lsource_t lsrc, size_t n, size_t *n_out)
{
    cu_wchar_t const *arr;
    size_t act_size;
    size_t req_size = n*sizeof(cu_wchar_t);
    arr = cutext_bufsource_look(&lsrc->bsrc, n*sizeof(cu_wchar_t), &act_size);
    cu_debug_assert(act_size % sizeof(cu_wchar_t) == 0);
    if (n_out)
	*n_out = act_size / sizeof(cu_wchar_t);
    else if (act_size < req_size) {
	cu_buffer_extend_fullcap(&lsrc->bsrc.buf, req_size);
	memset(cu_buffer_content_end(&lsrc->bsrc.buf), 0, req_size - act_size);
	arr = cu_buffer_content_start(&lsrc->bsrc.buf);
    }
    return act_size? arr : NULL;
}

cu_wchar_t
cutext_lsource_peek(cutext_lsource_t lsrc)
{
    size_t size;
    cu_wchar_t const *arr;
    arr = cutext_bufsource_look(&lsrc->bsrc, sizeof(cu_wchar_t), &size);
    if (size == sizeof(cu_wchar_t))
	return *arr;
    else {
	cu_debug_assert(size == 0);
	return 0;
    }
}

cu_wchar_t
cutext_lsource_get(cutext_lsource_t lsrc)
{
    cu_wchar_t ch;
    size_t size = cutext_bufsource_read(&lsrc->bsrc, &ch, sizeof(cu_wchar_t));
    if (size == sizeof(cu_wchar_t)) {
	_pass_wchar(lsrc, ch);
	return ch;
    }
    else {
	cu_debug_assert(size == 0);
	return 0;
    }
}

void
cutext_locbound_put_wchar(cu_locbound_t locb, cu_wchar_t ch)
{
    cutext_wccat_t cat;

    if (ch < 128) {
	cu_locbound_put_char(locb, ch);
	return;
    }

    cat = cutext_wchar_wccat(ch);
    switch (cat) {
	case CUTEXT_WCCAT_MN:
	case CUTEXT_WCCAT_ME:
	case CUTEXT_WCCAT_CC:
	case CUTEXT_WCCAT_CF:
	case CUTEXT_WCCAT_CS:
	    break;
	default:
	    cu_locbound_skip_1(locb);
	    break;
    }
}
