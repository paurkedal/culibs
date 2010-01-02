/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2005--2009  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CU_WORDARR_H
#define CU_WORDARR_H

#include <cu/fwd.h>
#include <cu/bool.h>
#include <cu/hash.h>

CU_BEGIN_DECLARATIONS

/** True iff the arrays \a arr0 and \a arr1 of \a count words contain the same
 ** values. */
CU_SINLINE cu_bool_t
cu_wordarr_eq(size_t count, cu_word_t const *arr0, cu_word_t const *arr1)
{
    while (count--)
	if (*arr0++ != *arr1++)
	    return cu_false;
    return cu_true;
}

/** Initialise \a count words starting at \a dst to \a val. */
CU_SINLINE void
cu_wordarr_fill(size_t count, cu_word_t *dst, cu_word_t val)
{
    while (count--)
	*dst++ = val;
}

/** Copy \a src upto \a src + count to \a dst upto \a dst + \a count. */
CU_SINLINE void
cu_wordarr_copy(size_t count, cu_word_t *dst, cu_word_t const *src)
{
    while (count--) {
	*dst = *src;
	++dst, ++src;
    }
}

/** Initialise \a dst upto \a dst + \a count as the bitwise negation of the
 ** corresponding bits of words from \a src upto \a src + \a count. */
CU_SINLINE void
cu_wordarr_copy_bitnot(size_t count, cu_word_t *dst, cu_word_t const *src)
{
    while (count--) {
	*dst = ~*src;
	++dst, ++src;
    }
}

/** Store to \a dst upto \a dst + \a count the bitwise image of \a src upwards
 ** under \a f. */
void cu_wordarr_copy_bitimg(cu_bool1f_t f, size_t count, cu_word_t *dst,
			    cu_word_t const *src);

/** Store to \a dst upto \a dst + \a count the bitwise image under \a f of
 ** the corresponding pairs of bits taken from \a src0 and \a src1 at the same
 ** indices. */
void cu_wordarr_copy_bitimg2(cu_bool2f_t f, size_t count, cu_word_t *dst,
			     cu_word_t const *src0, cu_word_t const *src1);

/** Bitwise copy \a src from bit number \a src_offset into \a dst (from bit 0)
 ** upto \a dst + \a count.  The arguments are interpreted as an array of bits
 ** where bit number \e n is stored in bit number <code>\e n %
 ** CU_WORD_WIDTH</code> of word number <code>\e n / CU_WORD_WIDTH</code>. */
void cu_wordarr_skewcopy(size_t count, cu_word_t *dst,
			 int src_offset, cu_word_t const *src);

/** Store the bitwise negation of words of the array \a src from bit number \a
 ** src_offset into \a dst upto \a dst + \a count. */
void cu_wordarr_skewcopy_bitnot(size_t count, cu_word_t *dst,
				int src_offset, cu_word_t const *src);

/** Store the bitwise image under \a f of \a src from bit number \a src_offset
 ** into \a dst upto \a dst + \a count. */
void cu_wordarr_skewcopy_bitimg(cu_bool1f_t f, size_t count, cu_word_t *dst,
				int src_offset, cu_word_t const *src);

/** Store to \a dst upto \a dst + \a count the bitwise image under \a f of the
 ** pairs of corresponding bits drawn from \a src0 starting at bit \a offset0
 ** and \a src1 starting at bit \a offset1. */
void cu_wordarr_skewcopy_bitimg2(cu_bool2f_t f, size_t count, cu_word_t *dst,
				 int offset0, cu_word_t const *src0,
				 int offset1, cu_word_t const *src1);

#define cu_wordarr_hash cu_wordarr_hash_bj

CU_END_DECLARATIONS

#endif
