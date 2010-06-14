/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2010  Petter Urkedal <paurkedal@eideticdew.org>
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

#include <cuoo/oalloc.h>
#include <cuoo/intf.h>
#include <cu/wordarr.h>
#include <cu/size.h>

/* To reduce lock contention, several independent hash-consing sets are used,
 * indexed by the upper bits of the object hash.  CUOO_HSET_COUNT is the number
 * of sets, which must be a power of 2. */
#define CUOO_HSET_LOG_COUNT	5
#define CUOO_HSET_COUNT		(1 << CUOO_HSET_LOG_COUNT)

/* Each hash set has a local free-list for its internal nodes, which boosts
 * efficiency since we already holds the required lock when we need them. */
#define MAX_FREE_PAIR_COUNT 32
#define MAX_FREE_QUAD_COUNT 32

/* The minimum capacity and limits on the load before resizing a hash set. */
#define MIN_CAPACITY	8
#define MIN_LOAD_NUMER	3
#define MIN_LOAD_DENOM	8
#define MAX_LOAD_NUMER	3
#define MAX_LOAD_DENOM	2

/* Don't change these for production builds. */
#define VALIDATE_HSET	0  /* Very expensive, use only for debugging. */
#define USE_MALLOC	1  /* Don't use GC for internals due to locking. */
#define ENABLE_STATS	0


cu_dlog_def(_file, "dtag=cuoo.halloc");

#if USE_MALLOC
CU_SINLINE void *
_mallocz(size_t size)
{
    void *ptr = malloc(size);
    memset(ptr, 0, size);
    return ptr;
}
# define _unew_atomic(type)		((type *)malloc(sizeof(type)))
# define _unewarr_atomic(type, count)	((type *)malloc(sizeof(type) * count))
# define _unewarrz_atomic(type, count)	((type *)_mallocz(sizeof(type) * count))
# define _ufree_atomic			free
#else
# define _unew_atomic		cu_unew_atomic
# define _unewarr_atomic	cu_unewarr_atomic
# define _unewarrz_atomic	cu_unewarrz_atomic
# define _ufree_atomic		cu_ufree_atomic
#endif

#if ENABLE_STATS
# define IF_STATS(stmt) (stmt)
static size_t _stat_alloc_insert = 0;
static size_t _stat_alloc_found = 0;
static size_t _stat_xalloc_insert = 0;
static size_t _stat_xalloc_found = 0;
static size_t _stat_erase = 0;
static size_t _stat_missed_erase = 0;
#else
# define IF_STATS(stmt) ((void)0)
#endif

typedef struct _hset *_hset_t;
typedef struct _link *_link_t;
typedef struct _obj  *_obj_t;
typedef struct _pair *_pair_t;
typedef struct _quad *_quad_t;
typedef struct _freelist *_freelist_t;

struct _freelist
{
    _freelist_t next;
};

void GC_set_mark_bit(void *);

CU_SINLINE void
_obj_mark(void *obj)
{
    GC_set_mark_bit((cuex_meta_t *)obj - 1);
}

CU_SINLINE void *
_obj_new(cuex_meta_t meta, size_t key_sizew, void *key)
{
    void *obj;
    size_t sizeg;

    /* The size to allocate in words is 1 + CUOO_HCOBJ_SHIFTW + key_sizew, then
     * we add CU_GRAN_SIZEW - 1 for the upwards rounding to granules. */
    sizeg = (key_sizew + CUOO_HCOBJ_SHIFTW + CU_GRAN_SIZEW)/CU_GRAN_SIZEW;

    obj = cuexP_oalloc_unord_fin_raw(meta, sizeg);
    cu_wordarr_copy(key_sizew, obj, key);
    return obj;
}

CU_SINLINE void *
_xobj_new(cuex_meta_t meta, size_t sizeg, size_t key_sizew, void *key)
{
    void *obj = cuexP_oalloc_ord_fin_raw(meta, sizeg);
    cu_wordarr_copy(key_sizew, obj, key);
    return obj;
}

CU_SINLINE cu_bool_t
_obj_eq(cuex_meta_t meta0, size_t key0_sizew, _obj_t obj0, _obj_t obj1)
{
    return meta0 == cuex_meta(obj1)
	&& cu_wordarr_eq(key0_sizew, (cu_word_t *)obj0, (cu_word_t *)obj1);
}

