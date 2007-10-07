/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2007  Petter Urkedal <urkedal@nbi.dk>
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

#include <cucon/parr.h>
#include <cucon/list.h>
#include <cucon/slink.h>
#include <cucon/pset.h>
#include <cucon/pmap.h>
#include <cucon/uset.h>
#include <cuex/semilattice.h>
#include <cuex/ex.h>
#include <cuex/opn.h>
#include <cuex/oprdefs.h>
#include <cuex/binding.h>
#include <cuex/algo.h>
#include <cuex/compound.h>
#include <cuex/intf.h>
#include <cu/inherit.h>
#include <cu/ptr_seq.h>

typedef struct state_s *state_t;
typedef struct block_s *block_t;
typedef struct initial_frame_s *initial_frame_t;
typedef struct selectset_s *selectset_t;

struct state_s
{
    cu_inherit (cucon_listnode_s);
    block_t block;
    struct cucon_parr_s invtran; /* of cucon_slink_t of state_t */
    cuex_t e;
    state_t sub[1];
};

struct selectset_s
{
    cucon_slink_t state_link;
    size_t state_count;
};

struct block_s
{
    /* The current set of states of this block.  The states are incrementally
     * moved over to the target block during the iteration over t ∈ Δ a s where
     * s ∈ C i a. */
    struct cucon_list_s state_list; /* of state_s nodes */
    size_t state_count;

    /* The set C i a = { s | s ∈ B i ∧ ∃t δ a t = s }, regenerated from scratch
     * on each iteration for changed blocks. */
    size_t selectset_count;
    struct selectset_s *selectset_arr; /* of cucon_slink_t of state_t */

    /* The target block being built.  This is allocated on demand, which also
     * triggers linking the block on an update link. */
    block_t target;
    block_t next_to_update;

    /* For rebuilding expression. */
    cu_bool_t need_mubind;
    int level;
    cuex_t e;
};

CU_SINLINE state_t
state_from_node(cucon_listnode_t node)
{ return cu_from(state, cucon_listnode, node); }

static state_t
state_new(int r, cuex_t e)
{
    state_t state = cu_galloc(sizeof(struct state_s) + (r - 1)*sizeof(state_t));
    cu_debug_assert(e);
    cucon_parr_cct_empty(&state->invtran);
    state->e = e;
    state->block = NULL;
    cu_dprintf("cuex.optimal_fold",
	       "New state %p; r = %d, e = %!", state, r, e);
    return state;
}

/* Initial block for all λ-variables. */
#define LAMBDAVAR_BLOCK 0

#define for_states_in_block(state, block)				\
    for (state = state_from_node(cucon_list_begin(&block->state_list));	\
	 state != state_from_node(cucon_list_end(&block->state_list));	\
	 state = state_from_node(					\
	     cucon_listnode_next(cu_to(cucon_listnode, state))))

static void
block_cct(block_t block)
{
    block->state_count = 0;
    cucon_list_cct(&block->state_list);
    block->target = NULL;
    /* block->selectset_arr is created by block_update */
    /* block->next_to_update is used internally in refine_partition */

    block->need_mubind = cu_false;
    block->level = -1;
}

static void
block_insert_state(block_t block, state_t state)
{
    cucon_list_append_node_cct(
	&block->state_list, cu_to(cucon_listnode, state));
    ++block->state_count;
    state->block = block;
    cu_dprintf("cuex.optimal_fold",
	       "Added state %p to block %p", state, block);
}

/* Update the selectset_arr of block after splitting it. */
static void
block_update(block_t block)
{
    cu_rank_t a, r_max = 0;
    state_t state;
    for_states_in_block(state, block) {
	int a = cucon_parr_size(&state->invtran);
	if (a > r_max)
	    r_max = a;
    }
    block->selectset_count = r_max;
    block->selectset_arr = cu_galloc(sizeof(struct selectset_s)*r_max);
    for (a = 0; a < r_max; ++a) {
	block->selectset_arr[a].state_count = 0;
	block->selectset_arr[a].state_link = NULL;
    }
    /* Compute block->selectset_arr[a] = { state | Δ state a ≠ ∅ }. */
    cu_dprintf("cuex.optimal_fold",
	       "Update block %p, r_max = %d", block, r_max);
    for_states_in_block(state, block) {
	cucon_slink_t *itr_invtran;
	struct selectset_s *itr_selectset;
	a = cucon_parr_size(&state->invtran);
	itr_invtran = cucon_parr_ref_at(&state->invtran, a);
	itr_selectset = block->selectset_arr + a;
	while (a > 0) {
	    --a;
	    --itr_invtran;
	    --itr_selectset;
	    if (*itr_invtran) { /* Δ state a ≠ ∅ so state ∈ C i a */
		++itr_selectset->state_count;
		itr_selectset->state_link =
		    cucon_slink_prepend_ptr(itr_selectset->state_link, state);
	    }
	}
    }
}

