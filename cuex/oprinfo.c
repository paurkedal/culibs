/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2006--2007  Petter Urkedal <urkedal@nbi.dk>
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

#include <cuex/oprinfo.h>
#include <cuoo/meta.h>
#include <cu/diag.h>

static cu_bool_t oprinfo_initialised = cu_false;
static struct cucon_umap oprinfo_map;

cuex_oprinfo_t
cuex_oprinfo(cuex_meta_t opr)
{
    cuex_oprinfo_t oi;
    oi = (cuex_oprinfo_t)cucon_umap_find_node(&oprinfo_map, opr);
    return oi;
}

void
cuex_oprinfo_register(cuex_oprinfo_t arr)
{
    if (!oprinfo_initialised) {
	cucon_umap_init(&oprinfo_map);
	oprinfo_initialised = cu_true;
    }
    while (arr->name) {
	if (!cucon_umap_insert_init_node(&oprinfo_map, &arr->node))
	    cu_bugf("Conflicting defition for operator 0x%x = %s",
		    cuex_oprinfo_opr(arr), cuex_oprinfo_name(arr));
	++arr;
    }
}

cu_bool_t
cuex_oprinfo_conj(cu_clop(cb, cu_bool_t, cuex_oprinfo_t))
{
    return cucon_umap_conj_node(&oprinfo_map,
				(cu_clop(, cu_bool_t, cucon_umap_node_t))cb);
}

void
cuex_oprinfo_dump(cuex_oprinfo_t oi, FILE *out)
{
    cuex_meta_t opr = cuex_oprinfo_opr(oi);
    fprintf(out, "OPR 0x%08lx %2d %-30s cache_size=%ld\n",
	    (unsigned long)opr, cuex_opr_r(opr), cuex_oprinfo_name(oi),
	    (long)cuex_oprinfo_cache_size(oi));
}

cu_clos_def(_oprinfo_dump_cb,
	    cu_prot(cu_bool_t, cuex_oprinfo_t oi),
    ( FILE *out; ))
{
    cu_clos_self(_oprinfo_dump_cb);
    cuex_oprinfo_dump(oi, self->out);
    return cu_true;
}
void
cuex_oprinfo_dump_all(FILE *out)
{
    _oprinfo_dump_cb_t cb;
    cb.out = out;
    cuex_oprinfo_conj(_oprinfo_dump_cb_prep(&cb));
}
