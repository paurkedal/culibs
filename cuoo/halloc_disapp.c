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

/* This is a disappearing-link implementation of hash-consed memory.  It uses a
 * global weak hash-table consisting of an array of singly-linked lists.
 * Pointers to the objects are registered as disappearing links.  As a memory
 * optimisation, objects are also stored in place of terminating NULLs of the
 * lists, and objects and list nodes are distinguished by setting the lower bit
 * of the address of the latter.
 *
 * This implementation is used if the disclaim GC-patch is not detected or if
 * --disable-gc-disclaim is passed to configure.  The disclaim implementation
 *  is preferred, since it is faster and has a lower per-object memory
 *  overhead.  */

#include <cuoo/type.h>
#include <cuoo/oalloc.h>
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

/* The initial capacity.  Must be a non-zero power of 2. */
#define CUOO_HCONS_INIT_CAP 2

/* Used only for debugging this file. */
#define CUOO_HCONS_NDEBUG


cu_dlog_def(_file, "dtag=cuoo.hcons");


/* Disappearing Link Helpers
 * ========================= */

CU_SINLINE void
_init_disappearing(void **link, void *obj)
{
    *link = obj;
#ifdef CU_DEBUG_MEMORY
    obj = GC_base(obj);
#endif
#ifdef CU_HCONS_NDEBUG
    GC_general_register_disappearing_link(link, obj);
#else
    if (GC_general_register_disappearing_link(link, obj) != GC_SUCCESS)
	cu_bugf("Failed to register disappearing link %p -> %p", link, obj);
#endif
}

CU_SINLINE void
_unreg_disappearing(void *link)
{
#ifdef CU_HCONS_NDEBUG
    GC_unregister_disappearing_link(link);
#else
    if (!GC_unregister_disappearing_link(link))
	cu_bugf("Expected %p to be a disappearing link.", link);
#endif
}

static void *
_get_disappearing_mlck(void *link)
{
    return *(void **)link;
}

CU_SINLINE void *
_get_disappearing(void **link)
{
    return GC_call_with_alloc_lock(_get_disappearing_mlck, link);
}


/* Weak Hash-Table Implementation
 * ============================== */

typedef struct _hcset_s *_hcset_t;
typedef struct _hcnode_s *_hcnode_t;

struct _hcnode_s
{
    void *obj;
    void *next;
};

CU_SINLINE cu_bool_t
_is_hcnode(void *ptr)
{
    return (uintptr_t)ptr & 1;
}

CU_SINLINE _hcnode_t
_hcnode_from_ptr(void *ptr)
{
    return (_hcnode_t)((char *)ptr - 1);
}

CU_SINLINE void *
_hcnode_to_ptr(_hcnode_t node)
{
    return (char *)node + 1;
}

struct _hcset_s
{
    cu_mutex_t mutex;
    size_t mask;
    size_t insert_cnt;
    void **arr;
};

CU_SINLINE void **
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

static void
_hcset_insert(_hcset_t hcset, void *obj)
{
    cu_hash_t hash = cuex_key_hash(obj);
    void *coll_obj;
    void **new_link = _hcset_link(hcset, hash);

    /* If the head links to a node, push a new node in front. */
    if (_is_hcnode(*new_link)) {
	_hcnode_t new_node = cu_unew_atomic(struct _hcnode_s);
	_init_disappearing(&new_node->obj, obj);
	new_node->next = *new_link;
	*new_link = _hcnode_to_ptr(new_node);
    }

    /* If there is an object, take care of disapparing links as we move it to
     * a new node. */
    else if ((coll_obj = _get_disappearing(new_link))) {
	_hcnode_t new_node;
	_unreg_disappearing(new_link);
	new_node = cu_unew_atomic(struct _hcnode_s);
	_init_disappearing(&new_node->obj, obj);
	_init_disappearing(&new_node->next, coll_obj);
	*new_link = _hcnode_to_ptr(new_node);
    }

    /* If the collision-list is empty, just put our object there. */
    else
	_init_disappearing(new_link, obj);
}

static void
_hcset_resize_locked(_hcset_t hcset, size_t cnt)
{
    size_t new_size = cu_ulong_exp2_ceil_log2(cnt*FILL_TUB_DENOM/FILL_TUB_NUM);
    size_t new_mask = new_size - 1;
    void **old_begin, **old_next, **old_end;

    cu_dlogf(_file, "Resizing hash-cons table to %zd.", new_size);

    /* Get the old array. */
    old_begin = hcset->arr;
    old_end = old_begin + hcset->mask + 1;

    /* Initialise 'hcset' to an empty set of the right capacity. */
    hcset->arr = cu_unewarrz_atomic(void *, new_size);
    hcset->mask = new_mask;

    /* Drop disappearing links and free nodes from the old array, while
     * inserting all its objects into 'hcset'. */
    for (old_next = old_begin; old_next != old_end; ++old_next) {
	void **l = old_next;
	void *obj;
	_hcnode_t free_node = NULL;
	while (_is_hcnode(*l)) {
	    _hcnode_t node = _hcnode_from_ptr(*l);
	    if (free_node)
		cu_ufree_atomic(free_node);
	    free_node = node;
	    obj = _get_disappearing(&node->obj);
	    if (obj) {
		_unreg_disappearing(&node->obj);
		_hcset_insert(hcset, obj);
	    }
	    l = &node->next;
	}
	obj = _get_disappearing(l);
	if (obj) {
	    _unreg_disappearing(l);
	    _hcset_insert(hcset, obj);
	}

	if (free_node)
	    cu_ufree_atomic(free_node);
    }
    cu_ufree_atomic(old_begin);
}

