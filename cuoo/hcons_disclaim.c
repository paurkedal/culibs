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


/* Tuning Options
 * ============== */

/* The hash set is split into individual sets for ranges of hash codes to
 * avoid clogging locks.  1<<CUOO_HCSET_LOG_CNT should be greater than the
 * number of threads for best efficiency, maybe 4 times.  Using more is even
 * better, except that it is good to keep the size on the order of the cache
 * granularity to avoid wasting cache.
 *
 * 32 bit platforms (using cu_rarex_t):
 *     sizeof(struct _hcset_s) = 64 bytes
 *     CUOO_HCSET_LOG_CNT = 4:  16 hash sets gives 1 kiB
 *     CUOO_HCSET_LOG_CNT = 7: 128 hash sets gives 8 kiB
 * 64 bit platforms (using cu_rarex_t):
 *     sizeof(struct _hcset_s) = 128 bytes
 *     CUOO_HCSET_LOG_CNT = 4:  16 hash sets gives  2 kiB
 *     CUOO_HCSET_LOG_CNT = 7: 128 hash sets gives 16 kiB
 */
#if CUCONF_SIZEOF_LONG > 4
#  define CUOO_HCSET_LOG_CNT 5
#else
#  define CUOO_HCSET_LOG_CNT 7
#endif
#define CUOO_HCSET_CNT (1 << CUOO_HCSET_LOG_CNT)

/* Define non-zero to use cu/rarex.h for locking. */
#define CUOO_HCSET_USE_RAREX 0

/* Define non-zero to use GC_is_marked and GC_set_mark_bit instead of encoding
 * the mark in the object itself. */
#define CUOO_HC_USE_GC_MARK 1

/* The limits to the size of the hash-sets, expressed as fractions of the
 * capacity. */
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

/* Print collision statistics on exit to aid tuning. */
#define CUOO_ENABLE_COLL_STATS 0


#include <cuoo/hcobj.h>
#include <cuoo/oalloc.h>
#include <cu/memory.h>
#include <cu/int.h>
#include <cu/ptr.h>
#include <cu/wordarr.h>
#include <cu/conf.h>
#include <cu/thread.h>
#include <cu/debug.h>
#ifdef CUOO_ENABLE_KEYED_PROP
#  include <cucon/umap.h>
#endif
#include <inttypes.h>

#include <gc/gc_mark.h>
#if CUOO_HCSET_USE_RAREX
#  include <cu/rarex.h>
#endif
#include <atomic_ops.h>

cu_dlog_def(_file, "cuoo.hcons");

/* These are used to reallocate the global arrays. */
#if 1
#   define ARR_ALLOC(size) malloc(size)
#   define ARR_FREE(ptr) free(ptr)
#else
#   define ARR_ALLOC(size) cu_ualloc_atomic(size)
#   define ARR_FREE(ptr) cu_ufree_atomic(ptr)
#endif

#if CUOO_ENABLE_COLL_STATS
static int_least64_t _coll_count = 0;
static int_least64_t _noncoll_count = 0;
#endif


/* Access to Object Next-Link and Mark
 * =================================== */

#if CUOO_HC_GENERATION

static AO_t _g_hcons_generation = 0;

CU_SINLINE cu_bool_t
_hcobj_is_marked(cuooP_hcobj_t obj)
{ return obj->generation >= AO_load_read(&_g_hcons_generation); }

CU_SINLINE void
_hcobj_unmark_lck(cuooP_hcobj_t obj)
{}

CU_SINLINE void
_hcobj_mark_lck(cuooP_hcobj_t obj)
{ obj->generation = AO_load_read(&_g_hcons_generation)
		  | (obj->generation & 1); }

CU_SINLINE void
_hcobj_set_has_prop(cuooP_hcobj_t obj)
{ AO_or(&obj->generation, 1); }

CU_SINLINE cu_bool_t
_hcobj_has_prop(cuooP_hcobj_t obj)
{ return obj->generation & 1; }

