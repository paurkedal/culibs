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

cu_dlog_def(_file, "dtag=cuex.optimal_fold");

typedef struct _block_s *_block_t;
typedef struct _buildframe_s *_buildframe_t;
typedef struct _buildstate_s *_buildstate_t;
typedef struct _state_s *_state_t;
typedef struct _state_set_s *_state_set_t;


/* States
 * ====== */

struct _state_s
{
    cu_inherit (cucon_listnode_s);
    _block_t block;
    struct cucon_parr_s invtran; /* of cucon_slink_t of _state_t */
    cuex_t e;
    size_t r;
    _state_t sub[1];
};

struct _state_set_s
{
    cucon_slink_t state_link;
    size_t state_count;
};

CU_SINLINE _state_t
_state_from_node(cucon_listnode_t node)
{ return cu_from(_state, cucon_listnode, node); }

/* Creates a new state with the given arity and expression. */
static _state_t
_state_new(int r, cuex_t e)
{
    _state_t state;
    state = cu_galloc(sizeof(struct _state_s) + (r - 1)*sizeof(_state_t));
    cu_debug_assert(e);
    cucon_parr_init_empty(&state->invtran);
    state->e = e;
    state->block = NULL;
    state->r = r;
    cu_dlogf(_file, "New state %p; r = %d, e = %!", state, r, e);
    return state;
}

/* Connects "state --[a]--> substate" with "b" as the index of the backlink.
 * Usually "a = b", but "b = 0" is used for setlike expressions.  "a" must be
 * smaller than the arity for the source state. */
static void
_state_connect(_state_t state, cu_rank_t a, cu_rank_t b, _state_t substate)
{
    cucon_slink_t *invlink;

    cu_debug_assert(a < state->r);
    state->sub[a] = substate;

    /* Insert 'state ∈ δ⁻¹(substate, b)' */
    if (cucon_parr_size(&substate->invtran) <= b)
	cucon_parr_resize_exactmax_fill(&substate->invtran,
					b + 1, NULL);
    invlink = cucon_parr_ref_at(&substate->invtran, b);
    *invlink = cucon_slink_prepend_ptr(*invlink, state);
    cu_dlogf(_file,
	     "Inserting %p ∈ δ⁻¹(%p, %d) (a = %d, r_max = %d)",
	     state, substate, b, a, cucon_parr_size(&substate->invtran));
}


/* Blocks
 * ====== */

/* Holds various information about an element of a partition. */
struct _block_s
{
    /* The current set of states of this block.  The states are incrementally
     * moved over to the target block during the iteration over t ∈ δ⁻¹(s, a)
     * where s ∈ C(i, a). */
    struct cucon_list_s state_list; /* of _state_s nodes */
    size_t state_count;

    /* The set C(i, a) = { s | s ∈ B(i) ∧ ∃t δ(a, t) = s }, regenerated from
     * scratch on each iteration for changed blocks. */
    size_t occur_set_count;
    struct _state_set_s *occur_set_arr; /* of cucon_slink_t of _state_t */

    /* The target block being built.  This is allocated on demand, which also
     * triggers linking the block on an update link. */
    _block_t target;
    _block_t next_to_update;

    /* For rebuilding expression. */
    cu_bool_t need_mubind;
    int level;
};

/* Initial block for all λ-variables. */
#define LAMBDAVAR_BLOCK 0

#define _for_states_in_block(state, block)				\
    for (state = _state_from_node(cucon_list_begin(&block->state_list));\
	 state != _state_from_node(cucon_list_end(&block->state_list));	\
	 state = _state_from_node(					\
	     cucon_listnode_next(cu_to(cucon_listnode, state))))

static void
_block_init(_block_t block)
{
    block->state_count = 0;
    cucon_list_init(&block->state_list);
    block->target = NULL;
    /* block->occur_set_arr is created by _block_reindex */
    /* block->next_to_update is used internally in _refine_partition */

    block->need_mubind = cu_false;
    block->level = -1;
}

