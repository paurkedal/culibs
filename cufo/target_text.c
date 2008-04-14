/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2008  Petter Urkedal <urkedal@nbi.dk>
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

#include <cufo/stream.h>
#include <cufo/tagdefs.h>
#include <cuos/dsink.h>
#include <cu/inherit.h>
#include <cu/dsink.h>
#include <cu/diag.h>
#include <cu/debug.h>
#include <cu/int.h>
#include <cu/memory.h>
#include <string.h>
#include <wctype.h>

typedef struct tx_stream_s *tx_stream_t;
struct tx_stream_s
{
    cu_inherit (cufo_stream_s);
    struct cu_buffer_s buf;
    cu_dsink_t sink;
    int tabstop;
    int col;
    int left_margin, right_margin;
};

#define TX_STREAM(os) cu_from(tx_stream, cufo_stream, os)

static cu_bool_t
tx_raw_write(tx_stream_t tos, cu_wchar_t const *arr, size_t len)
{
    size_t size_wr;
    size_t size = len*sizeof(cu_wchar_t);
    size_wr = cu_dsink_write(tos->sink, arr, size);
    if (size_wr != size) {
	if (size_wr == (size_t)-1)
	    return cu_false;
	else
	    cu_bugf("Sink should but did not consume all data.");
    }
    return cu_true;
}

CU_SINLINE cu_bool_t
tx_newline(tx_stream_t tos)
{
    return tx_raw_write(tos, L"\n", 1);
}

static cu_bool_t
tx_indent(tx_stream_t tos)
{
    int nt, ns;
    cu_wchar_t *s0, *s;

    ns = tos->left_margin;
    if (tos->tabstop) {
	nt = ns/tos->tabstop;
	ns = ns%tos->tabstop;
    } else
	nt = 0;
    s0 = s = cu_salloc((nt + ns)*sizeof(cu_wchar_t));
    while (nt--) *s++ = L'\t';
    while (ns--) *s++ = L' ';
    return tx_raw_write(tos, s0, s - s0);
}

static double
tx_breakpair(tx_stream_t tos, wint_t ch0, wint_t ch1)
{
    /* TODO: Improve me. */
    cu_bool_t ch0_is_word = iswalnum(ch0) || ch0 == L'_';
    cu_bool_t ch1_is_word = iswalnum(ch1) || ch1 == L'_';
    cu_bool_t ch0_is_space = iswspace(ch0);
    cu_bool_t ch1_is_space = iswspace(ch1);
    if (ch0_is_space || ch1_is_space)
	return 0.0;
    else if (ch0_is_word && !ch1_is_word)
	return 0.04;
    else if (!ch0_is_word && ch1_is_word)
	return 0.05;
    else
	return 1.0;
}

static int
tx_find_break(tx_stream_t tos, cu_wchar_t const *s, size_t len, size_t *pos_out)
{
    size_t n = len;
    double bv_min = 1e9;
    double dist_badness_diff = 1.0/(tos->right_margin - tos->left_margin);
    double dist_badness = 0.0;
    int bv_pos = len;
    cu_debug_assert(len);
    while (--n) {
	double break_badness = tx_breakpair(tos, s[n - 1], s[n]);
	double bv = dist_badness + break_badness;
	if (bv < bv_min) {
	    bv_min = bv;
	    bv_pos = n;
	}
	dist_badness += dist_badness_diff;
    }
    while (bv_pos > 0 && iswspace(s[bv_pos - 1]))
	--bv_pos;
    *pos_out = bv_pos;
    return bv_pos; /* column */
}

static cu_bool_t
tx_write_line_wrap(tx_stream_t tos)
{
    cu_wchar_t const *s = cu_buffer_content_start(&tos->buf);
    size_t len = cu_buffer_content_size(&tos->buf)/sizeof(cu_wchar_t);
    size_t pos_br;
    int col_diff;

    col_diff = tx_find_break(tos, s, len, &pos_br);
    if (!tx_indent(tos))
	return cu_false;
    if (!tx_raw_write(tos, s, pos_br))
	return cu_false;
    cu_buffer_incr_content_start(&tos->buf, pos_br*sizeof(cu_wchar_t));
    tos->col -= col_diff;
    return tx_newline(tos);
}

