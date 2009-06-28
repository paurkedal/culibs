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

#include <cu/clos.h>
#include <cuflow/cont.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <time.h>
#include <cu/conf.h>
#ifdef CUCONF_HAVE_GC_GC_H
#  include <gc/gc.h>
#else
#  include <gc.h>
#endif

/*
 * Times: t_nondet = 0.870000, t_plain = 0.020000, ratio = 43.500000.
 * Copied 369 MiB.  Investigated 4000000 possibilities.
 *
 * Times: t_nondet = 0.890000, t_plain = 0.020000, ratio = 44.500000.
 * Copied 369 MiB.  Investigated 4000000 possibilities.
 */

/*-- Clip: chi/slist.h */
typedef struct _chi_slist_node_s _chi_slist_node_t;
typedef struct _chi_slist_t chi_slist_t;
typedef _chi_slist_node_t *chi_slist_it_t;

struct _chi_slist_node_s {
    _chi_slist_node_t *next;
};
struct _chi_slist_t {
    _chi_slist_node_t head;
    _chi_slist_node_t tail;
};

/*
 *  Basis
 */

void			chi_slist_cct(chi_slist_t *);
#define			chi_slist_clear(list) chi_slist_cct(list)
#define			chi_slist_begin(list)	(&(list)->head)
#define			chi_slist_end(list)	((list)->tail.next)
chi_slist_it_t		chi_slist_insert_alloc(chi_slist_it_t, size_t);
chi_slist_it_t		chi_slist_insert_ptr(chi_slist_it_t, void *);
void			chi_slist_it_erase(chi_slist_it_t);
#define			chi_slist_it_get(it)	((void*)((it)->next+1))
#define			chi_slist_it_get_ptr(it) (*((void**)((it)->next+1)))
#define			chi_slist_it_next(it)	((it)->next)

/*
 *  Derived
 */

#define			chi_slist_prepend_alloc(list, size) \
	chi_slist_insert_alloc(chi_slist_begin(list), (size))
#define			chi_slist_prepend_ptr(list, p) \
	chi_slist_insert_ptr(chi_slist_begin(list), (p))
#define			chi_slist_append_alloc(list, size) \
	chi_slist_insert_alloc(chi_slist_end(list), (size))
#define			chi_slist_append_ptr(list, p) \
	chi_slist_insert_ptr(chi_slist_end(list), (p))
/*-- End Clip */

/*-- Clip: chi/slist.c */
void
chi_slist_cct(chi_slist_t *list)
{
    list->head.next = &list->tail;
    list->tail.next = &list->head;
}

void
chi_slist_it_erase(chi_slist_it_t it)
{
    _chi_slist_node_t *nd = it->next;
    it->next = it->next->next;
    if (nd->next->next == nd)
	nd->next->next = it;
}

chi_slist_it_t
chi_slist_insert_alloc(chi_slist_it_t it, size_t size)
{
    _chi_slist_node_t *p = GC_MALLOC(sizeof(_chi_slist_node_t) + size);
    p->next = it->next;
    it->next = p;
    if (p->next->next == it)
	p->next->next = p;
    return it;
}

chi_slist_it_t
chi_slist_insert_ptr(chi_slist_it_t it, void *ptr)
{
    _chi_slist_node_t *p = GC_MALLOC(sizeof(_chi_slist_node_t) + sizeof(void*));
    p->next = it->next;
    it->next = p;
    if (p->next->next == it)
	p->next->next = p;
    chi_slist_it_get_ptr(it) = ptr;
    return it;
}
/*-- End Clip */






cu_clop_def0(test0, void)
{
    int i, j;
    cuflow_mode_t flow;
    cuflow_set_save_current_flow(cuflow_mode_nondet, &flow);
    i = cuflow_split(3)*100;
    j = cuflow_split(4);
    printf("%d ", i+j);
    if (cuflow_mode_count() > 1)
	cuflow_split(0);
    printf("\n");
    cuflow_set_current_mode(flow);
}

/* Benchmark: split-version */

cu_clos_def(test1,
	    cu_prot0(void),
	    (chi_slist_t lst;))
{
    cu_clos_self(test1);
    chi_slist_it_t it;
    int i, j, k;
    int *ptr;
    chi_slist_cct(&self->lst);
/*     printf("test1: slist @ %p.\n", &self->lst); */
    if (cuflow_split(2) == 1)
	return;
    i = cuflow_split(200) + 1;
    j = cuflow_split(200) + 1;
    k = cuflow_split(100) + 1;
    if (i*i + j*j != k*k*k)
	cuflow_split(0);
    it = chi_slist_prepend_alloc(&self->lst, sizeof(int)*3);
    ptr = chi_slist_it_get(it);
    ptr[0] = i;
    ptr[1] = j;
    ptr[2] = k;
    cuflow_split(0);
}

/* Benchmark: inlined loop version */

cu_clos_def(test1_plain,
	    cu_prot0(void),
	    (chi_slist_t lst;))
{
    cu_clos_self(test1_plain);
    int i, j, k;
    chi_slist_cct(&self->lst);
/*     printf("test1_plain: slist @ %p.\n", &self->lst); */
    for (i = 1; i <= 200; ++i)
	for (j = 1; j <= 200; ++j)
	    for (k = 1; k <= 100; ++k) {
		chi_slist_it_t it;
		int *ptr;
		if (i*i + j*j != k*k*k)
		    continue;
		it = chi_slist_prepend_alloc(&self->lst, sizeof(int)*3);
		ptr = chi_slist_it_get(it);
		ptr[0] = i;
		ptr[1] = j;
		ptr[2] = k;
	    }
}

/* Benchmark: functional version */

