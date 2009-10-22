/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2004--2007  Petter Urkedal <urkedal@nbi.dk>
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

#define CUCON_PRITREE_C
#include <cucon/pritree.h>
#include <cu/memory.h>
#include <cu/int.h>
#include <cu/debug.h>
#include <float.h>


void
cucon_pritree_init(cucon_pritree_t pritree)
{
    pritree->front = NULL;
    pritree->size = 0;
}

cucon_pritree_t
cucon_pritree_new()
{
    cucon_pritree_t pritree = cu_gnew(struct cucon_pritree_s);
    pritree->front = NULL;
    pritree->size = 0;
    return pritree;
}

CU_SINLINE void
_swap_left(cucon_prinode_t t)
{
    cucon_prinode_t tp = t->prior;
    cucon_prinode_t tl = t->left;
    cucon_prinode_t tll = tl->left;
    cucon_prinode_t tlr = tl->right;
    if (tp) {
	if (t == tp->left)
	    tp->left = tl;
	else
	    tp->right = tl;
    }
    tl->prior = tp;
    tl->left = t;
    tl->right = t->right;
    if (tl->right)
	tl->right->prior = tl;
    t->prior = tl;
    t->left = tll;
    if (tll)
	tll->prior = t;
    t->right = tlr;
    if (tlr)
	tlr->prior = t;
}
CU_SINLINE void
_swap_right(cucon_prinode_t t)
{
    cucon_prinode_t tp = t->prior;
    cucon_prinode_t tr = t->right;
    cucon_prinode_t trr = tr->right;
    cucon_prinode_t trl = tr->left;
    if (tp) {
	if (t == tp->right)
	    tp->right = tr;
	else
	    tp->left = tr;
    }
    tr->prior = tp;
    tr->right = t;
    tr->left = t->left;
    if (tr->left)
	tr->left->prior = tr;
    t->prior = tr;
    t->right = trr;
    if (trr)
	trr->prior = t;
    t->left = trl;
    if (trl)
	trl->prior = t;
}

static void
_increase_priority_to(cucon_pritree_t pritree, cucon_prinode_t t, double priority)
{
    cucon_prinode_t tp;
    t->priority = priority;
    tp = t->prior;
    if (!tp)
	return;
    while (tp->priority < priority) {
	if (t == tp->left)
	    _swap_left(tp);
	else
	    _swap_right(tp);
	assert(!t->left || t->left->prior == t);
	assert(!t->right || t->right->prior == t);
	assert(t->prior == 0 || t->prior->left == t || t->prior->right == t);
	assert(t->left || !t->right);
	tp = t->prior;
	if (!tp) {
	    pritree->front = t;
	    return;
	}
    }
}

static void
_decrease_priority_to(cucon_pritree_t pritree, cucon_prinode_t t, double priority)
{
    cucon_prinode_t tl = t->left;
    cucon_prinode_t tr = t->right;
    t->priority = priority;
    if (!t->prior && tl) {
	if (!tr) {
	    if (tl->priority <= priority)
		return;
	    pritree->front = tl;
	}
	else if (tl->priority < tr->priority) {
	    if (tr->priority <= priority)
		return;
	    pritree->front = tr;
	}
	else {
	    if (tl->priority <= priority)
		return;
	    pritree->front = tl;
	}
    }
    for (;;) {
	if (!tl) {
	    assert(!tr);
	    return;
	}
	else if (!tr) {
	    assert(!tl->left && !tl->right);
	    if (tl->priority <= priority)
		return;
	    _swap_left(t);
	    return;
	}
	else if (tl->priority >= tr->priority) {
	    if (tl->priority <= priority)
		return;
	    _swap_left(t);
	}
	else {
	    if (tr->priority <= priority)
		return;
	    _swap_right(t);
	}
	tl = t->left;
	tr = t->right;
    }
}

void
cucon_pritree_change_priority(cucon_pritree_t pritree, cucon_prinode_t t,
			    double priority)
{
    cu_debug_assert(priority < DBL_MAX && priority > -DBL_MAX);
    if (priority > t->priority)
	_increase_priority_to(pritree, t, priority);
    else if (priority < t->priority)
	_decrease_priority_to(pritree, t, priority);
}

