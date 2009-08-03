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

#include <cugra/graph.h>
#include <cugra/graph_algo.h>
#include <cu/diag.h>

void
cugra_graph_fwrite_dot(cugra_graph_t G,
		       cu_clop(vertex_label, cu_str_t, cugra_vertex_t v),
		       cu_clop(arc_label, cu_str_t, cugra_arc_t a),
		       FILE *fout)
{
    cugra_vertex_t v;
    char const *arc_fmt;
    fprintf(fout, "digraph g%p {\n", (void *)G);
    if (cu_clop_is_null(arc_label))
	arc_fmt = cugra_graph_is_directed(G)
	    ? "v%p->v%p;\n" : "v%p->v%p[dir=none];\n";
    else
	arc_fmt = cugra_graph_is_directed(G)
	    ? "v%p->v%p[label=\"%S\"];\n"
	    : "v%p->v%p[dir=none,label=\"%S\"];\n";
    for (v = cugra_graph_vertices_begin(G);
	    v != cugra_graph_vertices_end(G);
	    v = cugra_graph_vertices_next(v)) {
	cugra_arc_t a;
	if (!cu_clop_is_null(vertex_label))
	    cu_fprintf(fout, "v%p[label=\"%S\"];\n", v,
		       cu_call(vertex_label, v));
	if (cu_clop_is_null(arc_label))
	    for (a = cugra_vertex_outarcs_begin(v);
		    a != cugra_vertex_outarcs_end(v);
		    a = cugra_vertex_outarcs_next(a))
		fprintf(fout, arc_fmt, v, cugra_arc_head(a));
	else
	    for (a = cugra_vertex_outarcs_begin(v);
		    a != cugra_vertex_outarcs_end(v);
		    a = cugra_vertex_outarcs_next(a))
		fprintf(fout, arc_fmt, v, cugra_arc_head(a),
			cu_call(arc_label, a));
    }
    fputs("}\n", fout);
}

cu_bool_t
cugra_graph_save_dot(cugra_graph_t G,
		     cu_clop(vertex_label, cu_str_t, cugra_vertex_t v),
		     cu_clop(arc_label, cu_str_t, cugra_arc_t a),
		     char const *path)
{
    FILE *fout = fopen(path, "w");
    if (!fout)
	return cu_false;
    cugra_graph_fwrite_dot(G, vertex_label, arc_label, fout);
    fclose(fout);
    return cu_true;
}
