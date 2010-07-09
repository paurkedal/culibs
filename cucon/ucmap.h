/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2005--2010  Petter Urkedal <paurkedal@eideticdew.org>
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

#ifndef CUCON_UCMAP_H
#define CUCON_UCMAP_H

/* This tuning option has a significant performance hit on insertions, but
 * may be good if client needs quick equality between sets. */
#define CUCONP_UCMAP_ENABLE_HCONS 1

#include <cucon/fwd.h>
#include <stdio.h>
#include <cu/clos.h>
#if CUCONP_UCMAP_ENABLE_HCONS
#  include <cuoo/hcobj.h>
#else
#  include <cuoo/fwd.h>
#endif
#include <stdint.h>
#include <limits.h>

CU_BEGIN_DECLARATIONS
/** \defgroup cucon_ucmap_h cucon/ucmap.h: Constructive Maps from Unsigned Integers to Pointers
 ** @{ \ingroup cucon_maps_and_sets_mod
 **
 ** The following implements maps from integers to pointers, represented as
 ** binary trees with a structure based on the bit-pattern of the integer keys.
 ** The implementation is more compact than a trie due to the omission of
 ** single-choice branches.  Some key features are
 **   - O(1) copy since modifications are non-destructive.
 **   - O(1) equality and arbitrary ordering due to hash-consing.
 **   - Search and insert complexity ranges from <i>O</i>(log <i>n</i>)
 **     on average for containers of <i>n</i> random numbers,
 **     to <i>O</i>(<i>n</i>) for some pathological cases of containers with
 **     <i>n</i> powers of two.  The pathological cases are limited by
 **     the range of integers, e.g. <i>n</i> ≤ 32 for 32 bit integers.
 **
 ** \sa \ref cucon_ucset_h "cucon/ucset.h": A wrapper specialising to set.
 ** \sa \ref cucon_pcmap_h "cucon/pcmap.h": A wrapper specialising to pointer
 ** keys.
 ** \sa \ref cucon_pmap_h "cucon/umap.h": Hash maps which are more efficient
 ** for uses which do not require constructive updates.
 **/

/** A node of the map, where the root node also represents the whole map. */
struct cucon_ucmap
{
#if CUCONP_UCMAP_ENABLE_HCONS
    CUOO_HCOBJ
#else
    CUOO_OBJ
#endif
    uintptr_t key;
    cucon_ucmap_t left;
    cucon_ucmap_t right;
    uintptr_t value;
};

struct cucon_ucmap_element
{
    uintptr_t key;
    uintptr_t value;
};

#if CUCONP_UCMAP_ENABLE_HCONS
extern cuoo_type_t cuconP_ucmap_type;
CU_SINLINE cuoo_type_t cucon_ucmap_type()
{ return cuconP_ucmap_type; }
#endif

#define cucon_ucmap_int_none INT_MIN

/** The empty map. */
CU_SINLINE cucon_ucmap_t cucon_ucmap_empty() { return NULL; }

/** True iff \a map is the empty map. */
CU_SINLINE cu_bool_t
cucon_ucmap_is_empty(cucon_ucmap_t map) { return map == NULL; }

/** Return the map {\a key ↦ \a val}. */
cucon_ucmap_t cucon_ucmap_singleton(uintptr_t key, uintptr_t val);

/** \copydoc cucon_ucmap_singleton */
CU_SINLINE cucon_ucmap_t
cucon_ucmap_singleton_ptr(uintptr_t key, void *val)
{ return cucon_ucmap_singleton(key, (uintptr_t)val); }

/** True iff the domain of \a map is a singleton. */
CU_SINLINE cu_bool_t
cucon_ucmap_is_singleton(cucon_ucmap_t map)
{ return map != NULL && map->left == NULL && map->right == NULL; }

/** Returns a map of elements given by the array \a arr of \a len (key,
 ** value)-elements, which must be sorted in ascending key order. */
cucon_ucmap_t
cucon_ucmap_from_sorted_array(struct cucon_ucmap_element *arr, size_t len);

/** Return a map which agrees with \a map everywhere, except that \a key maps
 ** to \a val. */
cucon_ucmap_t cucon_ucmap_insert(cucon_ucmap_t, uintptr_t, uintptr_t);

/** \copydoc cucon_ucmap_insert */
CU_SINLINE cucon_ucmap_t
cucon_ucmap_insert_ptr(cucon_ucmap_t map, uintptr_t key, void *val)
{ return cucon_ucmap_insert(map, key, (uintptr_t)val); }

/** \copydoc cucon_ucmap_insert */
CU_SINLINE cucon_ucmap_t
cucon_ucmap_insert_int(cucon_ucmap_t map, uintptr_t key, int val)
{ return cucon_ucmap_insert(map, key, val); }

/** If \a map has a mapping from \a key, returns the result of erasing it,
 ** otherwise returns map. */
cucon_ucmap_t cucon_ucmap_erase(cucon_ucmap_t map, uintptr_t key);

