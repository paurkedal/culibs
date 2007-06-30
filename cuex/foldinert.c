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
#include <cuex/monoid.h>
#include <cuex/var.h>
#include <cucon/uset.h>
#include <cucon/pmap.h>


struct fii_frame_s
{
    cucon_uset_t fv_set;  /* available only in μ-bind frames */
    int min_fii;
};

cu_clos_def(merge_fvset, cu_prot(void, uintptr_t index),
    ( cucon_uset_t target;
      int mu_body_level;
#ifndef CUEX_NDEBUG
      struct fii_frame_s *sp_max;
#endif
    ))
{
    cu_clos_self(merge_fvset);
    int ref_level;
    /* index is the de Bruijn index relative to the μ body.  We want the
     * absolute level from the top of the expression.  If index points outside
     * the full expression, the we'll insert a negative value. */
    ref_level = self->mu_body_level - index;
    cu_debug_assert(index > 0);
    /* Check that ref_level refers to a λ-variable. */
#ifndef CUEX_NDEBUG
    cu_debug_assert(ref_level <= 0 || self->sp_max[-ref_level].fv_set == NULL);
#endif
    cu_dprintf("cuex.foldinert",
	       "Inserting λ ref to level %d due to μ-var at %d\n",
	       ref_level, self->mu_body_level);
    cucon_uset_insert(self->target, ref_level);
}

cu_clos_def(adjust_fii, cu_prot(void, uintptr_t level_ref),
    ( struct fii_frame_s *sp;
      struct fii_frame_s *sp_max;
      int lambda_body_level;
      cucon_uset_t super_fv;
      int min_fii; ))
{
    cu_clos_self(adjust_fii);
    int index = self->lambda_body_level - level_ref;
    if (index == 0)
	return;  /* the current λ-abstraction */
    cu_debug_assert(index > 0);
    cucon_uset_insert(self->super_fv, level_ref);
    cu_debug_assert(self->sp + index < self->sp_max);
    cu_debug_assert(self->sp[index].fv_set == NULL);
    if (self->sp[index].min_fii < self->min_fii) {
	cu_dprintf("cuex.foldinert",
		   "Adjusting min_fii: sp=%p, lambda_body_level=%d, "
		   "level_ref=%d, index=%d, min_fii=%d",
		   self->sp, self->lambda_body_level, level_ref, index,
		   self->min_fii);
	self->sp[index].min_fii = self->min_fii;
    }
}

static void
foldinert_indices(cuex_t e, cuex_t mu_context, cucon_pmap_t mu_fv_sets,
		  int level, int last_mu_level, cucon_uset_t fv,
		  struct fii_frame_s *sp,
		  struct fii_frame_s *sp_min, struct fii_frame_s *sp_max,
		  cucon_pmap_t fiimap)
{
    cuex_meta_t em = cuex_meta(e);
    cu_debug_assert(sp_max - sp == level); /* TODO: simplify */
    if (cuex_meta_is_opr(em)) {
	if (cuex_og_hole_contains(em)) {
	    /* Update the running free variable set either by inserting a λ
	     * variable or all λ variables which are free in a μ variable. */
	    int ref_index = cuex_oa_hole_index(em);
	    if (ref_index < level) {
		cu_debug_assert(sp + ref_index < sp_max);
		if (sp[ref_index].fv_set) {
		    /* Found a μ-variable. Insert it's free λ-variables into
		     * the current FV set. */
		    merge_fvset_t cb;
		    cb.target = fv;
		    cb.mu_body_level = level - ref_index;
#ifndef CUEX_NDEBUG
		    cb.sp_max = sp_max;
#endif
		    cucon_uset_iter(sp[ref_index].fv_set,
				    merge_fvset_prep(&cb));
		} else {
		    /* Found a λ-variable. Insert it into the current FV set. */
		    cu_dprintf("cuex.foldinert",
			       "At %d, inserting λ-ref to level %d "
			       "(ref_index = %d)",
			       level, level - ref_index, ref_index);
		    cucon_uset_insert(fv, level - ref_index);
		}
	    }
	} else if (cuex_og_binder_contains(em)) {
	    cu_debug_assert(sp > sp_min);
	    --sp;
	    ++level;
	    sp[0].min_fii = 0;
	    if (em == CUEX_O1_MU) {
		/* At a μ-bind, store the known FV set on the stack, and
		 * proceed with the subexpression. */
		mu_context = cuex_mupath_pair(level - last_mu_level,
					      mu_context, e);
		sp[0].fv_set = cucon_pmap_find_mem(mu_fv_sets, mu_context);
		cu_dprintf("cuex.foldinert", "At level=%d set fv_set = %&.",
			   sp_max - sp, cucon_uset_print, sp[0].fv_set);
		cu_debug_assert(sp[0].fv_set);
		foldinert_indices(cuex_opn_at(e, 0), mu_context, mu_fv_sets,
				  level, level,
				  fv, sp, sp_min, sp_max, fiimap);
	    } else {
		struct cucon_uset_s sub_fv;
		adjust_fii_t cb;
		cu_dprintf("cuex.foldinert", "At level=%d clear fv_set.",
			   sp_max - sp);
		sp[0].fv_set = NULL; /* Indicate that this is a λ-frame. */

		/* Collect free variables in body of λ, and process body. */
		cucon_uset_cct(&sub_fv);
		foldinert_indices(cuex_opn_at(e, 0), mu_context, mu_fv_sets,
				  level, last_mu_level,
				  &sub_fv, sp, sp_min, sp_max, fiimap);

		/* For each variable which occurs in the body, adjust it's
		 * minimum index to one above the current index. */
		cb.sp = sp;
		cb.sp_max = sp_max;
		cb.min_fii = sp[0].min_fii + 1;
		cb.lambda_body_level = level;
		cb.super_fv = fv;
		cucon_uset_iter(&sub_fv, adjust_fii_prep(&cb));

		/* Save the current FII into result. */
		mu_context = cuex_mupath_pair(level - last_mu_level,
					      mu_context, e);
		cu_dprintf("cuex.foldinert", "Fixing index=%d for %!",
			   sp[0].min_fii, mu_context);
		cucon_pmap_insert_int(fiimap, mu_context, sp[0].min_fii);
	    }
	} else
	    CUEX_OPN_ITER(em, e, ep,
			  foldinert_indices(ep, mu_context, mu_fv_sets,
					    level, last_mu_level,
					    fv, sp, sp_min, sp_max, fiimap));
    }
}

