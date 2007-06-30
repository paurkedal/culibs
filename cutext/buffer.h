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

#ifndef CUTEXT_BUFFER_H
#define CUTEXT_BUFFER_H

#include <cutext/fwd.h>

CU_BEGIN_DECLARATIONS

typedef enum {
    cutext_status_ok,
    cutext_status_buffer_too_small,
    cutext_status_eos,
    cutext_status_sync,
    cutext_status_error,
} cutext_status_t;

struct cutext_buffer_s
{
    void *buf_start;
    void *data_start;
    void *data_end;
    void *buf_end;
};

void cutext_buffer_cct(cutext_buffer_t buf, size_t init_cap);
cutext_buffer_t cutext_buffer_new(size_t init_cap);

void cutext_buffer_cct_move(cutext_buffer_t buf_cct, cutext_buffer_t buf_dct);

/* Internal */
void cutextP_buffer_lookahead_fixup(cutext_buffer_t buf, size_t size);

/* Make sure [data_start, buf_end) is at least 'size' B. */
CU_SINLINE void
cutext_buffer_lookahead(cutext_buffer_t buf, size_t size)
{
    if (buf->data_start + size > buf->data_end)
	cutextP_buffer_lookahead_fixup(buf, size);
}


CU_END_DECLARATIONS

#endif
