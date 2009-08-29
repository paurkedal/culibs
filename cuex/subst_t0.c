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

#include <cuex/subst.h>
#include <cu/str.h>
#include <cuex/pvar.h>
#include <cu/idr.h>
#include <cuex/oprdefs.h>
#include <cuex/opn.h>
#include <cu/test.h>
#include <cufo/stream.h>

static void
unify(cuex_subst_t subst, cuex_t ex0, cuex_t ex1, cu_bool_t expect)
{
    if (!!cuex_subst_unify(subst, ex0, ex1) != !!expect) {
	if (expect)
	    cu_test_bugf("Could not unify %! with %!.\n", ex0, ex1);
	else
	    cu_test_bugf("Unified %! with %!.\n", ex0, ex1);
    }
}

int
main()
{
    cuex_subst_t sig0;
    cuex_subst_t sig1;
    cuex_t v[10];
    cuex_t c[2];
    int i;
    cuex_init();
    cu_test_on_bug(cu_test_bugaction_cont, 0);
    sig0 = cuex_subst_new_uw();
    for (i = 0; i < 10; ++i)
	v[i] = cuex_var_new_u();
	//v[i] = cuex_pvar_to_ex(
	//    cuex_pvar_new_named(cuex_qcode_active_s,
	//		      cu_str_to_cstr(cu_str_new_fmt("x%d", i))));
    c[0] = cu_idr_by_cstr("a");
    c[1] = cu_idr_by_cstr("b");
    unify(sig0, v[0], v[1], cu_true);
    unify(sig0, v[0], cuex_o2_apply(v[2], c[0]), cu_true);
    cu_test_assert_ptr_eq(cuex_subst_cref(sig0, cuex_var_from_ex(v[0])),
			  cuex_subst_cref(sig0, cuex_var_from_ex(v[1])));
    unify(sig0, v[2], v[1], cu_false);
    unify(sig0, v[3], v[4], cu_true);

    sig1 = cuex_subst_new_uw_clone(sig0);
    unify(sig1, v[5], cuex_o2_lambda(v[0], c[1]), cu_true);
    unify(sig1, v[5], v[3], cu_true);
    cu_test_assert_ptr_eq(cuex_subst_cref(sig1, cuex_var_from_ex(v[5])),
			  cuex_subst_cref(sig1, cuex_var_from_ex(v[3])));
    cu_test_assert(sig1->shadowed);
    cuex_subst_dump(sig1, stdout); fflush(stdout);
    cufo_oprintf("Last subst prettyprited:\n\t");
    cuex_subst_print(sig1, cufo_stdout, "\n\t");
    for (i = 0; i < 60; ++i)
	cuex_subst_cref(sig1, cuex_var_from_ex(v[9]));
    cufo_putc(cufo_stdout, '\n');
    cu_test_assert(!sig1->shadowed);
    cu_test_assert_ptr_eq(cuex_subst_cref(sig1, cuex_var_from_ex(v[5])),
			  cuex_subst_cref(sig1, cuex_var_from_ex(v[3])));

    sig0 = cuex_subst_new(cuex_qcset_uw);
    unify(sig0, v[1], v[0], cu_true);
    unify(sig0, v[1], v[4], cu_true);
    unify(sig0, v[1], v[3], cu_true);
    unify(sig0, v[2], v[0], cu_true);
    cu_test_assert(cuex_subst_lookup(sig0, cuex_var_from_ex(v[1])) == v[3]);

#ifdef CUCONF_HAVE_BUDDY
    /* non-idempotent substitutions */
    sig0 = cuex_subst_new_nonidem(cuex_qcset_uw);
    unify(sig0, v[0], cuex_o2_apply(v[1], v[1]), cu_true);
    unify(sig0, v[1], cuex_o2_apply(v[0], v[0]), cu_true);
    unify(sig0, v[2], v[0], cu_true);
    unify(sig0, v[3], v[1], cu_true);
    unify(sig0, v[4], cuex_o2_apply(v[0], v[1]), cu_true);
    cuex_subst_render_idempotent(sig0);
    cuex_subst_dump(sig0, stdout);

    sig0 = cuex_subst_new_nonidem(cuex_qcset_uw);
    unify(sig0, v[0], cuex_o2_farrow(v[1], v[2]), cu_true);
    unify(sig0, v[1], cuex_o2_farrow(v[2], v[0]), cu_true);
    unify(sig0, v[2], cuex_o2_farrow(v[0], v[1]), cu_true);
    cuex_subst_render_idempotent(sig0);
    cuex_subst_dump(sig0, stdout);
#endif

    return !!cu_test_bug_count();
}
