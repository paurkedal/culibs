/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2005--2010  Petter Urkedal <paurkedal@eideticdew.org>
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

#ifndef CU_HASH_H
#define CU_HASH_H

#include <cu/fwd.h>
#include <cu/conf.h>

CU_BEGIN_DECLARATIONS

#define CUP_PRIME16_A UINT16_C(0x4e05)
#define CUP_PRIME16_B UINT16_C(0xc2b3)
#define CUP_PRIME16_C UINT16_C(0x483d)
#define CUP_PRIME16_D UINT16_C(0x58a3)
#define CUP_PRIME16_E UINT16_C(0x38a7)
#define CUP_PRIME16_F UINT16_C(0xf72d)
#define CUP_PRIME16_G UINT16_C(0x4c33)
#define CUP_PRIME16_H UINT16_C(0xe903)

#define CUP_PRIME32_A UINT32_C(0x5d5692d9)
#define CUP_PRIME32_B UINT32_C(0x916c5adf)
#define CUP_PRIME32_C UINT32_C(0xc45d771d)
#define CUP_PRIME32_D UINT32_C(0x3bb19507)
#define CUP_PRIME32_E UINT32_C(0x4465360d)
#define CUP_PRIME32_F UINT32_C(0xad6ffe13)
#define CUP_PRIME32_G UINT32_C(0xfb1ab673)
#define CUP_PRIME32_H UINT32_C(0x4c56a673)

#if CUCONF_SIZEOF_CU_HASH_T > 4
#  define CUP_PRIME_HA CUP_PRIME32_A
#  define CUP_PRIME_HB CUP_PRIME32_B
#else
#  define CUP_PRIME_HA CUP_PRIME16_A
#  define CUP_PRIME_HB CUP_PRIME16_B
#endif

CU_SINLINE cu_hash_t cu_hash_mix3(cu_hash_t key)
{ return (key ^ (key >> 7)) - ((key >> 11) ^ (key >> 19)); }

CU_SINLINE cu_hash_t cu_hash_mix4(cu_hash_t key)
{ return (key >> 7) + ((key >> 11) ^ (key >> 19)) + key*1030739; }

CU_SINLINE cu_hash_t cu_hash_mix5(cu_hash_t key)
{
    key *= CUP_PRIME_HA;
    key ^= key >> sizeof(cu_hash_t)*4;
    return key;
}
CU_SINLINE cu_hash_t cu_hash_mix6(cu_hash_t key)
{
    key *= CUP_PRIME_HA;
    key ^= key >> sizeof(cu_hash_t)*4;
    key ^= key >> sizeof(cu_hash_t)*2;
    return key;
}

CU_SINLINE cu_hash_t
cu_hash_rot(cu_hash_t x, int k)
{
    return (x << k) | (x >> (8*sizeof(cu_hash_t) - k));
}

#define cu_hash_mix cu_hash_mix5

cu_hash_t cu_wordarr_hash_bj(size_t cnt, cu_word_t const *arr, cu_hash_t init);
cu_hash_t cu_1word_hash_bj(cu_word_t d0, cu_hash_t init);
cu_hash_t cu_2word_hash_bj(cu_word_t d0, cu_word_t d1, cu_hash_t init);

cu_hash_t cu_wordarr_hash_noinit_bj(size_t count, cu_word_t const *arr);
cu_hash_t cu_1word_hash_noinit_bj(cu_word_t d0);
cu_hash_t cu_2word_hash_noinit_bj(cu_word_t d0, cu_word_t d1);

cu_hash_t cu_wordarr_hash_2pm(size_t len, cu_word_t const *arr, cu_hash_t init);
cu_hash_t cu_wordarr_hash_3pm(size_t len, cu_word_t const *arr, cu_hash_t init);
extern cu_hash_t (*cu_wordarr_hash_byenv)(size_t, cu_word_t const *, cu_hash_t);
#define cu_wordarr_hash cu_wordarr_hash_byenv

CU_END_DECLARATIONS

#endif
