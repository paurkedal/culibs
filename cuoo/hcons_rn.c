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

#include <cuoo/hcons_rn.h>
#include <cuoo/oalloc.h>
#include <cu/memory.h>
#include <cu/int.h>
#include <cu/ptr.h>
#include <cu/wordarr.h>
#include <cu/conf.h>
#ifdef CUOO_ENABLE_KEYED_PROP
#  include <cucon/umap.h>
#endif
#include <inttypes.h>

#define CAP_MIN 2
#if 1	/* full on average */
#  define FILL_MIN_NUMER 1
#  define FILL_MIN_DENOM 2
#  define FILL_MAX_NUMER 2
#  define FILL_MAX_DENOM 1
#else	/* half-full on average */
#  define FILL_MIN_NUMER 1
#  define FILL_MIN_DENOM 4
#  define FILL_MAX_NUMER 1
#  define FILL_MAX_DENOM 1
#endif

#if 1
#   define ARR_ALLOC(size) malloc(size)
#   define ARR_FREE(ptr) free(ptr)
#else
#   define ARR_ALLOC(size) cu_galloc_au(size)
#   define ARR_FREE(ptr) cu_gfree_au(ptr)
#endif

#if CUPRIV_ENABLE_COLL_STATS
static int_least64_t _coll_count = 0;
static int_least64_t _noncoll_count = 0;
#endif

void
cuooP_hcset_init(cuooP_hcset_t hcset)
{
    hcset->mask = CAP_MIN - 1;
    hcset->arr = ARR_ALLOC(sizeof(void *)*CAP_MIN);
    memset(hcset->arr, 0, sizeof(void *)*CAP_MIN);
    hcset->cnt = 0;
#ifdef CUCONF_ENABLE_THREADS
# if CU_HCSET_USE_RAREX
    cu_rarex_init(&hcset->rarex);
# else
    cu_mutex_init(&hcset->mutex);
# endif
#endif
}

static void
cuooP_hcset_shrink_wlck(cuooP_hcset_t hcset, size_t new_cap, cuooP_hcobj_t *new_arr)
{
    cu_hash_t hash;
    size_t old_cap = hcset->mask + 1;
    size_t new_mask = new_cap - 1;
    cuooP_hcobj_t *old_arr = hcset->arr;
    cuooP_hcobj_t *old_arr_end = old_arr + old_cap;
    cu_debug_assert(0 < new_cap && new_cap < old_cap);
    memset(new_arr, 0, sizeof(void *)*new_cap);
    cu_debug_inform("New cap = %d\n", new_cap);
    hash = 0;
    while (old_arr != old_arr_end) {
	cuooP_hcobj_t obj = *old_arr;
	while (obj) {
	    cuooP_hcobj_t next_obj = cuooP_hcset_hasheqv_next(obj);
	    cuooP_hcobj_t *p = &new_arr[hash & new_mask];
	    cu_debug_assert(GC_base(obj) == (void *)obj - sizeof(cuex_meta_t));
#if CUOO_HC_GENERATION || CUOO_HC_USE_GC_MARK
	    obj->hcset_next = ~(AO_t)*p;
#else
	    obj->hcset_next = (obj->hcset_next & 1) | ~((AO_t)*p | 1);
#endif
	    *p = obj;
	    obj = next_obj;
	}
	++old_arr;
	++hash;
    }
    ARR_FREE(hcset->arr);
    hcset->arr = new_arr;
    hcset->mask = new_mask;
}

static void
cuooP_hcset_grow_wlck(cuooP_hcset_t hcset, size_t new_cap, cuooP_hcobj_t *new_arr)
{
    size_t old_cap = hcset->mask + 1;
    size_t new_mask = new_cap - 1;
    cuooP_hcobj_t *old_arr = hcset->arr;
    cuooP_hcobj_t *old_arr_end = old_arr + old_cap;
    cu_debug_assert(new_cap > old_cap);
    memset(new_arr, 0, sizeof(void *)*new_cap);
    cu_debug_inform("New cap = %d\n", new_cap);
    while (old_arr != old_arr_end) {
	cuooP_hcobj_t obj = *old_arr;
	while (obj) {
	    cuooP_hcobj_t next_obj = cuooP_hcset_hasheqv_next(obj);
	    cu_hash_t hash = cuex_key_hash(obj);
	    cuooP_hcobj_t *p = &new_arr[hash & new_mask];
	    cu_debug_assert(GC_base(obj) == (void *)obj - sizeof(cuex_meta_t));
#if CUOO_HC_GENERATION || CUOO_HC_USE_GC_MARK
	    obj->hcset_next = ~(AO_t)*p;
#else
	    obj->hcset_next = (obj->hcset_next & 1) | ~((AO_t)*p | 1);
#endif
	    *p = obj;
	    obj = next_obj;
	}
	++old_arr;
    }
    ARR_FREE(hcset->arr);
    hcset->arr = new_arr;
    hcset->mask = new_mask;
}

