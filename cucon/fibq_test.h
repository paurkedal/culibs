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

#ifndef CUCON_FIBQ_TEST_H
#define CUCON_FIBQ_TEST_H

#include <cucon/fibq.h>
#include <cu/inherit.h>
#include <cu/memory.h>
#include <cu/test.h>
#include <limits.h>

#define MAXP_VALUE 256

CU_BEGIN_DECLARATIONS

typedef struct _fibqnode *_fibqnode_t;
struct _fibqnode
{
    cu_inherit (cucon_fibqnode);
    int value;
};

CU_SINLINE int
_fibqnode_value(cucon_fibqnode_t node)
{
    return cu_downcast(_fibqnode, cucon_fibqnode, node)->value;
}

CU_SINLINE void
_fibqnode_set_value(cucon_fibqnode_t node, int value)
{
    cu_downcast(_fibqnode, cucon_fibqnode, node)->value = value;
}

cu_clop_def(_fibq_prioreq, cu_bool_t,
	    cucon_fibqnode_t lhs, cucon_fibqnode_t rhs)
{
    return _fibqnode_value(lhs) <= _fibqnode_value(rhs);
}

static cucon_fibqnode_t
_fibqnode_new(int i)
{
    _fibqnode_t node = cu_gnew(struct _fibqnode);
    node->value = i;
    return cu_upcast(cucon_fibqnode, node);
}

static void
_fibq_insert(cucon_fibq_t Q, int i)
{
    cucon_fibq_insert(Q, _fibqnode_new(i));
}

static int
_fibq_pop(cucon_fibq_t Q)
{
    cucon_fibqnode_t node = cucon_fibq_pop_front(Q);
    if (node)
	return _fibqnode_value(node);
    else
	return -1;
}

CU_END_DECLARATIONS

#endif
