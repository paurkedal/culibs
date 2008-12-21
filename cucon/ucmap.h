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

#ifndef CUCON_UCMAP_H
#define CUCON_UCMAP_H

/* This tuning option has a significant performance hit on insertions, but
 * may be good if client needs quick equality between sets. */
#define cuconP_UCMAP_ENABLE_HCONS 1

/* When set, pack an existence-bit in the left-pointer, instead of using
 * a special null-value.  */
#define cuconP_UCMAP_EXIST_IN_LEFT 0

#include <cucon/fwd.h>
#include <stdio.h>
#include <cu/clos.h>
#if cuconP_UCMAP_ENABLE_HCONS
#  include <cuoo/hcobj.h>
#else
#  include <cuoo/fwd.h>
#endif
#include <stdint.h>
#include <limits.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cucon_ucmap_h cucon/ucmap.h: Constructive Maps from Unsigned Integers to Pointers
 * @{\ingroup cucon_maps_and_sets_mod
 * cucon_ucmap_t implements maps from integers to pointers.
 * It is based on binary trees with a structure based on the bit-pattern
 * of the integer.  The main features are
 * <ul>
 *   <li>O(1) copy since modifications are non-destructive</li>
 *   <li>Search and insert complexity ranges from <i>O</i>(log <i>n</i>)
 *     on avarage for containers of <i>n</i> random numbers,
 *     to <i>O</i>(<i>n</i>) for some patological cases of containers with
 *     <i>n</i> powers of two.  The patological cases are limited by
 *     the range of integers, e.g. <i>n</i> ≤ 32 for 32 bit integers.</li>
 * </ul> 
 * \sa
 * \ref cucon_ucset_h "cucon/ucset.h": Sets with similar implementation
 * \sa
 * \ref cucon_pmap_h "cucon/pmap.h": Hash maps are more efficient for
 * uses which do not require constructive updates.
 */

struct cucon_ucmap_s
{
#if cuconP_UCMAP_ENABLE_HCONS
    CUOO_HCOBJ
#else
    CUOO_OBJ
#endif
    uintptr_t key;
    cucon_ucmap_t left;
    cucon_ucmap_t right;
    uintptr_t value;
};

#if cuconP_UCMAP_ENABLE_HCONS
extern cuoo_type_t cuconP_ucmap_type;
CU_SINLINE cuoo_type_t cucon_ucmap_type()
{ return cuconP_ucmap_type; }
#endif

#define cucon_ucmap_int_none INT_MIN

/*!The empty map. */
CU_SINLINE cucon_ucmap_t cucon_ucmap_empty() { return NULL; }

/*!True iff \a map is the empty map. */
CU_SINLINE cu_bool_t
cucon_ucmap_is_empty(cucon_ucmap_t map) { return map == NULL; }

cucon_ucmap_t cuconP_ucmap_insert_raw(cucon_ucmap_t, uintptr_t, uintptr_t);

/*!Return \a map with any present \a key ↦ \e oldval removed an
 * \a key ↦ \a val inserted. */
CU_SINLINE cucon_ucmap_t
cucon_ucmap_insert_ptr(cucon_ucmap_t map, uintptr_t key, void *val)
{ return cuconP_ucmap_insert_raw(map, key, (uintptr_t)val); }

/*!\copydoc cucon_ucmap_insert_ptr */
CU_SINLINE cucon_ucmap_t
cucon_ucmap_insert_int(cucon_ucmap_t map, uintptr_t key, uintptr_t val)
{
#if cuconP_UCMAP_EXIST_IN_LEFT
    return cuconP_ucmap_insert_raw(map, key, val);
#else
    return cuconP_ucmap_insert_raw(map, key, val - cucon_ucmap_int_none);
#endif
}

/*!If \a map has a mapping from \a key, returns the result of erasing it,
 * otherwise returns map. */
cucon_ucmap_t cucon_ucmap_erase(cucon_ucmap_t map, uintptr_t key);

/*!The mapping for \a key in \a map or \c NULL if none. */
void *cucon_ucmap_find_ptr(cucon_ucmap_t map, uintptr_t key);

/*!The mapping for \a key in \a map, or \c cucon_ucmap_int_none if not
 * found. */
int cucon_ucmap_find_int(cucon_ucmap_t map, uintptr_t key);

/*!Returns the number of elements in \a map. */
size_t cucon_ucmap_card(cucon_ucmap_t map);

/*!Call \a f for each key and value pair in \a M, assuming values are
 * pointers. */
void cucon_ucmap_iter_ptr(cucon_ucmap_t M, cu_clop(f, void, uintptr_t, void *));

/*!Call \a f for each key and value pair in \a M, assuming values are
 * integers. */
void cucon_ucmap_iter_int(cucon_ucmap_t M, cu_clop(f, void, uintptr_t, int));

/*!Sequential conjunction over mappings in order of increasing keys. */
cu_bool_t cucon_ucmap_conj_ptr(cucon_ucmap_t map,
			       cu_clop(f, cu_bool_t, uintptr_t, void *));

/*!Sequential conjunction over mappings in order of increasing keys. */
cu_bool_t cucon_ucmap_conj_int(cucon_ucmap_t map,
			       cu_clop(cb, cu_bool_t, uintptr_t, int));

/*!The minimum key in \a map when keys are interpreted as unsigned. */
uintptr_t cucon_ucmap_min_ukey(cucon_ucmap_t map);

/*!The maximum key in \a map when keys are interpreted as unsigned. */
uintptr_t cucon_ucmap_max_ukey(cucon_ucmap_t map);

/*!Debug dump of \a map. */
void cucon_ucmap_dump(cucon_ucmap_t map, FILE *out);

/*!@}*/

CU_END_DECLARATIONS

#endif
