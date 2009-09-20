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

#include <cu/int.h>
#include <cu/size.h>
#include <cu/debug.h>

/* Tuning Options
 * -------------- */

#define USE_TABLE_FOR_BIT_COUNT 1
#define USE_TABLE_FOR_FLOOR_LOG2 1


/* Bit Count
 * --------- */

#if USE_TABLE_FOR_BIT_COUNT
static unsigned char bit_count_table[] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8,
};
#endif

unsigned int
cu_uint8_bit_count(register uint_fast8_t x)
{
#if USE_TABLE_FOR_BIT_COUNT
    return bit_count_table[x];
#else
    x = (UINT8_C(0x55) & (x >> 1)) + (UINT8_C(0x55) & x);
    x = (UINT8_C(0x33) & (x >> 2)) + (UINT8_C(0x33) & x);
    return (x >> 4) + (UINT8_C(0x0f) & x);
#endif
}

unsigned int
cu_uint16_bit_count(register uint_fast16_t x)
{
#if USE_TABLE_FOR_BIT_COUNT
    unsigned int r = bit_count_table[x & 0xff];
    x >>= 8;
    return r + bit_count_table[x];
#else
    x = (UINT16_C(0x5555) & (x >> 1)) + (UINT16_C(0x5555) & x);
    x = (UINT16_C(0x3333) & (x >> 2)) + (UINT16_C(0x3333) & x);
    x = (UINT16_C(0x0f0f) & (x >> 4)) + (UINT16_C(0x0f0f) & x);
    return (x >> 8) + (UINT16_C(0x00ff) & x);
#endif
}

unsigned int
cu_uint32_bit_count(register uint_fast32_t x)
{
#if USE_TABLE_FOR_BIT_COUNT
    unsigned int r = bit_count_table[x & 0xff];
    x >>= 8;
    r += bit_count_table[x & 0xff];
    x >>= 8;
    r += bit_count_table[x & 0xff];
    x >>= 8;
    return r + bit_count_table[x];
#else
    x = (UINT32_C(0x55555555) & (x >> 1)) + (UINT32_C(0x55555555) & x);
    x = (UINT32_C(0x33333333) & (x >> 2)) + (UINT32_C(0x33333333) & x);
    x = (UINT32_C(0x0f0f0f0f) & (x >> 4)) + (UINT32_C(0x0f0f0f0f) & x);
    x = (UINT32_C(0x00ff00ff) & (x >> 8)) + (UINT32_C(0x00ff00ff) & x);
    return (x >> 16) + (UINT32_C(0x0000ffff) & x);
#endif
}

unsigned int
cu_uint64_bit_count(register uint_fast64_t x)
{
#if USE_TABLE_FOR_BIT_COUNT
    unsigned int r = bit_count_table[x & 0xff];
    x >>= 8;
    r += bit_count_table[x & 0xff];
    x >>= 8;
    r += bit_count_table[x & 0xff];
    x >>= 8;
    r += bit_count_table[x & 0xff];
    x >>= 8;
    r += bit_count_table[x & 0xff];
    x >>= 8;
    r += bit_count_table[x & 0xff];
    x >>= 8;
    r += bit_count_table[x & 0xff];
    x >>= 8;
    return r + bit_count_table[x];
#else
    x = (UINT64_C(0x5555555555555555) & (x >> 1))
      + (UINT64_C(0x5555555555555555) & x);
    x = (UINT64_C(0x3333333333333333) & (x >> 2))
      + (UINT64_C(0x3333333333333333) & x);
    x = (UINT64_C(0x0f0f0f0f0f0f0f0f) & (x >> 4))
      + (UINT64_C(0x0f0f0f0f0f0f0f0f) & x);
    x = (UINT64_C(0x00ff00ff00ff00ff) & (x >> 8))
      + (UINT64_C(0x00ff00ff00ff00ff) & x);
    x = (UINT64_C(0x0000ffff0000ffff) & (x >> 16))
      + (UINT64_C(0x0000ffff0000ffff) & x);
    return (x >> 32) + (UINT64_C(0x00000000ffffffff) & x);
#endif
}


/* dcover
 * ------ */

uint_fast8_t
cu_uint8_dcover(uint_fast8_t x)
{
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    return x;
}

