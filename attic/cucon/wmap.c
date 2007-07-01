/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2002--2007  Petter Urkedal <urkedal@nbi.dk>
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

#include <cucon/wmap.h>
#include <cucon/fwd.h>
#include <cu/memory.h>
#include <cu/debug.h>
#include <cu/int.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>

/* Tuning options
 * ============== */

/* The initial and minimum size of the array indexed by the hash
 * codes.  This MUST BE A POWER OF 2.  It should be small, maybe 4 or
 * 8, since client may create many empty or small instances. */
#define MIN_SIZE 8

/* The maximum filling ratio before the array is expanded. */
#define MAX_FILL_NOM 2
#define MAX_FILL_DENOM 3

/* The minimum filling ratio before the array is shrunk. */
#define MIN_FILL_NOM 1
#define MIN_FILL_DENOM 3

/* Prime numbers used for hashing. */
#define PRIME0 10000849
#define PRIME1 17
#define PRIME2 5

/* Define this to make the wmap inaccessible to the garbage collector
 * from the keys.  XXX Does not work. */
/*#define CUCON_TUNING_WMAP_COLLECTABLE_BEFORE_KEYS 1*/

/* The following is for debugging this file. */
/* #define CUCON_WMAP_DISABLE_KEY_GC 1 */

/* The hash function. */
typedef unsigned long cucon_wmap_hash_t;
/* This is too weak. */
#define HASH0(key) ((key) + ((key) >> 17))
/* This should do it. */
#define HASH1(key) (((key) ^ ((key) >> 11)) + ((key) >> 19))
/* Or this. */
#define HASH2(key) (((key) ^ ((key) >> 7)) - (((key) >> 11) ^ ((key) >> 19)))
/* Or this. */
#define HASH3(key) ((key) ^ ((key)*PRIME0 >> 16))
/* This should be fast on 64 bit architectures. */
#define HASH4(key) (((uint64_t)(key))*PRIME0 >> 21)
#define HASH(key) HASH2((cucon_wmap_hash_t)(key))


#if 1
#  define D_PRINTF(args...) fprintf(stderr, args)
#else
#  define D_PRINTF(args...) ((void)0)
#endif


/* Implementation
 * ============== */

typedef struct key_finaliser_args_s *key_finaliser_args_t;
struct key_finaliser_args_s
{
    cucon_wmap_t wmap;				/* the wmap */
    GC_finalization_proc  key_finaliser_fn;	/* the old finaliser fn */
    void		 *key_finaliser_cd;	/* the old finaliser CD */
};

#ifndef CUCON_WMAP_DISABLE_KEY_GC
static void
key_finaliser_without_subfinaliser(void *key0, void *wmap)
{
    int i;
#ifdef CUCON_TUNING_WMAP_COLLECTABLE_BEFORE_KEYS
    wmap = cu_reveal_ptr((cu_hidden_ptr_t)wmap);
#endif
    D_PRINTF("erase key %p\n", key0);
    /* assert(key0 == cu_gc_base(key0)); */
    i = cuconP_wmap_erase((cucon_wmap_t)wmap, key0, 1);
    cu_debug_assert(i);
}
static void
key_finaliser_with_subfinaliser(void *key0, void *args0)
{
#define args ((struct key_finaliser_args_s *)args0)
    int i;
#ifdef CUCON_TUNING_WMAP_COLLECTABLE_BEFORE_KEYS
    args0 = cu_reveal_ptr((cu_hidden_ptr_t)args);
#endif
    assert(key0 == cu_gc_base(key0));
    i = cuconP_wmap_erase(args->wmap, key0, 1);
    assert(i);
    (*args->key_finaliser_fn)(key0, args->key_finaliser_cd);
    cu_mfree(args);
#undef args
}
#endif

#ifdef CUCON_TUNING_WMAP_COLLECTABLE_BEFORE_KEYS
static void
wmap_dct(cucon_wmap_t wmap, void *ign)
{
    /* Restore all key finalisers. */
    size_t i;
    D_PRINTF("Collecting wmap, size = %d\n", wmap->size);
    GC_invoke_finalizers();
    for (i = 0; i <= wmap->mask; ++i) {
	struct cuconP_wmap_node_s* node = wmap->arr[i];
	while (node != NULL) {
	    cu_ptr_t key = cu_reveal_ptr((cu_hidden_ptr_t)node->key);
	    GC_finalization_proc fn_f;
	    void *cd_f;
	    cu_gc_register_finaliser_no_order(key, NULL, NULL, &fn_f, &cd_f);
	    if (fn_f == key_finaliser_with_subfinaliser) {
		key_finaliser_args_t args = cd_f;
		cu_gc_register_finaliser_no_order(
		    key, args->key_finaliser_fn,
		    args->key_finaliser_cd, NULL, NULL);
		cu_mfree(args);
	    }
	    else
		assert(fn_f == key_finaliser_without_subfinaliser);
	    node = node->next;
	}
    }
}
#endif