void
cuooP_hcset_adjust_wlck(cuooP_hcset_t hcset)
{
    size_t mask, cnt, new_cap;
    cuooP_hcobj_t *new_arr;
    mask = hcset->mask;
    cnt = hcset->cnt;
    if (cnt*FILL_MAX_DENOM > mask*FILL_MAX_NUMER) {
	new_cap = (mask + 1)*2;
	new_arr = ARR_ALLOC(sizeof(cuooP_hcobj_t *)*new_cap);
	cuooP_hcset_grow_wlck(hcset, new_cap, new_arr);
    }
    else if (cnt*FILL_MIN_DENOM < mask*FILL_MIN_NUMER && mask > CAP_MIN) {
	new_cap = cu_ulong_exp2_ceil_log2(cnt*FILL_MIN_DENOM/FILL_MIN_NUMER);
	new_arr = ARR_ALLOC(sizeof(cuooP_hcobj_t *)*new_cap);
	cuooP_hcset_shrink_wlck(hcset, new_cap, new_arr);
    }
}

void
cuooP_hcset_adjust(cuooP_hcset_t hcset)
{
#if 0
    /* This version makes a tentative choice whether to resize by reading from
     * unlocked storage, then allocates what it needs, locks, and rechecks if
     * the choice was correct.  This appears to be the faster
     * implementation. */
    size_t mask = hcset->mask;
    size_t cnt = hcset->cnt;
    if (cnt*FILL_MAX_DENOM > mask*FILL_MAX_NUMER) {
	size_t new_cap = (mask + 1)*2;
	cuooP_hcobj_t *new_arr = ARR_ALLOC(sizeof(void *)*new_cap);
	cuooP_hcset_lock_write(hcset);
	if (hcset->cnt*FILL_MAX_DENOM > mask*FILL_MAX_NUMER &&
	    new_cap - 1 > hcset->mask) {
	    cuooP_hcset_grow_wlck(hcset, new_cap, new_arr);
	    cuooP_hcset_unlock_write(hcset);
	}
	else {
	    cuooP_hcset_unlock_write(hcset);
	    ARR_FREE(new_arr);
	}
    }
    else if (cnt*FILL_MIN_DENOM < mask*FILL_MIN_NUMER && mask > CAP_MIN) {
	size_t new_cap
	    = cu_ulong_exp2_ceil_log2(cnt*FILL_MIN_DENOM/FILL_MIN_NUMER);
	cuooP_hcobj_t *new_arr = ARR_ALLOC(sizeof(void *)*new_cap);
	cuooP_hcset_lock_write(hcset);
	if (hcset->cnt*FILL_MIN_DENOM < hcset->mask*FILL_MIN_NUMER &&
	    new_cap - 1 < hcset->mask) {
	    cuooP_hcset_shrink_wlck(hcset, new_cap, new_arr);
	    cuooP_hcset_unlock_write(hcset);
	}
	else {
	    cuooP_hcset_unlock_write(hcset);
	    ARR_FREE(new_arr);
	}
    }
#else
    /* This version uses lock promotion, and if promotion fails, leaves the
     * capacity to be changed on next attempt.  It also avoids redundant
     * allocation when two threads tries to resize simultaneously, but at the
     * expense that if the set is very buzy, it may not be resized at all.
     *
     * NB. The ARR_ALLOC function must _not_ trigger any finalisers for the
     * objects stored hcset.  These finalisers are invoked by the collector
     * when reclaiming corresponding pages, so as long as the collector only
     * reclaims from pages it needs for a specific allocation, it is safe.  In
     * any case the malloc option (see top) should be safe. */
    size_t mask, cnt, new_cap;
    cuooP_hcobj_t *new_arr;
    cuooP_hcset_lock_read(hcset);
    mask = hcset->mask;
    cnt = hcset->cnt;
    if (cnt*FILL_MAX_DENOM > mask*FILL_MAX_NUMER)
	new_cap = (mask + 1)*2;
    else if (cnt*FILL_MIN_DENOM < mask*FILL_MIN_NUMER && mask > CAP_MIN)
	new_cap = cu_ulong_exp2_ceil_log2(cnt*FILL_MIN_DENOM/FILL_MIN_NUMER);
    else {
	cuooP_hcset_unlock_read(hcset);
	return;
    }
    if (!cuooP_hcset_try_promote_lock(hcset)) {
	cuooP_hcset_unlock_read(hcset);
	return;
    }
    new_arr = ARR_ALLOC(sizeof(cuooP_hcobj_t *)*new_cap);
    if (cnt*FILL_MAX_DENOM > mask*FILL_MAX_NUMER)
	cuooP_hcset_grow_wlck(hcset, new_cap, new_arr);
    else
	cuooP_hcset_shrink_wlck(hcset, new_cap, new_arr);
    cuooP_hcset_unlock_write(hcset);
#endif
}