struct initial_frame_s
{
    state_t state;
    cuex_t e;
    struct cucon_uset_s fvset;
    cucon_uset_t mfvset;
};

static cuex_t
strip(cuex_t e)
{
    cuex_meta_t e_meta = cuex_meta(e);
    if (cuex_meta_is_opr(e_meta))
	return cuex_o0_metanull();
    else
	return e;
}

static void
link_substate(state_t state, cu_rank_t a, cu_rank_t b, state_t substate)
{
    cucon_slink_t *invlink;

    state->sub[a] = substate;

    /* Insert 'state ∈ Δ b substate' */
    if (cucon_parr_size(&substate->invtran) <= b)
	cucon_parr_resize_exactmax_fill(&substate->invtran,
					b + 1, NULL);
    invlink = cucon_parr_ref_at(&substate->invtran, b);
    *invlink = cucon_slink_prepend_ptr(*invlink, state);
    cu_dprintf("cuex.optimal_fold", "Inserting %p ∈ Δ %d %p (r_max = %d)",
	       state, b, substate, cucon_parr_size(&substate->invtran));
}

static void
upframe_add_fv(initial_frame_t sp, initial_frame_t sp_max, int index)
{
    while (index > 0 && sp < sp_max) {
	if (!sp->mfvset) {
	    if (!cucon_uset_insert(&sp->fvset, index))
		return;
	}
	--index;
	++sp;
    }
}

cu_clos_def(upframe_add_fv_clos, cu_prot(void, uintptr_t index),
    ( initial_frame_t sp, sp_max;
      int shift; ))
{
    cu_clos_self(upframe_add_fv_clos);
    upframe_add_fv(self->sp, self->sp_max, index + self->shift);
}

