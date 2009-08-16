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
#include <cufo/textsink.h>
#include <cutext/wccat.h>
#include <cutext/wctype.h>
#include <cutext/conv.h>
#include <cuos/dsink.h>
#include <cucon/hzmap.h>
#include <cu/inherit.h>
#include <cu/diag.h>
#include <cu/debug.h>
#include <cu/int.h>
#include <cu/size.h>
#include <cu/memory.h>
#include <cu/wstring.h>
#include <cu/conf.h>
#include <string.h>

#define MIN_TEXT_WIDTH 4

#define TX_STREAM(os) cu_from(cufo_textsink, cufo_stream, os)

#ifdef CUCONF_DEBUG_SELF
#  define TX_DEBUG 1
#endif

/* The type of buf_markup entries. */
typedef struct _markup_entry_s *_markup_entry_t;
struct _markup_entry_s
{
    size_t input_pos;
    cufo_tag_t tag;
    cufo_attrbind_t attrbinds; /* NULL for end tags */
};

static void
_ts_enqueue_markup(cufo_textsink_t sink, cufo_tag_t tag, cufo_attrbind_t attrbinds)
{
    size_t buffer_len = cu_buffer_content_size(&sink->buf)/sizeof(cu_wchar_t);
    _markup_entry_t entry = cu_buffer_produce(&sink->buf_markup,
					      sizeof(struct _markup_entry_s));
    entry->input_pos = sink->input_pos + buffer_len;
    entry->tag = tag;
    entry->attrbinds = attrbinds;
}

static cu_bool_t
_ts_write_raw(cufo_textsink_t sink, cu_wchar_t const *arr, size_t len)
{
    size_t size_wr;
    size_t size = len*sizeof(cu_wchar_t);
    size_wr = cutext_sink_write(sink->subsink, arr, size);
    if (size_wr == size)
	return cu_true;
    else if (size_wr == (size_t)-1)
	return cu_false;
    else
	cu_bugf("Sink should but did not consume all data.");
}

static cu_bool_t
_ts_write_from_input(cufo_textsink_t sink, cu_wchar_t const *arr, size_t len)
{
    size_t input_pos = sink->input_pos;
    size_t final_pos = input_pos + len;
    for (;;) {
	size_t markup_pos;
	_markup_entry_t entry CU_NOINIT(NULL);

	if (cu_buffer_content_size(&sink->buf_markup) > 0) {
	    entry = cu_buffer_content_start(&sink->buf_markup);
	    markup_pos = entry->input_pos;
	}
	else
	    markup_pos = SIZE_MAX;

	if (input_pos < markup_pos) {
	    size_t next_pos = cu_size_min(markup_pos, final_pos);
	    size_t frag_len = next_pos - input_pos;
	    if (!_ts_write_raw(sink, arr, frag_len))
		return cu_false;
	    arr += frag_len;
	    input_pos = next_pos;
	}

	/* It's possible that markup_pos < input_pos due to elimination of
	 * spaces. */
	if (input_pos < markup_pos)
	    break;
	cu_debug_assert(entry);

	cutext_sink_flush(sink->subsink);
	if (entry->attrbinds)
	    cufo_sink_enter(sink->subsink, entry->tag, entry->attrbinds);
	else
	    cufo_sink_leave(sink->subsink, entry->tag);
	cu_buffer_incr_content_start(&sink->buf_markup,
				     sizeof(struct _markup_entry_s));
    }
    sink->input_pos = input_pos;
    return cu_true;
}

CU_SINLINE cu_bool_t
_ts_newline(cufo_textsink_t sink)
{
    return _ts_write_raw(sink, L"\n", 1);
}

CU_SINLINE int
_ts_wstring_width(cu_wstring_t s)
{
    /* TODO: Account for escape sequences, accents, and other characters of
     * widths other than 1 cell. */
    return cu_wstring_length(s);
}

static int
_ts_usable_width(cufo_textsink_t sink)
{
    int w = cufo_textsink_width(sink);
    if (sink->is_cont) {
	w -= sink->cont_indent;
	if (sink->cont_bol_insert)
	    w -= _ts_wstring_width(sink->cont_bol_insert);
    }
    return w;
}

