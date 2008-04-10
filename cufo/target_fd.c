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
#include <cu/data_seq.h>
#include <cu/memory.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

typedef struct fdsink_s *fdsink_t;
struct fdsink_s {
    cu_inherit (cu_data_sink_s);
    int fd;
    cu_bool_t do_close;
};

#define FDSINK(sink) cu_from(fdsink, cu_data_sink, sink)

static size_t
fdsink_write(cu_data_sink_t sink, void const *arr, size_t len)
{
    return write(FDSINK(sink)->fd, arr, len);
}

cu_word_t
fdsink_control(cu_data_sink_t sink, int cmd, va_list va)
{
    switch (cmd) {
	case CU_DATA_CONTROL_FINISH:
	    if (FDSINK(sink)->do_close)
		close(FDSINK(sink)->fd);
	    return 0;
	case CU_DATA_CONTROL_DISCARD:
	    if (FDSINK(sink)->do_close)
		close(FDSINK(sink)->fd);
	    return 0;
	default:
	    return CU_DATA_STATUS_UNIMPL;
    }
}

cufo_stream_t
cufo_open_fd(char const *encoding, int fd, cu_bool_t do_close)
{
    fdsink_t sink;
    sink = cu_gnew(struct fdsink_s);
    cu_data_sink_init(cu_to(cu_data_sink, sink), fdsink_write, fdsink_control);
    sink->fd = fd;
    sink->do_close = do_close;
    return cufo_open_sink(encoding, cu_to(cu_data_sink, sink));
}

cufo_stream_t
cufo_open_file(char const *encoding, char const *path)
{
    int fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, 0666);
    return cufo_open_fd(encoding, fd, cu_true);
}
