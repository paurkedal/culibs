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
#include <cuex/oprdefs.h>
#include <cuex/opn.h>
#include <cuex/var.h>
#include <cuex/recursion.h>
#include <cuex/print.h>
#include <cucon/pmap.h>
#include <cucon/frame.h>
#include <cu/test.h>
#include <cu/int.h>
#include <stdlib.h>
#include <limits.h>

#define VERBOSE 0
#if 1
#  define REPEAT 10000
#  define PLOT_BIN_COUNT 20
#  define PLOT_NODECOUNT_GRAN 10
#else
#  define REPEAT 40
#  define PLOT_BIN_COUNT 10
#  define PLOT_NODECOUNT_GRAN 2
#endif
#define MAX_NODE_COUNT (PLOT_BIN_COUNT*PLOT_NODECOUNT_GRAN)

#if 0
#define MINIMISE cuex_mu_minimise
#else
#define MINIMISE cuex_optimal_fold
#endif

typedef struct dmu_depth_eq_frame_s
{
    cuex_t e;
    int depth;
} *dmu_depth_eq_frame_t;

static cu_bool_t
dmu_depth_eq_aux(cuex_t e0, dmu_depth_eq_frame_t muf0,
		 cuex_t e1, dmu_depth_eq_frame_t muf1, int depth)
{
    cuex_meta_t m0, m1;

    if (depth < 0)
	return cu_true;

    m0 = cuex_meta(e0);
    m1 = cuex_meta(e1);
    if (cuex_meta_is_opr(m0)) {
	if (cuex_og_hole_contains(m0)) {
	    int i = cuex_oa_hole_index(m0);
	    dmu_depth_eq_frame_t mufp = cucon_frame_at(muf0, i);
	    if (cuex_meta(mufp->e) == CUEX_O1_MU) {
		muf0 = mufp;
		e0 = cuex_opn_at(muf0->e, 0);
		m0 = cuex_meta(e0);
	    }
	}
	else if (m0 == CUEX_O1_MU) {
	    muf0 = cucon_frame_push(muf0, sizeof(struct dmu_depth_eq_frame_s));
	    muf0->e = e0;
	    muf0->depth = depth;
	    e0 = cuex_opn_at(e0, 0);
	    m0 = cuex_meta(e0);
	}
	if (cuex_og_binder_contains(m0)) {
	    muf0 = cucon_frame_push(muf0, sizeof(struct dmu_depth_eq_frame_s));
	    muf0->e = e0;
	    muf0->depth = depth;
	}
    }
    if (cuex_meta_is_opr(m1)) {
	if (cuex_og_hole_contains(m1)) {
	    int i = cuex_oa_hole_index(m1);
	    dmu_depth_eq_frame_t mufp = cucon_frame_at(muf1, i);
	    if (cuex_meta(mufp->e) == CUEX_O1_MU) {
		muf1 = mufp;
		e1 = cuex_opn_at(muf1->e, 0);
		m1 = cuex_meta(e1);
	    }
	}
	else if (m1 == CUEX_O1_MU) {
	    muf1 = cucon_frame_push(muf1, sizeof(struct dmu_depth_eq_frame_s));
	    muf1->e = e1;
	    muf1->depth = depth;
	    e1 = cuex_opn_at(e1, 0);
	    m1 = cuex_meta(e1);
	}
	if (cuex_og_binder_contains(m1)) {
	    muf1 = cucon_frame_push(muf1, sizeof(struct dmu_depth_eq_frame_s));
	    muf1->e = e1;
	    muf1->depth = depth;
	}
    }

    if (cuex_meta_is_opr(m0) && cuex_og_hole_contains(m0) &&
	cuex_meta_is_opr(m1) && cuex_og_hole_contains(m1)) {
	muf0 = cucon_frame_at(muf0, cuex_oa_hole_index(m0));
	muf1 = cucon_frame_at(muf1, cuex_oa_hole_index(m1));
	cu_debug_assert(cuex_og_binder_contains(cuex_meta(muf0->e)));
	cu_debug_assert(cuex_og_binder_contains(cuex_meta(muf1->e)));
	return muf0->depth == muf1->depth;
    }

    if (m0 != m1)
	return cu_false;
    if (cuex_meta_is_opr(m0)) {
	cu_rank_t i, r;
	r = cuex_opr_r(m0);
	for (i = 0; i < r; ++i)
	    if (!dmu_depth_eq_aux(cuex_opn_at(e0, i), muf0,
				  cuex_opn_at(e1, i), muf1,
				  depth - 1))
		return cu_false;
	return cu_true;
    }
    else
	return e0 == e1;
}

/* Approximation to the equal predicate for μ-recursive expressions.  It will
 * always return true if e0 and e1 are equal, and it will return false if a
 * difference in e0 and e1 can be found within a certain depth.  This function
 * is only used for testing the Hopcroft minimisation, since that algorithm
 * will be used for the real equality test if needed. */
static cu_bool_t
dmu_depth_eq(cuex_t e0, cuex_t e1)
{
    int d;
    d = 2*cu_int_max(cuex_max_depth(e0), cuex_max_depth(e1));
    if (d > 10)
	d = 10; /* Avoid combinatorial explosion. */
    return dmu_depth_eq_aux(e0, NULL, e1, NULL, d);
}


