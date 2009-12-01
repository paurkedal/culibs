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

#ifndef CUFLOW_CACHECONF_H
#define CUFLOW_CACHECONF_H

#if 0
#include <cuflow/cache.h>
#endif
#include <cu/inherit.h>
#include <cu/dlink.h>
#include <time.h>
#include <atomic_ops.h>

CU_BEGIN_DECLARATIONS
/** \defgroup cuflow_cacheconf_h cuflow/cacheconf.h: Cache Configuration
 ** @{ \ingroup cuflow_x_mod
 **
 ** This is used by \ref cuflow_cached_h "cuflow/cached.h" and \ref
 ** cuflow_cache_h "cuflow/cache.h". */

struct cuflow_cacheconf
{
    /* Set by this library. */
#if 0
    pthread_mutex_t cache_link_mutex;
    struct cu_dlink cache_link;
#endif
    AO_t current_ticks;
    struct timespec target_time;

    /* Set by manager. */
    unsigned int byte_cost_per_tick;
    struct timespec tick_period;

    cu_bool_t (*manager)(cuflow_cacheconf_t conf, struct timespec *t_now);
};

/** Construct a cache configuration.  \a manager is a function which will be
 ** called once during this construction, then occationally afterwards.  If the
 ** parameters were not set prior to this call, then \a manager must set them
 ** on it's first invokation.  It may also update other parameters contituously
 ** to implement heuristics for controlling memory consumption.
 **
 ** \a manager should normally always return true, but if there is need to shut
 ** down the cache configuration, false may be returned.  After that, cache
 ** clock will stop and cached objects will no longer be freed.  Therefore, the
 ** client should make sure to destruct associated caches with \ref
 ** cuflow_cache_dct in conjuction with a false return from \a manager.  */
void cuflow_cacheconf_init(cuflow_cacheconf_t conf,
			   cu_bool_t (*manager)(cuflow_cacheconf_t conf,
						struct timespec *t_now));

/** A predefined cache configuration.  Note that calling this the first time
 ** will spawn one worker thread if there are none running, see \ref
 ** cuflow_workers_h. */
cuflow_cacheconf_t cuflow_default_cacheconf(void);

#if 0
CU_SINLINE void cuflow_cacheconf_lock_cache_range(cuflow_cacheconf_t conf)
{ cu_mutex_lock(&conf->cache_link_mutex); }

CU_SINLINE void cuflow_cacheconf_unlock_cache_range(cuflow_cacheconf_t conf)
{ cu_mutex_unlock(&conf->cache_link_mutex); }

CU_SINLINE cuflow_cache_t cuflow_cacheconf_first_cache(cuflow_cacheconf_t conf)
{ return cu_from(cuflow_cache, cu_dlink, conf->cache_link.next); }

CU_SINLINE cuflow_cache_t cuflow_cacheconf_end_cache(cuflow_cacheconf_t conf)
{ return cu_from(cuflow_cache, cu_dlink, &conf->cache_link); }

CU_SINLINE cuflow_cache_t cuflow_cacheconf_next_cache(cuflow_cache_t cache)
{ return cu_from(cuflow_cache, cu_dlink, cu_to(cu_dlink, cache)->next); }
#endif

/** @} */
CU_END_DECLARATIONS

#endif
