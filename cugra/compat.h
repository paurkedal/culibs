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

#if CU_COMPAT < 20091122 && !defined(cugra_graph_s)
#  define cugra_graph_s			cugra_graph
#  define cugra_vertex_s		cugra_vertex
#  define cugra_arc_s			cugra_arc
#  define cugra_graph_with_arcset_s	cugra_graph_with_arcset
#  define cugra_walk_SCC_s		cugra_walk_SCC
#  define cugra_walk_SCC_vt_s		cugra_walk_SCC_vt
#endif
