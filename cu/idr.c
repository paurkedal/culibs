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

cu_idr_t
cu_idr_by_cstr(char const *cstr)
{
    size_t cstr_size = strlen(cstr) + 1;
    size_t idr_size = sizeof(struct cu_idr_s) + cu_aligned_ceil(cstr_size);
    cu_idr_t idr = cu_salloc(idr_size);
    char *a_cstr = (char *)(idr + 1);
    idr->key_size = idr_size - offsetof(struct cu_idr_s, key_size);
    *(cu_word_t *)((char *)idr + idr_size - sizeof(cu_word_t)) = 0;
    memcpy(a_cstr, cstr, cstr_size);
    idr = cuex_halloc_by_value(cudyn_type_to_meta(cu_idr_type()), idr_size, idr);
    return idr;
}

cu_idr_t
cu_idr_by_charr(char const *arr, size_t charr_size)
{
    size_t cstr_size = charr_size + 1;
    size_t idr_size = sizeof(struct cu_idr_s) + cu_aligned_ceil(cstr_size);
    cu_idr_t idr = cu_salloc(idr_size);
    char *a_arr = (char *)(idr + 1);
    idr->key_size = idr_size - offsetof(struct cu_idr_s, key_size);
    *(cu_word_t *)((char *)idr + idr_size - sizeof(cu_word_t)) = 0;
    memcpy(a_arr, arr, charr_size);
    a_arr[charr_size] = 0;
    idr = cuex_halloc_by_value(cudyn_type_to_meta(cu_idr_type()), idr_size, idr);
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
