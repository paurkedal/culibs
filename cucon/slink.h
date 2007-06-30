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

#ifndef CUCON_SLINK_H
#define CUCON_SLINK_H

#include <cucon/fwd.h>
#include <cu/memory.h>
#include <cu/util.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cucon_slink_h cucon/slink.h: A singly linked list.
 * @{\ingroup cucon_linear_mod */

struct cucon_slink_s
{
    cucon_slink_t next;
};

/*!Prepend a node with slot size \a size in front of \a rest, and return the
 * result.  Use \ref cucon_slink_mem to obtain a pointer to the allocated slot.
 */
CU_SINLINE cucon_slink_t
cucon_slink_prepend_mem(cucon_slink_t rest, size_t size)
{
    cucon_slink_t l;
    l = cu_galloc(cu_aligned_ceil(sizeof(struct cucon_slink_s)) + size);
    l->next = rest;
    return l;
}

/*!Prepend a node in front of \a rest with a slot containing \a ptr, and return
 * the result.  */
cucon_slink_t cucon_slink_prepend_ptr(cucon_slink_t slink, void *ptr);

/*!The list \a l with the head node removed.
 * \pre \a l is not empty (\c NULL). */
CU_SINLINE cucon_slink_t cucon_slink_next(cucon_slink_t l) { return l->next; }

/*!The memory slot of the head node of \a l.
 * \pre \a l is not empty. */
CU_SINLINE void *cucon_slink_mem(cucon_slink_t l)
{ return CU_ALIGNED_MARG_END(cucon_slink_t, l); }

/*!The value of the head node of \a l, assuming it's slot contains a pointer.
 * \pre \a l is not empty. */
CU_SINLINE void *cucon_slink_ptr(cucon_slink_t l)
{ return *(void **)cucon_slink_mem(l); }

/*!@}*/

/*\ingroup deprecated \deprecated, just use \c NULL. */
CU_SINLINE cucon_slink_t cucon_slink_empty(void) { return NULL; }
/*\ingroup deprecated \deprecated, just compare to \c NULL. */
CU_SINLINE cu_bool_t cucon_slink_is_empty(cucon_slink_t l) { return l==NULL; }
/*\ingroup deprecated \deprecated Old name. */
#define cucon_slink_get_mem cucon_slink_mem
/*\ingroup deprecated \deprecated Old name. */
#define cucon_slink_get_ptr cucon_slink_ptr

CU_END_DECLARATIONS

#endif
