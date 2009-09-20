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

#ifndef CU_UTIL_H
#define CU_UTIL_H

#include <cu/fwd.h>

CU_BEGIN_DECLARATIONS
/** \defgroup cu_util_h cu/util.h: General Utilities
 ** @{ \ingroup cu_util_mod */

#define CU_SWAP(value_t, x, y)						\
    do {								\
	value_t cuL_tmp = (x);						\
	(x) = (y);							\
	(y) = cuL_tmp;							\
    } while (0)

#define CU_RETURN_UNLESS_FIRST_CALL					\
    do {								\
	static int cuL_done = 0;					\
	if (cuL_done)							\
	    return;							\
	cuL_done = 1;							\
    } while (0)

/** Assuming \a ptr points to \a field in \a type, returns a pointer to the
 ** whole \a type.  Uses <tt>offsetof(\a type, \a field)</tt>. */
#define cu_ptr_context(type, field, ptr) \
    ((type *)((char *)(ptr) - offsetof(type, field)))


/* Alignment
 * ========= */

union cuP_alignment_u {
    double cuP_dummy_0;
    intmax_t cuP_dummy_1;
    void *cuP_dummy_2;
    void (*cuP_dummy_3)();
};
struct cuP_alignment_test_s {
    char ch;
    union cuP_alignment_u aligner;
};
#define CU_FULL_ALIGNMENT						\
    (sizeof(struct cuP_alignment_test_s) - sizeof(union cuP_alignment_u))

#define cu_aligned_floor(size)						\
    ((size)/CU_FULL_ALIGNMENT*CU_FULL_ALIGNMENT)
#define cu_aligned_ceil(size)						\
    (((size) + CU_FULL_ALIGNMENT - 1)/CU_FULL_ALIGNMENT*CU_FULL_ALIGNMENT)
#define CU_ALIGNED_SIZEOF(type) cu_aligned_ceil(sizeof(type))
#define CU_ALIGNED_PTR_END(ptr)						\
    ((void *)((char*)(ptr) + cu_aligned_ceil(sizeof(*ptr))))
#define CU_ALIGNED_PTR_FROM_END(ptr_t, ptr)				\
    ((ptr_t)((char*)(ptr) - cu_aligned_ceil(sizeof(*(ptr_t)NULL))))
#define CU_ALIGNED_MARG_END(ptr_t, ptr)					\
    CU_ALIGNED_PTR_END(CU_MARG(ptr_t, ptr))

/** @} */
CU_END_DECLARATIONS

#endif