static cu_bool_t
_ts_indent(cufo_textsink_t sink)
{
    int nt, ns;
    cu_wchar_t *s0, *s;

    ns = sink->left_margin;
    if (sink->is_cont)
	ns += sink->cont_indent;
    if (sink->tabstop) {
	nt = ns/sink->tabstop;
	ns = ns%sink->tabstop;
    } else
	nt = 0;
    s0 = s = cu_salloc((nt + ns)*sizeof(cu_wchar_t));
    while (nt--) *s++ = L'\t';
    while (ns--) *s++ = L' ';
    if (!_ts_write_raw(sink, s0, s - s0))
	return cu_false;
    if (sink->is_cont && sink->cont_bol_insert)
	return _ts_write_raw(sink, cu_wstring_array(sink->cont_bol_insert),
			     cu_wstring_length(sink->cont_bol_insert));
    else
	return cu_true;
}

static double
_ts_tiedness(cufo_textsink_t sink, cu_wint_t ch0, cu_wint_t ch1)
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

#ifdef TX_DEBUG
static void
_ts_check_buffered_width(cufo_textsink_t sink)
{
    int col = cu_buffer_content_size(&sink->buf)/sizeof(cu_wchar_t);
    cu_debug_assert(col == sink->buffered_width);
}
#else
#define _ts_check_buffered_width(sink) ((void)0)
#endif

/* Examine [s, s + len) for the best place to break the line.  Return the
 * offset within s in *pos_out, and return the number of columns in
 * [s, s + *pos_out]. */
static int
_ts_find_break(cufo_textsink_t sink, int text_width,
	       cu_wchar_t const *s, size_t len)
{
    size_t n;
    double bv_min = 1e9;
    double dist_badness_diff = -1.0/text_width;
    double dist_badness = 1.0;
    int bv_pos = 0;
    cu_wchar_t last_char = 0x20;

    if (len > text_width)
	len = text_width;
    cu_debug_assert(len);
    for (n = 0; n < len; ++n) {
	double break_badness, bv;
	break_badness = _ts_tiedness(sink, last_char, s[n]);
	last_char = s[n];
	bv = dist_badness + break_badness;
	if (bv < bv_min) {
	    bv_min = bv;
	    bv_pos = n;
	}
	dist_badness += dist_badness_diff;
    }
    while (bv_pos > 0 && cutext_iswspace(s[bv_pos - 1]))
	--bv_pos;
    return bv_pos;
}

static cu_bool_t
_ts_write_line_wrap(cufo_textsink_t sink)
{
    cu_wchar_t const *s;
    size_t len;
    int line_width, text_width;

    text_width = _ts_usable_width(sink);
    if (sink->cont_eol_insert)
	text_width -= _ts_wstring_width(sink->cont_eol_insert);
    if (text_width < MIN_TEXT_WIDTH)
	text_width = MIN_TEXT_WIDTH;

    /* Determine how many characters to write before wrapping. */
    len = cu_buffer_content_size(&sink->buf)/sizeof(cu_wchar_t);
    s = cu_buffer_content_start(&sink->buf);
    line_width = _ts_find_break(sink, text_width, s, len);
    cu_debug_assert(line_width <= text_width);

    /* Output the line. */
    if (!_ts_indent(sink))
	return cu_false;
    if (!_ts_write_from_input(sink, s, line_width))
	return cu_false;
    if (sink->cont_eol_insert)
	if (!_ts_write_raw(sink, cu_wstring_array(sink->cont_eol_insert),
			   cu_wstring_length(sink->cont_eol_insert)))
	    return cu_false;
    _ts_check_buffered_width(sink);

    /* Update the buffer. */
    while (line_width < len && cutext_iswspace(s[line_width]))
	++line_width;
    cu_buffer_incr_content_start(&sink->buf, line_width*sizeof(cu_wchar_t));
    sink->buffered_width -= line_width;
    _ts_check_buffered_width(sink);

    sink->is_cont = cu_true;
    return _ts_newline(sink);
}

static cu_bool_t
_ts_write_line_nl(cufo_textsink_t sink)
{
    cu_wchar_t const *s = cu_buffer_content_start(&sink->buf);
    size_t len = cu_buffer_content_size(&sink->buf)/sizeof(cu_wchar_t);
    if (!_ts_indent(sink))
	return cu_false;
    if (!_ts_write_from_input(sink, s, len))
	return cu_false;
    cu_buffer_clear(&sink->buf);
    sink->buffered_width = 0;
    sink->is_cont = cu_false;
    return _ts_newline(sink);
}

static void
_ts_strip_leading_space(cufo_textsink_t sink)
{
    cu_wchar_t *s = cu_buffer_content_start(&sink->buf);
    cu_wchar_t *s_end = cu_buffer_content_end(&sink->buf);
    int n_skip = 0;
    while (s < s_end && cutext_iswspace(*s)) {
	++n_skip;
	++s;
    }
    cu_buffer_set_content_start(&sink->buf, s);
    sink->buffered_width -= n_skip;
    sink->input_pos += n_skip;
}

