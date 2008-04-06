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

#include <cufo/target_fd.h>
#include <cufo/stream.h>
#include <cu/memory.h>
#include <cu/diag.h>
#include <cu/wchar.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFER(fos) cu_to(cu_buffer, fos)
#define FDSTREAM(fos) cu_from(cufo_fdstream, cufo_stream, fos)

static void
target_fd_flush(cufo_stream_t fos, cu_bool_t must_clear)
{
    char *src_buf = cu_buffer_content_start(BUFFER(fos));
    size_t src_size = cu_buffer_content_size(BUFFER(fos));
    char *wr_buf;
    size_t wr_size;
    ssize_t sz;
    struct cufo_fdconvinfo_s *convinfo;
    convinfo = &cu_from(cufo_fdstream, cufo_stream, fos)
	->convinfo[fos->is_wide];
    if (convinfo->cd == NULL) {
	wr_buf = src_buf;
	wr_size = src_size;
	cu_buffer_clear(BUFFER(fos));
    } else {
	char *wr_cur;
	size_t wr_lim;
	size_t cz;
	wr_lim = wr_size = convinfo->wr_scale*src_size;
	wr_cur = wr_buf = cu_salloc(wr_lim);
	cz = iconv(convinfo->cd, &src_buf, &src_size, &wr_cur, &wr_lim);
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
	    cufo_flag_error(fos);
	    break;
	}
	wr_size -= sz;
    }
}

static void
target_fd_enter(cufo_stream_t fos, cufo_tag_t tag, va_list va)
{
    /* Nothing to do. */
}

static void
target_fd_leave(cufo_stream_t fos, cufo_tag_t tag)
{
    /* Nothing to do. */
}

static void *
target_fd_close(cufo_stream_t fos)
{
    if (FDSTREAM(fos)->do_close)
	close(FDSTREAM(fos)->fd);
    return NULL;
}

struct cufo_target_s cufoP_target_fd = {
    .flush = target_fd_flush,
    .enter = target_fd_enter,
    .leave = target_fd_leave,
    .close = target_fd_close,
};

void
cufo_fdstream_init(cufo_fdstream_t fos, int fd, char const *encoding)
{
    cufo_stream_init(cu_to(cufo_stream, fos), &cufoP_target_fd);
    fos->fd = fd;
    fos->do_close = cu_false;
    if (encoding && strcmp(encoding, "UTF-8") == 0) {
	fos->convinfo[0].cd = iconv_open(encoding, "UTF-8");
	fos->convinfo[0].wr_scale = 4;
	fos->convinfo[1].cd = iconv_open(encoding, cu_wchar_encoding);
	fos->convinfo[1].wr_scale = 6;
    } else {
	fos->convinfo[0].cd = NULL;
	fos->convinfo[1].cd = iconv_open("UTF-8", cu_wchar_encoding);
	fos->convinfo[1].wr_scale = 6;
    }
}

cufo_stream_t
cufo_open_fd(int fd, char const *encoding)
{
    cufo_fdstream_t fos = cu_gnew(struct cufo_fdstream_s);
    cufo_fdstream_init(fos, fd, encoding);
    return cu_to(cufo_stream, fos);
}

cufo_stream_t
cufo_open_file(char const *path, char const *encoding)
{
    int fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, 0666);
    cufo_stream_t fos = cufo_open_fd(fd, encoding);
    FDSTREAM(fos)->do_close = cu_true;
    return fos;
}
