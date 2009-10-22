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
#include <cucon/pmap.h>
#include <cucon/pset.h>
#include <cucon/stack.h>
#include <limits.h>

/* Maximum strongly connected subgraphs */

typedef struct msc_tag_s *msc_tag_t;
typedef struct msc_vinfo_s *msc_vinfo_t;

struct msc_tag_s
{
    int level;
    msc_tag_t replacement;
    cucon_pset_t vertex_set;
};

CU_SINLINE msc_tag_t
_canonical_tag(msc_tag_t tag)
{
    while (tag->replacement)
	tag = tag->replacement;
    return tag;
}

struct msc_vinfo_s
{
    int level;
    msc_tag_t tag;
};

static msc_tag_t
_collect_msc(cugra_vertex_t v, cucon_pmap_t vinfomap, int level)
{
    msc_vinfo_t vinfo;
    if (cucon_pmap_insert_mem(vinfomap, v, sizeof(struct msc_vinfo_s), &vinfo)) {
	msc_tag_t tag = NULL;
	cugra_arc_t a;

	/* Process out-arcs */
	vinfo->level = level; /* visiting */
	vinfo->tag = NULL;
	cugra_vertex_for_outarcs(a, v) {
	    msc_tag_t tagp;
	    tagp = _collect_msc(cugra_arc_head(a), vinfomap, level + 1);
	    if (tagp && tagp != tag && tagp->level <= level) {
		cu_debug_assert(!tagp->replacement);
		if (tag) {
		    if (tag->level < tagp->level
			    || (tag->level == tagp->level && tag < tagp)) {
			tagp->replacement = tag;
		    }
		    else {
			tag->replacement = tagp;
			tag = tagp;
		    }
		    cu_debug_assert(!tag->replacement);
		}
		else
		    tag = tagp;
	    }
	}
	vinfo->level = INT_MAX; /* not visiting */

	if (vinfo->tag) { /* one of the out-arcs linked back here */
	    cu_debug_assert(tag);
	    cu_debug_assert(tag->level <= level);
	    if (tag->level == level) {
		cu_debug_assert(tag == vinfo->tag);
		tag->level = INT_MAX;
		return NULL;
	    }
	    else { /* an even lower level back-link was merged in here */
		if (tag != vinfo->tag) {
		    cu_debug_assert(_canonical_tag(vinfo->tag) == tag);
		    vinfo->tag->level = INT_MAX;
		    vinfo->tag = tag;
		}
		return tag;
	    }
	}
	else {
	    cu_debug_assert(!tag || tag->level < level);
	    vinfo->tag = tag;
	    return tag;
	}
    }
    else if (vinfo->level != INT_MAX) { /* already visiting v */
	msc_tag_t tag = vinfo->tag;
	cu_debug_assert(vinfo->level <= level);
	if (!tag) {
	    tag = cu_gnew(struct msc_tag_s);
	    tag->level = vinfo->level;
	    tag->replacement = NULL;
	    tag->vertex_set = NULL;
	    vinfo->tag = tag;
	}
	else
	    vinfo->tag = tag = _canonical_tag(tag);
	return tag;
    }
    else if (vinfo->tag)
	return (vinfo->tag = _canonical_tag(vinfo->tag));
    else
	return NULL;
}

cu_clos_def(_reverse_msc_tag_cb,
	    cu_prot(cu_bool_t, void const *vertex, void *mem),
    ( cucon_stack_t vertex_set_stack;
      cucon_pmap_t vertex_index_map;
      int current_index; ))
{
#define vertex ((cugra_vertex_t)vertex)
    cu_clos_self(_reverse_msc_tag_cb);
    msc_tag_t tag = ((msc_vinfo_t)mem)->tag;
    cucon_pset_t vertex_set;
    if (!tag)
	return cu_true;
    tag = _canonical_tag(tag);
    if (self->vertex_set_stack) {
	vertex_set = tag->vertex_set;
	if (!vertex_set) {
	    cu_debug_assert(tag->level == INT_MAX);
	    vertex_set = cucon_pset_new();
	    CUCON_STACK_PUSH(self->vertex_set_stack, cucon_pset_t, vertex_set);
	    tag->vertex_set = vertex_set;
	}
	cucon_pset_insert(vertex_set, vertex);
    }
    if (self->vertex_index_map) {
	int *i;
	if (tag->level == INT_MAX)
	    tag->level = self->current_index++;
	cucon_pmap_insert_mem(self->vertex_index_map, vertex, sizeof(int), &i);
	*i = tag->level;
    }
    return cu_true;
#undef vertex
}

static void
_reverse_msc_tag(cucon_pmap_t vinfomap,
		 cucon_stack_t vertex_set_stack,
		 cucon_pmap_t vertex_index_map)
{
    _reverse_msc_tag_cb_t cb;
    cb.vertex_set_stack = vertex_set_stack;
    cb.vertex_index_map = vertex_index_map;
    cb.current_index = 0;
    cucon_pmap_conj_mem(vinfomap, _reverse_msc_tag_cb_prep(&cb));
}

void
cugra_identify_MSC(cugra_graph_t G, cucon_stack_t vertex_set_stack,
		   cucon_pmap_t vertex_index_map)
{
    cugra_vertex_t v;
    struct cucon_pmap_s vinfomap;
    cucon_pmap_init(&vinfomap);
    cugra_graph_for_vertices(v, G)
	_collect_msc(v, &vinfomap, 0);
    _reverse_msc_tag(&vinfomap, vertex_set_stack, vertex_index_map);
}

void
cugra_move_MSC_subgraphs(cugra_graph_t G, cucon_stack_t KG)
{
    struct cucon_stack_s KV;
    cucon_stack_init(&KV);
    cugra_identify_MSC(G, &KV, NULL);
    while (!cucon_stack_is_empty(&KV)) {
	cucon_pmap_t V = CUCON_STACK_TOP(&KV, cucon_pmap_t);
	cugra_graph_t Gp = cugra_graph_new(G->gflags);
	CUCON_STACK_POP(&KV, cucon_pmap_t);
	cugra_move_induced_subgraph(V, G, Gp);
	CUCON_STACK_PUSH(KG, cugra_graph_t, Gp);
    }
}
