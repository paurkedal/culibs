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

#include <cucon/pmap.h>
#include <cucon/umap.h>
#include <cucon/pset.h>
#include <cucon/fwd.h>
#include <cu/memory.h>
#include <cu/debug.h>
#include <cu/int.h>
#include <cu/idr.h>
#include <cu/util.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <limits.h>
#include <inttypes.h>


/* Tuning options
 * ============== */

/* The initial and minimum size of the array indexed by the hash
 * codes.  This MUST BE A POWER OF 2.  It should be small, maybe 4 or
 * 8, since client may create many empty or small instances. */
#define MIN_SIZE 8

/* The minimum/maximum filling ratio before the array is shrunk/expanded. */
#ifdef CUCONF_OPT_SPEED
#  define MIN_FILL_NUMER 1
#  define MIN_FILL_DENOM 3
#  define MAX_FILL_NUMER 2
#  define MAX_FILL_DENOM 3
#else
#  define MIN_FILL_NUMER 1
#  define MIN_FILL_DENOM 2
#  define MAX_FILL_NUMER 3
#  define MAX_FILL_DENOM 2
#endif

/* Prime numbers used for hashing. */
#define PRIME0 10000849
#define PRIME1 17
#define PRIME2 5

/* The hash function. */
typedef unsigned long cucon_pmap_hash_t;
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
#define HASH(key) HASH2((cucon_pmap_hash_t)(key))


#if 0
#  define D_PRINTF(args...) fprintf(stderr, args)
#else
#  define D_PRINTF(args...) ((void)0)
#endif


/* Implementation
 * ============== */

void
cucon_umap_init(cucon_umap_t umap)
{
    umap->arr = cu_galloc(sizeof(cucon_umap_node_t)
			   *(MIN_SIZE + 1));
    umap->mask = MIN_SIZE - 1;
    umap->size = 0;
    memset(umap->arr, 0, MIN_SIZE*sizeof(cucon_umap_node_t));
    umap->arr[MIN_SIZE] = (void*)-1;
}

cucon_umap_t
cucon_umap_new()
{
    cucon_umap_t umap = cu_galloc(sizeof(struct cucon_umap_s));
    cucon_umap_init(umap);
    return umap;
}

void
cucon_umap_init_copy_void(cucon_umap_t dst, cucon_umap_t src)
{
    size_t N = src->mask;
    size_t n;
    dst->size = src->size;
    dst->mask = N;
    ++N;
    dst->arr = cu_galloc(sizeof(cucon_umap_node_t)*(N + 1));
    for (n = 0; n < N; ++n) {
	cucon_umap_node_t src_node = src->arr[n];
	cucon_umap_node_t *dst_node = &dst->arr[n];
	while (src_node) {
	    *dst_node = cu_gnew(struct cucon_umap_node_s);
	    (*dst_node)->key = src_node->key;
	    dst_node = &(*dst_node)->next;
	    src_node = src_node->next;
	}
	*dst_node = NULL;
    }
    dst->arr[N] = (void *)-1;
}

cucon_umap_t
cucon_umap_new_copy_void(cucon_umap_t src)
{
    cucon_umap_t dst = cu_gnew(struct cucon_umap_s);
    cucon_umap_init_copy_void(dst, src);
    return dst;
}

void
cucon_umap_init_copy_mem(cucon_umap_t dst, cucon_umap_t src, size_t slot_size)
{
    size_t N = src->mask;
    size_t n;
    size_t full_size = sizeof(struct cucon_umap_node_s) + slot_size;
    dst->size = src->size;
    dst->mask = N;
    ++N;
    dst->arr = cu_galloc(sizeof(cucon_umap_node_t)*(N + 1));
    for (n = 0; n < N; ++n) {
	cucon_umap_node_t src_node = src->arr[n];
	cucon_umap_node_t *dst_node = &dst->arr[n];
	while (src_node) {
	    *dst_node = cu_galloc(full_size);
	    memcpy(*dst_node, src_node, full_size);
	    dst_node = &(*dst_node)->next;
	    src_node = src_node->next;
	}
	*dst_node = NULL;
    }
    dst->arr[N] = (void *)-1;
}