void
cucon_wmap_cct(cucon_wmap_t wmap)
{
    wmap->arr = cu_galloc(sizeof(struct cuconP_wmap_node_s*)*(MIN_SIZE + 1));
    wmap->mask = MIN_SIZE - 1;
    wmap->size = 0;
    memset(wmap->arr, 0, MIN_SIZE*sizeof(struct cuconP_wmap_node_s*));
    wmap->arr[MIN_SIZE] = (void*)-1;
#ifdef CUCON_TUNING_WMAP_COLLECTABLE_BEFORE_KEYS
    cu_debug_assert(cu_gc_base(wmap) == wmap);
    cu_gc_register_finaliser(wmap, (GC_finalization_proc)wmap_dct,
			      NULL, NULL, NULL);
#endif
}

cucon_wmap_t
cucon_wmap_new()
{
    cucon_wmap_t wmap = cu_galloc(sizeof(struct cucon_wmap_s));
    cucon_wmap_cct(wmap);
    return wmap;
}

static void
set_capacity(cucon_wmap_t wmap, size_t new_cap)
{
    size_t i;
    struct cuconP_wmap_node_s** new_arr;
    struct cuconP_wmap_node_s** old_arr = wmap->arr;
    size_t old_cap = wmap->mask + 1;
    size_t new_mask;
    if (old_cap == new_cap)
	return;

    new_arr = cu_galloc(sizeof(cuconP_wmap_node_t)*(new_cap + 1));
    D_PRINTF("set_capacity: new_arr = %p\n", new_arr);

    /* Before calling this function, 'GC_invoke_finalizers' was
     * called, but to be really sure, check if 'cu_galloc' decided to
     * invoke more invokations of 'key_finaliser' on 'wmap'.  */
    if (old_cap != wmap->mask + 1) {
	D_PRINTF("cucon_wmap_t capacity has already changed, returning.\n");
	cu_gfree(new_arr);
	return;
    }

    D_PRINTF("set_capacity: Changing capacity of "
	     "#[cucon_wmap_t @%p size=%ld] from %ld to %ld.\n",
	     wmap, (long)wmap->size, (long)old_cap, (long)new_cap);
    wmap->mask = new_mask = new_cap - 1;
    wmap->arr = new_arr;
    memset(new_arr, 0, sizeof(cuconP_wmap_node_t)*new_cap);
    new_arr[new_cap] = (void *)-1;
    for (i = 0; i < old_cap; ++i) {
	cuconP_wmap_node_t node = old_arr[i];
	while (node) {
	    cuconP_wmap_node_t next = node->next;
	    size_t index = HASH(node->key) & new_mask;
	    node->next = new_arr[index];
	    new_arr[index] = node;
	    node = next;
	}
    }
    D_PRINTF("set_capacity: freeing %p\n", old_arr);
    cu_gfree(old_arr);
}

