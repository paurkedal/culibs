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

#include <cuoo/oalloc.h>
#include <cuoo/intf.h>
#include <cu/memory.h>
#include <cu/tstate.h>
#include <cu/ptr.h>
#include <gc/gc_mark.h>
#if USE_GC_INLINE
#  include <gc/gc_inline.h>
#endif
#ifdef CUCONF_HAVE_GC_GC_TINY_FL
#  include <gc/gc_tiny_fl.h>
#endif
#if CUCONF_ENABLE_GC_DISCLAIM
#  include <gc/gc_disclaim.h>
#endif

static cu_bool_t _done_init = 0;
int cuooP_ord_objkind, cuooP_unord_objkind;
void **cuooP_ord_gfl, **cuooP_unord_gfl;

#define EXTRA_BYTES 0

#if USE_GC_INLINE
/* This is needed by GC_FAST_MALLOC_GRANS. */
typedef char *ptr_t;
#define word GC_word
#define obj_link(ptr) (*(void **)(ptr))
#define PREFETCH_FOR_WRITE(ptr) cu_noop()
#define GC_ASSERT(cond) cu_debug_assert(cond)
#endif


#ifdef CUCONF_ENABLE_GC_DISCLAIM

void GC_generic_malloc_many(size_t, int, void **);

void *
cuexP_oalloc_ord_fin_raw(cuex_meta_t meta, size_t sizeg)
{
    void *r;
    cu_debug_assert(_done_init);
    if (sizeg >= cuP_FL_CNT) {
	r = GC_generic_malloc(sizeg*CU_GRAN_SIZE, cuooP_ord_objkind);
	if (r == NULL) {
	    cu_raise_out_of_memory(sizeg*CU_GRAN_SIZE);
	    cu_debug_unreachable();
	}
    } else {
	cuP_tstate_t st = cuP_tstate();
	void **fl = st->ord_fl_arr + sizeg;
	if (!*fl) {
	    GC_generic_malloc_many(sizeg*CU_GRAN_SIZE, cuooP_ord_objkind, fl);
	    if (!*fl) {
		cu_raise_out_of_memory(sizeg*CU_GRAN_SIZE);
		cu_debug_unreachable();
	    }
	}
	r = *fl;
	*fl = *(void **)r;
    }
    *(cuex_meta_t *)r = meta + 1;
    return (cuex_meta_t *)r + 1;
}

void *
cuexP_oalloc_unord_fin_raw(cuex_meta_t meta, size_t sizeg)
{
    void *r;
    cu_debug_assert(_done_init);
    if (sizeg >= cuP_FL_CNT) {
	r = GC_generic_malloc(sizeg*CU_GRAN_SIZE, cuooP_unord_objkind);
	if (r == NULL) {
	    cu_raise_out_of_memory(sizeg*CU_GRAN_SIZE);
	    cu_debug_unreachable();
	}
    } else {
	cuP_tstate_t st = cuP_tstate();
	void **fl = st->unord_fl_arr + sizeg;
	if (!*fl) {
	    GC_generic_malloc_many(sizeg*CU_GRAN_SIZE, cuooP_unord_objkind, fl);
	    if (!*fl) {
		cu_raise_out_of_memory(sizeg*CU_GRAN_SIZE);
		cu_debug_unreachable();
	    }
	}
	r = *fl;
	*fl = *(void **)r;
    }
    *(cuex_meta_t *)r = meta + 1;
    return (cuex_meta_t *)r + 1;
}

#else /* !CUCONF_ENABLE_GC_DISCLAIM */

#ifdef CUOO_INTF_FINALISE
static void
_stdobj_finaliser(void *base, void *cd)
{
    cuex_meta_t meta = *(cuex_meta_t *)base - 1;
    cuoo_type_t t;
    cu_debug_assert(cuex_meta_is_type(meta) &&
		    cuoo_type_is_metatype(cuoo_type_from_meta(meta)));
    t = cuoo_type_from_meta(meta);
    if (t->shape & CUOO_SHAPEFLAG_FIN)
	(*t->impl)(CUOO_INTF_FINALISE, cu_ptr_add(base, sizeof(cuex_meta_t)));
}
#endif