CU_SINLINE void
cuooP_hcset_hasheqv_erase_wlck(cuooP_hcset_t hcset, cu_hash_t hash,
			    cuooP_hcobj_t obj)
{
    size_t cnt;
    cuooP_hcobj_t *p = &hcset->arr[hash & hcset->mask];
    cu_debug_assert(*p);
    if (*p == obj)
	*p = cuooP_hcset_hasheqv_next(obj);
    else {
	cuooP_hcobj_t obj0 = *p;
	cuooP_hcobj_t obj1;
	while ((obj1 = cuooP_hcset_hasheqv_next(obj0)) != obj) {
	    cu_debug_assert(obj1);
	    obj0 = obj1;
	}
#if CUOO_HC_GENERATION || CUOO_HC_USE_GC_MARK
	obj0->hcset_next = obj1->hcset_next;
#else
	cu_debug_assert(((uintptr_t)obj1->hcset_next & 1) == 0);
	obj0->hcset_next = (AO_t)(((uintptr_t)obj0->hcset_next & 1) |
				   (uintptr_t)obj1->hcset_next);
#endif
	cu_debug_assert((uintptr_t)obj0->hcset_next != 0);
    }
    cnt = --hcset->cnt;
#if CUOO_HC_ADJUST_IN_INSERT_ERASE
    if (cnt*FILL_MIN_DENOM < hcset->mask*FILL_MIN_NUMER
	    && hcset->mask > CAP_MIN) {
	size_t new_cap;
	cuooP_hcobj_t *new_arr;
	new_cap = cu_ulong_exp2_ceil_log2(cnt*FILL_MAX_DENOM/FILL_MAX_NUMER);
	if (new_cap < CAP_MIN)
	    new_cap = CAP_MIN;
	new_arr = ARR_ALLOC(sizeof(cuooP_hcobj_t *)*new_cap);
	cuooP_hcset_shrink_wlck(hcset, new_cap, new_arr);
    }
#endif
}

#if CU_WORD_SIZE != CUEX_META_SIZE || CU_WORD_SIZE != CUCONF_SIZEOF_VOID_P
#  error Inconsistent sizes for cu_word_t, cuex_meta_t, and void *
#endif
#if 2*CU_WORD_SIZE/CU_GRAN_SIZE*CU_GRAN_SIZE != 2*CU_WORD_SIZE
#  error Hash-consed object headers are not multiples of granules.
#endif

/* In the arguments to cuexP_halloc_raw and cuooP_hxalloc_init_raw, sizeg is
 * the full size to be allocated, in granules.  This includes the cuex_meta_t
 * field in front of the returned pointer. */