static void
_hcset_adjust(_hcset_t hcset)
{
    void **link = hcset->arr;
    void **link_end = link + hcset->mask + 1;
    size_t cnt = 0;
    size_t cap;

    /* Count the objects. */
    for (; link != link_end; ++link) {
	void *ptr = *link;
	while (_is_hcnode(ptr)) {
	    _hcnode_t node = _hcnode_from_ptr(ptr);
	    if (node->obj)
		++cnt;
	    ptr = node->next;
	}
	if (ptr)
	    ++cnt;
    }

    _hcset_lock(hcset);

    /* Resize if object count over capacity is outside the desired range. */
    cap = hcset->mask + 1;
    if (cnt*FILL_MIN_DENOM < cap*FILL_MIN_NUM ||
	cnt*FILL_MAX_DENOM > cap*FILL_MAX_NUM)
	_hcset_resize_locked(hcset, cnt);

    /* Otherwise, remove links for disappeared objects. */
    else for (; link != link_end; ++link) {
	void **l = link;
	while (_is_hcnode(*l)) {  /* loop over collision links */
	    _hcnode_t node = _hcnode_from_ptr(*l);
	    if (node->obj)
		l = &node->next;
	    else if (_is_hcnode(node->next)) {
		*l = node->next;
		cu_ufree_atomic(node);
	    }
	    else {
		void *obj = _get_disappearing(&node->next);
		if (obj) {
		    _unreg_disappearing(&node->next);
		    _init_disappearing(l, obj);
		}
		else
		    *l = NULL;
		cu_ufree_atomic(node);
	    }
	}
    }

    hcset->insert_cnt = 0;
    _hcset_unlock(hcset);
}


/* Allocation
 * ========== */

static struct _hcset_s _g_hcset;

static void *
_halloc(cuex_meta_t meta, size_t size, size_t key_sizew, void *key,
	cu_clop(init_nonkey, void, void *))
{
    cuooP_hcobj_t obj;
    cu_hash_t hash = cu_wordarr_hash(key_sizew, key, meta);
    void **link;
    void **obj_link = NULL;
    _hcnode_t node CU_NOINIT(NULL);

    /* Go though all the hash-equivalence links to search for an equal object
     * or an unused node, according to the disappearing links, to reuse. */
    _hcset_lock(&_g_hcset);
    link = _hcset_link(&_g_hcset, hash);
    while (_is_hcnode(*link)) {
	node = _hcnode_from_ptr(*link);
	obj = _get_disappearing(&node->obj);
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
    if ((obj = _get_disappearing(link))) {
	if (cuex_meta(obj) == meta
		&& cu_wordarr_eq(key_sizew, key, CUOO_HCOBJ_KEY(obj))) {
	    _hcset_unlock(&_g_hcset);
	    return obj;
	}

	/* If we didn't find a free link node, create one.  "link" is the last slot
	 * of the hash-equivalence chain, either on a link node or in the table.
	 * It currently holds "obj", which we must move. */
	if (!obj_link) {
	    _unreg_disappearing(link);
	    *link = NULL;
	    node = cu_unew_atomic(struct _hcnode_s);
	    _init_disappearing(&node->obj, obj);
	    *link = _hcnode_to_ptr(node);
	    obj_link = &node->next;
	}
    }
    else
	obj_link = link;

    /* Allocate the object and save it, then update and unlock the hash set. */
    obj = cuexP_oalloc(meta, size);
    cu_wordarr_copy(key_sizew, CUOO_HCOBJ_KEY(obj), key);
    cu_call(init_nonkey, obj);
    _init_disappearing(obj_link, obj);
    ++_g_hcset.insert_cnt;
    _hcset_unlock(&_g_hcset);

    /* Adjust the capacity if needed. */
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

void
cuooP_hcons_init()
{
    cu_mutex_init(&_g_hcset.mutex);
    _g_hcset.mask = CUOO_HCONS_INIT_CAP - 1;
    _g_hcset.insert_cnt = 0;
    _g_hcset.arr = cu_unewarrz_atomic(void *, CUOO_HCONS_INIT_CAP);
}
