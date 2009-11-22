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

#include <cucon/fibheap.h>
#include <cu/size.h>
#include <cu/memory.h>
#include <cu/util.h>
#include <string.h>
#include <math.h>

#define _prioreq(H, n0, n1) cu_call(H->prioreq, n0, n1)
#define _node(n)	cu_downcast(cucon_fibnode, cu_dlink, n)
#define _dlink(n)	cu_to(cu_dlink, n)
#define _next_node(n)	_node(_dlink(n)->next)

void
cucon_fibheap_init(cucon_fibheap_t H, cucon_fibnode_prioreq_t prioreq)
{
    H->prioreq = prioreq;
    H->card = 0;
    H->min_root = NULL;
}

cucon_fibheap_t
cucon_fibheap_new(cucon_fibnode_prioreq_t prioreq)
{
    cucon_fibheap_t H = cu_gnew(struct cucon_fibheap);
    cucon_fibheap_init(H, prioreq);
    return H;
}

#if 0 /* There is no other reason to pull in libm at the moment ... */
size_t
cuconP_fib(int n)
{
    const double phi = 1.61803398874989484820L;
    const double inv_sqrt5 = 0.44721359549995793928L;
    return (size_t)ceil(inv_sqrt5*pow(phi, n) + 0.5);
}
#else /* ... so do it the slow way. */
size_t
cuconP_fib(int n)
{
    int i;
    size_t f[2] = {0, 1};
    for (i = 2; i <= n; ++i)
	f[i % 2] += f[(i - 1) % 2];
    return f[n % 2];
}
#endif

static size_t
_validate_node(cucon_fibnode_t x, int max_degree)
{
    cucon_fibnode_t y, y0;
    size_t count;
    int degree;

    y = y0 = x->children;
    if (y == NULL)
	return 1;
    cu_debug_assert(!cu_dlink_cocyclic(_dlink(x), _dlink(y)));
    cu_dlink_validate(_dlink(y));

    /* Check children and count nodes. */
    count = 1;
    degree = 0;
    do {
	cu_debug_assert(y->parent == x);
	count += _validate_node(y, max_degree);
	++degree;
	y = _next_node(y);
    }
    while (y != y0);

    /* Check degree and node count. */
    cu_debug_assert(x->degree == degree);
    cu_debug_assert(degree <= max_degree);
    cu_debug_assert(count >= cuconP_fib(degree + 2));

    return count;
}

void
cucon_fibheap_validate(cucon_fibheap_t H)
{
    if (H->card == 0) {
	cu_debug_assert(!H->min_root);
    }
    else if (H->card == 1) {
	cu_debug_assert(H->min_root);
	cu_debug_assert(cu_dlink_is_singleton(_dlink(H->min_root)));
	cu_debug_assert(H->min_root->children == NULL);
    }
    else {
	int max_degree = 2*cu_size_ceil_log2(H->card);  /* See below. */
	cucon_fibnode_t x, x0;
	x = x0 = H->min_root;
	do {
	    cu_debug_assert(!x->mark);
	    cu_debug_assert(!x->parent);
	    _validate_node(x, max_degree);
	    x = _next_node(x);
	} while (x != x0);
    }
}

void
cucon_fibheap_insert(cucon_fibheap_t H, cucon_fibnode_t node)
{
    node->parent = NULL;
    node->children = NULL;
    node->mark = cu_false;
    node->degree = 0;
    if (H->min_root == NULL) {
	cu_debug_assert(H->card == 0);
	cu_dlink_init_singleton(_dlink(node));
	H->min_root = node;
	H->card = 1;
    }
    else {
	cu_dlink_insert_before(_dlink(H->min_root), _dlink(node));
	if (!_prioreq(H, H->min_root, node))
	    H->min_root = node;
	++H->card;
    }
}

void
cucon_fibheap_swap(cucon_fibheap_t H0, cucon_fibheap_t H1)
{
    CU_SWAP(cucon_fibnode_prioreq_t, H0->prioreq, H1->prioreq);
    CU_SWAP(size_t, H0->card, H1->card);
    CU_SWAP(cucon_fibnode_t, H0->min_root, H1->min_root);
}

void
cucon_fibheap_union_d(cucon_fibheap_t H0, cucon_fibheap_t H1_d)
{
    if (H1_d->min_root != NULL) {
	if (H0->min_root == NULL) {
	    H0->card = H1_d->card;
	    H0->min_root = H1_d->min_root;
	}
	else {
	    cu_dlink_splice_before(_dlink(H0->min_root),
				   _dlink(H1_d->min_root));
	    if (!_prioreq(H0, H0->min_root, H1_d->min_root))
		H0->min_root = H1_d->min_root;
	    H0->card += H1_d->card;
	}
	CU_GWIPE(H1_d->min_root);
    }
}

static void
_move_below(cucon_fibnode_t parent, cucon_fibnode_t node)
{
    cu_debug_assert(node != parent);
    if (parent->children)
	cu_dlink_move_before(_dlink(parent->children), _dlink(node));
    else {
	cu_dlink_extract(_dlink(node));
	parent->children = node;
    }
    node->parent = parent;
    ++parent->degree;

    /* Only used for root nodes, otherwise we would clear the mark. */
    cu_debug_assert(!parent->mark);
}

