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

#ifndef CUCON_BITVECT_H
#define CUCON_BITVECT_H

#include <cucon/fwd.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cucon_bitvect cucon/bitvect.h: Compact vectors of bools/bits
 * @{\ingroup cucon_linear_mod */

typedef unsigned int cuconP_bitvect_word_t;
#define cuconP_bitvect_word_width (sizeof(cuconP_bitvect_word_t)*8)

struct cucon_bitvect_s
{
    size_t size;
    cuconP_bitvect_word_t *arr;
};

/*!Construct an uninitialised vector which can hold \a size bits. */
void cucon_bitvect_init_uninit(cucon_bitvect_t bv, size_t size);
/*!Return an uninitialised vector which can hold \a size bits. */
cucon_bitvect_t cucon_bitvect_new_uninit(size_t size);

/*!Construct an vector of \a size bits initialised to \a val. */
void cucon_bitvect_init_fill(cucon_bitvect_t bv, size_t size, cu_bool_t val);
/*!Return a vector of \a size bits initialised to \a val. */
cucon_bitvect_t cucon_bitvect_new_fill(size_t size, cu_bool_t val);

/*!Construct \a bv as a copy of \a bv_src. */
void cucon_bitvect_init_copy(cucon_bitvect_t bv, cucon_bitvect_t bv_src);
/*!Return a copy of \a bv_src. */
cucon_bitvect_t cucon_bitvect_new_copy(cucon_bitvect_t bv_src);

/*!Set bits [\a low, \a high) in \a bv to \a val. */
void cucon_bitvect_fill(cucon_bitvect_t bv, size_t low, size_t high,
			cu_bool_t val);

/*!The number of bits in \a bv. */
CU_SINLINE size_t
cucon_bitvect_size(cucon_bitvect_t bv) { return bv->size; }

/*!Set element \a i of \a bv to \a value.
 * \pre cucon_bitvect_size(\a bv) >= \a i */
CU_SINLINE void
cucon_bitvect_set_at(cucon_bitvect_t bv, size_t i, cu_bool_t value)
{
    cuconP_bitvect_word_t mask;
    mask = (cuconP_bitvect_word_t)1 << (i % cuconP_bitvect_word_width);
    if (value)
	bv->arr[i/cuconP_bitvect_word_width] |= mask;
    else
	bv->arr[i/cuconP_bitvect_word_width] &= ~mask;
}

/*!Return element \a i of \a bv.
 * \pre cucon_bitvect_size(\a bv) >= \a i */
CU_SINLINE cu_bool_t
cucon_bitvect_at(cucon_bitvect_t bv, size_t i)
{
    return !!(bv->arr[i/cuconP_bitvect_word_width] &
	      ((cuconP_bitvect_word_t)1 << (i % cuconP_bitvect_word_width)));
}

size_t cucon_bitvect_find(cucon_bitvect_t bv, size_t start, cu_bool_t value);

size_t cucon_bitvect_find2(cucon_bitvect_t bv0, cucon_bitvect_t bv1,
			   size_t start, cu_bool_t val0, cu_bool_t val1);

/*!\deprecated Use \ref cucon_bitvect_init_uninit. */
#define cucon_bitvect_cct_uninit	cucon_bitvect_init_uninit
/*!\deprecated Use \ref cucon_bitvect_init_fill. */
#define cucon_bitvect_cct_fill		cucon_bitvect_init_fill
/*!\deprecated Use \ref cucon_bitvect_init_copy. */
#define cucon_bitvect_cct_copy		cucon_bitvect_init_copy

/*!@}*/
CU_END_DECLARATIONS

#endif
