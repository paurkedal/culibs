/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2008  Petter Urkedal <urkedal@nbi.dk>
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

#include <cucon/hzmap.h>
#include <cu/hash.h>
#include <cu/memory.h>
#include <cu/wordarr.h>
#include <cu/size.h>
#include <string.h>

#define INIT_CAP_EXPT 2
#define INIT_CAP ((size_t)1 << INIT_CAP_EXPT)
#define MINFILL_NUMER 1
#define MINFILL_DENOM 2
#define MAXFILL_NUMER 2
#define MAXFILL_DENOM 1

typedef cucon_hzmap_t map_t;
typedef cucon_hzmap_node_t node_t;

#if 0
#  define key_hash(key_size_w, key) cu_wordarr_hash_bj(key_size_w, key, 0)
#  define key_hash_1w(key) cu_1word_hash_bj(((cu_word_t *)key)[0], 0)
#  define key_hash_2w(key) cu_2word_hash_bj(((cu_word_t *)key)[0], ((cu_word_t *)key)[1], 0)
#else
#  define key_hash(key_size_w, key) cu_wordarr_hash_noinit_bj(key_size_w, key)
#  define key_hash_1w(key) cu_1word_hash_noinit_bj(((cu_word_t *)key)[0])
#  define key_hash_2w(key) cu_2word_hash_noinit_bj(((cu_word_t *)key)[0], ((cu_word_t *)key)[1])
#endif
#define key_eq(key_size_w, k0, k1) cu_wordarr_eq(key_size_w, k0, k1)
#define key_copy(key_size_w, dst, src) cu_wordarr_copy(key_size_w, dst, src)
#define node_key(node) ((cu_word_t *)((node_t)(node) + 1))
#define node_hash(key_size_w, node) key_hash(key_size_w, node_key(node))
#define node_hash_1w(node) key_hash_1w(node_key(node))
#define node_hash_2w(node) key_hash_2w(node_key(node))
#ifdef CUCON_HZMAP_COMPACT
#  define map_cap(map) ((size_t)1 << (map)->cap_expt)
#  define map_cap_est(map) map_cap(map)
#  define map_mask(map) (map_cap(map) - (size_t)1)
#else
#  define map_cap(map) ((map)->mask + 1)
#  define map_cap_est(map) ((map)->mask)
#  define map_mask(map) ((map)->mask)
#endif

void
cucon_hzmap_init(map_t map, size_t key_size_w)
{
    map->key_size_w = key_size_w;
#ifdef CUCON_HZMAP_COMPACT
    map->cap_expt = INIT_CAP_EXPT;
#else
    map->mask = INIT_CAP - 1;
#endif
    map->size = 0;
    map->arr = cu_galloc(INIT_CAP*sizeof(node_t));
}

map_t
cucon_hzmap_new(size_t key_size_w)
{
    map_t map = cu_gnew(struct cucon_hzmap_s);
    cucon_hzmap_init(map, key_size_w);
    return map;
}

static void
shrink(map_t map, size_t new_cap)
{
    size_t old_cap, new_mask, i;
    node_t *new_arr, *old_arr;

    if (new_cap < INIT_CAP)
	return;

    old_arr = map->arr;
    old_cap = map_cap(map);
    cu_debug_assert(0 < new_cap && new_cap < old_cap &&
		    ((new_cap - 1) & new_cap) == 0);
    map->arr  = new_arr  = cu_galloc(sizeof(node_t)*new_cap);
#ifdef CUCON_HZMAP_COMPACT
    new_mask = new_cap - 1;
    map->cap_expt = cu_size_ceil_log2(new_cap);
#else
    map->mask = new_mask = new_cap - 1;
#endif

    memcpy(new_arr, old_arr, new_cap*sizeof(node_t));
    for (i = new_cap; i < old_cap; ++i) {
	if (old_arr[i]) {
	    node_t *p = &new_arr[i & new_mask];
	    while (*p)
		p = &(*p)->next;
	    *p = old_arr[i];
	}
    }
}

