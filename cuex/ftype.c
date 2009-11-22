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

#include <cuex/ftype.h>
#include <cuex/type.h>
#include <cuex/oprdefs.h>
#include <cuex/opn.h>
#include <cuex/var.h>
#include <cuex/fpvar.h>

cuex_t
cuex_ftype_apply_n(cuex_t ftype, cu_rank_t n)
{
    cu_rank_t i;
    for (i = 0; i < n; ++i) {
	if (cuex_meta(ftype) == CUEX_O2_FARROW)
	    ftype = cuex_opn_at(ftype, 1);
	else
	    return NULL;
    }
    return ftype;
}

cuex_t
cuex_ftype_apply_argtuple(cuex_t ftype, cuex_t arg_tuple)
{
    cu_rank_t r = cuex_opn_r(arg_tuple);
    cu_rank_t i;
    for (i = 0; i < r; ++i) {
	if (cuex_meta(ftype) == CUEX_O2_FARROW) {
	    cuex_t param_type = cuex_opn_at(ftype, 0);
	    cuex_t arg_type = cuex_typeof(cuex_opn_at(arg_tuple, i));
	    if (param_type != arg_type)
		cu_bugf("Argument type %! does not match parameter type %!.",
			arg_type, param_type);
	    ftype = cuex_opn_at(ftype, 1);
	}
	else
	    return NULL;
    }
    return ftype;
}

static cu_bool_t
fill_argtuple(cuex_t *argv, cu_rank_t index_begin, cu_rank_t index_end,
	      cuex_t ftype, cuex_t *r_type_out)
{
    while (index_begin < index_end) {
	cuex_t arg_type;
	if (cuex_meta(ftype) != CUEX_O2_FARROW)
	    return cu_false;
	arg_type = cuex_opn_at(ftype, 0);
	*argv++ = cuex_fpvar(index_begin, arg_type);
	ftype = cuex_opn_at(ftype, 1);
	++index_begin;
    }
    if (r_type_out)
	*r_type_out = ftype;
    return cu_true;
}

cuex_t
cuex_ftype_argtuple(cuex_t ftype,
		    cu_rank_t index_begin, cu_rank_t index_end,
		    cuex_t *r_type_out)
{
    cu_rank_t r = index_end - index_begin;
    cuex_t *argv = cu_salloc(sizeof(cuex_t)*r);
    if (fill_argtuple(argv, index_begin, index_end, ftype, r_type_out))
	return cuex_opn_by_arr(CUEX_OR_TUPLE(r), argv);
    else
	return NULL;
}

cuex_t
cuex_ftype_argtuple_carg(cuex_t carg_type, cuex_t ftype,
			 cu_rank_t index_begin, cu_rank_t index_end,
			 cuex_t *r_type_out)
{
    cu_rank_t r = index_begin - index_end;
    cuex_t *argv = cu_salloc(sizeof(cuex_t)*r);
    argv[0] = cuex_fpvar(index_begin++, carg_type);
    if (fill_argtuple(argv + 1, index_begin, index_end, ftype, r_type_out))
	return cuex_opn_by_arr(CUEX_OR_TUPLE(r), argv);
    else
	return NULL;
}

static cuex_t
extract_and_order_tparam(cuex_t e, cucon_pmap_t tparam_to_index,
			 cu_rank_t *next_index)
{
tailcall:
    switch (cuex_meta(e)) {
	case CUEX_O2_FORALL: {
	    cuex_t param = cuex_opn_at(e, 0);
	    cu_rank_t *index;
	    cucon_pmap_insert_mem(tparam_to_index, param,
				  sizeof(cu_rank_t), &index);
	    *index = -1;
	    e = cuex_opn_at(e, 1);
	    goto tailcall;
	}
	case CUEX_O2_FARROW: {
	    cuex_t ea, er;
	    ea = extract_and_order_tparam(cuex_opn_at(e, 0),
					     tparam_to_index, next_index);
	    er = extract_and_order_tparam(cuex_opn_at(e, 1),
					     tparam_to_index, next_index);
	    return cuex_o2_farrow(ea, er);
	}
	default: {
	    cu_rank_t *index = cucon_pmap_find_mem(tparam_to_index, e);
	    if (index) {
		if (*index == -1)
		    *index = (*next_index)++;
		return cuex_ivar(cuex_qcode_u, *index);
	    }
	    else
		return e;
	}
    }
}

cuex_t
cuex_ftype_normalise_no_quant(cuex_t e, cu_rank_t *tparam_cnt_out)
{
    struct cucon_pmap tparam_to_index;
    cucon_pmap_init(&tparam_to_index);
    *tparam_cnt_out = 0;
    return extract_and_order_tparam(e, &tparam_to_index, tparam_cnt_out);
}

cuex_t
cuex_ftype_normalise_outmost_quant(cuex_t e, cu_rank_t *tparam_cnt_out)
{
    struct cucon_pmap tparam_to_index;
    cu_rank_t i;
    cucon_pmap_init(&tparam_to_index);
    *tparam_cnt_out = 0;
    e = extract_and_order_tparam(e, &tparam_to_index, tparam_cnt_out);
    for (i = 0; i < *tparam_cnt_out; ++i)
	e = cuex_o2_forall(cuex_ivar(cuex_qcode_u, i), e);
    return e;
}
