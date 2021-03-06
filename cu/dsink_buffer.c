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

#include <cu/dsink.h>
#include <cu/inherit.h>
#include <cu/buffer.h>
#include <cu/size.h>
#include <cu/memory.h>
#include <cu/diag.h>
#include <string.h>

#define WRITE_THRESHOLD 512

typedef struct _bufsink *_bufsink_t;
struct _bufsink
{
    cu_inherit (cu_dsink);
    cu_dsink_t subsink;
    struct cu_buffer buf;
};

static cu_word_t
_bufsink_flush(_bufsink_t sink)
{
    size_t size_wr;
    size_wr = cu_dsink_write(sink->subsink,
			     cu_buffer_content_start(&sink->buf),
			     cu_buffer_content_size(&sink->buf));
    if (size_wr == (size_t)-1) {
	cu_dsink_discard(sink->subsink);
	sink->subsink = NULL;
	return CU_DSINK_ST_FAILURE;
    }
    cu_buffer_incr_content_start(&sink->buf, size_wr);

    /* If clog is bigger than half the buffer capacity, extend the capacity. */
    if (cu_buffer_content_size(&sink->buf)*2 > cu_buffer_fullcap(&sink->buf))
	cu_buffer_extend_fullcap(&sink->buf,
				 cu_buffer_content_size(&sink->buf)*2);
    return CU_DSINK_ST_SUCCESS;
}

static size_t
_bufsink_write(cu_dsink_t sink_, void const *data, size_t size)
{
    _bufsink_t sink = cu_from(_bufsink, cu_dsink, sink_);
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
	    if (_bufsink_flush(sink) == CU_DSINK_ST_FAILURE)
		return (size_t)-1;
	}
	else
	    /* This is guaranteed as long as the buffer is at least twice
	     * WRITE_THRESHOLD, cf condition of cu_buffer_maybe_realign. */
	    cu_debug_assert(size_left == 0);
    }
    return size;
}

static cu_word_t
_bufsink_control(cu_dsink_t sink_, int fc, va_list va)
{
    _bufsink_t sink = cu_from(_bufsink, cu_dsink, sink_);
    switch (fc) {
	case CU_DSINK_FN_IS_CLOGFREE:
	    return CU_DSINK_ST_SUCCESS;
	case CU_DSINK_FN_DEBUG_DUMP:
	    cu_verbf(0, "dsink buffer, storage_size = %d, content_size = %d.",
		     cu_buffer_storage_size(&sink->buf),
		     cu_buffer_content_size(&sink->buf));
	    return CU_DSINK_ST_SUCCESS;
	case CU_DSINK_FN_DISCARD:
	    if (!sink->subsink)
		return CU_DSINK_ST_SUCCESS;
	    /* Fall though. */
	default:
	    if (sink->subsink) {
		if (fc & 1)
		    _bufsink_flush(sink);
		return cu_dsink_control_va(sink->subsink, fc, va);
	    }
	    else
		return CU_DSINK_ST_FAILURE;
    }
}

cu_dsink_t
cu_dsink_stack_buffer(cu_dsink_t subsink)
{
    _bufsink_t sink = cu_gnew(struct _bufsink);
    cu_dsink_init(cu_to(cu_dsink, sink), _bufsink_control, _bufsink_write);
    sink->subsink = subsink;
    cu_buffer_init(&sink->buf, WRITE_THRESHOLD*2);
    return cu_to(cu_dsink, sink);
}
