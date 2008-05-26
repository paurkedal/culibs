/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2005--2007  Petter Urkedal <urkedal@nbi.dk>
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

#include <cuex/tvar.h>
#include <cuoo/intf.h>

cuex_tvar_t
cuex_tvar_new_type(cuex_qcode_t qc, cuoo_type_t type_opt)
{
    cuex_tvar_t tv;
    tv = cuexP_oalloc(cuex_tvarmeta(qc), sizeof(struct cuex_tvar_s));
    cuoo_type_init_general(cu_to(cuoo_type, tv), cuoo_typekind_tvar,
			   cuoo_impl_none, NULL);
    tv->type = (AO_t)type_opt;
    return tv;
}

cuex_tvar_t
cuex_tvar_new_obj(cuex_qcode_t qc, cuoo_type_t type_opt)
{
    cuex_tvar_t tv;
    tv = cuexP_oalloc(cuex_tvarmeta(qc), sizeof(struct cuex_tvar_s));
    cuoo_type_init_general(cu_to(cuoo_type, tv), cuoo_typekind_none,
			   cuoo_impl_none, NULL);
    tv->type = (AO_t)type_opt;
    return tv;
}

cuex_tvar_t
cuex_tvar_new_as(cuex_tvar_t v_tpl)
{
    cuex_meta_t meta = cuex_meta(v_tpl);
    cuex_tvar_t v;
    cu_debug_assert(cuex_is_tvarmeta(meta));
    v = cuexP_oalloc(meta, sizeof(struct cuex_tvar_s));
    cuoo_type_init_general(cu_to(cuoo_type, v),
			   cuoo_type_typekind(cu_to(cuoo_type, v_tpl)),
			   cuoo_impl_none, NULL);
    v->type = v_tpl->type;
    return v;
}
