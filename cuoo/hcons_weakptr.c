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

#include <cuoo/type.h>
#include <cuoo/oalloc.h>
#include <cu/weakptr.h>
#include <cu/int.h>
#include <cu/wordarr.h>
#include <cu/thread.h>
#include <cu/memory.h>


/* Lower and upper limits for fill ration before resize. */
#define FILL_MIN_NUM 1
#define FILL_MIN_DENOM 4
#define FILL_MAX_NUM 1
#define FILL_MAX_DENOM 1

/* Upper bound of target fill ratio. */
#define FILL_TUB_NUM 2
#define FILL_TUB_DENOM 3

typedef struct _hcset_s *_hcset_t;
typedef struct _hcnode_s *_hcnode_t;
typedef union _hclink_u _hclink_t;

union _hclink_u
{
    _hcnode_t node;
    cu_hidden_ptr_t obj;
};

struct _hcnode_s
{
    cu_hidden_ptr_t obj;
    _hclink_t next;
};

struct _hcset_s
{
    cu_mutex_t mutex;
    size_t mask;
    size_t insert_cnt;
    _hclink_t *arr;
};

/* A global weak set of hash-consed objects. */
static struct _hcset_s _g_hcset;

CU_SINLINE _hclink_t *
_hcset_link(_hcset_t hcset, cu_hash_t hash)
{
    return &hcset->arr[hcset->mask & hash];
}

CU_SINLINE void
_hcset_lock(_hcset_t hcset)
{
    cu_mutex_lock(&hcset->mutex);
}

CU_SINLINE void
_hcset_unlock(_hcset_t hcset)
{
    cu_mutex_unlock(&hcset->mutex);
}

static void _hcset_adjust(_hcset_t hcset);

static void
_hcset_insert(_hcset_t hcset, void *obj)
{
    cu_hash_t hash = cuex_key_hash(obj);
    _hclink_t *arr = hcset->arr;
    _hclink_t *new_link = &arr[hash & hcset->mask];
    if (new_link->node) {
	if ((uintptr_t)new_link->node & 1) {  /* tail object */
	    void *coll_obj = cu_weakptr_get(&new_link->obj);
	    if (coll_obj) {
		_hcnode_t new_node = cu_gnew(struct _hcnode_s);
		cu_weakptr_dct_even(&new_link->obj);
		cu_weakptr_init(&new_node->obj, obj);
		cu_weakptr_init(&new_node->next.obj, coll_obj);
		new_link->node = new_node;
	    }
	    else
		cu_weakptr_set_even(&new_link->obj, obj);
	}
	else {
	    _hcnode_t new_node = cu_gnew(struct _hcnode_s);
	    cu_weakptr_init(&new_node->obj, obj);
	    new_node->next.node = new_link->node;
	    new_link->node = new_node;
	}
    }
    else
	cu_weakptr_set_even(&new_link->obj, obj);
}

static void
_hcset_resize_locked(_hcset_t hcset, size_t cnt)
{
    size_t size = cu_ulong_exp2_ceil_log2(3*cnt);
    size_t mask = size - 1;
    _hclink_t *arr = cu_galloc(sizeof(void *)*size);
    _hclink_t *link;
    _hclink_t *link_end;
    memset(arr, 0, sizeof(void *)*size);
    link = hcset->arr;
    link_end = link + hcset->mask + 1;
    hcset->arr = arr;
    hcset->mask = mask;
    while (link != link_end) {
	void *obj;
	_hclink_t *l = link;
	while (((uintptr_t)l->node & 1) == 0) {  /* nodes (or NULL) */
	    _hcnode_t node = l->node;
	    if (!node)
		goto next_link;
	    obj = cu_weakptr_get(&node->obj);
	    if (obj) {
		_hcset_insert(hcset, obj);
		cu_weakptr_dct_even(&node->obj);
	    }
	    l = &node->next;
	}
	obj = cu_weakptr_get(&l->obj);
	if (obj)
	    _hcset_insert(hcset, obj);
    next_link:
	++link;
    }
}