cuex_t
random_expr(int depth, int *size_limit)
{
    cuex_t e;
    cuex_meta_t opr;
    int i, r;
    cu_bool_t wrap_mu;
    unsigned long sel_minor, sel_kind;

    if (*size_limit <= 1)
	return cuex_var_new_e();

    sel_minor = lrand48();
    sel_kind = sel_minor & 3;
    sel_minor >>= 2;

    if (sel_kind == 0 && depth)
	return cuex_hole(sel_minor % depth);
    else if (sel_kind == 1) {
	--*size_limit;
	return cuex_o1_lambda(random_expr(depth + 1, size_limit));
    }
    else {
	cuex_t e_arr[3];
	static const cuex_meta_t opr_choices[] = {
	    CUEX_O0_NULL,	CUEX_O0_UNKNOWN,
	    CUEX_O1_IDENT,	CUEX_O1_SINGLETON,
	    CUEX_O2_GPROD,	CUEX_O2_APPLY,
	    CUEX_O3_IF,		CUEX_O3_IF
	};
	wrap_mu = sel_minor & 1;
	sel_minor >>= 1;
	opr = opr_choices[sel_minor % 8];
	--*size_limit;
	if (wrap_mu)
	    ++depth;
	r = cuex_opr_r(opr);
	for (i = 0; i < r; ++i)
	    e_arr[i] = random_expr(depth, size_limit);
	e = cuex_opn_by_arr(opr, e_arr);
	if (wrap_mu)
	    e = cuex_o1_mu(e);
	return e;
    }
}

cuex_t
unfold_random(cuex_t e, int *n, double x)
{
    cuex_meta_t e_meta = cuex_meta(e);
    if (*n > 0 && cuex_meta_is_opr(e_meta)) {
	if (e_meta == CUEX_O1_MU && drand48() < x) {
	    e = cuex_mu_unfold(e);
	    --*n;
	}
	else {
	    x /= cuex_opr_r(e_meta);
	    CUEX_OPN_TRAN(e_meta, e, ep, unfold_random(ep, n, x));
	}
    }
    return e;
}

struct run_stats_s
{
    int count;
    clock_t time;
};

void
test(struct run_stats_s *stats_arr)
{
    static int run = 0;
    cuex_t e, ep, epp, eppp, eT;
    cu_bool_t are_eq;
    int size_limit = 20;
    int n, plot_bin;
    size_t eT_size, epp_size;
    cuex_stats_t stats;

    e = random_expr(0, &size_limit);
    ep = MINIMISE(e);
    are_eq = dmu_depth_eq(e, ep);
    if (!are_eq || (VERBOSE && e != ep && run++ < 30))
	cu_fprintf(stderr, "\n INPUT: %!\nOUTPUT: %!\n", e, ep);
    if (!are_eq) {
//	cu_fprintf(stderr, "FOLDINERT: %!\n", cuex_foldinert_rebind(e, -1));
	cu_test_bugf("Equality test failed.");
    }

    n = 1;
    eT = unfold_random(ep, &n, 0.5);
    cuex_stats(eT, &stats);
    eT_size = stats.node_cnt + stats.var_cnt;
    do {
	n = 2;
	epp = eT;
	epp_size = eT_size;
	eT = unfold_random(eT, &n, 0.5);
	cuex_stats(eT, &stats);
	eT_size = stats.node_cnt + stats.var_cnt;
    } while (lrand48() % (MAX_NODE_COUNT/4) && eT_size < MAX_NODE_COUNT);
    if (epp_size >= MAX_NODE_COUNT)
	return;

    plot_bin = epp_size/PLOT_NODECOUNT_GRAN;
    stats_arr[plot_bin].time -= clock();
    eppp = MINIMISE(epp);
    stats_arr[plot_bin].time += clock();
    ++stats_arr[plot_bin].count;

    if (eppp != ep) {
	cu_fprintf(stderr, " INITIAL: %!\n", e);
	cu_fprintf(stderr, "   INPUT: %!\n  OUTPUT: %!\nEXPECTED: %!\n",
		   epp, eppp, ep);
//	cu_fprintf(stderr, "FOLDINERT: %!\n", cuex_foldinert_rebind(epp, -1));
	cuex_save_dot(e, NULL, NULL);
	cuex_save_dot(ep, NULL, NULL);
	cuex_save_dot(epp, NULL, NULL);
	cuex_save_dot(eppp, NULL, NULL);
	cu_test_bugf("Incomplete or incorrect simplification.");
    }
}

int
main()
{
    FILE *out;
    int i;
    struct run_stats_s stats_arr[PLOT_BIN_COUNT];
    cuex_init();
    cu_test_on_bug(cu_test_bugaction_exit, 40);

    memset(stats_arr, 0, sizeof(stats_arr));
    for (i = 0; i < REPEAT; ++i)
	test(stats_arr);

    out = fopen("tmp.hopcroft_t0.data", "w");
    if (out) {
	for (i = 0; i < PLOT_BIN_COUNT; ++i)
	    if (stats_arr[i].count)
		fprintf(out, "%lg %lg\n", (i + 0.5)*PLOT_NODECOUNT_GRAN,
			stats_arr[i].time
			/((double)CLOCKS_PER_SEC*stats_arr[i].count));
	fclose(out);
    }

    if (cu_test_bug_count()) {
	cu_errf("%d tests failed.", cu_test_bug_count());
	return 2;
    }
    return 0;
}