typedef enum {
    OBJ_LINK = 0,
    PAIR_LINK,
    QUAD_LINK,
} _link_type_t;

#define _tag(p, tag)	((void *)((uintptr_t)(p) | (tag)))
#define _untag(p)	((void *)((uintptr_t)(p) & ~(uintptr_t)3))

CU_SINLINE _link_type_t _link_type(_link_t p) { return ((uintptr_t)p & 3); }

struct _pair {
    _obj_t obj[2];
};

struct _quad {
    _obj_t obj[3];
    _link_t link;
};

CU_SINLINE _obj_t  _link_as_obj(_link_t ln)  { return _untag(ln); }
CU_SINLINE _pair_t _link_as_pair(_link_t ln) { return _untag(ln); }
CU_SINLINE _quad_t _link_as_quad(_link_t ln) { return _untag(ln); }

CU_SINLINE _link_t _link_of_obj(_obj_t p)   { return _tag(p, OBJ_LINK); }
CU_SINLINE _link_t _link_of_pair(_pair_t p) { return _tag(p, PAIR_LINK); }
CU_SINLINE _link_t _link_of_quad(_quad_t p) { return _tag(p, QUAD_LINK); }

struct _hset
{
    cu_mutex_t mutex;
    size_t size, mask;
    _link_t *arr;
    unsigned short free_pair_count, free_quad_count;
    _freelist_t free_pairs;
    _freelist_t free_quads;
};

static void
_hset_init(_hset_t hset)
{
    cu_mutex_init(&hset->mutex);
    hset->size = 0;
    hset->mask = MIN_CAPACITY - 1;
    hset->arr = _unewarrz_atomic(_link_t, MIN_CAPACITY);
    hset->free_pair_count = hset->free_quad_count = 0;
    hset->free_pairs      = hset->free_quads      = NULL;
}

#if VALIDATE_HSET
static void
_hset_validate(_hset_t hset)
{
    size_t i;
    int j;
    size_t size = hset->size;
    size_t mask = hset->mask;
    cu_debug_assert(size * MIN_LOAD_DENOM >= mask * MIN_LOAD_NUMER ||
		    mask + 1 == MIN_CAPACITY);
    cu_debug_assert(size * MAX_LOAD_DENOM <= mask * MAX_LOAD_NUMER);
    for (i = 0; i <= mask; ++i) {
	_link_t link = hset->arr[i];
	_obj_t obj;
	_pair_t pair;
	_quad_t quad;
next_link:
	if (link) switch (_link_type(link)) {
	    case OBJ_LINK:
		--size;
		obj = _link_as_obj(link);
		cu_debug_assert((cuex_key_hash(obj) & mask) == i);
		break;
	    case PAIR_LINK:
		size -= 2;
		pair = _link_as_pair(link);
		for (j = 0; j < 2; ++j)
		    cu_debug_assert((cuex_key_hash(pair->obj[j]) & mask) == i);
		break;
	    case QUAD_LINK:
		size -= 3;
		quad = _link_as_quad(link);
		for (j = 0; j < 3; ++j)
		    cu_debug_assert((cuex_key_hash(quad->obj[j]) & mask) == i);
		link = quad->link;
		goto next_link;
	}
    }
    cu_debug_assert(size == 0);
}
#else
# define _hset_validate(hset) ((void)0)
#endif

CU_SINLINE _pair_t
_alloc_pair(_hset_t hset)
{
    if (hset->free_pair_count) {
	_freelist_t fl;
	--hset->free_pair_count;
	fl = hset->free_pairs;
	hset->free_pairs = fl->next;
	return (_pair_t)fl;
    }
    else
	return _unew_atomic(struct _pair);
}

CU_SINLINE _quad_t
_alloc_quad(_hset_t hset)
{
    if (hset->free_quad_count) {
	_freelist_t fl;
	--hset->free_quad_count;
	fl = hset->free_quads;
	hset->free_quads = fl->next;
	return (_quad_t)fl;
    }
    else
	return _unew_atomic(struct _quad);
}

/* Indiscriminately push a pair on the freelist.  For use in aggregate
 * operations link growing and shrinking.  Call _hset_prune_freelists before
 * returning to client code. */