static state_t
initial_partition(cuex_t e, initial_frame_t sp, initial_frame_t sp_max,
		  cucon_pmap_t ekey_to_block, block_t lambdavar_block,
		  int r_max, struct cucon_pset_s *pending_arr,
		  int mudepth, cuex_t mupath, cucon_pmap_t fvmap)
{
    cuex_meta_t e_meta;
    initial_frame_t sp_mu;
    state_t state;
    int a, r;

    /* == μ-Bind ==
     *
     * If μ-bind, push stack and proceed directly with it's body.  The μ-bind
     * does not itself contribute as a structural node, so it's state is
     * identical to the body's state.  Therefore sp_mu->state will be set below
     * when created. */
    e_meta = cuex_meta(e);
    if (e_meta == CUEX_O1_MU) {
	sp_mu = --sp;
	cu_dprintf("cuex.optimal_fold",
		   "Pushing μ-frame, new level %d", sp_max - sp);
	sp_mu->e = e;
#ifndef CU_NDEBUG
	sp_mu->state = NULL;
#endif
	mupath = cuex_mupath_pair(mudepth + 1, mupath, e);
	mudepth = 0;
	sp_mu->mfvset = cucon_pmap_find_mem(fvmap, mupath);
	cu_debug_assert(sp_mu->mfvset);
	e = cuex_opn_at(e, 0);
	e_meta = cuex_meta(e);
    } else
	sp_mu = NULL;

    if (cuex_meta_is_opr(e_meta) && cuex_og_hole_contains(e_meta)) {
	int index;
	initial_frame_t sp_ref;

	/* == λ- and μ-Variables == */

	index = cuex_oa_hole_index(e_meta);
	sp_ref = sp + index;
	cu_debug_assert(sp_ref < sp_max); /* TODO */
	cu_debug_assert(sp_ref->state);
	if (cuex_meta(sp_ref->e) == CUEX_O1_MU) {
	    upframe_add_fv_clos_t add_fv_cb;

	    /* μ-variable are equivalent to what they refer to */
	    cu_debug_assert(!sp_mu || index != 0);
	    cu_debug_assert(sp_ref->state != NULL);
	    state = sp_ref->state;
	    cu_dprintf("cuex.optimal_fold", "Ref μ variable, index=%d; %!",
		       index, sp_ref->e);
	    cu_debug_assert(sp_ref->mfvset);
	    add_fv_cb.sp = sp;
	    add_fv_cb.sp_max = sp_max;
	    add_fv_cb.shift = index;
	    cucon_uset_iter(sp_ref->mfvset,
			    upframe_add_fv_clos_prep(&add_fv_cb));
	} else { /* e is a λ variable */
	    /* λ-variables have individual states in a shared block.  We
	     * synthetically link the point of reference as a substate. */
	    state = state_new(1, e);
	    block_insert_state(lambdavar_block, state);
	    link_substate(state, 0, 0, sp_ref->state);
	    cu_dprintf("cuex.optimal_fold", "Ref λ variable, index=%d; %!",
		       index, sp_ref->e);
	    upframe_add_fv(sp, sp_max, index);
	}
	if (sp_mu)
	    sp_mu->state = state;
    } else {
	block_t block;
	cuex_t ekey = e;

	/* == Structural Expressions and λ-Bind == */

	if (cuex_meta_is_opr(e_meta)) {

	    /* Allocate a new state. */
	    r = cuex_opr_r(e_meta);
	    state = state_new(r, e);

	    /* If we had a surrounding μ-bind, set it's state. */
	    if (sp_mu)
		sp_mu->state = state;


	    CUEX_OPN_TRAN(e_meta, ekey, ep, strip(ep));

	    /* If λ-bind, push stack. */
	    cu_debug_assert(!cuex_og_hole_contains(e_meta));
	    if (cuex_og_binder_contains(e_meta)) {
		cu_debug_assert(e_meta != CUEX_O1_MU);
		++mudepth;
		--sp;
		cu_dprintf("cuex.optimal_fold",
			   "Pushing λ-frame, new level %d", sp_max - sp);
		sp->state = state;
		sp->e = e;
		sp->mfvset = NULL;
		cucon_uset_cct(&sp->fvset);
	    }

	    /* Process subexpressions and add transitions for δ and Δ. */
	    for (a = 0; a < r; ++a) {
		if (cuex_opn_at(ekey, a) != cuex_o0_metanull())
		    state->sub[a] = NULL;
		else {
		    state_t substate;

		    substate = initial_partition(cuex_opn_at(e, a), sp, sp_max,
						 ekey_to_block, lambdavar_block,
						 r_max, pending_arr,
						 mudepth, mupath, fvmap);
		    link_substate(state, a, a, substate);
		}
	    }

	    if (cuex_og_binder_contains(e_meta)) {
		cu_dprintf("cuex.optimal_fold",
			   "%& = FV(%!)\n", cucon_uset_print, &sp->fvset, e);
		ekey = cuex_o2_metapair(
		    ekey, cuex_hole(cucon_uset_size(&sp->fvset)));
	    }
	}
	else {
	    state = NULL;
	    if (cuex_meta_is_type(e_meta)) {
		cuoo_type_t e_type = cuoo_type_from_meta(e_meta);
		cuex_intf_compound_t impl;
		cu_ptr_source_t source;
		cuex_t ep;

		/* Deal with compounds.  This is analogous to operations,
		 * except that we use the commutative view in order to avoid
		 * high arities. */
		impl = cuoo_type_impl_ptr(e_type, CUEX_INTF_COMPOUND);
		if (impl) {
		    r = cuex_compound_size(impl, e);

		    /* Allocate a new state, update any surrounding μ-bind. */
		    state = state_new(r, e);
		    if (sp_mu)
			sp_mu->state = state;

		    /* Process subexpressions. */
		    source = cuex_compound_comm_iter_source(impl, e);
		    a = 0;
		    ekey = cuex_joinlattice_bottom(CUEX_O2_METAJOIN);
		    while ((ep = cu_ptr_source_get(source))) {
			cuex_t epp = strip(ep);
			if (epp != cuex_o0_metanull()) {
			    ekey = cuex_joinlattice_join(CUEX_O2_METAJOIN,
							 ekey, epp);
			    state->sub[a] = NULL;
			}
			else {
			    state_t substate;

			    substate = initial_partition(
				ep, sp, sp_max, ekey_to_block, lambdavar_block,
				r_max, pending_arr, mudepth, mupath, fvmap);
			    /* The back-refereces all gets the same tag (0) since
			     * we use the commutative view of the compound. */
			    link_substate(state, a, 0, substate);
			    ++a;
			}
		    }
		    ekey = cuex_o2_metapair(ekey, e_type);
		}
	    }
	    if (!state) { /* fall-through from above block */
		/* Allocate a new state, update any surrounding μ-bind. */
		state = state_new(0, e);
		if (sp_mu)
		    sp_mu->state = state;
	    }
	}

	/* Locate or create the block */
	if (cucon_pmap_insert_mem(ekey_to_block, ekey,
				  sizeof(struct block_s), &block))
	    block_cct(block);
	block_insert_state(block, state);
    }
    return state;
}

