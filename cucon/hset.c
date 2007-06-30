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

#include <cucon/hset.h>
#include <cu/memory.h>
#include <cu/debug.h>
#include <cu/diag.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#ifndef CU_NDEBUG
#  include <stdio.h>
#endif

#define PRIME0 10000019
#define PRIME1 10000079
#define PRIME2 10000103
#define PRIME3 10000121
#ifndef CUCON_HSET_CAPACITY_MIN
#  define CUCON_HSET_CAPACITY_MIN 256
#endif
#ifndef CUCON_HSET_CAPACITY_INIT
#  define CUCON_HSET_CAPACITY_INIT 32
#endif
#if CUCON_HSET_CAPACITY_INIT & (CUCON_HSET_CAPACITY_INIT-1) != 0
#  error "CUCON_HSET_CAPACITY_INIT must be a power of 2!"
#endif
#if CUCON_HSET_CAPACITY_MIN & (CUCON_HSET_CAPACITY_MIN-1) != 0
#  error "CUCON_HSET_CAPACITY_MIN must be a power of 2!"
#endif


#if 0
#  include <stdio.h>
#  define D_PRINTF(args...) fprintf(stderr, args)
#else
#  define D_PRINTF(args...) ((void)0)
#endif


void
cucon_hset_cct(cucon_hset_t hs,
	     cu_clop(equals, cu_bool_t, void *, void *),
	     cu_clop(hash, cu_hash_t, void *))
{
    hs->table = cu_galloc(sizeof(void*)*(CUCON_HSET_CAPACITY_INIT+1));
    memset(hs->table, 0, sizeof(void*)*CUCON_HSET_CAPACITY_INIT);
    hs->mask = CUCON_HSET_CAPACITY_INIT-1;
    hs->table[hs->mask + 1] = &hs->tail;
    hs->count = 0;
    hs->equals = equals;
    hs->hash = hash;
}

cucon_hset_t
cucon_hset_new(cu_clop(equals, cu_bool_t, void *, void *),
	     cu_clop(hash, cu_hash_t, void *))
{
    cucon_hset_t hs = cu_galloc(sizeof(struct cucon_hset_s));
    cucon_hset_cct(hs, equals, hash);
    return hs;
}

void
cucon_hset_clear(cucon_hset_t hs)
{
    cu_hash_t hc;
    for (hc = 0; hc <= hs->mask; ++hc) {
	cucon_hset_node_t node = hs->table[hc];
	while (node) {
	    cucon_hset_node_t next = node->next;
/* 	    cu_gfree(node->key); */
/* 	    cu_gfree(node); */
	    node = next;
	}
	hs->table[hc] = NULL;
    }
    hs->count = 0;
}

void *
cucon_hset_find(cucon_hset_t hs, void *o)
{
    cu_hash_t hc;
    cucon_hset_node_t node;
    hc = cu_call(hs->hash, o);
    node = hs->table[hc & hs->mask];
    while (node) {
	if (cu_call(hs->equals, node->key, o))
	    return node->key;
	node = node->next;
    }
    return NULL;
}

void *
cucon_hset_insert(cucon_hset_t hs, void *o)
{
    cu_hash_t hc = cu_call(hs->hash, o);
    cu_hash_t idx = hc & hs->mask;
    cucon_hset_node_t node;
    D_PRINTF("cucon_hset_insert(..., %p); idx = %ld, hc = %ld", o, idx, hc);
    node = hs->table[idx];
    while (node) {
	if (cu_call(hs->equals, node->key, o)) {
	    D_PRINTF(" Existing entry.\n");
	    return node->key;
	}
	node = node->next;
    }
    node = cu_galloc(sizeof(struct cucon_hset_node_s));
    node->key = o;
    node->next = hs->table[idx];
    hs->table[idx] = node;
    D_PRINTF(", size = %d\n", hs->count);
    if (hs->count++ > (hs->mask>>1))
	cucon_hset_set_capacity(hs, 2*(hs->mask + 1));
    return NULL;
}

void
cucon_hset_multi_insert_node(cucon_hset_t hs, cucon_hset_node_t node)
{
    cu_hash_t hc = cu_call(hs->hash, node->key);
    cu_hash_t idx = hc & hs->mask;
    node->next = hs->table[idx];
    hs->table[idx] = node;
    ++hs->count;
}

void *
cucon_hset_replace(cucon_hset_t hs, void *o)
{
    cu_hash_t hc = cu_call(hs->hash, o);
    cu_hash_t idx = hc & hs->mask;
    cucon_hset_node_t node;
    node = hs->table[idx];
    while (node) {
	if (cu_call(hs->equals, node->key, o)) {
	    void *old_key = node->key;
	    node->key = o;
	    return old_key;
	}
	node = node->next;
    }
    node = cu_galloc(sizeof(struct cucon_hset_node_s));
    node->key = o;
    node->next = hs->table[idx];
    hs->table[idx] = node;
    if (hs->count++ > (hs->mask>>1))
	cucon_hset_set_capacity(hs, 2*(hs->mask + 1));
    return NULL;
}

void *
cucon_hset_erase(cucon_hset_t hs, void *o)
{
    cu_hash_t idx = cu_call(hs->hash, o) & hs->mask;
    cucon_hset_node_t *node = &hs->table[idx];
    D_PRINTF("cucon_hset_erase...");
    while (*node) {
	if (cu_call(hs->equals, (*node)->key, o)) {
	    void *ret = (*node)->key;
	    *node = (*node)->next;
	    if (hs->count-- < (hs->mask>>3) &&
		hs->mask > 2*CUCON_HSET_CAPACITY_MIN)
		cucon_hset_set_capacity(hs, (hs->mask+1)/2);
	    D_PRINTF(", size = %d\n", hs->count);
	    return ret;
	}
	node = &(*node)->next;
    }
    D_PRINTF(" No such entry.\n");
    return NULL;
}