CU_SINLINE void
_stash_pair(_hset_t hset, _pair_t pair)
{
    ((_freelist_t)pair)->next = hset->free_pairs;
    hset->free_pairs = (_freelist_t)pair;
    ++hset->free_pair_count;
}

/* Indiscriminately push a quad on the freelist.  For use in aggregate
 * operations link growing and shrinking.  Call _hset_prune_freelists before
 * returning to client code. */
CU_SINLINE void
_stash_quad(_hset_t hset, _quad_t quad)
{
    ((_freelist_t)quad)->next = hset->free_quads;
    hset->free_quads = (_freelist_t)quad;
    ++hset->free_quad_count;
}

CU_SINLINE void
_free_pair(_hset_t hset, _pair_t pair)
{
    if (hset->free_pair_count < MAX_FREE_PAIR_COUNT)
	_stash_pair(hset, pair);
    else
	_ufree_atomic(pair);
}

CU_SINLINE void
_free_quad(_hset_t hset, _quad_t quad)
{
    if (hset->free_quad_count < MAX_FREE_QUAD_COUNT)
	_stash_quad(hset, quad);
    else
	_ufree_atomic(quad);
}

/* Remove excess entries in the pair and quad freelists, which may have been
 * kept during aggregate processing. */
void
_hset_prune_freelists(_hset_t hset)
{
    _freelist_t fl;
    int count;

    count = hset->free_pair_count;
    if (count > MAX_FREE_PAIR_COUNT) {
	fl = hset->free_pairs;
	do {
	    void *top = fl;
	    fl = fl->next;
	    _ufree_atomic(top);
	} while (--count > MAX_FREE_PAIR_COUNT);
	hset->free_pairs = fl;
	hset->free_pair_count = MAX_FREE_PAIR_COUNT;
    }
    count = hset->free_quad_count;
    if (count > MAX_FREE_QUAD_COUNT) {
	fl = hset->free_quads;
	do {
	    void *top = fl;
	    fl = fl->next;
	    _ufree_atomic(top);
	} while (--count > MAX_FREE_QUAD_COUNT);
	hset->free_quads = fl;
	hset->free_quad_count = MAX_FREE_QUAD_COUNT;
    }
}

CU_SINLINE void
_hset_lock(_hset_t hset)
{ cu_mutex_lock(&hset->mutex); }

CU_SINLINE void
_hset_unlock(_hset_t hset)
{ cu_mutex_unlock(&hset->mutex); }

CU_SINLINE cu_bool_t
_hset_trylock(_hset_t hset)
{ return cu_mutex_trylock(&hset->mutex); }

/* Insert obj into *dst_slot. */
static void
_merge_obj(_hset_t hset, _link_t *dst_slot, _obj_t obj)
{
    _link_t link;
    _pair_t pair;
    _quad_t quad;

tail_call:
    link = *dst_slot;
    if (!link)
	*dst_slot = _link_of_obj(obj);
    else switch (_link_type(link)) {
	case OBJ_LINK:
	    pair = _alloc_pair(hset);
	    pair->obj[0] = _link_as_obj(link);
	    pair->obj[1] = obj;
	    *dst_slot = _link_of_pair(pair);
	    break;
	case PAIR_LINK:
	    pair = _link_as_pair(link);
	    quad = _alloc_quad(hset);
	    quad->obj[0] = pair->obj[0];
	    quad->obj[1] = pair->obj[1];
	    quad->obj[2] = obj;
	    quad->link = NULL;
	    _stash_pair(hset, pair);
	    *dst_slot = _link_of_quad(quad);
	    break;
	case QUAD_LINK:
	    dst_slot = &_link_as_quad(link)->link;
	    goto tail_call;
    }
}

/* Either merge src_pair into *dst_slot or insert its too components and free
 * src_pair. */
