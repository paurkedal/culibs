/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2010  Petter Urkedal <paurkedal@eideticdew.org>
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

#ifndef CUCON_UCMMAP_H
#define CUCON_UCMMAP_H

#include <cucon/ucmap.h>
#include <stdio.h>

CU_BEGIN_DECLARATIONS
/** \defgroup cucon_ucmultimap_h cucon/ucmultimap.h: Constructive Multi-Maps of \c uintptr_t
 ** \ingroup cucon_mod
 **
 ** This container combines \ref cucon_ucmap_h and \ref cucon_ucset_h into a
 ** constructive multi-map from \a uintptr_t to \a uintptr_t.  It also works
 ** with pointers to hash-consed objects, or other pointers where pointer
 ** equality makes sense.
 **
 ** @{ */

/** Array element type for handling multi-sets in de-constructed from. */
struct cucon_ucmultimap_element
{
    uintptr_t key;
    uintptr_t value;
};

/** A callback suitable to pass to qsort to sort an array of
 ** \ref cucon_ucmultimap_element before passing it to \ref
 ** cucon_ucmultimap_from_sorted_array. */
int cucon_ucmultimap_element_cmp(void const *e0, void const *e1);

/** The empty multi-map. */
CU_SINLINE cucon_ucmultimap_t
cucon_ucmultimap_empty()
{ return NULL; }

/** True iff \a M is the empty multi-map. */
CU_SINLINE cu_bool_t
cucon_ucmultimap_is_empty(cucon_ucmultimap_t M)
{ return M == NULL; }

/** The number of keys in \a M. */
CU_SINLINE size_t
cucon_ucmultimap_corange_card(cucon_ucmultimap_t M)
{ return cucon_ucmap_card((cucon_ucmap_t)M); }

/** True iff \a M has at least one mapping from \a k. */
CU_SINLINE cu_bool_t
cucon_ucmultimap_corange_contains(cucon_ucmultimap_t M, uintptr_t k)
{ return cucon_ucmap_contains((cucon_ucmap_t)M, k); }

/** The smallest multi-map which agrees on \a M on keys within the range [\a
 ** k_min, \a k_max]. */
CU_SINLINE cucon_ucmultimap_t
cucon_ucmultimap_clip(cucon_ucmultimap_t M, uintptr_t k_min, uintptr_t k_max)
{ return (cucon_ucmultimap_t)cucon_ucmap_clip((cucon_ucmap_t)M, k_min, k_max); }

/** The multi-map which takes \a k to \a v and is undefined elsewhere. */
cucon_ucmultimap_t cucon_ucmultimap_singleton(uintptr_t k, uintptr_t v);

/** True iff \a M is a singleton map. */
cu_bool_t cucon_ucmultimap_is_singleton(cucon_ucmultimap_t M);

/** Returns a multi-map from the \a len elements starting at \a arr.  The
 ** elements must be sorted in ascending lexicographical order, where the key
 ** is the most significant. */
cucon_ucmultimap_t
cucon_ucmultimap_from_sorted_array(struct cucon_ucmultimap_element *arr,
				   size_t len);

/** The multi-map which agrees on \a M everywhere except \a k has an extra
 ** mapping to \a v if not already present.  Other mappings from \a k are
 ** kept. */
cucon_ucmultimap_t
cucon_ucmultimap_insert(cucon_ucmultimap_t M, uintptr_t k, uintptr_t v);

/** The multi-map \a M with \a v removed from the mappings of \a k.  If, as a
 ** result, \a k has no mappings, the key will no longer be in the corange of
 ** the result.  */
cucon_ucmultimap_t
cucon_ucmultimap_erase(cucon_ucmultimap_t M, uintptr_t k, uintptr_t v);

/** True iff \a k maps to \a v in \a M.  The presence of other mappings for \a
 ** k does not affect the result. */
cu_bool_t
cucon_ucmultimap_contains(cucon_ucmultimap_t M, uintptr_t k, uintptr_t v);

/** The full cardinality of \a M.  This takes time linear in the number of keys
 ** in \a M.  To query the number of keys in the corange, use \ref
 ** cucon_ucmultimap_corange_card instead. */
size_t cucon_ucmultimap_card(cucon_ucmultimap_t M);

/** Call \a f on each element of \a M as long as \a f returns true.  If \a f
 ** ever returns false, the result is false, otherwise the result is true.  */
cu_bool_t cucon_ucmultimap_iterA(cu_clop(f, cu_bool_t, uintptr_t, uintptr_t),
				 cucon_ucmultimap_t M);

/** True iff \a M0 and \a M1 are extensionally equal. */
cu_bool_t cucon_ucmultimap_eq(cucon_ucmultimap_t M0, cucon_ucmultimap_t M1);

cucon_ucmultimap_t
cucon_ucmultimap_union(cucon_ucmultimap_t M0, cucon_ucmultimap_t M1);

/** A singleton of the iso-key view of a multimap. */
CU_SINLINE cucon_ucmultimap_t
cucon_ucmultimap_isokey_singleton(uintptr_t k, cucon_ucset_t V)
{ return (cucon_ucmultimap_t)cucon_ucmap_singleton(k, (uintptr_t)V); }

/** Returns the set of values to which \a k maps in \a M. */
CU_SINLINE cucon_ucset_t
cucon_ucmultimap_isokey_find(cucon_ucmultimap_t M, uintptr_t k)
{ return (cucon_ucset_t)cucon_ucmap_find_ptr((cucon_ucmap_t)M, k); }

/** Returns the result of replacing the image of \a k in \a M with the set \a
 ** V. */
CU_SINLINE cucon_ucmultimap_t
cucon_ucmultimap_isokey_insert(cucon_ucmultimap_t M,
			       uintptr_t k, cucon_ucset_t V)
{
    return (cucon_ucmultimap_t)cucon_ucmap_insert((cucon_ucmap_t)M,
						  k, (uintptr_t)V);
}

/** Returns the map which agrees on \a M everywhere except that \a k has no
 ** mapping. */
CU_SINLINE cucon_ucmultimap_t
cucon_ucmultimap_isokey_erase(cucon_ucmultimap_t M, uintptr_t k)
{ return (cucon_ucmultimap_t)cucon_ucmap_erase((cucon_ucmap_t)M, k); }

/** Returns the result of replacing the image of \a k in \a M with the union of
 ** the original image and \a V. */
cucon_ucmultimap_t
cucon_ucmultimap_isokey_merge(cucon_ucmultimap_t M,
			      uintptr_t k, cucon_ucset_t V);

/** Returns the result of replacing the image of \a k in \a M with the
 ** complement of \a V relative to the original image. */
cucon_ucmultimap_t
cucon_ucmultimap_isokey_unmerge(cucon_ucmultimap_t M,
				uintptr_t k, cucon_ucset_t V);

/** Calls \a f on each element of the iso-key view of \a M. */
CU_SINLINE cu_bool_t
cucon_ucmultimap_isokey_iterA(cu_clop(f, cu_bool_t, uintptr_t, cucon_ucset_t),
			      cucon_ucmultimap_t M)
{ return cucon_ucmap_conj_ptr((cucon_ucmap_t)M,
			      (cu_clop(, cu_bool_t, uintptr_t, void *))f); }

void cucon_ucmultimap_fprint_uintptr(cucon_ucmultimap_t M, FILE *fh);

/** @} */
CU_END_DECLARATIONS

#endif
