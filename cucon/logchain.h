/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2003--2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUCON_LOGCHAIN_H
#define CUCON_LOGCHAIN_H

#include <cucon/fwd.h>
#include <assert.h>

#ifdef CU_NDEBUG
#  define CUCON_LOGCHAIN_NDEBUG 1
#endif

CU_BEGIN_DECLARATIONS
/*!\defgroup cucon_logchain cucon/logchain.h: Chained chunks of memory with logarithmic time access
 * @{\ingroup cucon_linear_mod
 * \deprecated Use \ref cucon_frame_h */

#define cuconP_LOGCHAIN_LINK(logchain, i) (((void **)(logchain))[-i-1])

/* An unsigned integer type which can hold the depth of a logchain. */
typedef unsigned int cucon_logchain_depth_t;

/* Create a logchain of size 'size' as a root logchain if 'logchain_prev' is
 * NULL, else as a continuation of 'logchain_prev'. */
void *cucon_logchain_galloc(cucon_logchain_depth_t depth, size_t size,
			    void *lch_prev);

void *cuconP_logchain_find_nontrivial(void *, size_t, size_t);

/* Return the logchain at 'dst_depth' given that of 'src_lch' at depth
 * 'src_depth'.  Pre: 'src_depth ≤ dst_depth' */
CU_SINLINE void *
cucon_logchain_find(void *src_lch, cucon_logchain_depth_t src_depth,
		  cucon_logchain_depth_t dst_depth)
{
    if (src_depth == dst_depth)
	return src_lch;
    else
	return cuconP_logchain_find_nontrivial(src_lch, src_depth, dst_depth);
}

/* Equivalent to 'cucon_logchain_find(logchain, src_depth, src_depth - 1)'. */
CU_SINLINE void *
cucon_logchain_prev(void *logchain, cucon_logchain_depth_t src_depth)
{
    assert(src_depth > 0);
    return cuconP_LOGCHAIN_LINK(logchain, 0);
}

/*!@}*/
CU_END_DECLARATIONS

#endif
