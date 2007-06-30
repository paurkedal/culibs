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

#include <cu/hcons_wp.h>
#include <cu/dyn.h>
#include <cu/int.h>

#define FILL_MIN_NOM 1
#define FILL_MIN_DENOM 4
#define FILL_MAX_NOM 1
#define FILL_MAX_DENOM 1

struct cu_hcset_s cuP_hcset;


static void
hcset_insert(cu_hcset_t hcset, void *obj)
{
    cu_hash_t hash = cuex_key_hash(obj);
    void **arr = hcset->arr;
    void **new_link = &arr[hash & hcset->mask];
    if (*new_link) {
	if ((uintptr_t)*new_link & 1) {
	    void *coll_obj = cu_weakptr_get((cu_hidden_ptr_t *)new_link);
	    if (coll_obj) {
		cu_hcnode_t new_node = cu_gnew(struct cu_hcnode_s);
		cu_weakptr_dct_even((cu_hidden_ptr_t *)new_link);
		cu_weakptr_cct(&new_node->ptr, obj);
		cu_weakptr_cct((cu_hidden_ptr_t *)&new_node->next,
			       coll_obj);
		*new_link = new_node;
	    }
	    else
		cu_weakptr_set_even((cu_hidden_ptr_t *)new_link, obj);
	}
	else {
	    cu_hcnode_t new_node = cu_gnew(struct cu_hcnode_s);
	    cu_weakptr_cct(&new_node->ptr, obj);
	    new_node->next = *new_link;
	    *new_link = new_node;
	}
    }
    else
	cu_weakptr_set_even((cu_hidden_ptr_t *)new_link, obj);
}

static void
hcset_resize_locked(cu_hcset_t hcset, size_t cnt)
{
    size_t size = cu_ulong_exp2_ceil_log2(3*cnt);
    size_t mask = size - 1;
    void **arr = cu_galloc(sizeof(void *)*size);
    void **link;
    void **link_end;
    memset(arr, 0, sizeof(void *)*size);
    link = hcset->arr;
    link_end = link + hcset->mask + 1;
    hcset->arr = arr;
    hcset->mask = mask;
    while (link != link_end) {
	void *obj;
	void **l = link;
	while (((uintptr_t)*l & 1) == 0) {  /* nodes (or NULL) */
	    cu_hcnode_t node = *l;
	    if (!node)
		goto next_link;
	    obj = cu_weakptr_get(&node->ptr);
	    if (obj) {
		hcset_insert(hcset, obj);
		cu_weakptr_dct_even(&node->ptr);
	    }
	    l = &node->next;
	}
	obj = cu_weakptr_get((cu_hidden_ptr_t *)l);
	if (obj)
	    hcset_insert(hcset, obj);
    next_link:
	++link;
    }
}

void
cu_hcset_adjust(cu_hcset_t hcset)
{
    void **link = hcset->arr;
    void **link_end = link + hcset->mask + 1;
    size_t cnt = 0;
    size_t cap;
    cu_hcset_lock(hcset);
    while (link != link_end) {
	void **l = link;
	while (((uintptr_t)*l & 1) == 0) {
	    cu_hcnode_t node = *l;
	    if (!node)
		goto next_link;
	    if (!node->ptr) {
		if (((uintptr_t)node->next & 1)) {
		    void *obj;
		    obj = cu_weakptr_get((cu_hidden_ptr_t *)&node->next);
		    if (obj) {
			cu_weakptr_dct_even((cu_hidden_ptr_t *)&node->next);
			cu_weakptr_cct((cu_hidden_ptr_t *)l, obj);
		    }
		    else
			*l = NULL;
		    ++cnt;
		    goto next_link;
		}
		else
		    *l = node->next;
	    }
	    else {
		++cnt;
		l = &node->next;
	    }
	}
	++cnt;
    next_link:
	++link;
    }

    cap = hcset->mask + 1;
    if (cnt*FILL_MIN_DENOM < cap*FILL_MIN_NOM)
	hcset_resize_locked(hcset,
		cu_ulong_exp2_ceil_log2(cnt*FILL_MIN_DENOM/FILL_MIN_NOM));
    if (cnt*FILL_MAX_DENOM > cap*FILL_MAX_NOM)
	hcset_resize_locked(hcset, cnt*2);
    cu_hcset_unlock(hcset);
}

#define HCSET_SIZE_INIT 8
void
cuP_hcset_init()
{
    cu_mutex_cct(&cuP_hcset.mutex);
    cuP_hcset.mask = HCSET_SIZE_INIT - 1;
    cuP_hcset.insert_cnt = 0;
    cuP_hcset.arr = cu_galloc(sizeof(void *)*HCSET_SIZE_INIT);
}
