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
#include <cucon/pset.h>
#include <cu/hash.h>
#include <cu/memory.h>
#include <cu/diag.h>
#include <cu/util.h>
#include <string.h>

/* Tuning */
#ifdef CUCONF_OPT_SPEED
#  define ARCSET_MINFILL_NUMER 1
#  define ARCSET_MINFILL_DENOM 3
#  define ARCSET_MAXFILL_NUMER 1
#  define ARCSET_MAXFILL_DENOM 1
#else
#  define ARCSET_MINFILL_NUMER 1
#  define ARCSET_MINFILL_DENOM 2
#  define ARCSET_MAXFILL_NUMER 3
#  define ARCSET_MAXFILL_DENOM 2
#endif

typedef struct cugra_graph_with_arcset *cugra_graph_with_arcset_t;

#define ASGRAPH(G) cu_from(cugra_graph_with_arcset, cugra_graph, G)

CU_SINLINE cu_hash_t
_vertex_pair_hash(cugra_vertex_t v0, cugra_vertex_t v1)
{
    return cu_2word_hash_noinit_bj((cu_word_t)v0, (cu_word_t)v1);
}

CU_SINLINE void
_init_arc(cugra_arc_t a, cugra_vertex_t v_tail, cugra_vertex_t v_head)
{
    cu_dlink_insert_before(&v_tail->adj_link[cugra_direction_out],
			   &a->adj[cugra_direction_out].link);
    a->adj[cugra_direction_out].vertex = v_head;
    cu_dlink_insert_before(&v_head->adj_link[cugra_direction_in],
			   &a->adj[cugra_direction_in].link);
    a->adj[cugra_direction_in].vertex = v_tail;
}

CU_SINLINE void
_unlink_arc(cugra_arc_t a)
{
    cu_dlink_erase(&a->adj[0].link);
    cu_dlink_erase(&a->adj[1].link);
}


/* Arcset Internals
 * ---------------- */

typedef struct cugraP_arcset_node_s *_arcset_node_t;
struct cugraP_arcset_node_s
{
    _arcset_node_t next;
    struct cugra_arc arc;
};

static void
_arcset_init(cugra_graph_with_arcset_t G)
{
    static _arcset_node_t null_arr[1] = {NULL};
    G->arcset_size = 0;
    G->arcset_mask = 0;
    G->arcset_arr = null_arr;
}

static void
_arcset_shrink(cugra_graph_with_arcset_t G, size_t new_cap)
{
    size_t old_cap, new_mask, i;
    _arcset_node_t *new_arr, *old_arr;

    old_arr = G->arcset_arr;
    old_cap = G->arcset_mask + 1;
    G->arcset_arr = new_arr = cu_galloc(sizeof(_arcset_node_t)*new_cap);
    G->arcset_mask = new_mask = new_cap - 1;

    memcpy(new_arr, old_arr, new_cap*sizeof(_arcset_node_t));
    for (i = new_cap; i < old_cap; ++i) {
	if (old_arr[i]) {
	    _arcset_node_t *p = &new_arr[i & new_mask];
	    while (*p)
		p = &(*p)->next;
	    *p = old_arr[i];
	}
    }
}

static void
_arcset_extend(cugra_graph_with_arcset_t G, size_t new_cap)
{
    size_t old_cap, new_mask, i;
    _arcset_node_t *new_arr, *old_arr;

    old_arr = G->arcset_arr;
    old_cap = G->arcset_mask + 1;
    G->arcset_arr  = new_arr  = cu_galloc(sizeof(_arcset_node_t)*new_cap);
    G->arcset_mask = new_mask = new_cap - 1;

    for (i = 0; i < old_cap; ++i) {
	_arcset_node_t node = old_arr[i];
	while (node) {
	    _arcset_node_t next_node = node->next;
	    cu_hash_t hash = _vertex_pair_hash(cugra_arc_tail(&node->arc),
					       cugra_arc_head(&node->arc));
	    _arcset_node_t *p = &new_arr[hash & new_mask];
	    node->next = *p;
	    *p = node;
	    node = next_node;
	}
    }
}

