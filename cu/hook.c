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

#include <cu/hook.h>
#include <cu/memory.h>

void
cuP_hook_init(cu_hook_node_t hook)
{
    cu_dlink_init_singleton(cu_to(cu_dlink, hook));
}

void
cuP_hook_prepend(cu_hook_node_t hook, cu_clop0(f, void))
{
    cu_hook_node_t node = cu_gnew(struct cu_hook_node);
    node->f = f;
    cu_dlink_insert_after(cu_to(cu_dlink, hook), cu_to(cu_dlink, node));
}

void
cuP_hook_append(cu_hook_node_t hook, cu_clop0(f, void))
{
    cu_hook_node_t node = cu_gnew(struct cu_hook_node);
    node->f = f;
    cu_dlink_insert_before(cu_to(cu_dlink, hook), cu_to(cu_dlink, node));
}

void
cu_iter_hook_call(cu_iter_hook_t hook)
{
    cu_hook_node_t node;
    CU_HOOK_FOR(node, hook)
	cu_call0(node->f);
}

cu_bool_t
cu_iterA_hook_call(cu_iterA_hook_t hook)
{
    cu_hook_node_t node;
    CU_HOOK_FOR(node, hook)
	if (!cu_call0((cu_clop0(, cu_bool_t))node->f))
	    return cu_false;
    return cu_true;
}

void
cu_iter_ptr_hook_call(cu_iter_ptr_hook_t hook, void *arg)
{
    cu_hook_node_t node;
    CU_HOOK_FOR(node, hook)
	cu_call((cu_clop(, void, void *))node->f, arg);
}

cu_bool_t
cu_iterA_ptr_hook_call(cu_iterA_ptr_hook_t hook, void *arg)
{
    cu_hook_node_t node;
    CU_HOOK_FOR(node, hook)
	if (!cu_call((cu_clop(, cu_bool_t, void *))node->f, arg))
	    return cu_false;
    return cu_true;
}

void *
cu_ptr_compose_hook_call(cu_ptr_compose_hook_t hook, void *arg)
{
    cu_hook_node_t node;
    CU_HOOK_FOR(node, hook) {
	if (!arg)
	    return NULL;
	arg = cu_call((cu_clop(, void *, void *))node->f, arg);
    }
    return arg;
}
