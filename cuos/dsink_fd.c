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

#include <cuos/dsink.h>
#include <cu/inherit.h>
#include <cu/dsink.h>
#include <cu/memory.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

typedef struct fd_sink_s *fd_sink_t;
struct fd_sink_s
{
    cu_inherit (cu_dsink_s);
    int fd;
};

#define FD_SINK(sink) cu_from(fd_sink, cu_dsink, sink)

size_t
fd_sink_write(cu_dsink_t sink, void const *data, size_t size)
{
    return write(FD_SINK(sink)->fd, data, size);
}

cu_word_t
fd_sink_control_if_not_close(cu_dsink_t sink, int cmd, va_list va)
{
    return CU_DSINK_ST_UNIMPL;
}

cu_word_t
fd_sink_control_if_close(cu_dsink_t sink, int cmd, va_list va)
{
    switch (cmd) {
	case CU_DSINK_FN_DISCARD:
	case CU_DSINK_FN_FINISH:
	    close(FD_SINK(sink)->fd);
	    return CU_DSINK_ST_SUCCESS;
	default:
	    return CU_DSINK_ST_UNIMPL;
    }
}

cu_dsink_t
cuos_dsink_fdopen(int fd)
{
    fd_sink_t sink = cu_gnew(struct fd_sink_s);
    cu_dsink_init(cu_to(cu_dsink, sink),
		  fd_sink_control_if_not_close, fd_sink_write);
    sink->fd = fd;
    return cu_to(cu_dsink, sink);
}

cu_dsink_t
cuos_dsink_fdopen_close(int fd)
{
    fd_sink_t sink = cu_gnew(struct fd_sink_s);
    cu_dsink_init(cu_to(cu_dsink, sink),
		  fd_sink_control_if_close, fd_sink_write);
    sink->fd = fd;
    return cu_to(cu_dsink, sink);
}

cu_dsink_t
cuos_dsink_fopen(char const *path)
{
    int fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (fd != -1)
	return cuos_dsink_fdopen_close(fd);
    else
	return NULL;
}
