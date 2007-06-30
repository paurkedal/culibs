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

#include <cuex/ex.h>
#include <cuex/opn.h>
#include <cuex/var.h>
#include <cuex/algo.h>
#include <cuex/oprdefs.h>
#include <cuex/binding.h>
#include <cucon/pmap.h>
#include <cucon/uset.h>
#include <cucon/pset.h>
#include <cucon/parr.h>
#include <cucon/list.h>
#include <cucon/stack.h>
#include <cu/clos.h>
#include <limits.h>

#define DEBUG_DUMP_PART 0

typedef struct partinfo_s *partinfo_t;
struct partinfo_s
{
    struct cucon_list_s Bi; /* of inherited state_s */
    struct cucon_parr_s Ci; /* of cucon_list_t of cuex_t */
};

typedef struct state_s *state_t;
struct state_s
{
    cu_inherit (cucon_listnode_s);
    cuex_t e;
    int j;
    cu_bool_t had_mu;
    state_t sub[1];
};

typedef struct mu_initial_muf_s *mu_initial_muf_t;
struct mu_initial_muf_s
{
    cuex_t e;
    state_t s;
    partinfo_t BCi;
    int fii;  /* fold-inert index if a λ-frame, -1 otherwise */
};

CU_SINLINE cu_bool_t mu_initial_muf_is_mubind(mu_initial_muf_t muf)
{ return muf->BCi != NULL; }

CU_SINLINE cuex_t
crop_rec_helper(cuex_t e, cucon_stack_t mus)
{
    cuex_meta_t meta = cuex_meta(e);
    if (cuex_meta_is_opr(meta)) {
	if (cuex_og_hole_contains(meta)) {
	    int index;
	    mu_initial_muf_t muf;

	    index = cuex_oa_hole_index(meta);
	    /* FIXME deal with escaping indices */
	    muf = cucon_stack_at(mus, index*sizeof(struct mu_initial_muf_s));
	    if (!mu_initial_muf_is_mubind(muf))  /* if a λ-variable */
		return cuex_ivar_n(muf->fii);
	}
	return cuex_unknown();
    }
    else
	return e;
}

    static cuex_t
crop_rec(cuex_t e, cucon_stack_t mus,
	 int level, int last_mu_level, cuex_t fiictx, cucon_pmap_t fiimap)
{
    cuex_meta_t meta = cuex_meta(e);
    cu_debug_assert(meta != CUEX_O1_MU);
    if (cuex_meta_is_opr(meta)) {
	cu_debug_assert(!cuex_og_hole_contains(meta));
	if (cuex_og_binder_contains(meta)) {

	    /* It's a λ-bind, so create a frame to the benefit of
	     * crop_rec_helper. */
	    mu_initial_muf_t muf;
	    muf = cucon_stack_alloc(mus, sizeof(struct mu_initial_muf_s));
	    ++level;
	    muf->e = e;
	    muf->BCi = NULL;
	    muf->fii = cucon_pmap_find_int(
		fiimap,
		cuex_mupath_pair(level - last_mu_level, fiictx, e));
	    cu_debug_assert(muf->fii >= 0);

	    /* Then rewrite to explicit λ-bind in order to include the variable
	     * as part of the key. */
	    e = cuex_o2_lambda(cuex_ivar_n(muf->fii),
			       crop_rec_helper(cuex_opn_at(e, 0), mus));
	    cucon_stack_free(mus, sizeof(struct mu_initial_muf_s));
	    return e;
	}

	/* All other operations: Just crop subexpressions. */
	CUEX_OPN_TRAN(meta, e, ep, crop_rec_helper(ep, mus));
    }
    return e;
}

