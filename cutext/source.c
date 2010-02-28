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
#include <cu/debug.h>

cu_box_t
cutext_source_info_inherit(cutext_source_t src, cutext_source_info_key_t key,
			   cutext_source_t subsrc)
{
    if (cutext_source_info_key_inherits(key))
	return cutext_source_info(subsrc, key);
    else
	return cutext_source_default_info(src, key);
}

cu_box_t
cutext_source_default_info(cutext_source_t src, cutext_source_info_key_t key)
{
    switch (key) {
	case CUTEXT_SOURCE_INFO_ENCODING:
	    return cu_box_ptr(cutext_source_info_encoding_t, NULL);
	case CUTEXT_SOURCE_INFO_TABSTOP:
	    return cu_box_int(8);
	default:
	    cu_bug_unreachable();
    }
}

size_t
cutext_source_null_read(cutext_source_t src, void *buf, size_t size)
{ return 0; }

void
cutext_source_noop_close(cutext_source_t src)
{}

cutext_source_t
cutext_source_no_subsource(cutext_source_t src)
{ return NULL; }


size_t
cutext_source_count(cutext_source_t src)
{
    size_t count = 0, n;
    while ((n = cutext_source_read(src, NULL, 1024)))
	count += n;
    return count;
}
