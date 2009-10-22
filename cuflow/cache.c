/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2006--2007  Petter Urkedal <urkedal@nbi.dk>
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

#include <cuflow/cache.h>
#include <cuflow/cacheconf.h>
#include <cuflow/workers.h>
#include <cuflow/timespec.h>
#include <cu/wordarr.h>
#include <cu/hash.h>
#include <cu/memory.h>
#include <math.h>

#define FILL_MIN_NOM 1
#define FILL_MIN_DENOM 2
#define FILL_MAX_NOM 4
#define FILL_MAX_DENOM 3
#define MIN_CAP 2

#define CUFLOWP_CACHE_BIN_BITNO \
    (sizeof(cu_hash_t)*8 - CUFLOWP_CACHE_LOG2_BIN_COUNT)
#define HASH_BIN(hash) ((hash) >> CUFLOWP_CACHE_BIN_BITNO)
#define HASH_SLOT(hash, cap) ((hash) & (cap - 1))
#define HDR CUFLOWP_CACHEOBJ_HDR
#define CUFLOW_CACHEOBJ_FNCODE(obj) ((obj)->fncode)
#define CACHEOBJ_KEY_SIZEW(obj) \
    CUFLOW_FNCODE_KEY_SIZEW(CUFLOW_CACHEOBJ_FNCODE(obj))

void
cuflow_cache_cct(cuflow_cache_t cache, cuflow_cacheconf_t conf,
		 cuflow_cacheobj_t (**fn_arr)(cuflow_cacheobj_t key))
{
    int i;
    cache->conf = conf;
    cache->fn_arr = fn_arr;
    for (i = 0; i < CUFLOWP_CACHE_BIN_COUNT; ++i) {
	cuflowP_cachebin_t bin = &cache->bin_arr[i];
	cu_mutex_init(&bin->mutex);
	bin->cap = MIN_CAP;
	bin->size = 0;
	bin->access_since_pruned = 0;
	bin->link_arr = cu_cgalloc(MIN_CAP*sizeof(struct cuflowP_cachebin_s));
    }
#if 0
    cu_mutex_lock(&conf->cache_link_mutex);
    cu_dlink_insert_before(&conf->cache_link, cu_to(cu_dlink, cache));
    cu_mutex_unlock(&conf->cache_link_mutex);
#endif
}

void
cuflow_cache_dct(cuflow_cache_t cache)
{
#if 0
    cu_mutex_lock(&cache->conf->cache_link_mutex);
    cu_dlink_erase(cu_to(cu_dlink, cache));
    cu_mutex_unlock(&cache->conf->cache_link_mutex);
#endif
}

cuflow_cacheobj_t
cuflow_cacheobj_alloc(size_t full_size, cuflow_cacheobj_t key)
{
    cuflowP_cacheobjhdr_t base;
    cuflow_cacheobj_t obj;
    size_t key_sizew = CACHEOBJ_KEY_SIZEW(key);
    full_size += sizeof(struct cuflowP_cacheobjhdr_s);
    base = cu_galloc(full_size);
    obj = (cuflow_cacheobj_t)(base + 1);
    cu_wordarr_copy(key_sizew, (cu_word_t *)obj, (cu_word_t *)key);
    return obj;
}

CU_SINLINE cu_bool_t
_cacheobj_eq(cu_word_t fncode, cuflow_cacheobj_t obj0, cuflow_cacheobj_t obj1)
{
    size_t key_sizew = CUFLOW_FNCODE_KEY_SIZEW(fncode);
    return cu_wordarr_eq(key_sizew, (cu_word_t *)obj0, (cu_word_t *)obj1);
}

CU_SINLINE cu_hash_t
_cacheobj_hash(cu_word_t fncode, cuflow_cacheobj_t obj)
{
    size_t key_sizew = CUFLOW_FNCODE_KEY_SIZEW(fncode);
    return cu_wordarr_hash(key_sizew - 1, (cu_word_t *)obj + 1, fncode);
}

static void
_resize_lck(cuflowP_cachebin_t bin, size_t new_cap)
{
    cuflow_cacheobj_t *old_link_arr = bin->link_arr;
    cuflow_cacheobj_t *old_link_arr_end = old_link_arr + bin->cap;
    bin->cap = new_cap;
    bin->link_arr = cu_cgalloc(sizeof(struct cuflowP_cachebin_s)*new_cap);
    while (old_link_arr != old_link_arr_end) {
	cuflow_cacheobj_t obj = *old_link_arr;
	while (obj) {
	    cuflow_cacheobj_t next_obj = HDR(obj)->next;
	    cu_word_t fncode = CUFLOW_CACHEOBJ_FNCODE(obj);
	    cu_hash_t hash = _cacheobj_hash(fncode, obj);
	    cuflow_cacheobj_t *slot;
	    slot = &bin->link_arr[HASH_SLOT(hash, new_cap)];
	    HDR(obj)->next = *slot;
	    *slot = obj;
	    obj = next_obj;
	}
	++old_link_arr;
    }
}

CU_SINLINE cu_bool_t
_drop_condition(cuflow_cacheconf_t conf, cuflow_cacheobj_t obj)
{
    return (unsigned long)conf->byte_cost_per_tick
	 > (unsigned long)HDR(obj)->access_function;
}

CU_SINLINE void
_update(cuflow_cacheconf_t conf, cuflow_cacheobj_t obj)
{
    unsigned int current_ticks = conf->current_ticks;
    unsigned int delta_ticks = current_ticks - HDR(obj)->access_ticks;
    HDR(obj)->access_ticks = current_ticks;
    if (delta_ticks >= sizeof(HDR(obj)->access_function)*8)
	HDR(obj)->access_function = 0;
    else
	HDR(obj)->access_function >>= delta_ticks;
}

