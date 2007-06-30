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

#include <cuex/algo.h>
#include <cuex/var.h>
#include <cuex/ex.h>
#include <cuex/oprdefs.h>
#include <cudyn/misc.h>
#include <cu/test.h>

cu_clop_def(is_const, cu_bool_t, cuex_t e)
{
    cu_verbf(0, "Test %! gives %d", e, !cuex_is_var(e));
    return !cuex_is_var(e);
}

cu_clop_def(show_tr, cuex_t, cuex_t e)
{
    cu_verbf(0, "Transforming %!", e);
    return cuex_o1_ptr_to(e);
}

void
test()
{
    cuex_t e =
	cuex_o2_gexpt(
		cuex_o2_gprod(
			cuex_o2_gprod(cudyn_int(0), cudyn_int(1)),
			cudyn_int(2)),
		cuex_o2_gprod(cudyn_int(1), cuex_var_new_e()));
    cuex_t ep = cuex_maxtrees_of_leaftest_tran(e, is_const, show_tr);
    cu_verbf(0, "Result: %!", ep);
}

cu_clos_def(msg_cb,
	    cu_prot(cuex_t, cuex_t *arg_arr),
    (int i;))
{
    cu_clos_self(msg_cb);
    return cuex_ivar(cuex_qcode_u, self->i++);
}
cu_clos_def(msg_cb2,
	    cu_prot(cuex_t, cuex_t e0, cuex_t e1),
    (int i;))
{
    cu_clos_self(msg_cb2);
    return cuex_ivar(cuex_qcode_u, self->i++);
}

void
test_msg()
{
    cuex_t e[2];
    cuex_t ep;
    msg_cb_t cb;
    msg_cb2_t cb2;
    e[0] = cuex_o2_gprod(
		   cuex_o2_gexpt(cudyn_int(1), cudyn_int(2)),
		   cuex_o2_gexpt(cuex_var_new_e(), cudyn_int(3)));
    e[1] = cuex_o2_gprod(
		   cuex_o2_gexpt(
			   cuex_o1_singleton(cudyn_int(4)),
			   cudyn_int(2)),
		   cuex_var_new_e());
    cb.i = 0;
    ep = cuex_msg_unify_by_arr(2, e, msg_cb_prep(&cb));
    cu_verbf(0, "MSG(⟦%!⟧, ⟦%!⟧) = ⟦%!⟧", e[0], e[1], ep);
    cb2.i = 0;
    cu_test_assert(ep == cuex_msg_unify(e[0], e[1], msg_cb2_prep(&cb2)));
}

int
main()
{
    cuex_init();
    test();
    test_msg();
    return 0;
}
