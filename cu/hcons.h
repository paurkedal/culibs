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

#ifndef CU_HCONS_H
#define CU_HCONS_H

#include <cu/fwd.h>
#include <cu/conf.h>
#include <cu/wordarr.h>
#ifdef CUCONF_ENABLE_GC_DISCLAIM
#  include <cu/hcons_rn.h>
#else
#  include <cu/hcons_wp.h>
#endif

CU_BEGIN_DECLARATIONS

#if 0
CU_SINLINE cu_hash_t
cu_hc_key_hash(cu_word_t *src, int cnt, cu_hash_t h)
{
#if 1
    if (cnt)
	do {
	    h *= 7;
	    h ^= *src++;
	} while (--cnt);
#else
    if (cnt) {
	--cnt;
	if (cnt) {
	    do {
		h *= 7;
		h ^= *src++;
		h *= 7;
		h ^= *src++;
		cnt -= 2;
	    } while (cnt > 0);
	}
	if (cnt == 0) {
	    h *= 7;
	    h ^= *src++;
	}
    }
#endif
    h *= CUPRIV_PRIME_HA;
    h ^= h >> sizeof(cu_hash_t)*4;
    return h;
}
#else
#define cu_hc_key_hash cu_wordarr_hash
#endif

CU_SINLINE cu_hash_t
cu_hash_ptr_arr(void **start, void **end)
{
    cu_hash_t hash = 0;
    while (start != end) {
	hash = cu_hash_mix(hash + (uintptr_t)*start);
	++start;
    }
    return hash;
}

CU_SINLINE cu_bool_t
cu_ptr_arr_eq(void **start0, void **end0, void **start1)
{
    while (start0 != end0) {
	if (*start0 != *start1)
	    return cu_false;
	++start0;
	++start1;
    }
    return cu_true;
}

CU_END_DECLARATIONS

#endif
