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

#include <cufo/target_sink.h>
#include <cu/data_seq.h>
#include <cu/wstring.h>

#define BUFSINK(sink) cu_from(bufsink, cu_data_sink, sink)

typedef struct bufsink_s *bufsink_t;
struct bufsink_s
{
    cu_inherit (cu_data_sink_s);
    struct cu_buffer_s buffer;
};

size_t
bufsink_write(cu_data_sink_t sink, void const *arr, size_t len)
{
    void *dst = cu_buffer_produce(&BUFSINK(sink)->buffer, len);
    memcpy(dst, arr, len);
    return len;
}

cu_word_t
wstring_control(cu_data_sink_t sink, int op, va_list va)
{
    switch (op) {
	    cu_buffer_t buf;
	case CU_DATA_CONTROL_FINISH:
	    buf = &BUFSINK(sink)->buffer;
	    cu_debug_assert(cu_buffer_content_size(buf) % 4 == 0);
	    return (cu_word_t)cu_wstring_of_arr(cu_buffer_content_start(buf),
						cu_buffer_content_size(buf)/4);
	default:
	    return CU_DATA_STATUS_UNIMPL;
    }
}

cufo_stream_t
cufo_open_wstring_recode(char const *encoding)
{
    bufsink_t sink = cu_gnew(struct bufsink_s);
    cu_data_sink_init(cu_to(cu_data_sink, sink),
		      bufsink_write, wstring_control);
    cu_buffer_init(&sink->buffer, 32);
    return cufo_open_sink(encoding, cu_to(cu_data_sink, sink));
}

cufo_stream_t
cufo_open_wstring()
{
    return cufo_open_wstring_recode(cu_wchar_encoding);
}