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
#include <cu/size.h>
#include <cu/ptr.h>
#include <cu/memory.h>
#include <cu/str.h>
#include <cu/wstring.h>
#include <string.h>

#define MEMSOURCE(src) cu_from(_memsource, cutext_source, src)

struct _memsource
{
    cu_inherit (cutext_source);
    void const *data;
    size_t size;
    char const *encoding;
};

static size_t
_memsource_read(cutext_source_t src, void *buf, size_t size)
{
    size_t skip = cu_size_min(size, MEMSOURCE(src)->size);
    void const *data = MEMSOURCE(src)->data;
    MEMSOURCE(src)->data = (char const *)data + skip;
    MEMSOURCE(src)->size -= skip;
    if (buf)
	memcpy(buf, data, skip);
    return skip;
}

static void const *
_memsource_look(cutext_source_t src, size_t size, size_t *size_out)
{
    *size_out = MEMSOURCE(src)->size;
    return MEMSOURCE(src)->data;
}

static cu_box_t
_memsource_info(cutext_source_t src, cutext_source_info_key_t key)
{
    switch (key) {
	case CUTEXT_SOURCE_INFO_ENCODING:
	    return cu_box_ptr(cutext_source_info_encoding_t,
			      MEMSOURCE(src)->encoding);
	default:
	    return cutext_source_default_info(src, key);
    }
}

static struct cutext_source_descriptor _memsource_descr = {
    .read = _memsource_read,
    .look = _memsource_look,
    .close = cutext_source_noop_close,
    .subsource = cutext_source_no_subsource,
    .info = _memsource_info,
};

cutext_source_t
cutext_source_new_mem(char const *encoding, void const *data, size_t size)
{
    struct _memsource *msrc = cu_gnew(struct _memsource);
    msrc->data = data;
    msrc->size = size;
    msrc->encoding = encoding;
    cutext_source_init(cu_to(cutext_source, msrc), &_memsource_descr);
    return cu_to(cutext_source, msrc);
}

cutext_source_t
cutext_source_new_cstr(char const *cstr)
{
    return cutext_source_new_mem("utf-8", cstr, strlen(cstr));
}

cutext_source_t
cutext_source_new_str(cu_str_t str)
{
    return cutext_source_new_mem("utf-8", cu_str_charr(str), cu_str_size(str));
}

cutext_source_t
cutext_source_new_wstring(cu_wstring_t wstr)
{
    return cutext_source_new_mem(cu_wchar_encoding,
				 cu_wstring_array(wstr),
				 cu_wstring_length(wstr) * sizeof(cu_wchar_t));
}
