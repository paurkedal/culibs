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

#include <cuex/monoid.h>
#include <cuex/oprdefs.h>
#include <cuex/compound.h>
#include <cuex/intf.h>
#include <cuex/test.h>
#include <cudyn/misc.h>
#include <cu/ptr_seq.h>

#define OPR CUEX_O2_TUPLE

#define N 500

int
main()
{
    cuex_t x;
    int i;
    int j;
    cuex_monoid_itr_t it;
    cuex_init();

    /* Check simple construction and iteration */
    cuex_monoid_itr_init_full(OPR, &it, cudyn_int(0));
    cuex_test_assert_eq(cuex_monoid_itr_get(&it), cudyn_int(0));
    cuex_test_assert_eq(cuex_monoid_itr_get(&it), NULL);
    x = cuex_monoid_product(OPR, cudyn_int(0), cudyn_int(1));
    cuex_monoid_itr_init_full(OPR, &it, x);
    cuex_test_assert_eq(cuex_monoid_itr_get(&it), cudyn_int(0));
    cuex_test_assert_eq(cuex_monoid_itr_get(&it), cudyn_int(1));
    cuex_test_assert_eq(cuex_monoid_itr_get(&it), NULL);

    /* Check construction and iteration */
    x = cuex_monoid_identity(OPR);
    cu_test_assert(cuex_is_monoid_identity(OPR, x));
    cu_test_assert(cuex_is_monoid(OPR, x));
    x = cuex_monoid_product(OPR, x, cudyn_int(0));
    cu_test_assert(!cuex_is_monoid_product(OPR, x));
    cuex_test_assert_eq(x, cudyn_int(0));
    for (i = 1; i < N; ++i) {
	x = cuex_monoid_product(OPR, x, cudyn_int(i));
	cu_test_assert(cuex_is_monoid_product(OPR, x));
	cu_test_assert_int_eq(cuex_monoid_factor_count(OPR, x), i + 1);
	cuex_monoid_itr_init_full(OPR, &it, x);
	for (j = 0; j <= i; ++j) {
	    cuex_monoid_itr_t it_r;
	    cuex_test_assert_eq(cuex_monoid_itr_get(&it), cudyn_int(j));
	    cuex_test_assert_eq(cuex_monoid_factor_at(OPR, x, j), cudyn_int(j));
	    cuex_monoid_itr_init_slice(OPR, &it_r, x, j, SIZE_MAX);
	    cuex_test_assert_eq(cuex_monoid_itr_get(&it_r), cudyn_int(j));
	}
	cuex_test_assert_eq(cuex_monoid_itr_get(&it), NULL);
    }

    /* Check associativity */
    for (j = 0; j < N; ++j) {
	cuex_t y = cuex_monoid_identity(OPR);
	cuex_t z = cuex_monoid_identity(OPR);
	for (i = 0; i < j; ++i)
	    y = cuex_monoid_product(OPR, y, cudyn_int(i));
	for (i = j; i < N; ++i)
	    z = cuex_monoid_product(OPR, z, cudyn_int(i));
	cuex_test_assert_eq(x, cuex_monoid_product(OPR, y, z));
	cuex_test_assert_eq(cuex_monoid_factor_slice(OPR, x, 0, j), y);
	cuex_test_assert_eq(cuex_monoid_factor_slice(OPR, x, j, N), z);
    }

    for (j = 0; j < N; ++j) {
	cuex_t identity = cuex_monoid_identity(OPR);
	cuex_t x = cuex_monoid_identity(OPR);
	cuex_t y;
	cuex_intf_compound_t impl;
	cu_ptr_sinktor_t y_sinktor;
	cu_ptr_junctor_t z_junctor;
	impl = cuoo_type_impl_ptr(cuex_monoid_type(), CUEX_INTF_COMPOUND);
	y_sinktor = impl->ncomm_build_sinktor(impl, identity);
	for (i = 0; i < j; ++i) {
	    cuex_t ei = cudyn_int(i);
	    x = cuex_monoid_product(OPR, x, ei);
	    cu_ptr_sinktor_put(y_sinktor, ei);
	}
	y = cu_ptr_sinktor_finish(y_sinktor);
	cuex_test_assert_eq(x, y);
	if (j != 1) {
	    z_junctor = cuex_compound_pref_image_junctor(impl, x);
	    cu_ptr_junction_short(cu_to(cu_ptr_junction, z_junctor));
	    cuex_test_assert_eq(x, cu_ptr_junctor_finish(z_junctor));
	}
    }

    return 0;
}
