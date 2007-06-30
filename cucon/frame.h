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

#ifndef CUCON_FRAME_H
#define CUCON_FRAME_H

#include <cucon/fwd.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cucon_frame_h cucon/frame.h: Constructive Stack
 *@{\ingroup cucon_linear_mod
 *
 * This implements a stack with constructive operations.  The time and space
 * complexity are logarithmic in the number of stack levels.
 *
 * \see cucon_stack_h
 */

CU_SINLINE void **
cuconP_frame_link_slot(void *frame, cu_logsize_fast_t i)
{
    return (void **)((char *)frame
		     - sizeof(uintptr_t)
		     - (i + 1)*sizeof(void *));
}

CU_SINLINE uintptr_t *
cuconP_frame_depth_slot(void *frame)
{ return ((uintptr_t *)frame - 1); }

/*!The depth of \a frame, where \c NULL has depth 0. */
CU_SINLINE size_t cucon_frame_depth(void *frame)
{ return frame? *cuconP_frame_depth_slot(frame) : 0; }

/*!Push a subframe of \a frame with \a size bytes of data.  Pass \a frame = \c
 * NULL to create a top level frame.  The returned pointer points into the
 * allocated memory where the user data starts. */
void *cucon_frame_push(void *frame, size_t size);

/*!The parent frame of \a frame, where \a frame is a pointer which was
 * obtained with \ref cucon_frame_push. */
CU_SINLINE void *cucon_frame_pop(void *frame)
{ return *cuconP_frame_link_slot(frame, 0); }

/*!Equivalent to the composition of \a depth calls to \ref cucon_frame_pop,
 * except that the time complexity is logarithmic in \a depth. */
void *cucon_frame_at(void *frame, size_t depth);

/*!@}*/
CU_END_DECLARATIONS

#endif
