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

#include <cu/buffer.h>
#include <cu/memory.h>
#include <string.h>

void
cu_buffer_init(cu_buffer_t buf, size_t init_cap)
{
    void *p = cu_galloc(init_cap);
    buf->storage_start = p;
    buf->content_start = p;
    buf->content_end = p;
    buf->storage_end = p + init_cap;
}

cu_buffer_t
cu_buffer_new(size_t init_cap)
{
    cu_buffer_t buf = cu_gnew(struct cu_buffer_s);
    cu_buffer_init(buf, init_cap);
    return buf;
}

void
cu_buffer_init_drop(cu_buffer_t buf_cct, cu_buffer_t buf_dct)
{
    memcpy(buf_cct, buf_dct, sizeof(struct cu_buffer_s));
    memset(buf_dct, 0, sizeof(struct cu_buffer_s));
}

void
cu_buffer_swap(cu_buffer_t buf0, cu_buffer_t buf1)
{
    struct cu_buffer_s buf2;
    memcpy(&buf2, &buf0, sizeof(struct cu_buffer_s));
    memcpy(&buf1, &buf2, sizeof(struct cu_buffer_s));
    memcpy(&buf0, &buf1, sizeof(struct cu_buffer_s));
}

void
cuP_buffer_fixup(cu_buffer_t buf, size_t size)
{
    void *storage_start = buf->storage_start;
    void *content_start = buf->content_start;
    void *content_end = buf->content_end;
    void *storage_end = buf->storage_end;
    size_t data_size = content_end - content_start;
    if (storage_end - storage_start >= 2*size) {
	memmove(storage_start, content_start, data_size);
	buf->content_start = storage_start;
	buf->content_end = storage_start + data_size;
    }
    else {
	if (size < (storage_end - storage_start)*2)
	    size = (storage_end - storage_start)*2;
	void *p = cu_galloc(size);
	buf->storage_start = p;
	buf->content_start = p;
	buf->content_end = p + data_size;
	buf->storage_end = p + size;
	memcpy(p, content_start, data_size);
    }
}
