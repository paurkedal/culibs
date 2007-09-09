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

#include <cuex/ex.h>
#include <cuex/pvar.h>
#include <cuex/ssfn.h>
#include <cudyn/misc.h>
#include <cu/idr.h>
#include <cucon/list.h>
#include <cuex/subst.h>
#include <cuex/oprdefs.h>
#include <stdio.h>


static void
ssfn_find_alt(cuex_ssfn_t ssfn, cuex_t key,
	      cu_clop(out, cu_bool_t, cu_count_t, cuex_t *, void *))
{
    struct cuex_ssfn_find_it_s it;
    cuex_ssfn_find_it_cct(&it, ssfn, key);
    for (; !cuex_ssfn_find_it_is_end(&it); cuex_ssfn_find_it_advance(&it))
	if (cu_call(out,
		     cuex_ssfn_find_it_arg_cnt(&it),
		     cuex_ssfn_find_it_arg_arr(&it),
		     cuex_ssfn_find_it_slot(&it)))
	    cuex_ssfn_find_it_report_match(&it);
}

cu_clos_def(collect_match,
	    cu_prot(cu_bool_t, cu_count_t arg_cnt,
			       cuex_t *arg_arr, void *slot),
	( cucon_list_t lst; ))
{
    cu_clos_self(collect_match);
    cucon_list_append_ptr(self->lst, *(cuex_t*)slot);
    return cu_true;
}

static void
test_insert(cuex_ssfn_t ssfn, cuex_t ex)
{
    void *slot;
    if (cuex_ssfn_insert_mem(ssfn, ex, 0, sizeof(cuex_t), &slot,
			    NULL, NULL)) {
	cu_fprintf(stdout, "Inserted new pattern ‘%!’.\n", ex);
	*(cuex_t*)slot = ex;
    }
    else
	cu_fprintf(stdout, "Not inserted existing pattern ‘%!’.\n", ex);
}

cu_clop_def(copy_slot, void, cuex_t *dst, cuex_t *src)
{
    *dst = *src;
}

static int errors = 0;