static cu_bool_t
_arcset_insert(cugra_graph_with_arcset_t G,
	       cugra_vertex_t tail, cugra_vertex_t head,
	       size_t arc_size, cugra_arc_t *arc_out)
{
    cu_hash_t hash;
    _arcset_node_t *p;

    /* If fill ration is over limit, extend. Make sure the test always trigges
     * for arcset_mask == 0, due to the way _arcset_init is implemented. */
    if (G->arcset_size*ARCSET_MAXFILL_DENOM
	    >= G->arcset_mask*ARCSET_MAXFILL_NUMER)
	_arcset_extend(G, (G->arcset_mask + 1)*2);

    /* Look for existing arc from tail to head. */
    hash = _vertex_pair_hash(tail, head);
    p = &G->arcset_arr[hash & G->arcset_mask];
    while (*p) {
	if (cugra_arc_connects(&(*p)->arc, tail, head)) {
	    *arc_out = &(*p)->arc;
	    return cu_false;
	}
	p = &(*p)->next;
    }

    /* No existing arc, insert it. */
    *p = cu_galloc(arc_size
		   + sizeof(struct cugraP_arcset_node_s)
		   - sizeof(struct cugra_arc));
    CU_GCLEAR_PTR((*p)->next);
    *arc_out = &(*p)->arc;
    _init_arc(*arc_out, tail, head);
    ++G->arcset_size;
    return cu_true;
}

static int
_arcset_erase(cugra_graph_with_arcset_t G,
	      cugra_vertex_t tail, cugra_vertex_t head)
{
    cu_hash_t index;
    _arcset_node_t *p;

    index = _vertex_pair_hash(tail, head) & G->arcset_mask;
    p = &G->arcset_arr[index];
    while (*p) {
	if (cugra_arc_connects(&(*p)->arc, tail, head)) {
	    _unlink_arc(&(*p)->arc);
	    --G->arcset_size;
	    *p = (*p)->next;

	    /* Shrink if fill ration is below limit. Don't resize below
	     * arcset_mask == 1, cf _arcset_insert. */
	    if (G->arcset_size*ARCSET_MINFILL_DENOM
			< G->arcset_mask*ARCSET_MINFILL_NUMER
		    && G->arcset_mask > 1)
		_arcset_shrink(G, (G->arcset_mask + 1)/2);

	    return 1;
	}
	p = &(*p)->next;
    }
    return 0;
}


/* Graph Primitives
 * ---------------- */

void
cugra_graph_init(cugra_graph_t G, unsigned int gflags)
{
    G->gflags = gflags;
    if (gflags & CUGRA_GFLAG_SIMPLEARCED)
	cu_bugf("cugra_graph_init: CUGRA_GFLAG_SIMPLEARCED is unsupported by "
		"this constructor");
    cu_dlink_init_singleton(&G->vertices);
}

cugra_graph_t
cugra_graph_new(unsigned int gflags)
{
    cugra_graph_t G;
    if (gflags & CUGRA_GFLAG_SIMPLEARCED) {
	G = cu_galloc(sizeof(struct cugra_graph_with_arcset));
	_arcset_init(ASGRAPH(G));
    } else
	G = cu_gnew(struct cugra_graph);
    G->gflags = gflags;
    cu_dlink_init_singleton(&G->vertices);
    return G;
}

void
cugra_graph_merge(cugra_graph_t G_dst, cugra_graph_t G_src)
{
    cu_dlink_insert_list_before(&G_dst->vertices, &G_src->vertices);
}

CU_SINLINE void
_init_vertex(cugra_graph_t G, cugra_vertex_t v)
{
    cu_dlink_insert_before(&G->vertices, &v->in_graph);
    cu_dlink_init_singleton(&v->adj_link[0]);
    cu_dlink_init_singleton(&v->adj_link[1]);
}

void
cugra_graph_vertex_init(cugra_graph_t G, cugra_vertex_t v)
{
    _init_vertex(G, v);
}