void
cucon_umap_init_copy_mem_ctor(
    cucon_umap_t dst, cucon_umap_t src, size_t slot_size,
    cu_clop(value_cct_copy, void, void *, void *, uintptr_t))
{
    size_t N = src->mask;
    size_t n;
    dst->size = src->size;
    dst->mask = N;
    ++N;
    dst->arr = cu_galloc(sizeof(cucon_umap_node_t)*(N + 1));
    for (n = 0; n < N; ++n) {
	cucon_umap_node_t src_node = src->arr[n];
	cucon_umap_node_t *dst_node = &dst->arr[n];
	while (src_node) {
	    *dst_node = cu_galloc(CU_ALIGNED_SIZEOF(struct cucon_umap_node_s)
				   + slot_size);
	    (*dst_node)->key = src_node->key;
	    cu_call(value_cct_copy,
		    CU_ALIGNED_PTR_END(*dst_node),
		    CU_ALIGNED_PTR_END(src_node),
		    src_node->key);
	    dst_node = &(*dst_node)->next;
	    src_node = src_node->next;
	}
	*dst_node = NULL;
    }
    dst->arr[N] = (void *)-1;
}

void
cucon_umap_init_copy_node(
    cucon_umap_t dst, cucon_umap_t src,
    cu_clop(node_new_copy, cucon_umap_node_t, void *, uintptr_t))
{
    size_t N = src->mask;
    size_t n;
    dst->size = src->size;
    dst->mask = N;
    ++N;
    dst->arr = cu_galloc(sizeof(cucon_umap_node_t)*(N + 1));
    for (n = 0; n < N; ++n) {
	cucon_umap_node_t src_node = src->arr[n];
	cucon_umap_node_t *dst_node = &dst->arr[n];
	while (src_node) {
	    *dst_node = cu_call(node_new_copy,
				 CU_ALIGNED_PTR_END(src_node),
				 src_node->key);
	    (*dst_node)->key = src_node->key;
	    dst_node = &(*dst_node)->next;
	    src_node = src_node->next;
	}
	*dst_node = NULL;
    }
    dst->arr[N] = (void*)-1;
}

void
cucon_umap_swap(cucon_umap_t umap0, cucon_umap_t umap1)
{
    struct cucon_umap_s umap2;
    memcpy(&umap2, umap0, sizeof(umap2));
    memcpy(umap0, umap1, sizeof(umap2));
    memcpy(umap1, &umap2, sizeof(umap2));
}

static void
set_capacity(cucon_umap_t umap, size_t new_cap)
{
    size_t i;
    cucon_umap_node_t *new_arr;
    cucon_umap_node_t *old_arr = umap->arr;
    size_t old_cap = umap->mask + 1;
    size_t new_mask;
    if (old_cap == new_cap)
	return;

    new_arr = cu_galloc(sizeof(cucon_umap_node_t)*(new_cap + 1));

    /* 2007-12-19 Keeping old comment for reference:
     * Before calling this function, 'GC_invoke_finalizers' was
     * called, but to be really sure, check if 'cu_galloc' decided to
     * invoke more invokations of 'key_finaliser' on 'umap'.  */
    if (old_cap != umap->mask + 1) {
	D_PRINTF("cucon_umap_t capacity has already changed, returning.\n");
	cu_gfree(new_arr);
	return;
    }

    D_PRINTF("Changing capacity of #[cucon_umap_t @%p size=%d] from %d to %d.\n",
	     umap, umap->size, old_cap, new_cap);
    umap->mask = new_mask = new_cap - 1;
    umap->arr = new_arr;
    memset(new_arr, 0, sizeof(cucon_umap_node_t)*new_cap);
    new_arr[new_cap] = (void *)-1;
    for (i = 0; i < old_cap; ++i) {
	cucon_umap_node_t node = old_arr[i];
	while (node) {
	    cucon_umap_node_t next = node->next;
	    size_t index = HASH(node->key) & new_mask;
	    node->next = new_arr[index];
	    new_arr[index] = node;
	    node = next;
	}
    }
    cu_gfree(old_arr);
}

