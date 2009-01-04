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

#include <cuex/occurtree.h>
#include <cuex/opn.h>
#include <cuex/binding.h>
#include <cuex/compound.h>
#include <cuex/oprinfo.h>
#include <cucon/ucset.h>
#include <cu/clos.h>
#include <inttypes.h>

cuex_occurtree_t
cuex_folded_occurtree(cuex_t e, cu_bool_t force_comm)
{
    cuex_occurtree_t ot, sub_ot;
    cuex_meta_t e_meta = cuex_meta(e);

    if (cuex_meta_is_opr(e_meta)) {
	int i, r = cuex_opr_r(e_meta);
	ot = cu_galloc(sizeof(struct cuex_occurtree_s)
		       + (r - 1)*sizeof(cuex_occurtree_t));
	ot->e = e;
	if (cuex_og_hole_contains(e_meta)) {
	    cu_debug_assert(r == 0);
	    ot->free_vars = cucon_ucset_singleton(cuex_oa_hole_index(e_meta));
	}
	else {
	    cucon_ucset_t free_vars = cucon_ucset_empty();
	    for (i = 0; i < r; ++i) {
		sub_ot = cuex_folded_occurtree(cuex_opn_at(e, i), force_comm);
		ot->sub[i] = sub_ot;
		free_vars = cucon_ucset_union(free_vars, sub_ot->free_vars);
	    }
	    if (cuex_og_binder_contains(e_meta))
		free_vars = cucon_ucset_translate_uclip(free_vars, -1,
							1, UINTPTR_MAX);
	    ot->free_vars = free_vars;
	}
	return ot;
    }
    else if (cuex_meta_is_type(e_meta)) {
	cuoo_type_t e_type = cuoo_type_from_meta(e_meta);
	cuex_intf_compound_t ci = cuex_type_compound(e_type);
	if (ci) {
	    cucon_ucset_t free_vars = cucon_ucset_empty();
	    int i = 0, r = cuex_compound_size(ci, e);
	    cu_ptr_source_t ps;
	    cuex_t ep;
	    ot = cu_galloc(sizeof(struct cuex_occurtree_s)
			   + (r - 1)*sizeof(cuex_occurtree_t));
	    ot->e = e;
	    if (force_comm)
		ps = cuex_compound_comm_iter_source(ci, e);
	    else
		ps = cuex_compound_pref_iter_source(ci, e);
	    while ((ep = cu_ptr_source_get(ps))) {
		sub_ot = cuex_folded_occurtree(ep, force_comm);
		ot->sub[i++] = sub_ot;
		free_vars = cucon_ucset_union(free_vars, sub_ot->free_vars);
	    }
	    ot->free_vars = free_vars;
	    return ot;
	}
	/* else fall though */
    }
    ot = cu_galloc(sizeof(struct cuex_occurtree_s) - sizeof(cuex_occurtree_t));
    ot->e = e;
    ot->free_vars = cucon_ucset_empty();
    return ot;
}

typedef struct _unfolded_occurtree_frame_s *_unfolded_occurtree_frame_t;
struct _unfolded_occurtree_frame_s
{
    cuex_occurtree_t ot;
};

cu_clos_def(_expand_mu_vars, cu_prot(void, uintptr_t l),
    ( _unfolded_occurtree_frame_t sp, sp_bottom;
      cucon_ucset_t unfolded; ))
{
    cu_clos_self(_expand_mu_vars);
    _unfolded_occurtree_frame_t sp_ref = self->sp + l;
    if (sp_ref < self->sp_bottom && cuex_meta(sp_ref->ot->e) == CUEX_O1_MU) {
	cucon_ucset_t vars;
	vars = cucon_ucset_translate(sp_ref->ot->free_vars, l + 1);
	self->unfolded = cucon_ucset_union(self->unfolded, vars);
    }
}