static state_t
mu_initial(cuex_t e, cucon_parr_t BC, cucon_stack_t mus,
	   int last_mu_level, cuex_t fiictx, cucon_pmap_t fiimap,
	   cucon_pmap_t crox_to_partno,
	   size_t *state_count_accu,
	   partinfo_t *BCi_out, struct cucon_pmap_s *inv_delta)
{
    int level;
    cuex_meta_t e_meta = cuex_meta(e);
    int *index;
    partinfo_t BCi;
    cuex_t crox;
    state_t s;
    cu_rank_t r;
    mu_initial_muf_t muf_outer;

    level = cucon_stack_size(mus)/sizeof(struct mu_initial_muf_s);
    if (e_meta == CUEX_O1_MU) {
	/* Process μ-bind, and the body in the same call.  This allows us to
	 * link in s and BCi for the body before doing the full processing of
	 * the body. */
	++level;
	fiictx = cuex_mupath_pair(level - last_mu_level, fiictx, e);
	e = cuex_opn_at(e, 0);
	e_meta = cuex_meta(e);
	cu_debug_assert(cuex_meta_is_opr(e_meta));
	muf_outer = cucon_stack_alloc(mus, sizeof(struct mu_initial_muf_s));
	muf_outer->e = e;
	muf_outer->BCi = (void *)-1; /* indicate μ-frame but set value later */
	muf_outer->fii = -1;
	last_mu_level = level;
    }
    else
	muf_outer = NULL;

    /* Insert 'e', as a state, into the partition identified by
     * it's non-recursive structure. */
    crox = crop_rec(e, mus, level, last_mu_level, fiictx, fiimap);
    if (cucon_pmap_insert_mem(crox_to_partno, crox, sizeof(int), &index)) {
	*index = cucon_pmap_size(crox_to_partno) - 1;
	BCi = cu_gnew(struct partinfo_s);
	cucon_list_cct(&BCi->Bi);
	cucon_parr_cct_empty(&BCi->Ci);
	cucon_parr_append_gp(BC, BCi);
	cu_debug_assert(cucon_parr_size(BC) == *index + 1);
    }
    else
	BCi = cucon_parr_at(BC, *index);

    ++*state_count_accu;
    r = cuex_meta_is_opr(e_meta)? cuex_opr_r(e_meta) : 0;
    s = cu_galloc(sizeof(struct state_s) + sizeof(state_t)*(r - 1));
    s->e = e;
    s->j = *index;
    s->had_mu = muf_outer != NULL;
    cucon_list_append_node_cct(&BCi->Bi, cu_to(cucon_listnode, s));

    if (muf_outer) {
	/* Finish the outer frame if we hit a μ-bind. */
	muf_outer->s = s;
	muf_outer->BCi = BCi;
    }

    if (cuex_meta_is_opr(e_meta)) {
	cu_rank_t a;

	if (cuex_og_binder_contains(e_meta)) { /* λ-frame */
	    mu_initial_muf_t muf;
	    muf = cucon_stack_alloc(mus, sizeof(struct mu_initial_muf_s));
	    ++level;
	    muf->e = e;
	    muf->s = NULL;
	    muf->BCi = NULL;
	    muf->fii = cucon_pmap_find_int(
		fiimap,
		cuex_mupath_pair(level - last_mu_level, fiictx, e));
	    cu_debug_assert(muf->fii != INT_MIN);
	}

	for (a = 0; a < r; ++a) {
	    cuex_t ep;
	    cuex_meta_t ep_meta;
	    int ap;
	    state_t sp;
	    partinfo_t BCj;
	    cucon_list_t inv_delta_a_s;
	    cucon_list_t Cj;

	    /* Check for recursion, if none, process subexpression. */
	    ep = cuex_opn_at(e, a);
	    ep_meta = cuex_meta(ep);
	    if (!cuex_meta_is_opr(ep_meta)) {
		/* Operand a was part of the key identifying s. */
		s->sub[a] = NULL;
		continue;
	    }
	    if (cuex_og_hole_contains(ep_meta)) {
		int i = cuex_oa_hole_index(ep_meta);
		mu_initial_muf_t lfr;
		if (i*sizeof(struct mu_initial_muf_s)
		    < cucon_stack_size(mus)) {
		    lfr = cucon_stack_at(mus,
					 i*sizeof(struct mu_initial_muf_s));
		    if (!mu_initial_muf_is_mubind(lfr)) {
			cu_debug_assert(lfr->s == NULL);
			s->sub[a] = lfr->s; /* CHECKME */
			continue;
		    }
		    sp = lfr->s;
		    BCj = lfr->BCi;
		}
		else {
		    s->sub[a] = NULL; /* XXX add the case to key */
		    continue;
		}
	    }
	    else {
		sp = mu_initial(ep, BC, mus,
				last_mu_level, fiictx, fiimap, crox_to_partno,
				state_count_accu, &BCj, inv_delta);
	    }
	    s->sub[a] = sp;

	    /* Add s ∈ δ⁻¹(s′, a). */
	    if (cucon_pmap_insert_mem(&inv_delta[a], sp,
				      sizeof(struct cucon_list_s),
				      &inv_delta_a_s))
		cucon_list_cct(inv_delta_a_s);
	    cucon_list_append_ptr(inv_delta_a_s, s);

	    /* Here, δ⁻¹(s′, a) ≠ ∅, so add s′ to C(j, a). */
	    ap = cucon_parr_size(&BCj->Ci);
	    if (ap <= a) {
		cucon_parr_resize_gpmax(&BCj->Ci, a + 1);
		while (ap < a)
		    cucon_parr_set_at(&BCj->Ci, ap++, NULL);
		Cj = cucon_list_new();
		cucon_parr_set_at(&BCj->Ci, a, Cj);
	    }
	    else {
		Cj = cucon_parr_at(&BCj->Ci, a);
		if (!Cj)
		    Cj = cucon_list_new();
		cucon_parr_set_at(&BCj->Ci, a, Cj);
	    }
	    cucon_list_append_ptr(Cj, sp);
	}

	if (cuex_og_binder_contains(e_meta))
	    cucon_stack_free(mus, sizeof(struct mu_initial_muf_s));
    }
    if (muf_outer)
	cucon_stack_free(mus, sizeof(struct mu_initial_muf_s));
    *BCi_out = BCi;
    return s;
}