void *
cuexP_halloc_raw(cuex_meta_t meta, size_t key_sizew, void *key)
{
    cuooP_hcobj_t *slot, obj;
    size_t mask;
    cu_hash_t hash;
    cuooP_hcset_t hcset;
    size_t sizeg;

    /* The size to allocate in words is 1 + CUOO_HCOBJ_SHIFTW + key_sizew, then
     * we add CU_GRAN_SIZEW - 1 for the upwards rounding. */
    sizeg = (key_sizew + CUOO_HCOBJ_SHIFTW + CU_GRAN_SIZEW)/CU_GRAN_SIZEW;

    cu_debug_assert(meta);
    hash = cu_wordarr_hash(key_sizew, key, meta);
#ifndef CU_NDEBUG
    if (cuex_meta_is_type(meta)) {
	cuoo_type_t type = cuoo_type_from_meta(meta);
	cu_debug_assert(cuoo_is_type(type) && cuoo_type_is_hctype(type));
    }
#endif
    hcset = cuooP_hcset(hash);
    cuooP_hcset_lock_write(hcset);
    mask = hcset->mask;

    /* If present, return existing object. */
    slot = &hcset->arr[hash & hcset->mask];
    for (obj = *slot; obj; obj = cuooP_hcset_hasheqv_next(obj)) {
	if (cuex_meta(obj) == meta
	    && cu_wordarr_eq(key_sizew, key, CUOO_HCOBJ_KEY(obj))) {
	    cuooP_hcobj_mark_lck(obj);
	    cuooP_hcset_unlock_write(hcset);
	    return obj;
	}
    }

    /* Otherwise, insert new object. */
#if CUOO_HC_ADJUST_IN_INSERT_ERASE
    if (hcset->cnt*FILL_MAX_DENOM > mask*FILL_MAX_NUMER) {
	size_t new_cap = (mask + 1)*2;
	cuooP_hcobj_t *new_arr = ARR_ALLOC(sizeof(cuooP_hcobj_t *)*new_cap);
	cuooP_hcset_grow_wlck(hcset, new_cap, new_arr);
	slot = &new_arr[hash & hcset->mask];
    }
#endif
    obj = cuexP_oalloc_unord_fin_raw(meta, sizeg);
    cu_wordarr_copy(key_sizew, CUOO_HCOBJ_KEY(obj), key);
#if CUPRIV_ENABLE_COLL_STATS
    if (*slot) ++_coll_count; else ++_noncoll_count;
#endif
    obj->hcset_next = ~(AO_t)*slot;
    cu_debug_assert((uintptr_t)obj->hcset_next != 0);
    *slot = obj;
    ++hcset->cnt;
    cuooP_hcset_unlock_write(hcset);

    return obj;
}

void *
cuexP_hxalloc_raw(cuex_meta_t meta, size_t sizeg, size_t key_sizew, void *key,
		  cu_clop(init_nonkey, void, void *))
{
    cuooP_hcobj_t *slot, obj;
    size_t mask;
    cu_hash_t hash;
    cuooP_hcset_t hcset;

    cu_debug_assert(meta);
    hash = cu_wordarr_hash(key_sizew, key, meta);
#ifndef CU_NDEBUG
    if (cuex_meta_is_type(meta)) {
	cuoo_type_t type = cuoo_type_from_meta(meta);
	cu_debug_assert(cuoo_is_type(type) && cuoo_type_is_hctype(type));
    }
#endif
    hcset = cuooP_hcset(hash);
    cuooP_hcset_lock_write(hcset);
    mask = hcset->mask;

    /* If present, return existing object. */
    slot = &hcset->arr[hash & hcset->mask];
    for (obj = *slot; obj; obj = cuooP_hcset_hasheqv_next(obj)) {
	if (cuex_meta(obj) == meta
	    && cu_wordarr_eq(key_sizew, key, CUOO_HCOBJ_KEY(obj))) {
	    cuooP_hcobj_mark_lck(obj);
	    cuooP_hcset_unlock_write(hcset);
	    return obj;
	}
    }

    /* Otherwise, insert new object. */
#if CUOO_HC_ADJUST_IN_INSERT_ERASE
    if (hcset->cnt*FILL_MAX_DENOM > mask*FILL_MAX_NUMER) {
	size_t new_cap = (mask + 1)*2;
	cuooP_hcobj_t *new_arr = ARR_ALLOC(sizeof(cuooP_hcobj_t *)*new_cap);
	cuooP_hcset_grow_wlck(hcset, new_cap, new_arr);
	slot = &new_arr[hash & hcset->mask];
    }
#endif
    obj = cuexP_oalloc_ord_fin_raw(meta, sizeg);
    cu_wordarr_copy(key_sizew, CUOO_HCOBJ_KEY(obj), key);
    cu_call(init_nonkey, obj);
#if CUPRIV_ENABLE_COLL_STATS
    if (*slot) ++_coll_count; else ++_noncoll_count;
#endif
    obj->hcset_next = ~(AO_t)*slot;
    cu_debug_assert((uintptr_t)obj->hcset_next != 0);
    *slot = obj;
    ++hcset->cnt;
    cuooP_hcset_unlock_write(hcset);

    return obj;
}

