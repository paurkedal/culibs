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
#include <cu/diag.h>
#include <cu/debug.h>
#include <cu/memory.h>
#include <errno.h>
#include <iconv.h>

#define IC_WRITE_BUFSIZE 2048

typedef struct ic_dsink_s *ic_dsink_t;
struct ic_dsink_s
{
    cu_inherit (cu_dsink_s);
    cu_dsink_t target_sink;
    iconv_t cd;
};

#define IC_DSINK(sink) cu_from(ic_dsink, cu_dsink, sink)

static size_t
ic_write(cu_dsink_t sink, void const *data_start, size_t data_size)
{
    char buf[IC_WRITE_BUFSIZE];
    char *src_ptr = (char *)data_start;
    size_t src_size = data_size;

    if (!data_size)
	return 0;

    for (;;) {
	char *dst_ptr = buf;
	size_t dst_cap = IC_WRITE_BUFSIZE;
	size_t st;

	st = iconv(IC_DSINK(sink)->cd, &src_ptr, &src_size, &dst_ptr, &dst_cap);
	if (dst_cap < IC_WRITE_BUFSIZE) {
	    size_t sub_st;
	    sub_st = cu_dsink_write(IC_DSINK(sink)->target_sink,
				    buf, IC_WRITE_BUFSIZE - dst_cap);
	    if (sub_st != IC_WRITE_BUFSIZE - dst_cap) {
		if (sub_st == (size_t)-1)
		    return (size_t)-1;
		else
		    cu_bugf("Write to clog-free sink did not consume all "
			    "data.");
	    }
	}
	if (st == (size_t)-1) switch (errno) {
	    case EILSEQ:
		cu_errf("Invalid character sequence during conversion.");
		return (size_t)-1;
	    case EINVAL: /* Incomplete sequence: Done. */
		return data_size - src_size;
	    case E2BIG:  /* Out of buffer space: Next round. */
		if (dst_cap == IC_WRITE_BUFSIZE)
		    cu_bugf("%d byte buffer does not fit a single character?",
			    IC_WRITE_BUFSIZE);
		break;
	}
	else {
	    cu_debug_assert(src_size == 0);
	    return data_size;
	}
    }
}

static cu_word_t
ic_control(cu_dsink_t sink, int fn, va_list va)
{
    cu_word_t res = cu_dsink_control_va(IC_DSINK(sink)->target_sink, fn, va);
    switch (fn) {
	case CU_DSINK_FN_DISCARD:
	case CU_DSINK_FN_FINISH:
	    iconv_close(IC_DSINK(sink)->cd);
	    break;
    }
    return res;
}

cu_dsink_t
cuos_dsink_open_iconv(char const *source_encoding, char const *target_encoding,
		      cu_dsink_t target_sink)
{
    ic_dsink_t sink = cu_gnew(struct ic_dsink_s);
    cu_dsink_init(cu_to(cu_dsink, sink), ic_control, ic_write);
    sink->target_sink = target_sink;
    sink->cd = iconv_open(target_encoding, source_encoding);
    if (sink->cd == (iconv_t)-1)
	return NULL;
    else
	return cu_to(cu_dsink, sink);
}
