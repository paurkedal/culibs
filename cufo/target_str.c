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
#include <cu/str.h>

#define STRSINK(sink) cu_from(strsink, cu_data_sink, sink)

typedef struct strsink_s *strsink_t;
struct strsink_s
{
    cu_inherit (cu_data_sink_s);
    cu_str_t str;
};

size_t
strsink_write(cu_data_sink_t sink, void const *arr, size_t len)
{
    cu_str_append_charr(STRSINK(sink)->str, arr, len);
    return len;
}

cu_word_t
strsink_control(cu_data_sink_t sink, int op, va_list va)
{
    switch (op) {
	case CU_DATA_CONTROL_FINISH:
	    return (cu_word_t)STRSINK(sink)->str;
	default:
	    return CU_DATA_STATUS_UNIMPL;
    }
}

cufo_stream_t
cufo_open_str_recode(char const *encoding)
{
    strsink_t sink = cu_gnew(struct strsink_s);
    cu_data_sink_init(cu_to(cu_data_sink, sink),
		      strsink_write, strsink_control);
    sink->str = cu_str_new();
    return cufo_open_sink(encoding, cu_to(cu_data_sink, sink));
}

cufo_stream_t
cufo_open_str()
{
    return cufo_open_str_recode(NULL);
}
