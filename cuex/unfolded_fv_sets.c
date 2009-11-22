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

#include <cuex/binding.h>
#include <cuex/oprdefs.h>
#include <cuex/opn.h>
#include <cuex/monoid.h>
#include <cuex/compound.h>
#include <cuex/intf.h>
#include <cucon/pmap.h>
#include <cucon/uset.h>
#include <cucon/slink.h>
#include <cu/ptr_seq.h>

cu_dlog_def(_file, "dtag=cuex.binding_info");

/* Flat binding info
 * ----------------- */

typedef struct FBI_s *FBI_t;
typedef struct FBI_link_s *FBI_link_t;

struct FBI_s
{
    struct cucon_uset fvset;
    FBI_link_t sub_chain;
    cuex_t e;
};

struct FBI_link_s
{
    FBI_link_t next;
    FBI_t fbi;
    int depth;	/* the depth of fbi relative to context μ bind */
};

cu_clos_def(compute_FBI_propagate, cu_prot(void, uintptr_t index),
    ( cucon_uset_t target_fvset;
      int target_index; ))
{
    cu_clos_self(compute_FBI_propagate);
    if (index >= self->target_index)
	cucon_uset_insert(self->target_fvset, index - self->target_index);
}

static void
compute_FBI(cuex_t e, int depth, FBI_t fbi, cucon_pmap_t e_to_fbi)
{
    cuex_meta_t e_meta = cuex_meta(e);
    if (cuex_meta_is_opr(e_meta)) {
	if (cuex_og_hole_contains(e_meta)) {
	    int index = cuex_oa_hole_index(e_meta);
	    if (index >= depth)  /* if points to μ-bind or outside */
		cucon_uset_insert(&fbi->fvset, index - depth);
	    return;
	} else if (cuex_og_binder_contains(e_meta)) {
	    ++depth;
	    if (e_meta == CUEX_O1_MU) {
		int ist;
		FBI_t sub_fbi;
		FBI_link_t link;
		compute_FBI_propagate_t prop_cb;

		ist = cucon_pmap_insert_mem(e_to_fbi, e,
					    sizeof(struct FBI_s), &sub_fbi);
		link = cu_gnew(struct FBI_link_s);
		link->next = fbi->sub_chain;
		link->fbi = sub_fbi;
		link->depth = depth;
		fbi->sub_chain = link;
		if (ist) {
		    cucon_uset_init(&sub_fbi->fvset);
		    sub_fbi->sub_chain = NULL;
		    sub_fbi->e = e;
		    compute_FBI(cuex_opn_at(e, 0), 0, sub_fbi, e_to_fbi);
		}
		prop_cb.target_fvset = &fbi->fvset;
		prop_cb.target_index = depth;
		cucon_uset_iter(&sub_fbi->fvset,
				compute_FBI_propagate_prep(&prop_cb));
		return;
	    }
	}
	CUEX_OPN_ITER(e_meta, e, ep, compute_FBI(ep, depth, fbi, e_to_fbi));
    } else if (cuex_meta_is_type(e_meta)) {
	cuoo_type_t e_type = cuoo_type_from_meta(e_meta);
	cuex_intf_compound_t e_c;
	e_c = cuoo_type_impl_ptr(e_type, CUEX_INTF_COMPOUND);
	if (e_c) {
	    cu_ptr_source_t src = cuex_compound_pref_iter_source(e_c, e);
	    cuex_t ep;
	    while ((ep = cu_ptr_source_get(src)))
		compute_FBI(ep, depth, fbi, e_to_fbi);
	}
    }
}

/* Unfolded binding info
 * --------------------- */

cu_clos_def(uset_shifted_union_cb, cu_prot(void, uintptr_t key),
    ( cucon_uset_t target;
      int shift; ))
{
    cu_clos_self(uset_shifted_union_cb);
    cu_dlogf(_file, "Adding λ-variable %d due to μ %d.",
	     key + self->shift, self->shift);
    cucon_uset_insert(self->target, key + self->shift);
}