void *
cucon_hset_erase_keep_capacity(cucon_hset_t hs, void *o)
{
    cu_hash_t idx = cu_call(hs->hash, o) & hs->mask;
    cucon_hset_node_t *node = &hs->table[idx];
    D_PRINTF("cucon_hset_erase_keep_capacity...");
    while (*node) {
	if (cu_call(hs->equals, (*node)->key, o)) {
	    void *ret = (*node)->key;
	    *node = (*node)->next;
	    --hs->count;
	    D_PRINTF(", size = %d\n", hs->count);
	    return ret;
	}
	node = &(*node)->next;
    }
    D_PRINTF(" No such entry.\n");
    return NULL;
}

void
cucon_hset_check_integrity(cucon_hset_t hs)
{
    size_t i;
    size_t count = 0;
    cucon_hset_node_t node;
    for (i = 0; i <= hs->mask; ++i) {
	node = hs->table[i];
	while (node != NULL) {
	    cu_hash_t hc = cu_call(hs->hash, node->key);
	    if ((hc & hs->mask) != i) {
		cu_errf("cucon_hset: Element %p with hc = %ld placed at %d, "
			 "should be at %ld.",
			node->key, hc, i, hc & hs->mask);
		abort();
	    }
	    ++count;
	    node = node->next;
	}
    }
    if (count != hs->count) {
	cu_errf("Wrong element count, %d != %d.\n", count, hs->count);
	abort();
    }
}

void
cucon_hset_set_capacity(cucon_hset_t hs, int new_cap)
{
    cucon_hset_node_t *it;
    cucon_hset_node_t *it_begin = hs->table;
    cucon_hset_node_t *it_end = it_begin + (hs->mask + 1);
    D_PRINTF("cucon_hset: Changing capacity to %d (cur. mask = %d).\n",
	     new_cap, (int)hs->mask);
    hs->table = cu_galloc(sizeof(cucon_hset_node_t)*(new_cap + 1));
    memset(hs->table, 0, sizeof(cucon_hset_node_t)*new_cap);
    hs->mask = new_cap - 1;
    hs->table[hs->mask + 1] = &hs->tail;
    for (it = it_begin; it != it_end; ++it) {
	cucon_hset_node_t node = *it;
	while (node) {
	    cucon_hset_node_t node_next = node->next;
	    cu_hash_t hc = cu_call(hs->hash, node->key);
	    cu_hash_t idx = hc & hs->mask;
	    node->next = hs->table[idx];
	    hs->table[idx] = node;
	    node = node_next;
	}
    }
/*     cu_gfree(it_begin); */
}

cu_bool_t
cucon_hset_conj(cucon_hset_t hs, cu_clop(cb, cu_bool_t, void *))
{
    size_t i;
    size_t I = hs->mask + 1;
    for (i = 0; i < I; ++i) {
	cucon_hset_node_t node = hs->table[i];
	while (node) {
	    if (!cu_call(cb, node->key))
		return cu_false;
	    node = node->next;
	}
    }
    return cu_true;
}

cu_clos_def(cucon_hset_subeq_cb,
	    cu_prot(cu_bool_t, void *key),
	    (cucon_hset_t hs1;))
{
    cu_clos_self(cucon_hset_subeq_cb);
    return cucon_hset_find(self->hs1, key) != NULL;
}

cu_bool_t
cucon_hset_subeq(cucon_hset_t hs0, cucon_hset_t hs1)
{
    if (cucon_hset_size(hs0) > cucon_hset_size(hs1))
	return cu_false;
    else {
	cucon_hset_subeq_cb_t cb;
	cb.hs1 = hs1;
	return cucon_hset_conj(hs0, cucon_hset_subeq_cb_prep(&cb));
    }
}

cu_bool_t
cucon_hset_sub(cucon_hset_t hs0, cucon_hset_t hs1)
{
    if (cucon_hset_size(hs0) >= cucon_hset_size(hs1))
	return cu_false;
    else {
	cucon_hset_subeq_cb_t cb;
	cb.hs1 = hs1;
	return cucon_hset_conj(hs0, cucon_hset_subeq_cb_prep(&cb));
    }
}

cu_bool_t
cucon_hset_eq(cucon_hset_t hs0, cucon_hset_t hs1)
{
    if (cucon_hset_size(hs0) != cucon_hset_size(hs1))
	return cu_false;
    else {
	cucon_hset_subeq_cb_t cb;
	cb.hs1 = hs1;
	return cucon_hset_conj(hs0, cucon_hset_subeq_cb_prep(&cb));
    }
}


/* Iterators
 * --------- */

cucon_hset_it_t
cucon_hset_begin(cucon_hset_t hs)
{
    cucon_hset_it_t it;
    it.node_head = hs->table;
    while (!*it.node_head)
	++it.node_head;
    it.node = *it.node_head;
    return it;
}

cucon_hset_it_t
cucon_hset_end(cucon_hset_t hs)
{
    cucon_hset_it_t it;
    it.node_head = hs->table + (hs->mask + 1);
    it.node = NULL;
    return it;
}

cucon_hset_it_t
cucon_hset_next(cucon_hset_it_t it)
{
    if (!(it.node = it.node->next)) {
	++it.node_head;
	while (*it.node_head)
	    ++it.node_head;
    }
    return it;
}
