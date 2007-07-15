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

#ifndef CU_HCONS_RN_H
#define CU_HCONS_RN_H

#include <cu/conf.h>
#if CUCONF_SIZEOF_LONG > 4
#  define CU_HCSET_USE_RAREX 0
#  define CU_HCSET_LOG_CNT 5
#  define CU_HC_USE_GC_MARK 0
#else
#  define CU_HCSET_USE_RAREX 0
#  define CU_HCSET_LOG_CNT 7
#  define CU_HC_USE_GC_MARK 0
#endif
#define CUPRIV_ENABLE_COLL_STATS 0
#define CU_HC_ADJUST_IN_INSERT_ERASE 1

#include <cu/fwd.h>
#include <cu/hcobj.h>
#include <gc/gc_mark.h>
#include <cu/thread.h>
#include <cu/memory.h>
#include <cu/debug.h>
#if CU_HCSET_USE_RAREX
#  include <cu/rarex.h>
#endif
#include <atomic_ops.h>

CU_BEGIN_DECLARATIONS
/*!\file
 * \warning For internal use. This is not even available in all
 * configurations. */

/* The hash set is split into individual sets for ranges of hash codes to
 * avoid clogging locks.  1<<CU_HCSET_LOG_CNT should be greater than the
 * number of threads for best efficiency, maybe 4 times.  Using more is even
 * better, except that it is good to keep the size on the order of the cache
 * granularity to avoid wasting cache.
 *
 * 32 bit platforms (using cu_rarex_t):
 *     sizeof(struct cu_hcset_s) = 64 bytes
 *     CU_HCSET_LOG_CNT = 4:  16 hash sets gives 1 kiB
 *     CU_HCSET_LOG_CNT = 7: 128 hash sets gives 8 kiB
 * 64 bit platforms (using cu_rarex_t):
 *     sizeof(struct cu_hcset_s) = 128 bytes
 *     CU_HCSET_LOG_CNT = 4:  16 hash sets gives  2 kiB
 *     CU_HCSET_LOG_CNT = 7: 128 hash sets gives 16 kiB
 */
#define CU_HCSET_CNT (1 << CU_HCSET_LOG_CNT)

typedef struct cu_hcset_s *cu_hcset_t;
struct cu_hcset_s
{
    size_t mask;
    cu_hcobj_t *arr;
    size_t cnt;
#ifdef CUCONF_ENABLE_THREADS
# if CU_HCSET_USE_RAREX
    cu_rarex_t rarex;
# else
    cu_mutex_t mutex;
# endif
#endif
};
extern struct cu_hcset_s cuP_hcset[CU_HCSET_CNT];

#if CU_HC_GENERATION

extern AO_t cuP_hc_generation;

CU_SINLINE cu_bool_t
cu_hcobj_is_marked(cu_hcobj_t obj)
{ return obj->generation >= AO_load_read(&cuP_hc_generation); }

CU_SINLINE void
cu_hcobj_unmark_lck(cu_hcobj_t obj)
{}

CU_SINLINE void
cu_hcobj_mark_lck(cu_hcobj_t obj)
{ obj->generation = AO_load_read(&cuP_hc_generation)
		  | (obj->generation & 1); }

CU_SINLINE void
cu_hcobj_set_has_prop(cu_hcobj_t obj)
{ AO_or(&obj->generation, 1); }

CU_SINLINE cu_bool_t
cu_hcobj_has_prop(cu_hcobj_t obj)
{ return obj->generation & 1; }

#elif CU_HC_USE_GC_MARK

int GC_is_marked(void *);
void GC_set_mark_bit(void *);

CU_SINLINE cu_bool_t
cu_hcobj_is_marked(cu_hcobj_t obj)
{ return GC_is_marked(obj); }

CU_SINLINE void
cu_hcobj_unmark_lck(cu_hcobj_t obj)
{}

CU_SINLINE void
cu_hcobj_mark_lck(cu_hcobj_t obj)
{ GC_set_mark_bit(obj); }

#else

CU_SINLINE cu_bool_t
cu_hcobj_is_marked(cu_hcobj_t obj)
{ return ((uintptr_t)obj->hcset_next & 1); }

CU_SINLINE void
cu_hcobj_unmark_lck(cu_hcobj_t obj)
{ obj->hcset_next = obj->hcset_next & ~(AO_t)1; }