static size_t
_ts_write(cutext_sink_t tsink, void const *req_data, size_t req_size)
{
    cufo_textsink_t sink = cu_from(cufo_textsink, cutext_sink, tsink);
    int usable_width;
    cu_wchar_t const *frag_start = req_data;
    cu_wchar_t const *s_cur = req_data;
    cu_wchar_t const *s_end
	= (cu_wchar_t const *)((char const *)req_data + req_size);

    usable_width = _ts_usable_width(sink);
    while (s_cur < s_end) {
	switch (*s_cur) {
	    case L'\n':
		cu_buffer_write(&sink->buf, frag_start,
				cu_ptr_diff(s_cur, frag_start));
		if (!_ts_write_line_nl(sink))
		    return (size_t)-1;
		frag_start = ++s_cur;
		break;
	    default:
		++sink->buffered_width;
		++s_cur;
		break;
	}
	if (sink->buffered_width > usable_width) {
	    cu_buffer_write(&sink->buf, frag_start,
			    cu_ptr_diff(s_cur, frag_start));
	    if (!_ts_write_line_wrap(sink))
		return (size_t)-1;
	    _ts_strip_leading_space(sink);
	    frag_start = s_cur;
	    usable_width = _ts_usable_width(sink);
	}
    }
    cu_buffer_write(&sink->buf, frag_start, cu_ptr_diff(s_cur, frag_start));
    return req_size;
}

static cu_bool_t
_ts_flush(cutext_sink_t tsink)
{
    cufo_textsink_t sink = cu_from(cufo_textsink, cutext_sink, tsink);
    return cutext_sink_flush(sink->subsink);
}

void
cufo_textsink_block_boundary(cufo_textsink_t sink)
{
    cu_debug_assert(cu_buffer_content_size(&sink->buf) == 0);
    if (sink->is_cont) {
	_ts_newline(sink);
	sink->is_cont = cu_false;
    }
}

static void
_ts_styler_insert(cufo_textsink_t sink, cu_wstring_t s)
{
    cu_buffer_write(&sink->buf, cu_wstring_array(s),
		    cu_wstring_length(s)*sizeof(cu_wchar_t));
}

static cu_bool_t
_ts_enter(cutext_sink_t tsink, cufo_tag_t tag, cufo_attrbind_t attrbinds)
{
    cufo_textsink_t sink = cu_from(cufo_textsink, cutext_sink, tsink);
    cucon_hzmap_node_t styler_node;
    cu_word_t tag_word = (cu_word_t)tag;
    cu_debug_assert(attrbinds != NULL);
    _ts_enqueue_markup(sink, tag, attrbinds);
    styler_node = cucon_hzmap_1w_find(&sink->style->tag_to_styler, &tag_word);
    if (styler_node) {
	cufo_textstyler_t styler;
	cu_wstring_t s;
	styler = cu_from(cufo_textstyler, cucon_hzmap_node, styler_node);
	s = (*styler->enter)(sink, tag, attrbinds);
	if (s)
	    _ts_styler_insert(sink, s);
	return cu_true;
    }
    else if (sink->style->default_enter) {
	cu_wstring_t s = (*sink->style->default_enter)(sink, tag, attrbinds);
	if (s) {
	    _ts_styler_insert(sink, s);
	    return cu_true;
	}
	else
	    return cu_false;
    }
    else
	return cu_false;
}

static void
_ts_leave(cutext_sink_t tsink, cufo_tag_t tag)
{
    cufo_textsink_t sink = cu_from(cufo_textsink, cutext_sink, tsink);
    cucon_hzmap_node_t styler_node;
    cu_word_t tag_word = (cu_word_t)tag;
    styler_node = cucon_hzmap_1w_find(&sink->style->tag_to_styler, &tag_word);
    if (styler_node) {
	cufo_textstyler_t styler;
	cu_wstring_t s;
	styler = cu_from(cufo_textstyler, cucon_hzmap_node, styler_node);
	s = (*styler->leave)(sink, tag);
	if (s)
	    _ts_styler_insert(sink, s);
    } else if (sink->style->default_leave) {
	cu_wstring_t s = (*sink->style->default_leave)(sink, tag);
	if (s)
	    _ts_styler_insert(sink, s);
    }
    _ts_enqueue_markup(sink, tag, NULL);
}

