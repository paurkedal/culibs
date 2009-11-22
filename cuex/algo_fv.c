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

#include <cuex/algo.h>
#include <cuex/opn.h>
#include <cuex/oprdefs.h>
#include <cuex/compound.h>
#include <cuex/labelling.h>
#include <cuex/intf.h>
#include <cucon/pset.h>
#include <cucon/pmap.h>
#include <cu/ptr_seq.h>


/* Core Algorithms
 * =============== */

static cu_bool_t
_free_vars_conj(cuex_t e, cuex_qcset_t qcset, cucon_pset_t excl,
		cu_clop(f, cu_bool_t, cuex_var_t, cucon_pset_t))
{
    cuex_meta_t e_meta = cuex_meta(e);
    if (cuex_meta_is_opr(e_meta)) {
	if (cuex_og_scoping_contains(e_meta)) {
	    size_t i, r = cuex_opr_r(e_meta);
	    cuex_t var;
	    cu_debug_assert(cuex_opr_r(e_meta) > 0);
	    var = cuex_opn_at(e, 0);
	    if (cuex_is_labelling(var)) {
		cu_ptr_source_t src;
		cuex_t pair, *unshadowed_vars;
		size_t var_count;

		src = cuex_labelling_comm_iter_source(var);
		var_count = cu_ptr_source_count(src);
		unshadowed_vars = cu_salloc((var_count + 1)*sizeof(cuex_t));
		unshadowed_vars += var_count;
		*unshadowed_vars = NULL;
		src = cuex_labelling_comm_iter_source(var);
		while ((pair = cu_ptr_source_get(src))) {
		    cuex_t v = cuex_opn_at(pair, 0);
		    if (cucon_pset_insert(excl, v))
			*--unshadowed_vars = v;
		}
		src = cuex_labelling_comm_iter_source(var);
		while ((pair = cu_ptr_source_get(src))) {
		    cuex_t vp = cuex_opn_at(pair, 1);
		    if (!_free_vars_conj(vp, qcset, excl, f)) {
			i = 0;
			goto failed;
		    }
		}
		for (i = 1; i < r; ++i)
		    if (!_free_vars_conj(cuex_opn_at(e, i), qcset, excl, f))
			break;
failed:
		while (*unshadowed_vars)
		    cucon_pset_erase(excl, *unshadowed_vars++);
		return i == r;
	    }
	    else {
		cu_bool_t not_shadowed;
		not_shadowed = cucon_pset_insert(excl, var);
		for (i = 1; i < r; ++i)
		    if (!_free_vars_conj(cuex_opn_at(e, i), qcset, excl, f)) {
			if (not_shadowed)
			    cucon_pset_erase(excl, var);
			return cu_false;
		    }
		if (not_shadowed)
		    cucon_pset_erase(excl, var);
	    }
	}
	else
	    CUEX_OPN_CONJ_RETURN(e_meta, e, subex,
		    _free_vars_conj(subex, qcset, excl, f));
    }
    else if (cuex_meta_is_type(e_meta)) {
	cuoo_type_t type = cuoo_type_from_meta(e_meta);
	cuex_intf_compound_t c_impl;
	c_impl = cuoo_type_impl_ptr(type, CUEX_INTF_COMPOUND);
	if (c_impl) {
	    cu_ptr_source_t src;
	    cuex_t ep;
	    src = cuex_compound_pref_iter_source(c_impl, e);
	    while ((ep = cu_ptr_source_get(src)))
		if (!_free_vars_conj(ep, qcset, excl, f))
		    return cu_false;
	}
    }
    else if (cuex_is_varmeta(e_meta) &&
	     cuex_qcset_contains(qcset, cuex_varmeta_qcode(e_meta))) {
	if (!cucon_pset_find(excl, e))
	    return cu_call(f, e, excl);
    }
    return cu_true;
}

