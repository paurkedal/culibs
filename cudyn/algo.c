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

#include <cudyn/algo.h>
#include <cuex/opn.h>

cu_bool_t
cudyn_conj(void *obj, cu_clop(pred, cu_bool_t, void *))
{
    cuex_meta_t meta = cuex_meta(obj);
    switch (cuex_meta_kind(meta)) {
	void **opd;
	void **opd_end;

    case cuex_meta_kind_type:
	return cu_call(cudyn_stdtype_from_meta(meta)->conj, obj, pred);

    case cuex_meta_kind_opr:
	opd = cuex_opn_begin(obj);
	opd_end = cuex_opn_end(obj);
	while (opd != opd_end) {
	    if (!cu_call(pred, *opd))
		return cu_false;
	    ++opd;
	}
	return cu_true;

    case cuex_meta_kind_other:
	return cu_true;

    default:
	cu_debug_unreachable();
	return cu_false;
    }
}

void *
cudyn_tran(void *obj, cu_clop(trn, void *, void *))
{
    cuex_meta_t meta = cuex_meta(obj);
    switch (cuex_meta_kind(meta)) {
	void **opd_begin;
	void **opd;
	void **opd_end;
	void *sub;
	void **new_arr_begin;
	void **new_arr;

    case cuex_meta_kind_type:
	return cu_call(cudyn_stdtype_from_meta(meta)->tran, obj, trn);

    case cuex_meta_kind_opr:
	opd = cuex_opn_begin(obj);
	opd_end = cuex_opn_end(obj);
	for (;;) {
	    if (opd == opd_end)
		return obj;
	    sub = cu_call(trn, *opd);
	    if (sub != *opd)
		break;
	    ++opd;
	}
	opd_begin = cuex_opn_begin(obj);
	new_arr_begin = cu_salloc(sizeof(void *)*cuex_opn_arity(obj));
	new_arr = new_arr_begin;
	memcpy(new_arr, opd_begin, (char *)opd - (char *)opd_begin);
	new_arr += opd - opd_begin;
	*new_arr = sub;
	while (++opd != opd_end)
	    *++new_arr = cu_call(trn, *opd);
	return cuex_opn_by_arr(meta, new_arr_begin);

    case cuex_meta_kind_other:
	return obj;

    default:
	cu_unreachable();
	return NULL;
    }
}
