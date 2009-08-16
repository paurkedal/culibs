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

#include <cutext/sink.h>
#include <cu/diag.h>
#include <cu/debug.h>
#include <cu/memory.h>
#include <cu/str.h>
#include <errno.h>
#include <iconv.h>

#define IC_WRITE_BUFSIZE 2048

typedef struct _iconvsink_s *_iconvsink_t;
struct _iconvsink_s
{
    cu_inherit (cutext_sink_s);
    cutext_sink_t subsink;
    iconv_t cd;
    char const *encoding;
};

#define ICSINK(sink) cu_from(_iconvsink, cutext_sink, sink)

static size_t
_iconv_write(cutext_sink_t sink, void const *data_start, size_t data_size)
{
    char buf[IC_WRITE_BUFSIZE];
    char *src_ptr = (char *)data_start;
    size_t src_size = data_size;

    if (!data_size)
	return 0;

    for (;;) {
	char *dst_ptr = buf;
	size_t dst_cap = IC_WRITE_BUFSIZE;
	size_t st;

	st = iconv(ICSINK(sink)->cd, &src_ptr, &src_size, &dst_ptr, &dst_cap);
	if (dst_cap < IC_WRITE_BUFSIZE) {
	    size_t sub_st;
	    sub_st = cutext_sink_write(ICSINK(sink)->subsink,
				       buf, IC_WRITE_BUFSIZE - dst_cap);
	    if (sub_st != IC_WRITE_BUFSIZE - dst_cap) {
		if (sub_st == (size_t)-1)
		    return (size_t)-1;
		else
		    cu_bugf("Write to clog-free sink did not consume all "
			    "data.");
	    }
	}
	if (st == (size_t)-1) switch (errno) {
	    case EILSEQ:
		cu_errf("Invalid character sequence during conversion.");
		return (size_t)-1;
	    case EINVAL: /* Incomplete sequence: Done. */
		return data_size - src_size;
	    case E2BIG:  /* Out of buffer space: Next round. */
		if (dst_cap == IC_WRITE_BUFSIZE)
		    cu_bugf("%d byte buffer does not fit a single character?",
			    IC_WRITE_BUFSIZE);
		break;
	}
	else {
	    cu_debug_assert(src_size == 0);
	    return data_size;
	}
    }
}

static cu_bool_t
_iconv_flush(cutext_sink_t sink)
{
    return cutext_sink_flush(ICSINK(sink)->subsink);
}

static cu_box_t
_iconv_finish(cutext_sink_t sink)
{
    return cutext_sink_finish(ICSINK(sink)->subsink);
}

static void
_iconv_discard(cutext_sink_t sink)
{
    cutext_sink_discard(ICSINK(sink)->subsink);
}

static cu_bool_t
_iconv_iterA_subsinks(cutext_sink_t sink, cu_clop(f, cu_bool_t, cutext_sink_t sink))
{
    return cu_call(f, ICSINK(sink)->subsink);
}

static cu_box_t
_iconv_info(cutext_sink_t sink, cutext_sink_info_key_t key)
{
    switch (key) {
	case CUTEXT_SINK_INFO_ENCODING:
	    return cu_box_ptr(cutext_sink_info_encoding_t,
			      ICSINK(sink)->encoding);
	case CUTEXT_SINK_INFO_DEBUG_STATE:
	    return cu_box_ptr(cutext_sink_info_debug_state_t,
			      cu_str_new_fmt("iconv from %s",
					     ICSINK(sink)->encoding));
	default:
	    return cutext_sink_info_inherit(sink, key, ICSINK(sink)->subsink);
    }
}

static cu_bool_t
_iconv_enter(cutext_sink_t sink, struct cufo_tag_s *tag,
	     struct cufo_attrbind_s *attrbinds)
{
    cutext_sink_t subsink = ICSINK(sink)->subsink;
    return (*subsink->descriptor->enter)(subsink, tag, attrbinds);
}

static void
_iconv_leave(cutext_sink_t sink, struct cufo_tag_s *tag)
{
    cutext_sink_t subsink = ICSINK(sink)->subsink;
    (*subsink->descriptor->leave)(subsink, tag);
}

static struct cutext_sink_descriptor_s _iconv_descriptor = {
    CUTEXT_SINK_DESCRIPTOR_DEFAULTS,
    .flags = 0,
    .write = _iconv_write,
    .flush = _iconv_flush,
    .finish = _iconv_finish,
    .discard = _iconv_discard,
    .iterA_subsinks = _iconv_iterA_subsinks,
    .info = _iconv_info,
    .enter = _iconv_enter,
    .leave = _iconv_leave
};

cutext_sink_t
cutext_sink_stack_iconv(char const *new_encoding, cutext_sink_t subsink)
{
    char const *sub_encoding = cutext_sink_encoding(subsink);
    _iconvsink_t sink = cu_gnew(struct _iconvsink_s);
    if (sub_encoding == NULL)
	cu_bugf("The subsink passed to cutext_sink_open_iconv must report "
		"its encoding.");
    cutext_sink_init(cu_to(cutext_sink, sink), &_iconv_descriptor);
    sink->subsink = subsink;
    sink->cd = iconv_open(sub_encoding, new_encoding);
    sink->encoding = new_encoding;
    if (sink->cd == (iconv_t)-1)
	return NULL;
    else
	return cu_to(cutext_sink, sink);
}
