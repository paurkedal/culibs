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

#ifndef CUCON_UCSET_H
#define CUCON_UCSET_H

/* This tuning option has a significant performance hit on insertions, but
 * may be good if client needs quick equality between sets. */
#define CUCON_UCSET_ENABLE_HCONS 1

#include <cucon/fwd.h>
#include <stdio.h>
#include <cu/clos.h>
#if CUCON_UCSET_ENABLE_HCONS
#  include <cuoo/hcobj.h>
#else
#  include <cuoo/fwd.h>
#endif

CU_BEGIN_DECLARATIONS
/*!\defgroup cucon_ucset_h cucon/ucset.h: Constructive Integer Sets
 * @{\ingroup cucon_maps_and_sets_mod
 * This provides constructive tests of integers, implemented as a trie of
 * bitsets.
 * It is thus suitable to represent both sparse and dense sets.
 * The main features are
 * <ul>
 *   <li>O(1) copy since modifications are non-destructive</li>
 *   <li>Search and insert complexity ranges from <i>O</i>(log <i>n</i>)
 *     on avarage for containers of <i>n</i> random numbers,
 *     to <i>O</i>(<i>n</i>) for some patological cases like of containers with
 *     <i>n</i> powers of two.  The patological cases are limited by
 *     the range of integers.  For instance, on a 32 bit platform,
 *     <i>n</i> ≤ 32 - log<sub>2</sub> 64 = 26.
 *     (log<sub>2</sub> 64 comes from the 64-element bitsets at the
 *     leaves.)</li>
 * </ul> 
 * \sa
 * \ref cucon_ucmap_h "cucon/ucmap.h": Maps with a similar implementation.
 * \sa
 * \ref cucon_pmap_h "cucon/pmap.h": This hash map can also be used as a set.
 */

struct cucon_ucset_s
{
#if CUCON_UCSET_ENABLE_HCONS
    CUOO_HCOBJ
#else
    CUOO_OBJ
#endif
    uintptr_t key;
    cucon_ucset_t left;
    cucon_ucset_t right;
};

#if CUCON_UCSET_ENABLE_HCONS
extern cuoo_type_t cuconP_ucset_type;
CU_SINLINE cuoo_type_t cucon_ucset_type()
{ return cuconP_ucset_type; }

extern cuoo_type_t cuconP_ucset_leaf_type;
CU_SINLINE cuoo_type_t cucon_ucset_leaf_type()
{ return cuconP_ucset_leaf_type; }
#endif

/*!True iff \a set0 and \a set1 contains exactly the same elements. */
#if CUCON_UCSET_ENABLE_HCONS
CU_SINLINE cu_bool_t
cucon_ucset_eq(cucon_ucset_t set0, cucon_ucset_t set1)
{ return set0 == set1; }
#else
cu_bool_t cucon_ucset_eq(cucon_ucset_t set0, cucon_ucset_t set1);
#endif

/*!True iff \a set0 ⊆ \a set1. */
cu_bool_t cucon_ucset_subeq(cucon_ucset_t set0, cucon_ucset_t set1);

/*!Returs the empty set (\c NULL). */
CU_SINLINE cucon_ucset_t cucon_ucset_empty(void) { return NULL; }

/*!True iff \a set is the empty set. */
CU_SINLINE cu_bool_t cucon_ucset_is_empty(cucon_ucset_t set)
{ return set == NULL; }

/*!Returns the singleton set {\a key}. */
cucon_ucset_t cucon_ucset_singleton(uintptr_t key);

/*!True iff \a set is a singleton set. */
cu_bool_t cucon_ucset_is_singleton(cucon_ucset_t set);

/*!Return \a set with \a key inserted. */
cucon_ucset_t cucon_ucset_insert(cucon_ucset_t set, uintptr_t key);

/*!Returns \a set ∖ {\a key}. */
cucon_ucset_t cucon_ucset_erase(cucon_ucset_t set, uintptr_t key);

/*!Returns \a set0 ∪ \a set1. */
cucon_ucset_t cucon_ucset_union(cucon_ucset_t set0, cucon_ucset_t set1);

/*!Returns \a set0 ∩ \a set1. */
cucon_ucset_t cucon_ucset_isecn(cucon_ucset_t set0, cucon_ucset_t set1);

/*!Returns \a set0 ∖ \a set1. */
cucon_ucset_t cucon_ucset_compl(cucon_ucset_t set0, cucon_ucset_t set1);