static void
_block_insert_state(_block_t block, _state_t state)
{
    cucon_list_append_init_node(
	&block->state_list, cu_to(cucon_listnode, state));
    ++block->state_count;
    state->block = block;
    cu_dlogf(_file, "Added state %p to block %p", state, block);
}

/* Update the "occur_set_arr" of block after splitting it. */
static void
_block_reindex(_block_t block)
{
    cu_rank_t a, r_max = 0;
    _state_t state;
    _for_states_in_block(state, block) {
	int a = cucon_parr_size(&state->invtran);
	if (a > r_max)
	    r_max = a;
    }
    block->occur_set_count = r_max;
    block->occur_set_arr = cu_galloc(sizeof(struct _state_set_s)*r_max);
    for (a = 0; a < r_max; ++a) {
	block->occur_set_arr[a].state_count = 0;
	block->occur_set_arr[a].state_link = NULL;
    }
    /* Compute "block->occur_set_arr[a] = { state | δ⁻¹(state, a) ≠ ∅ }". */
    cu_dlogf(_file, "Update block %p, r_max = %d", block, r_max);
    _for_states_in_block(state, block) {
	cucon_slink_t *itr_invtran;
	struct _state_set_s *itr_selectset;
	a = cucon_parr_size(&state->invtran);
	itr_invtran = cucon_parr_ref_at(&state->invtran, a);
	itr_selectset = block->occur_set_arr + a;
	while (a > 0) {
	    --a;
	    --itr_invtran;
	    --itr_selectset;
	    if (*itr_invtran) { /* δ⁻¹(state, a) ≠ ∅ so state ∈ C(i, a) */
		++itr_selectset->state_count;
		itr_selectset->state_link =
		    cucon_slink_prepend_ptr(itr_selectset->state_link, state);
	    }
	}
    }
}


/* Partition State
 * =============== */

struct _buildframe_s
{
    _state_t state;
    cuex_t e;
    struct cucon_uset_s fvset;

    /* On a μ-frame this is the set of de Bruijn indices of the free
     * λ-variables in the body of the μ-expression. */
    cucon_uset_t mfvset;
};

struct _buildstate_s
{
    int r_max;

    /* Maps from pruned expressions to the block of the initial partition where
     * the corresponding state belongs. */
    struct cucon_pmap_s ekey_to_block;

    /* This block contains the states of all λ-variables.  Each of these states
     * link back to the state of the expression binding the variable. */
    struct _block_s lambdavar_block;

    /* Maps from each μ-path to a cucon_uset_s of indices of the free
     * λ-variables which are free in that μ-expression. */
    cucon_pmap_t fvmap;

    /* The biggest stack address, points just after the bottom element. */
    _buildframe_t sp_max;
};

static cu_bool_t
_buildstate_init(_buildstate_t bst, cuex_t e)
{
    int depth;
    int r_max = cuex_max_arity(e);
    if (r_max == 0)
	return cu_false;
    if (r_max < 2)
	r_max = 2; /* label nodes for commutative view of labellings */
    depth = cuex_max_binding_depth(e);
    if (depth == 0)
	return cu_false;
    bst->r_max = r_max;
    cucon_pmap_init(&bst->ekey_to_block);
    _block_init(&bst->lambdavar_block);
    bst->fvmap = cuex_unfolded_fv_sets(e, -1);
    bst->sp_max = cu_galloc(sizeof(struct _buildframe_s)*depth);
    bst->sp_max += depth;
    return cu_true;
}


/* Initial Partition
 * ================= */

static cuex_t
strip(cuex_t e)
{
    cuex_meta_t e_meta = cuex_meta(e);
    if (cuex_meta_is_opr(e_meta))
	return cuex_o0_metanull();
    else if (cuex_meta_is_type(e_meta)) {
	cuoo_type_t type = cuoo_type_from_meta(e_meta);
	if (cuoo_type_impl(type, CUEX_INTF_COMPOUND))
	    return cuex_o0_metanull();
    }
    return e;
}

