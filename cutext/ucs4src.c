/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2004--2010  Petter Urkedal <paurkedal@eideticdew.org>
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

#include <cutext/ucs4src.h>
#include <cu/memory.h>
#include <cucon/pmap.h>
#include <cutext/wccat.h>
#include <string.h>

cu_dlog_def(_file, "dtag=cutext.ucs4src");

void
cutext_ucs4src_init(cutext_ucs4src_t ucs4src, cutext_producer_t producer,
		    cu_str_t path, int line, int column)
{
    cutext_src_init(&ucs4src->src, producer);
    ucs4src->properties = NULL;
    if (cutext_src_lookahead(&ucs4src->src, 4)
	== cutext_status_eos)
	cutextP_ucs4src_terminate(ucs4src);
    cu_sref_init(&ucs4src->srf, path, line, column);
    ucs4src->indentation = -1;
}

cutext_ucs4src_t
cutext_ucs4src_new(cutext_producer_t producer,
		   cu_str_t path, int line, int column)
{
    cutext_ucs4src_t ucs4src = cu_gnew(struct cutext_ucs4src);
    cutext_ucs4src_init(ucs4src, producer, path, line, column);
    return ucs4src;
}

cu_str_t
cutext_ucs4src_scan_str(cutext_ucs4src_t ucs4src,
			cu_clop(is_part, cu_bool_t, cu_wchar_t ch))
{
    cu_str_t str = cu_str_new();
    cu_wchar_t ch;
    while (ch = cutext_ucs4src_peek(ucs4src),
	   ch != 0 && cu_call(is_part, ch)) {
	int err;
	char buf[6];
	size_t buf_cnt = 6;
	char *storage_end = buf;
	err = cutext_wchar_to_charr(ch, &storage_end, &buf_cnt);
	if (err) {
	    cu_errf_at(&ucs4src->srf, "%s", strerror(err));
	    return NULL;
	}
	cu_str_append_charr(str, buf, storage_end - buf);
	cutext_ucs4src_advance(ucs4src, 1);
    }
    return str;
}

cu_clop_def(_header_idrchar, cu_bool_t, cu_wchar_t ch)
{
    switch (ch) {
    case ' ':
    case '\t':
    case '\n':
    case '=':
    case '"':
	return cu_false;
    default:
	return cu_true;
    }
}

cu_clos_def(_header_strchar,
	    cu_prot(cu_bool_t, cu_wchar_t ch),
	    (cu_bool_t have_quote;))
{
    cu_clos_self(_header_strchar);
    if (self->have_quote) {
	self->have_quote = cu_false;
	return cu_true;
    }
    else switch (ch) {
    case '"':
    case '\n':
	return cu_false;
    case '\\':
	self->have_quote = cu_true;
    default:
	return cu_true;
    }
}