/* This function is run on the blocks before refine_partition to prepare the
 * initial work and call block_update on the blocks. */
cu_clos_def(add_pending_block, cu_prot(void, void const *key, void *block),
    ( cu_rank_t r_max;
      struct cucon_pset_s *pending_arr; ))
{
    cu_rank_t a;
    cu_clos_self(add_pending_block);
    block_update(block);
    cu_dprintf("cuex.optimal_fold", "Seeding with block %p", block);
    for (a = 0; a < self->r_max; ++a)
	/* We're allowed to omit one block from each pending_arr, but
	 * it won't affect the n log n complexity for this initial
	 * configuration.  For simplicity we therefore omit the
	 * lambdavar_block. */
	cucon_pset_insert(&self->pending_arr[a], block);
}

static cu_bool_t
pick(int r_max, struct cucon_pset_s *pending_arr, int *a, block_t *block_out)
{
    int ap = *a;
    do {
	while (!cucon_pset_is_empty(&pending_arr[ap])) {
	    *block_out = cucon_pset_pop_any(&pending_arr[ap]);
	    if ((*block_out)->selectset_count > ap) {
		*a = ap;
		return cu_true;
	    }
	}
	ap = (ap + 1) % r_max;
    } while (ap != *a);
    return cu_false;
}

static void
refine_partition(int r_max, struct cucon_pset_s *pending_arr)
{
    block_t block;
    int a = 0;

    while (pick(r_max, pending_arr, &a, &block)) {
	block_t update_chain = NULL;
	cucon_slink_t itr_state = block->selectset_arr[a].state_link;
	cu_dprintf("cuex.optimal_fold",
		   "Splitting on %d-triggered transitions to block %p",
		   a, block);
	while (itr_state) {
	    state_t state = cucon_slink_ptr(itr_state);
	    cucon_slink_t itr_invtran = cucon_parr_at(&state->invtran, a);
	    while (itr_invtran) {
		state_t statep;
		block_t block_j, block_k;
		statep = cucon_slink_ptr(itr_invtran);
		block_j = statep->block;	/* selectal block */
		block_k = block_j->target;	/* block to move statep to */

		/* Allocate block_k if necessary. */
		if (!block_k) {
		    /* Construct a new block to split j into. */
		    block_k = cu_gnew(struct block_s);
		    block_j->target = block_k;
		    block_cct(block_k);

		    /* Add block j and, by implication from target link, k to
		     * update chain */
		    block_j->next_to_update = update_chain;
		    update_chain = block_j;
		}

		/* Move statep from block_j to block_k. */
		--block_j->state_count;
		++block_k->state_count;
		cucon_list_erase_node(cu_to(cucon_listnode, statep));
		cucon_list_append_node_cct(&block_k->state_list,
					   cu_to(cucon_listnode, statep));
		statep->block = block_k;
		cu_dprintf("cuex.optimal_fold",
			   "Moved state %p from block %p to block %p.",
			   statep, block_j, block_k);

		itr_invtran = cucon_slink_next(itr_invtran);
	    }
	    itr_state = cucon_slink_next(itr_state);
	}
	while (update_chain) {
	    cu_rank_t ap, ap_max;
	    block_t block_j = update_chain;
	    block_t block_k = update_chain->target;
	    block_update(block_j);
	    block_update(block_k);

	    /* For each input, schedule block j or k. */
	    ap_max = block_j->selectset_count;
	    if (block_k->selectset_count > ap_max)
		ap_max = block_k->selectset_count;
	    for (ap = 0; ap < block_k->selectset_count; ++ap) {
		/* We only need to use one of the blocks for further
		 * refinement.  Choose the smallest one. */
		size_t cnt_j, cnt_k;
		cnt_j = block_j->selectset_count <= ap
		      ? 0 : block_j->selectset_arr[ap].state_count;
		cnt_k = block_k->selectset_count <= ap
		      ? 0 : block_k->selectset_arr[ap].state_count;
		if (cnt_j == 0) {
		    if (cucon_pset_erase(&pending_arr[ap], block_j) && cnt_k)
			cucon_pset_insert(&pending_arr[ap], block_k);
		} else if (cnt_k == 0) {
		    /* Leave block j if pending else we're done here. */
		} else if (cnt_k <= cnt_j) {
		    cucon_pset_insert(&pending_arr[ap], block_k);
		} else {
		    if (!cucon_pset_insert(&pending_arr[ap], block_j))
			cucon_pset_insert(&pending_arr[ap], block_k);
		}
	    }

	    cu_debug_assert(update_chain->target);
	    cu_debug_assert(!block_k->target);
	    update_chain->target = NULL;
	    update_chain = update_chain->next_to_update;
	}
    }
}

