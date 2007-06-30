/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2005--2007  Petter Urkedal <urkedal@nbi.dk>
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

#include <cuex/fwd.h>
#include <cugra/fwd.h>
#include <cuex/var.h>
#include <cuex/tpvar.h>
#include <cuex/tvar.h>
#include <cuex/opn.h>
#include <cuex/oprinfo.h>
#include <cuex/dunion.h>
#include <cudyn/properties.h>
#include <cucon/pmap.h>
#include <cucon/pset.h>
#include <cucon/stack.h>
#include <cu/va_ref.h>
#include <cu/diag.h>

#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

/* This printing is meant for debugging purposes.  It will lock all printed
 * variables from being GCed.  That could be avoided by using cucon_wmap_t
 * instead of cucon_pmap_t in varindex, but then two variables which prints
 * the same may in fact be different. */

static int
varindex(cuex_t var)
{
    static cucon_pmap_t index_map = NULL;
    static int current_index[] = {0, 0, 0, 0};
    static cu_mutex_t mutex = CU_MUTEX_INITIALISER;
    int *i;
    cuex_qcode_t qc = cuex_varmeta_qcode(cuex_meta(var));
    cu_debug_assert(qc < 4);
    cu_mutex_lock(&mutex);
    if (!index_map)
	index_map = cucon_pmap_new();
    if (cucon_pmap_insert_mem(index_map, var, sizeof(int), &i))
	*i = ++current_index[qc];
    cu_mutex_unlock(&mutex);
    return *i;
}

static void
cuex_print_ex(cuex_t ex, FILE *out)
{
    cuex_meta_t meta = cuex_meta(ex);
    switch (cuex_meta_kind(meta)) {
	    cuex_oprinfo_t oi;
	    int i, r;
	    int oa_cnt;
	    int oa_arr[CUEX_OPRDEFS_OA_COUNT_MAX];
	case cuex_meta_kind_opr:
	    meta = cuex_oprdefs_decode_opr(meta, &oa_cnt, oa_arr);
	    oi = cuex_oprinfo(meta);
	    if (oi)
		fprintf(out, "(#%s", cuex_oprinfo_name(oi));
	    else
		fprintf(out, "(#%lx", (unsigned long)meta);
	    for (i = 0; i < oa_cnt; ++i)
		fprintf(out, ":%d", oa_arr[i]);
	    r = cuex_opr_r(meta);
	    for (i = 0; i < r; ++i) {
		fputc(' ', out);
		cuex_print_ex(cuex_opn_at(ex, i), out);
	    }
	    fputc(')', out);
	    break;
	case cuex_meta_kind_type:
	    if (!cudyn_raw_print(ex, out))
		fprintf(out, "C%p", ex);
	    break;
	default:
	    if (cuex_is_rvarmeta(meta))
		fprintf(out, "μ%u", (unsigned int)cuex_varmeta_index(meta));
	    else if (cuex_is_varmeta(meta)) {
		char *prefix;
		if (cuex_is_tvarmeta(meta) && cuex_tvar_is_type(ex))
		    switch (cuex_varmeta_qcode(meta)) {
			case cuex_qcode_u:
			    prefix = "α";
			    break;
			case cuex_qcode_e:
			    prefix = "ξ";
			    break;
			default:
			    prefix = "ζ";
			    break;
		    }
		else
		    switch (cuex_varmeta_qcode(meta)) {
			case cuex_qcode_u:
			    prefix = "u";
			    break;
			case cuex_qcode_e:
			    prefix = "e";
			    break;
			case cuex_qcode_w:
			    prefix = "w";
			    break;
			case cuex_qcode_n:
			    prefix = "n";
			    break;
		    }
		if (cuex_is_ivarmeta(meta))
		    fprintf(out, "%s%d", prefix, cuex_varmeta_index(meta));
		else
		    fprintf(out, "%s#%d", prefix, varindex(ex));
	    }
	    else
		fprintf(out, "special_%p", ex);
    }
}

cu_clop_def(format_expression, void, cu_str_t fmt, cu_va_ref_t va, FILE *out)
{
    cuex_t ex = cu_va_ref_arg(va, cuex_t);
    cuex_print_ex(ex, out);
}

