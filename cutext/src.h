/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2004--2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUTEXT_SRC_H
#define CUTEXT_SRC_H

#include <cutext/fwd.h>
#include <cu/clos.h>
#include <cu/buffer.h>
#include <cu/debug.h>

CU_BEGIN_DECLARATIONS

typedef cu_clop(cutext_producer_t, cutext_status_t, void **p, size_t size);

struct cutext_src
{
    struct cu_buffer buf;
    cutext_producer_t produce;
};

void cutext_src_init(cutext_src_t, cutext_producer_t);
cutext_src_t cutext_src_new(cutext_producer_t);

void cutext_src_init_grab(cutext_src_t src_init, cutext_src_t src_drop);
cutext_src_t cutext_src_new_grab(cutext_src_t src_drop);

/* Internal */
cutext_status_t cutextP_src_lookahead(cutext_src_t src, size_t size);

CU_SINLINE cutext_status_t
cutext_src_lookahead(cutext_src_t src, size_t size)
{
    if (cu_ptr_diff(src->buf.content_end, src->buf.content_start) < size)
	return cutextP_src_lookahead(src, size);
    else
	return cutext_status_ok;
}

CU_SINLINE void *
cutext_src_data_start(cutext_src_t src) { return src->buf.content_start; }

CU_SINLINE void *
cutext_src_data_end(cutext_src_t src) { return src->buf.content_end; }

CU_SINLINE size_t
cutext_src_data_size(cutext_src_t src)
{
    return cu_ptr_diff(src->buf.content_end, src->buf.content_start);
}

CU_SINLINE void
cutext_src_advance(cutext_src_t src, size_t size)
{
    src->buf.content_start = cu_ptr_add(src->buf.content_start, size);
    cu_debug_assert(src->buf.content_start <= src->buf.content_end);
}

cutext_producer_t cutext_producer_new_charr(char const *arr, size_t size);
cutext_producer_t cutext_producer_new_read(int fd);
cutext_producer_t cutext_producer_new_iconv(cutext_src_t src,
					    cutext_encoding_t src_chenc,
					    cutext_encoding_t self_chenc);

char const *cutext_encoding_name(cutext_encoding_t chenc);
char const *cutextP_encoding_name(cutext_encoding_t chenc);
cutext_encoding_t cutext_encoding_by_name(char const *cstr);

/* Detect character encoding of 'src', assuming it is one of UTF-8,
 * UTF-16, UTF-16LE, UCS-4, or UCS-4LE, and that the sequence starts
 * with either a byte order mark or an ASCII character.  UCS-2 and
 * UCS-2LE will be wrongly detected as UTF-16 and UTF-16LE rsp. */
cutext_encoding_t cutext_src_detect_chenc(cutext_src_t src);

/* Backwards compatibility. */
#define cutext_src_cct		cutext_src_init
#define cutext_src_cct_move	cutext_src_init_grab
#define cutext_src_new_move	cutext_src_new_grab

CU_END_DECLARATIONS

#endif
