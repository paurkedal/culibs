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

/* Test constructive version of insert */

#include <cucon/rbtree.h>
#include <cu/memory.h>
#include <assert.h>
#include <time.h>

static clock_t t_ctive = 0;
static clock_t t_dtive = 0;

cu_clop_def(intptr_cmp, int, int *i, int *j)
{
    if (*i < *j)
	return -1;
    else if (*i > *j)
	return 1;
    else
	return 0;
}

void
insert_some(cucon_rbtree_t tree_ctive, cucon_rbtree_t tree_dtive)
{
    int const I = 400;
    int i;
    for (i = 0; i < I; ++i) {
	cu_bool_fast_t b0, b1;
	int *n = cu_gnew(int);
	*n = lrand48() % I;
	t_dtive -= clock();
	b0 = cucon_rbtree_insert2p_ptr(tree_dtive,
			(cu_clop(, int, void *, void *))intptr_cmp, &n);
	t_dtive += clock();
	t_ctive -= clock();
	b1 = cucon_rbtree_cinsert2p_ptr(tree_ctive,
			(cu_clop(, int, void *, void *))intptr_cmp, &n);
	t_ctive += clock();
	assert(b0 == b1);
    }
}

int
main()
{
    int i;
    cucon_rbtree_t tree_ctive;
    cucon_rbtree_t tree_dtive;
    cucon_rbtree_t tree_clone;
    cucon_rbtree_t tree_copy;
    cu_init();
    for (i = 0; i < 200; ++i) {
	tree_ctive = cucon_rbtree_new();
	tree_dtive = cucon_rbtree_new();
	insert_some(tree_ctive, tree_dtive);
	tree_clone = cucon_rbtree_new_copy_ctive(tree_ctive);
	tree_copy = cucon_rbtree_new_copy_ptr(tree_dtive);
	insert_some(tree_clone, tree_copy);
	insert_some(tree_ctive, tree_dtive);
    }
    printf("t_ctive = %lf s, t_dtive = %lf s, ratio = %lf\n",
	   t_ctive/(double)CLOCKS_PER_SEC, t_dtive/(double)CLOCKS_PER_SEC,
	   t_ctive/(double)t_dtive);
    return 0;
}
