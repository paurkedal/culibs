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
#include <cu/buffer.h>
#include <cu/wstring.h>
#include <cu/str.h>

#define BUFSINK(sink) cu_from(cutext_buffersink, cutext_sink, sink)

static size_t
_bufsink_write(cutext_sink_t sink, void const *arr, size_t len)
{
    void *dst = cu_buffer_produce(&BUFSINK(sink)->buffer, len);
    memcpy(dst, arr, len);
    return len;
}

static cu_box_t
_wstringsink_finish(cutext_sink_t sink)
{
    cu_buffer_t buf = &BUFSINK(sink)->buffer;
    cu_wstring_t ws;
    cu_debug_assert(cu_buffer_content_size(buf) % 4 == 0);
    ws = cu_wstring_of_arr(cu_buffer_content_start(buf),
			   cu_buffer_content_size(buf)/sizeof(cu_wchar_t));
    return cu_box_ptr(cu_wstring_t, ws);
}

static cu_box_t
_strsink_finish(cutext_sink_t sink)
{
    cu_buffer_t buf = &BUFSINK(sink)->buffer;
    cu_str_t str = cu_str_new_charr(cu_buffer_content_start(buf),
				    cu_buffer_content_size(buf));
    return cu_box_ptr(cu_str_t, str);
}

static cu_box_t
_wstringsink_info(cutext_sink_t sink, cutext_sink_info_key_t key)
{
    switch (key) {
	    size_t len;
	case CUTEXT_SINK_INFO_ENCODING:
	    return cu_box_ptr(cutext_sink_info_encoding_t, cu_wchar_encoding);
	case CUTEXT_SINK_INFO_DEBUG_STATE:
	    len = cu_buffer_content_size(&BUFSINK(sink)->buffer)
		/ sizeof(cu_wchar_t);
	    return cu_box_ptr(cutext_sink_info_debug_state_t,
			      cu_str_new_fmt("writing to cu_wstring_t "
					     "(size=%zd)", len));
	default:
	    return cutext_sink_default_info(sink, key);
    }
}

static cu_box_t
_strsink_info(cutext_sink_t sink, cutext_sink_info_key_t key)
{
    switch (key) {
	    size_t len;
	case CUTEXT_SINK_INFO_ENCODING:
	    return cu_box_ptr(cutext_sink_info_encoding_t, "UTF-8");
	case CUTEXT_SINK_INFO_DEBUG_STATE:
	    len = cu_buffer_content_size(&BUFSINK(sink)->buffer);
	    return cu_box_ptr(cutext_sink_info_debug_state_t,
			      cu_str_new_fmt("writing to cu_str_t "
					     "(size=%zd)", len));
	default:
	    return cutext_sink_default_info(sink, key);
    }
}

static struct cutext_sink_descriptor _wstringsink_descriptor = {
    CUTEXT_SINK_DESCRIPTOR_DEFAULTS,
    .write = _bufsink_write,
    .finish = _wstringsink_finish,
    .info = _wstringsink_info
};

static struct cutext_sink_descriptor _strsink_descriptor = {
    CUTEXT_SINK_DESCRIPTOR_DEFAULTS,
    .write = _bufsink_write,
    .finish = _strsink_finish,
    .info = _strsink_info
};

cutext_sink_t
cutext_sink_new_wstring()
{
    cutext_buffersink_t sink = cu_gnew(struct cutext_buffersink);
    cutext_sink_init(cu_to(cutext_sink, sink), &_wstringsink_descriptor);
    cu_buffer_init(&sink->buffer, 32);
    return cu_to(cutext_sink, sink);
}

cutext_sink_t
cutext_sink_new_str(void)
{
    cutext_buffersink_t sink = cu_gnew(struct cutext_buffersink);
    cutext_sink_init(cu_to(cutext_sink, sink), &_strsink_descriptor);
    cu_buffer_init(&sink->buffer, 16);
    return cu_to(cutext_sink, sink);
}
