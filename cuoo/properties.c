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
#include <cu/sref.h>

struct cuoo_prop_s cuooP_raw_c_name_prop;
struct cuoo_prop_s cuooP_sref_prop;
struct cuoo_prop_s cuooP_raw_print_fn_prop;

cu_bool_t cuoo_type_print(cuoo_type_t, FILE *);

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
	void (*print)(void *, FILE *)
	    = cuoo_prop_get_ptr(&cuooP_raw_print_fn_prop, type);
	if (print) {
	    print(ex, out);
	    return cu_true;
	}
#if 0 /* FIXME */
	if (cuoo_type_is_typeoftypes(type) && cuoo_type_print(ex, out))
	    return cu_true;
#endif
    }
    return cu_false;
}

void
cuooP_properties_init()
{
    cuoo_prop_cct(&cuooP_raw_c_name_prop);
    cuoo_prop_cct(&cuooP_sref_prop);
    cuoo_prop_cct(&cuooP_raw_print_fn_prop);
}