CU_SINLINE void
update_cap_after_erase(cucon_umap_t umap)
{
    if (umap->size*MIN_FILL_DENOM <= umap->mask*MIN_FILL_NUMER) {
	size_t new_cap = cu_ulong_exp2_ceil_log2(umap->size + 1);
	if (new_cap > MIN_SIZE)
	    set_capacity(umap, new_cap);
	else
	    set_capacity(umap, MIN_SIZE);
    }
}

CU_SINLINE void
update_cap_after_insert(cucon_umap_t umap)
{
    if (umap->size*MAX_FILL_DENOM > umap->mask*MAX_FILL_NUMER) {
	size_t new_cap = cu_ulong_exp2_ceil_log2(umap->size + 1);
	set_capacity(umap, new_cap);
    }
}

void
cucon_umap_update_cap(cucon_umap_t umap)
{
    if (umap->size*MAX_FILL_DENOM > umap->mask*MAX_FILL_NUMER) {
	size_t new_cap = cu_ulong_exp2_ceil_log2(umap->size + 1);
	set_capacity(umap, new_cap);
    }
    else if (umap->size*MIN_FILL_DENOM <= umap->mask*MIN_FILL_NUMER) {
	size_t new_cap = cu_ulong_exp2_ceil_log2(umap->size + 1);
	if (new_cap > MIN_SIZE)
	    set_capacity(umap, new_cap);
	else
	    set_capacity(umap, MIN_SIZE);
    }
}

cu_bool_t
cucon_umap_insert_general(cucon_umap_t umap, uintptr_t key,
			  cu_clop0(alloc_node, cucon_umap_node_t),
			  cucon_umap_node_t *node_out)
{
    size_t index;
    cucon_umap_node_t *node0;
    cucon_umap_node_t node;

    index = HASH(key) & umap->mask;
    node0 = &umap->arr[index];
    while (*node0) {
	if ((*node0)->key == key) {
	    if (node_out)
		*node_out = *node0;
	    return cu_false;
	}
	node0 = &(*node0)->next;
    }
    ++umap->size;
    node = *node0 = cu_call0(alloc_node);
    node->key = key;
    node->next = NULL;
    if (node_out)
	*node_out = node;
    update_cap_after_insert(umap);
    return cu_true;
}

cu_bool_t
cucon_umap_insert_new_node(cucon_umap_t map, uintptr_t key,
			   size_t node_size, cu_ptr_ptr_t node_out)
{
    size_t index;
    cucon_umap_node_t *head, node;

    index = HASH(key) & map->mask;
    head = &map->arr[index];
    node = *head;
    while (node) {
	if (node->key == key) {
	    *(cucon_umap_node_t *)node_out = node;
	    return cu_false;
	}
	node = node->next;
    }
    ++map->size;
    node = cu_galloc(node_size);
    node->key = key;
    node->next = *head;
    *head = *(cucon_umap_node_t *)node_out = node;
    update_cap_after_insert(map);
    return cu_true;
}

cu_bool_t
cucon_umap_insert_init_node(cucon_umap_t umap, cucon_umap_node_t node)
{
    size_t index;
    cucon_umap_node_t *node0;

    index = HASH(node->key) & umap->mask;
    node0 = &umap->arr[index];
    while (*node0) {
	if ((*node0)->key == node->key)
	    return cu_false;
	node0 = &(*node0)->next;
    }
    ++umap->size;
    node->next = NULL;
    *node0 = node;
    update_cap_after_insert(umap);
    return cu_true;
}

cu_bool_t
cucon_umap_insert_mem(cucon_umap_t umap, uintptr_t key, size_t size,
		      cu_ptr_ptr_t value)
{
    size_t index;
    cucon_umap_node_t *node0;
    cucon_umap_node_t node;

    index = HASH(key) & umap->mask;
    node0 = &umap->arr[index];
    while (*node0) {
	if ((*node0)->key == key) {
	    if (value)
		*(void **)value = CU_ALIGNED_PTR_END(*node0);
	    return cu_false;
	}
	node0 = &(*node0)->next;
    }
    ++umap->size;
    node = *node0
	= cu_galloc(CU_ALIGNED_SIZEOF(struct cucon_umap_node_s) + size);
    node->key = key;
    node->next = NULL;
    if (value)
	*(void **)value = CU_ALIGNED_PTR_END(node);
    update_cap_after_insert(umap);
    return cu_true;
}