cugra_vertex_t
cugra_graph_vertex_new(cugra_graph_t G)
{
    cugra_vertex_t v = cu_gnew(struct cugra_vertex);
    _init_vertex(G, v);
    return v;
}

cugra_vertex_t
cugra_graph_vertex_new_mem(cugra_graph_t G, size_t size)
{
    cugra_vertex_t v = cu_galloc(sizeof(struct cugra_vertex) + size);
    _init_vertex(G, v);
    return v;
}

cugra_vertex_t
cugra_graph_vertex_new_ptr(cugra_graph_t G, void *ptr)
{
    cugra_vertex_t v;
    v = cu_galloc(sizeof(struct cugra_vertex) + sizeof(void *));
    *(void **)cugra_vertex_mem(v) = ptr;
    _init_vertex(G, v);
    return v;
}

cugra_arc_t
cugra_graph_arc_new(cugra_vertex_t v_tail, cugra_vertex_t v_head)
{
    cugra_arc_t a;
    a = cu_gnew(struct cugra_arc);
    _init_arc(a, v_tail, v_head);
    return a;
}

cugra_arc_t
cugra_graph_arc_new_mem(cugra_vertex_t v_tail, cugra_vertex_t v_head,
			size_t size)
{
    cugra_arc_t a;
    a = cu_galloc(sizeof(struct cugra_arc) + size);
    _init_arc(a, v_tail, v_head);
    return a;
}

cugra_arc_t
cugra_graph_arc_new_ptr(cugra_vertex_t v_tail, cugra_vertex_t v_head,
			void *ptr)
{
    cugra_arc_t a;
    a = cu_galloc(sizeof(struct cugra_arc) + sizeof(void *));
    *(void **)cugra_arc_mem(a) = ptr;
    _init_arc(a, v_tail, v_head);
    return a;
}

cu_bool_t
cugra_connect_custom(cugra_graph_t G,
		     cugra_vertex_t tail, cugra_vertex_t head,
		     size_t arc_size, cugra_arc_t *arc_out)
{
    cugra_arc_t arc;
    if (G->gflags & CUGRA_GFLAG_SIMPLEARCED) {
	if ((G->gflags & CUGRA_GFLAG_UNDIRECTED) && tail > head)
	    CU_SWAP(cugra_vertex_t, tail, head);
	return _arcset_insert(ASGRAPH(G), tail, head, arc_size, arc_out);
    } else {
	arc = cu_galloc(arc_size);
	_init_arc(arc, tail, head);
	*arc_out = arc;
	return cu_true;
    }
}

cu_bool_t
cugra_connect(cugra_graph_t G,
	      cugra_vertex_t tail, cugra_vertex_t head)
{
    cugra_arc_t arc;
    if (G->gflags & CUGRA_GFLAG_SIMPLEARCED) {
	if ((G->gflags & CUGRA_GFLAG_UNDIRECTED) && tail > head)
	    CU_SWAP(cugra_vertex_t, tail, head);
	return _arcset_insert(ASGRAPH(G), tail, head,
			      sizeof(struct cugra_arc), &arc);
    } else {
	arc = cu_gnew(struct cugra_arc);
	_init_arc(arc, tail, head);
	return cu_true;
    }
}

int
cugra_disconnect(cugra_graph_t G, cugra_vertex_t tail, cugra_vertex_t head)
{
    if (G->gflags & CUGRA_GFLAG_SIMPLEARCED) {
	if ((G->gflags & CUGRA_GFLAG_UNDIRECTED) && tail > head)
	    CU_SWAP(cugra_vertex_t, tail, head);
	return _arcset_erase(ASGRAPH(G), tail, head);
    } else {
	cugra_arc_t arc = cugra_vertex_outarcs_begin(tail);
	int count = 0;
	while (arc != cugra_vertex_outarcs_end(tail)) {
	    cugra_arc_t next_arc = cugra_vertex_outarcs_next(arc);
	    if (cugra_arc_head(arc) == head) {
		_unlink_arc(arc);
		++count;
	    }
	    arc = next_arc;
	}
	return count;
    }
}

