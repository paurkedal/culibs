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

#include <cucon/rbtree.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* #define DUMP */

cu_clos_def(int_compare,
	    cu_prot(int, void *slot),
	    (int elt;))
{
    cu_clos_self(int_compare);
    if (self->elt < *(int *)slot)
	return -1;
    else if (self->elt > *(int *)slot)
	return 1;
    else
	return 0;
}

#ifdef DUMP
void
print_int_label(void *slot, FILE *out)
{
    fprintf(out, "%d", *(int *)slot);
}
void
print_elt(void *slot)
{
    printf(" %d", *(int *)slot);
}
#endif


int
main()
{
    int_compare_t compare;
    cu_clop(compare_clop, int, void *) = int_compare_prep(&compare);
    size_t i;
#ifdef DUMP
#  define max_elt 0x80
#else
#  define max_elt 0x8000
#endif
    static int elt_arr[max_elt];
    cucon_rbtree_t tree;

    cu_init();

    tree = cucon_rbtree_new();
    memset(elt_arr, 0, sizeof(elt_arr));
    for (i = 0; i < max_elt; ++i) {
	int elt = lrand48() % max_elt;
	int *slot;
	compare.elt = elt;
	if (cucon_rbtree_insert1m_mem(tree, compare_clop,
				    sizeof(int), (void*)&slot))
	    *slot = elt;
	else
	    assert(*slot == elt);
	elt_arr[elt] = 1;
    }
#ifdef DUMP
    {
	void cu_nonclos(cb)(void *);
	cu_nonclos_cct(&cb, print_elt);
	printf("/* Ordered contents:");
	cucon_rbtree_iter_mem(tree, cu_nonclos_ref(&cb));
	printf(" */\n");
    }
#endif

    for (i = 0; i < max_elt; ++i) {
	int elt = lrand48() % max_elt;
	int *slot;
	compare.elt = elt;
	slot = cucon_rbtree_erase1m(tree, compare_clop);
	if (slot) {
	    assert(*slot == elt);
	    assert(elt_arr[elt]);
	    elt_arr[elt] = 0;
	}
	else
	    assert(!elt_arr[elt]);
    }

#ifdef DUMP
    {
	void cu_nonclos(cb)(void *, FILE *);
	cu_nonclos_cct(&cb, print_int_label);
	cucon_rbtree_dump_as_graphviz(tree, cu_nonclos_ref(&cb), stdout);
    }
#endif

    for (i = 0; i < max_elt; ++i) {
	int *slot;
	compare.elt = i;
	slot = cucon_rbtree_find1m_mem(tree, compare_clop);
	if (slot) {
	    assert(*slot == (int)i);
	    assert(elt_arr[i]);
	}
	else
	    assert(!elt_arr[i]);
    }
    return 0;
}
