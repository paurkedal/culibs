/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2008  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUGRA_GRAPH_SCC_H
#define CUGRA_GRAPH_SCC_H

#include <cugra/graph.h>
#include <cu/clos.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cugra_algo_SCC_h cugra/algo_SCC.h: Detect strongly connected components
 *@{\ingroup cugra_mod */

typedef struct cugra_walk_SCC *cugra_walk_SCC_t;
typedef struct cugra_walk_SCC_vt const *cugra_walk_SCC_vt_t;

struct cugra_walk_SCC_vt
{
    void *(*enter_component)(cugra_walk_SCC_t self);
    void (*pass_vertex)(cugra_walk_SCC_t self, void *, cugra_vertex_t);
    void (*leave_component)(cugra_walk_SCC_t self, void *);
    void (*connect_components)(cugra_walk_SCC_t self, void *, void *);
};

#define cugra_walk_SCC_def_vt(name) \
    struct cugra_walk_SCC_vt name##_vt = { \
	.enter_component = name##_enter_component; \
	.pass_vertext = name##_pass_vertex; \
	.leave_component = name##_leave_component; \
	.connect_components = name##_connect_components; \
    }

struct cugra_walk_SCC
{
    cugra_walk_SCC_vt_t vt;
};

/*!Walks though the strongly connected components of \a G where \a dir
 * indicates arrow direction of traversal, and calls the various \a walk_struct
 * callbacks when entering and leaving components and passing over vertices. */
void cugra_walk_SCC(cugra_walk_SCC_t walk_struct,
		    cugra_graph_t G, cugra_direction_t dir);

#if 0
cugra_graph_t
cugra_SCC_graph_of_lists(cugra_graph_t G, size_t vslot_size,
			 cu_clop(vslot_copy, void, void *src, void *dst));
#endif

/*!@}*/
CU_END_DECLARATIONS

#endif
