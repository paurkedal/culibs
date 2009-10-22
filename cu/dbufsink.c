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
#include <cu/buffer.h>
#include <cu/wstring.h>
#include <cu/str.h>

#define BUFSINK(sink) cu_from(cu_dbufsink, cu_dsink, sink)

static size_t
_bufsink_write(cu_dsink_t sink, void const *arr, size_t len)
{
    void *dst = cu_buffer_produce(&BUFSINK(sink)->buffer, len);
    memcpy(dst, arr, len);
    return len;
}

static cu_word_t
_wstring_control(cu_dsink_t sink, int fn, va_list va)
{
    switch (fn) {
	    cu_buffer_t buf;
	case CU_DSINK_FN_FINISH:
	    buf = &BUFSINK(sink)->buffer;
	    cu_debug_assert(cu_buffer_content_size(buf) % 4 == 0);
	    return (cu_word_t)cu_wstring_of_arr(
		cu_buffer_content_start(buf),
		cu_buffer_content_size(buf)/sizeof(cu_wchar_t));
	default:
	    return CU_DSINK_ST_UNIMPL;
    }
}

cu_dsink_t
cu_dsink_new_wstring()
{
    cu_dbufsink_t sink = cu_gnew(struct cu_dbufsink_s);
    cu_dsink_init(cu_to(cu_dsink, sink), _wstring_control, _bufsink_write);
    cu_buffer_init(&sink->buffer, 32);
    return cu_to(cu_dsink, sink);
}

static cu_word_t
_str_control(cu_dsink_t sink, int fn, va_list va)
{
    switch (fn) {
	    cu_buffer_t buf;
	case CU_DSINK_FN_FINISH:
	    buf = &BUFSINK(sink)->buffer;
	    return (cu_word_t)cu_str_new_charr(cu_buffer_content_start(buf),
					       cu_buffer_content_size(buf));
	default:
	    return CU_DSINK_ST_UNIMPL;
    }
}

cu_dsink_t
cu_dsink_new_str(void)
{
    cu_dbufsink_t sink = cu_gnew(struct cu_dbufsink_s);
    cu_dsink_init(cu_to(cu_dsink, sink), _str_control, _bufsink_write);
    cu_buffer_init(&sink->buffer, 16);
    return cu_to(cu_dsink, sink);
}
