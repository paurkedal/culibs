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

#include <cudyn/properties.h>
#include <cu/sref.h>

struct cudyn_prop_s cudynP_raw_c_name_prop;
struct cudyn_prop_s cudynP_sref_prop;
struct cudyn_prop_s cudynP_raw_print_fn_prop;

cu_bool_t cudyn_type_print(cudyn_type_t, FILE *);

void
cudyn_sref_push(cuex_t ex, cu_sref_t sref)
{
    cu_sref_t *slot;
    if (!cudyn_prop_set_mem_lock(&cudynP_sref_prop, ex,
				 sizeof(cu_sref_t), &slot))
	cu_sref_set_chain_tail(sref, *slot);
    *slot = sref;
    cudyn_prop_set_mem_unlock(&cudynP_sref_prop, ex);
}

cu_bool_t
cudyn_raw_print(cuex_t ex, FILE *out)
{
    cuex_meta_t meta = cuex_meta(ex);
    if (cuex_meta_is_type(meta)) {
	cudyn_type_t type = cudyn_type_from_meta(meta);
	void (*print)(void *, FILE *)
	    = cudyn_prop_get_ptr(&cudynP_raw_print_fn_prop, type);
	if (print) {
	    print(ex, out);
	    return cu_true;
	}
	if (cudyn_type_is_typeoftypes(type) && cudyn_type_print(ex, out))
	    return cu_true;
    }
    return cu_false;
}

void
cudynP_properties_init()
{
    cudyn_prop_cct(&cudynP_raw_c_name_prop);
    cudyn_prop_cct(&cudynP_sref_prop);
    cudyn_prop_cct(&cudynP_raw_print_fn_prop);
}
