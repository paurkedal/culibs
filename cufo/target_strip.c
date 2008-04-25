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
#include <cu/dsink.h>

#define BUFFER(fos) cu_to(cu_buffer, fos)
#define TS_STREAM(fos) cu_from(cufo_sinkstream, cufo_stream, fos)

typedef struct cufo_sinkstream_s *cufo_sinkstream_t;
struct cufo_sinkstream_s
{
    cu_inherit (cufo_stream_s);
    cu_dsink_t sink;
};

static size_t
ts_write(cufo_stream_t fos, void const *wr_buf, size_t wr_size)
{
    size_t wz = cu_dsink_write(TS_STREAM(fos)->sink, wr_buf, wr_size);
    if (wz == (size_t)-1) {
	cu_errf("Error writing to sink.");
	cufo_flag_error(fos);
	return 0;
    } else
	return wz;
}

static void
ts_enter(cufo_stream_t fos, cufo_tag_t tag, va_list va)
{
    /* Nothing to do. */
}

static void
ts_leave(cufo_stream_t fos, cufo_tag_t tag)
{
    /* Nothing to do. */
}

static void *
ts_close(cufo_stream_t fos)
{
    return cu_dsink_finish(TS_STREAM(fos)->sink);
}

static void
ts_flush(cufo_stream_t fos)
{
    cu_dsink_flush(TS_STREAM(fos)->sink);
}

struct cufo_target_s cufoP_target_sink = {
    .write = ts_write,
    .enter = ts_enter,
    .leave = ts_leave,
    .close = ts_close,
    .flush = ts_flush,
};

cufo_stream_t
cufo_open_strip_sink(char const *encoding, cu_dsink_t sink)
{
    cufo_sinkstream_t fos = cu_gnew(struct cufo_sinkstream_s);
    if (cufo_stream_init(cu_to(cufo_stream, fos), encoding,
			 &cufoP_target_sink)) {
	fos->sink = sink;
	return cu_to(cufo_stream, fos);
    }
    else
	return NULL;
}
