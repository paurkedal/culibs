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
#include <cucon/stack.h>
#include <cucon/uset.h>
#include <cucon/bitvect.h>
#include <cu/int.h>
#include <cu/ptr_seq.h>
#include <limits.h>

static cuex_t
_bfree_adjusted(cuex_t e, int l_diff, int l_top)
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
			  _bfree_adjusted(ep, l_diff, l_top));
	}
    } else if (cuex_meta_is_type(e_meta)) {
	cuoo_type_t type = cuoo_type_from_meta(e_meta);
	cuex_intf_compound_t e_c;
	e_c = cuoo_type_impl_ptr(type, CUEX_INTF_COMPOUND);
	if (e_c) {
	    cuex_t ep;
	    cu_ptr_junctor_t ij;
	    ij = cuex_compound_pref_image_junctor(e_c, e);
	    while ((ep = cu_ptr_junctor_get(ij)))
		cu_ptr_junctor_put(ij, _bfree_adjusted(ep, l_diff, l_top));
	    return cu_ptr_junctor_finish(ij);
	}
    }
    return e;
}

cuex_t
cuex_bfree_adjusted(cuex_t e, int l_diff)
{
    if (l_diff == 0)
	return e;
    else
	return _bfree_adjusted(e, l_diff, -1);
}

void
cuex_bfree_iter(cuex_t e, cu_clop(f, void, int, int), int l_top)
{
    cuex_meta_t e_meta = cuex_meta(e);
    if (cuex_meta_is_opr(e_meta)) {
	if (cuex_og_hole_contains(e_meta)) {
	    int l = cuex_oa_hole_index(e_meta);
	    if (l >= l_top)
		cu_call(f, l, l_top);
	}
	else {
	    if (cuex_og_binder_contains(e_meta))
		++l_top;
	    CUEX_OPN_ITER(e_meta, e, ep, cuex_bfree_iter(ep, f, l_top));
	}
    }
    else if (cuex_meta_is_type(e_meta)) {
	cuoo_type_t type = cuoo_type_from_meta(e_meta);
	cuex_intf_compound_t impl;
	impl = cuoo_type_impl_ptr(type, CUEX_INTF_COMPOUND);
	if (impl) {
	    cuex_t ep;
	    cu_ptr_source_t src;
	    src = cuex_compound_pref_iter_source(impl, e);
	    while ((ep = cu_ptr_source_get(src)))
		cuex_bfree_iter(ep, f, l_top);
	}
    }
}

cuex_t
cuex_bfree_tran(cuex_t e, cu_clop(f, cuex_t, int, int), int l_top)
{
    cuex_meta_t e_meta = cuex_meta(e);
    if (cuex_meta_is_opr(e_meta)) {
	if (cuex_og_hole_contains(e_meta)) {
	    int l = cuex_oa_hole_index(e_meta);
	    if (l >= l_top)
		return cu_call(f, l, l_top);
	}
	else {
	    if (cuex_og_binder_contains(e_meta))
		++l_top;
	    CUEX_OPN_TRAN(e_meta, e, ep, cuex_bfree_tran(ep, f, l_top));
	}
    }
    else if (cuex_meta_is_type(e_meta)) {
	cuoo_type_t type = cuoo_type_from_meta(e_meta);
	cuex_intf_compound_t impl;
	impl = cuoo_type_impl_ptr(type, CUEX_INTF_COMPOUND);
	if (impl) {
	    cuex_t ep;
	    cu_ptr_junctor_t ij;
	    ij = cuex_compound_pref_image_junctor(impl, e);
	    while ((ep = cu_ptr_junctor_get(ij)))
		cu_ptr_junctor_put(ij, cuex_bfree_tran(ep, f, l_top));
	    return cu_ptr_junctor_finish(ij);
	}
    }
    return e;
}

cu_bool_t
cuex_bfree_match(cu_clop(f, cu_bool_t, int, cuex_t, int),
		 cuex_t p, cuex_t e, int l_top)
{
    cuex_meta_t p_meta = cuex_meta(p);
    if (p == e)
	return cu_true;
    else if (cuex_meta_is_opr(p_meta)) {
	if (cuex_og_hole_contains(p_meta)) {
	    int l = cuex_oa_hole_index(p_meta);
	    if (l >= l_top)
		return cu_call(f, l, e, l_top);
	    else
		return p == e;
	}
	else if (p_meta != cuex_meta(e))
	    return cu_false;
	else {
	    int i, r;
	    if (cuex_og_binder_contains(p_meta))
		++l_top;
	    r = cuex_opr_r(p_meta);
	    for (i = 0; i < r; ++i)
		if (!cuex_bfree_match(f, cuex_opn_at(p, i), cuex_opn_at(e, i),
				      l_top))
		    return cu_false;
	    return cu_true;
	}
    }
    else if (p_meta != cuex_meta(e))
	return cu_false;
    else if (cuex_meta_is_type(p_meta)) {
	cuoo_type_t type = cuoo_type_from_meta(p_meta);
	cuex_intf_compound_t impl;
	impl = cuoo_type_impl_ptr(type, CUEX_INTF_COMPOUND);
	if (impl) {
	    cuex_t pp, ep;
	    cu_ptr_source_t p_src, e_src;
	    p_src = cuex_compound_pref_iter_source(impl, p);
	    e_src = cuex_compound_pref_iter_source(impl, e);
	    while ((pp = cu_ptr_source_get(p_src)) &&
		   (ep = cu_ptr_source_get(e_src)))
		if (!cuex_bfree_match(pp, ep, f, l_top))
		    return cu_false;
	    return cu_true;
	}
    }
    return p == e;
}