cucon_fibnode_t
cucon_fibheap_pop_front(cucon_fibheap_t H)
{
    cucon_fibnode_t rm_node = H->min_root;
    cucon_fibnode_t min_root;
    cucon_fibnode_t x, xs;
    cucon_fibnode_t *node_by_degree;
    size_t maxp_degree;

    if (!rm_node)
	return NULL;

    /* Splice children of rm_node into roots, and handle singleton case. */
    if (rm_node->children) {
	/* Clear parent links. */
	x = xs = rm_node->children;
	do {
	    x->parent = NULL;
	    x = _next_node(x);
	} while (x != xs);

	/* Splice with roots. */
	cu_dlink_splice_before(_dlink(rm_node), _dlink(rm_node->children));
	CU_GWIPE(rm_node->children);
	cu_debug_assert(_next_node(rm_node) != rm_node);
    }
    else if (_next_node(rm_node) == rm_node) {
	/* Singleton case. */
	cu_debug_assert(H->card == 1);
	H->min_root = NULL;
	H->card = 0;
	return rm_node;
    }
    cu_debug_assert(H->card >= 2);

    /* Pairwise link same-degree root nodes until they have different
     * degree.  Let ϕ = (1 + sqrt 5)/2, then the degree is bounded by
     *
     *     degree ≤ logϕ n = log2 n / log2 ϕ < 2 log2 n
     */
    maxp_degree = 2*cu_size_ceil_log2(H->card);
    node_by_degree = cu_snewarr(cucon_fibnode_t, maxp_degree);
    memset(node_by_degree, 0, sizeof(cucon_fibnode_t)*maxp_degree);
    x = min_root = _next_node(rm_node);
    cu_debug_assert(x != rm_node);
    do {
	int degree = x->degree;
	cucon_fibnode_t y;
	cu_debug_assert(degree < maxp_degree);
	y = node_by_degree[degree];
	if (y) {
	    node_by_degree[degree] = NULL;
	    if (_prioreq(H, x, y))
		_move_below(x, y);
	    else {
		cu_dlink_move_after(_dlink(x), _dlink(y));
		_move_below(y, x);
		x = y;
	    }
	    continue;
	}
	node_by_degree[degree] = x;
	if (_prioreq(H, x, min_root))
	    min_root = x;
	x = _next_node(x);
    } while (x != rm_node);

    /* Remove rm_node and update the heap struct. */
    cu_dlink_erase(_dlink(rm_node));
    H->min_root = min_root;
    --H->card;

    return rm_node;
}

static void
_move_to_root(cucon_fibheap_t H, cucon_fibnode_t node)
{
    do {
	/* Move to root. */
	cu_dlink_move_before(_dlink(H->min_root), _dlink(node));
	if (!_prioreq(H, H->min_root, node))
	    H->min_root = node;
	node->mark = cu_false;

	/* Fix parent node. */
	node = node->parent;
	node->parent = NULL;
	--node->degree;
    }
    while (node->mark);
}

void
cucon_fibheap_prioritise(cucon_fibheap_t H, cucon_fibnode_t node)
{
    if (node->parent && !_prioreq(H, node->parent, node))
	_move_to_root(H, node);
}

void
cucon_fibheap_remove(cucon_fibheap_t H, cucon_fibnode_t node)
{
    cucon_fibnode_t children;

    /* Handle removal of highest priority node as a special case, since
     * otherwise we can skip the update of min_root. */
    if (node == H->min_root) {
	cucon_fibheap_pop_front(H);
	return;
    }

    /* Unlink the node and move marked parent nodes to root. */
    cu_dlink_erase(_dlink(node));
    if (node->parent) {
	if (node->parent->mark)
	    _move_to_root(H, node->parent);
	CU_GWIPE(node->parent);
    }

    /* Merge children into roots.  They must have lower priority than
     * min_root. */
    cu_dlink_erase(_dlink(node));
    children = node->children;
    if (children) {
	cucon_fibnode_t x = children;
	do {
	    x->parent = NULL;
	    x->mark = cu_false;
	    x = _next_node(x);
	} while (x != children);
    }
    cu_dlink_splice_before(_dlink(H->min_root), _dlink(children));
    CU_GWIPE(node->children);
}

static cu_bool_t
_iterA(cucon_fibnode_t xs, cu_clop(f, cu_bool_t, cucon_fibnode_t))
{
    if (xs) {
	cucon_fibnode_t x;

	if (!cu_call(f, xs))
	    return cu_false;

	x = xs;
	do {
	    if (!_iterA(x, f))
		return cu_false;
	    x = _next_node(x);
	}
	while (x != xs);
    }
    return cu_true;
}

cu_bool_t
cucon_fibheap_iterA(cucon_fibheap_t H, cu_clop(f, cu_bool_t, cucon_fibnode_t))
{
    return _iterA(H->min_root, f);
}