void
cutext_ucs4src_init_detect(cutext_ucs4src_t ucs4src,
			   cutext_producer_t producer,
			   cu_str_t path, int line, int column)
{
    cutext_encoding_t chenc;
    cu_wchar_t *s;
    cu_wchar_t ch;
    cutext_src_init(&ucs4src->src, producer);
    ucs4src->properties = NULL;
    chenc = cutext_src_detect_chenc(&ucs4src->src);
    if (chenc != CUTEXT_ENCODING_UCS4HOST) {
	cutext_src_t src = cutext_src_new_grab(&ucs4src->src);
	cutext_src_init(&ucs4src->src,
			cutext_producer_new_iconv(src, chenc,
						  CUTEXT_ENCODING_UCS4HOST));
    }
    if (cutext_src_lookahead(&ucs4src->src, 4)
	== cutext_status_eos)
	cutextP_ucs4src_terminate(ucs4src);
    cu_sref_init(&ucs4src->srf, path, line, column);
    ucs4src->indentation = -1;
    if (cutext_ucs4src_peek(ucs4src) == 0xfeff)
	cutext_ucs4src_advance(ucs4src, 1);
    s = cutext_ucs4src_peek_arr(ucs4src, 2);
    if (s[0] == '#' && s[1] == '?') {
	cu_str_t str;
	ucs4src->properties = cucon_pmap_new();
	cutext_ucs4src_advance(ucs4src, 2);
	while (ch = cutext_ucs4src_peek(ucs4src), ch == ' ' || ch == '\t')
	    cutext_ucs4src_advance(ucs4src, 1);
	if (cutext_ucs4src_peek(ucs4src) == '\n')
	    goto header_error;
	str = cutext_ucs4src_scan_str(ucs4src, _header_idrchar);
	if (!str)
	    goto header_error;
	cucon_pmap_replace_ptr(ucs4src->properties, cu_struniq(""), str);
	for (;;) {
	    _header_strchar_t clos_strchar;
	    cu_str_t key;
	    while (ch = cutext_ucs4src_peek(ucs4src), ch == ' ' || ch == '\t')
		cutext_ucs4src_advance(ucs4src, 1);
	    if (ch == '\n') {
		cutext_ucs4src_advance(ucs4src, 1);
		s = cutext_ucs4src_peek_arr(ucs4src, 2);
		if (s[0] != '#' || s[1] != '?')
		    goto header_ok;
		cutext_ucs4src_advance(ucs4src, 2);
		while (ch = cutext_ucs4src_peek(ucs4src),
		       ch == ' ' || ch == '\t')
		    cutext_ucs4src_advance(ucs4src, 1);
	    }

	    key = cutext_ucs4src_scan_str(ucs4src, _header_idrchar);
	    ch = cutext_ucs4src_get(ucs4src);
	    if (ch != '=')
		goto header_error;
	    ch = cutext_ucs4src_get(ucs4src);
	    if (ch != '"')
		goto header_error;
	    clos_strchar.have_quote = cu_false;
	    str = cutext_ucs4src_scan_str(ucs4src,
					  _header_strchar_prep(&clos_strchar));
	    if (!str)
		goto header_error;
	    ch = cutext_ucs4src_get(ucs4src);
	    if (ch != '"')
		goto header_error;
	    cucon_pmap_replace_ptr(ucs4src->properties,
				 cu_struniq(cu_str_to_cstr(key)), str);
	}
    }
    goto header_ok;
header_error:
    while (ch = cutext_ucs4src_peek(ucs4src), ch && ch != '\n')
	cutext_ucs4src_advance(ucs4src, 1);
    cu_errf_at(&ucs4src->srf, "Invalid header syntax.");
header_ok:
    if (ucs4src->properties) {
	cu_str_t str_tabstop = cucon_pmap_find_ptr(ucs4src->properties,
						   cu_struniq("tabstop"));
	cu_str_t str_encoding = cucon_pmap_find_ptr(ucs4src->properties,
						    cu_struniq("encoding"));
	if (str_tabstop) {
	    int w;
	    if (sscanf(cu_str_to_cstr(str_tabstop), "%d", &w) == 1)
		cu_sref_set_tabstop(&ucs4src->srf, w);
	    else
		cu_warnf("Invalid value for tabstop in header.");
	}
	if (str_encoding) {
	    cutext_encoding_t enc
		= cutext_encoding_by_name(cu_str_to_cstr(str_encoding));
	    if (enc == CUTEXT_ENCODING_UNKNOWN)
		cu_warnf("Unsupported character encoding %S",
			  str_encoding);
	    else if (enc != chenc) {
		/* XX The encoding shall be changed if sensible, else */
		cu_warnf("Encoding %s does not match autodetected %s.",
			  cutext_encoding_name(enc),
			  cutext_encoding_name(chenc));
	    }
	}
    }
}

cutext_ucs4src_t
cutext_ucs4src_new_detect(cutext_producer_t producer,
			  cu_str_t path, int line, int column)
{
    cutext_ucs4src_t ucs4src = cu_gnew(struct cutext_ucs4src);
    cutext_ucs4src_init_detect(ucs4src, producer, path, line, column);
    return ucs4src;
}

cu_wchar_t *
cutextP_ucs4src_peek_arr(cutext_ucs4src_t ucs4src, size_t size)
{
    cutext_status_t st = cutext_src_lookahead(&ucs4src->src, size*4);
    if (st != cutext_status_ok) {
	ucs4src->st = st;
	switch (st) {
	case cutext_status_error:
	    cu_errf_at(&ucs4src->srf, "Error reading from stream.");
	case cutext_status_eos:
	    cutextP_ucs4src_terminate(ucs4src);
	    break;
	default:
	    cu_debug_unreachable();
	}
    }
    return ucs4src->src.buf.content_start;
}

cutext_status_t
cutext_ucs4src_advance(cutext_ucs4src_t ucs4src, size_t size)
{
    cutext_status_t st;
    cu_wchar_t *arr = cutext_ucs4src_peek_arr(ucs4src, size);
    cu_wchar_t *cur = arr;
    cu_wchar_t *end = arr + size;
    while (cur < end && *cur)
	switch (*cur++) {
	case '\n':
	    cu_sref_newline(&ucs4src->srf);
	    ucs4src->indentation = -1;
	    break;
	case '\t':
	    cu_sref_tab(&ucs4src->srf);
	    break;
	case ' ':
	    cu_sref_next(&ucs4src->srf);
	    break;
	default:
	    if (ucs4src->indentation == -1)
		ucs4src->indentation = cu_sref_column(&ucs4src->srf);
	    cu_sref_next(&ucs4src->srf);
	    break;
	}
    cutext_src_advance(&ucs4src->src, size*4);
    st = cutext_src_lookahead(&ucs4src->src, 4);
    cu_dlogf(_file, "Advance %zd, loc = %d:%d, st = %d\n", size,
	     cu_sref_line(&ucs4src->srf), cu_sref_column(&ucs4src->srf), st);
    if (st != cutext_status_ok) {
	cutextP_ucs4src_terminate(ucs4src);
	ucs4src->st = st;
    }
    return st;
}

void
cutextP_ucs4src_terminate(cutext_ucs4src_t ucs4src)
{
    cu_buffer_extend_freecap(&ucs4src->src.buf, 4);
    *(cu_wchar_t*)ucs4src->src.buf.content_end = 0;
}