static void
_prune_and_adjust_lck(cuflow_cache_t cache, cuflowP_cachebin_t bin)
{
    cuflow_cacheobj_t *link_arr = bin->link_arr;
    cuflow_cacheobj_t *link_arr_end = bin->link_arr + bin->cap;
    cuflow_cacheconf_t conf = cache->conf;

    /* Go through bin and prune recently unused objects */
    while (link_arr != link_arr_end) {
	cuflow_cacheobj_t *obj_slot = link_arr;
	cuflow_cacheobj_t obj;
	while ((obj = *obj_slot)) {
	    _update(conf, obj);
	    if (_drop_condition(conf, obj)) {
		--bin->size;
		*obj_slot = HDR(obj)->next;
	    }
	    else
		obj_slot = &HDR(obj)->next;
	}
	++link_arr;
    }

    /* Adjust capacity if necessary. */
    if (bin->size*FILL_MAX_DENOM > bin->cap*FILL_MAX_NOM)
	_resize_lck(bin, bin->cap*2);
    else if (bin->size*FILL_MIN_DENOM < bin->cap*FILL_MIN_NOM
	     && bin->cap/2 >= MIN_CAP)
	_resize_lck(bin, bin->cap/2);
    bin->access_since_pruned = 0;
}

cuflow_cacheobj_t
cuflow_cache_call(cuflow_cache_t cache, cu_word_t fncode,
		  cuflow_cacheobj_t key)
{
    cu_hash_t hash;
    cuflowP_cachebin_t bin;
    cuflow_cacheobj_t *slot;
    cuflow_cacheobj_t obj;

    key->fncode = fncode;

    /* Lookup key, return if found. */
    hash = _cacheobj_hash(fncode, key);
    bin = &cache->bin_arr[HASH_BIN(hash)];
    cu_mutex_lock(&bin->mutex);
    slot = &bin->link_arr[HASH_SLOT(hash, bin->cap)];
    obj = *slot;
    while (obj) {
	if (_cacheobj_eq(fncode, key, obj)) {
	    _update(cache->conf, obj);
	    HDR(obj)->access_function += HDR(obj)->gain;
	    if (++bin->access_since_pruned == bin->cap)
		_prune_and_adjust_lck(cache, bin);
	    cu_mutex_unlock(&bin->mutex);
	    return obj;
	}
	obj = HDR(obj)->next;
    }

    /* Not found, prune and adjust if needed, cache computation. */
    if (bin->size*FILL_MAX_DENOM > bin->cap*FILL_MAX_NOM)
	_prune_and_adjust_lck(cache, bin);
    obj = cache->fn_arr[CUFLOW_FNCODE_SLOT(fncode)](key);

    /* Insert new object. */
    HDR(obj)->next = *slot;
    if (HDR(obj)->gain == 0)
	cu_bugf("Callback %d for cache object did not set gain.",
		CUFLOW_FNCODE_SLOT(fncode));
    HDR(obj)->access_ticks = cache->conf->current_ticks;
    HDR(obj)->access_function = HDR(obj)->gain;
    *slot = obj;
    ++bin->size;
    cu_mutex_unlock(&bin->mutex);
    return obj;
}


/* Cache Configuration
 * =================== */

cu_clos_def(_cacheconf_update,
	    cu_prot(void, struct timespec *t_now),
	    ( cuflow_cacheconf_t conf; ))
{
    cu_clos_self(_cacheconf_update);
    cuflow_cacheconf_t conf = self->conf;
    if (!(*conf->manager)(conf, t_now))
	return;
    do {
	cuflow_timespec_add(&conf->target_time, &conf->tick_period);
	++conf->current_ticks;
    } while (cuflow_timespec_lt(&conf->target_time, t_now));
    cuflow_workers_call_at(_cacheconf_update_ref(self), &conf->target_time);
}

void
cuflow_cacheconf_cct(cuflow_cacheconf_t conf,
		     cu_bool_t (*manager)(cuflow_cacheconf_t conf,
					  struct timespec *t_now))
{
    _cacheconf_update_t *confupdate;
    struct timespec t_now;
#if 0
    cu_mutex_init(&conf->cache_link_mutex);
    cu_dlink_init_singleton(&conf->cache_link);
#endif
    clock_gettime(CLOCK_REALTIME, &t_now);
    if (!(*manager)(conf, &t_now))
	return;
    cuflow_timespec_add(&t_now, &conf->tick_period);
    cuflow_workers_spawn_at_least(1);
    cuflow_timespec_assign_sum(&conf->target_time, &t_now, &conf->tick_period);
    conf->manager = manager;
    confupdate = cu_gnew(_cacheconf_update_t);
    confupdate->conf = conf;
    cuflow_workers_call_at(_cacheconf_update_prep(confupdate),
			   &conf->target_time);
}

static struct cuflow_cacheconf_s _default_cacheconf;

static cu_bool_t
_default_manager(cuflow_cacheconf_t conf, struct timespec *t_now)
{
    return cu_true;
}

cuflow_cacheconf_t
cuflow_default_cacheconf()
{
    static int done_init = 0;
    if (!done_init) {
	_default_cacheconf.tick_period.tv_sec = 0;
	_default_cacheconf.tick_period.tv_nsec = 10000000; /* 10 ms */
	_default_cacheconf.byte_cost_per_tick = 100;
	cuflow_cacheconf_cct(&_default_cacheconf, _default_manager);
    }
    return &_default_cacheconf;
}