static void
reconstruct_binding(block_t block)
{
    cuex_meta_t e_meta;
    state_t state;

#ifndef CU_NDEBUG
    cu_debug_assert(!cucon_list_is_empty(&block->state_list));
    if (cu_debug_key("cuex.optimal_fold")) {
	cu_fprintf(stderr, "BLOCK %p\n", block);
	if (block->target != (block_t)-1) {
	    int a, r;
	    for_states_in_block(state, block) {
		cu_fprintf(stderr, "  STATE %p; %!\n", state, state->e);
		e_meta = cuex_meta(state->e);
		if (cuex_og_hole_contains(e_meta))
		    r = 1;
		else if (cuex_meta_is_opr(e_meta))
		    r = cuex_opr_r(e_meta);
		else
		    r = 0;
		for (a = 0; a < r; ++a) {
		    if (state->sub[a])
			cu_fprintf(stderr, "    SUB %p; block=%p\n",
				   state->sub[a], state->sub[a]->block);
		    else
			cu_fprintf(stderr, "    SUB NULL\n");
		}
	    }
	}
	block->target = (block_t)-1;
    }
#endif

    state = state_from_node(cucon_list_begin(&block->state_list));
    e_meta = cuex_meta(state->e);

    /* Skip back-references for λ-variables, as they already have bind nodes.
     * Note that μ-variables have been replaced by direct links, so they don't
     * trigger here. */
    if (cuex_og_hole_contains(e_meta)) {
	cu_dprintf("cuex.optimal_fold", "λ-variable on state %p", state);
	return;
    }

    if (block->level == -2) {
	cu_dprintf("cuex.optimal_fold",
		   "reconstruct_binding: back ref on %p.", block);
	block->need_mubind = cu_true;
	return;
    }
    if (cuex_meta_is_opr(e_meta)) {
	cu_rank_t a, r;
	r = cuex_opr_r(e_meta);
	block->level = -2;
	for (a = 0; a < r; ++a)
	    if (state->sub[a])
		reconstruct_binding(state->sub[a]->block);
	block->level = -1;
    }
}

