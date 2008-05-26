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
#include <cu/wchar.h>
#include <cu/wstring.h>
#include <cu/str.h>
#include <cu/ptr.h>
#include <cuoo/halloc.h>
#include <cuoo/intf.h>

cu_idr_t
cu_idr_by_cstr(char const *cstr)
{
    size_t cstr_size = strlen(cstr) + 1;
    size_t key_size = CUOO_HCOBJ_KEY_SIZE(sizeof(struct cu_idr_s) + cstr_size);
    cu_idr_t idr = cu_salloc(key_size + CUOO_HCOBJ_SHIFT);
    char *a_cstr = (char *)(idr + 1);
    idr->key_size = key_size;
    memcpy(a_cstr, cstr, cstr_size);
    memset(a_cstr + cstr_size, 0,
	   key_size - sizeof(struct cu_idr_s) - cstr_size + CUOO_HCOBJ_SHIFT);
    idr = cuoo_halloc(cu_idr_type(), key_size, (char *)idr + CUOO_HCOBJ_SHIFT);
    return idr;
}

cu_idr_t
cu_idr_by_charr(char const *arr, size_t charr_size)
{
    size_t cstr_size = charr_size + 1;
    size_t key_size = CUOO_HCOBJ_KEY_SIZE(sizeof(struct cu_idr_s) + cstr_size);
    cu_idr_t idr = cu_salloc(key_size + CUOO_HCOBJ_SHIFT);
    char *a_arr = (char *)(idr + 1);
    idr->key_size = key_size;
    memcpy(a_arr, arr, charr_size);
    memset(a_arr + charr_size, 0,
	   key_size - sizeof(struct cu_idr_s) - charr_size + CUOO_HCOBJ_SHIFT);
    idr = cuoo_halloc(cu_idr_type(), key_size, (char *)idr + CUOO_HCOBJ_SHIFT);
    return idr;
}

cu_idr_t
cu_idr_by_wchararr(wchar_t const *arr, size_t len)
{
    char *inarr = (char *)arr;
    size_t inlen = len*sizeof(wchar_t);
    size_t outcap = len*CU_MAX_MBLEN;
    char *charr = cu_salloc(outcap);
    char *outarr = charr;
    iconv_t cd = cu_iconv_for_wchar_to_char();
    if (iconv(cd, &inarr, &inlen, &outarr, &outcap) == (size_t)-1) {
	switch (errno) {
	    case E2BIG:
		cu_debug_unreachable();
	    case EILSEQ:
	    case EINVAL:
		return NULL;
	}
    }
    return cu_idr_by_charr(charr, outarr - charr);
}

cu_idr_t
cu_idr_by_wstring(cu_wstring_t s)
{
    return cu_idr_by_wchararr(s->arr, s->len);
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

cu_clop_def(idr_key_hash_clop, cu_hash_t, void *idr)
{
    size_t key_size = ((cu_idr_t)idr)->key_size;
    return cu_wordarr_hash(key_size/sizeof(cu_word_t),
			   cu_ptr_add(idr, CUOO_HCOBJ_SHIFT),
			   cuoo_type_to_meta(cuP_idr_type));
}

cuoo_type_t cuP_idr_type;

static void
idr_print(void *idr, FILE *out)
{
    char const *s = cu_idr_to_cstr(idr);
    fputs(s, out);
}

static cu_word_t
idr_impl(cu_word_t intf_number, ...)
{
    switch (intf_number) {
	case CUOO_INTF_PRINT_FN: return (cu_word_t)idr_print;
	case CUOO_INTF_TO_STR_FN: return (cu_word_t)cu_str_new_idr;
	default: return CUOO_IMPL_NONE;
    }
}

void
cuP_idr_init(void)
{
    cuP_idr_type = cuoo_type_new_opaque_hcf(idr_impl, idr_key_hash_clop);
}