void
cugra_erase_arc(cugra_graph_t G, cugra_arc_t arc)
{
    if (G->gflags & CUGRA_GFLAG_SIMPLEARCED)
	_arcset_erase(ASGRAPH(G),
		      cugra_arc_tail(arc), cugra_arc_head(arc));
    else
	_unlink_arc(arc);
}

void
cugra_erase_vertex(cugra_graph_t G, cugra_vertex_t v)
{
    cugra_direction_t dir;
    for (dir = 0; dir < 2; ++dir) {
	cugra_arc_t a;
	cu_dlink_erase(&v->in_graph);
	a = cugra_vertex_arcs_begin(dir, v);
	while (a != cugra_vertex_arcs_end(dir, v)) {
	    cugra_arc_t a_next = cugra_vertex_arcs_next(dir, a);
	    cugra_erase_arc(G, a);
	    a = a_next;
	}
    }
}

cu_clos_efun(cugra_erase_vertex_clos, cu_prot(void, cugra_vertex_t v))
{
    cu_clos_self(cugra_erase_vertex_clos);
    cugra_erase_vertex(self->G, v);
}

void
cugra_eliminate_vertex(cugra_graph_t G, cugra_vertex_t v)
{
    cugra_arc_t out, in;
    if (cugra_graph_is_simplearced(G)) {
	cugra_vertex_for_inarcs(in, v) {
	    cugra_vertex_t v_tail = cugra_arc_tail(in);
	    cugra_vertex_for_outarcs(out, v) {
		cugra_vertex_t v_head = cugra_arc_head(out);
		cugra_connect(G, v_tail, v_head);
	    }
	}
    } else {
	cugra_vertex_for_inarcs(in, v) {
	    struct cucon_pset V;
	    cugra_vertex_t v_tail = cugra_arc_tail(in);
	    cucon_pset_init(&V);
	    cugra_vertex_for_outarcs(out, v_tail)
		cucon_pset_insert(&V, cugra_arc_head(out));
	    cugra_vertex_for_outarcs(out, v) {
		cugra_vertex_t v_head = cugra_arc_head(out);
		if (!cucon_pset_find(&V, v_head))
		    cugra_connect(G, v_tail, v_head);
	    }
	}
    }
    cugra_erase_vertex(G, v);
}

void
cugra_vertex_erase_loops(cugra_graph_t G, cugra_vertex_t v)
{
    cugra_arc_t a;
    a = cugra_vertex_outarcs_begin(v);
    while (a != cugra_vertex_outarcs_end(v)) {
	cugra_arc_t a_next = cugra_vertex_outarcs_next(a);
	if (cugra_arc_head(a) == v)
	    cugra_erase_arc(G, a);
	a = a_next;
    }
}

void
cugra_graph_erase_loops(cugra_graph_t G)
{
    cugra_vertex_t v;
    cugra_graph_for_vertices(v, G)
	cugra_vertex_erase_loops(G, v);
}

cu_bool_t
cugra_vertex_has_loop(cugra_vertex_t v)
{
    cugra_arc_t a;
    cugra_vertex_for_outarcs(a, v)
	if (cugra_arc_is_loop(a))
	    return cu_true;
    return cu_false;
}

cugra_arc_t
cugra_graph_arcs_begin(cugra_graph_t G)
{
    cugra_vertex_t v = cugra_graph_vertices_begin(G);
    cugra_arc_t a;
    for (;;) {
	if (v == cugra_graph_vertices_end(G))
	    return NULL;
	a = cugra_vertex_outarcs_begin(v);
	if (a != cugra_vertex_outarcs_end(v))
	    return a;
	v = cugra_graph_vertices_next(v);
    }
}

cugra_arc_t
cugra_graph_arcs_next(cugra_graph_t G, cugra_arc_t a)
{
    cugra_vertex_t v = cugra_arc_tail(a);
    a = cugra_vertex_outarcs_next(a);
    while (a == cugra_vertex_outarcs_end(v)) {
	v = cugra_graph_vertices_next(v);
	if (v == cugra_graph_vertices_end(G))
	    return NULL;
	a = cugra_vertex_outarcs_begin(v);
    }
    return a;
}