static void
_merge_pair(_hset_t hset, _link_t *dst_slot, _pair_t src_pair)
{
    _link_t link;
    _pair_t pair;
    _quad_t quad;

tail_call:
    link = *dst_slot;
    if (!link)
	*dst_slot = _link_of_pair(src_pair);
    else switch (_link_type(link)) {
	case OBJ_LINK:
	    quad = _alloc_quad(hset);
	    quad->obj[0] = src_pair->obj[0];
	    quad->obj[1] = src_pair->obj[1];
	    _stash_pair(hset, src_pair);
	    quad->obj[2] = _link_as_obj(link);
	    quad->link = NULL;
	    *dst_slot = _link_of_quad(quad);
	    break;
	case PAIR_LINK:
	    quad = _alloc_quad(hset);
	    quad->obj[0] = src_pair->obj[0];
	    quad->obj[1] = src_pair->obj[1];
	    _stash_pair(hset, src_pair);
	    pair = _link_as_pair(link);
	    quad->obj[2] = pair->obj[0];
	    quad->link = _link_of_obj(pair->obj[1]);
	    _stash_pair(hset, pair);
	    *dst_slot = _link_of_quad(quad);
	    break;
	case QUAD_LINK:
	    dst_slot = &_link_as_quad(link)->link;
	    goto tail_call;
    }
}

/* Either merge src_quad into *dst_slot or insert its components and free
 * src_quad. */
static void
_merge_quad(_hset_t hset, _link_t *dst_slot, _quad_t src_quad)
{
    _link_t link;

tail_call:
    link = *dst_slot;
    if (!link)
	*dst_slot = _link_of_quad(src_quad);
    else if (!src_quad->link) {
	src_quad->link = link;
	*dst_slot = _link_of_quad(src_quad);
    }
    else switch (_link_type(link)) {
	case OBJ_LINK:
	    _merge_obj(hset, &src_quad->link, _link_as_obj(link));
	    *dst_slot = _link_of_quad(src_quad);
	    break;
	case PAIR_LINK:
	    _merge_pair(hset, &src_quad->link, _link_as_pair(link));
	    *dst_slot = _link_of_quad(src_quad);
	    break;
	case QUAD_LINK:
	    dst_slot = &_link_as_quad(link)->link;
	    goto tail_call;
    }
}

static void
_hset_shrink(_hset_t hset, size_t dst_cap)
{
    size_t round_count, slot_count;
    _link_t *src_slot, *dst_slot;
    size_t src_cap = hset->mask + 1;
    _link_t *src_arr = hset->arr;
    _link_t *dst_arr = _unewarr_atomic(_link_t, dst_cap);

    cu_dlogf(_file, "Reducing HC set from %zd to %zd buckets.", src_cap, dst_cap);
    cu_debug_assert(src_cap > dst_cap);

    memcpy(dst_arr, src_arr, dst_cap * sizeof(_link_t));
    src_slot = src_arr + dst_cap;
    round_count = src_cap/dst_cap - 1;
    do {
	dst_slot = dst_arr;
	slot_count = dst_cap;
	do {
	    _link_t src_link, dst_link;

	    dst_link = *dst_slot;
	    src_link = *src_slot;
	    if (src_link) {
		if (!dst_link)
		    *dst_slot = src_link;
		else switch (_link_type(src_link)) {
		    case OBJ_LINK:
			_merge_obj(hset, dst_slot, _link_as_obj(src_link));
			break;
		    case PAIR_LINK:
			_merge_pair(hset, dst_slot, _link_as_pair(src_link));
			break;
		    case QUAD_LINK:
			_merge_quad(hset, dst_slot, _link_as_quad(src_link));
			break;
		}
	    }
	    ++dst_slot;
	    ++src_slot;
	} while (--slot_count);
    } while (--round_count);

    hset->arr = dst_arr;
    hset->mask = dst_cap - 1;
    _ufree_atomic(src_arr);
    _hset_prune_freelists(hset);
}

CU_SINLINE void
_hset_reinsert(_hset_t hset, _obj_t obj)
{
    _link_t *slot = &hset->arr[cuex_key_hash(obj) & hset->mask];
    _merge_obj(hset, slot, obj);
}