#elif CUOO_HC_USE_GC_MARK

int GC_is_marked(void *);
void GC_set_mark_bit(void *);

CU_SINLINE cu_bool_t
_hcobj_is_marked(cuooP_hcobj_t obj)
{ return GC_is_marked(obj); }

CU_SINLINE void
_hcobj_unmark_lck(cuooP_hcobj_t obj)
{}

CU_SINLINE void
_hcobj_mark_lck(cuooP_hcobj_t obj)
{ GC_set_mark_bit(obj); }

#else

CU_SINLINE cu_bool_t
_hcobj_is_marked(cuooP_hcobj_t obj)
{ return ((uintptr_t)obj->hcset_next & 1); }

CU_SINLINE void
_hcobj_unmark_lck(cuooP_hcobj_t obj)
{ obj->hcset_next = obj->hcset_next & ~(AO_t)1; }

CU_SINLINE void
_hcobj_mark_lck(cuooP_hcobj_t obj)
{ obj->hcset_next = obj->hcset_next | 1; }

CU_SINLINE void
_hcobj_set_has_prop(cuooP_hcobj_t obj)
{ AO_or(&obj->hcset_next, 2); }

CU_SINLINE cu_bool_t
_hcobj_has_prop(cuooP_hcobj_t obj)
{ return obj->hcset_next & 2; }

#endif


/* The Global Hash Set
 * =================== */

typedef struct _hcset_s *_hcset_t;
struct _hcset_s
{
    size_t mask;
    cuooP_hcobj_t *arr;
    size_t cnt;
#ifdef CUCONF_ENABLE_THREADS
# if CUOO_HCSET_USE_RAREX
    cu_rarex_t rarex;
# else
    cu_mutex_t mutex;
# endif
#endif
};

static struct _hcset_s _g_hcset_arr[CUOO_HCSET_CNT];

CU_SINLINE _hcset_t
_hcset(cu_hash_t hash)
{
#if CUOO_HCSET_CNT > 1
    return &_g_hcset_arr[hash >> (sizeof(cu_hash_t)*8 - CUOO_HCSET_LOG_CNT)];
#else
    return &_g_hcset_arr[0];
#endif
}

#ifdef CUCONF_ENABLE_THREADS
# if CUOO_HCSET_USE_RAREX
CU_SINLINE void		_hcset_lock_read(_hcset_t hcset)
{ cu_rarex_lock_read(&hcset->rarex); }
CU_SINLINE void		_hcset_unlock_read(_hcset_t hcset)
{ cu_rarex_unlock_read(&hcset->rarex); }
CU_SINLINE void		_hcset_lock_write(_hcset_t hcset)
{ cu_rarex_lock_write(&hcset->rarex); }
CU_SINLINE cu_bool_t	_hcset_trylock_write(_hcset_t hcset)
{ return cu_rarex_trylock_write(&hcset->rarex); }
CU_SINLINE void		_hcset_unlock_write(_hcset_t hcset)
{ cu_rarex_unlock_write(&hcset->rarex); }
CU_SINLINE cu_bool_t	_hcset_try_promote_lock(_hcset_t hcset)
{ return cu_rarex_try_promote(&hcset->rarex); }
# else
CU_SINLINE void		_hcset_lock_read(_hcset_t hcset)
{ cu_mutex_lock(&hcset->mutex); }
CU_SINLINE void		_hcset_unlock_read(_hcset_t hcset)
{ cu_mutex_unlock(&hcset->mutex); }
CU_SINLINE void		_hcset_lock_write(_hcset_t hcset)
{ cu_mutex_lock(&hcset->mutex); }
CU_SINLINE cu_bool_t	_hcset_trylock_write(_hcset_t hcset)
{ return cu_mutex_trylock(&hcset->mutex); }
CU_SINLINE void		_hcset_unlock_write(_hcset_t hcset)
{ cu_mutex_unlock(&hcset->mutex); }
CU_SINLINE cu_bool_t	_hcset_try_promote_lock(_hcset_t hcset)
{ return cu_true; }
# endif
#else
CU_SINLINE void		_hcset_lock_read(_hcset_t hcset) {}
CU_SINLINE void		_hcset_unlock_read(_hcset_t hcset) {}
CU_SINLINE void		_hcset_lock_write(_hcset_t hcset) {}
CU_SINLINE cu_bool_t	_hcset_trylock_write(_hcset_t hcset)
{ return cu_true; }
CU_SINLINE void		_hcset_unlock_write(_hcset_t hcset) {}
CU_SINLINE cu_bool_t	_hcset_try_promote_lock(_hcset_t hcset)
{ return cu_true; }
#endif