static int
mu_pick_position_and_partno(struct cucon_uset_s *L, int a_max, int *a)
{
    int a_start = *a;
    cu_debug_assert(a_max > 0);
    do {
	if (!cucon_uset_is_empty(&L[*a]))
	    return cucon_uset_pop_any(&L[*a]);
	if (++*a >= a_max)
	    *a = 0;
    } while (*a != a_start);
    return -1;
}

void
mu_rebuild_C(partinfo_t BCj, struct cucon_pmap_s *inv_delta, size_t *Cj_sizes)
{
    int a;
    int Cj_a_max = cucon_parr_size(&BCj->Ci);
    for (a = 0; a < Cj_a_max; ++a) {
	cucon_listnode_t it_Bj;
	cucon_pmap_t inv_delta_a = &inv_delta[a];
	cucon_list_t Cja = cucon_list_new();
	size_t size = 0;
	for (it_Bj = cucon_list_begin(&BCj->Bi);
	     it_Bj != cucon_list_end(&BCj->Bi);
	     it_Bj = cucon_listnode_next(it_Bj)) {
	    state_t s = cu_from(state, cucon_listnode, it_Bj);
	    cucon_list_t inv_delta_a_s = cucon_pmap_find_mem(inv_delta_a, s);
	    if (inv_delta_a_s) {
		cu_debug_assert(!cucon_list_is_empty(inv_delta_a_s));
		cucon_list_append_ptr(Cja, s);
		++size;
	    }
	}
	if (size == 0)
	    Cja = NULL;
	cucon_parr_set_at(&BCj->Ci, a, Cja);
	Cj_sizes[a] = size;
    }
}

static void
mu_refine_partition(int a_max, size_t state_count,
		    struct cucon_uset_s *L, cucon_parr_t BC,
		    struct cucon_pmap_s *inv_delta)
{
    int i;
    int a;
    partinfo_t *BCpp;
    int *j_split_begin, *j_split_end;
    size_t *Cj_sizes, *Ck_sizes;

    BCpp = cu_salloc(state_count*sizeof(partinfo_t));
    memset(BCpp, 0, state_count*sizeof(partinfo_t));
    j_split_begin = cu_salloc(state_count*sizeof(int));
    Cj_sizes = cu_salloc(a_max*sizeof(size_t));
    Ck_sizes = cu_salloc(a_max*sizeof(size_t));