static void
_hset_grow(_hset_t hset, size_t dst_cap)
{
    size_t count = hset->mask + 1;
    _link_t *src_arr, *dst_arr, *src_slot;
    _pair_t pair;
    _quad_t quad;

    cu_dlogf(_file, "Extending HC set of size %zd from %zd to %zd buckets.",
	     hset->size, count, dst_cap);

    src_slot = src_arr = hset->arr;
    hset->arr  = dst_arr = _unewarrz_atomic(_link_t, dst_cap);
    hset->mask = dst_cap - 1;
    while (count--) {
	_link_t src_link = *src_slot;
	while (src_link) {
	    switch (_link_type(src_link)) {
		case OBJ_LINK:
		    _hset_reinsert(hset, _link_as_obj(src_link));
		    goto next_slot;
		case PAIR_LINK:
		    pair = _link_as_pair(src_link);
		    _hset_reinsert(hset, pair->obj[0]);
		    _hset_reinsert(hset, pair->obj[1]);
		    _stash_pair(hset, pair);
		    goto next_slot;
		case QUAD_LINK:
		    quad = _link_as_quad(src_link);
		    _hset_reinsert(hset, quad->obj[0]);
		    _hset_reinsert(hset, quad->obj[1]);
		    _hset_reinsert(hset, quad->obj[2]);
		    src_link = quad->link;
		    _stash_quad(hset, quad);
		    break;
	    }
	}
next_slot:
	++src_slot;
    }
    _hset_prune_freelists(hset);
    _ufree_atomic(src_arr);
}

static _obj_t
_hset_link_pop(_hset_t hset, _link_t *slot)
{
    for (;;) {
	_pair_t pair;
	_quad_t quad;
	_link_t link = *slot;
	switch (_link_type(link)) {
	    case OBJ_LINK:
		*slot = NULL;
		return _link_as_obj(link);
	    case PAIR_LINK:
		pair = _link_as_pair(link);
		*slot = _link_of_obj(pair->obj[0]);
		_stash_pair(hset, pair);
		return pair->obj[1];
	    case QUAD_LINK:
		quad = _link_as_quad(link);
		if (!quad->link) {
		    pair = _alloc_pair(hset);
		    pair->obj[0] = quad->obj[0];
		    pair->obj[1] = quad->obj[1];
		    *slot = _link_of_pair(pair);
		    _stash_quad(hset, quad);
		    return quad->obj[2];
		}
		slot = &quad->link;
		break;
	}
    }
}

static void
_hset_erase(_hset_t hset, cu_hash_t hash, _obj_t obj_erase)
{
    int i;
    _link_t *slot, link;
    _pair_t pair;
    _quad_t quad;
    size_t size, mask, new_cap;

    mask = hset->mask;
    slot = &hset->arr[hash & mask];

next_link:
    link = *slot;
    cu_debug_assert(link);
    if (_link_of_obj(obj_erase) == link)
	*slot = NULL;
    else switch (_link_type(link)) {
#ifndef CU_NDEBUG
	case OBJ_LINK:
	    cu_debug_unreachable();
	    break;
#endif
	case PAIR_LINK:
	    pair = _link_as_pair(link);
	    if (pair->obj[0] == obj_erase)
		*slot = _link_of_obj(pair->obj[1]);
	    else {
		cu_debug_assert(pair->obj[1] == obj_erase);
		*slot = _link_of_obj(pair->obj[0]);
	    }
	    _free_pair(hset, pair);
	    break;

	case QUAD_LINK:
	    quad = _link_as_quad(link);
	    for (i = 0; i < 3; ++i) {
		if (quad->obj[i] == obj_erase) {
		    if (!quad->link) {
			pair = _alloc_pair(hset);
			pair->obj[0] = quad->obj[(i + 1) % 3];
			pair->obj[1] = quad->obj[(i + 2) % 3];
			_free_quad(hset, quad);
			*slot = _link_of_pair(pair);
		    }
		    else
			quad->obj[i] = _hset_link_pop(hset, &quad->link);
		    goto break_switch;
		}
	    }
	    slot = &quad->link;
	    goto next_link;
    }

break_switch:
    size = --hset->size;
    if (size * MIN_LOAD_DENOM < mask * MIN_LOAD_NUMER && mask > MIN_CAPACITY) {
	new_cap = cu_size_exp2ceil(size * MAX_LOAD_DENOM / MAX_LOAD_NUMER);
	if (new_cap < MIN_CAPACITY)
	    new_cap = MIN_CAPACITY;
	_hset_shrink(hset, new_cap);
    }
}

static struct _hset _hset_arr[CUOO_HSET_COUNT];

CU_SINLINE _hset_t
_hset_for_hash(cu_hash_t hash)
{
#if CUOO_HSET_LOG_COUNT
    return &_hset_arr[hash >> (sizeof(cu_hash_t)*8 - CUOO_HSET_LOG_COUNT)];
#else
    return &_hset_arr[0];
#endif
}

