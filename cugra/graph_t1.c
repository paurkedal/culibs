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

#include <cugra/graph.h>
#include <cu/test.h>
#include <cu/memory.h>

void
test_random_simple()
{
    static int const nV = 100;
    static int const nA = 20000;
    int i, k, l;
    cugra_graph_t G = cugra_graph_new(CUGRA_GFLAG_SIMPLEARCED);
    cugra_vertex_t *v_arr = cu_galloc(sizeof(cugra_vertex_t)*nV);
    int conn_count, disconn_count;

    for (i = 0; i < nV; ++i)
	v_arr[i] = cugra_graph_vertex_new(G);

    conn_count = 0;
    for (i = 0; i < nA; ++i) {
	k = lrand48() % nV;
	l = lrand48() % nV;
	if (cugra_connect(G, v_arr[k], v_arr[l]))
	    ++conn_count;
    }
    printf("Inserted %d out of %d random arcs.\n", conn_count, nA);

    disconn_count = 0;
    for (k = 0; k < nV; ++k)
	for (l = 0; l < nV; ++l)
	    if (cugra_disconnect(G, v_arr[k], v_arr[l]))
		++disconn_count;
    printf("Disconnected %d arcs.\n", disconn_count);

    cu_test_assert_int_eq(
	cu_from(cugra_graph_with_arcset, cugra_graph, G)->arcset_size,
	conn_count - disconn_count);
}

int
main()
{
    cu_init();
    test_random_simple();
    return 2*!!cu_test_bug_count();
}
