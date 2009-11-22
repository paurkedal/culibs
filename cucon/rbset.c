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

#include <cucon/rbset.h>
#include <cu/memory.h>
#include <cu/str.h>
#include <cu/clos.h>

void
cucon_rbset_init(cucon_rbset_t rbset, cu_clop(cmp, int, void *, void *))
{
    cucon_rbtree_init(cu_to(cucon_rbtree, rbset));
    rbset->cmp = cmp;
}

cucon_rbset_t
cucon_rbset_new(cu_clop(cmp, int, void *, void *))
{
    cucon_rbset_t rbset = cu_gnew(struct cucon_rbset);
    cucon_rbtree_init(cu_to(cucon_rbtree, rbset));
    rbset->cmp = cmp;
    return rbset;
}

void
cucon_rbset_init_str_cmp(cucon_rbset_t rbset)
{
    cucon_rbset_init(rbset, (cu_clop(, int, void *, void *))cu_str_cmp_clop);
}

cucon_rbset_t
cucon_rbset_new_str_cmp(void)
{
    return cucon_rbset_new((cu_clop(, int, void *, void *))cu_str_cmp_clop);
}

void
cucon_rbset_nearest(cucon_rbset_t set, void *key,
		    cu_ptr_ptr_t below_out,
		    cu_ptr_ptr_t equal_out,
		    cu_ptr_ptr_t above_out)
{
    cucon_rbnode_t below, equal, above;
    cucon_rbtree_nearest2p(cu_to(cucon_rbtree, set), set->cmp,
			   key, &below, &equal, &above);
    if (below)
	*(void **)below_out = cucon_rbnode_ptr(below);
    else
	*(void **)below_out = NULL;
    if (equal)
	*(void **)equal_out = cucon_rbnode_ptr(equal);
    else
	*(void **)equal_out = NULL;
    if (above)
	*(void **)above_out = cucon_rbnode_ptr(above);
    else
	*(void **)above_out = NULL;
}

