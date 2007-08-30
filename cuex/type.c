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

#include <cuex/oprdefs.h>
#include <cuex/acidefs.h>
#include <cuex/type.h>
#include <cuex/algo.h>
#include <cuex/sig.h>
#include <cuex/tvar.h>
#include <cudyn/type.h>
#include <cudyn/misc.h>

cu_rank_t
cuex_farrow_arity(cuex_t type)
{
    cu_rank_t r = 0;
    for (;;) switch (cuex_meta(type)) {
	case CUEX_O2_FORALL:
	    type = cuex_opn_at(type, 1);
	    break;
	case CUEX_O2_FARROW:
	    ++r;
	    type = cuex_opn_at(type, 1);
	    break;
	default:
	    return r;
    }
}

size_t
cuex_type_bitsize(cuex_t type)
{
    cuex_meta_t meta;
tailcall:
    if (cuoo_is_type(type)) {
	if (cuoo_type_typekind(type) != cuoo_typekind_by_expr)
	    return cuoo_type_bitsize(type);
	else
	    type = cuoo_type_as_expr(type);
    }

    meta = cuex_meta(type);
    if (cuex_is_tvarmeta_q(meta, cuex_qcode_u))
	return 8*sizeof(void *);
    else if (cuex_meta_is_opr(meta))
	switch (cuex_meta(type)) {
		cuex_t ty0;
	    case CUEX_O2_FORALL:
		type = cuex_opn_at(type, 1);
		goto tailcall;
	    case CUEX_O4ACI_SIGPROD:
	    case CUEX_O2_SIGBASE:
		return cuex_sig_bitsize(type);
	    case CUEX_O0ACI_SIGPROD:
	    case CUEX_O1_SINGLETON:
		return 0;
	    case CUEX_O2_GEXPT:
		return cuex_type_bitsize(cuex_opn_at(type, 0))
		     * cudyn_castget_ulong(cuex_opn_at(type, 1));
	    case CUEX_O1_PTR_TO:
		return 8*sizeof(void *);
	    case CUEX_O2_FARROW:
	    case CUEX_O2_FARROW_NATIVE:
		return 8*sizeof(void *);
	    case CUEX_O1_VTABLE_SLOTS:
		ty0 = cuex_opn_at(type, 0);
		if (cuex_is_unknown(ty0))
		    return 0;
		return 8*sizeof(void *)*(cuex_farrow_arity(ty0) + 1);
	    case CUEX_O0_UNKNOWN:
		return 0;
	    default:
		cu_debug_unreachable();
		return 0;
	}
    else
	cu_debug_unreachable();
}

cu_offset_t
cuex_type_bitalign(cuex_t type)
{
    if (cuoo_is_type(type))
	return cuoo_type_bitalign(type);
    switch (cuex_meta(type)) {
	case CUEX_O0ACI_SIGPROD:
	case CUEX_O1_SINGLETON:
	    return 1;
	case CUEX_O4ACI_SIGPROD:
	case CUEX_O2_SIGBASE:
	    return cuex_sig_bitalign(type);
	default:
	    return 8*sizeof(void *);
    }
}

cuex_t
cuex_typeof(cuex_t e)
{
    cuex_meta_t m = cuex_meta(e);
    switch (cuex_meta_kind(m)) {
	case cuex_meta_kind_type:
	    return cuoo_type_from_meta(m);
	case cuex_meta_kind_opr:
	    return cudyn_cuex_type();
	case cuex_meta_kind_other:
	    return cudyn_cuex_type();
	default:
	    cu_debug_unreachable();
    }
}

cuex_t cuexP_generic_ptr_type;

void
cuexP_type_init()
{
    cuexP_generic_ptr_type = cuex_o1_ptr_to(cudyn_singular_type());
}