static cuex_t
cuexP_write_dot_decls(cuex_t e, FILE *out, cucon_pset_t done, cucon_stack_t bsk)
{
    cuex_meta_t e_meta = cuex_meta(e);
    cu_bool_t is_new = cucon_pset_insert(done, e);
    if (cuex_og_binder_contains(e_meta)) {
	if (e_meta == CUEX_O1_MU) {
	    *(cuex_t *)cucon_stack_alloc(bsk, sizeof(cuex_t)) = e;
	    e = cuex_opn_at(e, 0);
	    cuexP_write_dot_decls(e, out, done, bsk);
	    cucon_stack_free(bsk, sizeof(cuex_t));
	    return e;
	}
	*(cuex_t *)cucon_stack_alloc(bsk, sizeof(cuex_t)) = e;
    }
    if (cuex_meta_is_opr(e_meta)) {
	char const *vertex_attrs = "";
	cu_rank_t i, r = cuex_opr_r(e_meta);
	cuex_oprinfo_t oi;
	for (i = 0; i < r; ++i) {
	    cuex_t ep = cuex_opn_at(e, i);
	    cuex_meta_t ep_meta = cuex_meta(ep);
	    if (cuex_og_hole_contains(ep_meta)) {
		int j = cuex_oa_hole_index(ep_meta);
		if (j < cucon_stack_size(bsk)/sizeof(cuex_t)) {
		    cuex_t epp;
		    char *edge_attrs;
		    epp = *(cuex_t *)cucon_stack_at(bsk, j*sizeof(cuex_t));
		    if (cucon_pset_insert(done, cuex_o2_internal0(e, epp))) {
			if (cuex_meta(epp) == CUEX_O1_MU) {
			    edge_attrs = "style=dashed, arrowhead=dot, arrowtail=nomal, ";
			    epp = cuex_opn_at(epp, 0);
			} else
			    edge_attrs = "style=dotted, arrowhead=dot, arrowtail=odiamond, ";
			fprintf(out,
				"_%p->_%p [%sdir=back, headlabel=\"%d\"];\n",
				epp, e, edge_attrs, i);
		    }
		}
		else if (is_new)
		    fprintf(out,
			    "_%p->_%p [dir=back, style=dotted, "
			    "headlabel=\"%d\", arrowtail=odot];\n",
			    ep, e, i);
	    }
	    else {
		ep = cuexP_write_dot_decls(ep, out, done, bsk);
		if (is_new)
		    fprintf(out, "_%p->_%p [taillabel=\"%d\"];\n", e, ep, i);
	    }
	}
	if (is_new) {
	    oi = cuex_oprinfo(e_meta);
	    if (cuex_og_binder_contains(e_meta))
		vertex_attrs = "style=filled,fillcolor=\"#99ff99\",";
	    if (oi)
		fprintf(out, "_%p [%slabel=\"%s\"];\n",
			e, vertex_attrs, oi->name);
	    else
		fprintf(out, "_%p [%slabel=\"%"CUEX_PRIxMETA"\"];\n",
			e, vertex_attrs, e_meta);
	}
    }
    else if (is_new)
	cu_fprintf(out, "_%p [label=\"%!\"];\n", e, e);
    if (cuex_og_binder_contains(e_meta))
	cucon_stack_free(bsk, sizeof(cuex_t));
    return e;
}

void
cuex_write_dot_decls(cuex_t e, FILE *out)
{
    struct cucon_pset_s done;
    struct cucon_stack_s bsk;
    cucon_pset_cct(&done);
    cucon_stack_cct(&bsk);
    cuexP_write_dot_decls(e, out, &done, &bsk);
}

cu_bool_t
cuex_save_dot(cuex_t e, char const *graph_name, char const *path)
{
    FILE *out;
    if (!graph_name)
	graph_name = "expression";
    if (!path) {
	char tmp_path[] = "/tmp/cuex-expression.dot.XXXXXX";
	int fd;
	fd = mkstemp(tmp_path);
	if (fd == -1) {
	    cu_errf("Could not save to %s.", path);
	    return cu_false;
	}
	close(fd);
	path = tmp_path;
	cu_verbf(0, "Saving expression to %s", tmp_path);
    }
    out = fopen(path, "w");
    if (!out) {
	cu_errf("Could not save to %s.", path);
	return cu_false;
    }
    cu_fprintf(out, "# %!\n", e);
    fprintf(out,
	    "digraph %s {\n"
	    "bgcolor=\"#ffffff\";\n"
	    "node[width=0, height=0, style=filled, fillcolor=\"#dddddd\"];\n"
	    "edge[arrowtail=dot, arrowhead=none];\n",
	    graph_name);
    cuex_write_dot_decls(e, out);
    fprintf(out, "}\n");
    fclose(out);
    return cu_true;
}

void
cuexP_print_init()
{
    cu_diag_define_format_key('!', format_expression);
}
