/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUCON_RPMAP_H
#define CUCON_RPMAP_H

#include <cucon/rumap.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cucon_rpmap_h cucon/rpmap.h: Recursive Hash Map with Pointer Keys
 *@{\ingroup cucon_maps_and_sets_mod */

struct cucon_rpmap
{
    struct cucon_rumap impl;
};

CU_SINLINE void cucon_rpmap_init(cucon_rpmap_t rmap)
{ cucon_rumap_init(&rmap->impl); }

CU_SINLINE cucon_rpmap_t cucon_rpmap_new(void)
{ return (cucon_rpmap_t)cucon_rumap_new(); }

CU_SINLINE void *cucon_rpmap_value(cucon_rpmap_t rmap)
{ return cucon_rumap_value(&rmap->impl); }

CU_SINLINE void cucon_rpmap_set_value(cucon_rpmap_t rmap, void *value)
{ cucon_rumap_set_value(&rmap->impl, value); }

CU_SINLINE cu_bool_t cucon_rpmap_is_leaf(cucon_rpmap_t rmap)
{ return cucon_rumap_is_leaf(&rmap->impl); }

CU_SINLINE cucon_rpmap_t cucon_rpmap_mref(cucon_rpmap_t rmap, void *key)
{ return (cucon_rpmap_t)cucon_rumap_mref(&rmap->impl, (uintptr_t)key); }

CU_SINLINE cucon_rpmap_t
cucon_rpmap_cref(cucon_rpmap_t rpmap, void *key)
{ return (cucon_rpmap_t)cucon_rumap_cref(&rpmap->impl, (uintptr_t)key); }

CU_SINLINE cu_bool_t
cucon_rpmap_conj(cucon_rpmap_t rpmap,
		 cu_clop(fn, cu_bool_t, void const *, cucon_rpmap_t))
{
    return cucon_umap_conj_mem(&rpmap->impl.branches,
			       (cu_clop(, cu_bool_t, uintptr_t, void *))fn);
}

/*!@}*/
CU_END_DECLARATIONS

#endif
