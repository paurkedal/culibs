/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2009  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CU_SEMIWEAKPTR_H
#define CU_SEMIWEAKPTR_H

#include <cu/memory.h>

CU_BEGIN_DECLARATIONS

void *cuP_weakptr_get_locked(void *link);

/** \defgroup cu_bistptr_h cu/bistptr.h: Bi-Strength Pointer
 ** @{ \ingroup cu_util_mod
 **
 ** This header implements a discriminated union of a strong and a weak
 ** pointer, but with the restriction that all pointers stored must be even,
 ** e.g. due to alignment.
 **
 ** \see cu_weakptr_h "cu/weakptr.h" */

typedef struct { cu_hidden_ptr_t hptr; } cu_bistptr_t;

/** True iff \a bsp is a weak pointer.  The \c NULL pointer counts as
 ** strong. */
CU_SINLINE cu_bool_t
cu_bistptr_is_weak(cu_bistptr_t *bsp)
{
    return (uintptr_t)bsp->hptr & 1;
}

/** The complement of \ref cu_bistptr_is_weak, for symmetry. */
CU_SINLINE cu_bool_t
cu_bistptr_is_strong(cu_bistptr_t *bsp)
{
    return !cu_bistptr_is_weak(bsp);
}

/** Returns the weak pointer held by \a bsp.
 ** \pre \ref cu_bistptr_is_weak(\a ptr) */
CU_SINLINE void *
cu_bistptr_get_weak(cu_bistptr_t *bsp)
{
    return GC_call_with_alloc_lock(cuP_weakptr_get_locked, bsp);
}

/** Returns the strong pointer of \a bsp.
 ** \pre \ref cu_bistptr_is_strong(\a ptr) */
CU_SINLINE void *
cu_bistptr_get_strong(cu_bistptr_t *bsp)
{
    return (void *)bsp->hptr;
}

/** Returns the pointer stored in \a bsp, independent of strength. */
CU_SINLINE void *
cu_bistptr_get(cu_bistptr_t *bsp)
{
    if (cu_bistptr_is_weak(bsp))
	return cu_bistptr_get_weak(bsp);
    else
	return cu_bistptr_get_strong(bsp);
}

/** Initialise \a bsp to a weak pointer to \a obj.
 **
 ** \pre \a obj is aligned to an even address and points to or inside
 ** collectable memory. */
CU_SINLINE void
cu_bistptr_init_weak(cu_bistptr_t *bsp, void *obj)
{
    bsp->hptr = cu_hide_ptr(obj);
    GC_general_register_disappearing_link((void **)bsp, obj);
}

/** Initialise \a bsp to a strong pointer to \a obj.
 **
 ** \pre \a obj is aligned to an even address. */
CU_SINLINE void
cu_bistptr_init_strong(cu_bistptr_t *bsp, void *obj)
{
    bsp->hptr = (cu_hidden_ptr_t)obj;
}

/** Deininialise \a bsp.  If \a bsp holds a weak pointer, this unregisters
 ** the corresponding disappearing link. */
CU_SINLINE void
cu_bistptr_deinit(cu_bistptr_t *bsp)
{
    if (cu_bistptr_is_weak(bsp))
	GC_unregister_disappearing_link((void **)&bsp->hptr);
}

/** Reinitialise \a bsp to hold a weak pointer to \a obj.
 **
 ** \pre \a obj is aligned to an even address and points to or inside
 ** collectable memory. */
CU_SINLINE void
cu_bistptr_assign_weak(cu_bistptr_t *bsp, void *obj)
{
    cu_bistptr_deinit(bsp);
    cu_bistptr_init_weak(bsp, obj);
}

/** Reinitialise \a bsp to hold a strong pointer to \a obj.
 ** \pre \a obj is aligned to an even address. */
CU_SINLINE void
cu_bistptr_assign_strong(cu_bistptr_t *bsp, void *obj)
{
    cu_bistptr_deinit(bsp);
    bsp->hptr = (cu_hidden_ptr_t)obj;
}

/** @} */
CU_END_DECLARATIONS

#endif
