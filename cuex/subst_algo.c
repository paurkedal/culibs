/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2004--2007  Petter Urkedal <urkedal@nbi.dk>
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

#define CUEX_SUBST_ALGO_C
#include <cuex/subst.h>
#include <cuex/opn.h>
#include <cucon/pmap.h>

cuex_var_t
cuex_subst_insert_fragment_accum(cuex_subst_t dst, cucon_pmap_t ex_to_var,
				 cuex_t ex, cuex_subst_t src)
{
    cuex_meta_t meta;
    cuex_var_t *var_slot;
rec:
    meta = cuex_meta(ex);
    if (cuex_is_varmeta(meta)) {
	cuex_t ex0 = cuex_subst_lookup(src, cuex_var_from_ex(ex));
	if (ex0 && ex0 != ex) {
	    ex = ex0;
	    goto rec;
	}
	else
	    return cuex_var_from_ex(ex);
    }
    if (cuex_is_opn(ex))
	CUEX_OPN_TRAN(meta, ex, subex,
	    cuex_subst_insert_fragment_accum(dst, ex_to_var, subex, src));
    if (cucon_pmap_insert_mem(ex_to_var, ex, sizeof(cuex_var_t), &var_slot)) {
	cuex_var_t var = cuex_var_new(cuex_qcode_active_s);
	*var_slot = var;
	cuex_subst_unify(dst, cuex_var_to_ex(var), ex);
	return var;
    }
    else
	return *var_slot;
}

cuex_var_t
cuex_subst_insert_expand(cuex_subst_t dst, cuex_t ex, cuex_subst_t src)
{
    struct cucon_pmap_s ex_to_var;
    cucon_pmap_init(&ex_to_var);
    return cuex_subst_insert_fragment_accum(dst, &ex_to_var, ex, src);
}

#if 0
cu_clos_def(expand_cb,
	    cu_prot(void, void const *var, void *slot),
	    ( struct cucon_pmap_s ex_to_var;
	      cuex_subst_t src;
	      cuex_subst_t dst; ))
{
    cu_clos_self(expand_cb);
#define var ((cuex_var_t)var)
    cuex_t ex = cuex_subst_lookup(self->src, var);
    cu_debug_assert(ex);
    *(cuex_var_t *)slot
	= cuex_subst_insert_fragment_accum(self->dst, &self->ex_to_var, ex,
					  self->src);
#undef var
}

cuex_subst_t
cuex_subst_fragment_project(cuex_subst_t subst, cucon_pmap_t vi)
{
    expand_cb_t cb;
    cucon_pmap_init(&cb.ex_to_var);
    cb.dst = cuex_subst_new();
    cb.src = subst;
    cucon_pmap_iter_mem(vi, expand_cb_prep(&cb));
    return cb.dst;
}
#endif
