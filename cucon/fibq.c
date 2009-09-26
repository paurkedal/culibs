/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2009  Petter Urkedal <urkedal@nbi.dk>
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

#include <cucon/fibq.h>
#include <cu/memory.h>
#include <cu/size.h>
#include <string.h>

#define _prioreq(Q, n0, n1) cu_call((Q)->prioreq, n0, n1)

void
cucon_fibq_init(cucon_fibq_t Q, cucon_fibq_prioreq_t prioreq)
{
    Q->prioreq = prioreq;
    Q->card = 0;
    Q->roots = NULL;
    Q->root_tail = &Q->roots;
}

cucon_fibq_t
cucon_fibq_new(cucon_fibq_prioreq_t prioreq)
{
    cucon_fibq_t Q = cu_gnew(struct cucon_fibq);
    cucon_fibq_init(Q, prioreq);
    return Q;
}

size_t cuconP_fib(int n);

static size_t
_validate_node(cucon_fibqnode_t x, int max_degree)
{
    cucon_fibqnode_t y;
    size_t count;
    int degree;

    y = x->children;
    count = 1;
    degree = 0;
    while (y) {
	cu_debug_assert(y != x);
	count += _validate_node(y, max_degree);
	++degree;
	y = y->siblings;
    }

    cu_debug_assert(x->degree == degree);
    cu_debug_assert(degree <= max_degree);
    cu_debug_assert(count >= cuconP_fib(degree + 2));

    return count;
}

void
cucon_fibq_validate(cucon_fibq_t Q)
{
    size_t count = 0;

    if (Q->card == 0) {
	cu_debug_assert(!Q->roots);
	cu_debug_assert(Q->root_tail == &Q->roots);
    }
    else if (Q->card == 1) {
	cu_debug_assert(Q->roots);
	cu_debug_assert(!Q->roots->siblings);
	cu_debug_assert(!Q->roots->children);
	cu_debug_assert(!Q->roots->mark);
	cu_debug_assert(Q->roots->degree == 0);
    }
    else {
	int max_degree = cu_size_ceil_log2(Q->card);
	cucon_fibqnode_t x, *root_slot = &Q->roots;
	while ((x = *root_slot)) {
	    cu_debug_assert(!x->mark);
	    count += _validate_node(x, max_degree);
	    root_slot= &x->siblings;
	}
	cu_debug_assert(root_slot == Q->root_tail);
	cu_debug_assert(count == Q->card);
    }
}

void
cucon_fibq_insert(cucon_fibq_t Q, cucon_fibqnode_t node)
{
    node->children = NULL;
    node->mark = cu_false;
    node->degree = 0;
    if (Q->roots && _prioreq(Q, node, Q->roots)) {
	node->siblings = Q->roots;
	Q->roots = node;
    }
    else {
	node->siblings = NULL;
	*Q->root_tail = node;
	Q->root_tail = &node->siblings;
    }
    ++Q->card;
}

cucon_fibqnode_t
cucon_fibq_pop_front(cucon_fibq_t Q)
{
    int maxp_degree, degree;
    cucon_fibqnode_t pop_node = Q->roots;
    cucon_fibqnode_t root, roots, min_root;
    cucon_fibqnode_t *nodes_by_degree;

    /* Handle simple cases. Thus, we can assmume below that card ≥ 2 after
     * removal. */
    switch (Q->card) {
	case 0:
	    cu_debug_assert(pop_node == NULL);
	    return NULL;
	case 1:
	    cu_debug_assert(pop_node != NULL);
	    cu_debug_assert(pop_node->children == NULL);
	    cu_debug_assert(pop_node->siblings == NULL);
	    Q->card = 0;
	    Q->roots = NULL;
	    Q->root_tail = &Q->roots;
	    return pop_node;
	case 2:
	    cu_debug_assert(pop_node != NULL);
	    Q->card = 1;
	    if (pop_node->siblings)
		Q->roots = pop_node->siblings;
	    else {
		Q->roots = pop_node->children;
		Q->roots->mark = cu_false;
		Q->root_tail = &Q->roots->siblings;
	    }
	    return pop_node;
    }
    cu_debug_assert(pop_node != NULL);

    /* Prepare for iterating over the current roots except pop_node followed by
     * the children of pop_node. The root_tail link is fixed later. */
    if (pop_node->children) {
	*Q->root_tail = pop_node->children;
	CU_GWIPE(pop_node->children);
    }
    root = pop_node->siblings;

    /* Pair up same-degree root nodes until all have different degrees. Store
     * them temporarily in a log N sized array, at most one per element. The
     * sibling links will be fixed in the next step. */
    maxp_degree = cu_size_ceil_log2(Q->card--);
    nodes_by_degree = cu_snewarr(cucon_fibqnode_t, maxp_degree);
    memset(nodes_by_degree, 0, sizeof(cucon_fibqnode_t)*maxp_degree);
    nodes_by_degree[root->degree] = root;
    root = root->siblings;
    while (root) {
	cucon_fibqnode_t y, next_root = root->siblings;
	degree = root->degree;
	cu_debug_assert(degree < maxp_degree);

	root->mark = cu_false; /* Since we also process children of pop_node. */
	while ((y = nodes_by_degree[degree])) {
	    cucon_fibqnode_t child;
	    nodes_by_degree[degree] = NULL;
	    if (!_prioreq(Q, y, root))
		child = y;
	    else {
		child = root;
		root = y;
	    }
	    child->siblings = root->children;
	    root->children = child;
	    root->degree = ++degree;
	    cu_debug_assert(degree < maxp_degree);
	    cu_debug_assert(root->mark == cu_false);
	}
	cu_debug_assert(root->degree == degree);
	nodes_by_degree[degree] = root;

	root = next_root;
    }

    /* Re-create the root chain from the array, fixing the sibling links. First
     * fetch two elements to simplify the remaning loop below. */
    degree = 0;
    do {
	cu_debug_assert(degree < maxp_degree);
	min_root = nodes_by_degree[degree++];
    } while (!min_root);
    roots = NULL;
    do {
	if (degree == maxp_degree) {
	    /* Handle the single-root case. */
	    min_root->siblings = NULL;
	    Q->roots = min_root;
	    Q->root_tail = &min_root->siblings;
	    return pop_node;
	}
	root = nodes_by_degree[degree++];
    } while (!root);
    if (_prioreq(Q, min_root, root))
	roots = root;
    else {
	roots = min_root;
	min_root = root;
    }
    roots->siblings = NULL;
    Q->root_tail = &roots->siblings;

    /* Now we have a preliminary min_root and the last of roots. Prepend each
     * of the rest on roots after swapping with min_root when needed. */
    while (degree < maxp_degree) {
	if ((root = nodes_by_degree[degree++])) {
	    if (!_prioreq(Q, min_root, root)) {
		min_root->siblings = roots;
		roots = min_root;
		min_root = root;
	    }
	    else {
		root->siblings = roots;
		roots = root;
	    }
	}
    }
    min_root->siblings = roots;

    /* Update Q. */
    Q->roots = min_root;
    cu_debug_assert((Q->card == 0) == (Q->roots == NULL));
    cu_debug_assert(!*Q->root_tail);

    return pop_node;
}