static void
_add_FV_to_parent_frames(_buildframe_t sp, _buildframe_t sp_max, int var_bi)
{
    while (var_bi > 0 && sp < sp_max) {
	if (!sp->mfvset) {
	    if (!cucon_uset_insert(&sp->fvset, var_bi))
		return;
	}
	--var_bi;
	++sp;
    }
}

cu_clos_def(_add_FV_to_parent_frames_clos, cu_prot(void, uintptr_t var_bi),
    ( _buildframe_t sp, sp_max;
      int shift; ))
{
    cu_clos_self(_add_FV_to_parent_frames_clos);
    _add_FV_to_parent_frames(self->sp, self->sp_max, var_bi + self->shift);
}

static _state_t
_build_partition(_buildstate_t bst, cuex_t e, _buildframe_t sp,
		 int mudepth, cuex_t mupath)
{
    cuex_meta_t e_meta;
    _buildframe_t sp_mu;
    _state_t state;
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
	cu_dlogf(_file, "Pushing μ-frame, new level %d", bst->sp_max - sp);
	sp_mu->e = e;
#ifndef CU_NDEBUG
	sp_mu->state = NULL;
#endif
	mupath = cuex_mupath_pair(mudepth + 1, mupath, e);
	mudepth = 0;
	sp_mu->mfvset = cucon_pmap_find_mem(bst->fvmap, mupath);
	cu_debug_assert(sp_mu->mfvset);
	e = cuex_opn_at(e, 0);
	e_meta = cuex_meta(e);
    }
    else
	sp_mu = NULL;

    if (cuex_meta_is_opr(e_meta) && cuex_og_hole_contains(e_meta)) {
	int var_bi;
	_buildframe_t sp_ref;

	/* == λ- and μ-Variables == */

	var_bi = cuex_oa_hole_index(e_meta);
	sp_ref = sp + var_bi;
	cu_debug_assert(sp_ref < bst->sp_max); /* TODO */
	cu_debug_assert(sp_ref->state);
	if (cuex_meta(sp_ref->e) == CUEX_O1_MU) {
	    _add_FV_to_parent_frames_clos_t add_fv_cb;

	    /* μ-variable are equivalent to what they refer to */
	    cu_debug_assert(!sp_mu || var_bi != 0);
	    cu_debug_assert(sp_ref->state != NULL);
	    state = sp_ref->state;
	    cu_dlogf(_file, "Ref μ variable, index=%d; %!", var_bi, sp_ref->e);
	    cu_debug_assert(sp_ref->mfvset);
	    add_fv_cb.sp = sp;
	    add_fv_cb.sp_max = bst->sp_max;
	    add_fv_cb.shift = var_bi;
	    cucon_uset_iter(sp_ref->mfvset,
			    _add_FV_to_parent_frames_clos_prep(&add_fv_cb));
	}
	else { /* e is a λ variable */
	    /* λ-variables have individual states in a shared block.  We
	     * synthetically link the point of reference as a substate. */
	    state = _state_new(1, e);
	    _block_insert_state(&bst->lambdavar_block, state);
	    _state_connect(state, 0, 0, sp_ref->state);
	    cu_dlogf(_file, "Ref λ variable, index=%d; %!", var_bi, sp_ref->e);
	    _add_FV_to_parent_frames(sp, bst->sp_max, var_bi);
	}
	if (sp_mu)
	    sp_mu->state = state;
    }
    else {
	_block_t block;
	cuex_t ekey = e;

	/* == Structural Expressions and λ-Bind == */

	if (cuex_meta_is_opr(e_meta)) {

	    /* Allocate a new state. */
	    r = cuex_opr_r(e_meta);
	    cu_debug_assert(r <= bst->r_max);
	    state = _state_new(r, e);

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
		cu_dlogf(_file, "Pushing λ-frame, new level %d",
			 bst->sp_max - sp);
		sp->state = state;
		sp->e = e;
		sp->mfvset = NULL;
		cucon_uset_init(&sp->fvset);
	    }

	    /* Process subexpressions and add transitions for δ and δ⁻¹. */
	    for (a = 0; a < r; ++a) {
		if (cuex_opn_at(ekey, a) != cuex_o0_metanull())
		    state->sub[a] = NULL;
		else {
		    _state_t substate;

		    substate = _build_partition(bst, cuex_opn_at(e, a), sp,
						mudepth, mupath);
		    _state_connect(state, a, a, substate);
		}
	    }

	    if (cuex_og_binder_contains(e_meta)) {
		cu_dlogf(_file,
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
		cu_dlogf(_file, "Found compound %!.", e);

		/* Deal with compounds.  This is analogous to operations,
		 * except that we use the commutative view in order to avoid
		 * high arities. */
		impl = cuoo_type_impl_ptr(e_type, CUEX_INTF_COMPOUND);
		if (impl) {
		    r = cuex_compound_size(impl, e);

		    /* Allocate a new state, update any surrounding μ-bind. */
		    state = _state_new(r, e);
		    if (sp_mu)
			sp_mu->state = state;

		    /* Process subexpressions. */
		    source = cuex_compound_comm_iter_source(impl, e);
		    ekey = cuex_joinlattice_bottom(CUEX_O2_METAJOIN);
		    for (a = 0; (ep = cu_ptr_source_get(source)); ++a) {
			cuex_t epp = strip(ep);
			if (epp != cuex_o0_metanull()) {
			    ekey = cuex_joinlattice_join(CUEX_O2_METAJOIN,
							 ekey, epp);
			    state->sub[a] = NULL;
			}
			else {
			    _state_t substate;

			    substate = _build_partition(bst, ep, sp,
							mudepth, mupath);
			    /* The back-refereces all gets the same tag (0) since
			     * we use the commutative view of the compound. */
			    _state_connect(state, a, 0, substate);
			}
		    }
		    cu_debug_assert(a == r);
		    ekey = cuex_o2_metapair(ekey, e_type);
		}
	    }
	    if (!state) { /* fall-through from above block */
		/* Allocate a new state, update any surrounding μ-bind. */
		state = _state_new(0, e);
		if (sp_mu)
		    sp_mu->state = state;
	    }
	}

	/* Locate or create the block */
	if (cucon_pmap_insert_mem(&bst->ekey_to_block, ekey,
				  sizeof(struct _block_s), &block)) {
	    _block_init(block);
	    cu_dlogf(_file, "New block %p: %!", block, e);
	} else
	    cu_dlogf(_file, "Old block %p: %!", block, e);
	_block_insert_state(block, state);
    }
    return state;
}


