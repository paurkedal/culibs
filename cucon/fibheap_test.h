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

#ifndef CUCON_FIBHEAP_TEST_H
#define CUCON_FIBHEAP_TEST_H

#include <cucon/fwd.h>
#include <cu/memory.h>

CU_BEGIN_DECLARATIONS

typedef struct _fibnode *_fibnode_t;
struct _fibnode
{
    cu_inherit (cucon_fibnode);
    int value;
};

CU_SINLINE int
_fibnode_value(cucon_fibnode_t node)
{
    return cu_downcast(_fibnode, cucon_fibnode, node)->value;
}

CU_SINLINE void
_fibnode_set_value(cucon_fibnode_t node, int value)
{
    cu_downcast(_fibnode, cucon_fibnode, node)->value = value;
}

cu_clop_def(_fibnode_prioreq, cu_bool_t,
	    cucon_fibnode_t lhs, cucon_fibnode_t rhs)
{
    return _fibnode_value(lhs) <= _fibnode_value(rhs);
}

static cucon_fibnode_t
_fibnode_new(int i)
{
    _fibnode_t node = cu_gnew(struct _fibnode);
    node->value = i;
    return cu_upcast(cucon_fibnode, node);
}

static void
_fibheap_insert(cucon_fibheap_t H, int i)
{
    cucon_fibheap_insert(H, _fibnode_new(i));
}

static int
_fibheap_pop(cucon_fibheap_t H)
{
    cucon_fibnode_t node = cucon_fibheap_pop_front(H);
    if (node)
	return _fibnode_value(node);
    else
	return -1;
}

CU_END_DECLARATIONS

#endif
