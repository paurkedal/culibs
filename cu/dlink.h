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

#ifndef CU_DLINK_H
#define CU_DLINK_H

#include <cu/fwd.h>
#include <cu/debug.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cu_dlink_h cu/dlink.h: Double Link Struct and Functions
 * @{\ingroup cu_util_mod
 * Defines a simple double link structure and inline functions to manipulate
 * them.  This is intended as a low-level functionality used to define higher
 * level structures.
 *
 * These links are represented as cyclic.  When used as a list, the cycle is
 * broken by one special head link which is used to refer to the whole list,
 * and which may represent the past-the-end iterator.
 */

typedef struct cu_dlink_s *cu_dlink_t;
/*!The double link structure.  As opposed to most structure in this library,
 * this can be considered trasparent.  */
struct cu_dlink_s
{
    cu_dlink_t next;	/*!< Points to the next element of the link. */
    cu_dlink_t prev;	/*!< Points to the previous element of the link. */
};

#ifdef CU_DEBUG_SLINK
#  define cu_debug_dlink_invalidate(l) ((void)((l)->next = NULL))
#  define cu_debug_dlink_assert_valid(l) cu_debug_assert((l)->next != NULL)
#else
#  define cu_debug_dlink_invalidate(l_init) ((void)(l_init))
#  define cu_debug_dlink_assert_valid(l) ((void)(l))
#endif

/*!Initialise \a l_init as a link with next and prev pointing to itself.  This
 * is typically used as the head of doubly linked lists. */
CU_SINLINE void
cu_dlink_cct_singular(cu_dlink_t l_init)
{
    l_init->next = l_init->prev = l_init;
}

CU_SINLINE cu_bool_t
cu_dlink_is_singular(cu_dlink_t l)
{ return l == l->next; }

CU_SINLINE cu_bool_t
cu_dlink_is_2node(cu_dlink_t l)
{ return l != l->next && l->prev == l->next; }

CU_SINLINE cu_bool_t
cu_dlink_count_leq_2(cu_dlink_t l)
{ return l->prev == l->next; }

/*!Erases and invalidates \a l from the link it is part of.  \a l must not be
 * singular. */
CU_SINLINE void
cu_dlink_erase(cu_dlink_t l)
{
    cu_debug_dlink_assert_valid(l);
    cu_debug_assert(l->next != l);
    l->prev->next = l->next;
    l->next->prev = l->prev;
    cu_debug_dlink_invalidate(l);
}

/*!Construct and insert \a l_init before \a l. */
CU_SINLINE void
cu_dlink_insert_before(cu_dlink_t l, cu_dlink_t l_init)
{
    cu_debug_dlink_assert_valid(l);
    l_init->prev = l->prev;
    l_init->next = l;
    l->prev->next = l_init;
    l->prev = l_init;
}

/*!Construct and insert \a l_init after \a l. */
CU_SINLINE void
cu_dlink_insert_after(cu_dlink_t l, cu_dlink_t l_init)
{
    cu_debug_dlink_assert_valid(l);
    l_init->prev = l;
    l_init->next = l->next;
    l->next->prev = l_init;
    l->next = l_init;
}

/*!Insert the list around \a l_head before \a l and invalidate \a l_head. */
CU_SINLINE void
cu_dlink_insert_list_before(cu_dlink_t l, cu_dlink_t l_head)
{
    cu_debug_dlink_assert_valid(l);
    cu_debug_dlink_assert_valid(l_head);
    l_head->prev->next = l;
    l_head->next->prev = l->prev;
    l->prev->next = l_head->next;
    l->prev = l_head->prev;
    cu_debug_dlink_invalidate(l_head);
}

/*!Insert the list around \a l_head after \a l and invalidate \a l_head. */
CU_SINLINE void
cu_dlink_insert_list_after(cu_dlink_t l, cu_dlink_t l_head)
{
    cu_debug_dlink_assert_valid(l);
    cu_debug_dlink_assert_valid(l_head);
    l_head->next->prev = l;
    l_head->prev->next = l->next;
    l->next->prev = l_head->prev;
    l->next = l_head->next;
    cu_debug_dlink_invalidate(l_head);
}

CU_END_DECLARATIONS

/*!@}*/
#endif