cu_clos_def(compute_UBI_mvar, cu_prot(void, uintptr_t key),
    ( cucon_uset_t target;
      cucon_uset_t *sp;
      cucon_uset_t *sp_max; ))
{
    cu_clos_self(compute_UBI_mvar);
    uset_shifted_union_cb_t lvar_cb;
    if (key == 0)
	return;  /* Ignore the μ-variabe of the binding itself. */
    if (self->sp + key < self->sp_max && self->sp[key]) {
	lvar_cb.target = self->target;
	lvar_cb.shift = key;
	cu_dlogf(_file, "Merging μ-variable %d.", key);
	cucon_uset_iter(self->sp[key], uset_shifted_union_cb_prep(&lvar_cb));
    } else {
	cu_debug_assert(key != 0);
	cu_dlogf(_file, "Adding λ-variable %d.", key);
	cucon_uset_insert(self->target, key);
    }
}

static void
compute_UBI(FBI_link_t fbi_link, cuex_t super_fiictx,
	    cucon_uset_t *super_sp,
	    cucon_uset_t *sp_min, cucon_uset_t *sp_max, cucon_pmap_t fiimap)
{
    while (fbi_link != NULL) {
	FBI_t fbi;
	compute_UBI_mvar_t mvar_cb;
	cucon_uset_t lambda_vars;
	cuex_t fiictx;

	fbi = fbi_link->fbi;
	fiictx = cuex_mupath_pair(fbi_link->depth, super_fiictx, fbi->e);
	cu_dlogf(_file, "fiimap=%p, Processing BI for %p, %!.",
		 fiimap, fiictx, fiictx);
	if (cucon_pmap_insert_mem(fiimap, fiictx,
				  sizeof(struct cucon_uset), &lambda_vars)) {
	    cucon_uset_t *sp = super_sp - fbi_link->depth;
	    memset(sp + 1, 0, (fbi_link->depth - 1)*sizeof(*sp));
	    cu_dlogf(_file, "Enter frame %d", sp_max - sp);
	    cu_debug_assert(fbi_link->depth > 0);
	    cu_debug_assert(sp >= sp_min);
	    cucon_uset_init(lambda_vars);
	    sp[0] = lambda_vars;
	    mvar_cb.target = lambda_vars;
	    mvar_cb.sp = sp;
	    mvar_cb.sp_max = sp_max;
	    cucon_uset_iter(&fbi->fvset, compute_UBI_mvar_prep(&mvar_cb));
	    /* We now know the complete set of λ variables on this frame, so we
	     * can process subframes. */
	    compute_UBI(fbi->sub_chain, fiictx, sp, sp_min, sp_max, fiimap);
	    cu_dlogf(_file, "Leave frame %d", sp_max - sp);
	}
	fbi_link = fbi_link->next;
    }
}

cucon_pmap_t
cuex_unfolded_fv_sets(cuex_t e, int max_binding_depth)
{
    struct FBI_s top_fbi;
    struct cucon_pmap e_to_fbi;
    cucon_pmap_t fiimap;
    int max_depth;
    cucon_uset_t *usp_min, *usp_max;

    /* Compute flat binding info. */
    cucon_uset_init(&top_fbi.fvset);
    cucon_pmap_init(&e_to_fbi);
    top_fbi.sub_chain = NULL;
    top_fbi.e = NULL;
    compute_FBI(e, 0, &top_fbi, &e_to_fbi);

    /* Expand μ-refs. */
    max_depth = max_binding_depth >= 0? max_binding_depth :
	cuex_max_binding_depth(e);
    usp_min = cu_salloc(sizeof(cucon_uset_t)*max_depth);
    usp_max = usp_min + max_depth;
    memset(usp_min, 0, sizeof(cucon_uset_t)*max_depth);
    fiimap = cucon_pmap_new();
    compute_UBI(top_fbi.sub_chain, cuex_mupath_null(),
		usp_max, usp_min, usp_max, fiimap);

    return fiimap;
}
