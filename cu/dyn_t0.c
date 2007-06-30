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

#include <cu/dyn.h>
#define DIM 10000

cuex_meta_t opr0;
cudyn_propkey_t k0, k1;

void
test_prop()
{
    cuex_t e0;
    k0 = cudyn_propkey_create();
    opr0 = cuex_opr(10, 0);
    e0 = cuex_opn(opr0);
    cudyn_prop_set(e0, k0, &k0);
    cu_debug_assert(cudyn_prop_get(e0, k0) == &k0);
    k1 = cudyn_propkey_create();
    cudyn_prop_set(e0, k1, &k1);
    cu_debug_assert(cudyn_prop_get(e0, k0) == &k0);
    cu_debug_assert(cudyn_prop_get(e0, k1) == &k1);
    GC_gcollect();
    cu_debug_assert(cudyn_prop_get(e0, k0) == &k0);
    cu_debug_assert(cudyn_prop_get(e0, k1) == &k1);
    e0 = cuex_opn(opr0);
    cu_debug_assert(cudyn_prop_get(e0, k0) == &k0);
    cu_debug_assert(cudyn_prop_get(e0, k1) == &k1);
    e0 = NULL;
}

void
set_some_props()
{
    int i;
    for (i = 1; i <= DIM; ++i)
	cudyn_prop_set(cuex_opn(cuex_opr(i, 0)), k0, (void *)i);
}

int
check_some_props()
{
    int i;
    int n_props = 0;
    void *prop;
    for (i = 1; i <= DIM; ++i) {
	prop = cudyn_prop_get(cuex_opn(cuex_opr(i, 0)), k0);
	if (prop) {
	    cu_debug_assert((void *)i == prop);
	    ++n_props;
	}
    }
    return n_props;
}

int main()
{
    int i;
    cu_init();
    test_prop();
    set_some_props();
    GC_gcollect();
    for (i = 1; i <= 10000; ++i)
	cuex_opn(cuex_opr(i, 0)); /* trigger reclaim */
    i = check_some_props();
    if (i > 0) {
	cu_warnf("%d of %d properties was not cleared after GC.", i, DIM);
	if (i < DIM/10)
	    cu_warnf("This is probably ok, it only needs to be "
		     "asymptotically fine.");
    }
    return 0;
}
