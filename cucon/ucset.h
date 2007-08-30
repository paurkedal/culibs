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

/* This packs close elements in bitsets at the leaves of the trees.  It
 * has little implication when elements are sparse, so enabled is generally
 * best. */
#define CUCON_UCSET_ENABLE_BITSET 1


#include <cucon/fwd.h>
#include <stdio.h>
#include <cu/clos.h>
#if CUCON_UCSET_ENABLE_HCONS
#include <cuoo/type.h>
#endif

CU_BEGIN_DECLARATIONS
/*!\defgroup cucon_ucset_h cucon/ucset.h: Unsigned Integer Constructive Sets
 * @{\ingroup cucon_maps_and_sets_mod
 * cucon_ucset_t implements sets of integers.
 * It is based on binary trees with a structure based on the bit-pattern
 * of the integer keys.
 * As an optimalisation for dense sets, the trees are replaced by
 * bit-patters at the lowest level without significantly hurting performance
 * for sparse sets.  The main features are
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

#if CUCON_UCSET_ENABLE_BITSET
#define CUPRIV_UCSET_BITSET_WORDCNT 2
typedef struct cucon_ucset_leaf_s *cucon_ucset_leaf_t;
struct cucon_ucset_leaf_s
{
#if CUCON_UCSET_ENABLE_HCONS
    CUOO_HCOBJ
#else
    CUOO_OBJ
#endif
    uintptr_t key;
    cu_word_t bitset[CUPRIV_UCSET_BITSET_WORDCNT];
};
#endif

#if CUCON_UCSET_ENABLE_HCONS
extern cuoo_stdtype_t cuconP_ucset_type;
CU_SINLINE cuoo_type_t cucon_ucset_type()
{ return cuoo_stdtype_to_type(cuconP_ucset_type); }
extern cuoo_stdtype_t cuconP_ucset_leaf_type;
CU_SINLINE cuoo_type_t cucon_ucset_leaf_type()
{ return cuoo_stdtype_to_type(cuconP_ucset_leaf_type); }
#endif

/*!Return \a set with \a key inserted. */
cucon_ucset_t cucon_ucset_insert(cucon_ucset_t set, uintptr_t key);

/*!True iff \a key ∈ \a set. */
cu_bool_t cucon_ucset_find(cucon_ucset_t set, uintptr_t key);

/*!Debug dump of \a set. */
void cucon_ucset_dump(cucon_ucset_t set, FILE *out);

/*!Sequential conjunction over elements in increasing order. */
cu_bool_t cucon_ucset_conj(cucon_ucset_t set,
			   cu_clop(cb, cu_bool_t, uintptr_t key));

uintptr_t cucon_ucset_min_ukey(cucon_ucset_t set);
uintptr_t cucon_ucset_max_ukey(cucon_ucset_t set);

/*!@}*/
CU_END_DECLARATIONS

#endif
