/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2006--2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUEX_ACI_PRIV_H
#define CUEX_ACI_PRIV_H

#include <cuex/fwd.h>
#include <cuex/aci.h>
#include <cuex/ex.h>
#include <cu/int.h>

CU_BEGIN_DECLARATIONS

#if 0
typedef uintptr_t aci_key_t;
#define ACI_KEY_MAX ((uintptr_t)-1)
#define ACI_KEY_PRIp PRIxPTR
#else
typedef cuex_t aci_key_t;
#define ACI_KEY_MAX ((void *)((uintptr_t)-1))
#define ACI_KEY_PRIp "p"
#endif

CU_SINLINE cu_bool_t key_eq(aci_key_t x, aci_key_t y) { return x == y; }
CU_SINLINE cu_bool_t key_leq(aci_key_t x, aci_key_t y) { return x <= y; }
CU_SINLINE cu_bool_t key_lt(aci_key_t x, aci_key_t y) { return x < y; }
CU_SINLINE cu_bool_t key_geq(aci_key_t x, aci_key_t y) { return x >= y; }
CU_SINLINE cu_bool_t key_gt(aci_key_t x, aci_key_t y) { return x > y; }

CU_SINLINE aci_key_t
key_min(aci_key_t x, aci_key_t y) { return x <= y? x : y; }

#define aci_left cuex_aci_left
#define aci_right cuex_aci_right

CU_SINLINE aci_key_t aci_key(cuex_opn_t e)
{ return (aci_key_t)cuex_opn_at(e, 2); }

CU_SINLINE cu_bool_t aci_is_idy(cuex_opn_t e) { return cuex_opn_r(e) == 0; }

#define aci_val_start 3
#define aci_idy(opr) cuex_opn(cuex_opr(cuex_opr_index(opr), 0))

#define key0 ((uintptr_t)key0p)
#define key1 ((uintptr_t)key1p)
CU_SINLINE cu_bool_t
aci_key_covers(aci_key_t key0p, aci_key_t key1p)
{
    uintptr_t mask0 = ~(key0 ^ (key0 - CU_UINTPTR_C(1)));
    return (key0 & mask0) == (key1 & mask0) && (key1 & ~mask0);
}
CU_SINLINE cu_bool_t
aci_key_coverseq(aci_key_t key0p, aci_key_t key1p)
{
    uintptr_t mask0 = ~(key0 ^ (key0 - CU_UINTPTR_C(1)));
    return (key0 & mask0) == (key1 & mask0);
}
CU_SINLINE aci_key_t
aci_key_clim(aci_key_t key0p, aci_key_t key1p)
{
    uintptr_t x = cu_ulong_dcover(key0 ^ key1);
    return (aci_key_t)((key0 & ~x) + (x + CU_UINTPTR_C(1))/2);
}
CU_SINLINE aci_key_t
aci_key_llim(aci_key_t key0p, aci_key_t key1p)
{
    uintptr_t x = cu_ulong_dcover(key0 ^ key1);
    return (aci_key_t)(key0 & ~x);
}
CU_SINLINE aci_key_t
aci_key_rlim(aci_key_t key0p, aci_key_t key1p)
{
    uintptr_t x = cu_ulong_dcover(key0 ^ key1);
    return (aci_key_t)((key0 & ~x) + (x + CU_UINTPTR_C(1)));
}
CU_SINLINE aci_key_t
aci_key_clim1(aci_key_t key0p)
{
    return (aci_key_t)key0;
}
CU_SINLINE aci_key_t
aci_key_llim1(aci_key_t key0p)
{
    uintptr_t mask0 = ~(key0 ^ (key0 - CU_UINTPTR_C(1)));
    return (aci_key_t)(key0 & mask0);
}
CU_SINLINE aci_key_t
aci_key_rlim1(aci_key_t key0p)
{
    uintptr_t x = (key0 ^ (key0 - CU_UINTPTR_C(1))) >> 1;
    return (aci_key_t)(key0 + (x + CU_UINTPTR_C(1)));
}
#undef key0
#undef key1


/* A key at the center of the range of keys storable under \a x. */
CU_SINLINE aci_key_t
aci_tree_clim(cuex_opn_t x)
{
    cu_debug_assert(!aci_is_idy(x));
    if (aci_is_idy(aci_left(x)))
	return aci_key_clim1(aci_key(x));
    else
	return aci_key_clim(aci_key(x), aci_key(aci_left(x)));
}

/* A key just below the minimum element storable under x.  It is also the
 * center of an immediate parent node which has x as its right node. */
CU_SINLINE aci_key_t
aci_tree_llim(cuex_opn_t x)
{
    cu_debug_assert(!aci_is_idy(x));
    if (aci_is_idy(aci_left(x)))
	return aci_key_llim1(aci_key(x));
    else
	return aci_key_llim(aci_key(x), aci_key(aci_left(x)));
}

/* A key just above the maximum element storable under x.  It is also the
 * center of an immediate parent node which has x as its left node. */
CU_SINLINE aci_key_t
aci_tree_rlim(cuex_opn_t x)
{
    cu_debug_assert(!aci_is_idy(x));
    if (aci_is_idy(aci_left(x)))
	return aci_key_rlim1(aci_key(x));
    else
	return aci_key_rlim(aci_key(x), aci_key(aci_left(x)));
}

CU_SINLINE void
aci_tree_lrlim(cuex_opn_t x, aci_key_t *llim, aci_key_t *rlim)
{
    cu_debug_assert(!aci_is_idy(x));
    if (aci_is_idy(aci_left(x))) {
	*llim = aci_key_llim1(aci_key(x));
	*rlim = aci_key_rlim1(aci_key(x));
    } else {
	*llim = aci_key_llim(aci_key(x), aci_key(aci_left(x)));
	*rlim = aci_key_rlim(aci_key(x), aci_key(aci_left(x)));
    }
}

CU_END_DECLARATIONS

#endif