    a = 0;
    while ((i = mu_pick_position_and_partno(L, a_max, &a)) >= 0) {
	partinfo_t BCi;
	cucon_list_t Cia;
	cucon_listnode_t Cia_node;
	cucon_pmap_t inv_delta_a = &inv_delta[a];

	BCi = cucon_parr_at(BC, i);
	cu_debug_assert(a < cucon_parr_size(&BCi->Ci));
	Cia = cucon_parr_at(&BCi->Ci, a);
	if (!Cia)
	    continue;

	/* Move {t ∈ B(j) | δ(t, a) ∈ C(i, a)} from B(j) to B″(j). */
	j_split_end = j_split_begin;
	for (Cia_node = cucon_list_begin(Cia);
	     Cia_node != cucon_list_end(Cia);
	     Cia_node = cucon_listnode_next(Cia_node)) {
	    cucon_listnode_t Bj_node;
	    state_t s = cucon_listnode_ptr(Cia_node);
	    cucon_list_t delta_a_s = cucon_pmap_find_mem(inv_delta_a, s);
	    cucon_listnode_t t_node;
	    cu_debug_assert(delta_a_s);
	    for (t_node = cucon_list_begin(delta_a_s);
		 t_node != cucon_list_end(delta_a_s);
		 t_node = cucon_listnode_next(t_node)) {
		state_t t = cucon_listnode_ptr(t_node);
		int j = t->j;
		partinfo_t BCj = cucon_parr_at(BC, j);
		if (!BCpp[j]) {
		    BCpp[j] = cu_gnew(struct partinfo_s);
		    cucon_list_cct(&BCpp[j]->Bi);
		    cucon_list_swap(&BCpp[j]->Bi, &BCj->Bi);
		    cu_debug_assert(j_split_end - j_split_begin < state_count);
		    *j_split_end++ = j;
		}
		Bj_node = cu_to(cucon_listnode, t);
		cucon_list_erase_node(Bj_node);
		cucon_list_append_node_cct(&BCj->Bi, Bj_node);
	    }
	}

	/* Build Cj and Ck, update L, and reindex moved states. */
	while (--j_split_end >= j_split_begin) {
	    int j = *j_split_end;
	    partinfo_t BCppj = BCpp[j];
	    if (BCppj && !cucon_list_is_empty(&BCppj->Bi)) {
		int k = cucon_parr_size(BC);
		partinfo_t BCj = cucon_parr_at(BC, j);
		cucon_listnode_t Bk_node;
		int Cj_a_max = cucon_parr_size(&BCj->Ci);
		int ap;

		mu_rebuild_C(BCj, inv_delta, Cj_sizes);
		cucon_parr_cct_size(&BCppj->Ci, Cj_a_max);
		mu_rebuild_C(BCppj, inv_delta, Ck_sizes);

		/* Reindex moved states. */
		for (Bk_node = cucon_list_begin(&BCppj->Bi);
		     Bk_node != cucon_list_end(&BCppj->Bi);
		     Bk_node = cucon_listnode_next(Bk_node)) {
		    state_t u = cu_from(state, cucon_listnode, Bk_node);
		    u->j = k;
		}
		cucon_parr_append_gp(BC, BCppj);

		/* Add to L. */
		for (ap = 0; ap < Cj_a_max; ++ap) {
		    cucon_uset_t Lap = &L[ap];
		    if (0 < Cj_sizes[ap] && Cj_sizes[ap] <= Ck_sizes[ap]
			&& !cucon_uset_find(Lap, j))
			cucon_uset_insert(Lap, j);
		    else
			cucon_uset_insert(Lap, k);
		}
	    }
	    BCpp[j] = NULL;
	}
    }
}

struct rebuildinfo_s
{
    cu_bool_least_t processing;
    cu_bool_least_t need_mu_bind;
    int mu_bind_level;
};

