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

#include <chi/fwd.h>
#include <chi/ssfn.h>
#include <chi/sy/parse.h>


static int error_cnt = 0;

static void
define(cuex_ssfn_t ssfn, cuex_t key, cu_bool_t expect)
{
    cuex_t *slot;
    size_t var_cnt_o;
    cuex_pvar_t *var_arr_o;
    chi_fprintf(stdout, "Def %!\n", key);
    if (cuex_ssfn_insert_mem(ssfn, key, 0, sizeof(cuex_t), &slot,
			    &var_cnt_o, &var_arr_o) != expect) {
	++error_cnt;
	if (expect)
	    chi_fprintf(stderr, "Failed to define %!.\n");
	else
	    chi_fprintf(stderr, "%! should already be defined.\n");
    }
    *slot = key;
}

cu_clos_def(check_cb,
	    cu_prot(cuex_ssfn_ctrl_t
		    void *slot,
		    cuex_subst_t subst,
		    cu_rank_t pat_arg_cnt, cuex_t *pat_arg_arr,
		    cuex_ssfn_matchinfo_t have_weak),
	    (int match_cnt;))
{
    cu_clos_self(check_cb);
    ++self->match_cnt;
    chi_fprintf(stdout, "    %!\n", *(cuex_t *)slot);
    return cuex_ssfn_ctrl_continue;
}
static void
check(cuex_ssfn_t ssfn, cuex_t key, int expect,
      cuex_ssfn_ctrl_t (*find)(cuex_ssfn_t, cuex_subst_t, cuex_t,
			      cuex_ssfn_seqno_t, cuex_ssfn_find_cb_t),
      char *find_name)
{
    check_cb_t cb;
    cb.match_cnt = 0;
    chi_fprintf(stdout, "%s %!\n", find_name, key);
    (*find)(ssfn, NULL, key, 0, check_cb_prep(&cb));
    if (expect != cb.match_cnt) {
	++error_cnt;
	fprintf(stderr, "Bad number of matches, expected %d.\n", expect);
	cuex_ssfn_dump_keys(ssfn, stderr);
    }
}

static cuex_t
parse(char const *str, cucon_pmap_t env)
{
    cuex_t ex = chi_sy_sparse(str);
    cu_debug_assert(ex);
    ex = cuex_autoquantify_uvw_xyz(ex, env);
    cu_debug_assert(ex);
    return ex;
}

int the_main()
{
#define F(ex) parse(ex, env)
#define D(ex) define(ssfn, F(ex), cu_true)
#define U_lgr(ex) cuex_ssfn_erase_lgr(ssfn, F(ex))
#define C_wpmgr(ex, n) \
	check(ssfn, F(ex), n, cuex_ssfn_find_wpmgr, "mge_strong")
#define C_mgr(ex, n) \
	check(ssfn, F(ex), n, cuex_ssfn_find_mgr, "mgr")
#define C_lgr(ex, n) \
	check(ssfn, F(ex), n, cuex_ssfn_find_lgr, "lgr")
#define C_wpmgu(ex, n) \
	check(ssfn, F(ex), n, cuex_ssfn_find_wpmgu, "wpmgu")
#define C_mgu(ex, n) \
	check(ssfn, F(ex), n, cuex_ssfn_find_mgu, "mgu")

    cuex_ssfn_t ssfn = cuex_ssfn_new();
    cucon_pmap_t env = cucon_pmap_new();

    D("true");
    D("is_zero 0");
    D("is_one 1");
    D("¬ is_zero u");
    D("diagonal x x");
    D("¬ diagonal u v");
    cuex_ssfn_dump_keys(ssfn, stdout);

    C_mgr("true", 1);
    C_mgr("is_zero 0", 1);
    C_mgu("is_one 1", 1);
    C_mgu("is_zero v", 1);
    C_wpmgu("is_zero v", 0);
    C_mgr("¬ is_zero 0", 1);
    C_wpmgr("¬ is_zero 0", 0);

    C_mgr("diagonal 0 0", 1);
    C_wpmgr("diagonal 0 0", 1);
    C_mgr("¬ is_zero a", 1);

    C_mgu("¬ is_zero (a + b)", 1);
    C_mgu("¬ x", 2);

    D("f u");
    D("f (f u)");
    D("f (f (f u))");
    D("f (f (g x))");
    D("f (g x)");
    C_mgr("f (f 1)", 2);
    C_lgr("f (f v)", 3);
    U_lgr("f (f u)");
    C_mgr("f (f 1)", 1);
    C_lgr("f (f v)", 0);
    cuex_ssfn_dump_keys(ssfn, stdout);

    D("h x0 x1 x0");
    C_lgr("h x0 x0 x1", 0);

    return !!error_cnt;
}

int main()
{
    chi_lang_init();
    return the_main();
}
