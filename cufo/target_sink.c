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

#include <cufo/target_sink.h>
#include <cufo/stream.h>
#include <cu/memory.h>
#include <cu/diag.h>
#include <cu/wchar.h>
#include <cu/data_seq.h>
#include <string.h>
#include <errno.h>

#define BUFFER(fos) cu_to(cu_buffer, fos)
#define SINKSTREAM(fos) cu_from(cufo_sinkstream, cufo_stream, fos)

static void
target_sink_flush(cufo_stream_t fos, cu_bool_t must_clear)
{
    char *src_buf = cu_buffer_content_start(BUFFER(fos));
    size_t src_size = cu_buffer_content_size(BUFFER(fos));
    char *wr_buf;
    size_t wr_size;
    size_t sz;
    struct cufo_sinkconvinfo_s *convinfo;
    convinfo = &cu_from(cufo_sinkstream, cufo_stream, fos)
	->convinfo[fos->is_wide];
    if (convinfo->cd == NULL) {
	wr_buf = src_buf;
	wr_size = src_size;
	cu_buffer_clear(BUFFER(fos));
    } else {
	char *wr_cur;
	size_t wr_lim;
	size_t cz;
	wr_lim = wr_size = convinfo->wr_scale*(src_size + 1);
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
	sz = cu_data_sink_write(SINKSTREAM(fos)->sink, wr_buf, wr_size);
	if (sz == (size_t)-1) {
	    /* TODO: Report error. */
	    cu_errf("Error writing to sink.");
	    cufo_flag_error(fos);
	    break;
	}
	wr_size -= sz;
    }
}

static void
target_sink_enter(cufo_stream_t fos, cufo_tag_t tag, va_list va)
{
    /* Nothing to do. */
}

static void
target_sink_leave(cufo_stream_t fos, cufo_tag_t tag)
{
    /* Nothing to do. */
}

static void *
target_sink_close(cufo_stream_t fos)
{
    int i;
    for (i = 0; i < 2; ++i)
	if (SINKSTREAM(fos)->convinfo[i].cd)
	    iconv_close(SINKSTREAM(fos)->convinfo[i].cd);
    return cu_data_sink_finish(SINKSTREAM(fos)->sink);
}

struct cufo_target_s cufoP_target_sink = {
    .flush = target_sink_flush,
    .enter = target_sink_enter,
    .leave = target_sink_leave,
    .close = target_sink_close,
};

cu_bool_t
cufo_sinkstream_init(cufo_sinkstream_t fos, char const *encoding,
		     cu_data_sink_t sink)
{
    cufo_stream_init(cu_to(cufo_stream, fos), &cufoP_target_sink);
    fos->sink = sink;

    /* Allocate multi-byte descriptor. */
    if (!encoding || strcmp(encoding, "UTF-8") == 0) {
	encoding = "UTF-8";
	fos->convinfo[0].cd = NULL;
    }
    else {
	fos->convinfo[0].cd = iconv_open(encoding, "UTF-8");
	fos->convinfo[0].wr_scale = 4;
	if (fos->convinfo[0].cd == (iconv_t)-1) {
	    cu_errf("iconv_open(\"%s\", \"UTF-8\"): %s",
		    encoding, strerror(errno));
	    return cu_false;
	}
    }

    /* Allocate wide-char descriptor. */
    if (strcmp(encoding, cu_wchar_encoding) == 0)
	fos->convinfo[1].cd = NULL;
    else {
	fos->convinfo[1].cd = iconv_open(encoding, cu_wchar_encoding);
	fos->convinfo[1].wr_scale = 2;
	if (fos->convinfo[1].cd == (iconv_t)-1) {
	    if (fos->convinfo[0].cd)
		iconv_close(fos->convinfo[0].cd);
	    cu_errf("iconv_open(\"%s\", \"%s\"): %s",
		    encoding, cu_wchar_encoding, strerror(errno));
	    return cu_false;
	}
    }

    return cu_true;
}

cufo_stream_t
cufo_open_sink(char const *encoding, cu_data_sink_t sink)
{
    cufo_sinkstream_t fos = cu_gnew(struct cufo_sinkstream_s);
    if (cufo_sinkstream_init(fos, encoding, sink))
	return cu_to(cufo_stream, fos);
    else
	return NULL;
}