cu_clos_def(test1_fnl_cntn2,
	    cu_prot(void, int k),
	    ( chi_slist_t lst;
	      int i, j; ))
{
    cu_clos_self(test1_fnl_cntn2);
    chi_slist_it_t it;
    int *ptr;
    int i = self->i;
    int j = self->j;
    if (i*i + j*j == k*k*k) {
	it = chi_slist_prepend_alloc(&self->lst, sizeof(int)*3);
	ptr = chi_slist_it_get(it);
	ptr[0] = i;
	ptr[1] = j;
	ptr[2] = k;
    }
}
cu_clos_def(test1_fnl_cntn1,
	    cu_prot(void, int j),
	    (chi_slist_t lst; int i;))
{
    cu_clos_self(test1_fnl_cntn1);
    int k;
    test1_fnl_cntn2_t cntn;
    cu_clop(cntn_clop, void, int) = test1_fnl_cntn2_prep(&cntn);
    for (k = 1; k <= 200; ++k) {
	cntn.lst = self->lst;
	cntn.i = self->i;
	cntn.j = j;
	cu_call(cntn_clop, k);
    }
}
cu_clos_def(test1_fnl_cntn0,
	    cu_prot(void, int i),
	    (chi_slist_t lst;))
{
    cu_clos_self(test1_fnl_cntn0);
    int j;
    test1_fnl_cntn1_t cntn;
    cu_clop(cntn_clop, void, int) = test1_fnl_cntn1_prep(&cntn);
    for (j = 1; j <= 200; ++j) {
	cntn.lst = self->lst;
	cntn.i = i;
	cu_call(cntn_clop, j);
    }
}
cu_clos_def(test1_fnl,
	    cu_prot0(void),
	    (chi_slist_t lst;))
{
    cu_clos_self(test1_fnl);
    int i;
    test1_fnl_cntn0_t cntn;
    cu_clop(cntn_clop, void, int) = test1_fnl_cntn0_prep(&cntn);
    chi_slist_cct(&self->lst);
    for (i = 1; i <= 200; ++i) {
	cntn.lst = self->lst;
	cu_call(cntn_clop, i);
    }
}

/* -------- */

cu_clop_def(simple, int, cu_clop(return_cl, void, void *), void *result)
{
    int arg = 191;
    cu_call(return_cl, &arg);
    assert(!"Not reached.");
    return 0;
}

int* dummy;

void
test3(int i, int inframe_p)
{
    cuflow_mode_t flow;
    int k;
    int arr[100];

    dummy = arr;
    if (i > 0) {
	test3(i - 1, inframe_p);
	return;
    }
    printf("\nCreated a large stack to see if frame breaks work.\n");

    printf("\nCall a trival function with CC "
	   "(gives the size of the big frame).\n");
    cuflow_set_save_current_flow(cuflow_mode_nondet, &flow);
    cuflow_call_with_cc(simple, &k, sizeof(k));
    cuflow_set_current_mode(flow);
    assert(k == 191);
    cuflow_continuation_print_stats();

    printf("\nDirect call (in big frame copied repeatedly).\n");
    cu_call0(test0);
    cuflow_continuation_print_stats();

    printf("\nNow, cuflow_call_in_root (big frame not copied).\n");
    cuflow_call_in_root(test0, cuflow_mode_nondet);
    cuflow_continuation_print_stats();

    printf("\nThen, cu_call_in_frame (big frame copied once).\n");
    cu_call_in_frame(test0, cuflow_mode_nondet);
    cuflow_continuation_print_stats();
}


cu_clos_def(alt_main,
	    cu_prot0(void),
	    (int argc; char** argv;))
{
    clock_t t1, t1_plain, t1_fnl;
    test1_t test1_cl;
    test1_plain_t test1_plain_cl;
    test1_fnl_t test1_fnl_cl;
    chi_slist_it_t it, it_plain;
    printf("Entering alt_main.\n");
    test3(100, 1);
    printf("\n");

    t1 = -clock();
    cuflow_call_in_root(test1_prep(&test1_cl), cuflow_mode_nondet);
    t1 += clock();

    t1_plain = -clock();
    cuflow_call_in_root(test1_plain_prep(&test1_plain_cl), cuflow_mode_nondet);
    t1_plain += clock();

    t1_fnl = -clock();
    cuflow_call_in_root(test1_fnl_prep(&test1_fnl_cl), cuflow_mode_nondet);
    t1_fnl += clock();

    it = chi_slist_begin(&test1_cl.lst);
    it_plain = chi_slist_begin(&test1_plain_cl.lst);
    while (it != chi_slist_end(&test1_cl.lst)) {
	int *ptr, *ptr_plain;
	assert(it_plain != chi_slist_end(&test1_plain_cl.lst));
	ptr = chi_slist_it_get(it);
	ptr_plain = chi_slist_it_get(it_plain);
	printf("%d^2 + %d^2 = %d^3\n", ptr[0], ptr[1], ptr[2]);
	assert(ptr[0] == ptr_plain[0] &&
	       ptr[1] == ptr_plain[1] &&
	       ptr[2] == ptr_plain[2]);
	it = chi_slist_it_next(it);
	it_plain = chi_slist_it_next(it_plain);
    }

    printf("Times: t_nondet = %lf, t_plain = %lf, ratio = %lf, t_fnl = %lf.\n",
	   t1/(double)CLOCKS_PER_SEC,
	   t1_plain/(double)CLOCKS_PER_SEC,
	   t1/(double)t1_plain,
	   t1_fnl/(double)CLOCKS_PER_SEC);

    cuflow_continuation_print_stats();
    printf("\nLeaving alt_main.\n");
}

int
main()
{
    alt_main_t alt_main_clos;
    cuflow_init();
    cuflow_call_in_root(alt_main_prep(&alt_main_clos), cuflow_mode_semidet);
    return 0;
}