cu_bool_t
cucon_umap_insert_ptr(cucon_umap_t umap, uintptr_t key, void *ptr)
{
    void **pp;
    if (cucon_umap_insert_mem(umap, key, sizeof(void *), (void *)&pp)) {
	*pp = ptr;
	return cu_true;
    }
    else
	return cu_false;
}

cu_bool_t
cucon_umap_insert_int(cucon_umap_t map, uintptr_t key, int val)
{
    int *i;
    if (cucon_umap_insert_mem(map, key, sizeof(int), (void *)&i)) {
	*i = val;
	return cu_true;
    }
    else
	return cu_false;
}

void *
cucon_umap_replace_ptr(cucon_umap_t umap, uintptr_t key, void *ptr)
{
    void **pp;
    if (!ptr)
	return cucon_umap_erase_ptr(umap, key);
    else if (cucon_umap_insert_mem(umap, key, sizeof(void*), (void *)&pp)) {
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
cucon_umap_replace_int(cucon_umap_t map, uintptr_t key, int val)
{
    int *slot;
    if (val == INT_MIN)
	return cucon_umap_erase_int(map, key);
    else if (cucon_umap_insert_mem(map, key, sizeof(int), &slot)) {
	*slot = val;
	return INT_MIN;
    }
    else {
	int old_val = *slot;
	*slot = val;
	return old_val;
    }
}

cu_bool_t
cucon_umap_erase_keep_cap(cucon_umap_t umap, uintptr_t key)
{
    size_t index = HASH(key) & umap->mask;
    cucon_umap_node_t *node0 = &umap->arr[index];
    while (*node0) {
	if ((*node0)->key == key) {
	    *node0 = (*node0)->next;
	    --umap->size;
	    return cu_true;
	}
	node0 = &(*node0)->next;
    }
    return cu_false;
}

cu_bool_t
cucon_umap_erase(cucon_umap_t umap, uintptr_t key)
{
    unsigned long index = HASH(key) & umap->mask;
    cucon_umap_node_t *node0 = &umap->arr[index];
    while (*node0) {
	if ((*node0)->key == key) {
	    *node0 = (*node0)->next;
	    --umap->size;
	    update_cap_after_erase(umap);
	    return cu_true;
	}
	node0 = &(*node0)->next;
    }
    return cu_false;
}

void *
cucon_umap_erase_ptr(cucon_umap_t map, uintptr_t key)
{
    unsigned long index = HASH(key) & map->mask;
    cucon_umap_node_t *node0 = &map->arr[index];
    while (*node0) {
	if ((*node0)->key == key) {
	    void *ptr = *(void **)CU_ALIGNED_PTR_END(*node0);
	    *node0 = (*node0)->next;
	    --map->size;
	    update_cap_after_erase(map);
	    return ptr;
	}
	node0 = &(*node0)->next;
    }
    return NULL;
}

int
cucon_umap_erase_int(cucon_umap_t map, uintptr_t key)
{
    unsigned long index = HASH(key) & map->mask;
    cucon_umap_node_t *node = &map->arr[index];
    while (*node) {
	if ((*node)->key == key) {
	    int val = *(int *)CU_ALIGNED_PTR_END(*node);
	    *node = (*node)->next;
	    --map->size;
	    update_cap_after_erase(map);
	    return val;
	}
	node = &(*node)->next;
    }
    return INT_MIN;
}

cucon_umap_node_t
cucon_umap_pop_any_node(cucon_umap_t map)
{
    cucon_umap_node_t *head, node;
    cu_debug_assert(!cucon_umap_is_empty(map));
    head = &map->arr[lrand48() & map->mask];
    while (*head == NULL) {
	++head;
	if (head - map->arr > map->mask)
	    head = &map->arr[0];
    }
    node = *head;
    *head = node->next;
    --map->size;
    update_cap_after_erase(map);
    return node;
}

cucon_umap_node_t
cucon_umap_find_node(cucon_umap_t umap, uintptr_t key)
{
    cu_hash_t hash = HASH(key) & umap->mask;
    cucon_umap_node_t node = umap->arr[hash];
    while (node) {
	if (node->key == key)
	    return node;
	node = node->next;
    }
    return NULL;
}

void *
cucon_umap_find_mem(cucon_umap_t umap, uintptr_t key)
{
    unsigned long hash = HASH(key) & umap->mask;
    struct cucon_umap_node_s *node0 = umap->arr[hash];
    while (node0) {
	if (node0->key == key)
	    return CU_ALIGNED_PTR_END(node0);
	node0 = node0->next;
    }
    return NULL;
}

void *
cucon_umap_find_ptr(cucon_umap_t umap, uintptr_t key)
{
    unsigned long hash = HASH(key) & umap->mask;
    struct cucon_umap_node_s *node0 = umap->arr[hash];
    while (node0) {
	if (node0->key == key)
	    return *(void**)CU_ALIGNED_PTR_END(node0);
	node0 = node0->next;
    }
    return NULL;
}

int
cucon_umap_find_int(cucon_umap_t map, uintptr_t key)
{
    unsigned long hash = HASH(key) & map->mask;
    cucon_umap_node_t node = map->arr[hash];
    while (node) {
	if (node->key == key)
	    return *(int *)CU_ALIGNED_PTR_END(node);
	node = node->next;
    }
    return INT_MIN;
}

void
cucon_pmap_iter_mem(cucon_pmap_t pmap,
		    cu_clop(cb, void, void const *, void *))
{
    size_t i;
    for (i = 0; i <= pmap->impl.mask; ++i) {
	cucon_umap_node_t node = pmap->impl.arr[i];
	while (node != NULL) {
	    cu_call(cb, (void*)node->key, CU_ALIGNED_PTR_END(node));
	    node = node->next;
	}
    }
}

void
cucon_pmap_iter_ptr(cucon_pmap_t pmap,
		    cu_clop(cb, void, void const *, void *))
{
    size_t i;
    for (i = 0; i <= pmap->impl.mask; ++i) {
	cucon_umap_node_t node = pmap->impl.arr[i];
	while (node != NULL) {
	    cu_call(cb, (void*)node->key,
		     *(void **)CU_ALIGNED_PTR_END(node));
	    node = node->next;
	}
    }
}

void
cucon_umap_iter_mem(cucon_umap_t umap, cu_clop(cb, void, uintptr_t, void *))
{
    size_t i;
    for (i = 0; i <= umap->mask; ++i) {
	cucon_umap_node_t node = umap->arr[i];
	while (node != NULL) {
	    cu_call(cb, node->key, CU_ALIGNED_PTR_END(node));
	    node = node->next;
	}
    }
}

cu_bool_t
cucon_umap_conj_general(cucon_umap_t map, size_t node_offset,
			cu_clop(cb, cu_bool_t, uintptr_t, void *))
{
    size_t i;
    for (i = 0; i <= map->mask; ++i) {
	cucon_umap_node_t node = map->arr[i];
	while (node) {
	    if (!cu_call(cb, node->key, (void *)node - node_offset))
		return cu_false;
	    node = node->next;
	}
    }
    return cu_true;
}

cu_bool_t
cucon_pmap_conj_general(cucon_pmap_t map, size_t node_offset,
			cu_clop(cb, cu_bool_t, void const *, void *))
{
    size_t i;
    for (i = 0; i <= map->impl.mask; ++i) {
	cucon_umap_node_t node = map->impl.arr[i];
	while (node) {
	    if (!cu_call(cb, (void const *)node->key,
			 (void *)node - node_offset))
		return cu_false;
	    node = node->next;
	}
    }
    return cu_true;
}

cu_bool_t
cucon_umap_conj_node(cucon_umap_t map,
		     cu_clop(cb, cu_bool_t, cucon_umap_node_t node))
{
    size_t i;
    for (i = 0; i <= map->mask; ++i) {
	cucon_umap_node_t node = map->arr[i];
	while (node) {
	    if (!cu_call(cb, node))
		return cu_false;
	    node = node->next;
	}
    }
    return cu_true;
}

cu_bool_fast_t
cucon_pmap_conj_mem(cucon_pmap_t pmap,
		    cu_clop(cb, cu_bool_t, void const *, void *))
{
    size_t i;
    for (i = 0; i <= pmap->impl.mask; ++i) {
	cucon_umap_node_t node = pmap->impl.arr[i];
	while (node != NULL) {
	    if (!cu_call(cb, (void*)node->key, CU_ALIGNED_PTR_END(node)))
		return cu_false;
	    node = node->next;
	}
    }
    return cu_true;
}

cu_bool_fast_t
cucon_pmap_conj_ptr(cucon_pmap_t pmap,
		    cu_clop(cb, cu_bool_t, void const *, void *))
{
    size_t i;
    for (i = 0; i <= pmap->impl.mask; ++i) {
	cucon_umap_node_t node = pmap->impl.arr[i];
	while (node != NULL) {
	    if (!cu_call(cb, (void*)node->key,
			  *(void **)CU_ALIGNED_PTR_END(node)))
		return cu_false;
	    node = node->next;
	}
    }
    return cu_true;
}

cu_bool_fast_t
cucon_umap_conj_mem(cucon_umap_t umap,
		  cu_clop(cb, cu_bool_t, uintptr_t, void *))
{
    size_t i;
    for (i = 0; i <= umap->mask; ++i) {
	cucon_umap_node_t node = umap->arr[i];
	while (node != NULL) {
	    if (!cu_call(cb, node->key, CU_ALIGNED_PTR_END(node)))
		return cu_false;
	    node = node->next;
	}
    }
    return cu_true;
}

void
cucon_pmap_iter_keys(cucon_pmap_t pmap, cu_clop(cb, void, void const *))
{
    size_t i;
    for (i = 0; i <= pmap->impl.mask; ++i) {
	cucon_umap_node_t node = pmap->impl.arr[i];
	while (node != NULL) {
	    cu_call(cb, (void*)node->key);
	    node = node->next;
	}
    }
}

void
cucon_umap_iter_keys(cucon_umap_t umap, cu_clop(cb, void, uintptr_t))
{
    size_t i;
    for (i = 0; i <= umap->mask; ++i) {
	cucon_umap_node_t node = umap->arr[i];
	while (node != NULL) {
	    cu_call(cb, node->key);
	    node = node->next;
	}
    }
}

static int
uintptr_cmp(const void *x, const void *y)
{
    return *(uintptr_t *)x - *(uintptr_t *)y;
}

void
cucon_umap_iter_increasing_keys(cucon_umap_t umap,
				cu_clop(cb, void, uintptr_t))
{
    size_t i;
    uintptr_t *arr = cu_salloc(sizeof(uintptr_t)*umap->size);
    uintptr_t *p = arr;
    for (i = 0; i <= umap->mask; ++i) {
	cucon_umap_node_t node = umap->arr[i];
	while (node != NULL) {
	    *p++ = node->key;
	    node = node->next;
	}
    }
    qsort(arr, umap->size, sizeof(uintptr_t), uintptr_cmp);
    p = arr;
    for (i = 0; i < umap->size; ++i)
	cu_call(cb, *p++);
}

cu_bool_fast_t
cucon_pmap_conj_keys(cucon_pmap_t pmap, cu_clop(cb, cu_bool_t, void const *))
{
    size_t i;
    for (i = 0; i <= pmap->impl.mask; ++i) {
	cucon_umap_node_t node = pmap->impl.arr[i];
	while (node != NULL) {
	    if (!cu_call(cb, (void const*)node->key))
		return cu_false;
	    node = node->next;
	}
    }
    return cu_true;
}

cu_bool_fast_t
cucon_umap_conj_keys(cucon_umap_t umap, cu_clop(cb, cu_bool_t, uintptr_t))
{
    size_t i;
    for (i = 0; i <= umap->mask; ++i) {
	cucon_umap_node_t node = umap->arr[i];
	while (node != NULL) {
	    if (!cu_call(cb, node->key))
		return cu_false;
	    node = node->next;
	}
    }
    return cu_true;
}

cu_bool_t
cucon_umap_eq_keys(cucon_umap_t umap0, cucon_umap_t umap1)
{
    size_t i;
    if (cucon_umap_size(umap0) != cucon_umap_size(umap1))
	return cu_false;
    for (i = 0; i <= umap0->mask; ++i) {
	cucon_umap_node_t node = umap0->arr[i];
	while (node != NULL) {
	    if (!cucon_umap_find_mem(umap1, node->key))
		return cu_false;
	    node = node->next;
	}
    }
    return cu_true;
}

cu_bool_t
cucon_umap_eq_mem(cucon_umap_t umap0, cucon_umap_t umap1, size_t slot_size)
{
    size_t i;
    if (cucon_umap_size(umap0) != cucon_umap_size(umap1))
	return cu_false;
    for (i = 0; i <= umap0->mask; ++i) {
	cucon_umap_node_t node = umap0->arr[i];
	while (node != NULL) {
	    void *slot = cucon_umap_find_mem(umap1, node->key);
	    if (!slot)
		return cu_false;
	    if (memcmp(CU_ALIGNED_PTR_END(node), slot, slot_size) != 0)
		return cu_false;
	}
    }
    return cu_true;
}

cu_hash_t
cucon_umap_hash_keys(cucon_umap_t umap)
{
    size_t i;
    cu_hash_t hash = 178841123;
    for (i = 0; i <= umap->size; ++i) {
	cucon_umap_node_t node = umap->arr[i];
	while (node != NULL) {
	    hash += HASH(node->key);
	    node = node->next;
	}
    }
    return HASH(hash);
}

cu_hash_t
cucon_umap_hash_mem(cucon_umap_t umap, size_t slot_size)
{
    size_t i;
    cu_hash_t hash = 178841123;
    for (i = 0; i <= umap->size; ++i) {
	cucon_umap_node_t node = umap->arr[i];
	while (node != NULL) {
	    cu_hash_t subhash = HASH(node->key);
	    void *slot = CU_ALIGNED_PTR_END(node);
	    void *stop = slot + slot_size;
	    /* If slot_size is not a multiple of sizeof(cu_word_t), we
	     * rely on the fact the the GC will allocate in multiples
	     * of words and clear the allocated memory.  The following
	     * simply includes the zero-padding. */
	    while (slot < stop) {
		subhash = HASH(subhash + *(cu_word_t *)slot);
		slot += sizeof(cu_word_t);
	    }
	    hash += subhash;
	    node = node->next;
	}
    }
    return HASH(hash);
}

void
cucon_umap_assign_isecn_union(cucon_umap_t umap0, cucon_umap_t umap1)
{
    size_t i;
    if (cucon_umap_size(umap0) > cucon_umap_size(umap1))
	cucon_umap_swap(umap0, umap1);
    for (i = 0; i <= umap0->mask; ++i) {
	cucon_umap_node_t *node0 = &umap0->arr[i];
	while (*node0 != NULL) {
	    cu_uintptr_t key0 = (*node0)->key;
	    unsigned long j = HASH(key0) & umap0->mask;
	    cucon_umap_node_t node1 = umap1->arr[j];
	    cucon_umap_node_t node_mv;
	    while (node1 != NULL) {
		if ((node1)->key == key0)
		    goto break0;
		node1 = node1->next;
	    }

	    /* Move node0 to umap1 */
	    node_mv = *node0;
	    *node0 = node_mv->next;
	    node_mv->next = umap1->arr[j];
	    umap1->arr[j] = node_mv;

	    --umap0->size;
	    ++umap1->size;
	    continue;
	break0:
	    node0 = &(*node0)->next;
	}
    }
    update_cap_after_erase(umap0);
    update_cap_after_insert(umap1);
}

void
cucon_umap_move_isecn(cucon_umap_t dst, cucon_umap_t src0, cucon_umap_t src1)
{
    size_t i;
    if (cucon_umap_size(src0) > cucon_umap_size(src1)) {
	cucon_umap_t tmp = src0;
	src0 = src1;
	src1 = tmp;
    }
    for (i = 0; i <= src0->mask; ++i) {
	cucon_umap_node_t *node0 = &src0->arr[i];
	while (*node0 != NULL) {
	    cu_uintptr_t key0 = (*node0)->key;
	    if (cucon_umap_erase_keep_cap(src1, key0)) {
		size_t hash0 = HASH(key0);
		size_t k = hash0 & dst->mask;
		cucon_umap_node_t node_mv = *node0;
		node_mv = *node0;
		*node0 = node_mv->next;
		node_mv->next = dst->arr[k];
		dst->arr[k] = node_mv;
		--src0->size;
		++dst->size;
	    }
	    else
		node0 = &(*node0)->next;
	}
    }
    update_cap_after_erase(src0);
    update_cap_after_erase(src1);
    update_cap_after_insert(dst);
}

void
cucon_umap_assign_isecn(cucon_umap_t dst, cucon_umap_t src)
{
    size_t i;
    for (i = 0; i <= dst->mask; ++i) {
	cucon_umap_node_t *dst_node = &dst->arr[i];
	while (*dst_node != NULL) {
	    if (cucon_umap_find_mem(src, (*dst_node)->key))
		dst_node = &(*dst_node)->next;
	    else
		*dst_node = (*dst_node)->next;
	}
    }
}

void
cucon_umap_assign_union_void(cucon_umap_t dst, cucon_umap_t src)
{
    size_t i;
    for (i = 0; i <= src->mask; ++i) {
	cucon_umap_node_t src_node = src->arr[i];
	while (src_node != NULL) {
	    cucon_umap_insert_void(dst, src_node->key);
	    src_node = src_node->next;
	}
    }
}

void
cucon_umap_dump_stats(cucon_umap_t umap, FILE *out)
{
    double avg_depth;
    size_t const n_profile = 8;
    size_t a_profile[n_profile];
    size_t i;
    int max_cnt = 0;
    memset(a_profile, 0, n_profile*sizeof(size_t));
    for (i = 0; i <= umap->mask; ++i) {
	cucon_umap_node_t node = umap->arr[i];
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
    fprintf(out, "debug: cucon_umap: depth profile:");
    avg_depth = 0;
    for (i = 0; i < n_profile; ++i) {
	double w;
	if (i > max_cnt)
	    break;
	w = a_profile[i]/(double)umap->size;
	fprintf(out, " %5.3f", w);
	avg_depth += i*w;
    }
    fprintf(out, "\n");
    fprintf(out, "debug: cucon_umap:     gives max_depth = %d, avg_depth = %f\n",
	    max_cnt, avg_depth);
}

cucon_umap_it_t
cucon_umap_begin(cucon_umap_t umap)
{
    cucon_umap_it_t it;
    if (umap->size == 0) {
	it.node_head = umap->arr + (umap->mask + 1);
	it.node = (void*)-1;
    }
    else {
	it.node_head = umap->arr;
	while (*it.node_head == NULL)
	    ++it.node_head;
	it.node = *it.node_head;
    }
    return it;
}

cucon_umap_it_t
cucon_umap_end(cucon_umap_t umap)
{
    cucon_umap_it_t it;
    it.node_head = umap->arr + (umap->mask + 1);
    it.node = (void*)-1;
    return it;
}

cucon_umap_it_t
cucon_umap_it_next(cucon_umap_it_t it)
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

cu_clos_def(umap_print_keys_cb,
	    cu_prot(void, uintptr_t key),
    ( int index;
      FILE *out; ))
{
    cu_clos_self(umap_print_keys_cb);
    if (self->index)
	fputs(", ", self->out);
    fprintf(self->out, "%"PRIdPTR, key);
    ++self->index;
}

void
cucon_umap_print_keys(cucon_umap_t map, FILE *out)
{
    umap_print_keys_cb_t cb;
    cb.index = 0;
    cb.out = out;
    fputc('{', out);
    cucon_umap_iter_increasing_keys(map, umap_print_keys_cb_prep(&cb));
    fputc('}', out);
}

cu_clos_def(dump_idr_ptr_cb,
	    cu_prot(void, cu_idr_t key, void *val),
    ( FILE *out; ))
{
    cu_clos_self(dump_idr_ptr_cb);
    fprintf(self->out, "\t%s ↦ %p\n", cu_idr_to_cstr(key), val);
}

void
cucon_pmap_dump_idr_ptr(cucon_pmap_t map, FILE *out)
{
    dump_idr_ptr_cb_t cb;
    cb.out = out;
    fprintf(out, "cucon_pmap_t @ %p:\n", map);
    cucon_pmap_iter_ptr(map,
			(cu_clop(, void, void const *, void *))
			dump_idr_ptr_cb_prep(&cb));
}
