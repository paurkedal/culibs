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
#include <cuex/oprdefs.h>
#include <cuex/var.h>
#include <cucon/uset.h>
#include <cucon/pmap.h>

static cuex_t _0, _1, _2, _3, _10;

cu_clop_def(show_bi, void, void const *key, void *lvars)
{
#define lvars ((cucon_uset_t)lvars)
#define key ((cuex_t)key)
    fputs("    ", stdout);
    cucon_uset_print(lvars, stdout);
    cu_fprintf(stdout, " are free in %!.\n", key);
#undef lvars
#undef key
}

void
test(cuex_t e)
{
    cucon_pmap_t ubi;
    int depth = cuex_max_binding_depth(e);
    cu_fprintf(stdout, "\nBindings of %!:\n", e);
    ubi = cuex_unfolded_fv_sets(e, depth);
    cucon_pmap_iter_mem(ubi, show_bi);

//    cu_fprintf(stdout, "Foldinert: %!\n", cuex_foldinert_rebind(e, depth));
}

#define M cuex_o1_mu
#define apply cuex_o2_apply
#define L cuex_o1_lambda
#define gprod cuex_o2_gprod
#define sing cuex_o1_singleton
#define ident cuex_o1_ident

int
main()
{
    cuex_init();
    cuex_t e1 = cuex_var_new_e();
    cuex_t e2 = cuex_var_new_e();
    cuex_t e3 = cuex_var_new_e();

    _0 = cuex_hole(0);
    _1 = cuex_hole(1);
    _2 = cuex_hole(2);
    _3 = cuex_hole(3);
    _10 = cuex_hole(10);

    test(cuex_o1_mu(cuex_o2_apply(cuex_o1_lambda(_10),
				  cuex_o1_mu(cuex_o2_apply(_0, cuex_o2_apply(_1, _2))))));
    test(L(M(apply(_0, _1))));
    test(L(M(L(apply(_0, apply(_1, _2))))));
    //test(M(sing(M(gprod(M(_2), L(M(_3)))))));
    test(M(L(M(_2))));
    test(L(gprod(M(L(gprod(_1, _0))), _0)));
    test(L(gprod(_0, M(L(gprod(_0, _1))))));
    test(L(M(gprod(M(sing(L(M(sing(_2))))), M(sing(_2))))));
//    test(gprod(M(gprod(_0, L(M(sing (gprod(_1, _0)))))),
//	       L(M(sing(gprod(_1, _0))))));
    test(gprod(M(gprod(_0, L(M(gprod(_1, _0))))),
	       L(M(gprod(_1, _0)))));
    test(L(L(gprod(M(gprod(_0, _2)), _0))));

// CU_SEED48=1181758125
//    test(L(M(gprod(gprod(L(gprod(e1, M(gprod(_0, _2)))),
//			       M(gprod(_0, _2))),
//			 L(e2)))));
    test(L(M(gprod(L(M(gprod(_0, _2))), M(gprod(_0, _2))))));
    // need to encode the level difference between μ-bind in μ-contexts
    // cuex_o2_meta_mupath:leveldiff(HEAD_SEQ, LAST_COMP)
    
    test(L(gprod(_0, M(sing(L(L(M(gprod(gprod(_1, _3), _0)))))))));
    return 0;
}
