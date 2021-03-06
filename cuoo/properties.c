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

#include <cuoo/properties.h>
#include <cuoo/type.h>
#include <cuoo/intf.h>
#include <cu/sref.h>

struct cuoo_prop cuooP_raw_c_name_prop;
struct cuoo_prop cuooP_sref_prop;

void
cuoo_sref_push(cuex_t ex, cu_sref_t sref)
{
    cu_sref_t *slot;
    if (!cuoo_prop_set_mem_lock(&cuooP_sref_prop, ex,
				sizeof(cu_sref_t), &slot))
	cu_sref_set_chain_tail(sref, *slot);
    *slot = sref;
    cuoo_prop_set_mem_unlock(&cuooP_sref_prop, ex);
}

cu_bool_t
cuoo_raw_print(cuex_t ex, FILE *out)
{
    cuex_meta_t meta = cuex_meta(ex);
    if (cuex_meta_is_type(meta)) {
	cuoo_type_t type = cuoo_type_from_meta(meta);
	cuoo_intf_print_fn_t print
	    = cuoo_type_impl_fptr(cuoo_intf_print_fn_t, type,
				  CUOO_INTF_PRINT_FN);
	if (print) {
	    print(ex, out);
	    return cu_true;
	}
    }
    return cu_false;
}

void
cuooP_properties_init()
{
    cuoo_prop_cct(&cuooP_raw_c_name_prop);
    cuoo_prop_cct(&cuooP_sref_prop);
}