static cuex_t
reconstruct(block_t block, int level)
{
    cuex_t e;
    cuex_meta_t e_meta;
    state_t state;

    cu_debug_assert(!cucon_list_is_empty(&block->state_list));
    state = state_from_node(cucon_list_begin(&block->state_list));
    e = state->e;
    e_meta = cuex_meta(e);
    if (cuex_meta_is_opr(e_meta)) {
	cu_rank_t a, r = cuex_opr_r(e_meta);
	cuex_t *arr;

	if (block->level != -1) {  /* We've crossed a μ-variable */
	    cu_debug_assert(block->level >= 0 && block->level <= level);
	    return cuex_hole(level - block->level);
	}
	if (cuex_og_hole_contains(e_meta)) {  /* On a λ-variable */
	    state_t state_ref = state->sub[0];
	    block_t block_ref = state_ref->block;
	    cu_debug_assert(cuex_og_binder_contains(cuex_meta(state_ref->e)));
	    cu_debug_assert(block_ref->level + 1 <= level);
	    return cuex_hole(level - block_ref->level - 1);
	}

	/* Pre: Insert μ-bind */
	if (block->need_mubind)
	    ++level;

	/* Process subexpressions. */
	block->level = level;
	if (cuex_og_binder_contains(e_meta))
	    ++level;
	arr = cu_salloc(sizeof(cuex_t)*r);
	for (a = 0; a < r; ++a) {
	    if (state->sub[a])
		arr[a] = reconstruct(state->sub[a]->block, level);
	    else
		arr[a] = cuex_opn_at(e, a);
	}
	e = cuex_opn_by_arr(e_meta, arr);
	block->level = -1;

	/* Post: Insert μ-bind */
	if (block->need_mubind)
	    e = cuex_o1_mu(e);
    } else if (cuex_meta_is_type(e_meta)) {
	cuoo_type_t type;
	cuex_intf_compound_t impl;

	type = cuoo_type_from_meta(e_meta);
	impl = cuoo_type_impl_ptr(type, CUEX_INTF_COMPOUND);
	if (impl) {
	    cu_ptr_junctor_t junctor;
	    cu_rank_t a;
	    cuex_t ep;

	    if (block->level != -1) {  /* We've crossed a μ-variable */
		cu_debug_assert(block->level >= 0 && block->level <= level);
		return cuex_hole(level - block->level);
	    }

	    /* Pre: Insert μ-bind */
	    if (block->need_mubind)
		++level;

	    /* Process subexpressions. */
	    block->level = level;
	    junctor = cuex_compound_comm_image_junctor(impl, e);
	    a = 0;
	    while ((ep = cu_ptr_junctor_get(junctor))) {
		cuex_t epp;
		if (state->sub[a])
		    epp = reconstruct(state->sub[a]->block, level);
		else
		    epp = ep;
		cu_ptr_junctor_put(junctor, epp);
		++a;
	    }
	    e = cu_ptr_junctor_finish(junctor);
	    block->level = -1;

	    /* Post: Insert μ-bind */
	    if (block->need_mubind)
		e = cuex_o1_mu(e);
	}
    } else
	cu_debug_assert(!block->need_mubind);
    return e;
}

cuex_t
cuex_optimal_fold(cuex_t e)
{
    struct cucon_pmap_s ekey_to_block;
    struct block_s lambdavar_block;
    int depth;
    struct initial_frame_s *sp_max;
    struct cucon_pset_s *pending_arr;
    int a, r_max;
    state_t top_state;
    add_pending_block_t apb_cb;
    cucon_pmap_t fvmap;

    r_max = cuex_max_arity(e);
    if (r_max == 0)
	return e;
    depth = cuex_max_binding_depth(e);
    if (depth == 0)
	return e;
    pending_arr = cu_salloc(sizeof(struct cucon_pset_s)*r_max);
    for (a = 0; a < r_max; ++a)
	cucon_pset_cct(&pending_arr[a]);

    /* Create initial partition. */
    sp_max = cu_salloc(sizeof(struct initial_frame_s)*depth);
    sp_max += depth;
    cucon_pmap_cct(&ekey_to_block);
    block_cct(&lambdavar_block);
    fvmap = cuex_unfolded_fv_sets(e, -1);
    top_state = initial_partition(e, sp_max, sp_max, &ekey_to_block,
				  &lambdavar_block, r_max, pending_arr,
				  0, cuex_mupath_null(), fvmap);

    /* Refine partition. */
    apb_cb.r_max = r_max;
    apb_cb.pending_arr = pending_arr;
    cucon_pmap_iter_mem(&ekey_to_block, add_pending_block_prep(&apb_cb));
    refine_partition(r_max, pending_arr);

    /* Re-construct expression. */
    reconstruct_binding(top_state->block);
    return reconstruct(top_state->block, 0);
}
