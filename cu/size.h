/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CU_SIZE_H
#define CU_SIZE_H

#include <cu/fwd.h>
#include <cu/int.h>

CU_BEGIN_DECLARATIONS
/** \defgroup cu_size_h cu/size.h: Functions on Sizes and Pointer Diffs
 ** @{ \ingroup cu_type_mod */

#if CUCONF_SIZEOF_SIZE_T == 4
#  define CUP_SIZE_NAME(pre, suf) pre##uint32_##suf
#elif CUCONF_SIZEOF_SIZE_T == 8
#  define CUP_SIZE_NAME(pre, suf) pre##uint64_##suf
#else
#  error Unexpected CUCONF_SIZEOF_SIZE_T
#endif

/** Returns the greatest argument. */
CU_SINLINE size_t cu_size_min(size_t n, size_t m) { return n < m? n : m; }

/** Returns the smallest argument. */
CU_SINLINE size_t cu_size_max(size_t n, size_t m) { return n > m? n : m; }

/** Returs a number with the lowest \a b bits set, and the remaining bits
 ** cleared. */
CU_SINLINE size_t cu_size_lmask(unsigned int b)
{ return ((size_t)(1) << b) - (size_t)1; }

/** \copydoc cu_uint8_dcover */
CU_SINLINE size_t cu_size_dcover(size_t x)
{ return CUP_SIZE_NAME(cu_,dcover)(x); }

/** \copydoc cu_uint_ucover */
CU_SINLINE size_t cu_size_ucover(size_t x) { return x | ~(x - 1); }

/** Returns \f$\lceil\frac{n}{m}\rceil\f$. */
CU_SINLINE size_t cu_size_ceil_div(size_t n, size_t m)
{ return (n + m - 1)/m; }

/** Returns \f$ m \lceil \frac{n}{m} \rceil \f$,
 ** i.e. \a n rounded up to the nearest multiple of \a m. */
CU_SINLINE size_t cu_size_mulceil(size_t n, size_t m)
{ return cu_size_ceil_div(n, m)*m; }

/** Returns \f$ m \lfloor \frac{n}{m} \rfloor \f$,
 ** i.e. \a n rounded down to the nearest multiple of \a m. */
CU_SINLINE size_t cu_size_mulfloor(size_t n, size_t m)
{ return (n/m)*m; }

/** Returns \f$ 2^b\lceil 2^{-b}n \rceil\f$,
 ** i.e. \a n rounded up to the nearest multiple of \f$2^b\f$. */
CU_SINLINE size_t cu_size_scal2ceil(size_t n, unsigned int b)
{ return (n + cu_size_lmask(b)) & ~cu_size_lmask(b); }

/** Returns \f$ 2^b\lfloor 2^{-b}n \rfloor\f$,
 ** i.e. \a n rounded down to the nearest multiple of \f$2^b\f$. */
CU_SINLINE size_t cu_size_scal2floor(size_t n, unsigned int b)
{ return n & ~cu_size_lmask(b); }

/** Round up \a n to the nearest fully aligned size. */
CU_SINLINE size_t cu_size_alignceil(size_t n)
{ return cu_size_mulceil(n, CUCONF_MAXALIGN); }

/** Round down \a n to the nearest fully aligned size. */
CU_SINLINE size_t cu_size_alignfloor(size_t n)
{ return cu_size_mulfloor(n, CUCONF_MAXALIGN); }

/** Returns \f$ 2^{\lceil \mathrm{log}_2 n\rceil} \f$,
 ** i.e. the smallest power of 2 which is greater or equal to \a n. */
CU_SINLINE size_t cu_size_exp2ceil(size_t n)
{ return cu_size_dcover(n - (size_t)1) + (size_t)1; }

/** Returns \f$ 2^{\lfloor \mathrm{log}_2 n\rfloor} \f$,
 ** i.e. the greatest power of 2 which is smaller or equal to \a n. */
CU_SINLINE size_t cu_size_exp2floor(size_t n)
{ return (cu_size_dcover(n) >> 1) + (size_t)1; }

/** Returns \f$ \lfloor \mathrm{log}_2 n \rfloor \f$.
 ** \pre \a n > 0. */
CU_SINLINE unsigned int cu_size_floor_log2(size_t n)
{ return CUP_SIZE_NAME(cu_,floor_log2)(n); }

/** Returns \f$ \lceil \mathrm{log}_2 n \rceil \f$.
 ** \pre \a n > 0. */
CU_SINLINE unsigned int cu_size_ceil_log2(size_t n)
{ return CUP_SIZE_NAME(cu_,ceil_log2)(n); }

/** The exponent of the lowermost non-zero bit in \a n. */
CU_SINLINE unsigned int cu_size_log2_lowbit(size_t n)
{ return CUP_SIZE_NAME(cu_,log2_lowbit)(n); }

/** Returns the greatest argument. */
CU_SINLINE ptrdiff_t cu_ptrdiff_min(ptrdiff_t n, ptrdiff_t m)
{ return n < m? n : m; }

/** Returns the smallest argument. */
CU_SINLINE ptrdiff_t cu_ptrdiff_max(ptrdiff_t n, ptrdiff_t m)
{ return n > m? n : m; }

/** Returns the absolute value of \a n. */
CU_SINLINE ptrdiff_t cu_ptrdiff_abs(ptrdiff_t n)
{ return n >= 0? n : -n; }

/** @} */
CU_END_DECLARATIONS

#endif
