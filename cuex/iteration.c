/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2009  Petter Urkedal <urkedal@nbi.dk>
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

#include <cuex/iteration.h>
#include <cuex/opn.h>
#include <cuex/compound.h>
#include <cu/ptr_seq.h>

cu_bool_t
cuex_bareA_operands(cu_bool_t (*f)(cuex_t), cuex_t e)
{
    cuex_meta_t e_meta = cuex_meta(e);
    if (cuex_meta_is_opr(e_meta))
	CUEX_OPN_CONJ_RETURN(e_meta, e, ep, (*f)(ep));
    else if (cuex_meta_is_type(e_meta)) {
	cuoo_type_t e_type = cuoo_type_from_meta(e_meta);
	cuex_intf_compound_t impl;
	impl = cuoo_type_impl_ptr(e_type, CUEX_INTF_COMPOUND);
	if (impl) {
	    cu_ptr_source_t src = cuex_compound_pref_iter_source(impl, e);
	    return cu_ptr_source_bareA(f, src);
	}
    }
    return cu_true;
}

cu_bool_t
cuex_bareE_operand(cu_bool_t (*f)(cuex_t), cuex_t e)
{
    cuex_meta_t e_meta = cuex_meta(e);
    if (cuex_meta_is_opr(e_meta))
	CUEX_OPN_DISJ_RETURN(e_meta, e, ep, (*f)(ep));
    else if (cuex_meta_is_type(e_meta)) {
	cuoo_type_t e_type = cuoo_type_from_meta(e_meta);
	cuex_intf_compound_t impl;
	impl = cuoo_type_impl_ptr(e_type, CUEX_INTF_COMPOUND);
	if (impl) {
	    cu_ptr_source_t src = cuex_compound_pref_iter_source(impl, e);
	    return cu_ptr_source_bareE(f, src);
	}
    }
    return cu_false;
}

void
cuex_iter_operands(cu_clop(f, void, cuex_t), cuex_t e)
{
    cuex_meta_t e_meta = cuex_meta(e);
    if (cuex_meta_is_opr(e_meta))
	CUEX_OPN_ITER(e_meta, e, ep, cu_call(f, ep));
    else if (cuex_meta_is_type(e_meta)) {
	cuoo_type_t e_type = cuoo_type_from_meta(e_meta);
	cuex_intf_compound_t impl;
	impl = cuoo_type_impl_ptr(e_type, CUEX_INTF_COMPOUND);
	if (impl) {
	    cu_ptr_source_t src = cuex_compound_pref_iter_source(impl, e);
	    cu_ptr_source_iter(f, src);
	}
    }
}

cu_bool_t
cuex_iterA_operands(cu_clop(f, cu_bool_t, cuex_t), cuex_t e)
{
    cuex_meta_t e_meta = cuex_meta(e);
    if (cuex_meta_is_opr(e_meta))
	CUEX_OPN_CONJ_RETURN(e_meta, e, ep, cu_call(f, ep));
    else if (cuex_meta_is_type(e_meta)) {
	cuoo_type_t e_type = cuoo_type_from_meta(e_meta);
	cuex_intf_compound_t impl;
	impl = cuoo_type_impl_ptr(e_type, CUEX_INTF_COMPOUND);
	if (impl) {
	    cu_ptr_source_t src = cuex_compound_pref_iter_source(impl, e);
	    return cu_ptr_source_iterA(f, src);
	}
    }
    return cu_true;
}

cu_bool_t
cuex_iterA_operands_view(cuex_opview_t view,
			 cu_clop(f, cu_bool_t, cuex_t), cuex_t e)
{
    cuex_meta_t e_meta = cuex_meta(e);
    if (cuex_meta_is_opr(e_meta)) {
	if (view == CUEX_FCOMM_VIEW)
	    return cuex_opn_comm_iterA(f, e);
	else
	    CUEX_OPN_CONJ_RETURN(e_meta, e, ep, cu_call(f, ep));
    }
    else if (cuex_meta_is_type(e_meta)) {
	cuoo_type_t e_type = cuoo_type_from_meta(e_meta);
	cuex_intf_compound_t impl;
	impl = cuoo_type_impl_ptr(e_type, CUEX_INTF_COMPOUND);
	if (impl) {
	    cu_ptr_source_t src;
	    if (view == CUEX_PREF_VIEW)
		src = cuex_compound_pref_iter_source(impl, e);
	    else
		src = cuex_compound_comm_iter_source(impl, e);
	    return cu_ptr_source_iterA(f, src);
	}
    }
    return cu_true;
}