#ifdef CUOO_ENABLE_KEYED_PROP
extern pthread_mutex_t cuooP_property_mutex;
extern struct cucon_umap_s cuooP_property_map;
#endif

int
cuooP_hcons_disclaim_proc(void *obj, void *null)
{
    cuex_meta_t meta;
    cu_hash_t hash;
    cuooP_hcset_t hcset;

    meta = *(cuex_meta_t *)obj - 1;
    if (cuex_meta_kind(meta) == cuex_meta_kind_ignore)
	return 0;
    obj = (cuex_meta_t *)obj + 1;
#ifdef CUOO_ENABLE_KEYED_PROP
    if (cuooP_hcobj_has_prop(obj)) {
	cu_mutex_lock(&cuooP_property_mutex);
	cucon_umap_erase(&cuooP_property_map, ~(uintptr_t)obj);
	cu_mutex_unlock(&cuooP_property_mutex);
    }
#endif
#if CU_HCSET_CNT > 1
    hash = cuex_key_hash(obj);
    hcset = cuooP_hcset(hash);
    if (!cuooP_hcset_trylock_write(hcset))
	return 1;
    else if (cuooP_hcobj_is_marked(obj)) {
	cuooP_hcobj_unmark_lck(obj);
	cuooP_hcset_unlock_write(hcset);
	return 1;
    }
    else {
	cuooP_hcset_hasheqv_erase_wlck(hcset, hash, obj);
	cuooP_hcset_unlock_write(hcset);
    }
#else
    hcset = cuooP_hcset(0);
    if (!cuooP_hcset_trylock_write(hcset))
	return 1;
    if (cuooP_hcobj_is_marked(obj)) {
	cuooP_hcobj_unmark_lck(obj);
	cuooP_hcset_unlock_write(hcset);
	return 1;
    }
    else {
	hash = cuex_key_hash(obj);
	cuooP_hcset_hasheqv_erase_wlck(hcset, hash, obj);
	cuooP_hcset_unlock_write(hcset);
    }
#endif
#ifdef CUOO_INTF_FINALISE
    if (cuex_meta_is_type(meta)) {
	cuoo_type_t t = cuoo_type_from_meta(meta);
	if (t->shape & CUOO_SHAPEFLAG_FIN)
	    (*t->impl)(CUOO_INTF_FINALISE, obj);
    }
#endif
    return 0;
}

struct cuooP_hcset_s cuooP_hcset_arr[CU_HCSET_CNT];

#if CUOO_HC_GENERATION
AO_t cuooP_hcons_generation = 0;

extern void (*GC_start_call_back)(void);
static void (*_next_gc_start_callback)(void);

void
cu_gc_start_callback(void)
{
    AO_fetch_and_add_full(&cuooP_hcons_generation, 2);
    //printf("generation = %ld\n", cuooP_hcons_generation);
    if (_next_gc_start_callback)
	_next_gc_start_callback();
}

#endif

#if CUPRIV_ENABLE_COLL_STATS
void
coll_stats(void)
{
    int_least64_t cnt = _coll_count + _noncoll_count;
    fprintf(stderr, "Hash cons collisions: %lf of %"PRIi64" inserts\n",
	    _coll_count/(double)cnt, cnt);
}
#endif

void
cuooP_hcons_init()
{
    size_t i;
#if CUOO_HC_GENERATION
    _next_gc_start_callback = GC_start_call_back;
    GC_start_call_back = cu_gc_start_callback;
#endif
    for (i = 0; i < CU_HCSET_CNT; ++i)
	cuooP_hcset_init(&cuooP_hcset_arr[i]);
#if CUPRIV_ENABLE_COLL_STATS
    atexit(coll_stats);
#endif
}