static cu_box_t
_ts_finish(cutext_sink_t tsink)
{
    cufo_textsink_t sink = cu_from(cufo_textsink, cutext_sink, tsink);
    size_t size_rest = cu_buffer_content_size(&sink->buf);
    if (size_rest)
	_ts_write_from_input(sink, cu_buffer_content_start(&sink->buf),
			     size_rest/sizeof(cu_wchar_t));
    return cutext_sink_finish(sink->subsink);
}

static void
_ts_discard(cutext_sink_t tsink)
{
    cufo_textsink_t sink = cu_from(cufo_textsink, cutext_sink, tsink);
    cutext_sink_discard(sink->subsink);
}

static cu_box_t
_ts_info(cutext_sink_t tsink, cutext_sink_info_key_t key)
{
    cufo_textsink_t sink = cu_from(cufo_textsink, cutext_sink, tsink);
    switch (key) {
	case CUTEXT_SINK_INFO_DEBUG_STATE:
	    return cu_box_ptr(cutext_sink_info_debug_state_t,
			      cu_str_new_cstr("cufo_textsink_t"));
	default:
	    return cutext_sink_info(sink->subsink, key);
    }
}

static cu_bool_t
_ts_iterA_subsinks(cutext_sink_t tsink, cu_clop(f, cu_bool_t, cutext_sink_t))
{
    cufo_textsink_t sink = cu_from(cufo_textsink, cutext_sink, tsink);
    return cu_call(f, sink->subsink);
}

struct cutext_sink_descriptor_s _textsink_descriptor = {
    CUTEXT_SINK_DESCRIPTOR_DEFAULTS,
    .flags = CUTEXT_SINK_FLAG_CLOGFREE,
    .write = _ts_write,
    .flush = _ts_flush,
    .finish = _ts_finish,
    .discard = _ts_discard,
    .enter = _ts_enter,
    .leave = _ts_leave,
    .info = _ts_info,
    .iterA_subsinks = _ts_iterA_subsinks
};

static cutext_sink_t
_ts_sink_new(cutext_sink_t subsink, cufo_textstyle_t style)
{
    char const *sub_encoding = cutext_sink_encoding(subsink);
    cufo_textsink_t sink;
    cu_debug_assert(style->sink_size >= sizeof(struct cufo_textsink_s));
    sink = cu_galloc(style->sink_size);
    if (!sub_encoding)
	sub_encoding = "UTF-8";
    if (!cu_encoding_is_wchar_compat(sub_encoding))
	subsink = cutext_sink_stack_iconv(cu_wchar_encoding, subsink);
    if (!cutext_sink_is_clogfree(subsink))
	subsink = cutext_sink_stack_buffer(subsink);
    cu_buffer_init(&sink->buf, 128);
    cu_buffer_init(&sink->buf_markup, sizeof(struct _markup_entry_s)*4);

    cutext_sink_init(cu_to(cutext_sink, sink), &_textsink_descriptor);
    sink->subsink = subsink;

    sink->buffered_width = 0;
    sink->style = style;
    sink->tabstop = 8;
    sink->left_margin = 0;
    sink->right_margin = 76;
    sink->cont_indent = 0;
    sink->cont_eol_insert = NULL;
    sink->cont_bol_insert = NULL;

    sink->is_cont = cu_false;
    sink->input_pos = 0;

    cu_call(style->sink_init, sink);
    return cu_to(cutext_sink, sink);
}

cufo_stream_t
cufo_open_text_sink(cufo_textstyle_t style, cutext_sink_t subsink)
{
    cutext_sink_t sink;
    cufo_stream_t fos = cu_gnew(struct cufo_stream_s);
    if (!style)
	style = cufo_default_textstyle();
    sink = _ts_sink_new(subsink, style);
    cufo_stream_init(fos, cu_wchar_encoding, sink);
    return fos;
}

void
cufo_textstyle_init(cufo_textstyle_t style, size_t stream_size,
		    cu_clop(stream_init, void, cufo_textsink_t))
{
    style->sink_size = stream_size;
    style->sink_init = stream_init;
    cucon_hzmap_init(&style->tag_to_styler, 1);
}

void
cufo_textstyle_bind_static(cufo_textstyle_t style,
			   cufo_tag_t tag, cufo_textstyler_t styler)
{
    styler->tag = (cu_word_t)tag;
    cucon_hzmap_insert_node(&style->tag_to_styler,
			    cu_to(cucon_hzmap_node, styler));
}
