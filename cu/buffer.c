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
    void *p = cu_galloc_a(init_cap);
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
cu_buffer_init_drop(cu_buffer_t buf_init, cu_buffer_t buf_drop)
{
    memcpy(buf_init, buf_drop, sizeof(struct cu_buffer_s));
    memset(buf_drop, 0, sizeof(struct cu_buffer_s));
}

void
cu_buffer_swap(cu_buffer_t buf0, cu_buffer_t buf1)
{
    struct cu_buffer_s buf2;
    memcpy(&buf2,  buf0, sizeof(struct cu_buffer_s));
    memcpy( buf1, &buf2, sizeof(struct cu_buffer_s));
    memcpy( buf0,  buf1, sizeof(struct cu_buffer_s));
}

void
cuP_buffer_fix_fullcap(cu_buffer_t buf, size_t fullcap)
{
    void *storage_start = buf->storage_start;
    void *content_start = buf->content_start;
    void *content_end = buf->content_end;
    void *storage_end = buf->storage_end;
    size_t content_size = content_end - content_start;
    if (storage_end - storage_start >= 2*fullcap) {
	memmove(storage_start, content_start, content_size);
	buf->content_start = storage_start;
	buf->content_end = cu_ptr_add(storage_start, content_size);
    }
    else {
	size_t storage_size = cu_ptr_diff(storage_end, storage_start);
	if (fullcap < storage_size*2)
	    fullcap = storage_size*2;
	void *p = cu_galloc_a(fullcap);
	buf->storage_start = p;
	buf->content_start = p;
	buf->content_end = cu_ptr_add(p, content_size);
	buf->storage_end = cu_ptr_add(p, fullcap);
	memcpy(p, content_start, content_size);
    }
}

void
cuP_buffer_fix_freecap(cu_buffer_t buf, size_t freecap)
{
    cuP_buffer_fix_fullcap(buf, cu_buffer_content_size(buf) + freecap);
}

void
cu_buffer_incr_content_end(cu_buffer_t buf, size_t incr)
{
    void *content_end = cu_ptr_add(buf->content_end, incr);
    if (content_end > buf->storage_end) {
	cuP_buffer_fix_fullcap(buf,
			       cu_ptr_diff(content_end, buf->content_start));
	content_end = cu_ptr_add(buf->content_end, incr);
    }
    buf->content_end = content_end;
}

void
cu_buffer_resize_content(cu_buffer_t buf, size_t size)
{
    void *content_end = cu_ptr_add(buf->content_start, size);
    if (content_end > buf->storage_end) {
	cuP_buffer_fix_fullcap(buf, size);
	content_end = cu_ptr_add(buf->content_start, size);
    }
    buf->content_end = content_end;
}

void *
cu_buffer_produce(cu_buffer_t buf, size_t incr)
{
    void *old_end = buf->content_end;
    void *new_end = cu_ptr_add(old_end, incr);
    if (new_end > buf->storage_end) {
	cuP_buffer_fix_fullcap(buf, cu_ptr_diff(new_end, buf->content_start));
	old_end = buf->content_end;
	new_end = cu_ptr_add(old_end, incr);
    }
    buf->content_end = new_end;
    return old_end;
}

void
cu_buffer_write(cu_buffer_t buf, void const *data, size_t size)
{
    void *p = cu_buffer_produce(buf, size);
    memcpy(p, data, size);
}

void
cu_buffer_force_realign(cu_buffer_t buf)
{
    size_t content_size = cu_buffer_content_size(buf);
    memmove(buf->storage_start, buf->content_start, content_size);
    buf->content_start = buf->storage_start;
    buf->content_end = cu_ptr_add(buf->storage_start, content_size);
}

void
cu_buffer_maybe_realign(cu_buffer_t buf)
{
    size_t content_size = cu_buffer_content_size(buf);
    size_t shift = cu_ptr_diff(buf->content_start, buf->storage_start);
    if (shift > content_size) {
	memmove(buf->storage_start, buf->content_start, content_size);
	buf->content_start = buf->storage_start;
	buf->content_end = cu_ptr_add(buf->storage_start, content_size);
    }
}
