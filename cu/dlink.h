/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2006--2009  Petter Urkedal <urkedal@nbi.dk>
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
/** \defgroup cu_dlink_h cu/dlink.h: Double Link Struct and Functions
 ** @{ \ingroup cu_util_mod
 **
 ** Defines a simple double link structure and inline functions to manipulate
 ** them.  This is intended as a low-level functionality used to define higher
 ** level structures.
 **
 ** These links are represented as cyclic.  When used as a list, the cycle is
 ** broken by one special head link which is used to refer to the whole list,
 ** and which may represent the past-the-end iterator.  */

typedef struct cu_dlink_s *cu_dlink_t;

/** The double link structure.  As opposed to most structure in this library,
 ** this can be considered transparent.  */
struct cu_dlink_s
{
    cu_dlink_t next;	/**< Points to the next element of the link. */
    cu_dlink_t prev;	/**< Points to the previous element of the link. */
};

#ifdef CU_DEBUG_DLINK
#  define cu_debug_dlink_invalidate(l) ((void)((l)->next = NULL))
#  define cu_debug_dlink_assert_valid(l) cu_debug_assert((l)->next != NULL)
#else
#  define cu_debug_dlink_invalidate(l_init) ((void)(l_init))
#  define cu_debug_dlink_assert_valid(l) ((void)(l))
#endif

#define CU_DLINK_SINGLETON_DECL(cuL_var) \
    struct cu_dlink_s cuL_var = {&cuL_var, &cuL_var}

/** Validate the link integrity of \a l. */
void cu_dlink_validate(cu_dlink_t l);

/** Initialise \a l_init as a link with next and prev pointing to itself.  This
 ** is typically used as the head of doubly linked lists. */
CU_SINLINE void
cu_dlink_init_singleton(cu_dlink_t l_init)
{ l_init->next = l_init->prev = l_init; }

/** True iff \a l is a singleton. */
CU_SINLINE cu_bool_t cu_dlink_is_singleton(cu_dlink_t l)
{ return l != NULL && l == l->next; }

/** True iff the cardinality of \a l is at most 1. */
CU_SINLINE cu_bool_t cu_dlink_card_leq_1(cu_dlink_t l)
{ return l == NULL || l->next == l; }

/** True iff the cardinality of \a l is 2. */
CU_SINLINE cu_bool_t cu_dlink_card_eq_2(cu_dlink_t l)
{ return l != NULL && l != l->next && l->prev == l->next; }

/** True iff the cardinality of \a l is at most 2. */
CU_SINLINE cu_bool_t cu_dlink_card_leq_2(cu_dlink_t l)
{ return l == NULL || l->prev == l->next; }

/** Return minimum of the \a limit and the number of elements of \a l.  The
 ** limit allows quick return if the client is only needs to distinguish cases
 ** up to a certain number.  Pass \c SIZE_MAX for \a limit for a full count. */
size_t cu_dlink_card_lim(cu_dlink_t l, size_t limit);

/** Erases and invalidates \a l from the link it is part of.  \a l must not be
 ** singular. */
CU_SINLINE void
cu_dlink_erase(cu_dlink_t l)
{
    cu_debug_dlink_assert_valid(l);
    cu_debug_assert(l->next != l);
    l->prev->next = l->next;
    l->next->prev = l->prev;
    cu_debug_dlink_invalidate(l);
}

/** Initialise \a l_init as the predecessor of \a l.
 ** \pre \a l is not \c NULL. */
CU_SINLINE void
cu_dlink_insert_before(cu_dlink_t l, cu_dlink_t l_init)
{
    cu_debug_dlink_assert_valid(l);
    l_init->prev = l->prev;
    l_init->next = l;
    l->prev->next = l_init;
    l->prev = l_init;
}

/** Initialise \a l_init as the successor of \a l.
 ** \pre \a l is not \c NULL. */
CU_SINLINE void
cu_dlink_insert_after(cu_dlink_t l, cu_dlink_t l_init)
{
    cu_debug_dlink_assert_valid(l);
    l_init->prev = l;
    l_init->next = l->next;
    l->next->prev = l_init;
    l->next = l_init;
}

/** Move \a l_mv right before \a l.  This works whether they are in the same or
 ** in different cycles.
 ** \pre Neither argument is \c NULL and \a l != l_mv. */
void cu_dlink_move_before(cu_dlink_t l, cu_dlink_t l_mv);

/** Move \a l_mv right after \a l.  This works whether they are in the same or
 ** in different cycles.
 ** \pre Neither argument is \c NULL and \a l != l_mv. */
void cu_dlink_move_after(cu_dlink_t l, cu_dlink_t l_mv);

/** Splice \a l0 and \a l1 right before the given nodes.  If \a l0 and \a l1
 ** are links of the same cycle, then the cycle is split into two cycles,
 ** otherwise the two separate cycles forms a single cycle.  This operation can
 ** therefore be used as concatenation if sentinel nodes are not used.
 ** \pre Neither link is \c NULL. */
void cu_dlink_splice_before(cu_dlink_t l0, cu_dlink_t l1);

/** Splice \a l0 and \a l1 right after the given nodes, otherwise the same as
 ** \ref cu_dlink_splice_before. */
void cu_dlink_splice_after(cu_dlink_t l0, cu_dlink_t l1);

/** Concatenate \a l0 and \a l1 and return the result.  This uses \ref
 ** cu_dlink_splice_before if applicable, and in case \a l0 is not \c NULL, it
 ** will form the first part of the cycle as seen from the returned link.  The
 ** arguments shall be considered destructed, as indicated by the \c _d
 ** suffix. */
cu_dlink_t cu_dlink_cat_d(cu_dlink_t l0, cu_dlink_t l1);

/** Insert the list around \a l_head before \a l and invalidate \a l_head. */
void cu_dlink_insert_list_before(cu_dlink_t l, cu_dlink_t l_head);

/** Insert the list around \a l_head after \a l and invalidate \a l_head. */
void cu_dlink_insert_list_after(cu_dlink_t l, cu_dlink_t l_head);

CU_END_DECLARATIONS

/** @} */

#define cu_dlink_cct_singular cu_dlink_init_singleton
#define cu_dlink_is_singular cu_dlink_is_singleton
#define cu_dlink_is_2node cu_dlink_card_eq_2
#define cu_dlink_count_leq_2 cu_dlink_card_leq_2

#endif
