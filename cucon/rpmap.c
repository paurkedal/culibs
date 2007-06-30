/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2004--2007  Petter Urkedal <urkedal@nbi.dk>
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

#include <cucon/rpmap.h>

void
cucon_rpmap_cct(cucon_rpmap_t rpmap)
{
    cucon_pmap_cct(&rpmap->pmap);
    rpmap->data = NULL;
}

cucon_rpmap_t
cucon_rpmap_new()
{
    cucon_rpmap_t rpmap = cu_gnew(struct cucon_rpmap_s);
    cucon_rpmap_cct(rpmap);
    return rpmap;
}

cucon_rpmap_t
cucon_rpmap_mref(cucon_rpmap_t rpmap, void *key)
{
    cucon_rpmap_t sub;
    if (cucon_pmap_insert_mem(&rpmap->pmap, key,
			    sizeof(struct cucon_rpmap_s), &sub))
	cucon_rpmap_cct(sub);
    return sub;
}

cucon_rumap_t
cucon_rumap_mref(cucon_rumap_t rumap, uintptr_t key)
{
    cucon_rumap_t sub;
    if (cucon_umap_insert_mem(&rumap->pmap.impl, key,
			      sizeof(struct cucon_rumap_s), &sub))
	cucon_rumap_cct(sub);
    return sub;
}

cucon_rpmap_t
cucon_rumap_mref_by_uint8_arr(cucon_rumap_t rumap,
			      uint8_t *key_arr, size_t key_cnt)
{
    while (key_cnt) {
	if (cucon_umap_insert_mem(&rumap->pmap.impl, *key_arr,
				  sizeof(struct cucon_rumap_s), &rumap))
	    cucon_rumap_cct(rumap);
	++key_arr;
	--key_cnt;
    }
    return rumap;
}

cucon_rpmap_t
cucon_rumap_mref_by_uint16_arr(cucon_rumap_t rumap,
			     uint16_t *key_arr, size_t key_cnt)
{
    while (key_cnt) {
	if (cucon_umap_insert_mem(&rumap->pmap.impl, *key_arr,
				  sizeof(struct cucon_rumap_s), &rumap))
	    cucon_rumap_cct(rumap);
	++key_arr;
	--key_cnt;
    }
    return rumap;
}

cucon_rpmap_t
cucon_rumap_mref_by_uint32_arr(cucon_rumap_t rumap,
			     uint32_t *key_arr, size_t key_cnt)
{
    while (key_cnt) {
	if (cucon_umap_insert_mem(&rumap->pmap.impl, *key_arr,
				  sizeof(struct cucon_rumap_s), &rumap))
	    cucon_rumap_cct(rumap);
	++key_arr;
	--key_cnt;
    }
    return rumap;
}

#ifdef CUCONF_HAVE_UINT64_T
cucon_rpmap_t
cucon_rumap_mref_by_uint64_arr(cucon_rumap_t rumap,
			     uint64_t *key_arr, size_t key_cnt)
{
    while (key_cnt) {
	if (cucon_umap_insert_mem(&rumap->pmap.impl, *key_arr,
				  sizeof(struct cucon_rumap_s), &rumap))
	    cucon_rumap_cct(rumap);
	++key_arr;
	--key_cnt;
    }
    return rumap;
}
#endif
