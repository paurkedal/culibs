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

#include <cuex/var.h>
#include <cuex/pvar.h>
#include <cu/debug.h>

int
main()
{
    int i, j;
    cuex_qcode_t qc_arr[] = {
	cuex_qcode_e, cuex_qcode_u, cuex_qcode_w, cuex_qcode_n
    };
    cuex_meta_t meta;
    cuex_pvar_t pv;
    cuex_var_t v;
    cuex_init();

    for (i = 0; i < sizeof(qc_arr)/sizeof(qc_arr[0]); ++i) {
	cuex_qcode_t qc = qc_arr[i];

	pv = cuex_pvar_new(qc);
	meta = cuex_meta(pv);
	cu_debug_assert(cuex_is_varmeta(meta));
	cu_debug_assert(cuex_is_varmeta_q(meta, qc));
	cu_debug_assert(cuex_meta_is_pvarmeta(meta));
	cu_debug_assert(cuex_meta_is_pvarmeta_q(meta, qc));

	v = cuex_var_new(qc);
	meta = cuex_meta(v);
	cu_debug_assert(cuex_is_varmeta(meta));
	cu_debug_assert(cuex_is_varmeta_q(meta, qc));
	cu_debug_assert(!cuex_meta_is_pvarmeta(meta));
    }

    for (i = 0; i < 100; ++i) {
	cuex_var_t ivar = cuex_ivar_e(i);
	cuex_var_t rvar = cuex_rvar(i);
	cu_debug_assert(ivar != rvar);
	cu_debug_assert(ivar != cuex_ivar_u(i));
	for (j = 0; j < 100; ++j) {
	    cu_debug_assert((ivar == cuex_ivar_e(j)) == (i == j));
	    cu_debug_assert((rvar == cuex_rvar(j)) == (i == j));
	}
    }

    return 0;
}