static void
_unfold_occurtree(cuex_occurtree_t ot, cu_bool_t force_comm,
		  _unfolded_occurtree_frame_t sp,
		  _unfolded_occurtree_frame_t sp_bottom)
{
    cuex_occurtree_t sub_ot;
    cuex_t e = ot->e;
    cuex_meta_t e_meta = cuex_meta(e);
    if (cuex_meta_is_opr(e_meta)) {
	if (e_meta == CUEX_O1_MU) {
	    _expand_mu_vars_t cb;
	    cb.sp = sp;
	    cb.sp_bottom = sp_bottom;
	    cb.unfolded = ot->free_vars;
	    cucon_ucset_iter(ot->free_vars, _expand_mu_vars_prep(&cb));
	    ot->free_vars = cb.unfolded;
	    --sp;
	    sp->ot = ot;
	    _unfold_occurtree(ot->sub[0], force_comm, sp, sp_bottom);
	}
	else if (cuex_og_hole_contains(e_meta)) {
	    int l = cuex_oa_hole_index(e_meta);
	    _unfolded_occurtree_frame_t sp_ref = sp + l;
	    if (sp_ref < sp_bottom && cuex_meta(sp_ref->ot->e) == CUEX_O1_MU) {
		cucon_ucset_t free_vars;
		free_vars = cucon_ucset_translate(sp_ref->ot->free_vars, l + 1);
		ot->free_vars = cucon_ucset_insert(free_vars, l);
	    }
	    else
		ot->free_vars = cucon_ucset_singleton(l);
	}
	else {
	    int i, r = cuex_opr_r(e_meta);
	    cucon_ucset_t free_vars = cucon_ucset_empty();
	    if (cuex_og_binder_contains(e_meta)) {
		--sp;
		sp->ot = ot;
	    }
	    for (i = 0; i < r; ++i) {
		sub_ot = ot->sub[i];
		_unfold_occurtree(sub_ot, force_comm, sp, sp_bottom);
		free_vars = cucon_ucset_union(free_vars, sub_ot->free_vars);
	    }
	    if (cuex_og_binder_contains(e_meta))
		free_vars = cucon_ucset_translate_uclip(free_vars, -1,
							1, UINTPTR_MAX);
	    ot->free_vars = free_vars;
	}
    }
    else if (cuex_meta_is_type(e_meta)) {
	cuoo_type_t e_type = cuoo_type_from_meta(e_meta);
	cuex_intf_compound_t ci = cuex_type_compound(e_type);
	if (ci) {
	    cucon_ucset_t free_vars = cucon_ucset_empty();
	    int i = 0;
	    cu_ptr_source_t ps;
	    cuex_t ep;
	    if (force_comm)
		ps = cuex_compound_comm_iter_source(ci, e);
	    else
		ps = cuex_compound_pref_iter_source(ci, e);
	    while ((ep = cu_ptr_source_get(ps))) {
		sub_ot = ot->sub[i++];
		_unfold_occurtree(sub_ot, force_comm, sp, sp_bottom);
		free_vars = cucon_ucset_union(free_vars, sub_ot->free_vars);
	    }
	    ot->free_vars = free_vars;
	}
    }
}

cuex_occurtree_t
cuex_unfolded_occurtree(cuex_t e, cu_bool_t force_comm)
{
    cuex_occurtree_t ot = cuex_folded_occurtree(e, force_comm);
    _unfolded_occurtree_frame_t sp;
    size_t sp_size = cuex_max_binding_depth(e) + 1;
    sp = cu_salloc(sizeof(struct _unfolded_occurtree_frame_s)*sp_size);
    sp += sp_size;
    _unfold_occurtree(ot, force_comm, sp, sp);
    return ot;
}

static void
_occurtree_dump(cuex_occurtree_t ot, FILE *out, int l)
{
    cuex_t e;
    cuex_meta_t e_meta;
    int i, r;
    for (i = 0; i < l; ++i)
	fputs("  ", out);
    if (ot == NULL) {
	fputs("NULL\n", out);
	return;
    }
    e = ot->e;
    e_meta = cuex_meta(ot->e);
    if (cuex_meta_is_opr(e_meta)) {
	if (cuex_og_hole_contains(e_meta)) {
	    r = 0;
	    fprintf(out, "_%d ", cuex_oa_hole_index(e_meta));
	}
	else {
	    cuex_oprinfo_t oi = cuex_oprinfo(e_meta);
	    r = cuex_opr_r(e_meta);
	    if (oi)
		fprintf(out, "%s ", oi->name);
	    else
		fprintf(out, "__O%d_%#"CUEX_PRIxMETA" ", r, e_meta);
	}
    }
    else if (cuex_meta_is_type(e_meta)) {
	cuoo_type_t e_type = cuoo_type_from_meta(e_meta);
	cuex_intf_compound_t ci = cuex_type_compound(e_type);
	if (ci) {
	    r = cuex_compound_size(ci, e);
	    cu_fprintf(out, "%! ", e_type);
	}
	else {
	    r = 0;
	    cu_fprintf(out, "%! ", e);
	}
    }
    else {
	r = 0;
	cu_fprintf(out, "%! ", e);
    }
    cucon_ucset_fprint_uintptr(ot->free_vars, out);
    fputc('\n', out);
    for (i = 0; i < r; ++i)
	_occurtree_dump(ot->sub[i], out, l + 1);
}

void
cuex_occurtree_dump(cuex_occurtree_t ot, FILE *out)
{
    fprintf(out, "cuex_occurtree @ %p\n", out);
    _occurtree_dump(ot, out, 1);
}
