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

#include <cuex/set.h>
#include <cuex/atree.h>
#include <cuoo/halloc.h>
#include <cuoo/intf.h>
#include <cuoo/type.h>

typedef struct cuex_set_s *cuex_set_t;
struct cuex_set_s
{
    CUOO_HCOBJ
    cuex_t atree;
};

#define SET(x) ((cuex_set_t)(x))


cu_clop_def(set_key, cu_word_t, cuex_t e)
{
    return (cu_word_t)e;
}

static cuex_t
set_new(cuex_t atree)
{
    cuoo_hctem_decl(cuex_set, tem);
    cuoo_hctem_init(cuex_set, tem);
    cuoo_hctem_get(cuex_set, tem)->atree = atree;
    return cuoo_hctem_new(cuex_set, tem);
}

cuex_t
cuex_set_insert(cuex_t S, cuex_t e)
{
    return set_new(cuex_atree_insert(set_key, SET(S)->atree, e));
}

cuex_t
cuex_set_erase(cuex_t S, cuex_t e)
{
    return set_new(cuex_atree_erase(set_key, SET(S)->atree, (cu_word_t)e));
}

cuex_t
cuex_set_union(cuex_t S0, cuex_t S1)
{
    return set_new(cuex_atree_left_union(set_key,
					 SET(S0)->atree, SET(S1)->atree));
}

cuex_t
cuex_set_isecn(cuex_t S0, cuex_t S1)
{
    return set_new(cuex_atree_left_isecn(set_key,
					 SET(S0)->atree, SET(S1)->atree));
}

cu_bool_t
cuex_set_contains(cuex_t S, cuex_t e)
{
    return !!cuex_atree_find(set_key, SET(S)->atree, (cu_word_t)e);
}

cu_bool_t
cuex_set_subeq(cuex_t S0, cuex_t S1)
{
    return cuex_atree_subseteq(set_key, SET(S0)->atree, SET(S1)->atree);
}

cu_order_t
cuex_set_order(cuex_t S0, cuex_t S1)
{
    return cuex_atree_order(set_key, SET(S0)->atree, SET(S1)->atree);
}

static cu_word_t
set_dispatch(cu_word_t intf_number, ...)
{
    switch (intf_number) {
	default:
	    return CUOO_IMPL_NONE;
    }
}

cuoo_stdtype_t cuexP_set_type;
cuex_t cuexP_set_empty;

void
cuexP_set_init()
{
    cuexP_set_type = cuoo_stdtype_new_hcs(
	set_dispatch, sizeof(struct cuex_set_s) - CUOO_HCOBJ_SHIFT);
    cuexP_set_empty = set_new(cuex_atree_empty());
}
