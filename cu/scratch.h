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

#ifndef CU_SCRATCH_H
#define CU_SCRATCH_H

#include <cu/fwd.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cu_scratch_h cu/scratch.h: Scratch Memory for Repeated Reuse
 *@{\ingroup cu_util_mod
 * This header defines a small object to manage reuse of a scratch memory
 * area.  It is mainly inteded to be defined in function bodies and used in
 * inner loops when \ref cu_salloc is not feasible.
 */

typedef struct cu_scratch *cu_scratch_t;

/*!Reusable scratch memory. */
struct cu_scratch
{
    size_t size;
    void *ptr;
};

/*!An initialiser which can be assigned in a declaration of a variable of type
 * <tt>struct \ref cu_scratch</tt>. */
#define CU_SCRATCH_INIT { 0, NULL }

/*!Alternative to \ref CU_SCRATCH_INIT. */
CU_SINLINE void
cu_scratch_init(cu_scratch_t scr) { scr->size = 0; scr->ptr = NULL; }

/*!The size of \a scr. */
CU_SINLINE size_t cu_scratch_size(cu_scratch_t scr) { return scr->size; }

/*!A pointer to the start of the scratch memory. */
CU_SINLINE void *cu_scratch_ptr(cu_scratch_t scr) { return scr->ptr; }

void cuP_scratch_alloc_min(cu_scratch_t, size_t);
void cuP_scratch_alloc_log(cu_scratch_t, size_t);
void cuP_scratch_realloc_min(cu_scratch_t, size_t);
void cuP_scratch_realloc_log(cu_scratch_t, size_t);

/*!Extend \a scr to \a size unless it already is at least that big, and
 * return a pointer to it's memory.  Does not preserve the contents. */
CU_SINLINE void *
cu_scratch_alloc_min(cu_scratch_t scr, size_t size)
{
    if (scr->size < size)
	cuP_scratch_alloc_min(scr, size);
    return scr->ptr;
}

/*!Extend \a scr to the maximum of \a size and twice the old size, unless the
 * old size is at least \a size, and return a pointer to it's memory.  Does
 * not preserve the contents. */
CU_SINLINE void *
cu_scratch_alloc_log(cu_scratch_t scr, size_t size)
{
    if (scr->size < size)
	cuP_scratch_alloc_log(scr, size);
    return scr->ptr;
}

/*!As \ref cu_scratch_alloc_min, but preseves content. */
CU_SINLINE void *
cu_scratch_realloc_min(cu_scratch_t scr, size_t size)
{
    if (scr->size < size)
	cuP_scratch_realloc_min(scr, size);
    return scr->ptr;
}

/*!As \ref cu_scratch_alloc_log, but preserves content. */
CU_SINLINE void *
cu_scratch_realloc_log(cu_scratch_t scr, size_t size)
{
    if (scr->size < size)
	cuP_scratch_realloc_log(scr, size);
    return scr->ptr;
}

/*!\deprecated Use \ref cu_scratch_init. */
#define cu_scratch_cct cu_scratch_init

/*!@}*/
CU_END_DECLARATIONS

#endif