static void
expand(map_t map, size_t new_cap)
{
    size_t old_cap, new_mask, i;
    node_t *new_arr, *old_arr;
    size_t key_size_w = map->key_size_w;

    old_arr = map->arr;
    old_cap = map_cap(map);
    cu_debug_assert(new_cap > old_cap && ((new_cap - 1) & new_cap) == 0);
    map->arr  = new_arr  = cu_galloc(sizeof(node_t)*new_cap);
#ifdef CUCON_HZMAP_COMPACT
    new_mask = new_cap - 1;
    map->cap_expt = cu_size_ceil_log2(new_cap);
#else
    map->mask = new_mask = new_cap - 1;
#endif

    for (i = 0; i < old_cap; ++i) {
	node_t node = old_arr[i];
	while (node) {
	    node_t next_node = node->next;
	    node_t *p = &new_arr[node_hash(key_size_w, node) & new_mask];
	    node->next = *p;
	    *p = node;
	    node = next_node;
	}
    }
}

void
cucon_hzmap_prepare_insert(map_t map, size_t count)
{
    size_t new_size = map->size + count;
    if (new_size*MAXFILL_DENOM > map_cap_est(map)*MAXFILL_NUMER)
	expand(map, cu_size_exp2ceil(new_size*MINFILL_DENOM/MINFILL_NUMER));
}

cu_bool_t
cucon_hzmap_insert(map_t map, void const *key,
		   size_t node_size, node_t *node_out)
{
    cu_hash_t index;
    node_t *p;
    size_t key_size_w = map->key_size_w;

    if (map->size*MAXFILL_DENOM > map_cap_est(map)*MAXFILL_NUMER)
	expand(map, map_cap(map)*2);

    index = key_hash(key_size_w, key) & map_mask(map);
    p = &map->arr[index];
    while (*p) {
	if (key_eq(key_size_w, node_key(*p), key)) {
	    *node_out = *p;
	    return cu_false;
	}
	p = &(*p)->next;
    }
    *p = cu_galloc(node_size);
    key_copy(key_size_w, node_key(*p), key);
    *node_out = *p;
    ++map->size;
    return cu_true;
}

cu_bool_t
cucon_hzmap_insert_void(map_t map, void const *key)
{
    cu_hash_t index;
    node_t *p;
    size_t key_size_w = map->key_size_w;

    if (map->size*MAXFILL_DENOM > map_cap_est(map)*MAXFILL_NUMER)
	expand(map, map_cap(map)*2);

    index = key_hash(key_size_w, key) & map_mask(map);
    p = &map->arr[index];
    while (*p) {
	if (key_eq(key_size_w, node_key(*p), key))
	    return cu_false;
	p = &(*p)->next;
    }
    *p = cu_galloc(sizeof(struct cucon_hzmap_node_s)
		   + key_size_w*sizeof(cu_word_t));
    key_copy(key_size_w, node_key(*p), key);
    ++map->size;
    return cu_true;
}

cu_bool_t
cucon_hzmap_erase(map_t map, void const *key)
{
    cu_hash_t index;
    node_t *p;
    size_t key_size_w = map->key_size_w;

    index = key_hash(key_size_w, key) & map_mask(map);
    p = &map->arr[index];
    while (*p) {
	if (key_eq(key_size_w, node_key(*p), key)) {
	    *p = (*p)->next;
	    --map->size;
	    return cu_true;
	}
	p = &(*p)->next;
    }

    if (map->size*MINFILL_DENOM < map_cap_est(map)*MINFILL_NUMER)
	shrink(map, map_cap(map)/2);

    return cu_false;
}

cu_bool_t
cucon_hzmap_step_erase(map_t map, void const *key)
{
    cu_hash_t index;
    node_t *p;
    size_t key_size_w = map->key_size_w;

    index = key_hash(key_size_w, key) & map_mask(map);
    p = &map->arr[index];
    while (*p) {
	if (key_eq(key_size_w, node_key(*p), key)) {
	    *p = (*p)->next;
	    --map->size;
	    return cu_true;
	}
	p = &(*p)->next;
    }
    return cu_false;
}

void
cucon_hzmap_finish_erase(map_t map)
{
    if (map->size*MINFILL_DENOM < map_cap_est(map)*MINFILL_NUMER)
	shrink(map, map_cap(map)/2);
}

