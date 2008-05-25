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

#define CUDYN_CTOR_C
#include <cudyn/ctor.h>
#include <cuoo/halloc.h>
#include <cuoo/intf.h>

cudyn_ctortype_t
cudyn_ctortype(cudyn_tuptype_t parttype, cudyn_duntype_t fulltype)
{
    cudyn_ctortype_t ctortype;
    cuoo_hctem_decl(cudyn_ctortype, key);
    cuoo_hctem_init(cudyn_ctortype, key);
    ctortype = cuoo_hctem_get(cudyn_ctortype, key);
    cuooP_type_cct_hcs(cu_to(cuoo_type, ctortype), cuoo_impl_none, NULL,
		       cuoo_typekind_ctortype, sizeof(unsigned int));
    ctortype->fulltype = fulltype;
    ctortype->parttype = parttype;
    return cuoo_hctem_new(cudyn_ctortype, key);
}

cuex_t
cudyn_ctor(cudyn_ctortype_t type, unsigned int ctor_num)
{
    return cuoo_halloc(cudyn_ctortype_to_type(type),
		       sizeof(unsigned int), &ctor_num);
}


cuoo_type_t cudynP_ctortype_type;

void
cudyn_ctor_init()
{
    cudynP_ctortype_type = cuoo_type_new_opaque_hcs(
	cuoo_impl_none, sizeof(struct cudyn_ctortype_s) - CUOO_HCOBJ_SHIFT);
}
