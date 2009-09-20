/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CU_WEAKPTR_H
#define CU_WEAKPTR_H

#include <cu/memory.h>

CU_BEGIN_DECLARATIONS
/** \defgroup cu_weakptr_h cu/weakptr.h: Weak Pointers
 ** @{ \ingroup cu_util_mod */

/** Construct \a link as a weak pointer to \a ptr.  If \a ptr gets collected,
 ** the link will be cleared, as manifested by a NULL return from \c
 ** cu_weakptr_get(link) */
CU_SINLINE void
cu_weakptr_init(cu_hidden_ptr_t *link, void *ptr)
{
    *link = cu_hide_ptr(ptr);
    GC_general_register_disappearing_link((void **)link, ptr);
}

/** Construct \a link as a weak pointer with a NULL value. */
CU_SINLINE void
cu_weakptr_init_null(cu_hidden_ptr_t *link)
{
    *link = 0;
}

CU_SINLINE void
cu_weakptr_dct_even(cu_hidden_ptr_t *link)
{
    if ((uintptr_t)*link & 1)
	GC_unregister_disappearing_link((void **)link);
}

CU_SINLINE void
cu_weakptr_clear_even(cu_hidden_ptr_t *link)
{
    if ((uintptr_t)*link & 1)
	GC_unregister_disappearing_link((void **)link);
    *link = 0;
}

CU_SINLINE void
cu_weakptr_set_even(cu_hidden_ptr_t *link, void *obj)
{
    if ((uintptr_t)*link & 1)
	GC_unregister_disappearing_link((void **)link);
    *link = cu_hide_ptr(obj);
    GC_general_register_disappearing_link((void **)link, obj);
}

void *cuP_weakptr_get_locked(void *link);

/** Safely return the value of the weak pointer \a link, or \c NULL if the
 ** object it pointed to was recycled. */
CU_SINLINE void *
cu_weakptr_get(cu_hidden_ptr_t *link)
{
    if (*link)
	return GC_call_with_alloc_lock(cuP_weakptr_get_locked, link);
    else
	return NULL;
}

/*!\deprecated Use cu_weakptr_init. */
#define cu_weakptr_cct		cu_weakptr_init
/*!\deprecated Use cu_weakptr_init_null. */
#define cu_weakptr_cct_null	cu_weakptr_init_null

/** @} */
CU_END_DECLARATIONS

#endif