CU_SINLINE cuooP_hcobj_t
_hcset_hasheqv_begin(_hcset_t hcset, cu_hash_t hash)
{ return hcset->arr[hash & hcset->mask]; }

CU_SINLINE cuooP_hcobj_t
_hcset_hasheqv_next(cuooP_hcobj_t obj)
{
#if CUOO_HC_GENERATION || CUOO_HC_USE_GC_MARK
    return (cuooP_hcobj_t)~obj->hcset_next;
#else
    return (cuooP_hcobj_t)~(obj->hcset_next | 3);
#endif
}

static void
_hcset_init(_hcset_t hcset)
{
    hcset->mask = CAP_MIN - 1;
    hcset->arr = ARR_ALLOC(sizeof(void *)*CAP_MIN);
    memset(hcset->arr, 0, sizeof(void *)*CAP_MIN);
    hcset->cnt = 0;
#ifdef CUCONF_ENABLE_THREADS
# if CUOO_HCSET_USE_RAREX
    cu_rarex_init(&hcset->rarex);
# else
    cu_mutex_init(&hcset->mutex);
# endif
#endif
}

static void
_hcset_shrink_wlck(_hcset_t hcset, size_t new_cap, cuooP_hcobj_t *new_arr)
{
    cu_hash_t hash;
    size_t old_cap = hcset->mask + 1;
    size_t new_mask = new_cap - 1;
    cuooP_hcobj_t *old_arr = hcset->arr;
    cuooP_hcobj_t *old_arr_end = old_arr + old_cap;
    cu_debug_assert(0 < new_cap && new_cap < old_cap);
    memset(new_arr, 0, sizeof(void *)*new_cap);
    cu_dlogf(_file, "New cap = %d\n", new_cap);
    hash = 0;
    while (old_arr != old_arr_end) {
	cuooP_hcobj_t obj = *old_arr;
	while (obj) {
	    cuooP_hcobj_t next_obj = _hcset_hasheqv_next(obj);
	    cuooP_hcobj_t *p = &new_arr[hash & new_mask];
	    cu_debug_assert(GC_base(obj) ==
			    cu_ptr_sub(obj, sizeof(cuex_meta_t)));
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
_hcset_grow_wlck(_hcset_t hcset, size_t new_cap, cuooP_hcobj_t *new_arr)
{
    size_t old_cap = hcset->mask + 1;
    size_t new_mask = new_cap - 1;
    cuooP_hcobj_t *old_arr = hcset->arr;
    cuooP_hcobj_t *old_arr_end = old_arr + old_cap;
    cu_debug_assert(new_cap > old_cap);
    memset(new_arr, 0, sizeof(void *)*new_cap);
    cu_dlogf(_file, "New cap = %d\n", new_cap);
    while (old_arr != old_arr_end) {
	cuooP_hcobj_t obj = *old_arr;
	while (obj) {
	    cuooP_hcobj_t next_obj = _hcset_hasheqv_next(obj);
	    cu_hash_t hash = cuex_key_hash(obj);
	    cuooP_hcobj_t *p = &new_arr[hash & new_mask];
	    cu_debug_assert(GC_base(obj) ==
			    cu_ptr_sub(obj, sizeof(cuex_meta_t)));
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

CU_SINLINE void
_hcset_hasheqv_erase_wlck(_hcset_t hcset, cu_hash_t hash, cuooP_hcobj_t obj)
{
    size_t cnt;
    cuooP_hcobj_t *p = &hcset->arr[hash & hcset->mask];
    cu_debug_assert(*p);
    if (*p == obj)
	*p = _hcset_hasheqv_next(obj);
    else {
	cuooP_hcobj_t obj0 = *p;
	cuooP_hcobj_t obj1;
	while ((obj1 = _hcset_hasheqv_next(obj0)) != obj) {
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
    if (cnt*FILL_MIN_DENOM < hcset->mask*FILL_MIN_NUMER
	    && hcset->mask > CAP_MIN) {
	size_t new_cap;
	cuooP_hcobj_t *new_arr;
	new_cap = cu_ulong_exp2_ceil_log2(cnt*FILL_MAX_DENOM/FILL_MAX_NUMER);
	if (new_cap < CAP_MIN)
	    new_cap = CAP_MIN;
	new_arr = ARR_ALLOC(sizeof(cuooP_hcobj_t *)*new_cap);
	_hcset_shrink_wlck(hcset, new_cap, new_arr);
    }
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
    _hcset_t hcset;
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
    hcset = _hcset(hash);
    _hcset_lock_write(hcset);
    mask = hcset->mask;

    /* If present, return existing object. */
    slot = &hcset->arr[hash & hcset->mask];
    for (obj = *slot; obj; obj = _hcset_hasheqv_next(obj)) {
	if (cuex_meta(obj) == meta
	    && cu_wordarr_eq(key_sizew, key, CUOO_HCOBJ_KEY(obj))) {
	    _hcobj_mark_lck(obj);
	    _hcset_unlock_write(hcset);
	    return obj;
	}
    }

    /* Otherwise, insert new object. */
    if (hcset->cnt*FILL_MAX_DENOM > mask*FILL_MAX_NUMER) {
	size_t new_cap = (mask + 1)*2;
	cuooP_hcobj_t *new_arr = ARR_ALLOC(sizeof(cuooP_hcobj_t *)*new_cap);
	_hcset_grow_wlck(hcset, new_cap, new_arr);
	slot = &new_arr[hash & hcset->mask];
    }
    obj = cuexP_oalloc_unord_fin_raw(meta, sizeg);
    cu_wordarr_copy(key_sizew, CUOO_HCOBJ_KEY(obj), key);
#if CUOO_ENABLE_COLL_STATS
    if (*slot) ++_coll_count; else ++_noncoll_count;
#endif
    obj->hcset_next = ~(AO_t)*slot;
    cu_debug_assert((uintptr_t)obj->hcset_next != 0);
    *slot = obj;
    ++hcset->cnt;
    _hcset_unlock_write(hcset);

    return obj;
}

void *
cuexP_hxalloc_raw(cuex_meta_t meta, size_t sizeg, size_t key_sizew, void *key,
		  cu_clop(init_nonkey, void, void *))
{
    cuooP_hcobj_t *slot, obj;
    size_t mask;
    cu_hash_t hash;
    _hcset_t hcset;

    cu_debug_assert(meta);
    hash = cu_wordarr_hash(key_sizew, key, meta);
#ifndef CU_NDEBUG
    if (cuex_meta_is_type(meta)) {
	cuoo_type_t type = cuoo_type_from_meta(meta);
	cu_debug_assert(cuoo_is_type(type) && cuoo_type_is_hctype(type));
    }
#endif
    hcset = _hcset(hash);
    _hcset_lock_write(hcset);
    mask = hcset->mask;

    /* If present, return existing object. */
    slot = &hcset->arr[hash & hcset->mask];
    for (obj = *slot; obj; obj = _hcset_hasheqv_next(obj)) {
	if (cuex_meta(obj) == meta
	    && cu_wordarr_eq(key_sizew, key, CUOO_HCOBJ_KEY(obj))) {
	    _hcobj_mark_lck(obj);
	    _hcset_unlock_write(hcset);
	    return obj;
	}
    }

    /* Otherwise, insert new object. */
    if (hcset->cnt*FILL_MAX_DENOM > mask*FILL_MAX_NUMER) {
	size_t new_cap = (mask + 1)*2;
	cuooP_hcobj_t *new_arr = ARR_ALLOC(sizeof(cuooP_hcobj_t *)*new_cap);
	_hcset_grow_wlck(hcset, new_cap, new_arr);
	slot = &new_arr[hash & hcset->mask];
    }
    obj = cuexP_oalloc_ord_fin_raw(meta, sizeg);
    cu_wordarr_copy(key_sizew, CUOO_HCOBJ_KEY(obj), key);
    cu_call(init_nonkey, obj);
#if CUOO_ENABLE_COLL_STATS
    if (*slot) ++_coll_count; else ++_noncoll_count;
#endif
    obj->hcset_next = ~(AO_t)*slot;
    cu_debug_assert((uintptr_t)obj->hcset_next != 0);
    *slot = obj;
    ++hcset->cnt;
    _hcset_unlock_write(hcset);

    return obj;
}

#ifdef CUOO_ENABLE_KEYED_PROP
extern pthread_mutex_t _property_mutex;
extern struct cucon_umap _property_map;
#endif

int
cuooP_hcons_disclaim_proc(void *obj, void *null)
{
    cuex_meta_t meta;
    cu_hash_t hash;
    _hcset_t hcset;

    meta = *(cuex_meta_t *)obj - 1;
    if (cuex_meta_kind(meta) == cuex_meta_kind_ignore)
	return 0;
    obj = (cuex_meta_t *)obj + 1;
#ifdef CUOO_ENABLE_KEYED_PROP
    if (_hcobj_has_prop(obj)) {
	cu_mutex_lock(&_property_mutex);
	cucon_umap_erase(&_property_map, ~(uintptr_t)obj);
	cu_mutex_unlock(&_property_mutex);
    }
#endif
#if CUOO_HCSET_CNT > 1
    hash = cuex_key_hash(obj);
    hcset = _hcset(hash);
    if (!_hcset_trylock_write(hcset))
	return 1;
    else if (_hcobj_is_marked(obj)) {
	_hcobj_unmark_lck(obj);
	_hcset_unlock_write(hcset);
	return 1;
    }
    else {
	_hcset_hasheqv_erase_wlck(hcset, hash, obj);
	_hcset_unlock_write(hcset);
    }
#else
    hcset = _hcset(0);
    if (!_hcset_trylock_write(hcset))
	return 1;
    if (_hcobj_is_marked(obj)) {
	_hcobj_unmark_lck(obj);
	_hcset_unlock_write(hcset);
	return 1;
    }
    else {
	hash = cuex_key_hash(obj);
	_hcset_hasheqv_erase_wlck(hcset, hash, obj);
	_hcset_unlock_write(hcset);
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

#if CUOO_HC_GENERATION
extern void (*GC_start_call_back)(void);
static void (*_next_gc_start_callback)(void);

static void
_gc_start_callback(void)
{
    AO_fetch_and_add_full(&_g_hcons_generation, 2);
    //printf("generation = %ld\n", _g_hcons_generation);
    if (_next_gc_start_callback)
	_next_gc_start_callback();
}

#endif

#if CUOO_ENABLE_COLL_STATS
static void
_collision_stats(void)
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
    GC_start_call_back = _gc_start_callback;
#endif
    for (i = 0; i < CUOO_HCSET_CNT; ++i)
	_hcset_init(&_g_hcset_arr[i]);
#if CUOO_ENABLE_COLL_STATS
    atexit(_collision_stats);
#endif
}
