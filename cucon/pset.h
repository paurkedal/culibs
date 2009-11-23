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

#ifndef CUCON_PSET_H
#define CUCON_PSET_H

#include <cucon/umap.h>

CU_BEGIN_DECLARATIONS
/** \defgroup cucon_pset_h cucon/pset.h: Poiter-Keyed Sets
 ** @{ \ingroup cucon_maps_and_sets_mod
 ** \see cucon_pmap_h */

/** A pointer-keyed set container. */
struct cucon_pset { struct cucon_umap impl; };

/** Constructs \a set as an empty set. */
CU_SINLINE void cucon_pset_init(cucon_pset_t set)
{ cucon_umap_init(&set->impl); }

/** Returns an empty set. */
CU_SINLINE cucon_pset_t cucon_pset_new(void)
{ return (cucon_pset_t)cucon_umap_new(); }

/** Constructs \a set as a copy of \a src_set. */
CU_SINLINE void cucon_pset_init_copy(cucon_pset_t set, cucon_pset_t src_set)
{ cucon_umap_init_copy_void(&set->impl, &src_set->impl); }

/** Returns a copy of \a src_set. */
CU_SINLINE cucon_pset_t cucon_pset_new_copy(cucon_pset_t src_set)
{ return (cucon_pset_t)cucon_umap_new_copy_void(&src_set->impl); }

/** Swap the contents of \a set0 and \a set1. */
CU_SINLINE void cucon_pset_swap(cucon_pset_t set0, cucon_pset_t set1)
{ cucon_umap_swap(&set0->impl, &set1->impl); }

/** True iff \a set0 and \a set1 contain the same elements. */
CU_SINLINE cu_bool_t cucon_pset_eq(cucon_pset_t set0, cucon_pset_t set1)
{ return cucon_umap_eq_keys(&set0->impl, &set1->impl); }

/** Returns a hash of the elements of \a set. */
CU_SINLINE cu_hash_t cucon_pset_hash(cucon_pset_t set)
{ return cucon_umap_hash_keys(&set->impl); }

/** The number of elements in \a set. */
CU_SINLINE size_t cucon_pset_size(cucon_pset_t set)
{ return cucon_umap_size(&set->impl); }

/** True iff \a set is empty. */
CU_SINLINE cu_bool_t cucon_pset_is_empty(cucon_pset_t set)
{ return cucon_umap_is_empty(&set->impl); }

/* TODO? cucon_pset_insert_node */
/* TODO? cucon_pset_insert_general */

/** If \a key ∉ \a set, insert it and return true, else return false. */
CU_SINLINE cu_bool_t cucon_pset_insert(cucon_pset_t set, void *key)
{ return cucon_umap_insert_void(&set->impl, (uintptr_t)key); }

/** If \a key ∈ \a set, erase it and return true, else return false. */
CU_SINLINE cu_bool_t cucon_pset_erase(cucon_pset_t set, void *key)
{ return cucon_umap_erase(&set->impl, (uintptr_t)key); }

/** Same as \ref cucon_pset_erase, but don't resize internal vector.  This can
 ** be faster if you need to erase many values in a row, and when done, call
 ** \ref cucon_pset_update_cap. */
CU_SINLINE cu_bool_t cucon_pset_erase_keep_cap(cucon_pset_t set, void *key)
{ return cucon_umap_erase_keep_cap(&set->impl, (uintptr_t)key); }

/** Call this after using \ref cucon_pset_erase_keep_cap. */
CU_SINLINE void cucon_pset_update_cap(cucon_pset_t set)
{ cucon_umap_update_cap(&set->impl); }

/** \copydoc cucon_uset_pop_any */
CU_SINLINE void *cucon_pset_pop_any(cucon_pset_t set)
{ return (void *)cucon_umap_pop_any_key(&set->impl); }

/** True iff \a key ∈ \a set. */
CU_SINLINE cu_bool_t cucon_pset_find(cucon_pset_t set, void *key)
{ return cucon_umap_find_void(&set->impl, (uintptr_t)key); }

#ifndef CU_IN_DOXYGEN
void cucon_pmap_iter_keys(cucon_pmap_t, cu_clop(, void, void const *));
cu_bool_t cucon_pmap_conj_keys(cucon_pmap_t,
			       cu_clop(, cu_bool_t, void const *));
#endif

/** Call <code>\a cb(\e k)</code> for each \e k ∈ \a set. */
CU_SINLINE void
cucon_pset_iter(cucon_pset_t set, cu_clop(cb, void, void const *key))
{ cucon_pmap_iter_keys((cucon_pmap_t)set, cb); }

/** Sequantially conjunct \a cb over elements of \a set. */
CU_SINLINE cu_bool_t
cucon_pset_conj(cucon_pset_t set, cu_clop(cb, cu_bool_t, void const *key))
{ return cucon_pmap_conj_keys((cucon_pmap_t)set, cb); }

/* TODO. Iterators. */

/** Assign \a set0 := \a set0 ∩ \a set1 and \a set1 := \a set0 ∪ \a set1 by
 * moving the elements \a set0 ∖ \a set1 from \a set0 to \a set1. */
CU_SINLINE void
cucon_pset_assign_isecn_union(cucon_pset_t set0, cucon_pset_t set1)
{ cucon_umap_assign_isecn_union(&set0->impl, &set1->impl); }

/** Assigns \a dst := \a src0 ∩ \a src1; \a src0 := \a src0 ∖ \a src1; \a src1
 ** := \a src1 ∖ src0. */
CU_SINLINE void
cucon_pset_move_isecn(cucon_pset_t dst, cucon_pset_t src0, cucon_pset_t src1)
{ cucon_umap_move_isecn(&dst->impl, &src0->impl, &src1->impl); }

/** Assigns \a dst := \a dst ∩ \a src. */
CU_SINLINE void
cucon_pset_assign_isecn(cucon_pset_t dst, cucon_pset_t src)
{ cucon_umap_assign_isecn(&dst->impl, &src->impl); }

/** Assigns \a dst := \a dst ∪ \a src. */
CU_SINLINE void
cucon_pset_assign_union(cucon_pset_t dst, cucon_pset_t src)
{ cucon_umap_assign_union_void(&dst->impl, &src->impl); }

/*!\deprecated Use \ref cucon_pset_init. */
#define cucon_pset_cct		cucon_pset_init
/*!\deprecated Use \ref cucon_pset_init_copy. */
#define cucon_pset_cct_copy	cucon_pset_init_copy

/** @} */
CU_END_DECLARATIONS

#endif