/* Partition Refinement
 * ==================== */

/* This function is run on the blocks before _refine_partition to prepare the
 * initial work and call _block_reindex on the blocks. */
cu_clos_def(_add_pending_block, cu_prot(void, void const *key, void *block),
    ( cu_rank_t r_max;
      struct cucon_pset_s *pending_arr; ))
{
    cu_rank_t a;
    cu_clos_self(_add_pending_block);
    _block_reindex(block);
    cu_debug_assert(((_block_t)block)->occur_set_count <= self->r_max);
    cu_dlogf(_file, "Seeding with block %p", block);
    for (a = 0; a < self->r_max; ++a)
	/* We're allowed to omit one block from each pending_arr, but
	 * it won't affect the n log n complexity for this initial
	 * configuration.  For simplicity we therefore omit the
	 * lambdavar_block. */
	cucon_pset_insert(&self->pending_arr[a], block);
}

/* Pick the next operand number and block by which to refine the partition. */
static cu_bool_t
_pick_block(int r_max, struct cucon_pset_s *pending_arr,
	    int *a_inout, _block_t *block_out)
{
    int ap = *a_inout;
    do {
	while (!cucon_pset_is_empty(&pending_arr[ap])) {
	    *block_out = cucon_pset_pop_any(&pending_arr[ap]);
	    if ((*block_out)->occur_set_count > ap) {
		*a_inout = ap;
		return cu_true;
	    }
	}
	ap = (ap + 1) % r_max;
    } while (ap != *a_inout);
    return cu_false;
}

