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

#include <cucon/po.h>
#include <cucon/pmap.h>
#include <cu/clos.h>
#include <cu/debug.h>
#include <cu/diag.h>
#include <cu/test.h>
#include <assert.h>
#include <stdio.h>

#define bitpreceq(x, y) (((x) & (y)) == (x))
#define bitprec(x, y) (bitpreceq(x, y) && x != y)

cu_clos_def(main0,
	    cu_prot(void, cucon_poelt_t elt),
	    ( cucon_umap_t accum;
	      int n; ))
{
    cu_clos_self(main0);
    int new_p;
    void* ptr;
    int i = *(int*)cucon_poelt_get_mem(elt);
    new_p = cucon_umap_insert_mem(self->accum, i, 0, &ptr);
    ++self->n;
    if (cu_verbosity() > 11)
	printf("%x ", i);
    assert(new_p); /* It should run over each element only once. */
}


cu_clos_def(check_inf_span_cb1,
	    cu_prot(void, void const *e1),
	    ( cucon_poelt_t e0;
	      int found_prec, found_succ; ))
{
#define e1 ((cucon_poelt_t)e1)
    cu_clos_self(check_inf_span_cb1);
    self->found_prec += !!cucon_po_prec(e1, self->e0);
    self->found_succ += !!cucon_po_prec(self->e0, e1);
#undef e1
}
cu_clos_def(check_inf_span_cb0,
	    cu_prot(void, void const *e0arg),
	    ( cucon_pmap_t pruned_inf_span; ))
{
#define e0arg ((cucon_poelt_t)e0arg)
    cu_clos_self(check_inf_span_cb0);
    check_inf_span_cb1_t cb;
    cb.e0 = e0arg;
    cb.found_prec = 0;
    cb.found_succ = 0;
    cucon_pmap_iter_keys(self->pruned_inf_span, check_inf_span_cb1_prep(&cb));
    if (cucon_pmap_find_mem(self->pruned_inf_span, e0arg)) {
	cu_test_assert(cb.found_prec == 0); /* self */
	cu_test_assert(cb.found_succ == 0);
    }
    else {
	cu_test_assert(cb.found_prec > 0);
	cu_test_assert(cb.found_succ == 0);
    }
#undef e0arg
}

void
check_inf_span(cucon_pmap_t inf_span, cucon_pmap_t pruned_inf_span)
{
    check_inf_span_cb0_t cb;
    cb.pruned_inf_span = pruned_inf_span;
    cu_verbf(0,"Inf span with size %d reduced to size %d.",
	      cucon_pmap_size(inf_span), cucon_pmap_size(pruned_inf_span));
    cucon_pmap_iter_keys(inf_span, check_inf_span_cb0_prep(&cb));
}


/* Check cucon_po_inf_of_list
 * ------------------------ */

cu_clos_def(check_inf_of_list_cbp,
	    cu_prot(void, void const *e1_inf),
	    ( cucon_poelt_t e0_inf; ))
{
#define e1_inf ((cucon_poelt_t)e1_inf)
    cu_clos_self(check_inf_of_list_cbp);
    cu_test_assert(self->e0_inf == e1_inf
		   || !cucon_po_prec(self->e0_inf, e1_inf));
#undef e1_inf
}
cu_clos_def(check_inf_of_list_cb,
	    cu_prot(void, void const *e_inf),
	    ( cucon_list_t L;
	      cucon_pmap_t S; ))
{
#define e_inf ((cucon_poelt_t)e_inf)
    cu_clos_self(check_inf_of_list_cb);
    cucon_listnode_t it_L;
    check_inf_of_list_cbp_t cbp;
    for (it_L = cucon_list_begin(self->L); it_L != cucon_list_end(self->L);
	 it_L = cucon_listnode_next(it_L)) {
	cucon_poelt_t e = cucon_listnode_ptr(it_L);
	cu_test_assert(cucon_po_preceq(e_inf, e));
    }
    cbp.e0_inf = e_inf;
    cucon_pmap_iter_keys(self->S, check_inf_of_list_cbp_prep(&cbp));
#undef e_inf
}