cu_bool_t
cuex_iterAk_operands(cu_clop(f, cu_bool_t, int, cuex_t), cuex_t e)
{
    cuex_meta_t e_meta = cuex_meta(e);
    if (cuex_meta_is_opr(e_meta)) {
	int k = 0;
	CUEX_OPN_CONJ_RETURN(e_meta, e, ep, cu_call(f, k++, ep));
    }
    else if (cuex_meta_is_type(e_meta)) {
	cuoo_type_t e_type = cuoo_type_from_meta(e_meta);
	cuex_intf_compound_t impl;
	impl = cuoo_type_impl_ptr(e_type, CUEX_INTF_COMPOUND);
	if (impl) {
	    cu_ptr_source_t src = cuex_compound_pref_iter_source(impl, e);
	    cuex_t ep;
	    int k = 0;
	    while ((ep = cu_ptr_source_get(src)))
		if (!cu_call(f, k++, ep))
		    return cu_false;
	}
    }
    return cu_true;
}

cu_bool_t
cuex_iterAk_operands_view(cuex_opview_t view,
			  cu_clop(f, cu_bool_t, int, cuex_t), cuex_t e)
{
    cuex_meta_t e_meta = cuex_meta(e);
    if (cuex_meta_is_opr(e_meta)) {
	if (view == CUEX_FCOMM_VIEW)
	    return cuex_opn_comm_iterAk(f, e);
	else {
	    int k = 0;
	    CUEX_OPN_CONJ_RETURN(e_meta, e, ep, cu_call(f, k++, ep));
	}
    }
    else if (cuex_meta_is_type(e_meta)) {
	cuoo_type_t e_type = cuoo_type_from_meta(e_meta);
	cuex_intf_compound_t impl;
	impl = cuoo_type_impl_ptr(e_type, CUEX_INTF_COMPOUND);
	if (impl) {
	    cu_ptr_source_t src;
	    cuex_t ep;
	    int k = 0;
	    if (view == CUEX_PREF_VIEW)
		src = cuex_compound_pref_iter_source(impl, e);
	    else
		src = cuex_compound_comm_iter_source(impl, e);
	    while ((ep = cu_ptr_source_get(src)))
		if (!cu_call(f, k++, ep))
		    return cu_false;
	}
    }
    return cu_true;
}

cu_bool_t
cuex_iterE_operand(cu_clop(f, cu_bool_t, cuex_t), cuex_t e)
{
    cuex_meta_t e_meta = cuex_meta(e);
    if (cuex_meta_is_opr(e_meta))
	CUEX_OPN_DISJ_RETURN(e_meta, e, ep, cu_call(f, ep));
    else if (cuex_meta_is_type(e_meta)) {
	cuoo_type_t e_type = cuoo_type_from_meta(e_meta);
	cuex_intf_compound_t impl;
	impl = cuoo_type_impl_ptr(e_type, CUEX_INTF_COMPOUND);
	if (impl) {
	    cu_ptr_source_t src = cuex_compound_pref_iter_source(impl, e);
	    return cu_ptr_source_iterE(f, src);
	}
    }
    return cu_false;
}

cuex_t
cuex_bareimg_operands(cuex_t (*f)(cuex_t), cuex_t e)
{
    cuex_meta_t e_meta = cuex_meta(e);
    if (cuex_meta_is_opr(e_meta)) {
	CUEX_OPN_TRAN(e_meta, e, ep, (*f)(ep));
	return e;
    }
    else if (cuex_meta_is_type(e_meta)) {
	cuoo_type_t type = cuoo_type_from_meta(e_meta);
	cuex_intf_compound_t impl;
	impl = cuoo_type_impl_ptr(type, CUEX_INTF_COMPOUND);
	if (impl) {
	    cu_ptr_junctor_t junctor;
	    junctor = cuex_compound_pref_image_junctor(impl, e);
	    return cu_ptr_junctor_image_cfn(f, junctor);
	}
	else
	    return e;
    }
    else
	return e;
}