static cu_bool_t
tx_write_line_nl(tx_stream_t tos)
{
    cu_wchar_t const *s = cu_buffer_content_start(&tos->buf);
    size_t len = cu_buffer_content_size(&tos->buf)/sizeof(cu_wchar_t);
    if (!tx_indent(tos))
	return cu_false;
    if (!tx_raw_write(tos, s, len))
	return cu_false;
    cu_buffer_clear(&tos->buf);
    tos->col = 0;
    return tx_newline(tos);
}

static void
tx_strip_leading_space(tx_stream_t tos)
{
    cu_wchar_t *s = cu_buffer_content_start(&tos->buf);
    cu_wchar_t *s_end = cu_buffer_content_end(&tos->buf);
    while (s < s_end && iswspace(*s)) {
	--tos->col;
	++s;
    }
    cu_buffer_set_content_start(&tos->buf, s);
}

static size_t
tx_write(cufo_stream_t fos, void const *req_data, size_t req_size)
{
    tx_stream_t tos = TX_STREAM(fos);
    cu_wchar_t const *frag_start = req_data;
    cu_wchar_t const *s_cur = req_data;
    cu_wchar_t const *s_end
	= (cu_wchar_t const *)((char const *)req_data + req_size);

    while (s_cur < s_end) {
	switch (*s_cur) {
	    case L'\n':
		cu_buffer_write(&tos->buf, frag_start,
				cu_ptr_diff(s_cur, frag_start));
		if (!tx_write_line_nl(tos))
		    return (size_t)-1;
		frag_start = ++s_cur;
		break;
	    default:
		++tos->col;
		++s_cur;
		break;
	}
	if (tos->col > tos->right_margin - tos->left_margin) {
	    cu_buffer_write(&tos->buf, frag_start,
			    cu_ptr_diff(s_cur, frag_start));
	    if (!tx_write_line_wrap(tos))
		return (size_t)-1;
	    tx_strip_leading_space(tos);
	    frag_start = s_cur;
	}
    }
    cu_buffer_write(&tos->buf, frag_start, cu_ptr_diff(s_cur, frag_start));
    return req_size;
}

void
tx_enter(cufo_stream_t fos, cufo_tag_t tag, va_list va)
{
    tx_stream_t tos = TX_STREAM(fos);
    cufo_flush(fos);
    if (tag == cufoT_indent)
	tos->left_margin += 4;
}

void
tx_leave(cufo_stream_t fos, cufo_tag_t tag)
{
    tx_stream_t tos = TX_STREAM(fos);
    cufo_flush(fos);
    if (tag == cufoT_indent)
	tos->left_margin -= 4;
}

void *
tx_close(cufo_stream_t fos)
{
    tx_stream_t tos = TX_STREAM(fos);
    size_t size_rest = cu_buffer_content_size(&tos->buf);
    if (size_rest)
	tx_raw_write(tos, cu_buffer_content_start(&tos->buf),
		     size_rest/sizeof(cu_wchar_t));
    return cu_dsink_finish(TX_STREAM(fos)->sink);
}

struct cufo_target_s tx_target = {
    .write = tx_write,
    .enter = tx_enter,
    .leave = tx_leave,
    .close = tx_close,
};

static void
tx_stream_init(tx_stream_t tos, char const *encoding, cu_dsink_t target_sink)
{
    if (!encoding)
	encoding = "UTF-8";
    if (!cu_encoding_is_wchar_compat(encoding)) {
	target_sink = cuos_dsink_open_iconv(cu_wchar_encoding, encoding,
					    target_sink);
	encoding = cu_wchar_encoding;
    }
//    if (!cu_dsink_is_clogfree(target_sink))
//	target_sink = cu_dsink_stack_buffer(target_sink);
    cufo_stream_init(cu_to(cufo_stream, tos), encoding, &tx_target);
    cu_buffer_init(&tos->buf, 128);
    tos->tabstop = 8;
    tos->col = 0;
    tos->left_margin = 0;
    tos->right_margin = 76;
    tos->sink = target_sink;
}

cufo_stream_t
cufo_open_text_sink(char const *encoding, cu_dsink_t target_sink)
{
    tx_stream_t tos = cu_gnew(struct tx_stream_s);
    tx_stream_init(tos, encoding, target_sink);
    return cu_to(cufo_stream, tos);
}
