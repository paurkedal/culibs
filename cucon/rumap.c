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

#include <cucon/rumap.h>
#include <cu/wstring.h>

void
cucon_rumap_cct(cucon_rumap_t rumap)
{
    cucon_pmap_cct(&rumap->pmap);
    rumap->data = NULL;
}

cucon_rumap_t
cucon_rumap_new()
{
    cucon_rumap_t rumap = cu_gnew(struct cucon_rumap_s);
    cucon_rumap_cct(rumap);
    return rumap;
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

cucon_rumap_t
cucon_rumap_mref_by_uint8_arr(cucon_rumap_t rumap,
			      uint8_t const *key_arr, size_t key_cnt)
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

cucon_rumap_t
cucon_rumap_mref_by_uint16_arr(cucon_rumap_t rumap,
			       uint16_t const *key_arr, size_t key_cnt)
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

cucon_rumap_t
cucon_rumap_mref_by_uint32_arr(cucon_rumap_t rumap,
			       uint32_t const *key_arr, size_t key_cnt)
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
cucon_rumap_t
cucon_rumap_mref_by_uint64_arr(cucon_rumap_t rumap,
			       uint64_t const *key_arr, size_t key_cnt)
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

cucon_rumap_t
cucon_rumap_mref_by_wstring(cucon_rumap_t rmap, cu_wstring_t s)
{
    return cucon_rumap_mref_by_wchar_arr(rmap, cu_wstring_array(s),
					 cu_wstring_length(s));
}