cucon_pmap_t
cuex_foldinert_indices(cuex_t e, int max_binding_depth)
{
    int depth = max_binding_depth >= 0? max_binding_depth :
	cuex_max_binding_depth(e);
    cucon_pmap_t fv_sets;
    struct cucon_uset_s cur_fv;
    struct fii_frame_s *sp;
    cucon_pmap_t fiimap;

    cu_dprintf("cuex.foldinert", "====");
    cu_dprintf("cuex.foldinert", "cuex_foldinert_indices: e = %!", e);
    fv_sets = cuex_unfolded_fv_sets(e, depth);
    cucon_uset_cct(&cur_fv);
    sp = cu_salloc(sizeof(struct fii_frame_s)*depth);
    sp += depth;
    fiimap = cucon_pmap_new();
    foldinert_indices(e, cuex_mupath_null(), fv_sets, 0, 0, &cur_fv, sp,
		      sp - depth, sp, fiimap);
    return fiimap;
}

static cuex_t
foldinert_rebind(cuex_t e, cuex_t mu_context,
		 int *sp, int level, int mu_level, int last_mu_level,
		 cucon_pmap_t indices)
{
    cuex_meta_t em = cuex_meta(e);
    if (cuex_meta_is_opr(em)) {
	if (cuex_og_hole_contains(em)) {
	    int index = cuex_oa_hole_index(em);
	    if (index <= level && sp[index] >= 0)
		return cuex_ivar_n(sp[index]);
	    else
		return cuex_hole(mu_level + sp[index]);
	} else if (cuex_og_binder_contains(em)) {
	    cuex_t ep;
	    --sp;
	    ++level;
	    if (em == CUEX_O1_MU) {
		++mu_level;
		sp[0] = -mu_level;
		mu_context = cuex_mupath_pair(level - last_mu_level,
					      mu_context, e);
		ep = foldinert_rebind(cuex_opn_at(e, 0), mu_context,
				      sp, level, mu_level, level, indices);
		return cuex_o1_mu(ep);
	    } else if (em == CUEX_O1_LAMBDA) {
		sp[0] = cucon_pmap_find_int(
		    indices,
		    cuex_mupath_pair(level - last_mu_level, mu_context, e));
		cu_debug_assert(sp[0] >= 0);
		ep = foldinert_rebind(cuex_opn_at(e, 0), mu_context, sp,
				      level, mu_level, last_mu_level, indices);
		return cuex_o2_lambda(cuex_ivar_n(sp[0]), ep);
	    } else {
		cu_bugf("Can not handle bind operator of %!", e);
		return e;
	    }
	} else {
	    CUEX_OPN_TRAN(em, e, ep,
			  foldinert_rebind(ep, mu_context, sp,
					   level, mu_level, last_mu_level,
					   indices));
	    return e;
	}
    } else
	return e;
}

cuex_t
cuex_foldinert_rebind(cuex_t e, int max_binding_depth)
{
    int depth = max_binding_depth >= 0? max_binding_depth :
	cuex_max_binding_depth(e);
    cucon_pmap_t indices = cuex_foldinert_indices(e, depth);
    int *sp = cu_salloc(sizeof(int)*depth);
    sp += depth;
    return foldinert_rebind(e, cuex_mupath_null(), sp, -1, 0, 0, indices);
}
