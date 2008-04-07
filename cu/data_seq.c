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

#include <cu/data_seq.h>

#define ALGO_BLOCK_SIZE 4096

cu_word_t
cu_data_source_control(cu_data_source_t source, int op, ...)
{
    cu_word_t res;
    va_list va;
    va_start(va, op);
    res = (*source->control)(source, op, va);
    va_end(va);
    return res;
}

cu_word_t
cu_data_sink_control(cu_data_sink_t sink, int op, ...)
{
    cu_word_t res;
    va_list va;
    va_start(va, op);
    res = (*sink->control)(sink, op, va);
    va_end(va);
    return res;
}

cu_bool_t
cu_data_source_sink_short(cu_data_source_t source, cu_data_sink_t sink)
{
    char buf[ALGO_BLOCK_SIZE];
    for (;;) {
	char *s;
	size_t rsize;
	rsize = cu_data_source_read(source, buf, ALGO_BLOCK_SIZE);
	if (!rsize)
	    break;
	s = buf;
	while (rsize > 0) {
	    size_t wsize;
	    wsize = cu_data_sink_write(sink, buf, rsize);
	    if (wsize == 0)
		return cu_false;
	    rsize -= wsize;
	    s += wsize;
	}
    }
    return cu_true;
}