#define test_find(ssfn, key) \
    (printf("‘"#key"’ = "), test_find_x(ssfn, key))
void
test_find_x(cuex_ssfn_t ssfn, cuex_t key)
{
//    cuex_t key = chi_sy_sparse(str);
    cucon_list_t lst0 = cucon_list_new();
    cucon_list_t lst1 = cucon_list_new();
    cucon_listnode_t it0, it1;
    collect_match_t cb;
    cu_clop(cb_clop, cu_bool_t, cu_count_t, cuex_t *, void *)
	= collect_match_prep(&cb);
//    printf("‘%s’ matches\n", str);
    cu_fprintf(stdout, "‘%!’ matches\n", key);
    cb.lst = lst0;
    cuex_ssfn_find(ssfn, key, cb_clop);
    cb.lst = lst1;
    ssfn_find_alt(ssfn, key, cb_clop);
    for (it0 = cucon_list_begin(lst0), it1 = cucon_list_begin(lst1);
	 it0 != cucon_list_end(lst0) && it1 != cucon_list_end(lst1);
	 it0 = cucon_listnode_next(it0), it1 = cucon_listnode_next(it1)) {
	cuex_t ex0 = cucon_listnode_ptr(it0);
	cuex_t ex1 = cucon_listnode_ptr(it1);
	if (cuex_eq(ex0, ex1))
	    cu_fprintf(stdout, "    %!\n", ex0);
	else {
	    cu_fprintf(stdout, "    v0: %!\n    v1: %!\n", ex0, ex1);
	    ++errors;
	}
    }
    while (it0 != cucon_list_end(lst0)) {
	cu_fprintf(stdout, "    v0: %!\n", cucon_listnode_ptr(it0));
	it0 = cucon_listnode_next(it0);
	++errors;
    }
    while (it1 != cucon_list_end(lst1)) {
	cu_fprintf(stdout, "    v1: %!\n", cucon_listnode_ptr(it1));
	it1 = cucon_listnode_next(it1);
	++errors;
    }
}

cu_clop_def(print_find_mgu_cb, cuex_ssfn_ctrl_t,
	    void *slot,
	    cuex_subst_t subst,
	    cu_rank_t pat_arg_cnt, cuex_t *pat_arg_arr,
	    cuex_ssfn_matchinfo_t ign)
{
    cu_rank_t i;
    cu_fprintf(stdout, "    %!\n\tkey_args: ", *(cuex_t*)slot);
    cuex_subst_print(subst, stdout, ", ");
    //putc('\n', stdout);
    //cuex_subst_dump(subst, stdout);
    fputs("\n\tpat_args: ", stdout);
    for (i = 0; i < pat_arg_cnt; ++i) {
	if (i != 0)
	    fputs(", ", stdout);
	if (pat_arg_arr[i])
	    cu_fprintf(stdout, "%!", pat_arg_arr[i]);
	else
	    fputs("NULL", stdout);
    }
    fputc('\n', stdout);
    return cuex_ssfn_ctrl_continue;
}

void
print_find_lgr(cuex_ssfn_t ssfn, cuex_t key)
{
    cuex_subst_t subst = NULL;
    cu_fprintf(stdout, "More specific and equally specific as %!:\n", key);
    cuex_ssfn_find_lgr(ssfn, subst, key, 0, print_find_mgu_cb);
}
void
print_find_mgu(cuex_ssfn_t ssfn, cuex_t key)
{
    cuex_subst_t subst = NULL;
    cu_fprintf(stdout, "MGUs of semantic function and %!:\n", key);
    cuex_ssfn_find_mgu(ssfn, subst, key, 0, print_find_mgu_cb);
}

void
print_find_mgr(cuex_ssfn_t ssfn, cuex_t key)
{
    cuex_subst_t subst = NULL;
    cu_fprintf(stdout, "More general or equally general as %!:\n", key);
    cuex_ssfn_find_mgr(ssfn, subst, key, 0, print_find_mgu_cb);
}

#define LAMBDA(v, b) cuex_o2_lambda(v, b)
#define PLUS(x, y) cuex_opn(cuex_opr(0x100, 2), x, y)
#define TIMES(x, y) cuex_opn(cuex_opr(0x101, 2), x, y)

int
main()
{
    int i;
    cuex_ssfn_t ssfn;
    cuex_t V[4];
    cuex_t I[2];
    //chi_scope_t scope;
    //chi_frame_t frame;

    cuex_init();
    //scope = chi_scope_new(NULL);
    ssfn = cuex_ssfn_new();
    for (i = 0; i < 4; ++i) {
	V[i] = cuex_pvar_to_ex(cuex_pvar_new(cuex_qcode_active_w));
	//cuex_pvar_set_scope(cuex_pvar_from_ex(V[i]), scope);
    }
    for (i = 0; i < 2; ++i) {
	cuex_pvar_t var = cuex_pvar_new(cuex_qcode_active_w);
	//cuex_pvar_set_scope(var, scope);
	I[i] = cuex_pvar_to_ex(var);
    }
    //frame = chi_frame_new_rec(scope, chi_frame_null());
//    for (i = 0; i < 2; ++i) {
//	cuex_pvar_set_type(cuex_pvar_from_ex(I[i]),
//			 chi_sy_sparse("integers"), frame);
//    }

    test_insert(ssfn, I[0]);
    test_insert(ssfn, V[0]);
//    test_insert(ssfn, chi_sy_sparse("⅂#A, #A", V[0], V[1]));
//    test_insert(ssfn, chi_sy_sparse("#A + #A", V[0], V[1]));
//    test_insert(ssfn, chi_sy_sparse("#A + #A", V[2], V[0]));
//    test_insert(ssfn, chi_sy_sparse("#A + #A", V[0], V[0]));
//    test_insert(ssfn, chi_sy_sparse("#A + #A⋅#A", V[0], V[1], V[2]));
//    test_insert(ssfn, chi_sy_sparse("#A⋅#A + #A", V[0], V[1], V[2]));
//    test_insert(ssfn, chi_sy_sparse("1 + #A", V[0]));
//    test_insert(ssfn, chi_sy_sparse("#A + 2", V[0]));
//    test_insert(ssfn, chi_sy_sparse("2 + #A", V[0]));
//    test_insert(ssfn, chi_sy_sparse("x + #A", V[0]));
//    test_insert(ssfn, chi_sy_sparse("#A + x", V[0]));
//    test_insert(ssfn, chi_sy_sparse("#A + y", V[0]));
//    test_insert(ssfn, chi_sy_sparse("⅂#A, #A⋅#A + #A",
//				    V[0], V[1], V[2], V[3]));
    test_insert(ssfn, LAMBDA(V[0], V[1]));
    test_insert(ssfn, PLUS(V[0], V[1]));
    test_insert(ssfn, PLUS(V[2], V[1]));
    test_insert(ssfn, PLUS(V[0], V[0]));
    test_insert(ssfn, PLUS(V[0], TIMES(V[1], V[2])));
    test_insert(ssfn, PLUS(TIMES(V[0], V[1]), V[2]));
    test_insert(ssfn, PLUS(cudyn_long(1), V[0]));
    test_insert(ssfn, PLUS(V[0], cudyn_long(2)));
    test_insert(ssfn, PLUS(cudyn_long(2), V[0]));
    test_insert(ssfn, PLUS(cu_idr_by_cstr("x"), V[0]));
    test_insert(ssfn, PLUS(V[0], cu_idr_by_cstr("x")));
    test_insert(ssfn, PLUS(V[0], cu_idr_by_cstr("y")));
    test_insert(ssfn, LAMBDA(V[0], PLUS(TIMES(V[1], V[2]), V[3])));

//    test_find(ssfn, "10");
//    test_find(ssfn, "\"10\"");
//    test_find(ssfn, "a");
//    test_find(ssfn, "1 + 2");
//    test_find(ssfn, "2 + 2");
//    test_find(ssfn, "1 + z");
//    test_find(ssfn, "x + 2");
//    test_find(ssfn, "x + y");
//    test_find(ssfn, "a + 8");
//    test_find(ssfn, "2⋅4 + 8");
//    test_find(ssfn, "a⋅a + 3⋅c");
//    test_find(ssfn, "⅂x, 0");
    test_find(ssfn, cudyn_long(10));
    test_find(ssfn, cu_idr_by_cstr("a"));
    test_find(ssfn, PLUS(cudyn_long(1), cudyn_long(2)));
    test_find(ssfn, PLUS(cudyn_long(2), cudyn_long(2)));
    test_find(ssfn, PLUS(cudyn_long(1), cu_idr_by_cstr("z")));
    test_find(ssfn, PLUS(cu_idr_by_cstr("x"), cudyn_long(2)));
    test_find(ssfn, PLUS(cu_idr_by_cstr("x"), cu_idr_by_cstr("y")));
    test_find(ssfn, PLUS(cu_idr_by_cstr("a"), cudyn_long(8)));
    test_find(ssfn, PLUS(TIMES(cudyn_long(2), cudyn_long(4)),
			 cudyn_long(8)));
    test_find(ssfn, PLUS(TIMES(cu_idr_by_cstr("a"), cu_idr_by_cstr("a")),
			 TIMES(cudyn_long(3), cu_idr_by_cstr("c"))));
    test_find(ssfn, LAMBDA(cu_idr_by_cstr("x"), cudyn_long(0)));

//    print_find_lgr(ssfn, chi_sy_sparse("#A + #A", V[2], V[3]));
//    print_find_lgr(ssfn, chi_sy_sparse("#A + #A", V[2], V[2]));
//    print_find_lgr(ssfn, chi_sy_sparse("⅂#A, #A", V[0], V[1]));
//    print_find_mgu(ssfn, chi_sy_sparse("#A + #A", V[2], V[3]));
//    print_find_mgr(ssfn, chi_sy_sparse("#A + #A", V[0], V[1]));
    print_find_lgr(ssfn, PLUS(V[2], V[3]));
    print_find_lgr(ssfn, PLUS(V[2], V[2]));
    print_find_lgr(ssfn, LAMBDA(V[0], V[1]));
    print_find_mgu(ssfn, PLUS(V[2], V[3]));
    print_find_mgr(ssfn, PLUS(V[0], V[1]));

    ssfn = cuex_ssfn_new_copy_mem(ssfn, sizeof(cuex_t),
				  (cu_clop(, void, void *, void *))copy_slot);
//    print_find_mgr(ssfn, chi_sy_sparse("#A + #A", V[0], V[1]));
    print_find_mgr(ssfn, PLUS(V[0], V[1]));

    if (errors) {
	printf("%d errors\n", errors);
	return 1;
    }
    else {
	printf("no errors\n");
	return 0;
    }
}
