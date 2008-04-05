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

#include <cufo/stream.h>
#include <cu/memory.h>
#include <cu/diag.h>
#include <cu/wchar.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define BUFFER(fos) cu_to(cu_buffer, fos)
#define FDSTREAM(fos) cu_from(cufo_fdstream, cufo_stream, fos)

struct iconv_info_s
{
    iconv_t cd;
    unsigned int wr_scale;
};

typedef struct cufo_fdstream_s *cufo_fdstream_t;
struct cufo_fdstream_s
{
    cu_inherit (cufo_stream_s);
    int fd;
    struct iconv_info_s iconv_info[2]; /* = { multibyte_iconv, wide_iconv } */
};

static void
fdtarget_flush(cufo_stream_t fos, cu_bool_t must_clear)
{
    char *src_buf = cu_buffer_content_start(BUFFER(fos));
    size_t src_size = cu_buffer_content_size(BUFFER(fos));
    char *wr_buf;
    size_t wr_size;
    ssize_t sz;
    struct iconv_info_s *iconv_info;
    iconv_info = &cu_from(cufo_fdstream, cufo_stream, fos)
	->iconv_info[fos->is_wide];
    if (iconv_info->cd == NULL) {
	wr_buf = src_buf;
	wr_size = src_size;
	cu_buffer_clear(BUFFER(fos));
    } else {
	char *wr_cur;
	size_t wr_lim;
	size_t cz;
	wr_lim = wr_size = iconv_info->wr_scale*src_size;
	wr_cur = wr_buf = cu_salloc(wr_lim);
	cz = iconv(iconv_info->cd, &src_buf, &src_size, &wr_cur, &wr_lim);
	if (cz == (size_t)-1) {
	    switch (errno) {
		case E2BIG:
		    cu_bugf("Unexpected insufficient space in output buffer.");
		    break;
		case EILSEQ:
		    cu_errf("Invalid multibyte sequence.");
		    abort();
		    break;
		case EINVAL:
		    cu_errf("Incomplete multibyte sequence.");
		    break;
		default:
		    cu_bug_unreachable();
	    }
	}
	wr_size -= wr_lim;
	cu_buffer_set_content_start(BUFFER(fos), src_buf);
    }
    while (wr_size > 0) {
	sz = write(FDSTREAM(fos)->fd, wr_buf, wr_size);
	if (sz == (ssize_t)-1) {
	    /* TODO: Report error. */
	    cu_errf("Error writing to file descriptor %d.", FDSTREAM(fos)->fd);
	    break;
	}
	wr_size -= sz;
    }
}

static void
fdtarget_enter(cufo_stream_t fos, cufo_tag_t tag, va_list va)
{
    /* Nothing to do. */
}

static void
fdtarget_leave(cufo_stream_t fos, cufo_tag_t tag)
{
    /* Nothing to do. */
}

struct cufo_target_s cufoP_fdtarget = {
    .flush = fdtarget_flush,
    .enter = fdtarget_enter,
    .leave = fdtarget_leave,
};

cufo_stream_t
cufo_stream_new_fd(int fd, char const *encoding)
{
    cufo_fdstream_t fos = cu_gnew(struct cufo_fdstream_s);
    cufo_stream_init(cu_to(cufo_stream, fos), &cufoP_fdtarget);
    fos->fd = fd;
    if (encoding && strcmp(encoding, "UTF-8") == 0) {
	fos->iconv_info[0].cd = iconv_open(encoding, "UTF-8");
	fos->iconv_info[0].wr_scale = 4;
	fos->iconv_info[1].cd = iconv_open(encoding, cu_wchar_encoding);
	fos->iconv_info[1].wr_scale = 6;
    } else {
	fos->iconv_info[0].cd = NULL;
	fos->iconv_info[1].cd = iconv_open("UTF-8", cu_wchar_encoding);
	fos->iconv_info[1].wr_scale = 6;
    }
    return cu_to(cufo_stream, fos);
}