int
cucon_wmap_insert_mem(cucon_wmap_t wmap, void const *key0,
		      size_t size, cu_ptr_ptr_t value)
{
    cu_uintptr_t key;
    unsigned long hash;
    cu_debug_assert(key0 != NULL);
    D_PRINTF("insert key %p\n", key0);
    key = (cu_uintptr_t)cu_hide_ptr(key0);
tail_rec:
    hash = HASH(key) & wmap->mask;
    struct cuconP_wmap_node_s** node0 = &wmap->arr[hash];
    while (*node0) {
	if ((*node0)->key == key) {
	    if (value)
		*(void**)value = CU_ALIGNED_PTR_END(*node0);
	    return 0;
	}
	node0 = &(*node0)->next;
    }
    if (wmap->size*MAX_FILL_DENOM > wmap->mask*MAX_FILL_NOM) {
	set_capacity(wmap, (wmap->mask + 1)*2);
	goto tail_rec;
    }
    else {
	struct cuconP_wmap_node_s* node;
	++wmap->size;
	node = *node0 = cu_galloc(
	    CU_ALIGNED_SIZEOF(struct cuconP_wmap_node_s) + size);
	node->key = key;
	node->next = 0;

#ifndef CUCON_WMAP_DISABLE_KEY_GC
	/* Register finaliser on 'key' to erase this entry. */
#  ifndef CU_NDEBUG
	if (GC_base((void*)key0) == NULL)
	    fprintf(stderr,
		    "cucon_wmap_insert: "
		    "warning: key @%p seems to be non-GC.\n",
		    key0);
	else {
#  endif
	    GC_finalization_proc fin_fn;
	    void *fin_cd;
	    /* The following should be the '_IGNORE_SELF' or
	       '_NO_ORDER' version despite the comments in 'gc.h',
	       since 'key0' is not inspected by the finaliser at
	       all, but used as a hash key. */
	    cu_gc_register_finaliser_no_order(
		(void*)key0,
		(GC_finalization_proc)key_finaliser_without_subfinaliser,
#ifndef CUCON_TUNING_WMAP_COLLECTABLE_BEFORE_KEYS
		wmap,
#else
		cu_hide_ptr(wmap),
#endif
		&fin_fn, &fin_cd);
	    if (fin_fn) { /* Include old finaliser in closure args. */
		/* Need GC_malloc_atomic_uncollectable, but that
		 * is not guaranteed to be defined. */
		key_finaliser_args_t args
		    = cu_malloc(sizeof(struct key_finaliser_args_s));
		args->wmap = wmap;
		args->key_finaliser_fn = fin_fn;
		args->key_finaliser_cd = fin_cd;
#ifdef CUCON_TUNING_WMAP_COLLECTABLE_BEFORE_KEYS
		args = (struct key_finaliser_args_s *)cu_hide_ptr(args);
#endif
		cu_gc_register_finaliser_no_order(
		    (void*)key0,
		    (GC_finalization_proc)key_finaliser_with_subfinaliser,
		    args, NULL, NULL);
	    }
#  ifndef CU_NDEBUG
	}
#  endif
#endif /* !defined(CUCON_WMAP_DISABLE_KEY_GC) */

	if (value)
	    *(void**)value = CU_ALIGNED_PTR_END(node);
	return 1;
    }
}

int
cucon_wmap_insert_ptr(cucon_wmap_t wmap, void const *key0, void **ptr)
{
    void **pp;
    if (cucon_wmap_insert_mem(wmap, key0, sizeof(void*), (void *)&pp)) {
	*pp = *ptr;
	return 1;
    }
    else {
	*ptr = *pp;
	return 0;
    }
}

void *
cucon_wmap_replace_ptr(cucon_wmap_t wmap, void const *key, void *ptr)
{
    void **pp;
    if (cucon_wmap_insert_mem(wmap, key, sizeof(void*), (void *)&pp)) {
	*pp = ptr;
	return NULL;
    }
    else {
	void *oldptr = *pp;
	*pp = ptr;
	return oldptr;
    }
}

int
cuconP_wmap_erase(cucon_wmap_t wmap, void const *key0, int is_in_finaliser)
{
    cu_uintptr_t key = (cu_uintptr_t)cu_hide_ptr(key0);
    unsigned long hash = HASH(key) & wmap->mask;
    struct cuconP_wmap_node_s** node0 = &wmap->arr[hash];
    while (*node0) {
	if ((*node0)->key == key) {
	    *node0 = (*node0)->next;
	    if ((--wmap->size)*MIN_FILL_DENOM <= wmap->mask*MIN_FILL_NOM) {
		int new_cap;

		if (is_in_finaliser)
		    /* See if there are more keys which needs to be
		     * removed before changing capacity. */
		    GC_invoke_finalizers();
		else {
		    /* Unregister the finaliser. */
		    GC_finalization_proc fn_f;
		    void *cd_f;
		    D_PRINTF("cuconP_wmap_erase(%p, %p)\n", wmap, key0);
		    cu_gc_register_finaliser_no_order((void *)key0,
						       NULL, NULL, &fn_f, &cd_f);
		    if (fn_f == key_finaliser_with_subfinaliser) {
			key_finaliser_args_t args = cd_f;
			cu_gc_register_finaliser_no_order(
			    (void *)key0,
			    args->key_finaliser_fn, args->key_finaliser_cd,
			    NULL, NULL);
			cu_mfree(args);
		    }
		    else
			assert(fn_f == key_finaliser_without_subfinaliser);
		}

		/* Then change the capacity.  Shrinking may be more
		 * than a factor 2 due to other finalisers.
		 * 'set_capacity' will return immediately for all but
		 * one call with the same capacity request.  */
		new_cap = cu_ulong_exp2_ceil_log2(wmap->size + 5);
		if (new_cap > MIN_SIZE)
		    set_capacity(wmap, new_cap);
		else
		    set_capacity(wmap, MIN_SIZE);
	    }
	    return 1;
	}
	node0 = &(*node0)->next;
    }
    return 0;
}