uint_fast16_t
cu_uint16_dcover(uint_fast16_t x)
{
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    return x;
}

uint_fast32_t
cu_uint32_dcover(uint_fast32_t x)
{
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x;
}

uint_fast64_t
cu_uint64_dcover(uint_fast64_t x)
{
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x |= x >> 32;
    return x;
}


/* floor(log2(x))
 * -------------- */

/* Note.  The internal versions of the floor ∘ log2 functions must return -1
 * for zero argument due to the way they are used to implement the
 * corresponding ceil ∘ log2 functions. */

#if USE_TABLE_FOR_FLOOR_LOG2
static signed char _floor_log2_table[] = {
   -1, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
};
#endif

CU_SINLINE unsigned int
_uint8_floor_log2(uint_fast8_t x)
{
#if USE_TABLE_FOR_FLOOR_LOG2
    return _floor_log2_table[x];
#elif 0
    return x >= 16
	   ? x >= 64 ? (x >> 7) + 6 : (x >> 5) + 4
	   : x >= 4  ? (x >> 3) + 2 : (x >> 1);
#elif 0
    return x >= 16
	   ? x >= 64 ? (x >= 128) + 6 : (x >= 32) + 4
	   : x >=  4 ? (x >=   8) + 2 : (x >=  2);
#elif 0
    if (x >= 16) {
	if (x >= 64)
	    return (x >> 7) + 6;
	else
	    return (x >> 5) + 4;
    }
    else {
	if (x >= 4)
	    return (x >> 3) + 2;
	else
	    return (x >> 1);
    }
#elif 0
    if (x >= 16) {
	x >>= 4;
	return (x*(157 - 7*x) >> 8) + 4;
    }
    else
	return x*(157 - 7*x) >> 8;
#else
   return (x & 0xf0)
	  ? (x & 0xc0) ? (x & 0x80) ? 7 : 6 : (x & 0x20) ? 5 : 4
	  : (x & 0x0c) ? (x & 0x08) ? 3 : 2 : (x & 0x02) ? 1 : 0;

#endif
}
CU_SINLINE unsigned int
_uint16_floor_log2(uint_fast16_t x)
{
    return (x >= 0x100)
	 ? _uint8_floor_log2(x >> 8) + 8
	 : _uint8_floor_log2(x);
}
CU_SINLINE unsigned int
_uint32_floor_log2(uint_fast32_t x)
{
    return (x >= 0x10000)
	 ? _uint16_floor_log2(x >> 16) + 16
	 : _uint16_floor_log2(x);
}
CU_SINLINE unsigned int
_uint64_floor_log2(uint_fast64_t x)
{
    return (x >= UINT64_C(0x100000000))
	 ? _uint32_floor_log2(x >> 32) + 32
	 : _uint32_floor_log2(x);
}

unsigned int
cu_uint8_floor_log2(uint_fast8_t x)
{
    cu_check_arg(0, x, x > 0);
    return _uint8_floor_log2(x);
}

unsigned int
cu_uint16_floor_log2(uint_fast16_t x)
{
    cu_check_arg(0, x, x > 0);
    return _uint16_floor_log2(x);
}

unsigned int
cu_uint32_floor_log2(uint_fast32_t x)
{
    cu_check_arg(0, x, x > 0);
    return _uint32_floor_log2(x);
}

unsigned int
cu_uint64_floor_log2(uint_fast64_t x)
{
    cu_check_arg(0, x, x > 0);
    return _uint64_floor_log2(x);
}

unsigned int
cu_uint8_ceil_log2(uint_fast8_t x)
{
    cu_check_arg(0, x, x > 0);
    return _uint8_floor_log2(x - UINT8_C(1)) + 1;
}

unsigned int
cu_uint16_ceil_log2(uint_fast16_t x)
{
    cu_check_arg(0, x, x > 0);
    return _uint16_floor_log2(x - UINT16_C(1)) + 1;
}

unsigned int
cu_uint32_ceil_log2(uint_fast32_t x)
{
    cu_check_arg(0, x, x > 0);
    return _uint32_floor_log2(x - UINT32_C(1)) + 1;
}

unsigned int
cu_uint64_ceil_log2(uint_fast32_t x)
{
    cu_check_arg(0, x, x > 0);
    return _uint64_floor_log2(x - UINT64_C(1)) + 1;
}