void *
cuexP_halloc_raw(cuex_meta_t meta, size_t key_sizew, void *key)
{
    int i;
    _obj_t ret_obj;
    _link_t link, *slot;
    _pair_t pair;
    _quad_t quad;
    cu_hash_t hash;
    size_t mask;
    _hset_t hset;

    hash = cu_wordarr_hash(key_sizew, key, meta);
    hset = _hset_for_hash(hash);
    _hset_lock(hset);

    mask = hset->mask;
    slot = &hset->arr[hash & mask];

next_link:
    link = *slot;
    if (link == NULL) {
	ret_obj = _obj_new(meta, key_sizew, key);
	*slot = _link_of_obj(ret_obj);
	goto inserted;
    }
    else switch (_link_type(link)) {
	case OBJ_LINK:
	    ret_obj = _link_as_obj(link);
	    if (_obj_eq(meta, key_sizew, key, ret_obj))
		goto found;
	    else {
		pair = _alloc_pair(hset);
		pair->obj[0] = ret_obj;
		pair->obj[1] = ret_obj = _obj_new(meta, key_sizew, key);
		*slot = _link_of_pair(pair);
		goto inserted;
	    }

	case PAIR_LINK:
	    pair = _link_as_pair(link);
	    if (_obj_eq(meta, key_sizew, key, pair->obj[0])) {
		ret_obj = pair->obj[0];
		goto found;
	    }
	    else if (_obj_eq(meta, key_sizew, key, pair->obj[1])) {
		ret_obj = pair->obj[1];
		goto found;
	    }
	    else {
		quad = _alloc_quad(hset);
		quad->obj[0] = pair->obj[0];
		quad->obj[1] = pair->obj[1];
		quad->obj[2] = ret_obj = _obj_new(meta, key_sizew, key);
		quad->link = NULL;
		_free_pair(hset, pair);
		*slot = _link_of_quad(quad);
		goto inserted;
	    }

	case QUAD_LINK:
	    quad = _link_as_quad(link);
	    for (i = 0; i < 3; ++i)
		if (_obj_eq(meta, key_sizew, key, quad->obj[i])) {
		    ret_obj = quad->obj[i];
		    goto found;
		}
	    slot = &quad->link;
	    goto next_link;
    }
    cu_bug_unreachable();

inserted:
    cu_debug_assert(hash == cuex_key_hash(ret_obj));
    ++hset->size;
    if (hset->size * MAX_LOAD_DENOM > mask * MAX_LOAD_NUMER) {
	size_t new_cap = (mask + 1) * 2;
	_hset_grow(hset, new_cap);
    }
    _hset_validate(hset);
    _hset_unlock(hset);
    IF_STATS(++_stat_alloc_insert);
    return ret_obj;

found:
    _hset_unlock(hset);
    _obj_mark(ret_obj);
    cu_debug_assert(hash == cuex_key_hash(ret_obj));
    IF_STATS(++_stat_alloc_found);
    return ret_obj;
}

