/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2007  Petter Urkedal <urkedal@nbi.dk>
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

#include <cuflow/cached.h>
#include <cuflow/wheel.h>
#include <cuflow/sched.h>
#include <cuflow/cdisj.h>
#include <cuflow/cacheconf.h>
#include <cuoo/halloc.h>
#include <cu/thread.h>

#define ARG_SIZE_COUNT 8
#define LOG2_STRIPE_COUNT 5
#define STRIPE_COUNT (1 << LOG2_STRIPE_COUNT)
#define STRIPE_MASK (STRIPE_COUNT - 1)
#define STRIPE_HASH_SHIFT (sizeof(cu_hash_t)*8 - LOG2_STRIPE_COUNT)
#define FILL_MIN_NOM 1
#define FILL_MIN_DENOM 2
#define FILL_MAX_NOM 4
#define FILL_MAX_DENOM 3

static cuflow_cacheconf_t cuflowP_cached_conf;

CU_SINLINE cu_bool_t
arg_eq(size_t sizeg, cu_word_t *arg0, cu_word_t *arg1)
{
    return memcmp(arg0, arg1, sizeg*CUFLOW_CACHED_GRAN_SIZEB) == 0;
}

CU_SINLINE void
arg_copy(size_t sizeg, cu_word_t *dst, cu_word_t *src)
{
    memcpy(dst, src, sizeg*CUFLOW_CACHED_GRAN_SIZEB);
}

CU_SINLINE cu_hash_t
arg_hash(size_t sizeg, cu_word_t *arg)
{
    return cu_wordarr_hash(sizeg*CUFLOW_CACHED_GRAN_SIZEW - 1, arg + 1, arg[0]);
}

CU_SINLINE cuflow_cached_clop_t
arg_clop(void *arg)
{
    return arg;
}

CU_SINLINE void *
node_arg(cuflowP_cached_node_t *node)
{
    return node + 1;
}

CU_SINLINE cuflow_cached_clop_t
node_clop(cuflowP_cached_node_t *node)
{
    return arg_clop(node_arg(node));
}

CU_SINLINE void
node_normalise_access(cuflowP_cached_node_t *node)
{
    unsigned int current_ticks = cuflowP_cached_conf->current_ticks;
    unsigned int delta_ticks = current_ticks - node->access_ticks;
    node->access_ticks = current_ticks;
    if (delta_ticks >= sizeof(node->access_function))
	node->access_function = 0;
    else
	node->access_function >>= delta_ticks;
}

CU_SINLINE cu_bool_t
node_drop_condition(cuflowP_cached_node_t *node)
{
    return !AO_load(&node->cdisj)
	&& cuflowP_cached_conf->byte_cost_per_tick
	 > node->access_function;
}

typedef struct cachebin_s
{
    pthread_mutex_t mutex;
    size_t mask;
    size_t size;
    size_t key_sizeg;	/* only for small-entry bins, else put before node */
    size_t access_count_since_pruned;
    cuflowP_cached_node_t **head_arr;
} *cachebin_t;

static struct cachebin_s cachebin_arr[STRIPE_COUNT][ARG_SIZE_COUNT];

CU_SINLINE cachebin_t
fcachebin(size_t arg_sizeg, cu_hash_t hash)
{
    return &cachebin_arr[(hash >> STRIPE_HASH_SHIFT) & STRIPE_MASK][arg_sizeg];
}
CU_SINLINE cachebin_t
vcachebin(cu_hash_t hash)
{
    return &cachebin_arr[(hash >> STRIPE_HASH_SHIFT) & STRIPE_MASK][0];
}

static void
cachebin_init(cachebin_t bin, size_t key_sizeg)
{
    cu_mutex_init(&bin->mutex);
    bin->size = 0;
    bin->mask = 0;
    bin->key_sizeg = key_sizeg;
    bin->access_count_since_pruned = 0;
    bin->head_arr = cu_galloc(sizeof(cuflowP_cached_node_t *));
}

static void
fcachebin_resize_locked(cachebin_t bin, size_t new_size)
{
    size_t old_mask = bin->mask;
    size_t new_mask = new_size - 1;
    size_t i;
    size_t key_sizeg = bin->key_sizeg;
    cuflowP_cached_node_t **old_head_arr = bin->head_arr;
    cuflowP_cached_node_t **new_head_arr;
    cu_debug_assert(key_sizeg);
    bin->mask = new_mask;
    bin->head_arr = new_head_arr
	= cu_galloc(sizeof(cuflowP_cached_node_t *)*new_size);
    for (i = 0; i <= old_mask; ++i) {
	cuflowP_cached_node_t *node = old_head_arr[i];
	while (node) {
	    cuflowP_cached_node_t *next_node = node->next;
	    cu_hash_t hash = arg_hash(key_sizeg, node_arg(node));
	    cuflowP_cached_node_t **new_node = &new_head_arr[hash & new_mask];
	    node->next = *new_node;
	    *new_node = node;
	    node = next_node;
	}
    }
}