CU_SINLINE void
cu_hcobj_mark_lck(cu_hcobj_t obj)
{ obj->hcset_next = obj->hcset_next | 1; }

CU_SINLINE void
cu_hcobj_set_has_prop(cu_hcobj_t obj)
{ AO_or(&obj->hcset_next, 2); }

CU_SINLINE cu_bool_t
cu_hcobj_has_prop(cu_hcobj_t obj)
{ return obj->hcset_next & 2; }

#endif


void cu_hcset_cct(cu_hcset_t hcset);

CU_SINLINE cu_hcset_t
cu_hcset(cu_hash_t hash)
{
#if CU_HCSET_CNT > 1
    return &cuP_hcset[hash >> (sizeof(cu_hash_t)*8 - CU_HCSET_LOG_CNT)];
#else
    return &cuP_hcset[0];
#endif
}

#ifdef CUCONF_ENABLE_THREADS
# if CU_HCSET_USE_RAREX
CU_SINLINE void		cu_hcset_lock_read(cu_hcset_t hcset)
{ cu_rarex_lock_read(&hcset->rarex); }
CU_SINLINE void		cu_hcset_unlock_read(cu_hcset_t hcset)
{ cu_rarex_unlock_read(&hcset->rarex); }
CU_SINLINE void		cu_hcset_lock_write(cu_hcset_t hcset)
{ cu_rarex_lock_write(&hcset->rarex); }
CU_SINLINE cu_bool_t	cu_hcset_trylock_write(cu_hcset_t hcset)
{ return cu_rarex_trylock_write(&hcset->rarex); }
CU_SINLINE void		cu_hcset_unlock_write(cu_hcset_t hcset)
{ cu_rarex_unlock_write(&hcset->rarex); }
CU_SINLINE cu_bool_t	cu_hcset_try_promote_lock(cu_hcset_t hcset)
{ return cu_rarex_try_promote(&hcset->rarex); }
# else
CU_SINLINE void		cu_hcset_lock_read(cu_hcset_t hcset)
{ cu_mutex_lock(&hcset->mutex); }
CU_SINLINE void		cu_hcset_unlock_read(cu_hcset_t hcset)
{ cu_mutex_unlock(&hcset->mutex); }
CU_SINLINE void		cu_hcset_lock_write(cu_hcset_t hcset)
{ cu_mutex_lock(&hcset->mutex); }
CU_SINLINE cu_bool_t	cu_hcset_trylock_write(cu_hcset_t hcset)
{ return cu_mutex_trylock(&hcset->mutex); }
CU_SINLINE void		cu_hcset_unlock_write(cu_hcset_t hcset)
{ cu_mutex_unlock(&hcset->mutex); }
CU_SINLINE cu_bool_t	cu_hcset_try_promote_lock(cu_hcset_t hcset)
{ return cu_true; }
# endif
#else
CU_SINLINE void		cu_hcset_lock_read(cu_hcset_t hcset) {}
CU_SINLINE void		cu_hcset_unlock_read(cu_hcset_t hcset) {}
CU_SINLINE void		cu_hcset_lock_write(cu_hcset_t hcset) {}
CU_SINLINE cu_bool_t	cu_hcset_trylock_write(cu_hcset_t hcset)
{ return cu_true; }
CU_SINLINE void		cu_hcset_unlock_write(cu_hcset_t hcset) {}
CU_SINLINE cu_bool_t	cu_hcset_try_promote_lock(cu_hcset_t hcset)
{ return cu_true; }
#endif

#if !CU_HC_ADJUST_IN_INSERT_ERASE
void cu_hcset_adjust(cu_hcset_t hcset);
void cu_hcset_adjust_wlck(cu_hcset_t hcset);
#endif

CU_SINLINE cu_hcobj_t
cu_hcset_hasheqv_begin(cu_hcset_t hcset, cu_hash_t hash)
{ return hcset->arr[hash & hcset->mask]; }

CU_SINLINE cu_hcobj_t
cu_hcset_hasheqv_next(cu_hcobj_t obj)
{
#if CU_HC_GENERATION || CU_HC_USE_GC_MARK
    return (cu_hcobj_t)~obj->hcset_next;
#else
    return (cu_hcobj_t)~(obj->hcset_next | 3);
#endif
}

CU_END_DECLARATIONS

#endif