static void
_increase_all_priorities_with(cucon_prinode_t prinode, double delta)
{
    while (prinode) {
	prinode->priority += delta;
	_increase_all_priorities_with(prinode->right, delta);
	prinode = prinode->left;
    }
}
void
cucon_pritree_add_to_all_priorities(cucon_pritree_t pritree, double delta)
{
    _increase_all_priorities_with(pritree->front, delta);
}

void
cucon_pritree_pop(cucon_pritree_t pritree)
{
    size_t n = pritree->size--;
    if (n == 1)
	pritree->front = NULL;
    else {
	unsigned int l;
	size_t m;
	cucon_prinode_t prinode = pritree->front;
	prinode->priority = -DBL_MAX;

	l = cu_ulong_floor_log2(n);
	m = 1 << (l - 1);
	if ((n & m) == 0) {
	    cucon_prinode_t tl = prinode->left;
	    _swap_left(prinode);
	    _decrease_priority_to(pritree, tl, tl->priority);
	}
	else {
	    cucon_prinode_t tr = prinode->right;
	    _swap_right(prinode);
	    _decrease_priority_to(pritree, tr, tr->priority);
	}
	pritree->front = prinode->prior;
	while (m >>= 1, m > 0) {
	    if ((n & m) == 0) {
		cucon_prinode_t tl = prinode->left;
		_swap_left(prinode);
		_decrease_priority_to(pritree, tl, tl->priority);
	    }
	    else {
		cucon_prinode_t tr = prinode->right;
		_swap_right(prinode);
		_decrease_priority_to(pritree, tr, tr->priority);
	    }
	}
	assert(!prinode->left && !prinode->right);
	if (prinode == prinode->prior->left) {
	    assert(!prinode->prior->right);
	    prinode->prior->left = NULL;
	}
	else
	    prinode->prior->right = NULL;
    }
}

void
cucon_pritree_erase(cucon_pritree_t pritree, cucon_prinode_t prinode)
{
    _increase_priority_to(pritree, prinode, DBL_MAX);
    cucon_pritree_pop(pritree);
}

void
cucon_pritree_insert_init(cucon_pritree_t pritree, cucon_prinode_t newprinode,
			  double priority)
{
    cucon_prinode_t t = pritree->front;
    size_t n = ++pritree->size;
    unsigned int l;
    size_t m;
    if (n == 1) {
	pritree->front = newprinode;
	newprinode->priority = priority;
	newprinode->prior = NULL;
	newprinode->left = NULL;
	newprinode->right = NULL;
	return;
    }
    else {
	l = cu_ulong_floor_log2(n);
	m = 1 << l;
	while (m >>= 1, m > 1) {
	    assert(t);
	    if ((n & m) == 0)
		t = t->left;
	    else
		t = t->right;
	}
	assert(t);
	if ((n & m) == 0) {
	    assert(!t->left);
	    assert(!t->right);
	    t->left = newprinode;
	}
	else {
	    assert(t->left);
	    assert(!t->right);
	    t->right = newprinode;
	}
	newprinode->prior = t;
	newprinode->left = NULL;
	newprinode->right = NULL;
	_increase_priority_to(pritree, newprinode, priority);
    }
}

cu_bool_t
cucon_prinode_conj_down_to(cucon_prinode_t t, double min_priority,
			   cu_clop(cb, cu_bool_t, cucon_prinode_t))
{
    while (t->priority >= min_priority) {
	if (t->left && !cucon_prinode_conj_down_to(t->left, min_priority, cb))
	    return cu_false;
	if (!cu_call(cb, t))
	    return cu_false;
	if (!t->right)
	    break;
	t = t->right;
    }
    return cu_true;
}

static void
_dump_priorities(cucon_prinode_t t, FILE *out, int level)
{
    while (t) {
	fprintf(out, "\t%4d %10.3lg\n", level, t->priority);
	if (t->left)
	    _dump_priorities(t->left, out, level + 1);
	++level;
	t = t->right;
    }
}

void
cucon_pritree_dump_priorities(cucon_pritree_t pritree, FILE *out)
{
    fprintf(out, "pritree @ %p\n", (void *)pritree);
    _dump_priorities(pritree->front, out, 0);
}