static void
fcachebin_prune_locked(cachebin_t bin)
{
    cuflowP_cached_node_t **head_arr = bin->head_arr;
    cuflowP_cached_node_t **head_arr_end = bin->head_arr + bin->mask + 1;
    while (head_arr != head_arr_end) {
	cuflowP_cached_node_t **slot = head_arr;
	cuflowP_cached_node_t *node;
	while ((node = *slot)) {
	    node_normalise_access(node);
	    if (node_drop_condition(node)) {
		--bin->size;
		*slot = node->next;
	    } else
		slot = &node->next;
	}
	++head_arr;
    }
    if (bin->size*FILL_MIN_DENOM < bin->mask*FILL_MIN_NOM && bin->mask/2 > 1)
	fcachebin_resize_locked(bin, (bin->mask + 1)/2);
    bin->access_count_since_pruned = 0;
}

static cu_bool_t
fcachebin_insert(size_t arg_sizeg, void *arg, size_t full_size,
		 cuflowP_cached_node_t **res)
{
    cuflowP_cached_node_t **head;
    cuflowP_cached_node_t *node;
    cu_hash_t hash = arg_hash(arg_sizeg, arg);
    cachebin_t bin = fcachebin(arg_sizeg, hash);
    cu_mutex_lock(&bin->mutex);
    head = &bin->head_arr[hash & bin->mask];
    node = *head;
    while (node) {
	if (arg_eq(arg_sizeg, arg, node_arg(node))) {
	    node_normalise_access(node);
	    node->access_function += node->access_gain;
	    if (++bin->access_count_since_pruned > bin->mask)
		fcachebin_prune_locked(bin);
	    cu_mutex_unlock(&bin->mutex);
	    *res = node;
	    return cu_false;
	}
	node = node->next;
    }
    node = cu_galloc(full_size);
    node->cdisj = 1;	/* one to compute */
    node->next = *head;
    node->access_ticks = cuflowP_cached_conf->current_ticks;
    node->access_function = 0;
#ifndef CU_NDEBUG_CLIENT
    node->access_gain = -1; /* OBS: must be set by callback */
#endif
    *head = node;
    arg_copy(arg_sizeg, node_arg(node), arg);
    if (++bin->size*FILL_MAX_DENOM > bin->mask*FILL_MAX_NOM)
	fcachebin_resize_locked(bin, (bin->mask + 1)*2);
    cu_mutex_unlock(&bin->mutex);
    *res = node;
    return cu_true;
}

void *
cuflowP_cached_call(void *arg, size_t arg_sizeg, size_t full_size)
{
    if (arg_sizeg < ARG_SIZE_COUNT) {
	cuflowP_cached_node_t *node;
	if (fcachebin_insert(arg_sizeg, arg, full_size, &node)) {
	    cu_call0(node_clop(node));
	    cuflow_cdisj_sub1_release_write(&node->cdisj);
	} else
	    cuflow_cdisj_wait_while(&node->cdisj);
#ifndef CU_NDEBUG_CLIENT
	if (node->access_gain == -1)
	    cu_bugf("Cached function did not set gain.");
#endif
	return node;
    } else {
	cu_debug_assert(!"Not implemented.");
	return NULL;
    }
}

void *
cuflowP_sched_cached_call(void *arg, size_t arg_sizeg, size_t full_size)
{
    if (arg_sizeg < ARG_SIZE_COUNT) {
	cuflowP_cached_node_t *node;
	if (fcachebin_insert(arg_sizeg, arg, full_size, &node))
	    cuflow_sched_call_sub1(node_clop(node), &node->cdisj);
	return node;
    } else {
	cu_debug_assert(!"Not implemented.");
	return NULL;
    }
}

void
cuflowP_cached_init(void)
{
    int i, j;
    cuflowP_cached_conf = cuflow_default_cacheconf();
    for (i = 0; i < STRIPE_COUNT; ++i)
	for (j = 0; j < ARG_SIZE_COUNT; ++j)
	    cachebin_init(&cachebin_arr[i][j], j);
}