/*!True iff \a key ∈ \a set. */
cu_bool_t cucon_ucset_find(cucon_ucset_t set, uintptr_t key);

/*!Sequential conjunction over elements in increasing order. */
cu_bool_t cucon_ucset_conj(cucon_ucset_t set,
			   cu_clop(cb, cu_bool_t, uintptr_t key));

/*!Calls \a f for each element of \a set. */
void cucon_ucset_iter(cucon_ucset_t set, cu_clop(f, void, uintptr_t));

/*!Returns the cardinality of \a set. */
size_t cucon_ucset_card(cucon_ucset_t set);

/*!Returns the minimum element of \a set, considering elements to be
 * unsigned. */
uintptr_t cucon_ucset_min_ukey(cucon_ucset_t set);

/*!Returns the maximum element of \a set, considering elements to be
 * unsigned. */
uintptr_t cucon_ucset_max_ukey(cucon_ucset_t set);

/*!Returns {\e x | \e x ∈ \a set ∧ \a f(\e x)}. */
cucon_ucset_t cucon_ucset_filter(cucon_ucset_t set,
				 cu_clop(f, cu_bool_t, uintptr_t));

/*!Returns {\a f(\e x) | \e x ∈ \a set}. */
cucon_ucset_t cucon_ucset_image(cucon_ucset_t set,
				cu_clop(f, uintptr_t, uintptr_t));

/*!Combines \ref cucon_ucset_filter and cucon_ucset_image in one operation.  \a
 * f is passed a pointer to each key in \a set.  If it returns false, the
 * element is skipped, otherwise the key at the pointer passed to \a f, which
 * may be modified by \a f, is included in the result. */
cucon_ucset_t cucon_ucset_filter_image(cucon_ucset_t set,
				       cu_clop(f, cu_bool_t, uintptr_t *));

/*!Returns \a set ∩ [clip_min, clip_max], using unsigned comparison. */
cucon_ucset_t cucon_ucset_uclip(cucon_ucset_t set,
				uintptr_t clip_min, uintptr_t clip_max);

/*!Returns \a set ∩ [clip_min, clip_max], using signed comparison. */
cucon_ucset_t cucon_ucset_sclip(cucon_ucset_t set,
				intptr_t clip_min, intptr_t clip_max);

/*!Returns the set {\e x + \a diff | \e x ∈ \a set ∧ \a clip_min ≤ \e x ≤ \a
 * clip_max}. */
cucon_ucset_t cucon_ucset_translate_uclip(cucon_ucset_t set, intptr_t diff,
					  uintptr_t clip_min,
					  uintptr_t clip_max);

/*!\copydoc cucon_ucset_translate_uclip*/
cucon_ucset_t cucon_ucset_translate_sclip(cucon_ucset_t set, intptr_t diff,
					  intptr_t clip_min,
					  intptr_t clip_max);

/*!Returns the set {\e x + \a diff | \e x ∈ \a set}. */
cucon_ucset_t cucon_ucset_translate(cucon_ucset_t set, intptr_t diff);

/*!Print \a set to \a out as a set of unsigend integers using normal set
 * notation. */
void cucon_ucset_fprint_uintptr(cucon_ucset_t set, FILE *out);

/*!Print \a set to \a out as a set of signed integers using normal set
 * notation. */
void cucon_ucset_fprint_intptr(cucon_ucset_t set, FILE *out);

/*!Debug dump of \a set. */
void cucon_ucset_dump(cucon_ucset_t set, FILE *out);

struct cucon_ucset_itr_s
{
    int sp, pos;
    cucon_ucset_t node_stack[1];
};

/*!Returns the size needed and interator over the elements of \a set.  */
size_t cucon_ucset_itr_size(cucon_ucset_t set);

/*!Initialises \a itr for iteration over \a set.  The memory at \a itr must be
 * at least the size reported by \ref cucon_ucset_itr_size(\a set). */
void cucon_ucset_itr_init(cucon_ucset_itr_t itr, cucon_ucset_t set);

/*!Returns an iterator over the elements of \a set. */
cucon_ucset_itr_t cucon_ucset_itr_new(cucon_ucset_t set);

/*!True iff \a itr is past the last element. */
CU_SINLINE cu_bool_t cucon_ucset_itr_at_end(cucon_ucset_itr_t itr)
{ return itr->sp < 0; }

/*!Returns and advances over the next element of \a itr. */
uintptr_t cucon_ucset_itr_get(cucon_ucset_itr_t itr);

/*!@}*/
CU_END_DECLARATIONS

#endif