node_t
cucon_hzmap_find(map_t map, void const *key)
{
    size_t key_size_w = map->key_size_w;
    cu_hash_t index = key_hash(key_size_w, key) & map_mask(map);
    node_t node = map->arr[index];
    while (node) {
	if (key_eq(key_size_w, node_key(node), key))
	    return node;
	node = node->next;
    }
    return NULL;
}

node_t
cucon_hzmap_1w_find(map_t map, void const *key)
{
    cu_hash_t index = key_hash_1w(key) & map_mask(map);
    node_t node = map->arr[index];
    cu_debug_assert(map->key_size_w == 1);
    while (node) {
	if (key_eq(1, node_key(node), key))
	    return node;
	node = node->next;
    }
    return NULL;
}
node_t
cucon_hzmap_2w_find(map_t map, void const *key)
{
    cu_hash_t index = key_hash_2w(key) & map_mask(map);
    node_t node = map->arr[index];
    cu_debug_assert(map->key_size_w == 2);
    while (node) {
	if (key_eq(2, node_key(node), key))
	    return node;
	node = node->next;
    }
    return NULL;
}

cu_bool_t
cucon_hzmap_forall(cu_clop(f, cu_bool_t, cucon_hzmap_node_t),
		   cucon_hzmap_t map)
{
    cucon_hzmap_node_t *head, *head_end;
    head = map->arr;
    head_end = head + map_cap(map);
    while (head != head_end) {
	cucon_hzmap_node_t node = *head;
	while (node) {
	    if (!cu_call(f, node))
		return cu_false;
	    node = node->next;
	}
	++head;
    }
    return cu_true;
}

cu_bool_t
cucon_hzmap_forall_keys(cu_clop(f, cu_bool_t, void const *), cucon_hzmap_t map)
{
    cucon_hzmap_node_t *head, *head_end;
    head = map->arr;
    head_end = head + map_cap(map);
    while (head != head_end) {
	cucon_hzmap_node_t node = *head;
	while (node) {
	    if (!cu_call(f, node_key(node)))
		return cu_false;
	    node = node->next;
	}
	++head;
    }
    return cu_true;
}

void
cucon_hzmap_filter(cu_clop(f, cu_bool_t, cucon_hzmap_node_t),
		   cucon_hzmap_t map)
{
    cucon_hzmap_node_t *head, *head_end;
    head = map->arr;
    head_end = head + map_cap(map);
    while (head != head_end) {
	cucon_hzmap_node_t *p = head;
	while (*p) {
	    if (cu_call(f, *p))
		p = &(*p)->next;
	    else {
		*p = (*p)->next;
		--map->size;
	    }
	}
	++head;
    }
    cucon_hzmap_finish_erase(map);
}

void
cucon_hzmap_filter_keys(cu_clop(f, cu_bool_t, void const *), cucon_hzmap_t map)
{
    cucon_hzmap_node_t *head, *head_end;
    head = map->arr;
    head_end = head + map_cap(map);
    while (head != head_end) {
	cucon_hzmap_node_t *p = head;
	while (*p) {
	    if (cu_call(f, node_key(*p)))
		p = &(*p)->next;
	    else {
		*p = (*p)->next;
		--map->size;
	    }
	}
	++head;
    }
    cucon_hzmap_finish_erase(map);
}

void
cucon_hzmap_itr_init(cucon_hzmap_itr_t itr, cucon_hzmap_t map)
{
    itr->arr_cur = map->arr;
    itr->arr_end = map->arr + cucon_hzmap_capacity(map);
    itr->node = NULL;
}

cucon_hzmap_node_t
cucon_hzmap_itr_get(cucon_hzmap_itr_t itr)
{
    cucon_hzmap_node_t node = itr->node;
    while (!node) {
	if (itr->arr_cur == itr->arr_end)
	    return NULL;
	node = *itr->arr_cur++;
    }
    itr->node = node->next;
    return node;
}

void const *
cucon_hzmap_itr_get_key(cucon_hzmap_itr_t itr)
{
    cucon_hzmap_node_t node = itr->node;
    while (!node) {
	if (itr->arr_cur == itr->arr_end)
	    return NULL;
	node = *itr->arr_cur++;
    }
    itr->node = node->next;
    return node_key(node);
}