int (cucon_wmap_erase)(cucon_wmap_t wmap, void const *key)
{ return cucon_wmap_erase(wmap, key); }

void *
cucon_wmap_find_mem(cucon_wmap_t wmap, void const *key0)
{
    cu_uintptr_t key = (cu_uintptr_t)cu_hide_ptr(key0);
    unsigned long hash = HASH(key) & wmap->mask;
    cuconP_wmap_node_t node0 = wmap->arr[hash];
    while (node0) {
	if (node0->key == key)
	    return CU_ALIGNED_PTR_END(node0);
	node0 = node0->next;
    }
    return NULL;
}

void *
cucon_wmap_find_ptr(cucon_wmap_t wmap, void const *key0)
{
    cu_uintptr_t key = (cu_uintptr_t)cu_hide_ptr(key0);
    unsigned long hash = HASH(key) & wmap->mask;
    cuconP_wmap_node_t node0 = wmap->arr[hash];
    while (node0) {
	if (node0->key == key)
	    return *(void**)CU_ALIGNED_PTR_END(node0);
	node0 = node0->next;
    }
    return NULL;
}

void
cucon_wmap_for(cucon_wmap_t wmap, cu_clop(cb, void, void const *, void *))
{
    size_t i;
    for (i = 0; i <= wmap->mask; ++i) {
	struct cuconP_wmap_node_s* node = wmap->arr[i];
	while (node != NULL) {
	    cu_call(cb, cu_reveal_ptr((cu_hidden_ptr_t)node->key),
		     CU_ALIGNED_PTR_END(node));
	    node = node->next;
	}
    }
}

void
cucon_wmap_for_keys(cucon_wmap_t wmap, cu_clop(cb, void, void const *))
{
    size_t i;
    for (i = 0; i <= wmap->mask; ++i) {
	struct cuconP_wmap_node_s* node = wmap->arr[i];
	while (node != NULL) {
	    cu_call(cb, cu_reveal_ptr((cu_hidden_ptr_t)node->key));
	    node = node->next;
	}
    }
}

void
cucon_wmap_show_stats(cucon_wmap_t wmap)
{
    double avg_depth;
    size_t const n_profile = 8;
    size_t a_profile[n_profile];
    size_t i;
    int max_cnt = 0;
    memset(a_profile, 0, n_profile*sizeof(size_t));
    for (i = 0; i <= wmap->mask; ++i) {
	struct cuconP_wmap_node_s* node = wmap->arr[i];
	size_t cnt = 0;
	while (node) {
	    if (cnt < n_profile)
		++a_profile[cnt];
	    ++cnt;
	    node = node->next;
	}
	if (cnt > max_cnt)
	    max_cnt = cnt;
    }
    printf("debug: cucon_wmap: depth profile:");
    avg_depth = 0;
    for (i = 0; i < n_profile; ++i) {
	double w;
	if (i > max_cnt)
	    break;
	w = a_profile[i]/(double)wmap->size;
	printf(" %5.3f", w);
	avg_depth += i*w;
    }
    printf("\n");
    printf("debug: cucon_wmap:     gives max_depth = %d, avg_depth = %f\n",
	   max_cnt, avg_depth);
}

cucon_wmap_it_t
cucon_wmap_begin(cucon_wmap_t wmap)
{
    cucon_wmap_it_t it;
    if (wmap->size == 0) {
	it.node_head = wmap->arr + (wmap->mask + 1);
	it.node = (void*)-1;
    }
    else {
	it.node_head = wmap->arr;
	while (*it.node_head == NULL)
	    ++it.node_head;
	it.node = *it.node_head;
    }
    return it;
}

cucon_wmap_it_t
cucon_wmap_end(cucon_wmap_t wmap)
{
    cucon_wmap_it_t it;
    it.node_head = wmap->arr + (wmap->mask + 1);
    it.node = (void*)-1;
    return it;
}

cucon_wmap_it_t
cucon_wmap_it_next(cucon_wmap_it_t it)
{
    it.node = it.node->next;
    if (it.node == NULL) {
	do
	    ++it.node_head;
	while (*it.node_head == NULL);
	it.node = *it.node_head;
    }
    return it;
}

int (cucon_wmap_insert_void)(cucon_wmap_t wmap, void const *key)
{ return cucon_wmap_insert_void(wmap, key); }
