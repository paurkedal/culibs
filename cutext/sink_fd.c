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

#include <cutext/sink.h>
#include <cu/inherit.h>
#include <cu/memory.h>
#include <cu/str.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/* Short write indicate error. */
#define CUCONF_SYS_WRITE_IS_CLOGFREE 1

typedef struct _fd_sink *_fd_sink_t;
struct _fd_sink
{
    cu_inherit (cutext_sink);
    int fd;
    char const *encoding;
};

#define FD_SINK(sink) cu_from(_fd_sink, cutext_sink, sink)

static size_t
_fd_sink_write(cutext_sink_t sink, void const *data, size_t size)
{
#ifdef CUCONF_SYS_WRITE_IS_CLOGFREE
    return write(FD_SINK(sink)->fd, data, size);
#else
    return write(FD_SINK(sink)->fd, data, size) == size
	? size : CU_DSINK_WRITE_ERROR;
#endif
}

static cu_box_t
_fd_sink_finish_if_close(cutext_sink_t sink)
{
    close(FD_SINK(sink)->fd);
    return cu_box_void();
}

static void
_fd_sink_discard_if_close(cutext_sink_t sink)
{
    close(FD_SINK(sink)->fd);
}

static cu_box_t
_fd_sink_info(cutext_sink_t sink, cutext_sink_info_key_t key)
{
    switch (key) {
	case CUTEXT_SINK_INFO_ENCODING:
	    return cu_box_ptr(cutext_sink_info_encoding_t,
			      FD_SINK(sink)->encoding);
	case CUTEXT_SINK_INFO_DEBUG_STATE:
	    return cu_box_ptr(cutext_sink_info_debug_state_t,
			      cu_str_new_fmt("writing to file descriptor %d",
					     FD_SINK(sink)->fd));
	default:
	    return cutext_sink_default_info(sink, key);
    }
}

static struct cutext_sink_descriptor _fd_sink_descriptor_if_not_close = {
    CUTEXT_SINK_DESCRIPTOR_DEFAULTS,
#ifdef CUCONF_SYS_WRITE_IS_CLOGFREE
    .flags = CUTEXT_SINK_FLAG_CLOGFREE,
#endif
    .write = _fd_sink_write,
    .info = _fd_sink_info
};

static struct cutext_sink_descriptor _fd_sink_descriptor_if_close = {
    CUTEXT_SINK_DESCRIPTOR_DEFAULTS,
#ifdef CUCONF_SYS_WRITE_IS_CLOGFREE
    .flags = CUTEXT_SINK_FLAG_CLOGFREE,
#endif
    .write = _fd_sink_write,
    .finish = _fd_sink_finish_if_close,
    .discard = _fd_sink_discard_if_close,
    .info = _fd_sink_info
};

cutext_sink_t
cutext_sink_fdopen(char const *encoding, int fd, cu_bool_t close_fd)
{
    cutext_sink_descriptor_t descr = close_fd
	? &_fd_sink_descriptor_if_close
	: &_fd_sink_descriptor_if_not_close;
    _fd_sink_t sink = cu_gnew(struct _fd_sink);
    cutext_sink_init(cu_to(cutext_sink, sink), descr);
    sink->fd = fd;
    sink->encoding = encoding;
    return cu_to(cutext_sink, sink);
}

cutext_sink_t
cutext_sink_fopen(char const *encoding, char const *path)
{
    int fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (fd != -1)
	return cutext_sink_fdopen(encoding, fd, cu_true);
    else
	return NULL;
}
