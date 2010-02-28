/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2010  Petter Urkedal <paurkedal@eideticdew.org>
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

#include <cutext/source.h>
#include <cu/inherit.h>
#include <cu/buffer.h>
#include <cu/memory.h>
#include <string.h>

#define BUFSOURCE(src) cu_from(_bufsource, cutext_source, src)

struct _bufsource
{
    cu_inherit (cutext_source);
    cutext_source_t subsrc;
    struct cu_buffer buf;
};

static size_t
_bufsource_read(cutext_source_t src, void *dst_data, size_t dst_size)
{
    struct _bufsource *bsrc = BUFSOURCE(src);
    size_t buf_size = cu_buffer_content_size(&bsrc->buf);
    void *buf_data = cu_buffer_content_start(&bsrc->buf);
    if (buf_size >= dst_size) {
	if (dst_data)
	    memcpy(dst_data, buf_data, dst_size);
	cu_buffer_incr_content_start(&bsrc->buf, dst_size);
	return dst_size;
    }
    else {
	size_t sub_size;
	if (dst_data)
	    memcpy(dst_data, buf_data, buf_size);
	cu_buffer_clear(&bsrc->buf);
	sub_size = cutext_source_read(bsrc->subsrc,
				      dst_data ? cu_ptr_add(dst_data, buf_size)
					       : NULL,
				      dst_size - buf_size);
	return buf_size + sub_size;
    }
}

static void const *
_bufsource_look(cutext_source_t src, size_t req_size, size_t *act_size)
{
    struct _bufsource *bsrc = BUFSOURCE(src);
    size_t buf_size = cu_buffer_content_size(&bsrc->buf);
    if (buf_size < req_size) {
	size_t sub_size, add_size = req_size - buf_size;
	cu_buffer_extend_freecap(&bsrc->buf, add_size);
	sub_size = cutext_source_read(bsrc->subsrc,
				      cu_buffer_content_end(&bsrc->buf),
				      add_size);
	cu_buffer_incr_content_end(&bsrc->buf, sub_size);
	*act_size = buf_size + sub_size;
    }
    else
	*act_size = req_size;
    return cu_buffer_content_start(&bsrc->buf);
}

static void
_bufsource_close(cutext_source_t src)
{
    struct _bufsource *bsrc = BUFSOURCE(src);
    cutext_source_close(bsrc->subsrc);
}

static cutext_source_t
_bufsource_subsource(cutext_source_t src)
{
    return BUFSOURCE(src)->subsrc;
}

static cu_box_t
_bufsource_info(cutext_source_t src, cutext_source_info_key_t key)
{
    return cutext_source_info_inherit(src, key, BUFSOURCE(src)->subsrc);
}

static struct cutext_source_descriptor _bufsource_descr = {
    .read = _bufsource_read,
    .look = _bufsource_look,
    .close = _bufsource_close,
    .subsource = _bufsource_subsource,
    .info = _bufsource_info,
};

cutext_source_t
cutext_source_stack_buffer(cutext_source_t subsrc)
{
    struct _bufsource *bsrc = cu_gnew(struct _bufsource);
    bsrc->subsrc = subsrc;
    cu_buffer_init(&bsrc->buf, 16);
    cutext_source_init(cu_to(cutext_source, bsrc), &_bufsource_descr);
    return cu_to(cutext_source, bsrc);
}