static cuex_t
_free_vars_tran(cuex_t e, cuex_qcset_t qcset, cucon_pset_t excl,
		cu_clop(f, cuex_t, cuex_t, cucon_pset_t))
{
    cuex_meta_t e_meta = cuex_meta(e);
    if (cuex_meta_is_opr(e_meta)) {
	if (cuex_og_scoping_contains(e_meta)) {
	    cu_rank_t i = 0, r = cuex_opr_r(e_meta);
	    cuex_t var, *e_arr;
	    cu_debug_assert(cuex_opr_r(e_meta) > 0);
	    var = cuex_opn_at(e, 0);
	    e_arr = cu_salloc(sizeof(cuex_t)*r);
	    if (cuex_is_labelling(var)) {
		cu_ptr_source_t src;
		cu_ptr_junctor_t jct;
		cuex_t ep, pair, *unshadowed_vars;
		size_t var_count;

		src = cuex_labelling_comm_iter_source(var);
		var_count = cu_ptr_source_count(src);
		unshadowed_vars = cu_salloc((var_count + 1)*sizeof(cuex_t));
		unshadowed_vars += var_count;
		*unshadowed_vars = NULL;
		src = cuex_labelling_comm_iter_source(var);
		while ((pair = cu_ptr_source_get(src))) {
		    cuex_t v = cuex_opn_at(pair, 0);
		    if (cucon_pset_insert(excl, v))
			*--unshadowed_vars = v;
		}
		jct = cuex_labelling_ncomm_image_junctor(var);
		while ((ep = cu_ptr_junctor_get(jct))) {
		    cuex_t epp = _free_vars_tran(ep, qcset, excl, f);
		    if (!epp) {
			goto failed;
		    }
		    cu_ptr_junctor_put(jct, epp);
		}
		var = cu_ptr_junctor_finish(jct);
		for (i = 1; i < r; ++i) {
		    e_arr[i] = _free_vars_tran(cuex_opn_at(e, i), qcset,
					       excl, f);
		    if (!e_arr[i])
			break;
		}
failed:
		while (*unshadowed_vars)
		    cucon_pset_erase(excl, *unshadowed_vars++);
	    }
	    else {
		cu_bool_t not_shadowed;
		not_shadowed = cucon_pset_insert(excl, var);
		for (i = 1; i < r; ++i) {
		    e_arr[i] = _free_vars_tran(cuex_opn_at(e, i),
					       qcset, excl, f);
			break;
		}
		if (not_shadowed)
		    cucon_pset_erase(excl, var);
	    }
	    if (i == r)
		return cuex_opn_by_arr(e_meta, e_arr);
	    else
		return NULL;
	}
	else {
	    CUEX_OPN_TRAN(e_meta, e, ep, _free_vars_tran(ep, qcset, excl, f));
	    return e;
	}
    }
    else if (cuex_meta_is_type(e_meta)) {
	cuoo_type_t type = cuoo_type_from_meta(e_meta);
	cuex_intf_compound_t c_impl;
	c_impl = cuoo_type_impl_ptr(type, CUEX_INTF_COMPOUND);
	if (c_impl) {
	    cu_ptr_junctor_t jct;
	    cuex_t ep;
	    jct = cuex_compound_pref_image_junctor(c_impl, e);
	    while ((ep = cu_ptr_junctor_get(jct))) {
		cuex_t epp = _free_vars_tran(ep, qcset, excl, f);
		if (!epp)
		    return NULL;
		cu_ptr_junctor_put(jct, epp);
	    }
	    return cu_ptr_junctor_finish(jct);
	}
	else
	    return e;
    }
    else if (cuex_is_varmeta(e_meta) &&
	     cuex_qcset_contains(qcset, cuex_varmeta_qcode(e_meta)) &&
	     !cucon_pset_find(excl, e))
	return cu_call(f, e, excl);
    else
	return e;
}


/* Derived Algorithms
 * ================== */

cu_bool_t
cuex_free_vars_conj(cuex_t e, cuex_qcset_t qcset, cucon_pset_t excl,
		    cu_clop(f, cu_bool_t, cuex_var_t, cucon_pset_t))
{
    if (!excl) {
	excl = cu_salloc(sizeof(struct cucon_pset));
	cucon_pset_init(excl);
    }
    return _free_vars_conj(e, qcset, excl, f);
}

cu_clos_efun(cuex_pset_curried_insert_ex,
	     cu_prot(cu_bool_t, cuex_t var))
{
    cu_clos_self(cuex_pset_curried_insert_ex);
    cucon_pset_insert(self->accu, var);
    return cu_true;
}

cu_clos_efun(cuex_pset_curried_erase_ex,
	     cu_prot(cu_bool_t, cuex_t var))
{
    cu_clos_self(cuex_pset_curried_erase_ex);
    cucon_pset_erase(self->accu, var);
    return cu_true;
}

void
cuex_free_vars_insert(cuex_t e, cuex_qcset_t qcset, cucon_pset_t excl,
		      cucon_pset_t accu)
{
    cuex_pset_curried_insert_ex_t cb;
    cb.accu = accu;
    cuex_free_vars_conj(e, qcset, excl,
			(cu_clop(, cu_bool_t, cuex_var_t, cucon_pset_t))
				cuex_pset_curried_insert_ex_prep(&cb));
}

void
cuex_free_vars_erase(cuex_t e, cuex_qcset_t qcset, cucon_pset_t excl,
		     cucon_pset_t accu)
{
    cuex_pset_curried_erase_ex_t cb;
    cb.accu = accu;
    cuex_free_vars_conj(e, qcset, excl,
			(cu_clop(, cu_bool_t, cuex_var_t, cucon_pset_t))
				cuex_pset_curried_erase_ex_prep(&cb));
}

cu_clos_def(_free_vars_count_cb,
	    cu_prot(cu_bool_t, cuex_var_t var, cucon_pset_t bound),
	( int cnt; ))
{
    cu_clos_self(_free_vars_count_cb);
    ++self->cnt;
    return cu_true;
}

int
cuex_free_vars_count(cuex_t e, cuex_qcset_t qcset, cucon_pset_t excl)
{
    _free_vars_count_cb_t cb;
    cb.cnt = 0;
    cuex_free_vars_conj(e, qcset, excl, _free_vars_count_cb_prep(&cb));
    return cb.cnt;
}


cuex_t
cuex_free_vars_tran(cuex_t e, cuex_qcset_t qcset, cucon_pset_t excl,
		    cu_clop(f, cuex_t, cuex_t, cucon_pset_t))
{
    struct cucon_pset excl_l;
    if (!excl) {
	excl = &excl_l;
	cucon_pset_init(&excl_l);
    }
    return _free_vars_tran(e, qcset, excl, f);
}

cu_clos_def(_free_vars_tran_pmap_cb,
	    cu_prot(cuex_t, cuex_t var, cucon_pset_t bound_vars),
    ( cucon_pmap_t subst; ))
{
    cu_clos_self(_free_vars_tran_pmap_cb);
    cuex_t repl = cucon_pmap_find_ptr(self->subst, var);
    if (repl)
	return repl;
    else
	return var;
}

cuex_t
cuex_free_vars_tran_pmap(cuex_t e, cuex_qcset_t qcset, cucon_pset_t excl,
			 cucon_pmap_t subst)
{
    _free_vars_tran_pmap_cb_t cb;
    cb.subst = subst;
    return cuex_free_vars_tran(e, cuex_qcset_n, NULL,
			       _free_vars_tran_pmap_cb_prep(&cb));
}
