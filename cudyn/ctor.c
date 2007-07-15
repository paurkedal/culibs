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
#include <cu/halloc.h>

cudyn_ctortype_t
cudyn_ctortype(cudyn_tuptype_t parttype, cudyn_duntype_t fulltype)
{
    cudyn_ctortype_t ctortype;
    cudyn_hctem_decl(cudyn_ctortype, key);
    cudyn_hctem_init(cudyn_ctortype, key);
    ctortype = cudyn_hctem_get(cudyn_ctortype, key);
    cudynP_hctype_cct_hcs(cu_to(cudyn_hctype, ctortype), NULL,
			  cudyn_typekind_ctortype, sizeof(unsigned int));
    ctortype->fulltype = fulltype;
    ctortype->parttype = parttype;
    return cudyn_hctem_new(cudyn_ctortype, key);
}

cuex_t
cudyn_ctor(cudyn_ctortype_t type, unsigned int ctor_num)
{
    return cudyn_halloc(cudyn_ctortype_to_type(type),
			sizeof(unsigned int), &ctor_num);
}


cudyn_stdtype_t cudynP_ctortype_type;

void
cudyn_ctor_init()
{
    cudynP_ctortype_type = cudyn_stdtype_new_hcs(
	    sizeof(struct cudyn_ctortype_s) - CU_HCOBJ_SHIFT);
}
