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

#include <cugra/fwd.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cugra_graph_scc_h cugra/graph_scc.h: Detect strongly connected components
 *@{\ingroup cugra_mod */

typedef struct cugra_graph_SCC_cb_s *cugra_graph_SCC_cb_t;
struct cugra_graph_SCC_cb_s
{
    void *(*cpt_new)(cugra_graph_SCC_cb_t self);
    void (*cpt_insert)(cugra_graph_SCC_cb_t self, void *cpt, cugra_vertex_t v);
    void (*cpt_connect)(cugra_graph_SCC_cb_t self, void *tail_cpt, void *head_cpt);
};

void cugra_graph_SCC_cb(cugra_graph_t G, cugra_graph_SCC_cb_t cb);

/*!@}*/
CU_END_DECLARATIONS

#endif