/** True iff \a map has a mapping from \a key. */
cu_bool_t cucon_ucmap_contains(cucon_ucmap_t map, uintptr_t key);

/** If \a key is found in \a map, assign its value to <code>\a v_out</code> and
 ** return true, else return false. */
cu_bool_t cucon_ucmap_find(cucon_ucmap_t map, uintptr_t key, uintptr_t *v_out);

/** The mapping for \a key in \a map or \c NULL if none. */
void *cucon_ucmap_find_ptr(cucon_ucmap_t map, uintptr_t key);

/** The mapping for \a key in \a map, or \c cucon_ucmap_int_none if
 ** not found. */
int cucon_ucmap_find_int(cucon_ucmap_t map, uintptr_t key);

/** Returns the number of elements in \a map. */
size_t cucon_ucmap_card(cucon_ucmap_t map);

size_t cucon_ucmap_clipped_card(cucon_ucmap_t map,
				uintptr_t k_min, uintptr_t k_max);

/** Call \a f for each key and value pair in \a M. */
cu_bool_t cucon_ucmap_iterA(cu_clop(f, cu_bool_t, uintptr_t, uintptr_t),
			    cucon_ucmap_t M);

cu_bool_t cucon_ucmap_iterA_clipped(cu_clop(f, cu_bool_t, uintptr_t, uintptr_t),
				    cucon_ucmap_t M,
				    uintptr_t k_min, uintptr_t k_max);

/** Call \a f for each key and value pair in \a M, assuming values are
 ** pointers. */
void cucon_ucmap_iter_ptr(cucon_ucmap_t M, cu_clop(f, void, uintptr_t, void *));

/** Call \a f for each key and value pair in \a M, assuming values are
 ** integers. */
void cucon_ucmap_iter_int(cucon_ucmap_t M, cu_clop(f, void, uintptr_t, int));

/** Sequential conjunction over mappings in order of increasing keys. */
cu_bool_t cucon_ucmap_conj_ptr(cucon_ucmap_t map,
			       cu_clop(f, cu_bool_t, uintptr_t, void *));

/** Sequential conjunction over mappings in order of increasing keys. */
cu_bool_t cucon_ucmap_conj_int(cucon_ucmap_t map,
			       cu_clop(cb, cu_bool_t, uintptr_t, int));

/** Return a map with co-range equal to the union of the co-ranges of \a M0 and
 ** \a M1, and the mappings agree with \a M0 on its co-range and with \a M1
 ** elsewhere. */
cucon_ucmap_t cucon_ucmap_left_union(cucon_ucmap_t M0, cucon_ucmap_t M1);

/** Returns a map with co-range equal to the union of the co-ranges of \a M0
 ** and \a M1, and the mappings agree with \a M0 or \a M1 for non-intersecting
 ** keys, and the mappings is obtained by calling \a f on intersecting keys. */
cucon_ucmap_t
cucon_ucmap_combined_union(cu_clop(f, uintptr_t, uintptr_t, uintptr_t),
			   cucon_ucmap_t M0, cucon_ucmap_t M1);

/** The minimum key in \a map when keys are interpreted as unsigned. */
uintptr_t cucon_ucmap_min_ukey(cucon_ucmap_t map);

/** The maximum key in \a map when keys are interpreted as unsigned. */
uintptr_t cucon_ucmap_max_ukey(cucon_ucmap_t map);

cucon_ucmap_t cucon_ucmap_uclip(cucon_ucmap_t map,
				uintptr_t k_min, uintptr_t k_max);

cucon_ucmap_t cucon_ucmap_clip(cucon_ucmap_t map,
			       uintptr_t k_min, uintptr_t k_max);

/** True iff \a map0 and \a map1 are equal, where values are compared as
 ** integers or by pointer comparison. */
cu_bool_t cucon_ucmap_eq(cucon_ucmap_t map0, cucon_ucmap_t map1);

/** True iff \a map0 and \a map1 are equal, where \a f compares the values.
 ** \pre \a f must return true on equal pointers. */
cu_bool_t cucon_ucmap_eq_ptr(cu_clop(f, cu_bool_t, void const *, void const *),
			     cucon_ucmap_t map0, cucon_ucmap_t map1);

/** Return -1, 0, or 1 when \a map0 is less, equal, or greater than \a map1,
 ** where the value slots are compared as integers or by pointer equality. */
int cucon_ucmap_cmp(cucon_ucmap_t map0, cucon_ucmap_t map1);

/** Return -1, 0, or 1 when \a map0 is less, equal, or greater than \a map1,
 ** where the value solts are compared according to \a f.
 ** \pre \a f must return 0 on equal pointers. */
int cucon_ucmap_cmp_ptr(cu_clop(f, int, void const *, void const *),
			cucon_ucmap_t map0, cucon_ucmap_t map1);

/** Debug dump of \a map. */
void cucon_ucmap_dump(cucon_ucmap_t map, FILE *out);

/** @} */

CU_END_DECLARATIONS

#endif
