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
#include <cu/debug.h>
#include <cu/memory.h>
#include <iconv.h>
#include <errno.h>
#include <string.h>

cu_dlog_def(_file, "dtag=cutext.source_iconv");

#define ICONVSOURCE(src) cu_from(_iconvsource, cutext_source, src)
#define BLOCK_SIZE 512

struct _iconvsource
{
    cu_inherit (cutext_source);
    cutext_source_t subsrc;
    char const *encoding;
    iconv_t cd;
};

static size_t
_iconvsource_read(cutext_source_t src, void *dst_data, size_t dst_size)
{
    struct _iconvsource *isrc = ICONVSOURCE(src);
    size_t dst_size_left = dst_size;
    cu_dlogf(_file, "Read request for %zd output bytes.", dst_size);
    if (dst_data == NULL)
	dst_data = cu_salloc(dst_size);
    while (dst_size_left > 0) {
	size_t sub_size, sub_size_left;
	void const *sub_data;
	sub_data = cutext_source_look(isrc->subsrc, dst_size_left, &sub_size);
	if (sub_size == 0) {
	    cu_dlogf(_file, "Source is empty, returning %zd.",
		     dst_size - dst_size_left);
	    return dst_size - dst_size_left;
	}
	sub_size_left = sub_size;
	if (iconv(isrc->cd,
		  (char **)&sub_data, &sub_size_left,
		  (char **)&dst_data, &dst_size_left) == (size_t)-1)
	    switch (errno) {
		case EILSEQ:
		    cu_errf("Invalid multibyte sequence.");
		    return (size_t)-1;
		case EINVAL:
		    break;
		case E2BIG:
		    cu_debug_assert(dst_size_left == 0);
		    break;
		default:
		    cu_debug_unreachable();
	    }
	cu_dlogf(_file,
		 "Scanned %zd of %zd input bytes, %zd output bytes left.",
		 sub_size - sub_size_left, sub_size, dst_size_left);
	cutext_source_read(isrc->subsrc, NULL, sub_size - sub_size_left);
    }
    cu_dlogf(_file, "Filled destination buffer, returning %zd.", dst_size);
    return dst_size;
}

static void
_iconvsource_close(cutext_source_t src)
{
    struct _iconvsource *isrc = ICONVSOURCE(src);
    iconv_close(isrc->cd);
    cutext_source_close(isrc->subsrc);
}

static cutext_source_t
_iconvsource_subsource(cutext_source_t src)
{
    return ICONVSOURCE(src)->subsrc;
}

static cu_box_t
_iconvsource_info(cutext_source_t src, cutext_source_info_key_t key)
{
    struct _iconvsource *isrc = ICONVSOURCE(src);
    switch (key) {
	case CUTEXT_SOURCE_INFO_ENCODING:
	    return cu_box_ptr(cutext_source_info_encoding_t, isrc->encoding);
	default:
	    return cutext_source_info_inherit(src, key, isrc->subsrc);
    }
}

static struct cutext_source_descriptor _iconvsource_descr = {
    .read = _iconvsource_read,
    .look = NULL,
    .close = _iconvsource_close,
    .subsource = _iconvsource_subsource,
    .info = _iconvsource_info,
};

cutext_source_t
cutext_source_stack_iconv(char const *encoding, cutext_source_t subsrc)
{
    struct _iconvsource *isrc;
    char const *sub_encoding = cutext_source_encoding(subsrc);
    if (!sub_encoding)
	cu_bugf("Can't recode source at %p with unknown encoding.", subsrc);
    if (strcmp(encoding, sub_encoding) == 0)
	return subsrc;
    isrc = cu_gnew(struct _iconvsource);
    isrc->subsrc = subsrc;
    isrc->encoding = encoding;
    isrc->cd = iconv_open(encoding, sub_encoding);
    if (isrc->cd == NULL)
	return NULL;
    cutext_source_init(cu_to(cutext_source, isrc), &_iconvsource_descr);
    return cu_to(cutext_source, isrc);
}
