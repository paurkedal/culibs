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

#include <cudyn/prop.h>


void
cudyn_prop_cct(cudyn_prop_t prop)
{
    cucon_pmap_cct(&prop->pmap);
    cu_rarex_cct(&prop->rarex);
}

cudyn_prop_t
cudyn_prop_new()
{
    cudyn_prop_t prop = cu_gnew(struct cudyn_prop_s);
    cudyn_prop_cct(prop);
    return prop;
}

cu_bool_t
cudyn_prop_replace_ptr(cudyn_prop_t key, cuex_t ex, void *value)
{
    cu_bool_t res;
    void **slot;
    cu_rarex_lock_write(&key->rarex);
    res = cucon_pmap_insert_mem(&key->pmap, ex, sizeof(void *), &slot);
    *slot = value;
    cu_rarex_unlock_write(&key->rarex);
    return res;
}

cu_bool_t
cudyn_prop_condset_ptr(cudyn_prop_t key, cuex_t ex, void *value)
{
    cu_bool_t res;
    void **slot;
    cu_rarex_lock_write(&key->rarex);
    res = cucon_pmap_insert_mem(&key->pmap, ex, sizeof(void *), &slot);
    if (res)
	*slot = value;
    cu_rarex_unlock_write(&key->rarex);
    return res;
}

void
cudyn_prop_define_ptr(cudyn_prop_t key, cuex_t ex, void *value)
{
    cu_bool_t res = cudyn_prop_condset_ptr(key, ex, value);
    cu_debug_assert(res);
}

void *
cudyn_prop_get_ptr(cudyn_prop_t key, cuex_t ex)
{
    void *res;
    cu_rarex_lock_read(&key->rarex);
    res = cucon_pmap_find_ptr(&key->pmap, ex);
    cu_rarex_unlock_read(&key->rarex);
    return res;
}

cu_bool_t
cudyn_prop_set_mem_lock(cudyn_prop_t key, cuex_t ex,
			size_t size, cu_ptr_ptr_t slot)
{
    cu_rarex_lock_write(&key->rarex);
    return cucon_pmap_insert_mem(&key->pmap, ex, size, slot);
}

cu_bool_t
cudyn_prop_set_mem_condlock(cudyn_prop_t key, cuex_t ex,
			    size_t size, cu_ptr_ptr_t slot)
{
    cu_rarex_lock_write(&key->rarex);
    if (cucon_pmap_insert_mem(&key->pmap, ex, size, slot))
	return cu_true;
    else {
	cu_rarex_unlock_write(&key->rarex);
	return cu_false;
    }
}

void *
cudyn_prop_get_mem_lock(cudyn_prop_t key, cuex_t ex)
{
    cu_rarex_lock_read(&key->rarex);
    return cucon_pmap_find_mem(&key->pmap, ex);
}

void *
cudyn_prop_get_mem_condlock(cudyn_prop_t key, cuex_t ex)
{
    void *res;
    cu_rarex_lock_read(&key->rarex);
    res = cucon_pmap_find_mem(&key->pmap, ex);
    if (!res)
	cu_rarex_unlock_read(&key->rarex);
    return res;
}

