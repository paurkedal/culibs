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

#include <cuex/binding.h>
#include <cuex/oprdefs.h>
#include <cuex/opn.h>
#include <cuex/var.h>
#include <cuex/compound.h>
#include <cuex/intf.h>
#include <cucon/list.h>
#include <cucon/pmap.h>
#include <cu/int.h>
#include <cu/ptr_seq.h>
#include <limits.h>

cuex_t
cuexP_bfree_adjusted(cuex_t e, int l_diff, int l_top)
{
    cuex_meta_t e_meta = cuex_meta(e);
    if (cuex_meta_is_opr(e_meta)) {
	if (cuex_og_hole_contains(e_meta)) {
	    int l = cuex_oa_hole_index(e_meta);
	    if (l > l_top)
		return cuex_hole(l + l_diff);
	}
	else {
	    if (cuex_og_binder_contains(e_meta))
		++l_top;
	    CUEX_OPN_TRAN(e_meta, e, ep,
			  cuexP_bfree_adjusted(ep, l_diff, l_top));
	}
    }
    return e;
}

int
cuex_max_binding_depth(cuex_t e)
{
    cuex_meta_t e_meta = cuex_meta(e);
    if (cuex_meta_is_opr(e_meta)) {
	int i = 0;
	CUEX_OPN_ITER(e_meta, e, ep,
		      i = cu_int_max(i, cuex_max_binding_depth(ep)));
	if (cuex_og_binder_contains(e_meta))
	    ++i;
	return i;
    } else if (cuex_meta_is_type(e_meta)) {
	cuoo_type_t type = cuoo_type_from_meta(e_meta);
	cuex_intf_compound_t e_c = cuoo_type_impl_ptr(type, CUEX_INTF_COMPOUND);
	if (e_c) {
	    int i = 0;
	    cuex_t ep;
	    cu_ptr_source_t src;
	    src = cuex_compound_pref_iter_source(e_c, e);
	    while ((ep = cu_ptr_source_get(src)))
		i = cu_int_max(i, cuex_max_binding_depth(ep));
	    return i;
	}
    }
    return 0;
}

typedef struct sifi_cr_frame_s *sifi_cr_frame_t;
struct sifi_cr_frame_s
{
    cu_inherit (cucon_listnode_s);
    struct cucon_list_s fv_list;
    int fv_level;	/* Stack level where curretly linked, or -1. */
    int index;
};