void *
cuexP_hxalloc_raw(cuex_meta_t meta, size_t sizeg, size_t key_sizew,
		  void *key, cu_clop(init_nonkey, void, void *))
{
    int i;
    _obj_t ret_obj;
    _link_t link, *slot;
    _pair_t pair;
    _quad_t quad;
    cu_hash_t hash;
    size_t mask;
    _hset_t hset;

    hash = cu_wordarr_hash(key_sizew, key, meta);
    hset = _hset_for_hash(hash);
    _hset_lock(hset);

    mask = hset->mask;
    slot = &hset->arr[hash & mask];

next_link:
    link = *slot;
    if (link == NULL) {
	ret_obj = _xobj_new(meta, sizeg, key_sizew, key);
	*slot = _link_of_obj(ret_obj);
	goto inserted;
    }
    else switch (_link_type(link)) {
	case OBJ_LINK:
	    ret_obj = _link_as_obj(link);
	    if (_obj_eq(meta, key_sizew, key, ret_obj))
		goto found;
	    else {
		pair = _alloc_pair(hset);
		pair->obj[0] = ret_obj;
		pair->obj[1] = ret_obj = _xobj_new(meta, sizeg, key_sizew, key);
		*slot = _link_of_pair(pair);
		goto inserted;
	    }

	case PAIR_LINK:
	    pair = _link_as_pair(link);
	    if (_obj_eq(meta, key_sizew, key, pair->obj[0])) {
		ret_obj = pair->obj[0];
		goto found;
	    }
	    else if (_obj_eq(meta, key_sizew, key, pair->obj[1])) {
		ret_obj = pair->obj[1];
		goto found;
	    }
	    else {
		quad = _alloc_quad(hset);
		quad->obj[0] = pair->obj[0];
		quad->obj[1] = pair->obj[1];
		quad->obj[2] = ret_obj = _xobj_new(meta, sizeg, key_sizew, key);
		quad->link = NULL;
		_free_pair(hset, pair);
		*slot = _link_of_quad(quad);
		goto inserted;
	    }

	case QUAD_LINK:
	    quad = _link_as_quad(link);
	    for (i = 0; i < 3; ++i)
		if (_obj_eq(meta, key_sizew, key, quad->obj[i])) {
		    ret_obj = quad->obj[i];
		    goto found;
		}
	    slot = &quad->link;
	    goto next_link;
    }
    cu_bug_unreachable();

inserted:
    cu_call(init_nonkey, ret_obj);
    cu_debug_assert(hash == cuex_key_hash(ret_obj));
    ++hset->size;
    if (hset->size * MAX_LOAD_DENOM > mask * MAX_LOAD_NUMER) {
	size_t new_cap = (mask + 1) * 2;
	_hset_grow(hset, new_cap);
    }
    _hset_validate(hset);
    _hset_unlock(hset);
    IF_STATS(++_stat_xalloc_insert);
    return ret_obj;

found:
    _hset_unlock(hset);
    _obj_mark(ret_obj);
    cu_debug_assert(hash == cuex_key_hash(ret_obj));
    IF_STATS(++_stat_xalloc_insert);
    return ret_obj;
}

int
cuooP_hcons_disclaim_proc(void *obj, void *null)
{
    cuex_meta_t meta;
    cu_hash_t hash;
    _hset_t hset;

    meta = *(cuex_meta_t *)obj - 1;
    if (cuex_meta_kind(meta) == cuex_meta_kind_ignore) /* If on free-list, */
	return 0;
    obj = (cuex_meta_t *)obj + 1;

    hash = cuex_key_hash(obj);
    hset = _hset_for_hash(hash);
    if (!_hset_trylock(hset)) {
	IF_STATS(++_stat_missed_erase);
	return 1;
    }
    IF_STATS(++_stat_erase);

    _hset_erase(hset, hash, obj);
    _hset_validate(hset);
    _hset_unlock(hset);

    /* Call finalizer if enabled and present. */
#ifdef CUOO_INTF_FINALISE
    if (cuex_meta_is_type(meta)) {
	cuoo_type_t t = cuoo_type_from_meta(meta);
	if (t->shape & CUOO_SHAPEFLAG_FIN)
	    (*t->impl)(CUOO_INTF_FINALISE, obj);
    }
#endif

    return 0;
}

#if ENABLE_STATS
static void
_dump_stats(void)
{
    int i;
    size_t obj_count = 0;
    for (i = 0; i < CUOO_HSET_COUNT; ++i)
	obj_count += _hset_arr[i].size;

#   define SHOW(var, what) printf("%9zd %s\n", var, what)
    printf("\nHash-consing statistics:\n");
    SHOW(_stat_alloc_insert,	"unique  allocations");
    SHOW(_stat_alloc_found,	"matched allocations");
    if (_stat_xalloc_insert || _stat_xalloc_found) {
	SHOW(_stat_xalloc_insert, "unique  allocations with aux data");
	SHOW(_stat_xalloc_found,  "matched allocations with aux data");
    }
    SHOW(_stat_erase,		"disclaims successful");
    SHOW(_stat_missed_erase,	"disclaims missed due to locking");
    SHOW(obj_count,		"objects left at exit");
#   undef SHOW
}
#endif

void
cuooP_hcons_init(void)
{
    size_t i;
    for (i = 0; i < CUOO_HSET_COUNT; ++i)
	_hset_init(&_hset_arr[i]);
    IF_STATS(atexit(_dump_stats));
}
