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

#include <cuex/binding.h>
#include <cuex/var.h>
#include <cuex/oprdefs.h>
#include <cucon/pmap.h>
#include <cufo/stream.h>

cu_clop_def(show_indexing_cb, void, void const *e, void *slot)
{
    cufo_oprintf("    %d: %!\n", *(int *)slot, e);
}

void test(cuex_t e)
{
    cucon_pmap_t indexing = cuex_bi_sifi_indexing(e, CUEX_BI_SIFI_FLAG_PRUNE);
    cucon_pmap_iter_mem(indexing, show_indexing_cb);
}

#define qL cuex_o1_lambda
#define qM cuex_o1_mu
#define apply cuex_o2_apply
#define gprod cuex_o2_gprod
#define sgt cuex_o1_singleton
#define hole cuex_hole
#define _0 cuex_hole(0)
#define _1 cuex_hole(1)
#define _2 cuex_hole(2)
#define _3 cuex_hole(3)

int main()
{
    cuex_t e1, e2, e3;

    cuex_init();
    e1 = cuex_var_new_e();
    e2 = cuex_var_new_e();
    e3 = cuex_var_new_e();

    cufo_puts(cufo_stdout, "----\n");
    test(cuex_o1_lambda(cuex_o2_apply(cuex_o1_lambda(_1), _0)));
    cufo_puts(cufo_stdout, "----\n");
    test(cuex_o1_lambda(cuex_o2_apply(cuex_o1_lambda(_0), _0)));
    cufo_puts(cufo_stdout, "----\n");
    test(cuex_o1_lambda(cuex_o1_lambda(cuex_o1_lambda(_2))));

    cufo_puts(cufo_stdout, "----\n");
    test(cuex_o1_lambda(cuex_o1_lambda(cuex_o2_apply(_1, _0))));

    cufo_puts(cufo_stdout, "====\n");
    test(qL(qM(gprod(qL(_1), qM(sgt(_2))))));
    cufo_puts(cufo_stdout, "----\n");
    test(qM(qL(gprod(_1, sgt(_0)))));
    cufo_puts(cufo_stdout, "----\n");
    test(qL(qM(gprod(cuex_o1_lambda(_1), _1))));
    cufo_puts(cufo_stdout, "----\n");
    test(qM(qL(gprod(_1, _0))));

    cufo_puts(cufo_stdout, "====\n");
    test(gprod(gprod(qM(gprod(_0,
			      qL(_0))),
		     qL(_0)),
	       qL(_0)));

    cufo_puts(cufo_stdout, "====\n");
    test(qL(gprod(qL(_1),
		  gprod(qL(gprod(qM(gprod(e1, _2)), _0)),
			e2))));
    cufo_puts(cufo_stdout, "----\n");
    test(qL(gprod(qL(_1),
		  gprod(qL(gprod(gprod(e1, _1), _0)),
			e2))));
    cufo_puts(cufo_stdout, "====\n");
    test(qM(gprod(qL(qM(gprod(_1,
			      qL(qL(gprod(_2,
					  sgt(_0))))))),
		  _0)));
    cufo_puts(cufo_stdout, "----\n");
    test(gprod(qL(qM(gprod(_1,
			   qL(qL(gprod(_2,
				       sgt(_0))))))),
	       qM(gprod(qL(qM(gprod(_1,
				    qL(qL(gprod(_2,
						sgt(_0))))))),
			_0))));
    cufo_puts(cufo_stdout, "====\n");
    test(qM(gprod(gprod(sgt(qL(gprod(qM(gprod(qL(gprod(_1, _0)), _1)), _1))), gprod(e1, e2)), e3)));
    cufo_puts(cufo_stdout, "----\n");
    test(gprod(gprod(sgt(qL(gprod(qM(gprod(qL(gprod(_1, _0)), _1)), qM(gprod(gprod(sgt(qL(gprod(qM(gprod(qL(gprod(_1, _0)), _1)), _1))), gprod(e1, e2)), e3))))), gprod(e1, e2)), e3));
    cufo_puts(cufo_stdout, "----\n");
    test(gprod(gprod(sgt(qL(gprod(gprod(qL(gprod(qM(gprod(qL(gprod(_1, _0)), _2)), _0)), _0), gprod(gprod(sgt(qL(gprod(qM(gprod(qL(gprod(_1, _0)), _1)), qM(gprod(gprod(sgt(qL(gprod(qM(gprod(qL(gprod(_1, _0)), _1)), _1))), gprod(e1, e2)), e3))))), gprod(e1, e2)), e3)))), gprod(e1, e2)), e3));
    return 0;
}