static void
_hcset_adjust(_hcset_t hcset)
{
    _hclink_t *link = hcset->arr;
    _hclink_t *link_end = link + hcset->mask + 1;
    size_t cnt = 0;
    size_t cap;
    _hcset_lock(hcset);

    /* Remove links for disappeared objects, and count the remaining. */
    while (link != link_end) {
	_hclink_t *l = link;
	while (((uintptr_t)l->node & 1) == 0) {  /* loop over collision links */
	    _hcnode_t node = l->node;
	    if (!node)
		goto next_link;
	    if (!node->obj) {
		if (((uintptr_t)node->next.node & 1)) {
		    void *obj;
		    obj = cu_weakptr_get(&node->next.obj);
		    if (obj) {
			cu_weakptr_dct_even(&node->next.obj);
			cu_weakptr_init(&l->obj, obj);
			++cnt;
		    }
		    else
			l->node = NULL;
		    goto next_link;
		}
		else
		    l->node = node->next.node;
	    }
	    else {
		++cnt;
		l = &node->next;
	    }
	}
	++cnt;  /* last link pointer is a single object */
    next_link:
	++link;
    }

    cap = hcset->mask + 1;
    if (cnt*FILL_MIN_DENOM < cap*FILL_MIN_NUM ||
	cnt*FILL_MAX_DENOM > cap*FILL_MAX_NUM)
	_hcset_resize_locked(hcset,
		cu_ulong_exp2_ceil_log2(cnt*FILL_TUB_DENOM/FILL_TUB_NUM));
    hcset->insert_cnt = 0;
    _hcset_unlock(hcset);
}

static void *
_halloc(cuex_meta_t meta, size_t size, size_t key_sizew, void *key,
	cu_clop(init_nonkey, void, void *))
{
    cuooP_hcobj_t obj;
    cu_hash_t hash = cu_wordarr_hash(key_sizew, key, meta);
    _hclink_t *link;
    cu_hidden_ptr_t *obj_link = NULL;
    _hcnode_t node;

    _hcset_lock(&_g_hcset);
    link = _hcset_link(&_g_hcset, hash);
    while (((uintptr_t)(node = link->node) & 1) == 0 && node) {
	obj = cu_weakptr_get(&node->obj);
	if (obj) {
	    if (cuex_meta(obj) == meta
		    && cu_wordarr_eq(key_sizew, key, CUOO_HCOBJ_KEY(obj))) {
		_hcset_unlock(&_g_hcset);
		return obj;
	    }
	}
	else
	    obj_link = &node->obj;
	link = &node->next;
    }
    if (node) {
	obj = cu_weakptr_get(&link->obj);
	if (obj) {
	    if (cuex_meta(obj) == meta
		    && cu_wordarr_eq(key_sizew, key, CUOO_HCOBJ_KEY(obj))) {
		_hcset_unlock(&_g_hcset);
		return obj;
	    }
	    else if (!obj_link) {
		cu_weakptr_clear_even(&link->obj);
		node = cu_gnew(struct _hcnode_s);
		cu_weakptr_init(&node->obj, obj);
		link->node = node;
		obj_link = &node->next.obj;
	    }
	}
    }
    else
	obj_link = &link->obj;

    obj = cuexP_oalloc(meta, size);
    cu_wordarr_copy(key_sizew, CUOO_HCOBJ_KEY(obj), key);
    cu_call(init_nonkey, obj);
    cu_weakptr_init(obj_link, obj);
    ++_g_hcset.insert_cnt;
    _hcset_unlock(&_g_hcset);
    if (_g_hcset.insert_cnt*4 > _g_hcset.mask)
	_hcset_adjust(&_g_hcset);
    return obj;
}

cu_clop_def(_init_noop, void, void *obj) {}

void *
cuexP_halloc_raw(cuex_meta_t meta, size_t key_sizew, void *key)
{
    return _halloc(meta, (key_sizew + CUOO_HCOBJ_SHIFTW)*CU_WORD_SIZE,
		   key_sizew, key, _init_noop);
}

void *
cuexP_hxalloc_raw(cuex_meta_t meta, size_t sizeg, size_t key_sizew, void *key,
		  cu_clop(init_nonkey, void, void *))
{
    return _halloc(meta, sizeg*CU_GRAN_SIZE, key_sizew, key, init_nonkey);
}

#define HCSET_SIZE_INIT 8
void
cuooP_hcons_init()
{
    cu_mutex_init(&_g_hcset.mutex);
    _g_hcset.mask = HCSET_SIZE_INIT - 1;
    _g_hcset.insert_cnt = 0;
    _g_hcset.arr = cu_galloc(sizeof(void *)*HCSET_SIZE_INIT);
}