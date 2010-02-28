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
#include <cu/memory.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define FDSOURCE(src) cu_from(_fdsource, cutext_source, src)

struct _fdsource
{
    cu_inherit (cutext_source);
    char const *encoding;
    int fd;
};

static size_t
_fdsource_read(cutext_source_t src, void *dst_data, size_t dst_size)
{
    return (size_t)read(FDSOURCE(src)->fd, dst_data, dst_size);
}

static void
_fdsource_close(cutext_source_t src)
{
    close(FDSOURCE(src)->fd);
}

static cu_box_t
_fdsource_info(cutext_source_t src, cutext_source_info_key_t key)
{
    struct _fdsource *fsrc = FDSOURCE(src);
    switch (key) {
	case CUTEXT_SOURCE_INFO_ENCODING:
	    return cu_box_ptr(cutext_source_info_encoding_t, fsrc->encoding);
	default:
	    return cutext_source_default_info(src, key);
    }
}

struct cutext_source_descriptor _fdsource_descr_noclose = {
    .read = _fdsource_read,
    .look = NULL,
    .close = cutext_source_noop_close,
    .subsource = cutext_source_no_subsource,
    .info = _fdsource_info,
};
struct cutext_source_descriptor _fdsource_descr_close = {
    .read = _fdsource_read,
    .look = NULL,
    .close = _fdsource_close,
    .subsource = cutext_source_no_subsource,
    .info = _fdsource_info,
};

cutext_source_t
cutext_source_fdopen(char const *encoding, int fd, cu_bool_t close_fd)
{
    struct _fdsource *fsrc = cu_gnew(struct _fdsource);
    fsrc->encoding = encoding;
    fsrc->fd = fd;
    cutext_source_init(cu_to(cutext_source, fsrc),
		       close_fd ? &_fdsource_descr_close
				: &_fdsource_descr_noclose);
    return cu_to(cutext_source, fsrc);
}

cutext_source_t
cutext_source_fopen(char const *encoding, char const *path)
{
    int fd = open(path, O_RDONLY);
    if (fd == -1)
	return NULL;
    return cutext_source_fdopen(encoding, fd, cu_true);
}
