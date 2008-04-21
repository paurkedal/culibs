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
#include <cufo/textstyle.h>
#include <cutext/wccat.h>
#include <cutext/wctype.h>
#include <cuos/dsink.h>
#include <cucon/hzmap.h>
#include <cu/inherit.h>
#include <cu/dsink.h>
#include <cu/diag.h>
#include <cu/debug.h>
#include <cu/int.h>
#include <cu/memory.h>
#include <cu/wstring.h>
#include <string.h>

typedef struct tx_stream_s *tx_stream_t;
struct tx_stream_s
{
    cu_inherit (cufo_stream_s);
    struct cu_buffer_s buf;
    cu_dsink_t sink;
    int col;
    cu_bool_t is_cont : 1;

    cufo_textstyle_t style;
    struct cufo_textstate_s ts; /* NB! Last member, text style data follows. */
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

CU_SINLINE int
tx_wstring_width(cu_wstring_t s)
{
    /* TODO: Account for escape sequences, accents, and other characters of
     * widths other than 1 cell. */
    return cu_wstring_length(s);
}

static int
tx_usable_width(tx_stream_t tos)
{
    int w = cufo_textstate_width(&tos->ts);
    if (tos->is_cont) {
	w -= tos->ts.cont_indent;
	if (tos->ts.cont_bol_insert)
	    w -= tx_wstring_width(tos->ts.cont_bol_insert);
    }
    return w;
}

static cu_bool_t
tx_indent(tx_stream_t tos)
{
    int nt, ns;
    cu_wchar_t *s0, *s;

    ns = tos->ts.left_margin;
    if (tos->is_cont)
	ns += tos->ts.cont_indent;
    if (tos->ts.tabstop) {
	nt = ns/tos->ts.tabstop;
	ns = ns%tos->ts.tabstop;
    } else
	nt = 0;
    s0 = s = cu_salloc((nt + ns)*sizeof(cu_wchar_t));
    while (nt--) *s++ = L'\t';
    while (ns--) *s++ = L' ';
    if (!tx_raw_write(tos, s0, s - s0))
	return cu_false;
    if (tos->is_cont && tos->ts.cont_bol_insert)
	return tx_raw_write(tos, cu_wstring_array(tos->ts.cont_bol_insert),
			    cu_wstring_length(tos->ts.cont_bol_insert));
    else
	return cu_true;
}

static double
tx_tiedness(tx_stream_t tos, cu_wint_t ch0, cu_wint_t ch1)
{
    /* TODO: Improve me. */
    cutext_wccat_t ch0_cat = cutext_wchar_wccat(ch0);
    cutext_wccat_t ch1_cat = cutext_wchar_wccat(ch1);
    cu_bool_t ch0_is_alnum, ch1_is_alnum;
    cu_bool_t ch0_is_symalnum, ch1_is_symalnum;

    if (cutext_wccat_is_separator(ch0_cat) ||
	cutext_wccat_is_separator(ch1_cat) ||
	cutext_wccat_is_other(ch0_cat) ||
	cutext_wccat_is_other(ch1_cat))
	return 0.0;

    ch0_is_alnum = cutext_wccat_is_letter(ch0_cat)
		|| cutext_wccat_is_number(ch0_cat) || ch0 == 0x5f;
    ch1_is_alnum = cutext_wccat_is_letter(ch1_cat)
		|| cutext_wccat_is_number(ch1_cat) || ch1 == 0x5f;
    if (ch0_is_alnum && ch1_is_alnum)
	return 1.0;

    ch0_is_symalnum = cutext_wccat_is_symbol(ch0_cat) || ch0_is_alnum;
    ch1_is_symalnum = cutext_wccat_is_symbol(ch1_cat) || ch1_is_alnum;
    if (ch0_is_symalnum && !ch1_is_symalnum)
	return 0.1;
    if (!ch0_is_symalnum && ch1_is_symalnum)
	return 0.15;

    if (ch0_cat == ch1_cat)
	return 1.0;
    else
	return 0.8;
}

static int
tx_find_break(tx_stream_t tos, cu_wchar_t const *s, size_t len, size_t *pos_out)
{
    size_t n = len;
    double bv_min = 1e9;
    double dist_badness_diff = 1.0/len;
    double dist_badness = 0.0;
    int bv_pos = len;
    cu_debug_assert(len);
    while (--n) {
	double break_badness = tx_tiedness(tos, s[n - 1], s[n]);
	double bv = dist_badness + break_badness;
	if (bv < bv_min) {
	    bv_min = bv;
	    bv_pos = n;
	}
	dist_badness += dist_badness_diff;
    }
    while (bv_pos > 0 && cutext_iswspace(s[bv_pos - 1]))
	--bv_pos;
    *pos_out = bv_pos;
    return bv_pos; /* column */
}

static cu_bool_t
tx_write_line_wrap(tx_stream_t tos)
{
    cu_wchar_t const *s;
    size_t len;
    size_t pos_br;
    int col_diff;

    len = cu_buffer_content_size(&tos->buf)/sizeof(cu_wchar_t);
    if (tos->ts.cont_eol_insert)
	len -= tx_wstring_width(tos->ts.cont_eol_insert);
    s = cu_buffer_content_start(&tos->buf);
    col_diff = tx_find_break(tos, s, len, &pos_br);
    if (!tx_indent(tos))
	return cu_false;
    if (!tx_raw_write(tos, s, pos_br))
	return cu_false;
    if (tos->ts.cont_eol_insert)
	if (!tx_raw_write(tos, cu_wstring_array(tos->ts.cont_eol_insert),
			  cu_wstring_length(tos->ts.cont_eol_insert)))
	    return cu_false;
    cu_buffer_incr_content_start(&tos->buf, pos_br*sizeof(cu_wchar_t));
    tos->col -= col_diff;
    tos->is_cont = cu_true;
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
    tos->is_cont = cu_false;
    return tx_newline(tos);
}

static void
tx_strip_leading_space(tx_stream_t tos)
{
    cu_wchar_t *s = cu_buffer_content_start(&tos->buf);
    cu_wchar_t *s_end = cu_buffer_content_end(&tos->buf);
    while (s < s_end && cutext_iswspace(*s)) {
	--tos->col;
	++s;
    }
    cu_buffer_set_content_start(&tos->buf, s);
}

static size_t
tx_write(cufo_stream_t fos, void const *req_data, size_t req_size)
{
    int usable_width;
    tx_stream_t tos = TX_STREAM(fos);
    cu_wchar_t const *frag_start = req_data;
    cu_wchar_t const *s_cur = req_data;
    cu_wchar_t const *s_end
	= (cu_wchar_t const *)((char const *)req_data + req_size);

    usable_width = tx_usable_width(tos);
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
	if (tos->col > usable_width) {
	    cu_buffer_write(&tos->buf, frag_start,
			    cu_ptr_diff(s_cur, frag_start));
	    if (!tx_write_line_wrap(tos))
		return (size_t)-1;
	    tx_strip_leading_space(tos);
	    frag_start = s_cur;
	    usable_width = tx_usable_width(tos);
	}
    }
    cu_buffer_write(&tos->buf, frag_start, cu_ptr_diff(s_cur, frag_start));
    return req_size;
}

