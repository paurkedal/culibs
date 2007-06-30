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

#define USE_GC_INLINE 0

#include <cu/dyn.h>
#include <cu/memory.h>
#include <cu/tstate.h>
#include <gc/gc_mark.h>
#if USE_GC_INLINE
#  include <gc/gc_inline.h>
#endif
#ifdef CUCONF_HAVE_GC_GC_TINY_FL
#  include <gc/gc_tiny_fl.h>
#endif
#if CUCONF_HAVE_GC_DISCLAIM
#  include <gc/gc_disclaim.h>
#endif

static cu_bool_t done_init = 0;
int cuP_obj_objkind;
void **cuP_obj_gfl;

#define EXTRA_BYTES 0

#if USE_GC_INLINE
/* This is needed by GC_FAST_MALLOC_GRANS. */
typedef char *ptr_t;
#define word GC_word
#define obj_link(ptr) (*(void **)(ptr))
#define PREFETCH_FOR_WRITE(ptr) cu_noop()
#define GC_ASSERT(cond) cu_debug_assert(cond)
#endif


#ifdef CUCONF_HAVE_GC_DISCLAIM

void GC_generic_malloc_many(size_t, int, void **);

cu_bool_t
cu_dyn_ghaveavail_f(size_t bytes)
{
    size_t granules;
    cuP_tstate_t st = cuP_tstate();
    cu_debug_assert(done_init);
    bytes += sizeof(cuex_meta_t);
    granules = (bytes + cuP_FL_GRANULE_SIZE - 1 + EXTRA_BYTES)/cuP_FL_GRANULE_SIZE;
    if (granules >= cuP_FL_CNT)
	cu_debug_error(
	    "Use of cu_dyn_ghaveavail_f with too big objects, size = %ld.\n",
	    bytes);
    else {
	void **fl = st->obj_fl_arr + granules;
	return *fl != NULL;
    }
}

void
cu_dyn_gavail_f(size_t bytes, size_t cnt)
{
    size_t granules;
    cuP_tstate_t st = cuP_tstate();
    cu_debug_assert(done_init);
    bytes += sizeof(cuex_meta_t);
    granules = (bytes + cuP_FL_GRANULE_SIZE - 1 + EXTRA_BYTES)/cuP_FL_GRANULE_SIZE;
    if (granules >= cuP_FL_CNT)
	cu_debug_error(
	    "Use of cu_dyn_gavail_f with too big objects, size = %ld.\n",
	    bytes);
    else {
	void **fl = st->obj_fl_arr + granules;
	while (cnt) {
	    if (!*fl) {
		GC_generic_malloc_many(granules*cuP_FL_GRANULE_SIZE,
				       cuP_obj_objkind, fl);
		if (!*fl) {
		    cu_raise_out_of_memory(bytes);
		    cu_debug_unreachable();
		}
	    }
	    fl = *fl;
	    --cnt;
	}
    }
}

void *
cuex_oalloc_f(cuex_meta_t meta, size_t bytes)
{
    size_t granules;
    void *r;
    cuP_tstate_t st = cuP_tstate();
    cu_debug_assert(done_init);
    cu_debug_assert(!cuex_meta_is_type(meta)
	    || (cudyn_is_type(cudyn_type_from_meta(meta))
		&& cudyn_type_is_hctype(cudyn_type_from_meta(meta))));
    bytes += sizeof(cuex_meta_t);
    granules = (bytes + cuP_FL_GRANULE_SIZE - 1 + EXTRA_BYTES)
	     / cuP_FL_GRANULE_SIZE;
#if USE_GC_INLINE
    GC_FAST_MALLOC_GRANS(r, granules, st->obj_fl_arr, 0, cuP_obj_objkind,
			 GC_generic_malloc(bytes, cuP_obj_objkind),
			 *(cuex_meta_t *)r = meta + 1);
#else
    if (granules >= cuP_FL_CNT) {
	r = GC_generic_malloc(bytes, cuP_obj_objkind);
	if (r == NULL) {
	    cu_raise_out_of_memory(bytes);
	    cu_debug_unreachable();
	}
    }
    else {
	void **fl = st->obj_fl_arr + granules;
	if (!*fl) {
	    GC_generic_malloc_many(granules*cuP_FL_GRANULE_SIZE,
				   cuP_obj_objkind, fl);
	    if (!*fl) {
		cu_raise_out_of_memory(bytes);
		cu_debug_unreachable();
	    }
	}
	r = *fl;
	*fl = *(void **)r;
	cu_debug_assert(GC_size(r) >= bytes + EXTRA_BYTES);
    }
#endif
    cu_debug_assert(GC_base(r) == r);
    *(cuex_meta_t *)r = meta + 1;
    return (cuex_meta_t *)r + 1;
}

#else /* !CUCONF_HAVE_GC_DISCLAIM */

void cuP_stdobj_finaliser(void *base, void *cd)
{
    cuex_meta_t meta = *(cuex_meta_t *)base - 1;
    cudyn_stdtype_t t;
    cu_debug_assert(cuex_meta_is_type(meta) &&
		    cudyn_type_is_stdtype(cudyn_type_from_meta(meta)));
    t = cudyn_stdtype_from_meta(meta);
    if (!cu_clop_is_null(t->finalise))
	cu_call(t->finalise, base + sizeof(cuex_meta_t));
}

/* XXX cu_dyn_gavail_f and thread local alloc */
void *
cuex_oalloc_f(cuex_meta_t meta, size_t bytes)
{
    void *r;
    cu_debug_assert(cuex_meta_is_type(meta));
    cu_debug_assert(cudyn_type_is_stdtype(cudyn_type_from_meta(meta)));
    bytes += sizeof(cuex_meta_t);
    r = GC_malloc(bytes);
    if (!r) {
	cu_raise_out_of_memory(bytes);
	cu_debug_unreachable();
    }
    GC_register_finalizer(r, cuP_stdobj_finaliser, NULL, NULL, NULL);
    *(cuex_meta_t *)r = meta + 1;
    return (cuex_meta_t *)r + 1;
}

#endif /* !CUCONF_HAVE_GC_DISCLAIM */


int cuP_hc_disclaim_proc(void *obj, void *null);

void
cuP_obj_mlc_init(void)
{
    cu_debug_assert(!done_init);

#ifdef CUCONF_HAVE_GC_DISCLAIM
    cuP_obj_gfl = GC_new_free_list();
    cuP_obj_objkind = GC_new_kind(cuP_obj_gfl, 0 | GC_DS_LENGTH, 1, 1);

    /* Last arg 1 to mark from fragments of this kind even unmarked ones. This
     * is required when using cache regions to prevent the scenario
     *     • fragent F is no longer used and some operands get re-used
     *     • a fragment key-equal to F is constructed, resurrecting F
     *     • F now has wrong cache data. */
    GC_register_disclaim_proc(cuP_obj_objkind, cuP_hc_disclaim_proc, NULL, 1);
    /* Trigger some GC init needed for gc-7.0_alpha2 before using
     * GC_generic_malloc_many. */
    GC_generic_malloc(1, cuP_obj_objkind);
#endif /* CUCONF_HAVE_GC_DISCLAIM */

    done_init = cu_true;
}
