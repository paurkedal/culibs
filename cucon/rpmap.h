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

#ifndef CUCON_RPMAP_H
#define CUCON_RPMAP_H

#include <cucon/pmap.h>
#include <cu/conf.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cucon_rpmap cucon/rpmap.h: Recursive Hash Map with Pointer or Integer Keys
 * @{\ingroup cucon_maps_and_sets_mod */

struct cucon_rpmap_s
{
    struct cucon_pmap_s pmap;
    void *data;
};
#define cucon_rumap_s cucon_rpmap_s

void cucon_rpmap_cct(cucon_rpmap_t rpmap);
#define cucon_rumap_cct cucon_rpmap_cct

cucon_rpmap_t cucon_rpmap_new(void);
#define cucon_rumap_new cucon_rpmap_new

CU_SINLINE void *
cucon_rpmap_value(cucon_rpmap_t rpmap)
{
    return rpmap->data;
}
#define cucon_rumap_value cucon_rpmap_value

CU_SINLINE void
cucon_rpmap_set_value(cucon_rpmap_t rpmap, void *ptr)
{
    rpmap->data = ptr;
}
#define cucon_rumap_set_value cucon_rpmap_set_value

CU_SINLINE cu_bool_t
cucon_rpmap_is_leaf(cucon_rpmap_t rpmap)
{
    return cucon_pmap_is_empty(&rpmap->pmap);
}
#define cucon_rumap_is_leaf cucon_rpmap_is_leaf

cucon_rpmap_t cucon_rpmap_mref(cucon_rpmap_t rpmap, void *key);
cucon_rpmap_t cucon_rumap_mref(cucon_rpmap_t rpmap, uintptr_t key);

CU_SINLINE cucon_rpmap_t
cucon_rpmap_cref(cucon_rpmap_t rpmap, void *key)
{
    return cucon_pmap_find_mem(&rpmap->pmap, key);
}
CU_SINLINE cucon_rumap_t
cucon_rumap_cref(cucon_rumap_t rumap, uintptr_t key)
{
    return cucon_umap_find_mem(&rumap->pmap.impl, key);
}


/* Array lookup
 * ------------ */

cucon_rumap_t cucon_rumap_mref_by_uint8_arr(cucon_rumap_t rumap,
					uint8_t *key_arr, size_t key_cnt);
cucon_rumap_t cucon_rumap_mref_by_uint16_arr(cucon_rumap_t rumap,
					 uint16_t *key_arr, size_t key_cnt);
cucon_rumap_t cucon_rumap_mref_by_uint32_arr(cucon_rumap_t rumap,
					 uint32_t *key_arr, size_t key_cnt);
#if CUCONF_WIDTHOF_INTPTR_T >= 64
cucon_rumap_t cucon_rumap_mref_by_uint64_arr(cucon_rumap_t rumap,
					 uint64_t *key_arr, size_t key_cnt);
#endif

#if CUCONF_WIDTHOF_INTPTR_T < CUCONF_WIDTHOF_WCHAR_T \
 || CUCONF_WIDTHOF_INTPTR_T < 32
#error Failed size assumptions.  Please report this incompatibility including
#error the output of the generated file ccf/config.h, your architecture
#error (uname -a) and the compiler your used.
#endif

#define cuconP_APPLY(macro, args...) macro(args)
#define cuconP_RUMAP_MREF_ARR(width, rumap, key_arr, key_cnt)	\
	cucon_rumap_mref_by_uint##width##_arr(			\
	    rumap, (uint##width##_t *)key_arr, key_cnt)

CU_SINLINE cucon_rumap_t
cucon_rumap_mref_by_uchar_arr(cucon_rumap_t rumap,
			    unsigned char *key_arr, size_t key_cnt)
{
    return cuconP_APPLY(cuconP_RUMAP_MREF_ARR, CUCONF_WIDTHOF_CHAR,
		      rumap, key_arr, key_cnt);
}
CU_SINLINE cucon_rumap_t
cucon_rumap_mref_by_schar_arr(cucon_rumap_t rumap,
			    signed char *key_arr, size_t key_cnt)
{
    return cuconP_APPLY(cuconP_RUMAP_MREF_ARR, CUCONF_WIDTHOF_CHAR,
		      rumap, key_arr, key_cnt);
}
CU_SINLINE cucon_rumap_t
cucon_rumap_mref_by_short_arr(cucon_rumap_t rumap,
			    short *key_arr, size_t key_cnt)
{
    return cuconP_APPLY(cuconP_RUMAP_MREF_ARR, CUCONF_WIDTHOF_SHORT,
		      rumap, key_arr, key_cnt);
}
CU_SINLINE cucon_rumap_t
cucon_rumap_mref_by_ushort_arr(cucon_rumap_t rumap,
			     unsigned short *key_arr, size_t key_cnt)
{
    return cuconP_APPLY(cuconP_RUMAP_MREF_ARR, CUCONF_WIDTHOF_SHORT,
		      rumap, key_arr, key_cnt);
}
CU_SINLINE cucon_rumap_t
cucon_rumap_mref_by_int_arr(cucon_rumap_t rumap,
			  int *key_arr, size_t key_cnt)
{
    return cuconP_APPLY(cuconP_RUMAP_MREF_ARR, CUCONF_WIDTHOF_INT,
		      rumap, key_arr, key_cnt);
}
CU_SINLINE cucon_rumap_t
cucon_rumap_mref_by_uint_arr(cucon_rumap_t rumap,
			   unsigned int *key_arr, size_t key_cnt)
{
    return cuconP_APPLY(cuconP_RUMAP_MREF_ARR, CUCONF_WIDTHOF_INT,
		      rumap, key_arr, key_cnt);
}
#if 0 /* the pmap only supports uintptr_t */
CU_SINLINE cucon_rumap_t
cucon_rumap_mref_by_long_arr(cucon_rumap_t rumap,
			   long *key_arr, size_t key_cnt)
{
    return cuconP_APPLY(cuconP_RUMAP_MREF_ARR, CUCONF_WIDTHOF_LONG,
		      rumap, key_arr, key_cnt);
}
CU_SINLINE cucon_rumap_t
cucon_rumap_mref_by_ulong_arr(cucon_rumap_t rumap,
			    unsigned long *key_arr, size_t key_cnt)
{
    return cuconP_APPLY(cuconP_RUMAP_MREF_ARR, CUCONF_WIDTHOF_LONG,
		      rumap, key_arr, key_cnt);
}
#endif

CU_SINLINE cucon_rumap_t
cucon_rumap_mref_by_char_arr(cucon_rumap_t rumap,
			   char *key_arr, size_t key_cnt)
{
    return cuconP_APPLY(cuconP_RUMAP_MREF_ARR, CUCONF_WIDTHOF_CHAR,
		      rumap, key_arr, key_cnt);
}
CU_SINLINE cucon_rumap_t
cucon_rumap_mref_by_wchar_arr(cucon_rumap_t rumap,
			    wchar_t *key_arr, size_t key_cnt)
{
    return cuconP_APPLY(cuconP_RUMAP_MREF_ARR, CUCONF_WIDTHOF_WCHAR_T,
		      rumap, key_arr, key_cnt);
}
#undef cuconP_APPLY
#undef cuconP_RUMAP_MREF_ARR


/* Iteration
 * --------- */

CU_SINLINE cu_bool_t
cucon_rpmap_conj(cucon_rpmap_t rpmap,
	       cu_clop(fn, cu_bool_t, void const *, cucon_rpmap_t))
{
    return cucon_pmap_conj_mem(&rpmap->pmap,
			       (cu_clop(, cu_bool_t, void const *, void *))fn);
}
CU_SINLINE cu_bool_t
cucon_rumap_conj(cucon_rumap_t rumap,
	       cu_clop(fn, cu_bool_t, uintptr_t, cucon_rumap_t))
{
    return cucon_umap_conj_mem(&rumap->pmap.impl,
			       (cu_clop(, cu_bool_t, uintptr_t, void *))fn);
}

/*!@}*/
CU_END_DECLARATIONS

#endif
