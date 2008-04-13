/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2005--2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CU_ALGO_H
#define CU_ALGO_H

#include <cu/fwd.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cu_algo_h cu/algo.h: Supportive definitions for various algorithms
 * @{\ingroup cu_util_mod */

/*!Enumeration of partial order relations. */
typedef enum {
    cu_order_none,	/*!< no ordering relation applies */
    cu_order_lt,	/*!< less-than ordering */
    cu_order_gt,	/*!< greater-than ordering */
    cu_order_eq,	/*!< equal-to ordering */
} cu_order_t;

CU_SINLINE void
cu_ptr_swap(cu_ptr_ptr_t p0, cu_ptr_ptr_t p1)
{
    void *pS = *(void **)p0;
    *(void **)p0 = *(void **)p1;
    *(void **)p1 = pS;
}

CU_SINLINE void
cu_ptr_rotl(cu_ptr_ptr_t p0, cu_ptr_ptr_t p1, cu_ptr_ptr_t p2)
{
    void *pS = *(void **)p0;
    *(void **)p0 = *(void **)p1;
    *(void **)p1 = *(void **)p2;
    *(void **)p2 = pS;
}

/*!@}*/
CU_END_DECLARATIONS

#endif
