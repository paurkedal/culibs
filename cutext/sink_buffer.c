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
#include <cu/inherit.h>
#include <cu/buffer.h>
#include <cu/size.h>
#include <cu/memory.h>
#include <cu/diag.h>
#include <cu/str.h>
#include <string.h>

#define WRITE_THRESHOLD 512

typedef struct _bufsink_s *_bufsink_t;
struct _bufsink_s
{
    cu_inherit (cutext_sink_s);
    cutext_sink_t subsink;
    struct cu_buffer_s buf;
};

static cu_bool_t
_bufsink_flush(cutext_sink_t sink_)
{
    _bufsink_t sink = cu_from(_bufsink, cutext_sink, sink_);
    size_t size_wr;
    size_wr = cutext_sink_write(sink->subsink,
				cu_buffer_content_start(&sink->buf),
				cu_buffer_content_size(&sink->buf));
    if (size_wr == (size_t)-1) {
	cutext_sink_discard(sink->subsink);
	/* sink->subsink = NULL; */
	return cu_false;
    }
    cu_buffer_incr_content_start(&sink->buf, size_wr);

    /* If clog is bigger than half the buffer capacity, extend the capacity. */
    if (cu_buffer_content_size(&sink->buf)*2 > cu_buffer_fullcap(&sink->buf))
	cu_buffer_extend_fullcap(&sink->buf,
				 cu_buffer_content_size(&sink->buf)*2);
    return cu_true;
}

static size_t
_bufsink_write(cutext_sink_t sink_, void const *data, size_t size)
{
    _bufsink_t sink = cu_from(_bufsink, cutext_sink, sink_);
    size_t size_left;

    if (!sink->subsink)
	return (size_t)-1;

    size_left = size;
    while (size_left > 0) {
	size_t tmp_size;
	cu_buffer_maybe_realign(&sink->buf);

	/* Write into the current buffer as much as it's capacity permits. */
	tmp_size = cu_size_min(size_left, cu_buffer_freecap(&sink->buf));
	memcpy(cu_buffer_content_end(&sink->buf), data, tmp_size);
	data = (char const *)data + tmp_size;
	size_left -= tmp_size;
	cu_buffer_incr_content_end(&sink->buf, tmp_size);

	/* Write as much of current buffer as possible. */
	if (cu_buffer_content_size(&sink->buf) >= WRITE_THRESHOLD) {
	    if (!_bufsink_flush(sink_))
		return (size_t)-1;
	}
	else
	    /* This is guaranteed as long as the buffer is at least twice
	     * WRITE_THRESHOLD, cf condition of cu_buffer_maybe_realign. */
	    cu_debug_assert(size_left == 0);
    }
    return size;
}

static cu_box_t
_bufsink_finish(cutext_sink_t sink_)
{
    _bufsink_t sink = cu_from(_bufsink, cutext_sink, sink_);
    _bufsink_flush(sink_);
    return cutext_sink_finish(sink->subsink);
}

static void
_bufsink_discard(cutext_sink_t sink_)
{
    _bufsink_t sink = cu_from(_bufsink, cutext_sink, sink_);
    cutext_sink_discard(sink->subsink);
}

static cu_str_t
_bufsink_debug_state(_bufsink_t sink)
{
    return cu_str_new_fmt("buffering (storage_size = %d, content_size = %d)",
			  cu_buffer_storage_size(&sink->buf),
			  cu_buffer_content_size(&sink->buf));
}

static cu_box_t
_bufsink_info(cutext_sink_t sink_, cutext_sink_info_key_t key)
{
    _bufsink_t sink = cu_from(_bufsink, cutext_sink, sink_);
    switch (key) {
	case CUTEXT_SINK_INFO_DEBUG_STATE:
	    return cu_box_ptr(cutext_sink_info_debug_state_t,
			      _bufsink_debug_state(sink));
	default:
	    return cutext_sink_info_inherit(sink_, key, sink->subsink);
    }
}

static cu_bool_t
_bufsink_iterA_subsinks(cutext_sink_t sink_, cu_clop(f, cu_bool_t, cutext_sink_t))
{
    _bufsink_t sink = cu_from(_bufsink, cutext_sink, sink_);
    return cu_call(f, sink->subsink);
}

static cu_bool_t
_bufsink_enter(cutext_sink_t sink_, struct cufo_tag_s *tag,
	       struct cufo_attrbind_s *attrbinds)
{
    _bufsink_t sink = cu_from(_bufsink, cutext_sink, sink_);
    return (*sink->subsink->descriptor->enter)(sink->subsink, tag, attrbinds);
}

static void
_bufsink_leave(cutext_sink_t sink_, struct cufo_tag_s *tag)
{
    _bufsink_t sink = cu_from(_bufsink, cutext_sink, sink_);
    (*sink->subsink->descriptor->leave)(sink->subsink, tag);
}

static struct cutext_sink_descriptor_s _bufsink_descriptor = {
    CUTEXT_SINK_DESCRIPTOR_DEFAULTS,
    .flags = CUTEXT_SINK_FLAG_CLOGFREE,
    .write = _bufsink_write,
    .flush = _bufsink_flush,
    .finish = _bufsink_finish,
    .discard = _bufsink_discard,
    .iterA_subsinks = _bufsink_iterA_subsinks,
    .info = _bufsink_info,
    .enter = _bufsink_enter,
    .leave = _bufsink_leave,
};

cutext_sink_t
cutext_sink_stack_buffer(cutext_sink_t subsink)
{
    _bufsink_t sink = cu_gnew(struct _bufsink_s);
    cutext_sink_init(cu_to(cutext_sink, sink), &_bufsink_descriptor);
    sink->subsink = subsink;
    cu_buffer_init(&sink->buf, WRITE_THRESHOLD*2);
    return cu_to(cutext_sink, sink);
}
