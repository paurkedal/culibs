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

#include <cugra/algo_SCC.h>
#include <cugra/graph.h>
#include <cucon/uset.h>
#include <cu/test.h>

typedef struct my_vertex_s *my_vertex_t;
typedef struct my_subvertex_s *my_subvertex_t;
typedef struct my_cb_s *my_cb_t;

struct my_vertex_s
{
    cu_inherit (cugra_vertex_s);
    int label;
};

struct my_subvertex_s
{
    cu_inherit (cugra_vertex_s);
    struct cucon_uset_s labels;
};

struct my_cb_s
{
    cu_inherit (cugra_walk_SCC_s);
    cugra_graph_t G;
};

void *
my_enter_component(cugra_walk_SCC_t self)
{
    my_subvertex_t cpt = cu_gnew(struct my_subvertex_s);
    printf("New component:\n");
    cucon_uset_init(&cpt->labels);
    cugra_graph_vertex_init(cu_from(my_cb, cugra_walk_SCC, self)->G,
			    cu_to(cugra_vertex, cpt));
    return cpt;
}

void
my_pass_vertex(cugra_walk_SCC_t self, void *cpt, cugra_vertex_t v)
{
    int label = cu_from(my_vertex, cugra_vertex, v)->label;
    printf("    Vertex %d.\n", label);
    cucon_uset_insert(&((my_subvertex_t)cpt)->labels, label);
}

void my_leave_component(cugra_walk_SCC_t self, void *cpt) {}

void
my_connect_components(cugra_walk_SCC_t self, void *tail, void *head)
{
    printf("Connect ");
    cucon_uset_print(&((my_subvertex_t)tail)->labels, stdout);
    printf(" --> ");
    cucon_uset_print(&((my_subvertex_t)head)->labels, stdout);
    printf("\n");
}

struct cugra_walk_SCC_vt_s my_walk_vt = {
    .enter_component = my_enter_component,
    .pass_vertex = my_pass_vertex,
    .leave_component = my_leave_component,
    .connect_components = my_connect_components,
};

void
my_cb_init(my_cb_t cb)
{
    cu_to(cugra_walk_SCC, cb)->vt = &my_walk_vt;
    cb->G = cugra_graph_new(0);
}

cugra_vertex_t
vertex(cugra_graph_t G, cucon_umap_t M, int label)
{
    cugra_vertex_t *v;
    if (cucon_umap_insert_mem(M, label, sizeof(void *), &v)) {
	*v = cugra_graph_vertex_new_mem(G, sizeof(int));
	cu_from(my_vertex, cugra_vertex, *v)->label = label;
    }
    return *v;
}

void
test()
{
    struct cucon_umap_s M;
    struct my_cb_s cb;
    cugra_graph_t G = cugra_graph_new(0);

    cucon_umap_init(&M);
#define A(x, y) cugra_connect(G, vertex(G, &M, x), vertex(G, &M, y))
    A(0, 0); A(0, 1); A(1, 2); A(1, 3); A(2, 3); A(3, 0);
    A(4, 5); A(5, 4); A(4, 2); A(5, 0);
    A(6, 3);
    A(7, 6); A(7, 7);
    A(8, 9); A(9, 10); A(10, 8); A(9, 0); A(9, 6);
#undef A
    my_cb_init(&cb);
    cugra_walk_SCC(cu_to(cugra_walk_SCC, &cb), G, cugra_direction_out);
}

int
main()
{
    cu_init();
    test();
    return 2*!!cu_test_bug_count();
}