void
check_inf_of_list(cucon_poelt_t *ve)
{
    size_t const N = 100;
    size_t n;
    check_inf_of_list_cb_t cb;
    cb.L = cucon_list_new();
    for (n = 0; n < N; ++n) {
	if (ve[n])
	    cucon_list_append_ptr(cb.L, ve[n]);
    }
    cb.S = cucon_po_inf_of_list(cb.L);
    cu_verbf(10, "size L = %d; size (inf L) = %d",
	      cucon_list_count(cb.L), cucon_pmap_size(cb.S));
    cucon_pmap_iter_keys(cb.S, check_inf_of_list_cb_prep(&cb));
}


/* main
 * ---- */

void
check_all()
{
    /* NB. N must be larger than big, check_inf_of_list:N */
    int const N = 1000;
    int const small = 4;
    int const big = 0x1ff;
    int n_inrange = 0;
    cucon_poelt_t ve[N];
    cucon_po_t po = cucon_po_new();
    int i, j, k;
    main0_t main0cl;
    int n_inrange0;
    cucon_pmap_t inf_span, pruned_inf_span;
    cu_init();
    assert(small < big && big < N);
    main0cl.n = 0;
    for (i = 0; i < N; ++i)
	ve[i] = 0;
    for (k = 0; k < N/2; ++k) {
	if (k < 2) {
	    if (k == 0)
		i = small;
	    else
		i = big;
	}
	else
	    i = lrand48() % N;
	if (ve[i])
	    continue;
	if (bitprec(small, i) && bitprec(i, big))
	    ++n_inrange;
	ve[i] = cucon_po_insert_mem(po, sizeof(int));
	*(int*)cucon_poelt_get_mem(ve[i]) = i;
	for (j = 0; j < N; ++j) {
	    if (!ve[j])
		continue;
	    if (bitprec(j, i)) {
		cucon_po_constrain_prec(po, ve[j], ve[i]);
		cu_verbf(12, "%x ⊏ %x, ", j, i);
	    }
	    else if (bitprec(i, j)) {
		cucon_po_constrain_prec(po, ve[i], ve[j]);
		cu_verbf(12, "%x ⊐ %x, ", j, i);
	    }
	}
    }
    for (i = 0; i < N; ++i) {
	if (!ve[i])
	    continue;
	cucon_po_debug_check_nonredundant(ve[i]);
	for (j = 0; j < i; ++j)
	    if (!ve[j])
		continue;
	assert(cucon_po_preceq(ve[i], ve[j]) == bitpreceq(i, j));
	assert(cucon_po_preceq(ve[j], ve[i]) == bitpreceq(j, i));
    }

    main0cl.accum = cucon_umap_new();
    cucon_po_iter_open_range(ve[small], ve[big], main0_prep(&main0cl));
    n_inrange0 = main0cl.n;
    cu_verbf(11, "\n%d≟%d in range %x to %x\n", n_inrange, n_inrange0,
	      small, big);
    assert(n_inrange0 == n_inrange);

    /* Test inf span */
    inf_span = cucon_pmap_new();
    for (i = 0; i < N/8; ++i) {
	int i = lrand48() % N;
	if (ve[i])
	    cucon_pmap_insert_void(inf_span, ve[i]);
    }
    pruned_inf_span = cucon_po_pruned_lspanning(po, inf_span);
    check_inf_span(inf_span, pruned_inf_span);

    check_inf_of_list(ve);
}

int
main()
{
    int i;
    cu_init();
    cu_set_verbosity(11);
    check_all();
    cu_set_verbosity(10);
    for (i = 0; i < 20; ++i)
	check_all();
    return 2*!!cu_test_bug_count();
}
