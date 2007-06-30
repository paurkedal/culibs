/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2004--2007  Petter Urkedal <urkedal@nbi.dk>
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

#include <cutext/buffer.h>
#include <cu/memory.h>
#include <string.h>

void
cutext_buffer_cct(cutext_buffer_t buf, size_t init_cap)
{
    void *p = cu_galloc(init_cap);
    buf->buf_start = p;
    buf->data_start = p;
    buf->data_end = p;
    buf->buf_end = p + init_cap;
}

cutext_buffer_t
cutext_buffer_new(size_t init_cap)
{
    cutext_buffer_t buf = cu_gnew(struct cutext_buffer_s);
    cutext_buffer_cct(buf, init_cap);
    return buf;
}

void
cutext_buffer_cct_move(cutext_buffer_t buf_cct, cutext_buffer_t buf_dct)
{
    memcpy(buf_cct, buf_dct, sizeof(struct cutext_buffer_s));
    memset(buf_dct, 0, sizeof(struct cutext_buffer_s));
}

void
cutextP_buffer_lookahead_fixup(cutext_buffer_t buf, size_t size)
{
    void *buf_start = buf->buf_start;
    void *data_start = buf->data_start;
    void *data_end = buf->data_end;
    void *buf_end = buf->buf_end;
    size_t data_size = data_end - data_start;
    if (buf_end - buf_start >= 2*size) {
	memmove(buf_start, data_start, data_size);
	buf->data_start = buf_start;
	buf->data_end = buf_start + data_size;
    }
    else {
	if (size < (buf_end - buf_start)*2)
	    size = (buf_end - buf_start)*2;
	void *p = cu_galloc(size);
	buf->buf_start = p;
	buf->data_start = p;
	buf->data_end = p + data_size;
	buf->buf_end = p + size;
	memcpy(p, data_start, data_size);
    }
}

