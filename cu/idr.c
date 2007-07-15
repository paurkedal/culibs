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

#include <cu/idr.h>
#include <cu/util.h>
#include <cu/halloc.h>

cu_idr_t
cu_idr_by_cstr(char const *cstr)
{
    size_t cstr_size = strlen(cstr) + 1;
    size_t key_size = CU_HCOBJ_KEY_SIZE(sizeof(struct cu_idr_s) + cstr_size);
    cu_idr_t idr = cu_salloc(key_size + CU_HCOBJ_SHIFT);
    char *a_cstr = (char *)(idr + 1);
    idr->key_size = key_size;
    memcpy(a_cstr, cstr, cstr_size);
    memset(a_cstr + cstr_size, 0,
	   key_size - sizeof(struct cu_idr_s) - cstr_size + CU_HCOBJ_SHIFT);
    idr = cudyn_halloc(cu_idr_type(), key_size, (char *)idr + CU_HCOBJ_SHIFT);
    return idr;
}

cu_idr_t
cu_idr_by_charr(char const *arr, size_t charr_size)
{
    size_t cstr_size = charr_size + 1;
    size_t key_size = CU_HCOBJ_KEY_SIZE(sizeof(struct cu_idr_s) + cstr_size);
    cu_idr_t idr = cu_salloc(key_size + CU_HCOBJ_SHIFT);
    char *a_arr = (char *)(idr + 1);
    idr->key_size = key_size;
    memcpy(a_arr, arr, charr_size);
    memset(a_arr + charr_size, 0,
	   key_size - sizeof(struct cu_idr_s) - charr_size + CU_HCOBJ_SHIFT);
    idr = cudyn_halloc(cu_idr_type(), key_size, (char *)idr + CU_HCOBJ_SHIFT);
    return idr;
}

int
cu_idr_strcmp(cu_idr_t idr0, cu_idr_t idr1)
{
    return strcmp(cu_idr_to_cstr(idr0), cu_idr_to_cstr(idr1));
}

cu_clop_edef(cu_idr_strcmp_clop, int, cu_idr_t idr0, cu_idr_t idr1)
{
    return strcmp(cu_idr_to_cstr(idr0), cu_idr_to_cstr(idr1));
}

cu_clop_def(idr_key_size_clop, size_t, void *idr)
{
    return ((cu_idr_t)idr)->key_size;
}

cudyn_stdtype_t cuP_idr_type;

void
cuP_idr_init(void)
{
    cuP_idr_type = cudyn_stdtype_new_hcv(idr_key_size_clop);
}
