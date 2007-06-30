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

#ifndef CUCON_PCMAP_H
#define CUCON_PCMAP_H

#include <cucon/ucmap.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cucon_pcmap_h cucon/pcmap.h: Constructive Maps from Pointers to Pointers
 * @{\ingroup cucon_maps_and_sets_mod
 * This is a light wrapper over \ref cucon_ucmap_h. */

/*!The empty map. */
CU_SINLINE cucon_pcmap_t
cucon_pcmap_empty() { return NULL; }

/*!True iff \a map is the empty map. */
CU_SINLINE cu_bool_t
cucon_pcmap_is_empty(cucon_pcmap_t map) { return map == NULL; }

/*!Return \a map with \a key ↦ \a val inserted.  If \a map contains
 * a mapping from \a key, it is replaced. */
CU_SINLINE cucon_pcmap_t
cucon_pcmap_insert(cucon_pcmap_t map, void *key, void *val)
{
    return (cucon_pcmap_t)cucon_ucmap_insert((cucon_ucmap_t)map,
					     (uintptr_t)key, val);
}

/*!\copydoc cucon_pcmap_insert */
CU_SINLINE cucon_pcmap_t
cucon_pcmap_insert_int(cucon_pcmap_t map, void *key, int val)
{
    return (cucon_pcmap_t)cucon_ucmap_insert_int((cucon_ucmap_t)map,
						 (uintptr_t)key, val);
}

#define cucon_pcmap_int_none cucon_ucmap_int_none

/*!Return the mapping of \a key in \a map, or \c NULL if none. */
CU_SINLINE void *
cucon_pcmap_find(cucon_pcmap_t map, void *key)
{ return cucon_ucmap_find((cucon_ucmap_t)map, (uintptr_t)key); }

/*!Return the mapping of \a key in \a map, or \c (uintptr_t)-1 of none. */
CU_SINLINE int
cucon_pcmap_find_int(cucon_pcmap_t map, void *key)
{ return cucon_ucmap_find_int((cucon_ucmap_t)map, (uintptr_t)key); }

#if CUCONF_SIZEOF_INTPTR_T == CUCONF_SIZEOF_VOID_P

/*!Sequentially conjunct \a cb over mappings in increasing key order. */
CU_SINLINE cu_bool_t
cucon_pcmap_conj(cucon_pcmap_t map,
		 cu_clop(cb, cu_bool_t, void *key, void *val))
{ return cucon_ucmap_conj((cucon_ucmap_t)map,
			  (cu_clop(, cu_bool_t, uintptr_t, void *))cb); }

/*!Sequentially conjunct \a cb over mappings in increasing key order. */
CU_SINLINE cu_bool_t
cucon_pcmap_conj_int(cucon_pcmap_t map,
		     cu_clop(cb, cu_bool_t, void *key, int val))
{ return cucon_ucmap_conj_int((cucon_ucmap_t)map,
			      (cu_clop(, cu_bool_t, uintptr_t, int))cb); }
#else

/*!Sequentially conjunct \a cb over mappings in increasing key order. */
cu_bool_t
cucon_pcmap_conj(cucon_pcmap_t map,
		 cu_clop(cb, cu_bool_t, void *key, void *val));

/*!Sequentially conjunct \a cb over mappings in increasing key order. */
cu_bool_t
cucon_pcmap_conj_int(cucon_pcmap_t map,
		     cu_clop(cb, cu_bool_t, void *key, int val));

#endif

/*!Debug dump. */
CU_SINLINE void
cucon_pcmap_dump(cucon_pcmap_t map, FILE *out)
{ return cucon_ucmap_dump((cucon_ucmap_t)map, out); }

/*!@}*/
CU_END_DECLARATIONS

#endif
