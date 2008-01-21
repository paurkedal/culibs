/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2005--2007  Petter Urkedal <urkedal@nbi.dk>
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

#define CUPRIV_PRIME32A 4212831859U
#define CUPRIV_PRIME32B 2909797907U
#define CUPRIV_PRIME16A 493643U
#define CUPRIV_PRIME16B 382769U
#if CUCONF_SIZEOF_CU_HASH_T > 4
#  define CUPRIV_PRIME_HA CUPRIV_PRIME32A
#  define CUPRIV_PRIME_HB CUPRIV_PRIME32B
#else
#  define CUPRIV_PRIME_HA CUPRIV_PRIME16A
#  define CUPRIV_PRIME_HB CUPRIV_PRIME16B
#endif

CU_SINLINE cu_hash_t cu_hash_mix3(cu_hash_t key)
{ return (key ^ (key >> 7)) - ((key >> 11) ^ (key >> 19)); }

CU_SINLINE cu_hash_t cu_hash_mix4(cu_hash_t key)
{ return (key >> 7) + ((key >> 11) ^ (key >> 19)) + key*1030739; }

CU_SINLINE cu_hash_t cu_hash_mix5(cu_hash_t key)
{
    key *= CUPRIV_PRIME_HA;
    key ^= key >> sizeof(cu_hash_t)*4;
    return key;
}
CU_SINLINE cu_hash_t cu_hash_mix6(cu_hash_t key)
{
    key *= CUPRIV_PRIME_HA;
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
cu_hash_t cu_wordarr_hash_noinit_bj(size_t count, cu_word_t const *arr);
cu_hash_t cu_1word_hash_bj(cu_word_t d0, cu_hash_t init);
cu_hash_t cu_2word_hash_bj(cu_word_t d0, cu_word_t d1, cu_hash_t init);

CU_END_DECLARATIONS

#endif
