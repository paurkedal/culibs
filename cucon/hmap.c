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

#include <cucon/hmap.h>
#include <cu/memory.h>
#include <cu/util.h>
#include <string.h>
#include <stdlib.h>

#ifndef CUCON_HMAP_CAPACITY_MIN
#  define CUCON_HMAP_CAPACITY_MIN 32
#endif
#ifndef CUCON_HMAP_CAPACITY_INIT
#  define CUCON_HMAP_CAPACITY_INIT 8
#endif
#if CUCON_HMAP_CAPACITY_INIT & (CUCON_HMAP_CAPACITY_INIT-1) != 0
#  error "CUCON_HMAP_CAPACITY_INIT must be a power of 2!"
#endif
#if CUCON_HMAP_CAPACITY_MIN & (CUCON_HMAP_CAPACITY_MIN-1) != 0
#  error "CUCON_HMAP_CAPACITY_MIN must be a power of 2!"
#endif

#define PRIME0 10000019
#define PRIME1 10000079
#define PRIME2 10000103
#define PRIME3 10000121

void
cucon_hmap_cct(cucon_hmap_t map,
	     cu_clop(eq, cu_bool_t, void *, void *),
	     cu_clop(hash, cu_hash_t, void *))
{
    map->table = cu_galloc(sizeof(void*)*(CUCON_HMAP_CAPACITY_INIT+1));
    memset(map->table, 0, sizeof(void*)*CUCON_HMAP_CAPACITY_INIT);
    map->mask = CUCON_HMAP_CAPACITY_INIT-1;
    map->table[map->mask+1] = &map->tail;
    map->size = 0;
    map->eq = eq;
    map->hash = hash;
}

cucon_hmap_t
cucon_hmap_new(cu_clop(eq, cu_bool_t, void *, void *),
	     cu_clop(hash, cu_hash_t, void *))
{
    cucon_hmap_t map = cu_galloc(sizeof(struct cucon_hmap_s));
    cucon_hmap_cct(map, eq, hash);
    return map;
}

void
cucon_hmap_clear(cucon_hmap_t map)
{
    cu_hash_t hc;
    for (hc = 0; hc <= map->mask; ++hc) {
	cucon_hmap_node_t node = map->table[hc];
	while (node) {
	    cucon_hmap_node_t next = node->next;
/* 	    cu_gfree(node); */
	    node = next;
	}
    }
}

void *
cucon_hmap_find_mem(cucon_hmap_t map, void *o)
{
    cu_hash_t hc;
    cucon_hmap_node_t node;
    hc = cu_call(map->hash, o);
    node = map->table[hc & map->mask];
    while (node) {
	if (cu_call(map->eq, node->key, o))
	    return CU_ALIGNED_PTR_END(node);
	node = node->next;
    }
    return NULL;
}

void *
cucon_hmap_find_ptr(cucon_hmap_t map, void *o)
{
    void **p = cucon_hmap_find_mem(map, o);
    if (p)
	return *p;
    else
	return NULL;
}

cu_bool_fast_t
cucon_hmap_insert_mem(cucon_hmap_t map, void *key,
		    size_t slot_size, cu_ptr_ptr_t slot)
{
    cu_hash_t idx = cu_call(map->hash, key) & map->mask;
    cucon_hmap_node_t node;
    node = map->table[idx];
    while (node) {
	if (cu_call(map->eq, node->key, key)) {
	    *(void **)slot = CU_ALIGNED_PTR_END(node);
	    return cu_false;
	}
	node = node->next;
    }
    node = cu_galloc(CU_ALIGNED_SIZEOF(struct cucon_hmap_node_s) + slot_size);
    node->key = key;
    node->next = map->table[idx];
    map->table[idx] = node;
    if (map->size++ > (map->mask>>1))
	cucon_hmap_set_capacity(map, 2*(map->mask+1));
    *(void **)slot = CU_ALIGNED_PTR_END(node);
    return cu_true;
}

cu_bool_fast_t
cucon_hmap_insert_node(cucon_hmap_t map, cucon_hmap_node_t newnode)
{
    cu_hash_t idx;
    cucon_hmap_node_t node;
    idx = cu_call(map->hash, newnode->key) & map->mask;
    node = map->table[idx];
    while (node) {
	if (cu_call(map->eq, node->key, newnode->key))
	    return cu_false;
    }
    newnode->next = map->table[idx];
    map->table[idx] = newnode;
    return cu_true;
}

cu_bool_fast_t
cucon_hmap_multi_insert_node(cucon_hmap_t map, cucon_hmap_node_t newnode)
{
    cu_hash_t idx;
    idx = cu_call(map->hash, newnode->key) & map->mask;
    newnode->next = map->table[idx];
    map->table[idx] = newnode;
    return cu_true;
}

void *
cucon_hmap_erase(cucon_hmap_t map, void *key)
{
    cu_hash_t idx = cu_call(map->hash, key) & map->mask;
    cucon_hmap_node_t *node = &map->table[idx];
    while (*node) {
	if (cu_call(map->eq, (*node)->key, key)) {
	    void *ret = CU_ALIGNED_PTR_END(*node);
	    *node = (*node)->next;
	    if (map->size-- < (map->mask>>2) &&
		map->mask > CUCON_HMAP_CAPACITY_MIN)
		cucon_hmap_set_capacity(map, (map->mask+1)/2);
	    return ret;
	}
	node = &(*node)->next;
    }
    return NULL;
}

void
cucon_hmap_set_capacity(cucon_hmap_t map, int newsize)
{
    cucon_hmap_node_t *u;
    cucon_hmap_node_t *u_beg = map->table;
    cucon_hmap_node_t *u_end = u_beg + (map->mask + 1);
    map->table = cu_galloc(sizeof(cucon_hmap_node_t)*(newsize+1));
    memset(map->table, 0, sizeof(cucon_hmap_node_t)*newsize);
    map->size = 0;
    map->mask = newsize-1;
    map->table[newsize] = &map->tail;
    for (u = u_beg; u != u_end; ++u) {
	cucon_hmap_node_t node;
	for (node = *u; node != NULL;) {
	    cucon_hmap_node_t next_node = node->next;
	    cucon_hmap_multi_insert_node(map, node);
	    node = next_node;
	}
    }
    cu_gfree(u_beg);
}

cu_bool_t
cucon_hmap_conj_mem(cucon_hmap_t map,
		  cu_clop(cb, cu_bool_t, void const *, void *))
{
    size_t N = map->mask + 1;
    size_t n;
    for (n = 0; n < N; ++n) {
	cucon_hmap_node_t node = map->table[n];
	while (node) {
	    if (!cu_call(cb, node->key, CU_ALIGNED_PTR_END(node)))
		return cu_false;
	    node = node->next;
	}
    }
    return cu_true;
}

cucon_hmap_it_t
cucon_hmap_begin(cucon_hmap_t map)
{
    cucon_hmap_it_t it;
    it.node_head = map->table;
    while (!*it.node_head)
	++it.node_head;
    it.node = *it.node_head;
    return it;
}

cucon_hmap_it_t
cucon_hmap_end(cucon_hmap_t map)
{
    cucon_hmap_it_t it;
    it.node_head = map->table + (map->mask+1);
    it.node = NULL;
    return it;
}

cucon_hmap_it_t
cucon_hmap_next(cucon_hmap_it_t it)
{
    if (!(it.node = it.node->next)) {
	++it.node_head;
	while (*it.node_head)
	    ++it.node_head;
    }
    return it;
}
