/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2009  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CU_WORD_H
#define CU_WORD_H

#include <cu/fwd.h>
#include <cu/bool.h>
#include <cu/int.h>

CU_BEGIN_DECLARATIONS
/** \defgroup cu_word_h cu/word.h: Functions on Machine Words
 ** @{ \ingroup cu_type_mod
 **
 ** The following is a collection of functions on unsigned integers of the
 ** native word size.  This may be useful when dealing with data which can be
 ** processed in chunks of a fixed number of bits, such as \ref
 ** cucon_bitarray_h "bit arrays".  For higher level algorithms, the \ref
 ** cu_int_h "integer" variants may be more suitable.
 **
 ** \see cu_int_h
 ** \see cu_size_h
 ** \see cu_wordarr_h
 **/

/** \copydoc cu_uint8_dcover */
CU_SINLINE cu_word_t cu_word_dcover(cu_word_t x)
{ return CUP_WORD_NAME(cu_,dcover)(x); }

/** \copydoc cu_uint_ucover */
CU_SINLINE cu_word_t
cu_word_ucover(unsigned long x) { return x | ~(x - CU_WORD_C(1)); }

/** \copydoc cu_uint8_exp2_ceil_log2 */
CU_SINLINE cu_word_t
cu_word_exp2_ceil_log2(cu_word_t x)
{ return cu_word_dcover(x - CU_WORD_C(1)) + CU_WORD_C(1); }

/** \copydoc cu_uint8_bit_count */
CU_SINLINE cu_word_t cu_word_bit_count(cu_word_t x)
{ return CUP_WORD_NAME(cu_,bit_count)(x); }

/** \copydoc cu_uint8_floor_log2*/
CU_SINLINE unsigned int cu_word_floor_log2(cu_word_t x)
{ return CUP_WORD_NAME(cu_,floor_log2)(x); }

/** \copydoc cu_uint8_log2_lowbit*/
CU_SINLINE unsigned int
cu_word_log2_lowbit(cu_word_t x)
{ return CUP_WORD_NAME(cu_,log2_lowbit)(x); }

/** Returns -1, 0, or 1 if after reversing the bits \a w0 is less, equal, or
 ** greater than \a w1, respectively. */
CU_SINLINE int
cu_word_rcmp(cu_word_t w0, cu_word_t w1)
{
    cu_word_t d = w0 ^ w1;
    cu_word_t m = (d - 1) ^ d;
    w0 &= m; w1 &= m;
    return w0 < w1 ? -1 : w0 > w1 ? 1 : 0;
}

/** The bitwise image of \a x under \a f.  That is, the word where bit number
 ** \e i is the result of applying \a f to bit number \e i of \a x. */
CU_SINLINE cu_word_t
cu_word_bitimg(cu_bool1f_t f, cu_word_t x)
{
    return f & 1 ? (f & 2 ? ~CU_WORD_C(0) : ~x)
		 : (f & 2 ? x : CU_WORD_C(0));
}

/** The image under \a f of pairs of bits drawn from respective positions of \a
 ** x and \a y. */
cu_word_t cu_word_bitimg2(cu_bool2f_t f, cu_word_t x, cu_word_t y);

/** @} */
CU_END_DECLARATIONS

#endif