/* Iteratively refine the partition until it's stable.  pending_arr[i] is the
 * set of blocks which will be use to refine the partition, based on operand
 * number i of the states. */
static void
_refine_partition(int r_max, struct cucon_pset_s *pending_arr)
{
    _block_t block;
    int a = 0;

    while (_pick_block(r_max, pending_arr, &a, &block)) {
	_block_t update_chain = NULL;
	cucon_slink_t itr_state = block->occur_set_arr[a].state_link;
	cu_dlogf(_file,
		 "Splitting on %d-triggered transitions to block %p",
		 a, block);
	while (itr_state) {
	    _state_t state = cucon_slink_ptr(itr_state);
	    cucon_slink_t itr_invtran = cucon_parr_at(&state->invtran, a);
	    while (itr_invtran) {
		_state_t statep;
		_block_t block_j, block_k;
		statep = cucon_slink_ptr(itr_invtran);
		block_j = statep->block;	/* selectal block */
		block_k = block_j->target;	/* block to move statep to */

		/* Allocate block_k if necessary. */
		if (!block_k) {
		    /* Construct a new block to split j into. */
		    block_k = cu_gnew(struct _block_s);
		    block_j->target = block_k;
		    _block_init(block_k);

		    /* Add block j and, by implication from target link, k to
		     * update chain */
		    block_j->next_to_update = update_chain;
		    update_chain = block_j;
		}

		/* Move statep from block_j to block_k. */
		--block_j->state_count;
		++block_k->state_count;
		cucon_list_erase_node(cu_to(cucon_listnode, statep));
		cucon_list_append_init_node(&block_k->state_list,
					    cu_to(cucon_listnode, statep));
		statep->block = block_k;
		cu_dlogf(_file,
			 "Moved state %p from block %p to block %p.",
			 statep, block_j, block_k);

		itr_invtran = cucon_slink_next(itr_invtran);
	    }
	    itr_state = cucon_slink_next(itr_state);
	}
	while (update_chain) {
	    cu_rank_t ap, ap_max;
	    _block_t block_j = update_chain;
	    _block_t block_k = update_chain->target;
	    _block_reindex(block_j);
	    _block_reindex(block_k);

	    /* For each input, schedule block j or k. */
	    ap_max = block_j->occur_set_count;
	    if (block_k->occur_set_count > ap_max)
		ap_max = block_k->occur_set_count;
	    cu_debug_assert(block_k->occur_set_count <= r_max);
	    for (ap = 0; ap < block_k->occur_set_count; ++ap) {
		/* We only need to use one of the blocks for further
		 * refinement.  Choose the smallest one. */
		size_t cnt_j, cnt_k;
		cnt_j = block_j->occur_set_count <= ap
		      ? 0 : block_j->occur_set_arr[ap].state_count;
		cnt_k = block_k->occur_set_count <= ap
		      ? 0 : block_k->occur_set_arr[ap].state_count;
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


/* Reconstruction of μ-Expression
 * ============================== */

static void
reconstruct_binding(_block_t block)
{
    cuex_meta_t e_meta;
    _state_t state;

    /* Debug Output */
#ifndef CU_NDEBUG
    cu_debug_assert(!cucon_list_is_empty(&block->state_list));
    if (cu_debug_key("cuex.optimal_fold")) {
	cu_fprintf(stderr, "BLOCK %p\n", block);
	if (block->target != (_block_t)-1) {
	    int a, r;
	    _for_states_in_block(state, block) {
		cu_fprintf(stderr, "  STATE %p; %!\n", state, state->e);
		e_meta = cuex_meta(state->e);
		r = state->r;
		if (cuex_og_hole_contains(e_meta))
		    cu_debug_assert(r == 1);
		else if (cuex_meta_is_opr(e_meta))
		    cu_debug_assert(r == cuex_opr_r(e_meta));
		for (a = 0; a < r; ++a) {
		    if (state->sub[a])
			cu_fprintf(stderr, "    SUB %p; block=%p\n",
				   state->sub[a], state->sub[a]->block);
		    else
			cu_fprintf(stderr, "    SUB NULL\n");
		}
	    }
	}
	block->target = (_block_t)-1;
    }
#endif

    state = _state_from_node(cucon_list_begin(&block->state_list));
    e_meta = cuex_meta(state->e);

    /* Skip back-references for λ-variables, as they already have bind nodes.
     * Note that μ-variables have been replaced by direct links, so they don't
     * trigger here. */
    if (cuex_og_hole_contains(e_meta)) {
	cu_dlogf(_file, "λ-variable on state %p", state);
	return;
    }

    if (block->level == -2 && !cuex_og_metaregular_contains(e_meta)) {
	cu_dlogf(_file, "reconstruct_binding: back ref on %p.", block);
	block->need_mubind = cu_true;
	return;
    }
    if (state->r) {
	size_t a, r;
	r = state->r;
	block->level = -2;
	for (a = 0; a < r; ++a)
	    if (state->sub[a])
		reconstruct_binding(state->sub[a]->block);
	block->level = -1;
    }
}

static cuex_t
reconstruct(_block_t block, int level)
{
    cuex_t e;
    cuex_meta_t e_meta;
    _state_t state;

    cu_debug_assert(!cucon_list_is_empty(&block->state_list));
    state = _state_from_node(cucon_list_begin(&block->state_list));
    e = state->e;
    e_meta = cuex_meta(e);
    if (cuex_meta_is_opr(e_meta)) {
	cu_rank_t a, r = cuex_opr_r(e_meta);
	cuex_t *arr;

	if (block->level != -1 && !cuex_og_metaregular_contains(e_meta)) {
	    /* We've crossed a μ-variable */
	    cu_debug_assert(block->level >= 0 && block->level <= level);
	    return cuex_hole(level - block->level);
	}
	if (cuex_og_hole_contains(e_meta)) {  /* On a λ-variable */
	    _state_t state_ref = state->sub[0];
	    _block_t block_ref = state_ref->block;
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
	    size_t a;
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
	    cu_debug_assert(a == state->r);
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


/* API
 * === */

cuex_t
cuex_optimal_fold(cuex_t e)
{
    int a;
    struct _buildstate_s bst;
    _state_t top_state;
    _add_pending_block_t apb_cb;
    struct cucon_pset_s *pending_arr;

    /* Create initial partition. */
    if (!_buildstate_init(&bst, e))
	return e;
    top_state = _build_partition(&bst, e, bst.sp_max, 0, cuex_mupath_null());

    /* Refine partition. */
    pending_arr = cu_salloc(sizeof(struct cucon_pset_s)*bst.r_max);
    for (a = 0; a < bst.r_max; ++a)
	cucon_pset_init(&pending_arr[a]);
    apb_cb.r_max = bst.r_max;
    apb_cb.pending_arr = pending_arr;
    cucon_pmap_iter_mem(&bst.ekey_to_block, _add_pending_block_prep(&apb_cb));
    _refine_partition(bst.r_max, pending_arr);

    /* Re-construct expression. */
    reconstruct_binding(top_state->block);
    return reconstruct(top_state->block, 0);
}
