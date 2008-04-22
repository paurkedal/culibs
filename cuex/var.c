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

#include <cuex/var.h>
#include <cucon/umap.h>

#define CUEXP_SUBKIND_DYN_START (CUEX_META_C(1) << (CUEXP_VARMETA_INDEX_WIDTH - 1))

struct cucon_umap_s cuexP_xvarops;
static cuex_meta_t cuexP_next_subkind = CUEXP_SUBKIND_DYN_START;

cuex_meta_t
cuex_register_xvarkind(cuex_meta_t subkind, unsigned int wsize,
		       cuex_xvarops_t ops)
{
    cuex_xvarops_t *slot;
    if (subkind == (cuex_meta_t)-1)
	subkind = cuexP_next_subkind++;
    else if (subkind >= CUEXP_SUBKIND_DYN_START)
	cu_bugf("Subkind 0x%x is too big for registration of extended "
		"variable kind.", subkind);
    if (!cucon_umap_insert_mem(&cuexP_xvarops, subkind,
			       sizeof(cuex_xvarops_t), &slot))
	cu_bugf("Conflicting subkind 0x%x in registration of extended "
		"variable kind.", subkind);
    *slot = ops;
    return subkind;
}

void
cuexP_var_init(void)
{
    cucon_umap_init(&cuexP_xvarops);
}
