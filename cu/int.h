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

#ifndef CU_INT_H
#define CU_INT_H

#include <cu/fwd.h>
#include <cu/conf.h>
#include <cu/debug.h>
#include <stdint.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cu_int cu/int.h: Integer Functions
 * @{ \ingroup cu_type_mod */

#if CUCONF_SIZEOF_INT == 4
#  define CUP_UINT_NAME(prefix, name) prefix##uint32_##name
#elif CUCONF_SIZEOF_INT == 8
#  define CUP_UINT_NAME(prefix, name) prefix##uint64_##name
#else
#  error Unexpected CUCONF_SIZEOF_INT
#endif

#if CUCONF_SIZEOF_LONG == 4
#  define CUP_ULONG_NAME(prefix, name) prefix##uint32_##name
#elif CUCONF_SIZEOF_LONG == 8
#  define CUP_ULONG_NAME(prefix, name) prefix##uint64_##name
#else
#  error Unexpected CUCONF_SIZEOF_LONG
#endif

#if CUCONF_SIZEOF_INTPTR_T == 4
#  define CUP_UINTPTR_NAME(prefix, name) prefix##uint32_##name
#elif CUCONF_SIZEOF_INTPTR_T == 8
#  define CUP_UINTPTR_NAME(prefix, name) prefix##uint64_##name
#else
#  error Unexpected CUCONF_SIZEOF_UINTPTR_T
#endif

#if CU_WORD_SIZE == 4
#  define CUP_WORD_NAME(prefix, name) prefix##uint32_##name
#elif CU_WORD_SIZE == 8
#  define CUP_WORD_NAME(prefix, name) prefix##uint64_##name
#else
#  error Unexpected CU_WORD_SIZE
#endif

/*!The larger or \a x and \a y. */
CU_SINLINE int cu_int_max(int x, int y) { return x > y? x : y; }

/*!The smaller of \a x and \a y. */
CU_SINLINE int cu_int_min(int x, int y) { return x < y? x : y; }

/*!The absolute value af \a x. */
CU_SINLINE int cu_int_abs(int x) { return x >= 0? x : -x; }

/*!The larger of \a x and \a y. */
CU_SINLINE unsigned int cu_uint_max(unsigned int x, unsigned int y)
{ return x > y? x : y; }

/*!The smaller of \a x and \a y. */
CU_SINLINE unsigned int cu_uint_min(unsigned int x, unsigned int y)
{ return x < y? x : y; }

/*!The larger or \a x and \a y. */
CU_SINLINE long cu_long_max(long x, long y) { return x > y? x : y; }

/*!The smaller of \a x and \a y. */
CU_SINLINE long cu_long_min(long x, long y) { return x < y? x : y; }

/*!The absolute value af \a x. */
CU_SINLINE long cu_long_abs(long x) { return x >= 0? x : -x; }

/*!The larger of \a x and \a y. */
CU_SINLINE unsigned long cu_ulong_max(unsigned long x, unsigned long y)
{ return x > y? x : y; }

/*!The smaller of \a x and \a y. */
CU_SINLINE unsigned long cu_ulong_min(unsigned long x, unsigned long y)
{ return x < y? x : y; }

/*!Returns ⌈\a x/\a y⌉. */
CU_SINLINE unsigned int cu_uint_ceil_div(unsigned int x, unsigned int y)
{ return (x + y - 1)/y; }

/*!Returns ⌈\a x/\a y⌉. */
CU_SINLINE unsigned long cu_ulong_ceil_div(unsigned long x, unsigned long y)
{ return (x + y - 1)/y; }

/*!Returns a bitmask from the uppermost non-zero bit in \a x and downwards. */
uint_fast8_t cu_uint8_dcover(uint_fast8_t x) CU_ATTR_PURE;

/*!\copydoc cu_uint8_dcover */
uint_fast16_t cu_uint16_dcover(uint_fast16_t x) CU_ATTR_PURE;

/*!\copydoc cu_uint8_dcover */
uint_fast32_t cu_uint32_dcover(uint_fast32_t x) CU_ATTR_PURE;

/*!\copydoc cu_uint8_dcover */
uint_fast64_t cu_uint64_dcover(uint_fast64_t x) CU_ATTR_PURE;

/*!\copydoc cu_uint8_dcover */
CU_SINLINE unsigned int cu_uint_dcover(unsigned int x)
{ return CUP_UINT_NAME(cu_,dcover)(x); }

/*!\copydoc cu_uint8_dcover */
CU_SINLINE unsigned long cu_ulong_dcover(unsigned long x)
{ return CUP_ULONG_NAME(cu_,dcover)(x); }

/*!\copydoc cu_uint8_dcover */
CU_SINLINE uintptr_t cu_uintptr_dcover(uintptr_t x)
{ return CUP_UINTPTR_NAME(cu_,dcover)(x); }

/*!\copydoc cu_uint8_dcover */
CU_SINLINE cu_word_t cu_word_dcover(cu_word_t x)
{ return CUP_WORD_NAME(cu_,dcover)(x); }

/*!Returns a bitmask from the lowermost non-zero bit in \a x and upwards. */
CU_SINLINE unsigned int
cu_uint_ucover(unsigned int x) { return x | ~(x - 1); }

/*!\copydoc cu_uint_ucover */
CU_SINLINE unsigned long
cu_ulong_ucover(unsigned long x) { return x | ~(x - 1L); }

/*!\copydoc cu_uint_ucover */
CU_SINLINE cu_word_t
cu_word_ucover(unsigned long x) { return x | ~(x - CU_WORD_C(1)); }


/*!A bitmask which covers the lowermost continuous zeros of \a x. */
CU_SINLINE unsigned int
cu_uint_lzeros(unsigned int x) { return ~x & (x - 1); }

/*!\copydoc cu_uint_lzeros */
CU_SINLINE unsigned long
cu_ulong_lzeros(unsigned long x) { return ~x & (x - 1L); }

/*!A bitmask which covers the uppermost continuous zeroes of \a x. */
CU_SINLINE unsigned int
cu_uint_uzeros(unsigned int x) { return ~cu_uint_dcover(x); }

/*!\copydoc cu_uint_uzeros */
CU_SINLINE unsigned long
cu_ulong_uzeros(unsigned long x) { return ~cu_ulong_dcover(x); }


/*!Returns 2<sup>⌈log<sub>2</sub> <i>x</i>⌉</sup>. */
CU_SINLINE uint_fast8_t cu_uint8_exp2_ceil_log2(uint_fast8_t x)
{ return cu_uint8_dcover(x - 1) + 1; }

/*!\copydoc cu_uint8_exp2_ceil_log2 */
CU_SINLINE uint_fast16_t cu_uint16_exp2_ceil_log2(uint_fast16_t x)
{ return cu_uint16_dcover(x - 1) + 1; }

/*!\copydoc cu_uint8_exp2_ceil_log2 */
CU_SINLINE uint_fast32_t cu_uint32_exp2_ceil_log2(uint_fast32_t x)
{ return cu_uint32_dcover(x - UINT32_C(1)) + UINT32_C(1); }

/*!\copydoc cu_uint8_exp2_ceil_log2 */
CU_SINLINE uint_fast64_t cu_uint64_exp2_ceil_log2(uint_fast64_t x)
{ return cu_uint64_dcover(x - UINT64_C(1)) + UINT64_C(1); }

/*!\copydoc cu_uint8_exp2_ceil_log2 */
CU_SINLINE unsigned int
cu_uint_exp2_ceil_log2(unsigned int x)
{ return cu_uint_dcover(x - 1) + 1; }

/*!\copydoc cu_uint8_exp2_ceil_log2 */
CU_SINLINE unsigned long
cu_ulong_exp2_ceil_log2(unsigned long x)
{ return cu_ulong_dcover(x - 1L) + 1L; }

/*!\copydoc cu_uint8_exp2_ceil_log2 */
CU_SINLINE cu_word_t
cu_word_exp2_ceil_log2(cu_word_t x)
{ return cu_word_dcover(x - CU_WORD_C(1)) + CU_WORD_C(1); }


/*!Returns the number of high bits in \a x. */
unsigned int cu_uint8_bit_count(uint_fast8_t x) CU_ATTR_PURE;

/*!\copydoc cu_uint8_bit_count */
unsigned int cu_uint16_bit_count(uint_fast16_t x) CU_ATTR_PURE;

/*!\copydoc cu_uint8_bit_count */
unsigned int cu_uint32_bit_count(uint_fast32_t x) CU_ATTR_PURE;

/*!\copydoc cu_uint8_bit_count */
unsigned int cu_uint64_bit_count(uint_fast64_t x) CU_ATTR_PURE;

/*!\copydoc cu_uint8_bit_count */
CU_SINLINE unsigned int cu_uint_bit_count(unsigned int x)
{ return CUP_UINT_NAME(cu_,bit_count)(x); }

/*!\copydoc cu_uint8_bit_count */
CU_SINLINE unsigned long cu_ulong_bit_count(unsigned long x)
{ return CUP_ULONG_NAME(cu_,bit_count)(x); }

/*!\copydoc cu_uint8_bit_count */
CU_SINLINE cu_word_t cu_word_bit_count(cu_word_t x)
{ return CUP_WORD_NAME(cu_,bit_count)(x); }


/*!Returns the exponent of the uppermost non-zero bit in \a x, that is
 * ⌊log<sub>2</sub> <i>x</i>⌋. */
unsigned int cu_uint8_floor_log2(uint_fast8_t x) CU_ATTR_PURE;
/*!\copydoc cu_uint8_floor_log2*/
unsigned int cu_uint16_floor_log2(uint_fast16_t x) CU_ATTR_PURE;
/*!\copydoc cu_uint8_floor_log2*/
unsigned int cu_uint32_floor_log2(uint_fast32_t x) CU_ATTR_PURE;
/*!\copydoc cu_uint8_floor_log2*/
unsigned int cu_uint64_floor_log2(uint_fast64_t x) CU_ATTR_PURE;
/*!\copydoc cu_uint8_floor_log2*/
unsigned int cu_uint_floor_log2(unsigned int x) CU_ATTR_PURE;
/*!\copydoc cu_uint8_floor_log2*/
unsigned int cu_ulong_floor_log2(unsigned long x) CU_ATTR_PURE;
/*!\copydoc cu_uint8_floor_log2*/
CU_SINLINE unsigned int cu_word_floor_log2(cu_word_t x)
{ return CUP_WORD_NAME(cu_,floor_log2)(x); }

/*!Returns the exponent of the lowermost non-zero bit in \a x. */
CU_SINLINE unsigned int
cu_uint8_log2_lowbit(uint_fast8_t x)
{
    cu_debug_assert(x);
    return cu_uint8_bit_count(~x & (x - 1));
}
/*!\copydoc cu_uint8_log2_lowbit*/
CU_SINLINE unsigned int
cu_uint16_log2_lowbit(uint_fast16_t x)
{
    cu_debug_assert(x);
    return cu_uint16_bit_count(~x & (x - 1));
}
/*!\copydoc cu_uint8_log2_lowbit*/
CU_SINLINE unsigned int
cu_uint32_log2_lowbit(uint_fast32_t x)
{
    cu_debug_assert(x);
    return cu_uint32_bit_count(~x & (x - 1));
}
/*!\copydoc cu_uint8_log2_lowbit*/
CU_SINLINE unsigned int
cu_uint64_log2_lowbit(uint_fast64_t x)
{
    cu_debug_assert(x);
    return cu_uint64_bit_count(~x & (x - 1));
}

/*!\copydoc cu_uint8_log2_lowbit*/
CU_SINLINE unsigned int
cu_uint_log2_lowbit(unsigned int x)
{ return CUP_UINT_NAME(cu_,log2_lowbit)(x); }

/*!\copydoc cu_uint8_log2_lowbit*/
CU_SINLINE unsigned int
cu_ulong_log2_lowbit(unsigned long x)
{ return CUP_ULONG_NAME(cu_,log2_lowbit)(x); }

/*!\copydoc cu_intptr_log2_lowbit*/
CU_SINLINE unsigned int
cu_uintptr_log2_lowbit(uintptr_t x)
{ return CUP_UINTPTR_NAME(cu_,log2_lowbit)(x); }

/*!\copydoc cu_uint8_log2_lowbit*/
CU_SINLINE unsigned int
cu_word_log2_lowbit(cu_word_t x)
{ return CUP_WORD_NAME(cu_,log2_lowbit)(x); }


/* Network Byte Order and Host Byte Order Conversion
 * ------------------------------------------------- */

#ifdef CUCONF_WORDS_BIGENDIAN

/*!Returns \a i in network byte order */
CU_SINLINE uint16_t cu_uint16_hton(uint16_t i) { return i; }
/*!\copydoc cu_uint16_hton */
CU_SINLINE uint32_t cu_uint32_hton(uint32_t i) { return i; }

#else

CU_SINLINE uint16_t cu_uint16_hton(uint16_t i)
{
    return (i >> 8) | ((i & 0xff) << 8);
}
CU_SINLINE uint32_t cu_uint32_hton(uint32_t i)
{
    return (i >> 24) | ((i & 0xff) << 24)
	| ((i & 0xff0000) >> 8) | ((i & 0xff00) << 8);
}

#endif
#define cu_uint16_ntoh cu_uint16_hton
#define cu_uint32_ntoh cu_uint32_hton

/*!@}*/
CU_END_DECLARATIONS

#endif
