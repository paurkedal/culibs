/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2006--2007  Petter Urkedal <urkedal@nbi.dk>
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

#include <cucon/frame.h>
#include <cu/memory.h>
#include <cu/int.h>

void *
cucon_frame_push(void *frame, size_t size)
{
    void *new_frame;
    size_t depth, new_depth;
    size_t hidden_size;
    size_t pow2_bit;
    cu_logsize_fast_t n_link;
    cu_logsize_fast_t i;

    if (!frame) {
	new_frame =
	    (char *)cu_galloc(size + sizeof(uintptr_t) + sizeof(void *))
	    + sizeof(uintptr_t) + sizeof(void *);
	*cuconP_frame_depth_slot(new_frame) = 1;
	*cuconP_frame_link_slot(new_frame, 0) = NULL;
	return new_frame;
    }
    depth = *cuconP_frame_depth_slot(frame);
    new_depth = depth + 1;
    n_link = cu_uint_log2_lowbit(new_depth) + 1;
    hidden_size = sizeof(uintptr_t) + n_link*sizeof(void *);
    new_frame = (char *)cu_galloc(size + hidden_size) + hidden_size;
    *cuconP_frame_depth_slot(new_frame) = new_depth;
    pow2_bit = 1;
    for (i = 0; i < n_link; ++i) {
	frame = cucon_frame_at(frame, pow2_bit + depth - new_depth);
	depth = new_depth - pow2_bit;
	cu_debug_assert(cucon_frame_depth(frame) == depth);
	*cuconP_frame_link_slot(new_frame, i) = frame;
	pow2_bit <<= 1;
    }
    return new_frame;
}

void *
cucon_frame_at(void *frame, size_t frame_cnt)
{
    size_t pow2_bit;
    cu_logsize_fast_t bit;
    size_t cur_depth;
    size_t dst_depth;

    if (frame_cnt == 0)
	return frame;

    cur_depth = cucon_frame_depth(frame);
    if (frame_cnt >= cur_depth)
	return NULL;

    dst_depth = cur_depth - frame_cnt;
    pow2_bit = 1;
    bit = 0;
    for (;;) {
	size_t next_depth = cur_depth & ~pow2_bit;
	if (next_depth < dst_depth)
	    break;
	else if (next_depth != cur_depth) {
	    frame = *cuconP_frame_link_slot(frame, bit);
	    cur_depth = next_depth;
	    cu_debug_assert(cucon_frame_depth(frame) == cur_depth);
	    if (cur_depth == dst_depth)
		return frame;
	}
	pow2_bit <<= 1;
	++bit;
    }
    cu_debug_assert(cucon_frame_depth(frame) == cur_depth);
    frame_cnt = cur_depth - dst_depth;
    for (;;) {
	pow2_bit >>= 1;
	--bit;
	if ((pow2_bit & frame_cnt) != 0) {
	    frame = *cuconP_frame_link_slot(frame, bit);
	    cu_debug_assert(cucon_frame_depth(frame)
			    == (cur_depth -= pow2_bit));
	    frame_cnt &= ~pow2_bit;
	    if (frame_cnt == 0) {
		cu_debug_assert(cucon_frame_depth(frame) == dst_depth);
		return frame;
	    }
	}
    }
}
