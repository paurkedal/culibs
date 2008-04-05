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

#ifndef CU_BUFFER_H
#define CU_BUFFER_H

#include <cu/fwd.h>
#include <cu/ptr.h>
#include <cu/debug.h>

CU_BEGIN_DECLARATIONS

void cuP_buffer_fix_fullcap(cu_buffer_t buf, size_t fullcap);
void cuP_buffer_fix_freecap(cu_buffer_t buf, size_t freecap);

/*!\defgroup cu_buffer_h cu/buffer.h: Generic Self-Extending Buffer
 *@{\ingroup cu_mod
 *
 * This header defines a buffer for use with any data which can be moved
 * around.  It has a storage which is dynamically re-allocated as needed.
 * Within the storage is a sub-region with the current content.  The content
 * can be resized and moved upwards within the storage by moving its start and
 * end pointers.  When the content region reaches the limit of the storage, it
 * will be move to the start of the storage, possibly reallocating a bigger
 * storage region first.  To guarantee good time complexity, the storage is
 * extended in geometric progression, and the storage is kept big enough that
 * the content is moved at most once on the average.
 */

struct cu_buffer_s
{
    void *storage_start;
    void *content_start;
    void *content_end;
    void *storage_end;
};

/*!Returns an empty buffer with initial capacity \a init_cap. */
cu_buffer_t cu_buffer_new(size_t init_cap);

/*!Initialise \a buf to an empty buffer of with initial capacity
 * \a init_cap. */
void cu_buffer_init(cu_buffer_t buf, size_t init_cap);

/*!Set the content of \a buf to the empty sequence aligned to the start of the
 * storage. */
CU_SINLINE void cu_buffer_clear(cu_buffer_t buf)
{ buf->content_start = buf->content_end = buf->storage_start; }

/*!Initialise \a buf with the data of \a buf_drop, and invalidate
 * \a buf_drop. */
void cu_buffer_init_drop(cu_buffer_t buf, cu_buffer_t buf_drop);

/*!Swap the states of \a buf0 and \a buf1. */
void cu_buffer_swap(cu_buffer_t buf0, cu_buffer_t buf1);

/*!The start of the current buffer storage. */
CU_SINLINE void *
cu_buffer_storage_start(cu_buffer_t buf)
{ return buf->storage_start; }

/*!The end of the current buffer storage. */
CU_SINLINE void *
cu_buffer_storage_end(cu_buffer_t buf)
{ return buf->storage_end; }

/*!The number of bytes from the start of the buffer contents to the end of the
 * end of the current buffer storage. */
CU_SINLINE size_t
cu_buffer_fullcap(cu_buffer_t buf)
{ return cu_ptr_diff(buf->storage_end, buf->content_start); }

/*!Makes sure there is at least \a fullcap bytes allocated after the start of
 * the buffer content.  This is done either by moving the content to the start
 * of the buffer or by allocating a bigger chunck of memory. */
CU_SINLINE void
cu_buffer_extend_fullcap(cu_buffer_t buf, size_t fullcap)
{
    if (cu_ptr_add(buf->content_start, fullcap) > buf->storage_end)
	cuP_buffer_fix_fullcap(buf, fullcap);
}

/*!The number of bytes allocated after the current contents. */
CU_SINLINE size_t
cu_buffer_freecap(cu_buffer_t buf)
{ return cu_ptr_diff(buf->storage_end, buf->content_end); }

/*!Makes sure there is at least \a freecap bytes allocated after the end of the
 * buffer content. */
CU_SINLINE void
cu_buffer_extend_freecap(cu_buffer_t buf, size_t freecap)
{
    if (cu_ptr_add(buf->content_end, freecap) > buf->storage_end)
	cuP_buffer_fix_freecap(buf, freecap);
}

/*!Unconditionally realign content to start of storage. */
void cu_buffer_force_realign(cu_buffer_t buf);

/*!Realign content to start of storage if the displacement is larger than the
 * content size. */
void cu_buffer_maybe_realign(cu_buffer_t buf);

/*!Pointer to the start of the content of the buffer. */
CU_SINLINE void *
cu_buffer_content_start(cu_buffer_t buf)
{ return buf->content_start; }

/*!Pointer to the end of the content of the buffer. */
CU_SINLINE void *
cu_buffer_content_end(cu_buffer_t buf)
{ return buf->content_end; }

/*!The size in bytes of the buffer contents. */
CU_SINLINE size_t
cu_buffer_content_size(cu_buffer_t buf)
{ return cu_ptr_diff(buf->content_end, buf->content_start); }

/*!Sets the start of content pointer.
 * \pre \a start must be within the buffer, and less than the content end. */
CU_SINLINE void
cu_buffer_set_content_start(cu_buffer_t buf, void *start)
{
    cu_debug_assert(buf->storage_start <= start && start <= buf->content_end);
    buf->content_start = start;
}

/*!Increment the start of content by \a incr bytes, which in essence indicates
 * that the corresponding content is consumed, and the related buffer space
 * re-used.
 * \pre \a incr must be less than or equal to the content size. */
CU_SINLINE void
cu_buffer_incr_content_start(cu_buffer_t buf, size_t incr)
{
    buf->content_start = cu_ptr_add(buf->content_start, incr);
    cu_debug_assert(buf->content_start <= buf->content_end);
}

/*!Sets the end of content pointer.
 * \pre \a end must be within the buffer and after the content start. */
CU_SINLINE void
cu_buffer_set_content_end(cu_buffer_t buf, void *end)
{
    cu_debug_assert(buf->content_start <= end && end <= buf->storage_end);
    buf->content_end = end;
}

/*!Inclement the end of content by \a incr bytes, which in essence indicates
 * that the corresponding content is produced.  This function extends the
 * buffer capacity if needed. */
void cu_buffer_incr_content_end(cu_buffer_t buf, size_t incr);

/*!Sets the size of the content of \a buf.  This function extends the buffer
 * capacity if needed. */
void cu_buffer_resize_content(cu_buffer_t buf, size_t size);

void *cu_buffer_produce(cu_buffer_t buf, size_t incr);

/*!@}*/
CU_END_DECLARATIONS

#endif