static void
sifi_indexing(cuex_t e, unsigned int flags, int l_cur, sifi_cr_frame_t sp,
	      cucon_pmap_t bsite_to_ivar)
{
    cuex_meta_t e_meta = cuex_meta(e);
    if (cuex_meta_is_opr(e_meta)) {
	if (cuex_og_hole_contains(e_meta)) {
	    int l;
	    struct sifi_cr_frame_s *frame_ref, *frame_cur;

	    l = cuex_oa_hole_index(e_meta);
	    if (l >= l_cur) /* is free in full expression */
		return;
	    frame_ref = sp + l;
	    if (frame_ref->fv_level == -2) /* μ-bind */
		return;
	    frame_cur = sp;
	    if (frame_ref->fv_level < l_cur) {
		if (frame_ref->fv_level >= 0)
		    cucon_list_erase_node(cu_to(cucon_listnode, frame_ref));
		cucon_list_append_node_cct(&frame_cur->fv_list,
					   cu_to(cucon_listnode, frame_ref));
		frame_ref->fv_level = l_cur;
	    }
	}
	else if (cuex_og_binder_contains(e_meta)) {
	    struct sifi_cr_frame_s *frame_sub, *frame_cur;
	    int index_p1;

	    /* Create sub-frame (at l_cur + 1) and process sub-expressions. */
	    frame_cur = sp;
	    frame_sub = --sp;
	    cucon_list_cct(&frame_sub->fv_list);
	    frame_sub->index = 0;
	    if (e_meta == CUEX_O1_MU) {
		frame_sub->fv_level = -2;
		sifi_indexing(cuex_opn_at(e, 0), flags, l_cur + 1,
			      sp, bsite_to_ivar);
	    }
	    else {
		frame_sub->fv_level = -1;
		CUEX_OPN_ITER(e_meta, e, ep,
			      sifi_indexing(ep, flags, l_cur + 1, sp,
					    bsite_to_ivar));
	    }

	    /* Optionally prune unused variables. */
	    if (flags & CUEX_BI_SIFI_FLAG_PRUNE) {
		cucon_listnode_t node_ref;
		cu_bool_t occurs;

		/* Check if the variable of the frame occurs in a
		 * sub-expression... */
		occurs = cu_false;
		for (node_ref = cucon_list_begin(&frame_sub->fv_list);
		     node_ref != cucon_list_end(&frame_sub->fv_list);
		     node_ref = cucon_listnode_next(node_ref)) {
		    sifi_cr_frame_t frame_ref;
		    frame_ref = cu_from(sifi_cr_frame, cucon_listnode,
					node_ref);
		    if (frame_ref == frame_sub) {
			occurs = cu_true;
			cucon_list_erase_node(node_ref);
			break;
		    }
		}

		/* ... If not, just re-link all frames except the current up
		 * to be processed at the surrounding level. */
		if (!occurs) {
		    if (l_cur) {
			cucon_listnode_t node;
			for (node = cucon_list_begin(&frame_sub->fv_list);
			     node != cucon_list_end(&frame_sub->fv_list);
			     node = cucon_listnode_next(node)) {
			    sifi_cr_frame_t fr;
			    fr = cu_from(sifi_cr_frame, cucon_listnode, node);
			    fr->fv_level = l_cur;
			}
			cucon_list_append_list_dct(&frame_cur->fv_list,
						   &frame_sub->fv_list);
		    }
		    else
			cu_debug_assert(
				cucon_list_is_empty(&frame_sub->fv_list));
		    cucon_pmap_insert_int(bsite_to_ivar, e, INT_MAX);
		    return;
		}
	    }

	    /* The min allowed index has been computed, save it. */
	    cucon_pmap_insert_int(bsite_to_ivar, e, frame_sub->index);

	    /* Frames (above the current) of occuring variables are linked to
	     * the current one.  Update crashing indices re-link frames up to
	     * the surrounding frame. */
	    index_p1 = frame_sub->index + 1;
	    while (!cucon_list_is_empty(&frame_sub->fv_list)) {
		cucon_listnode_t node_ref;
		sifi_cr_frame_t frame_ref;

		node_ref = cucon_list_begin(&frame_sub->fv_list);
		cucon_list_erase_node(node_ref);
		frame_ref = cu_from(sifi_cr_frame, cucon_listnode, node_ref);
		if (frame_ref == frame_sub)
		    continue;
		if (index_p1 > frame_ref->index)
		    frame_ref->index = index_p1;
		cu_debug_assert(frame_ref->fv_level != -2);
		frame_ref->fv_level = l_cur;
		cucon_list_append_node_cct(&frame_cur->fv_list, node_ref);
	    }
	}
	else
	    CUEX_OPN_ITER(e_meta, e, ep,
			  sifi_indexing(ep, flags, l_cur, sp,
					bsite_to_ivar));
    }
}

void
cuex_bi_sifi_indexing_accu(cuex_t e, unsigned int flags, cucon_pmap_t accu)
{
    int max_depth = cuex_max_binding_depth(e) + 1;
    sifi_cr_frame_t sp;
    
    sp = cu_salloc(max_depth*sizeof(struct sifi_cr_frame_s));
    sp += max_depth;

    sifi_indexing(e, flags, 0, sp, accu);
}

cucon_pmap_t
cuex_bi_sifi_indexing(cuex_t e, unsigned int flags)
{
    cucon_pmap_t accu = cucon_pmap_new();
    cuex_bi_sifi_indexing_accu(e, flags, accu);
    return accu;
}