static void
tx_styler_insert(tx_stream_t tos, cu_wstring_t s)
{
    /* TODO: Interference with word wrap and sync issues. */
    cu_buffer_write(&tos->buf, cu_wstring_array(s),
		    cu_wstring_length(s)*sizeof(cu_wchar_t));
}

static void
tx_enter(cufo_stream_t fos, cufo_tag_t tag, va_list va)
{
    cucon_hzmap_node_t styler_node;
    tx_stream_t tos = TX_STREAM(fos);
    cu_word_t tag_word = (cu_word_t)tag;
    cufo_flush(fos);
    styler_node = cucon_hzmap_1w_find(&tos->style->tag_to_styler, &tag_word);
    if (styler_node) {
	cufo_textstyler_t styler;
	cu_wstring_t s;
	styler = cu_from(cufo_textstyler, cucon_hzmap_node, styler_node);
	s = (*styler->enter)(&tos->ts, tag, va);
	if (s)
	    tx_styler_insert(tos, s);
    }
}

static void
tx_leave(cufo_stream_t fos, cufo_tag_t tag)
{
    cucon_hzmap_node_t styler_node;
    tx_stream_t tos = TX_STREAM(fos);
    cu_word_t tag_word = (cu_word_t)tag;
    cufo_flush(fos);
    styler_node = cucon_hzmap_1w_find(&tos->style->tag_to_styler, &tag_word);
    if (styler_node) {
	cufo_textstyler_t styler;
	cu_wstring_t s;
	styler = cu_from(cufo_textstyler, cucon_hzmap_node, styler_node);
	s = (*styler->leave)(&tos->ts, tag);
	if (s)
	    tx_styler_insert(tos, s);
    }
}

static void *
tx_close(cufo_stream_t fos)
{
    tx_stream_t tos = TX_STREAM(fos);
    size_t size_rest = cu_buffer_content_size(&tos->buf);
    if (size_rest)
	tx_raw_write(tos, cu_buffer_content_start(&tos->buf),
		     size_rest/sizeof(cu_wchar_t));
    return cu_dsink_finish(TX_STREAM(fos)->sink);
}

static struct cufo_target_s tx_target = {
    .write = tx_write,
    .enter = tx_enter,
    .leave = tx_leave,
    .close = tx_close,
};

static void
tx_stream_init(tx_stream_t tos, char const *encoding, cu_dsink_t target_sink,
	       cufo_textstyle_t style)
{
    size_t stream_size;
    if (!encoding)
	encoding = "UTF-8";
    if (!cu_encoding_is_wchar_compat(encoding)) {
	target_sink = cuos_dsink_open_iconv(cu_wchar_encoding, encoding,
					    target_sink);
	encoding = cu_wchar_encoding;
    }
    if (!cu_dsink_is_clogfree(target_sink))
	target_sink = cu_dsink_stack_buffer(target_sink);
    stream_size = sizeof(struct cufo_stream_s)
		+ style->state_size - sizeof(struct cufo_textstate_s);
    cufo_stream_init(cu_to(cufo_stream, tos), encoding, &tx_target);
    cu_buffer_init(&tos->buf, 128);
    tos->sink = target_sink;
    tos->col = 0;
    tos->is_cont = cu_false;
    tos->style = style;
    tos->ts.tabstop = 8;
    tos->ts.left_margin = 0;
    tos->ts.right_margin = 76;
    tos->ts.cont_indent = 0;
    tos->ts.cont_eol_insert = NULL;
    tos->ts.cont_bol_insert = NULL;
    cu_call(style->state_init, &tos->ts);
}

cufo_stream_t
cufo_open_text_sink(char const *encoding, cufo_textstyle_t style,
		    cu_dsink_t target_sink)
{
    tx_stream_t tos = cu_gnew(struct tx_stream_s);
    if (!style)
	style = cufo_default_textstyle();
    tx_stream_init(tos, encoding, target_sink, style);
    return cu_to(cufo_stream, tos);
}