cuex_t
cuex_iterimg_operands(cu_clop(f, cuex_t, cuex_t), cuex_t e)
{
    cuex_meta_t e_meta = cuex_meta(e);
    if (cuex_meta_is_opr(e_meta)) {
	CUEX_OPN_TRAN(e_meta, e, ep, cu_call(f, ep));
	return e;
    }
    else if (cuex_meta_is_type(e_meta)) {
	cuoo_type_t e_type = cuoo_type_from_meta(e_meta);
	cuex_intf_compound_t impl;
	impl = cuoo_type_impl_ptr(e_type, CUEX_INTF_COMPOUND);
	if (impl) {
	    cu_ptr_junctor_t junctor;
	    junctor = cuex_compound_pref_image_junctor(impl, e);
	    return cu_ptr_junctor_image(f, junctor);
	}
	else
	    return e;
    }
    else
	return e;
}

cuex_t
cuex_iterimg_operands_view(cuex_opview_t view,
			   cu_clop(f, cuex_t, cuex_t), cuex_t e)
{
    cuex_meta_t e_meta = cuex_meta(e);
    if (cuex_meta_is_opr(e_meta)) {
	if (view == CUEX_FCOMM_VIEW)
	    return cuex_opn_comm_iterimg(f, e);
	else {
	    CUEX_OPN_TRAN(e_meta, e, ep, cu_call(f, ep));
	    return e;
	}
    }
    else if (cuex_meta_is_type(e_meta)) {
	cuoo_type_t e_type = cuoo_type_from_meta(e_meta);
	cuex_intf_compound_t impl;
	impl = cuoo_type_impl_ptr(e_type, CUEX_INTF_COMPOUND);
	if (impl) {
	    cu_ptr_junctor_t junctor;
	    if (view == CUEX_PREF_VIEW)
		junctor = cuex_compound_pref_image_junctor(impl, e);
	    else
		junctor = cuex_compound_comm_image_junctor(impl, e);
	    return cu_ptr_junctor_image(f, junctor);
	}
	else
	    return e;
    }
    else
	return e;
}

cuex_t
cuex_iterimgk_operands(cu_clop(f, cuex_t, int, cuex_t), cuex_t e)
{
    cuex_meta_t e_meta = cuex_meta(e);
    if (cuex_meta_is_opr(e_meta)) {
	int i = 0;
	CUEX_OPN_TRAN(e_meta, e, ep, cu_call(f, i++, ep));
	return e;
    }
    else if (cuex_meta_is_type(e_meta)) {
	cuoo_type_t e_type = cuoo_type_from_meta(e_meta);
	cuex_intf_compound_t impl;
	impl = cuoo_type_impl_ptr(e_type, CUEX_INTF_COMPOUND);
	if (impl) {
	    cuex_t ep;
	    int i = 0;
	    cu_ptr_junctor_t junctor;
	    junctor = cuex_compound_pref_image_junctor(impl, e);
	    while ((ep = cu_ptr_junctor_get(junctor)))
		cu_ptr_junctor_put(junctor, cu_call(f, i++, ep));
	    return cu_ptr_junctor_finish(junctor);
	}
	else
	    return e;
    }
    else
	return e;
}

cuex_t
cuex_iterimgk_operands_view(cuex_opview_t view,
			    cu_clop(f, cuex_t, int, cuex_t), cuex_t e)
{
    cuex_meta_t e_meta = cuex_meta(e);
    if (cuex_meta_is_opr(e_meta)) {
	if (view == CUEX_FCOMM_VIEW)
	    return cuex_opn_comm_iterimgk(f, e);
	else {
	    int i = 0;
	    CUEX_OPN_TRAN(e_meta, e, ep, cu_call(f, i++, ep));
	    return e;
	}
    }
    else if (cuex_meta_is_type(e_meta)) {
	cuoo_type_t e_type = cuoo_type_from_meta(e_meta);
	cuex_intf_compound_t impl;
	impl = cuoo_type_impl_ptr(e_type, CUEX_INTF_COMPOUND);
	if (impl) {
	    cuex_t ep;
	    int i = 0;
	    cu_ptr_junctor_t junctor;
	    if (view == CUEX_PREF_VIEW)
		junctor = cuex_compound_pref_image_junctor(impl, e);
	    else
		junctor = cuex_compound_comm_image_junctor(impl, e);
	    while ((ep = cu_ptr_junctor_get(junctor)))
		cu_ptr_junctor_put(junctor, cu_call(f, i++, ep));
	    return cu_ptr_junctor_finish(junctor);
	}
	else
	    return e;
    }
    else
	return e;
}
