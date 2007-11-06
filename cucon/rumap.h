/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2004--2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUCON_RUMAP_H
#define CUCON_RUMAP_H

#include <cucon/pmap.h>
#include <cu/conf.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cucon_rumap cucon/rumap.h: Recursive Hash Map with Integer Keys
 * @{\ingroup cucon_maps_and_sets_mod */

struct cucon_rumap_s
{
    struct cucon_pmap_s pmap;
    void *data;
};

void cucon_rumap_cct(cucon_rumap_t rmap);

cucon_rumap_t cucon_rumap_new(void);

CU_SINLINE void *
cucon_rumap_value(cucon_rumap_t rmap)
{
    return rmap->data;
}

CU_SINLINE void
cucon_rumap_set_value(cucon_rumap_t rmap, void *ptr)
{
    rmap->data = ptr;
}

CU_SINLINE cu_bool_t
cucon_rumap_is_leaf(cucon_rumap_t rmap)
{
    return cucon_pmap_is_empty(&rmap->pmap);
}

cucon_rumap_t cucon_rumap_mref(cucon_rumap_t rmap, uintptr_t key);

CU_SINLINE cucon_rumap_t
cucon_rumap_cref(cucon_rumap_t rmap, uintptr_t key)
{
    return cucon_umap_find_mem(&rmap->pmap.impl, key);
}


/* Array lookup
 * ------------ */

cucon_rumap_t
cucon_rumap_mref_by_uint8_arr(cucon_rumap_t rmap,
			      uint8_t const *key_arr, size_t key_cnt);
cucon_rumap_t
cucon_rumap_mref_by_uint16_arr(cucon_rumap_t rmap,
			       uint16_t const *key_arr, size_t key_cnt);
cucon_rumap_t
cucon_rumap_mref_by_uint32_arr(cucon_rumap_t rmap,
			       uint32_t const *key_arr, size_t key_cnt);
#if CUCONF_WIDTHOF_INTPTR_T >= 64
cucon_rumap_t
cucon_rumap_mref_by_uint64_arr(cucon_rumap_t rmap,
			       uint64_t const *key_arr, size_t key_cnt);
#endif

#if CUCONF_WIDTHOF_INTPTR_T < CUCONF_WIDTHOF_WCHAR_T \
 || CUCONF_WIDTHOF_INTPTR_T < 32
#error Failed size assumptions.  Please report this incompatibility including
#error the output of the generated file ccf/config.h, your architecture
#error (uname -a) and the compiler your used.
#endif

#define cuconP_APPLY(macro, args...) macro(args)
#define cuconP_RUMAP_MREF_ARR(width, rmap, key_arr, key_cnt)	\
	cucon_rumap_mref_by_uint##width##_arr(			\
	    rmap, (uint##width##_t *)key_arr, key_cnt)

CU_SINLINE cucon_rumap_t
cucon_rumap_mref_by_uchar_arr(cucon_rumap_t rmap,
			      unsigned char const *key_arr, size_t key_cnt)
{
    return cuconP_APPLY(cuconP_RUMAP_MREF_ARR, CUCONF_WIDTHOF_CHAR,
			rmap, key_arr, key_cnt);
}
CU_SINLINE cucon_rumap_t
cucon_rumap_mref_by_schar_arr(cucon_rumap_t rmap,
			      signed char const *key_arr, size_t key_cnt)
{
    return cuconP_APPLY(cuconP_RUMAP_MREF_ARR, CUCONF_WIDTHOF_CHAR,
			rmap, key_arr, key_cnt);
}
CU_SINLINE cucon_rumap_t
cucon_rumap_mref_by_short_arr(cucon_rumap_t rmap,
			      short const *key_arr, size_t key_cnt)
{
    return cuconP_APPLY(cuconP_RUMAP_MREF_ARR, CUCONF_WIDTHOF_SHORT,
			rmap, key_arr, key_cnt);
}
CU_SINLINE cucon_rumap_t
cucon_rumap_mref_by_ushort_arr(cucon_rumap_t rmap,
			       unsigned short const *key_arr, size_t key_cnt)
{
    return cuconP_APPLY(cuconP_RUMAP_MREF_ARR, CUCONF_WIDTHOF_SHORT,
			rmap, key_arr, key_cnt);
}
CU_SINLINE cucon_rumap_t
cucon_rumap_mref_by_int_arr(cucon_rumap_t rmap,
			    int const *key_arr, size_t key_cnt)
{
    return cuconP_APPLY(cuconP_RUMAP_MREF_ARR, CUCONF_WIDTHOF_INT,
			rmap, key_arr, key_cnt);
}
CU_SINLINE cucon_rumap_t
cucon_rumap_mref_by_uint_arr(cucon_rumap_t rmap,
			     unsigned int const *key_arr, size_t key_cnt)
{
    return cuconP_APPLY(cuconP_RUMAP_MREF_ARR, CUCONF_WIDTHOF_INT,
			rmap, key_arr, key_cnt);
}
#if 0 /* the pmap only supports uintptr_t */
CU_SINLINE cucon_rumap_t
cucon_rumap_mref_by_long_arr(cucon_rumap_t rmap,
			   long *key_arr, size_t key_cnt)
{
    return cuconP_APPLY(cuconP_RUMAP_MREF_ARR, CUCONF_WIDTHOF_LONG,
		      rmap, key_arr, key_cnt);
}
CU_SINLINE cucon_rumap_t
cucon_rumap_mref_by_ulong_arr(cucon_rumap_t rmap,
			    unsigned long *key_arr, size_t key_cnt)
{
    return cuconP_APPLY(cuconP_RUMAP_MREF_ARR, CUCONF_WIDTHOF_LONG,
		      rmap, key_arr, key_cnt);
}
#endif

CU_SINLINE cucon_rumap_t
cucon_rumap_mref_by_char_arr(cucon_rumap_t rmap,
			     char const *key_arr, size_t key_cnt)
{
    return cuconP_APPLY(cuconP_RUMAP_MREF_ARR, CUCONF_WIDTHOF_CHAR,
			rmap, key_arr, key_cnt);
}
CU_SINLINE cucon_rumap_t
cucon_rumap_mref_by_wchar_arr(cucon_rumap_t rmap,
			      cu_wchar_t const *key_arr, size_t key_cnt)
{
    return cuconP_APPLY(cuconP_RUMAP_MREF_ARR, CU_WCHAR_WIDTH,
			rmap, key_arr, key_cnt);
}
#undef cuconP_APPLY
#undef cuconP_RUMAP_MREF_ARR

cucon_rumap_t cucon_rumap_mref_by_wstring(cucon_rumap_t rmap, cu_wstring_t s);


/* Iteration
 * --------- */

CU_SINLINE cu_bool_t
cucon_rumap_conj(cucon_rumap_t rmap,
		 cu_clop(fn, cu_bool_t, uintptr_t, cucon_rumap_t))
{
    return cucon_umap_conj_mem(&rmap->pmap.impl,
			       (cu_clop(, cu_bool_t, uintptr_t, void *))fn);
}

/*!@}*/
CU_END_DECLARATIONS

#endif
