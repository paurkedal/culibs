/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2002--2007  Petter Urkedal <urkedal@nbi.dk>
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

#include <cucon/hset.h>
#include <cu/memory.h>
#include <cu/debug.h>
#include <cu/diag.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#ifndef CU_NDEBUG
#  include <stdio.h>
#endif

cu_clos_def(cucon_hset_subeq_cb,
	    cu_prot(cu_bool_t, void const *key),
	    (cucon_hset_t hs1;))
{
    cu_clos_self(cucon_hset_subeq_cb);
    return cucon_hset_contains(self->hs1, key);
}

cu_bool_t
cucon_hset_subeq(cucon_hset_t hs0, cucon_hset_t hs1)
{
    if (cucon_hset_size(hs0) > cucon_hset_size(hs1))
	return cu_false;
    else {
	cucon_hset_subeq_cb_t cb;
	cb.hs1 = hs1;
	return cucon_hset_conj(hs0, cucon_hset_subeq_cb_prep(&cb));
    }
}

cu_bool_t
cucon_hset_sub(cucon_hset_t hs0, cucon_hset_t hs1)
{
    if (cucon_hset_size(hs0) >= cucon_hset_size(hs1))
	return cu_false;
    else {
	cucon_hset_subeq_cb_t cb;
	cb.hs1 = hs1;
	return cucon_hset_conj(hs0, cucon_hset_subeq_cb_prep(&cb));
    }
}

cu_bool_t
cucon_hset_eq(cucon_hset_t hs0, cucon_hset_t hs1)
{
    if (cucon_hset_size(hs0) != cucon_hset_size(hs1))
	return cu_false;
    else {
	cucon_hset_subeq_cb_t cb;
	cb.hs1 = hs1;
	return cucon_hset_conj(hs0, cucon_hset_subeq_cb_prep(&cb));
    }
}