/* recurse and detect feedback to determine μ* indices. */
static void
mu_rebuild_feedback(state_t s, struct rebuildinfo_s *rb_arr)
{
    struct rebuildinfo_s *rb = &rb_arr[s->j];
    if (rb->processing)
	rb->need_mu_bind = cu_true;
    else {
	cu_rank_t a, r;
	r = cuex_opn_r(s->e);
	rb->processing = cu_true;
	for (a = 0; a < r; ++a)
	    if (s->sub[a])
		mu_rebuild_feedback(s->sub[a], rb_arr);
	rb->processing = cu_false;
    }
}

typedef struct mu_rebuild_ex_frame_s {
    int lev;
} *mu_rebuild_ex_frame_t;

/* lev - the current new level, generated according to new μ-bindings
 * stack - the old stack layout with info about corrected levels */
static cuex_t
mu_rebuild_ex(state_t s, struct rebuildinfo_s *rb_arr, int lev,
	      cucon_stack_t stack)
{
    struct rebuildinfo_s *rb = &rb_arr[s->j];
    cuex_meta_t meta;

    if (rb->processing) {
	cu_debug_assert(lev >= rb->mu_bind_level);
	return cuex_hole(lev - rb->mu_bind_level);
    }
    meta = cuex_meta(s->e);
    if (cuex_meta_is_opr(meta)) {
	cu_rank_t a, r;
	cuex_t *e_arr, e;
	int bind_cnt = 0;

	if (rb->need_mu_bind) {
	    ++lev;
	    rb->mu_bind_level = lev;
	}
	if (s->had_mu) {
	    mu_rebuild_ex_frame_t fr;
	    fr = cucon_stack_alloc(stack, sizeof(struct mu_rebuild_ex_frame_s));
	    fr->lev = -1;
	    ++bind_cnt;
	}
	if (cuex_og_binder_contains(meta)) {
	    mu_rebuild_ex_frame_t fr;
	    fr = cucon_stack_alloc(stack, sizeof(struct mu_rebuild_ex_frame_s));
	    fr->lev = ++lev;
	    ++bind_cnt;
	}

	r = cuex_opr_r(meta);
	e_arr = cu_salloc(sizeof(cuex_t)*r);
	rb->processing = cu_true;
	for (a = 0; a < r; ++a) {
	    if (s->sub[a])
		e_arr[a] = mu_rebuild_ex(s->sub[a], rb_arr, lev, stack);
	    else {
		cuex_t ep = cuex_opn_at(s->e, a);
		cuex_meta_t ep_meta = cuex_meta(ep);
		if (cuex_og_hole_contains(ep_meta)) {
		    mu_rebuild_ex_frame_t fr;
		    int index = cuex_oa_hole_index(ep_meta);
		    /* TODO: escaping indices */
		    fr = cucon_stack_at(stack,
					sizeof(struct mu_rebuild_ex_frame_s)
					*index);
		    cu_debug_assert(fr->lev >= 0 && lev >= fr->lev);
		    ep = cuex_hole(lev - fr->lev);
		}
		e_arr[a] = ep;
	    }
	}
	rb->processing = cu_false;
	cucon_stack_free(stack, sizeof(struct mu_rebuild_ex_frame_s)*bind_cnt);
	e = cuex_opn_by_arr(meta, e_arr);
	if (rb->need_mu_bind)
	    e = cuex_o1_mu(e);
	return e;
    }
    else
	return s->e;
}

static cuex_t
mu_rebuild(state_t s, cucon_parr_t BC)
{
    int k = cucon_parr_size(BC);
    struct cucon_stack_s stack;
    struct rebuildinfo_s *rb_arr = cu_salloc(sizeof(struct rebuildinfo_s)*k);
    memset(rb_arr, 0, sizeof(struct rebuildinfo_s)*k);
    mu_rebuild_feedback(s, rb_arr);
    cucon_stack_cct(&stack);
    return mu_rebuild_ex(s, rb_arr, -1, &stack);
}

