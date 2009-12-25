/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2004--2009  Petter Urkedal <urkedal@nbi.dk>
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

#include <cucon/umap.h>
#include <cu/conf.h>
#include <cu/wchar.h>

CU_BEGIN_DECLARATIONS
/** \defgroup cucon_rumap_h cucon/rumap.h: Recursive Hash Map with Integer Keys
 ** @{\ingroup cucon_maps_and_sets_mod */

struct cucon_rumap
{
    struct cucon_umap branches;
    void *data;
};

/** Initialise \a rmap as a leaf with no value. */
void cucon_rumap_init(cucon_rumap_t rmap);

/** Return a valueless leaf. */
cucon_rumap_t cucon_rumap_new(void);

/** Swap the values and subtrees of \a map0 and \a map1. */
void cucon_rumap_swap(cucon_rumap_t map0, cucon_rumap_t map1);

/** The value at \a rmap. */
CU_SINLINE void *
cucon_rumap_value(cucon_rumap_t rmap)
{
    return rmap->data;
}

/** Assign \a ptr as the value of \a rmap. */
CU_SINLINE void
cucon_rumap_set_value(cucon_rumap_t rmap, void *ptr)
{
    rmap->data = ptr;
}

/** True iff \a rmap has no branches. */
CU_SINLINE cu_bool_t
cucon_rumap_is_leaf(cucon_rumap_t rmap)
{
    return cucon_umap_is_empty(&rmap->branches);
}

/** Mutably dereference \a rmap at \a key, constructing a new leaf if \a key
 ** does not already have a binding. */
cucon_rumap_t cucon_rumap_mref(cucon_rumap_t rmap, uintptr_t key);

/** Return the branch of \a rmap at \a key or \c NULL if \a key is unbound. */
CU_SINLINE cucon_rumap_t
cucon_rumap_cref(cucon_rumap_t rmap, uintptr_t key)
{
    return (cucon_rumap_t)cucon_umap_find_mem(&rmap->branches, key);
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

#define cuconP_APPLY(macro, ...) macro(__VA_ARGS__)
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

size_t cucon_rumap_assign_left_union(cucon_rumap_t dst, cucon_rumap_t src);
size_t cucon_rumap_assign_right_union(cucon_rumap_t dst, cucon_rumap_t src);

/* Iteration
 * --------- */

CU_SINLINE cu_bool_t
cucon_rumap_conj(cucon_rumap_t rmap,
		 cu_clop(fn, cu_bool_t, uintptr_t, cucon_rumap_t))
{
    return cucon_umap_conj_mem(&rmap->branches,
			       (cu_clop(, cu_bool_t, uintptr_t, void *))fn);
}

/** @} */
CU_END_DECLARATIONS

#endif
