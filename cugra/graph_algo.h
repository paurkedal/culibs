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

#ifndef CUGRA_ALGO_H
#define CUGRA_ALGO_H

#include <cugra/fwd.h>
#include <cu/clos.h>
#include <cucon/fwd.h>
#include <stdio.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cugra_graph_algo_h cugra/graph_algo.h: Graph Algorithms
 *@{\ingroup cugra_mod */

/*!Copy \a G_src into \a G_dst, and insert into \a vsrc_to_vdst and \a
 * vdst_to_vsrc, if non-\c NULL, the vertex-mappings. */
void cugra_graph_copy(cugra_graph_t G_src, cugra_graph_t G_dst,
		      cucon_pmap_t vsrc_to_vdst, cucon_pmap_t vdst_to_vsrc);

/*!True iff \a G is acyclic. */
cu_bool_t cugra_graph_is_acyclic(cugra_graph_t G);

/*!Erase all isolated vertices in \a G. */
void cugra_graph_erase_isolated(cugra_graph_t G);

/*!Erase all arcs which connect \a v to a vertex not in \a V. */
void cugra_erase_vertex_to_vsetcompl_arcs(cugra_vertex_t v, cucon_pmap_t V);

/*!Move vertices in \a V_move from \a G_src to \a G_dst, cutting all arcs
 * between \a V_move and its complement. */
void cugra_move_induced_subgraph(cucon_pmap_t V_move,
				 cugra_graph_t G_src, cugra_graph_t G_dst);

/*!Finds the maximally connected subgraphs of \a G and report them in two
 * ways as follows.
 * If \a KV is non-\c NULL, push a \c cucon_pset_t for each subgraph where the
 * keys are the vertices of the subgraph.
 * If \a M is non-\c NULL, associate each vertex which is part of a loop with
 * an index identifying the maximally connected subgraph to which it belongs.
 */
void cugra_identify_MSC(cugra_graph_t G, cucon_stack_t KV, cucon_pmap_t M);

/*!Move all maximally connected subgraphs of \a G into separate graphs and
 * push them onto \a KG. */
void cugra_move_MSC_subgraphs(cugra_graph_t G, cucon_stack_t KG);

#ifdef CUCON_HAVE_BUDDY
/*!Inserts a minimum feedback vertex set of \a G into \a V.  Only available if
 * culibs is linked with BuDDY. */
void cugra_MFVS(cugra_graph_t G, cucon_pset_t V);
#endif

/*!Write \a G in Graphviz .dot format to \a fout, with labels for vertices and
 * arcs as given by \a vertex_label and \a arc_label, repectively, unless \ref
 * cu_clop_null. */
void cugra_graph_fwrite_dot(cugra_graph_t G,
			    cu_clop(vertex_label, cu_str_t, cugra_vertex_t),
			    cu_clop(arc_label, cu_str_t, cugra_arc_t),
			    FILE *fout);

/*!Write \a G in Graphviz .dot format to \a path, with labels for vertices and
 * arcs as given by \a vertex_label and \a arc_label, respectively, unless
 * \ref cu_clop_null. */
cu_bool_t cugra_graph_save_dot(cugra_graph_t G,
			       cu_clop(vertex_label, cu_str_t, cugra_vertex_t),
			       cu_clop(arc_label, cu_str_t, cugra_arc_t),
			       char const *path);

/*!Searches shortest path, according to distances given by \a arc_distance,
 * from \a v_start to a vertex for which \a vertex_test returns true.  If
 * found, call \a notify_path_unwind with arcs of the shortest path in revese
 * order and return the path's distance.  Otherwise, return \c INFINITY. */
double
cugra_shortest_path(cugra_direction_t dir, cugra_vertex_t v_start,
		    cu_clop(vertex_test, cu_bool_t, cugra_vertex_t),
		    cu_clop(arc_distance, double, cugra_arc_t),
		    cu_clop(notify_path_unwind, void, cugra_arc_t));

/*!@}*/
CU_END_DECLARATIONS

#endif