#if DEBUG_DUMP_PART
static void
mu_dump_part(state_t s, cucon_parr_t BC)
{
    int i, k, a;
    char *sep;
    k = cucon_parr_size(BC);
    fputs("----\n", stderr);
    for (i = 0; i < k; ++i) {
	partinfo_t BCi = cucon_parr_at(BC, i);
	cucon_listnode_t it_Bi;
	fprintf(stderr, "PARTITION %d\n", i);
	for (it_Bi = cucon_list_begin(&BCi->Bi);
	     it_Bi != cucon_list_end(&BCi->Bi);
	     it_Bi = cucon_listnode_next(it_Bi)) {
	    state_t sp = cu_from(state, cucon_listnode, it_Bi);
	    cu_fprintf(stderr, "  %s %p: %!\n",
		       sp == s? "*" : " ", sp, sp->e);
	}
	fprintf(stderr, "    C = {");
	sep = "";
	for (a = 0; a < cucon_parr_size(&BCi->Ci); ++a) {
	    cucon_list_t Cia = cucon_parr_at(&BCi->Ci, a);
	    cucon_listnode_t it_Cia;
	    fprintf(stderr, "%s%d ↦ {", sep, a);
	    sep = "";
	    for (it_Cia = cucon_list_begin(Cia);
		 it_Cia != cucon_list_end(Cia);
		 it_Cia = cucon_listnode_next(it_Cia)) {
		state_t s = cucon_listnode_ptr(it_Cia);
		fprintf(stderr, "%s%p", sep, s);
		sep = ", ";
	    }
	    fputs("}", stderr);
	    sep = ", ";
	}
	fputs("}\n", stderr);
    }
}
#endif

cuex_t
cuex_mu_minimise(cuex_t e)
{
    struct cucon_pmap_s crox_to_partno;
    struct cucon_parr_s BC;
    struct cucon_stack_s mus;
    cu_rank_t a, a_max;
    struct cucon_uset_s *L;
    int k;
    state_t s;
    partinfo_t BCi;
    struct cucon_pmap_s *inv_delta; /* of cucon_list_s */
    size_t state_count;
    cucon_pmap_t fiimap;

    a_max = cuex_max_arity(e);
    if (a_max == 0) {
	if (cuex_meta(e) == CUEX_O1_MU) {
	    e = cuex_opn_at(e, 0);
	    cu_debug_assert(cuex_opr_r(cuex_meta(e)) == 0);
	}
	return e;
    }

    /* Build partitions, B and C. */
    cucon_pmap_cct(&crox_to_partno);
    cucon_parr_cct_empty(&BC);
    cucon_stack_cct(&mus);
    e = cuex_o1_ident(e);
    inv_delta = cu_salloc(a_max*sizeof(struct cucon_pmap_s));
    for (a = 0; a < a_max; ++a)
	cucon_pmap_cct(&inv_delta[a]);
    state_count = 0;
    fiimap = cuex_foldinert_indices(e, -1);
    s = mu_initial(e, &BC, &mus, 0, cuex_mupath_null(), fiimap,
		   &crox_to_partno, &state_count, &BCi, inv_delta);
    cu_debug_assert(cucon_stack_is_empty(&mus));

    /* Build work set, L */
    L = cu_salloc(sizeof(struct cucon_uset_s)*a_max);
    k = cucon_parr_size(&BC);
    for (a = 0; a < a_max; ++a) {
	int j;
	cucon_uset_cct(&L[a]);
	for (j = 0; j < k; ++j) {
	    partinfo_t BCj = cucon_parr_at(&BC, j);
	    if (a < cucon_parr_size(&BCj->Ci))
		cucon_uset_insert(&L[a], j);
	}
    }

#if DEBUG_DUMP_PART
    mu_dump_part(s, &BC);
#endif

    /* Finish partitions. */
    mu_refine_partition(a_max, state_count, L, &BC, inv_delta);

#if DEBUG_DUMP_PART
    mu_dump_part(s, &BC);
#endif

    e = mu_rebuild(s, &BC);
    cu_debug_assert(cuex_meta(e) == CUEX_O1_IDENT);
    e = cuex_opn_at(e, 0);
    return e;
}

/* References
 * [1] John Hopcroft, "An n log n algorithm for minimizing states in a finite
 *     automaton"; 1971; Stanford University, STAN-CS-71-190.
 */