cu_clos_def(_bfree_into_uset_helper, cu_prot(void, int l, int l_top),
    ( cucon_uset_t set; int l_max; ))
{
    cu_clos_self(_bfree_into_uset_helper);
    l -= l_top;
    cucon_uset_insert(self->set, l);
    self->l_max = cu_int_max(self->l_max, l);
}

int
cuex_bfree_into_uset(cuex_t e, int l_top, cucon_uset_t set)
{
    _bfree_into_uset_helper_t cb;
    cb.set = set;
    cb.l_max = INT_MIN;
    cuex_bfree_iter(e, _bfree_into_uset_helper_prep(&cb), l_top);
    return cb.l_max;
}

cu_clos_def(_bfree_into_bitvect_helper, cu_prot(void, int l, int l_top),
  ( cucon_bitvect_t seen; int seen_count; ))
{
    cu_clos_self(_bfree_into_bitvect_helper);
    l -= l_top;
    cu_debug_assert(l >= 0);
    if (l >= 0 && l < cucon_bitvect_size(self->seen)
	    && !cucon_bitvect_at(self->seen, l)) {
	cucon_bitvect_set_at(self->seen, l, cu_true);
	++self->seen_count;
    }
}

int
cuex_bfree_into_bitvect(cuex_t e, int l_top, cucon_bitvect_t seen)
{
    _bfree_into_bitvect_helper_t helper;
    helper.seen_count = 0;
    helper.seen = seen;
    cuex_bfree_iter(e, _bfree_into_bitvect_helper_prep(&helper), l_top);
    return helper.seen_count;
}

cuex_t
cuex_reindex_by_int_stack(cuex_t e,
			  int stack_top_level, int stack_span,
			  cucon_stack_t stack)
{
    cuex_meta_t e_meta = cuex_meta(e);
    if (cuex_meta_is_opr(e_meta)) {
	if (cuex_og_hole_contains(e_meta)) {
	    int i = cuex_oa_hole_index(e_meta);
	    if (i >= stack_top_level) {
		int j = i - stack_top_level;
		int stack_size = cucon_stack_size_int(stack);
		int l_out;
		if (j >= stack_size)
		    l_out = j - stack_size + stack_span;
		else
		    l_out = cucon_stack_at_int(stack, j);
		return cuex_hole(l_out + stack_top_level);
	    }
	    else
		return e;
	}
	else {
	    if (cuex_og_binder_contains(e_meta))
		++stack_top_level;
	    CUEX_OPN_TRAN(e_meta, e, ep,
			  cuex_reindex_by_int_stack(ep, stack_top_level,
						    stack_span, stack));
	    return e;
	}
    }
    else if (cuex_meta_is_type(e_meta)) {
	cuoo_type_t e_type = cuoo_type_from_meta(e_meta);
	cuex_intf_compound_t comp;
	comp = cuoo_type_impl_ptr(e_type, CUEX_INTF_COMPOUND);
	if (comp) {
	    cuex_t ep;
	    cu_ptr_junctor_t jct;
	    jct = cuex_compound_pref_image_junctor(comp, e);
	    while ((ep = cu_ptr_junctor_get(jct))) {
		ep = cuex_reindex_by_int_stack(ep, stack_top_level,
					       stack_span, stack);
		cu_ptr_junctor_put(jct, ep);
	    }
	    return cu_ptr_junctor_finish(jct);
	}
	else
	    return e;
    }
    else
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

typedef struct sifi_cr_frame *sifi_cr_frame_t;
struct sifi_cr_frame
{
    cu_inherit (cucon_listnode);
    struct cucon_list fv_list;
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
	    struct sifi_cr_frame *frame_ref, *frame_cur;

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
		cucon_list_append_init_node(&frame_cur->fv_list,
					   cu_to(cucon_listnode, frame_ref));
		frame_ref->fv_level = l_cur;
	    }
	}
	else if (cuex_og_binder_contains(e_meta)) {
	    struct sifi_cr_frame *frame_sub, *frame_cur;
	    int index_p1;

	    /* Create sub-frame (at l_cur + 1) and process sub-expressions. */
	    frame_cur = sp;
	    frame_sub = --sp;
	    cucon_list_init(&frame_sub->fv_list);
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
		cucon_list_append_init_node(&frame_cur->fv_list, node_ref);
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
    
    sp = cu_salloc(max_depth*sizeof(struct sifi_cr_frame));
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
