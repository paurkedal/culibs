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

#ifndef CUCON_USET_H
#define CUCON_USET_H

#include <cucon/fwd.h>
#include <cucon/umap.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cucon_uset_h cucon/uset.h: Integer-Keyed (Sparse) Sets
 *@{\ingroup cucon_maps_and_sets_mod
 * \see cucon_umap_h
 * \see cucon_bitvect_h
 */

/*!A sparse integer-keyed set. */
struct cucon_uset_s { struct cucon_umap_s impl; };

/*!Constructs \a set as an empty set. */
CU_SINLINE void cucon_uset_cct(cucon_uset_t set)
{ cucon_umap_cct(&set->impl); }

/*!Returns an empty set. */
CU_SINLINE cucon_uset_t cucon_uset_new(void)
{ return (cucon_uset_t)cucon_umap_new(); }

/*!Constructs \a set as a copy of \a src_set. */
CU_SINLINE void cucon_uset_cct_copy(cucon_uset_t set, cucon_uset_t src_set)
{ cucon_umap_cct_copy_void(&set->impl, &src_set->impl); }

/*!Returns a copy of \a src_set. */
CU_SINLINE cucon_uset_t cucon_uset_new_copy(cucon_uset_t src_set)
{ return (cucon_uset_t)cucon_umap_new_copy_void(&src_set->impl); }

/*!Swap the contents of \a set0 and \a set1. */
CU_SINLINE void cucon_uset_swap(cucon_uset_t set0, cucon_uset_t set1)
{ cucon_umap_swap(&set0->impl, &set1->impl); }

/*!True iff \a set0 and \a set1 contain the same elements. */
CU_SINLINE cu_bool_t cucon_uset_eq(cucon_uset_t set0, cucon_uset_t set1)
{ return cucon_umap_eq_keys(&set0->impl, &set1->impl); }

/*!Returns a hash of the elements of \a set. */
CU_SINLINE cu_hash_t cucon_uset_hash(cucon_uset_t set)
{ return cucon_umap_hash_keys(&set->impl); }

/*!The number of elements in \a set. */
CU_SINLINE size_t cucon_uset_size(cucon_uset_t set)
{ return cucon_umap_size(&set->impl); }

/*!True iff \a set is empty. */
CU_SINLINE cu_bool_t cucon_uset_is_empty(cucon_uset_t set)
{ return cucon_umap_is_empty(&set->impl); }

/*!If \a key ∉ \a set, insert it and return true, else return false. */
CU_SINLINE cu_bool_t cucon_uset_insert(cucon_uset_t set, uintptr_t key)
{ return cucon_umap_insert_void(&set->impl, key); }

/*!If \a key ∈ \a set, erase it and return true, else return false. */
CU_SINLINE cu_bool_t cucon_uset_erase(cucon_uset_t set, uintptr_t key)
{ return cucon_umap_erase(&set->impl, key); }

/*!Same as \ref cucon_uset_erase, but don't resize the internal vector.  This
 * can speed up a sequence of multiple erase operations, when done, call \ref
 * cucon_uset_update_cap. */
CU_SINLINE cu_bool_t cucon_uset_erase_keep_cap(cucon_uset_t set, uintptr_t key)
{ return cucon_umap_erase_keep_cap(&set->impl, key); }

/*!Call this after using \ref cucon_uset_erase_keep_cap. */
CU_SINLINE void cucon_uset_update_cap(cucon_uset_t set)
{ return cucon_umap_update_cap(&set->impl); }

/*!Pop any element of \a set and return it.  A random number generator is used
 * to select a position in the hash array in order to preserve the average
 * case structure of \a set. */
CU_SINLINE uintptr_t cucon_uset_pop_any(cucon_uset_t set)
{ return cucon_umap_pop_any_key(&set->impl); }

/*!True iff \a key ∈ \a set. */
CU_SINLINE cu_bool_t cucon_uset_find(cucon_uset_t set, uintptr_t key)
{ return cucon_umap_find_void(&set->impl, key); }

/*!Calls \a cb(\e key) for each \e key ∈ \a set.
 * \see cucon_uset_iter_increasing */
CU_SINLINE void
cucon_uset_iter(cucon_uset_t set, cu_clop(cb, void, uintptr_t key))
{ cucon_umap_iter_keys(&set->impl, cb); }

/*!*Calls \a cb(\e key) for each \e key ∈ \a set in increasing order.
 * \see cucon_uset_iter */
CU_SINLINE void
cucon_uset_iter_increasing(cucon_uset_t set, cu_clop(cb, void, uintptr_t key))
{ cucon_umap_iter_increasing_keys(&set->impl, cb); }

/*!Sequentially conjunct \a cb over elements of \a set. */
CU_SINLINE cu_bool_t
cucon_uset_conj(cucon_uset_t set, cu_clop(cb, cu_bool_t, uintptr_t key))
{ return cucon_umap_conj_keys(&set->impl, cb); }

/* TODO. Iterators. */

/*!Assign (\a set0, \a set1) := (\a set0 ∩ \a set1, \a set1 := \a set0 ∪ \a
 * set1) by moving elements \a set0 ∖ \a set1 from \a set0 to \a set1. */
CU_SINLINE void
cucon_uset_assign_isecn_union(cucon_uset_t set0, cucon_uset_t set1)
{ cucon_umap_assign_isecn_union(&set0->impl, &set1->impl); }

/*!Assigns
 * (\a dst, \a src0, \a src1) :=
 * (\a src0 ∩ \a src1, \a src0 ∖ \a src1, \a src1 ∖ \a src0). */
CU_SINLINE void
cucon_uset_move_isecn(cucon_uset_t dst, cucon_uset_t src0, cucon_uset_t src1)
{ cucon_umap_move_isecn(&dst->impl, &src0->impl, &src1->impl); }

/*!Assigns \a dst := \a dst ∩ \a src. */
CU_SINLINE void
cucon_uset_assign_isecn(cucon_uset_t dst, cucon_uset_t src)
{ cucon_umap_assign_isecn(&dst->impl, &src->impl); }

/*!Assigns \a dst := \a dst ∪ \a src. */
CU_SINLINE void
cucon_uset_assign_union(cucon_uset_t dst, cucon_uset_t src)
{ cucon_umap_assign_union_void(&dst->impl, &src->impl); }

/*!Prints \a set in conventional set notation. */
CU_SINLINE void
cucon_uset_print(cucon_uset_t set, FILE *out)
{ cucon_umap_print_keys(&set->impl, out); }

CU_SINLINE void
cucon_uset_dump_stats(cucon_uset_t set, FILE *out)
{ cucon_umap_dump_stats(&set->impl, out); }

/*!@}*/
CU_END_DECLARATIONS

#endif
