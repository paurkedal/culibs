/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2008  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUCON_HZSET_H
#define CUCON_HZSET_H

#include <cucon/hzmap.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cucon_hzset_h cucon/hzset.h: Hash Set of Flat Fixed-Sized Keys
 *@{\ingroup cucon_maps_and_sets_mod
 * This is a thin layer over \ref cucon_hzmap_h "cucon/hzmap.h" specialised for
 * the case when there is no value associated with the keys.
 */

typedef struct cucon_hzset_node_s *cucon_hzset_node_t;
struct cucon_hzset_node_s
{
    struct cucon_hzmap_node_s impl;
};

struct cucon_hzset_s
{
    struct cucon_hzmap_s impl;
};

/*!The number of elements in \a set. */
CU_SINLINE size_t cucon_hzset_size(cucon_hzset_t set)
{ return cucon_hzmap_size(&set->impl); }

/*!The size of the underlying array of \a set. */
CU_SINLINE size_t cucon_hzset_capacity(cucon_hzset_t set)
{ return cucon_hzmap_capacity(&set->impl); }

/*!Initialise \a set an an empty hash set of keys of \a key_size_w words. */
CU_SINLINE void cucon_hzset_init(cucon_hzset_t set, cu_shortsize_t key_size_w)
{ cucon_hzmap_init(&set->impl, key_size_w); }

/*!Returns an empty hash set of keys of \a key_size_w words. */
CU_SINLINE cucon_hzset_t cucon_hzset_new(cu_shortsize_t key_size_w)
{ return (cucon_hzset_t)cucon_hzmap_new(key_size_w); }

/*!Given that \a node is initialised with a key of suitable size for \a set, if
 * the key exists in \a set, returns false, else inserts \a node into \a set
 * and returns true.  The \ref cucon_hzset_node_s base struct of \a node is
 * initialised by this call if the insert takes place. */
CU_SINLINE cu_bool_t
cucon_hzset_insert_node(cucon_hzset_t set, cucon_hzset_node_t node)
{ return cucon_hzmap_insert_node(&set->impl, &node->impl); }

/*!If \a set does not contain \a key, inserts it and returns true, else returns
 * false. */
CU_SINLINE cu_bool_t cucon_hzset_insert(cucon_hzset_t set, void const *key)
{ return cucon_hzmap_insert_void(&set->impl, key); }

/*!If \a set contains \a key, inserts it and returns true, else returns
 * false. */
CU_SINLINE cu_bool_t cucon_hzset_erase(cucon_hzset_t set, void const *key)
{ return cucon_hzmap_erase(&set->impl, key); }

/*!As \ref cucon_hzset_erase, but don't change the capacity of the undelying
 * array.  Use this to accelerate a series of deletions, and call \ref
 * cucon_hzset_finish_erase at the end. */
CU_SINLINE cu_bool_t cucon_hzset_step_erase(cucon_hzset_t set, void const *key)
{ return cucon_hzmap_step_erase(&set->impl, key); }

/*!Adjust the capacity after a series of \ref cucon_hzset_step_erase. */
CU_SINLINE void cucon_hzset_finish_erase(cucon_hzset_t set)
{ cucon_hzmap_finish_erase(&set->impl); }

/*!True iff \a set contains \a key. */
CU_SINLINE cu_bool_t cucon_hzset_contains(cucon_hzset_t set, void const *key)
{ return cucon_hzmap_find(&set->impl, key) != NULL; }

/*!A variant of \ref cucon_hzset_contains which is specialised for the case
 * when the keys size is known to be 1 word. */
CU_SINLINE cu_bool_t
cucon_hzset_1w_contains(cucon_hzset_t set, void const *key)
{ return cucon_hzmap_1w_find(&set->impl, key) != NULL; }

/*!A variant of \ref cucon_hzset_contains which is specialised for the case
 * when the keys size is known to be 2 words. */
CU_SINLINE cu_bool_t
cucon_hzset_2w_contains(cucon_hzset_t set, void const *key)
{ return cucon_hzmap_2w_find(&set->impl, key) != NULL; }

/*!True iff \a f maps all elements of \a set to true, otherwise returns
 * immediately on the first element which \a f maps to false. */
CU_SINLINE cu_bool_t
cucon_hzset_forall(cu_clop(f, cu_bool_t, void const *key), cucon_hzset_t set)
{ return cucon_hzmap_forall_keys(f, &set->impl); }

/*!Filter out all elements of \a set which \a f maps to false. */
CU_SINLINE void
cucon_hzset_filter(cu_clop(f, cu_bool_t, void const *key), cucon_hzset_t set)
{ return cucon_hzmap_filter_keys(f, &set->impl); }

struct cucon_hzset_itr_s
{
    struct cucon_hzmap_itr_s impl;
};

/*!Initialise \a itr for iterating over all elements of \a set. */
CU_SINLINE void cucon_hzset_itr_init(cucon_hzset_itr_t itr, cucon_hzset_t set)
{ cucon_hzmap_itr_init(&itr->impl, &set->impl); }

/*!Returns the next element of the sequence initialised by \ref
 * cucon_hzset_itr_init, or \c NULL if there are no more elements. */
CU_SINLINE void const *cucon_hzset_itr_get(cucon_hzset_itr_t itr)
{ return cucon_hzmap_itr_get_key(&itr->impl); }

/*!@}*/
CU_END_DECLARATIONS

#endif