void *
cuexP_oalloc_ord_fin_raw(cuex_meta_t meta, size_t sizeg)
{
    void *r;
    cu_debug_assert(_done_init);
    cu_debug_assert(cuex_meta_is_type(meta));
    cu_debug_assert(cuoo_type_is_metatype(cuoo_type_from_meta(meta)));
    r = GC_malloc(sizeg*CU_GRAN_SIZE);
    if (!r) {
	cu_raise_out_of_memory(sizeg*CU_GRAN_SIZE);
	cu_debug_unreachable();
    }
#ifdef CUOO_INTF_FINALISE
    GC_register_finalizer(r, _stdobj_finaliser, NULL, NULL, NULL);
#endif
    *(cuex_meta_t *)r = meta + 1;
    return (cuex_meta_t *)r + 1;
}

void *
cuexP_oalloc_unord_fin_raw(cuex_meta_t meta, size_t sizeg)
{
    void *r;
    cu_debug_assert(_done_init);
    cu_debug_assert(cuex_meta_is_type(meta));
    cu_debug_assert(cuoo_type_is_metatype(cuoo_type_from_meta(meta)));
    r = GC_malloc(sizeg*CU_GRAN_SIZE);
    if (!r) {
	cu_raise_out_of_memory(sizeg*CU_GRAN_SIZE);
	cu_debug_unreachable();
    }
#ifdef CUOO_INTF_FINALISE
    GC_register_finalizer_no_order(r, _stdobj_finaliser, NULL, NULL, NULL);
#endif
    *(cuex_meta_t *)r = meta + 1;
    return (cuex_meta_t *)r + 1;
}

#endif /* !CUCONF_ENABLE_GC_DISCLAIM */


int cuooP_hcons_disclaim_proc(void *obj, void *null);

void
cuooP_oalloc_init(void)
{
    cu_debug_assert(!_done_init);

#ifdef CUCONF_ENABLE_GC_DISCLAIM

    /* Object Kinds for Hash-Consing and Finalisation
     *
     * The last argument to GC_register_disclaim_proc determines if the
     * collector will mark from all fragments of this kind even those which are
     * themselves unmarked.  Passing non-zero here gives "ordered"
     * disclaim/finalisation, meaning that objects referred to from a
     * reclaimable object will not be reclaimable itself before the next GC.
     *
     * Ordered disclaim is required when using cache regions on hash-consed
     * objects to prevent the following scenario:
     *     • fragent F is no longer used and some operands get re-used
     *     • a fragment key-equal to F is constructed, resurrecting F
     *     • F now has wrong cache data. */

    /* Create the ordered object kind. */
    cuooP_ord_gfl = GC_new_free_list();
    cuooP_ord_objkind = GC_new_kind(cuooP_ord_gfl, 0 | GC_DS_LENGTH, 1, 1);
    GC_register_disclaim_proc(cuooP_ord_objkind, cuooP_hcons_disclaim_proc,
			      NULL, 1);

    /* Create the unordered object kind. */
    cuooP_unord_gfl = GC_new_free_list();
    cuooP_unord_objkind = GC_new_kind(cuooP_unord_gfl, 0 | GC_DS_LENGTH, 1, 1);
    GC_register_disclaim_proc(cuooP_unord_objkind, cuooP_hcons_disclaim_proc,
			      NULL, 0);

    /* Need to trigger some initialisation before using
     * GC_generic_malloc_many. */
    GC_generic_malloc(1, cuooP_ord_objkind);
    GC_generic_malloc(1, cuooP_unord_objkind);

#endif /* CUCONF_ENABLE_GC_DISCLAIM */

    _done_init = cu_true;
}
