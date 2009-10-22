/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2003--2007  Petter Urkedal <urkedal@nbi.dk>
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

#define CUEX_SSFN_C
#include <cu/idr.h>
#include <cu/memory.h>
#include <cu/debug.h>
#include <cuex/ssfn.h>
#include <cuex/ex.h>
#include <cuex/pvar.h>
#include <cuex/algo.h>
#include <cuex/opntpl.h>
#include <cuex/subst.h>
#include <cucon/priq.h>
#include <cucon/slink.h>
#include <cucon/rbtree.h>
#include <cucon/list.h>
#include <cuoo/properties.h>
#include <string.h>
#include <assert.h>

cu_dlog_def(_file, "dtag=cuex.ssfn");

#define SSFN_TUNING_AVOID_CROSS_SUBST_IN_MGU 1


/* Helpers
 * ------- */

#define CHIB_SCTR_SPECIFIC -1
#define CHIB_SCTR_GENERIC -2

void
cuexP_sctr_skip(int **x)
{
    int n = *--*x;
    if (n > 0) {
	while (n--)
	    cuexP_sctr_skip(x);
    }
}

/* If the paths in 'cuexP_ssfn_find' which produced 'lhs' and 'rhs' can
 * match the same expression, return true iff 'lhs' is at least as
 * specific as 'rhs'. */
cu_bool_t
cuexP_sctr_lgr_h(int **lhs, int **rhs)
{
    int n_lhs = *--*lhs;
    int n_rhs = *--*rhs;
#if 0
    printf("(%d,%d) ", n_lhs, n_rhs);
#endif
    switch (n_lhs) {
    case CHIB_SCTR_GENERIC:
	return n_rhs == CHIB_SCTR_GENERIC;
    case CHIB_SCTR_SPECIFIC:
	return cu_true;
    default:
	if (n_rhs == CHIB_SCTR_GENERIC) {
	    ++*lhs;
	    cuexP_sctr_skip(lhs);
	}
	else
	    while (n_lhs > 0) {
		if (!cuexP_sctr_lgr_h(lhs, rhs))
		    return cu_false;
		--n_lhs;
	    }
	return cu_true;
    }
}
cu_bool_t
cuexP_sctr_lgr(int *lhs, int *rhs)
{
#if 0 /* verbose dbg */
    printf("\t");
    cu_bool_t res = cuexP_sctr_lgr_h(&lhs, &rhs);
    printf("-> %d\n", res);
    return res;
#else
    assert(lhs != NULL && rhs != NULL);
    return cuexP_sctr_lgr_h(&lhs, &rhs);
#endif
}


/*  --------------------------------------------------------------------
		    Structural Semantic Functions
    --------------------------------------------------------------------  */

#define ssfn_unify_cntn(node, i) \
	(((cuex_ssfn_node_t *)CU_ALIGNED_MARG_END(cuex_ssfn_node_t, node))[i])


/* Constructors
 * ------------ */

static void
cuex_ssfn_node_cct(cuex_ssfn_node_t node, cuex_ssfn_seqno_t seqno)
{
    cucon_pmap_init(&node->match_exaddr);
    cucon_umap_init(&node->match_opr);
    node->var_cntn = NULL;
    node->var_qcode = cuex_qcode_na;
    node->seqno = seqno;
}

void
cuex_ssfn_cct(cuex_ssfn_t ssfn)
{
    cuex_ssfn_node_cct(cu_upcast(cuex_ssfn_node_s, ssfn), 0);
}

cuex_ssfn_t
cuex_ssfn_new(void)
{
    cuex_ssfn_t ssfn = cu_galloc(sizeof(struct cuex_ssfn_s));
    cuex_ssfn_cct(ssfn);
    return ssfn;
}

void
cuex_ssfn_dct(cuex_ssfn_t ssfn)
{
    memset(ssfn, 0, sizeof(struct cuex_ssfn_s));
}

void
cuex_ssfn_node_cct_sub(cuex_ssfn_node_t node, cu_count_t arg_cnt,
		      cuex_ssfn_seqno_t seqno)
{
    cuex_ssfn_node_cct(node, seqno);
    while (arg_cnt > 0)
	ssfn_unify_cntn(node, --arg_cnt) = NULL;
}

cuex_ssfn_node_t
cuex_ssfn_node_new_sub(cu_count_t arg_cnt, cuex_ssfn_seqno_t seqno)
{
    cuex_ssfn_node_t node
	= cu_galloc(CU_ALIGNED_SIZEOF(struct cuex_ssfn_node_s)
		     + sizeof(cuex_ssfn_node_t)*arg_cnt);
    cuex_ssfn_node_cct(node, seqno);
    while (arg_cnt > 0)
	ssfn_unify_cntn(node, --arg_cnt) = NULL;
    return node;
}

/* Copy and Clone
 * -------------- */

typedef struct _ssfn_init_copy_jargs_s
{
    size_t slot_size;
    cu_count_t var_cnt;
    cu_count_t pending_cnt;
    cu_clop(value_cct_copy, void, void *, void *);
} *_ssfn_init_copy_jargs_t;

static void _ssfn_init_copy(cuex_ssfn_node_t dst, cuex_ssfn_node_t src,
			    _ssfn_init_copy_jargs_t jargs);

cu_clos_def(_ssfn_init_copy_idr_or_exaddr,
	    cu_prot(void, void *dst_slot, void *src_slot, uintptr_t key),
	    (_ssfn_init_copy_jargs_t jargs;))
{
    cu_clos_self(_ssfn_init_copy_idr_or_exaddr);
    _ssfn_init_copy(dst_slot, src_slot, self->jargs);
}
cu_clos_def(_ssfn_init_copy_opr,
	    cu_prot(cucon_umap_node_t, void *src_slot, uintptr_t key),
	    (_ssfn_init_copy_jargs_t jargs;))
{
    cu_clos_self(_ssfn_init_copy_opr);
    cucon_umap_node_t node;
    _ssfn_init_copy_jargs_t jargs = self->jargs;
    cu_count_t pending_cnt_save = jargs->pending_cnt;
    jargs->pending_cnt += cuex_opr_r(key);
    if (jargs->pending_cnt == 0) {
	node = cucon_umap_node_alloc(jargs->slot_size);
	cu_call(jargs->value_cct_copy,
		cucon_umap_node_get_mem(node), src_slot);
    }
    else {
	cu_count_t var_cnt = jargs->var_cnt;
	node = cucon_umap_node_alloc(CU_ALIGNED_SIZEOF(struct cuex_ssfn_node_s)
				     + sizeof(cuex_ssfn_node_t)*var_cnt);
	_ssfn_init_copy(cucon_umap_node_get_mem(node), src_slot, jargs);
    }
    jargs->pending_cnt = pending_cnt_save;
    return node;
}

static void
_ssfn_init_copy(cuex_ssfn_node_t dst, cuex_ssfn_node_t src,
		_ssfn_init_copy_jargs_t jargs)
{
    if (jargs->pending_cnt == 0)
	cu_call(jargs->value_cct_copy, (void*)dst, (void*)src);
    else {
	cu_count_t var_cnt = jargs->var_cnt;
	cu_count_t i;
	size_t sub_size;
	size_t sub_size_var_cntn;
	_ssfn_init_copy_idr_or_exaddr_t idr_or_exaddr_cb;
	_ssfn_init_copy_opr_t opr_cb;

	--jargs->pending_cnt;
	if (jargs->pending_cnt > 0) {
	    sub_size
		= CU_ALIGNED_SIZEOF(struct cuex_ssfn_node_s)
		+ sizeof(cuex_ssfn_node_t)*var_cnt;
	    sub_size_var_cntn = sub_size + sizeof(cuex_ssfn_node_t);
	}
	else {
	    sub_size = jargs->slot_size;
	    sub_size_var_cntn = sub_size;
	}

	idr_or_exaddr_cb.jargs = jargs;
	cucon_pmap_init_copy_mem_ctor(&dst->match_exaddr, &src->match_exaddr,
		 sub_size,
		 _ssfn_init_copy_idr_or_exaddr_prep(&idr_or_exaddr_cb));

	opr_cb.jargs = jargs;
	cucon_umap_init_copy_node(&dst->match_opr, &src->match_opr,
				  _ssfn_init_copy_opr_prep(&opr_cb));

	dst->var_qcode = src->var_qcode;

	if (src->var_cntn) {
	    dst->var_cntn = cu_galloc(sub_size_var_cntn);
	    ++jargs->var_cnt;
	    _ssfn_init_copy(dst->var_cntn, src->var_cntn, jargs);
	    --jargs->var_cnt;
	}

	for (i = 0; i < var_cnt; ++i) {
	    cuex_ssfn_node_t src_cntn = ssfn_unify_cntn(src, i);
	    if (src_cntn) {
		cuex_ssfn_node_t dst_cntn = cu_galloc(sub_size);
		ssfn_unify_cntn(dst, i) = dst_cntn;
		_ssfn_init_copy(dst_cntn, src_cntn, jargs);
	    }
	    else
		ssfn_unify_cntn(dst, i) = NULL;
	}

	++jargs->pending_cnt;

	dst->seqno = src->seqno;
    }
}

void
cuex_ssfn_cct_copy_mem(cuex_ssfn_t dst, cuex_ssfn_t src,
		       size_t slot_size,
		       cu_clop(value_cct_copy, void, void *, void *))
{
    struct _ssfn_init_copy_jargs_s jargs;
    jargs.slot_size = slot_size;
    jargs.var_cnt = 0;
    jargs.pending_cnt = 1;
    jargs.value_cct_copy = value_cct_copy;
    _ssfn_init_copy(cu_upcast(cuex_ssfn_node_s, dst),
		    cu_upcast(cuex_ssfn_node_s, src), &jargs);
}

cuex_ssfn_t
cuex_ssfn_new_copy_mem(cuex_ssfn_t src,
		       size_t slot_size,
		       cu_clop(value_cct_copy, void, void *, void *))
{
    cuex_ssfn_t dst = cu_gnew(struct cuex_ssfn_s);
    cuex_ssfn_cct_copy_mem(dst, src, slot_size, value_cct_copy);
    return dst;
}


/* cuex_ssfn_find
 * -------------- */

static void
cuexP_ssfn_find(cuex_ssfn_node_t node, cuexP_ssfn_find_jargs_t jargs)
{
    cu_count_t i;
    cuex_t ex;
    cuex_meta_t ex_meta;
    if (cucon_stack_is_empty(&jargs->input)) {
	int *sctr_this;
	cucon_stack_it_t it;
	cu_count_t n;
	cu_count_t arg_cnt = cucon_stack_size(&jargs->output) / sizeof(cuex_t);
	cuex_t *arg_arr = cu_galloc(sizeof(cuex_var_t)*arg_cnt);
	it = cucon_stack_begin(&jargs->output);
	n = arg_cnt;
	while (n > 0) {
	    assert(!cucon_stack_it_is_end(it));
	    arg_arr[--n] = CUCON_STACK_IT_GET(it, cuex_t);
	    CUCON_STACK_IT_ADVANCE(&it, cuex_t);
	}
	n = cucon_stack_size(&jargs->sctr);
	sctr_this = cucon_stack_continuous_top(&jargs->sctr, n);
	n /= sizeof(int);
	if (jargs->sctr_most_specific) {
	    if (!cuexP_sctr_lgr(jargs->sctr_most_specific, sctr_this + n))
		cu_call(jargs->receiver, arg_cnt, arg_arr, (void *)node);
	}
	else {
	    if (cu_call(jargs->receiver, arg_cnt, arg_arr, (void *)node)) {
		/* Extra 1 byte to protect from GC. */
		memcpy(jargs->sctr_most_specific = cu_galloc(sizeof(int)*n+1),
		       sctr_this, sizeof(int)*n);
		jargs->sctr_most_specific += n;
	    }
	}
	return;
    }

    ex = CUCON_STACK_TOP(&jargs->input, cuex_t);
    ex_meta = cuex_meta(ex);
    CUCON_STACK_POP(&jargs->input, cuex_t);
    if (cuex_is_varmeta_e(ex_meta) || cuex_meta_is_type(ex_meta)) {
	cuex_ssfn_node_t subnode;
	subnode = cucon_pmap_find_mem(&node->match_exaddr, ex);
	if (subnode) {
	    CUCON_STACK_PUSH(&jargs->sctr, int, CHIB_SCTR_SPECIFIC);
	    cuexP_ssfn_find(subnode, jargs);
	    CUCON_STACK_POP(&jargs->sctr, int);
	}
    }
    else if (cuex_is_opn(ex)) {
	cu_rank_t n;
	cuex_ssfn_node_t subnode
	    = cucon_umap_find_mem(&node->match_opr, cuex_meta(ex));
	if (subnode) {
	    n = cuex_opn_r(cuex_opn_from_ex(ex));
	    CUCON_STACK_PUSH(&jargs->sctr, int, n);
	    while (n > 0)
		CUCON_STACK_PUSH(&jargs->input, cuex_t,
				 cuex_opn_at(cuex_opn_from_ex(ex), --n));
	    cuexP_ssfn_find(subnode, jargs);
	    CUCON_STACK_POP(&jargs->sctr, int);
	    cucon_stack_free(&jargs->input,
			     cuex_opr_r(ex_meta)*sizeof(cuex_t));
	}
    }
    for (i = 0; i < jargs->arg_cnt; ++i)
	if (ssfn_unify_cntn(node, i)) {
	    cuex_t *arg_pos
		= cucon_stack_at(&jargs->output, i*sizeof(cuex_t));
	    cuex_t ex0 = *arg_pos;
	    cuex_t ex1 = cuex_unify(ex0, ex);
	    if (!cuex_is_null(ex1)) {
		*arg_pos = ex1;
		CUCON_STACK_PUSH(&jargs->sctr, int, CHIB_SCTR_SPECIFIC);
		cuexP_ssfn_find(ssfn_unify_cntn(node, i), jargs);
		CUCON_STACK_POP(&jargs->sctr, int);
		*arg_pos = ex0;
	    }
	}
    if (node->var_cntn) {
	CUCON_STACK_PUSH(&jargs->output, cuex_t, ex);
	++jargs->arg_cnt;
	CUCON_STACK_PUSH(&jargs->sctr, int, CHIB_SCTR_GENERIC);
	cuexP_ssfn_find(node->var_cntn, jargs);
	CUCON_STACK_POP(&jargs->sctr, int);
	--jargs->arg_cnt;
	CUCON_STACK_POP(&jargs->output, cuex_t);
    }
    CUCON_STACK_PUSH(&jargs->input, cuex_t, ex);
}

void
cuex_ssfn_find(cuex_ssfn_t ssfn, cuex_t key,
	       cu_clop(out, cu_bool_t, cu_count_t, cuex_t *, void *))
{
    struct cuexP_ssfn_find_jargs_s jargs;
    cucon_stack_init(&jargs.input);
    cucon_stack_init(&jargs.output);
    cucon_stack_init(&jargs.sctr);
    jargs.sctr_most_specific = NULL;
    jargs.receiver = out;
    jargs.arg_cnt = 0;
    CUCON_STACK_PUSH(&jargs.input, cuex_t, key);
    cuexP_ssfn_find(cu_upcast(cuex_ssfn_node_s, ssfn), &jargs);
}



/* cuex_ssfn_find_it_t
 * ------------------ */

typedef enum {
    cuexP_ssfn_find_it_state_enter,
    cuexP_ssfn_find_it_state_matched,
    cuexP_ssfn_find_it_state_exaddr,
    cuexP_ssfn_find_it_state_idr,
    cuexP_ssfn_find_it_state_opn,
    cuexP_ssfn_find_it_state_obj,
    cuexP_ssfn_find_it_state_unify_var,
    cuexP_ssfn_find_it_state_next_var,
} cuexP_ssfn_find_it_state_t;

typedef struct cuexP_ssfn_find_it_frame_s *cuexP_ssfn_find_it_frame_t;
struct cuexP_ssfn_find_it_frame_s
{
    cuex_ssfn_node_t node;
    cuexP_ssfn_find_it_state_t state;
    cuex_t ex, ex0;
    int i;
};

void
cuex_ssfn_find_it_cct(cuex_ssfn_find_it_t find_it, cuex_ssfn_t ssfn, cuex_t key)
{
    cuexP_ssfn_find_it_frame_t frame;
    cucon_stack_init(&find_it->stack);
    frame = cucon_stack_alloc(&find_it->stack,
			    sizeof(struct cuexP_ssfn_find_it_frame_s));
    frame->node = cu_upcast(cuex_ssfn_node_s, ssfn);
    cucon_stack_init(&find_it->jargs.input);
    cucon_stack_init(&find_it->jargs.output);
    cucon_stack_init(&find_it->jargs.sctr);
    find_it->jargs.sctr_most_specific = NULL;
    find_it->jargs.arg_cnt = 0;
    CUCON_STACK_PUSH(&find_it->jargs.input, cuex_t, key);
    frame->state = cuexP_ssfn_find_it_state_enter;
    cuex_ssfn_find_it_advance(find_it);
}

cuex_ssfn_find_it_t
cuex_ssfn_find_it_new(cuex_ssfn_t ssfn, cuex_t key)
{
    cuex_ssfn_find_it_t find_it = cu_gnew(struct cuex_ssfn_find_it_s);
    cuex_ssfn_find_it_cct(find_it, ssfn, key);
    return find_it;
}

void
cuex_ssfn_find_it_advance(cuex_ssfn_find_it_t find_it)
{
    cuexP_ssfn_find_it_frame_t frame;
    int i;
    cuex_t ex;
    cuex_meta_t ex_meta;
    cuex_ssfn_node_t node;
    cuexP_ssfn_find_jargs_t jargs = &find_it->jargs;
returned:
    if (cucon_stack_is_empty(&find_it->stack)) {
	find_it->slot = NULL;
	return;
    }
    frame = cucon_stack_top(&find_it->stack);
    ex = frame->ex;
    node = frame->node;
    switch (frame->state) {
    case cuexP_ssfn_find_it_state_enter:
	break;
    case cuexP_ssfn_find_it_state_matched:
	goto return_after_match;
    case cuexP_ssfn_find_it_state_exaddr:
	goto return_in_exaddr;
    case cuexP_ssfn_find_it_state_opn:
	goto return_in_opn;
    case cuexP_ssfn_find_it_state_unify_var:
	goto return_in_unify_var;
    case cuexP_ssfn_find_it_state_next_var:
	goto return_in_next_var;
    default:
	assert(!"not reached");
    }

enter:
    node = frame->node;
    if (cucon_stack_is_empty(&jargs->input)) {
	cucon_stack_it_t it;
	cu_count_t n;
	cu_count_t arg_cnt = cucon_stack_size(&jargs->output) / sizeof(cuex_t);
	cuex_t *arg_arr = cu_galloc(sizeof(cuex_var_t)*arg_cnt);
	it = cucon_stack_begin(&jargs->output);
	n = arg_cnt;
	while (n > 0) {
	    assert(!cucon_stack_it_is_end(it));
	    arg_arr[--n] = CUCON_STACK_IT_GET(it, cuex_t);
	    CUCON_STACK_IT_ADVANCE(&it, cuex_t);
	}
	n = cucon_stack_size(&jargs->sctr);
	find_it->sctr_this = cucon_stack_continuous_top(&jargs->sctr, n);
	n /= sizeof(int);
	if (jargs->sctr_most_specific) {
	    if (!cuexP_sctr_lgr(jargs->sctr_most_specific,
				 find_it->sctr_this + n)) {
		frame->state = cuexP_ssfn_find_it_state_matched;
		find_it->sctr_this = NULL;
		find_it->arg_cnt = arg_cnt;
		find_it->arg_arr = arg_arr;
		find_it->slot = (void*)node;
		return;
	    }
	}
	else {
	    frame->state = cuexP_ssfn_find_it_state_matched;
	    find_it->arg_cnt = arg_cnt;
	    find_it->arg_arr = arg_arr;
	    find_it->slot = (void*)node;
	    return;
	}
    return_after_match:
	cucon_stack_free(&find_it->stack,
		       sizeof(struct cuexP_ssfn_find_it_frame_s));
	goto returned;
    }

    ex = CUCON_STACK_TOP(&jargs->input, cuex_t);
    ex_meta = cuex_meta(ex);
    frame->ex = ex;
    CUCON_STACK_POP(&jargs->input, cuex_t);
    if (cuex_is_varmeta_e(ex_meta) || cuex_meta_is_type(ex_meta)) {
	cuex_ssfn_node_t subnode;
	subnode = cucon_pmap_find_mem(&node->match_exaddr, ex);
	if (subnode) {
	    CUCON_STACK_PUSH(&jargs->sctr, int, CHIB_SCTR_SPECIFIC);
	    frame->state = cuexP_ssfn_find_it_state_exaddr;
	    frame = cucon_stack_alloc(&find_it->stack,
				      sizeof(struct cuexP_ssfn_find_it_frame_s));
	    frame->node = subnode;
	    goto enter;
	return_in_exaddr:
	    CUCON_STACK_POP(&jargs->sctr, int);
	}
    }
    else if (cuex_is_opn(ex)) {
	cu_rank_t n;
	cuex_ssfn_node_t subnode;
	subnode = cucon_umap_find_mem(&node->match_opr, cuex_meta(ex));
	if (subnode) {
	    n = cuex_opn_r(cuex_opn_from_ex(ex));
	    CUCON_STACK_PUSH(&jargs->sctr, int, n);
	    while (n > 0)
		CUCON_STACK_PUSH(&jargs->input, cuex_t,
				 cuex_opn_at(cuex_opn_from_ex(ex), --n));
	    frame->state = cuexP_ssfn_find_it_state_opn;
	    frame = cucon_stack_alloc(&find_it->stack,
				      sizeof(struct cuexP_ssfn_find_it_frame_s));
	    frame->node = subnode;
	    goto enter;
	return_in_opn:
	    CUCON_STACK_POP(&jargs->sctr, int);
	    cucon_stack_free(&jargs->input,
			     cuex_opn_r(cuex_opn_from_ex(ex))*sizeof(cuex_t));
	}
    }
    for (i = 0; i < jargs->arg_cnt; ++i)
	if (ssfn_unify_cntn(node, i)) {
	    cuex_t *arg_pos = cucon_stack_at(&jargs->output, i*sizeof(cuex_t));
	    cuex_t ex0 = *arg_pos;
	    cuex_t ex1 = cuex_unify(ex0, ex);
	    if (!cuex_is_null(ex1)) {
		*arg_pos = ex1;
		CUCON_STACK_PUSH(&jargs->sctr, int, CHIB_SCTR_SPECIFIC);
		frame->state = cuexP_ssfn_find_it_state_unify_var;
		frame = cucon_stack_alloc(
			    &find_it->stack,
			    sizeof(struct cuexP_ssfn_find_it_frame_s));
		frame->node = ssfn_unify_cntn(node, i);
		frame->i = i;
		frame->ex0 = ex0;
		goto enter;
	    return_in_unify_var:
		i = frame->i;
		ex0 = frame->ex0;
		arg_pos = cucon_stack_at(&jargs->output, i*sizeof(cuex_t));
		CUCON_STACK_POP(&jargs->sctr, int);
		*arg_pos = ex0;
	    }
	}
    if (node->var_cntn) {
	CUCON_STACK_PUSH(&jargs->output, cuex_t, ex);
	++jargs->arg_cnt;
	CUCON_STACK_PUSH(&jargs->sctr, int, CHIB_SCTR_GENERIC);
	frame->state = cuexP_ssfn_find_it_state_next_var;
	frame = cucon_stack_alloc(&find_it->stack,
				sizeof(struct cuexP_ssfn_find_it_frame_s));
	frame->node = node->var_cntn;
	goto enter;
    return_in_next_var:
	CUCON_STACK_POP(&jargs->sctr, int);
	--jargs->arg_cnt;
	CUCON_STACK_POP(&jargs->output, cuex_t);
    }
    CUCON_STACK_PUSH(&jargs->input, cuex_t, ex);
    cucon_stack_free(&find_it->stack,
		     sizeof(struct cuexP_ssfn_find_it_frame_s));
    goto returned;
}

void
cuex_ssfn_find_it_report_match(cuex_ssfn_find_it_t find_it)
{
    cu_count_t n = cucon_stack_size(&find_it->jargs.sctr) / sizeof(int);
    if (find_it->sctr_this) {
	/* Extra 1 byte to protect from GC. */
	memcpy(find_it->jargs.sctr_most_specific = cu_galloc(sizeof(int)*n+1),
	       find_it->sctr_this, sizeof(int)*n);
	find_it->jargs.sctr_most_specific += n;
    }
}

cu_bool_t
cuex_ssfn_find_it_match_lgr(cuex_ssfn_find_it_t it0, cuex_ssfn_find_it_t it1)
{
    assert(it0 != NULL && it1 != NULL);
    return cuexP_sctr_lgr(it0->jargs.sctr_most_specific,
			 it1->jargs.sctr_most_specific);
}
cu_clop_def(cuex_ssfn_find_it_match_lgr_clop, cu_bool_t,
	    cuex_ssfn_find_it_t it0, cuex_ssfn_find_it_t it1)
{
    assert(it0 != NULL && it1 != NULL);
    return cuexP_sctr_lgr(it0->jargs.sctr_most_specific,
			 it1->jargs.sctr_most_specific);
}


/* cuex_ssfn_intersection_find_aux
 * ------------------------------ */

void
cuex_ssfn_intersection_find_aux(
	cuex_ssfn_t ssfn_first, cucon_list_t ssfn_list, cuex_t key,
	cu_clop(out, cu_bool_t, cu_count_t, cuex_t *, void *))
{
    cuex_ssfn_find_it_t it_match;
    cuex_ssfn_find_it_t it_spec CU_NOINIT(NULL);
    cucon_listnode_t it_l;
    struct cucon_priq_s q_it;

    cucon_priq_init(&q_it, (cu_clop(, cu_bool_t, void *, void *))
				cuex_ssfn_find_it_match_lgr_clop);
    it_match = cuex_ssfn_find_it_new(ssfn_first, key);
    if (!cuex_ssfn_find_it_is_end(it_match))
	cucon_priq_insert(&q_it, it_match);
    for (it_l = cucon_list_begin(ssfn_list);
	 it_l != cucon_list_end(ssfn_list);
	 it_l = cucon_listnode_next(it_l)) {
	cuex_ssfn_t ssfn = cucon_listnode_ptr(it_l);
	it_match = cuex_ssfn_find_it_new(ssfn, key);
	if (!cuex_ssfn_find_it_is_end(it_match))
	    cucon_priq_insert(&q_it, it_match);
    }
    while (!cucon_priq_is_empty(&q_it)) {
	it_match = cucon_priq_pop_front(&q_it);
	if (cu_call(out,
		     cuex_ssfn_find_it_arg_cnt(it_match),
		     cuex_ssfn_find_it_arg_arr(it_match),
		     cuex_ssfn_find_it_slot(it_match))) {
	    it_spec = it_match;
	    cuex_ssfn_find_it_advance(it_match);
	    if (!cuex_ssfn_find_it_is_end(it_match))
		cucon_priq_insert(&q_it, it_match);
	    break;
	}
	cuex_ssfn_find_it_advance(it_match);
	if (!cuex_ssfn_find_it_is_end(it_match))
	    cucon_priq_insert(&q_it, it_match);
    }
    while (!cucon_priq_is_empty(&q_it)) {
	it_match = cucon_priq_pop_front(&q_it);
	if (cuex_ssfn_find_it_match_lgr(it_spec, it_match))
	    break;
	do {
	    cuex_ssfn_find_it_advance(it_match);
	    if (cuex_ssfn_find_it_is_end(it_match))
		break;
	    cu_call(out,
		     cuex_ssfn_find_it_arg_cnt(it_match),
		     cuex_ssfn_find_it_arg_arr(it_match),
		     cuex_ssfn_find_it_slot(it_match));
	} while (!cuex_ssfn_find_it_match_lgr(it_spec, it_match));
    }
}

/* cuex_ssfn_insert_mem
 * -------------------- */

typedef struct cuexP_ssfn_insert_jargs_s {
    struct cucon_stack_s input;
    size_t slot_size;
    void **slot;
    cu_count_t var_cnt;
    struct cucon_pmap_s var_to_index;	/* cu_var_t → cu_count_t */
    cuex_ssfn_seqno_t seqno;
}				     *cuexP_ssfn_insert_jargs_t;
static cu_bool_t
cuexP_ssfn_insert_mem(cuex_ssfn_node_t node, cuexP_ssfn_insert_jargs_t jargs)
{
    cuex_t ex;
    cuex_meta_t ex_meta;
tail_rec:
    node->seqno = jargs->seqno;
    assert(!cucon_stack_is_empty(&jargs->input));
    ex = CUCON_STACK_TOP(&jargs->input, cuex_t);
    ex_meta = cuex_meta(ex);
    /* deref scope rec */
    CUCON_STACK_POP(&jargs->input, cuex_t);
    switch (cuex_meta_kind(ex_meta)) {
	cu_rank_t n;

    case cuex_meta_kind_opr:
	n = cuex_opn_r(cuex_opn_from_ex(ex));
	while (n > 0)
	    CUCON_STACK_PUSH(&jargs->input, cuex_t,
			     cuex_opn_at(cuex_opn_from_ex(ex), --n));
	if (cucon_stack_is_empty(&jargs->input))
	    return cucon_umap_insert_mem(
		&node->match_opr, cuex_meta(ex),
		jargs->slot_size, jargs->slot);
	else {
	    cuex_ssfn_node_t subnode;
	    if (cucon_umap_insert_mem(
		    &node->match_opr, cuex_meta(ex),
		    CU_ALIGNED_SIZEOF(struct cuex_ssfn_node_s)
		    + sizeof(cuex_ssfn_node_t)*jargs->var_cnt,
		    (void *)&subnode))
		cuex_ssfn_node_cct_sub(subnode, jargs->var_cnt, jargs->seqno);
	    node = subnode;
	    goto tail_rec;
	}
	break;

    case cuex_meta_kind_type:
	if (cucon_stack_is_empty(&jargs->input))
	    return cucon_pmap_insert_mem(&node->match_exaddr, ex,
				       jargs->slot_size, jargs->slot);
	else {
	    cuex_ssfn_node_t subnode;
	    if (cucon_pmap_insert_mem(
		    &node->match_exaddr, ex,
		    CU_ALIGNED_SIZEOF(struct cuex_ssfn_node_s)
		    + sizeof(cuex_ssfn_node_t)*jargs->var_cnt,
		    (void *)&subnode))
		cuex_ssfn_node_cct_sub(subnode, jargs->var_cnt,
				      jargs->seqno);
	    node = subnode;
	    goto tail_rec;
	}
	break;

    default:
	if (cuex_is_varmeta(ex_meta)) {
	    cuex_qcode_t qcode = cuex_varmeta_qcode(ex_meta);
	    switch (qcode) {
		cu_count_t *index;
		cuex_ssfn_node_t *link;
	    case cuex_qcode_active_s:
	    case cuex_qcode_active_w:
		if (cucon_pmap_insert_mem(&jargs->var_to_index, ex,
			sizeof(cu_count_t), (void *)&index)) {
		    *index = jargs->var_cnt++;
		    link = &node->var_cntn;
		    node->var_qcode = qcode;
		}
		else
		    link = &ssfn_unify_cntn(node, (jargs->var_cnt - *index - 1));

		if (cucon_stack_is_empty(&jargs->input)) {
		    if (!*link) {
			*jargs->slot = *link = cu_galloc(jargs->slot_size);
			return cu_true;
		    }
		    else {
			*jargs->slot = *link;
			return cu_false;
		    }
		}
		else {
		    if (!*link)
			*link = cuex_ssfn_node_new_sub(jargs->var_cnt,
						      jargs->seqno);
		    node = *link;
		    goto tail_rec;
		}
		break;

	    case cuex_qcode_passive:
		if (cucon_stack_is_empty(&jargs->input))
		    return cucon_pmap_insert_mem(&node->match_exaddr, ex,
					       jargs->slot_size, jargs->slot);
		else {
		    cuex_ssfn_node_t subnode;
		    if (cucon_pmap_insert_mem(
			    &node->match_exaddr, ex,
			    CU_ALIGNED_SIZEOF(struct cuex_ssfn_node_s)
			    + sizeof(cuex_ssfn_node_t)*jargs->var_cnt,
			    (void *)&subnode))
			cuex_ssfn_node_cct_sub(subnode, jargs->var_cnt,
					      jargs->seqno);
		    node = subnode;
		    goto tail_rec;
		}

	    default:
		cu_errf_at(cuoo_sref(ex),
			   "Badly quantified variable %! for pattern.", ex);
		cu_debug_unreachable();
		/* XXX at least until there are useful cases which can
		 * provide hints of the correct semantics */
		return cu_false;
	    }
	}
	else {
	    cu_errf_at(cuoo_sref(ex), "‘%!’ is invaild in pattern.", ex);
	    return cu_false;
	}
    }
}

cu_bool_t
cuex_ssfn_insert_mem(cuex_ssfn_t ssfn, cuex_t patn,
		    cuex_ssfn_seqno_t seqno,
		    size_t slot_size, cu_ptr_ptr_t slot_out,
		    cu_count_t *var_cnt_out, cuex_var_t **var_arr_out)
{
    cucon_pmap_it_t it;
    size_t n;
    struct cuexP_ssfn_insert_jargs_s jargs;
    cu_bool_t st;
    cu_debug_assert(ssfn && patn);
    cucon_stack_init(&jargs.input);
    jargs.slot_size = slot_size;
    jargs.slot = slot_out;
    jargs.var_cnt = 0;
    cucon_pmap_init(&jargs.var_to_index);
    CUCON_STACK_PUSH(&jargs.input, cuex_t, patn);
    st = cuexP_ssfn_insert_mem(cu_upcast(cuex_ssfn_node_s, ssfn), &jargs);
    n = cucon_pmap_size(&jargs.var_to_index);
    if (var_cnt_out)
	*var_cnt_out = n;
    if (var_arr_out) {
	*var_arr_out = cu_galloc(sizeof(cuex_var_t)*n);
	for (it = cucon_pmap_begin(&jargs.var_to_index);
	     !cucon_pmap_end_eq(&jargs.var_to_index, it);
	     it = cucon_pmap_it_next(it))
	    (*var_arr_out)[*(cu_count_t*)cucon_pmap_it_value_mem(it)]
		= cucon_pmap_it_key(it);
    }
    return st;
}



/* cuex_ssfn_find_lgr
 * ----------------- */

typedef struct cuexP_ssfn_find_mgu_jargs_s
{
    struct cucon_stack_s targets;
    struct cucon_stack_s input;
    struct cucon_stack_s output;
    struct cucon_stack_s sctr;
    cuex_ssfn_find_cb_t cb;
    cuex_var_t key_var;
    cuex_t key_arg;
    cuex_subst_t subst;
    int *sctr_most_specific;
    cu_bool_t do_lgr;
    cu_bool_t do_lgr_w;
    cu_bool_t do_mgr;
    cu_bool_t do_mgr_w;
    struct cuex_ssfn_matchinfo_s minfo;
    cuex_ssfn_ctrl_t ctrl;
    cuex_ssfn_seqno_t min_seqno;
} *cuexP_ssfn_find_mgu_jargs_t;

static void
cuexP_ssfn_node_check_deletable(cuex_ssfn_node_t node,
				 cuexP_ssfn_find_mgu_jargs_t jargs)
{
    if (node->var_cntn != NULL
	|| !cucon_pmap_is_empty(&node->match_exaddr)
	|| !cucon_umap_is_empty(&node->match_opr)) {
	jargs->ctrl = cuex_ssfn_ctrl_continue;
	return;
    }
    else {
	cu_rank_t N = CUCON_STACK_SIZE(&jargs->output, cuex_t);
	cu_rank_t n;
	for (n = 0; n < N; ++n)
	    if (ssfn_unify_cntn(node, n)) {
		jargs->ctrl = cuex_ssfn_ctrl_continue;
		return;
	    }
    }
    jargs->ctrl = cuex_ssfn_ctrl_cont_delete;
    return;
}

static cu_bool_t
cuexP_ssfn_find_mgu_expand(cuex_ssfn_node_t node, cuex_t *target,
			  cuexP_ssfn_find_mgu_jargs_t jargs);
static cu_bool_t
cuexP_ssfn_find_mgu_maybe_expand(cuex_ssfn_node_t node,
				cuexP_ssfn_find_mgu_jargs_t jargs);
static cu_bool_t
cuexP_ssfn_find_mgu(cuex_ssfn_node_t node, cuexP_ssfn_find_mgu_jargs_t jargs);

cu_clos_def(_ssfn_find_mgu_exaddr,
	    cu_prot(cu_bool_t, void const *key, void *node),
	    ( cuex_t *target;
	      cuexP_ssfn_find_mgu_jargs_t jargs;
	      cucon_slink_t deletable; ))
{
#define key ((cuex_t)key)
#define node ((cuex_ssfn_node_t)node)
    cu_clos_self(_ssfn_find_mgu_exaddr);
    cu_bool_t do_cont;
    if (self->target)
	*self->target = key;
    do_cont = cuexP_ssfn_find_mgu_maybe_expand(node, self->jargs);
    if (cuex_ssfn_ctrl_do_delete(self->jargs->ctrl)) {
	self->deletable = cucon_slink_prepend_ptr(self->deletable, key);
	self->jargs->ctrl = cuex_ssfn_ctrl_continue;
    }
    return do_cont;
#undef key
#undef node
}
cu_clos_def(_ssfn_find_mgu_opr,
	    cu_prot(cu_bool_t, uintptr_t key, void *node),
	    ( cuex_t *target;
	      cuexP_ssfn_find_mgu_jargs_t jargs;
	      cucon_slink_t deletable; ))
{
#define node ((cuex_ssfn_node_t)node)
    cu_clos_self(_ssfn_find_mgu_opr);
    cu_bool_t do_cont;
    cu_rank_t n = cuex_opr_r(key);
    cu_rank_t i = n;
    cuex_t opntpl = cuex_opntpl_new(key);
    cuexP_ssfn_find_mgu_jargs_t jargs = self->jargs;

    while (i)
	CUCON_STACK_PUSH(&jargs->targets, cuex_t *,
			 cuex_opntpl_ref_at(opntpl, --i));
    if (self->target)
	*self->target = opntpl;
    do_cont = cuexP_ssfn_find_mgu_maybe_expand(node, jargs);

    if (cuex_ssfn_ctrl_do_delete(jargs->ctrl)) {
	uintptr_t *delkey;
	self->deletable
	    = cucon_slink_prepend_mem(self->deletable, sizeof(uintptr_t));
	delkey = cucon_slink_get_mem(self->deletable);
	*delkey = key;
	jargs->ctrl = cuex_ssfn_ctrl_continue;
    }
    if (!do_cont)
	return cu_false;
    cucon_stack_free(&jargs->targets, n*sizeof(cuex_t *));
    return do_cont;
#undef node
}

static cu_bool_t
cuexP_ssfn_find_mgu_maybe_expand(cuex_ssfn_node_t node,
				  cuexP_ssfn_find_mgu_jargs_t jargs)
{
    if (cucon_stack_is_empty(&jargs->targets)) {
	/* Then we're done building key_arg. */
	cuex_subst_t subst_save = jargs->subst;
	jargs->subst = cuex_subst_new_uw_clone(subst_save);
	if (cuex_subst_unify(jargs->subst, cuex_var_to_ex(jargs->key_var),
			     cuex_of_opntpl(jargs->key_arg))) {
	    cuex_var_t key_var_save = jargs->key_var;
	    cuex_t key_arg_save = jargs->key_arg;
	    cu_bool_t result = cuexP_ssfn_find_mgu(node, jargs);
	    jargs->key_var = key_var_save;
	    jargs->key_arg = key_arg_save;
	    jargs->subst = subst_save;
	    return result;
	}
	jargs->subst = subst_save;
    }
    else {
	cuex_t *target = CUCON_STACK_TOP(&jargs->targets, cuex_t *);
	CUCON_STACK_POP(&jargs->targets, cuex_t *);
	if (!cuexP_ssfn_find_mgu_expand(node, target, jargs))
	    return cu_false;
	CUCON_STACK_PUSH(&jargs->targets, cuex_t *, target);
    }
    return cu_true;
}

static cu_bool_t
cuexP_ssfn_find_mgu_expand(cuex_ssfn_node_t node, cuex_t *target,
			    cuexP_ssfn_find_mgu_jargs_t jargs)
{
    cu_bool_t have_deletion = cu_false;
    cu_bool_t do_continue = cu_true;
    {
	_ssfn_find_mgu_exaddr_t cb;
	cb.target = target;
	cb.jargs = jargs;
	cb.deletable = cucon_slink_empty();
	do_continue = cucon_pmap_conj_mem(&node->match_exaddr,
					  _ssfn_find_mgu_exaddr_prep(&cb));
	if (!cucon_slink_is_empty(cb.deletable)) {
	    have_deletion = cu_true;
	    do {
		cuex_t delkey = cucon_slink_get_ptr(cb.deletable);
		cucon_pmap_erase(&node->match_exaddr, delkey);
		cb.deletable = cucon_slink_next(cb.deletable);
	    } while (!cucon_slink_is_empty(cb.deletable));
	}
	if (!do_continue)
	    goto done;
    }
    {
	_ssfn_find_mgu_opr_t cb;
	cb.target = target;
	cb.jargs = jargs;
	cb.deletable = cucon_slink_empty();
	do_continue = cucon_umap_conj_mem(&node->match_opr,
					  _ssfn_find_mgu_opr_prep(&cb));
	if (!cucon_slink_is_empty(cb.deletable)) {
	    have_deletion = cu_true;
	    do {
		uintptr_t delkey
		    = *(uintptr_t*)cucon_slink_get_mem(cb.deletable);
		cucon_umap_erase(&node->match_opr, delkey);
		cb.deletable = cucon_slink_next(cb.deletable);
	    } while (!cucon_slink_is_empty(cb.deletable));
	}
	if (!do_continue)
	    goto done;
    }
    {
	cu_rank_t n = cucon_stack_size(&jargs->output)/sizeof(cuex_t);
	cu_rank_t i;
	for (i = 0; i < n; ++i)
	    if (ssfn_unify_cntn(node, i)) {
		cuex_t *arg_pos
		    = cucon_stack_at(&jargs->output, i*sizeof(cuex_t));
		if (target)
		    *target = *arg_pos;
		do_continue = cuexP_ssfn_find_mgu_maybe_expand(
				  ssfn_unify_cntn(node, i), jargs);
		if (cuex_ssfn_ctrl_do_delete(jargs->ctrl)) {
		    have_deletion = cu_true;
		    ssfn_unify_cntn(node, i) = NULL;
		    jargs->ctrl = cuex_ssfn_ctrl_continue;
		}
		if (!do_continue)
		    goto done;
	    }
    }
    if (node->var_cntn) {
	cuex_var_t var;
	cuex_subst_t subst_save;
#ifdef SSFN_TUNING_AVOID_CROSS_SUBST_IN_MGU
	if (jargs->do_mgr && node->var_qcode == cuex_qcode_active_s
	    && target == &jargs->key_arg)
	    goto done;
#endif
	var = cuex_var_new(node->var_qcode);
	subst_save = jargs->subst;
	if (!jargs->do_mgr) {
	    jargs->subst = cuex_subst_new_uw_clone(subst_save);
	    cuex_subst_block(jargs->subst, var);
	}
	if (target)
	    *target = cuex_var_to_ex(var);
	CUCON_STACK_PUSH(&jargs->output, cuex_t, cuex_var_to_ex(var));
	do_continue = cuexP_ssfn_find_mgu_maybe_expand(node->var_cntn, jargs);
	jargs->subst = subst_save;
	if (cuex_ssfn_ctrl_do_delete(jargs->ctrl)) {
	    have_deletion = cu_true;
	    node->var_cntn = NULL;
	    jargs->ctrl = cuex_ssfn_ctrl_continue;
	}
	if (!do_continue)
	    goto done;
	CUCON_STACK_POP(&jargs->output, cuex_t);
    }
done:
    if (have_deletion)
	cuexP_ssfn_node_check_deletable(node, jargs);
    return do_continue;
}

#if 0
static cu_rank_t
cuexP_count_active_vars(cuex_t ex)
{
    cu_rank_t sum = 0;
recurse:
    if (cuex_is_opn(ex)) {
	cu_rank_t n = cuex_opn_r(cuex_opn_from_ex(ex));
	if (n > 0) {
	    cu_rank_t i;
	    for (i = 1; i < n; ++i)
		sum += cuexP_count_active_vars(
		    cuex_opn_at(cuex_opn_from_ex(ex), i));
	    ex = cuex_opn_at(cuex_opn_from_ex(ex), 0);
	    goto recurse;
	}
    }
    else if (cuex_is_var(ex)) {
	switch (cuex_var_qcode(cuex_var_from_ex(ex))) {
	case cuex_qcode_active_s:
	case cuex_qcode_active_w:
	    ++sum;
	default:
	    break;
	}
    }
    return sum;
}
#endif

static cu_bool_t
cuexP_ssfn_find_mgu(cuex_ssfn_node_t node, cuexP_ssfn_find_mgu_jargs_t jargs)
{
/*     cuex_subst_t subst_save; */
    cuex_t key;
    cuex_meta_t key_meta;
    if (cucon_stack_is_empty(&jargs->input)) {
	cuex_ssfn_ctrl_t ctrl;
	int *sctr_this;
	cucon_stack_it_t it;
	cu_count_t n;
	cu_count_t pat_arg_cnt
	    = cucon_stack_size(&jargs->output) / sizeof(cuex_t);
	cuex_t *pat_arg_arr = cu_galloc(sizeof(cuex_var_t)*pat_arg_cnt);
	it = cucon_stack_begin(&jargs->output);
	n = pat_arg_cnt;
	while (n > 0) {
	    assert(!cucon_stack_it_is_end(it));
	    pat_arg_arr[--n] = CUCON_STACK_IT_GET(it, cuex_t);
	    CUCON_STACK_IT_ADVANCE(&it, cuex_t);
	}
	n = cucon_stack_size(&jargs->sctr);
	sctr_this = cucon_stack_continuous_top(&jargs->sctr, n);
	n /= sizeof(int);
	if (jargs->sctr_most_specific) {
	    if (!cuexP_sctr_lgr(jargs->sctr_most_specific, sctr_this + n)) {
		jargs->ctrl = cu_call(
		    jargs->cb, (void *)node,
		    jargs->subst, pat_arg_cnt, pat_arg_arr, &jargs->minfo);
	    }
	    return cuex_ssfn_ctrl_do_cont(jargs->ctrl);
	}
	else
	    ctrl = cu_call(
		jargs->cb, (void *)node,
		jargs->subst, pat_arg_cnt, pat_arg_arr, &jargs->minfo);
	if (ctrl == cuex_ssfn_ctrl_cut_mg) {
	    /* Extra 1 byte to protect from GC. */
	    memcpy(jargs->sctr_most_specific = cu_galloc(sizeof(int)*n + 1),
		   sctr_this, sizeof(int)*n);
	    jargs->sctr_most_specific += n;
	    ctrl = cuex_ssfn_ctrl_continue;
	}
	if (ctrl != cuex_ssfn_ctrl_unmatched)
	    jargs->ctrl = ctrl;
	return cuex_ssfn_ctrl_do_cont(ctrl);
    }
    if (node->seqno < jargs->min_seqno)
	return cuex_ssfn_ctrl_continue;

    key = CUCON_STACK_TOP(&jargs->input, cuex_t);
    key_meta = cuex_meta(key);
    CUCON_STACK_POP(&jargs->input, cuex_t);
    switch (cuex_meta_kind(key_meta)) {
	cuex_ssfn_node_t subssfn;
    case cuex_meta_kind_opr:
	subssfn = cucon_umap_find_mem(&node->match_opr, key_meta);
	if (subssfn) {
	    cu_rank_t n = cuex_opn_r(cuex_opn_from_ex(key));
	    cu_rank_t i = n;
	    while (i > 0)
		CUCON_STACK_PUSH(&jargs->input, cuex_t,
				 cuex_opn_at(cuex_opn_from_ex(key), --i));
	    CUCON_STACK_PUSH(&jargs->sctr, int, n);
	    if (!cuexP_ssfn_find_mgu(subssfn, jargs))
		return cu_false;
	    if (cuex_ssfn_ctrl_do_delete(jargs->ctrl)) {
		cucon_umap_erase(&node->match_opr, key_meta);
		cuexP_ssfn_node_check_deletable(node, jargs);
	    }
	    CUCON_STACK_POP(&jargs->sctr, int);
	    cucon_stack_free(&jargs->input, sizeof(cuex_t)*n);
	}
	break;

    case cuex_meta_kind_type:
	subssfn = cucon_pmap_find_mem(&node->match_exaddr, key);
	if (subssfn) {
	    CUCON_STACK_PUSH(&jargs->sctr, int, CHIB_SCTR_GENERIC);
	    if (!cuexP_ssfn_find_mgu(subssfn, jargs))
		return cu_false;
	    if (cuex_ssfn_ctrl_do_delete(jargs->ctrl)) {
		cucon_pmap_erase(&node->match_exaddr, key);
		cuexP_ssfn_node_check_deletable(node, jargs);
	    }
	    CUCON_STACK_POP(&jargs->sctr, int);
	}
	break;

    default:
	if (cuex_is_varmeta(key_meta))
	    switch (cuex_varmeta_qcode(key_meta)) {
	    case cuex_qcode_active_w:
		if (!jargs->do_lgr_w)
		    break;
	    case cuex_qcode_active_s:
		if (jargs->do_lgr) {
		    jargs->key_var = cuex_var_from_ex(key);
		    if (!cuexP_ssfn_find_mgu_expand(node, &jargs->key_arg,
						     jargs))
			return cu_false;
		}
		break;
	    case cuex_qcode_passive:
		subssfn = cucon_pmap_find_mem(&node->match_exaddr, key);
		if (subssfn) {
		    CUCON_STACK_PUSH(&jargs->sctr, int, CHIB_SCTR_GENERIC);
		    if (!cuexP_ssfn_find_mgu(subssfn, jargs))
			return cu_false;
		    if (cuex_ssfn_ctrl_do_delete(jargs->ctrl)) {
			cucon_pmap_erase(&node->match_exaddr, key);
			cuexP_ssfn_node_check_deletable(node, jargs);
		    }
		    CUCON_STACK_POP(&jargs->sctr, int);
		}
		break;
	    default:
		assert(!"Not implemented.");
		break;
	    }
	else {
	    cu_bugf("ssfn unimplemented for %!", key, key_meta);
	    assert(!"Not implemented.");
	}
	break;
    }
    if (jargs->do_mgr) {
	cu_bool_t is_wp;
	cu_rank_t pat_arg_cnt
	    = cucon_stack_size(&jargs->output)/sizeof(cuex_t);
	cu_rank_t i;
	cuex_subst_t subst_save = jargs->subst;
	for (i = 0; i < pat_arg_cnt; ++i)
	    if (ssfn_unify_cntn(node, i)) {
		cuex_t *arg_pos
		    = cucon_stack_at(&jargs->output, i*sizeof(cuex_t));
		/* [C0] See [C1]. */
		if (jargs->do_lgr
		    && (jargs->do_mgr_w
			|| !cuex_is_var(*arg_pos)
			|| cuex_var_qcode(cuex_var_from_ex(*arg_pos))
			== cuex_qcode_active_s)) {
		    jargs->subst = cuex_subst_new_uw_clone(subst_save);
		    cu_dlogf(_file, "Unify for pattern var, %! = %!?",
			     *arg_pos, key);
		    if (cuex_subst_unify(jargs->subst, *arg_pos, key)) {
			CUCON_STACK_PUSH(&jargs->sctr, int,
					 CHIB_SCTR_SPECIFIC);
			if (!cuexP_ssfn_find_mgu(ssfn_unify_cntn(node, i),
						  jargs))
			    return cu_false;
			CUCON_STACK_POP(&jargs->sctr, int);
		    }
		}
		else {
		    cu_dlogf(_file, "Compare %! = %!?", *arg_pos, key);
		    if (cuex_eq(*arg_pos, key)) {
			CUCON_STACK_PUSH(&jargs->sctr, int,
					 CHIB_SCTR_SPECIFIC);
			if (!cuexP_ssfn_find_mgu(ssfn_unify_cntn(node, i),
						  jargs))
			    return cu_false;
			CUCON_STACK_POP(&jargs->sctr, int);
		    }
		}
		if (cuex_ssfn_ctrl_do_delete(jargs->ctrl)) {
		    ssfn_unify_cntn(node, i) = NULL;
		    cuexP_ssfn_node_check_deletable(node, jargs);
		}
	    }
#ifdef SSFN_TUNING_AVOID_CROSS_SUBST_IN_MGU
	if (jargs->do_lgr && node->var_qcode == cuex_qcode_active_w
	    && ( cuex_is_varmeta_q(key_meta, cuex_qcode_u) ||
		 cuex_is_varmeta_q(key_meta, cuex_qcode_w) )
	    && cuex_subst_lookup(jargs->subst, cuex_var_from_ex(key)) == NULL)
	    goto done;
#endif
	jargs->subst = subst_save;
	is_wp = (node->var_qcode == cuex_qcode_active_s
		 || (cuex_is_var(key)
		     && cuex_var_qcode(cuex_var_from_ex(key))
		     == cuex_qcode_active_w));
	if (node->var_cntn && (jargs->do_mgr_w || is_wp)) {
/* 	    jargs->subst = cuex_subst_new_uw_clone(subst_save); */
	    if (jargs->do_mgr_w || !jargs->do_lgr
		|| (node->var_qcode == cuex_qcode_active_s
		    && (!cuex_is_var(key)
			|| cuex_var_qcode(cuex_var_from_ex(key))
			== cuex_qcode_active_s))) {
		cu_bool_t is_wp_save = jargs->minfo.is_wp;
		jargs->minfo.is_wp = is_wp;
		CUCON_STACK_PUSH(&jargs->output, cuex_t, key);
		CUCON_STACK_PUSH(&jargs->sctr, int, CHIB_SCTR_GENERIC);
		if (!cuexP_ssfn_find_mgu(node->var_cntn, jargs))
		    return cu_false;
		CUCON_STACK_POP(&jargs->sctr, int);
		CUCON_STACK_POP(&jargs->output, cuex_t);
		jargs->minfo.is_wp = is_wp_save;
	    }
	    else {
		/* [C1] Need to inform to unification [C0] that variable
		 * is strong. */
		cuex_var_t var = cuex_var_new(cuex_qcode_active_s);
		jargs->subst = cuex_subst_new_uw_clone(subst_save);
		cuex_subst_unify(jargs->subst, cuex_var_to_ex(var), key);
		CUCON_STACK_PUSH(&jargs->output, cuex_t, cuex_var_to_ex(var));
		CUCON_STACK_PUSH(&jargs->sctr, int, CHIB_SCTR_GENERIC);
		if (!cuexP_ssfn_find_mgu(node->var_cntn, jargs))
		    return cu_false;
		CUCON_STACK_POP(&jargs->sctr, int);
		CUCON_STACK_POP(&jargs->output, cuex_t);
		jargs->subst = subst_save;
	    }
/* 	    jargs->subst = subst_save; */
	    if (cuex_ssfn_ctrl_do_delete(jargs->ctrl)) {
		node->var_cntn = NULL;
		cuexP_ssfn_node_check_deletable(node, jargs);
	    }
	}
    }
done:
    CUCON_STACK_PUSH(&jargs->input, cuex_t, key);
    return cu_true;
}

cuex_ssfn_ctrl_t
cuex_ssfn_find_mgr(cuex_ssfn_t ssfn, cuex_subst_t subst, cuex_t key,
		   cuex_ssfn_seqno_t min_seqno,
		   cuex_ssfn_find_cb_t cb)
{
    struct cuexP_ssfn_find_mgu_jargs_s jargs;
    cucon_stack_init(&jargs.targets);
    cucon_stack_init(&jargs.input);
    cucon_stack_init(&jargs.output);
    cucon_stack_init(&jargs.sctr);
    CUCON_STACK_PUSH(&jargs.input, cuex_t, key);
    jargs.cb = cb;
    jargs.subst = cuex_subst_new_uw_clone(subst);
    jargs.do_lgr = cu_false;
    jargs.do_lgr_w = cu_false;
    jargs.do_mgr = cu_true;
    jargs.do_mgr_w = cu_true;
    jargs.minfo.is_wp = cu_true;
    jargs.sctr_most_specific = NULL;
    jargs.ctrl = cuex_ssfn_ctrl_unmatched;
    jargs.min_seqno = min_seqno;
    cuexP_ssfn_find_mgu(cu_upcast(cuex_ssfn_node_s, ssfn), &jargs);
    return jargs.ctrl;
}

cuex_ssfn_ctrl_t
cuex_ssfn_find_wpmgr(cuex_ssfn_t ssfn, cuex_subst_t subst, cuex_t key,
		    cuex_ssfn_seqno_t min_seqno,
		    cuex_ssfn_find_cb_t cb)
{
    struct cuexP_ssfn_find_mgu_jargs_s jargs;
    cucon_stack_init(&jargs.targets);
    cucon_stack_init(&jargs.input);
    cucon_stack_init(&jargs.output);
    cucon_stack_init(&jargs.sctr);
    CUCON_STACK_PUSH(&jargs.input, cuex_t, key);
    jargs.cb = cb;
    jargs.subst = cuex_subst_new_uw_clone(subst);
    jargs.do_lgr = cu_false;
    jargs.do_lgr_w = cu_false;
    jargs.do_mgr = cu_true;
    jargs.do_mgr_w = cu_false;
    jargs.minfo.is_wp = cu_true;
    jargs.sctr_most_specific = NULL;
    jargs.ctrl = cuex_ssfn_ctrl_unmatched;
    jargs.min_seqno = min_seqno;
    cuexP_ssfn_find_mgu(cu_upcast(cuex_ssfn_node_s, ssfn), &jargs);
    return jargs.ctrl;
}

cuex_ssfn_ctrl_t
cuex_ssfn_find_lgr(cuex_ssfn_t ssfn, cuex_subst_t subst, cuex_t key,
		  cuex_ssfn_seqno_t min_seqno,
		  cuex_ssfn_find_cb_t cb)
{
    struct cuexP_ssfn_find_mgu_jargs_s jargs;
    cucon_stack_init(&jargs.targets);
    cucon_stack_init(&jargs.input);
    cucon_stack_init(&jargs.output);
    cucon_stack_init(&jargs.sctr);
    CUCON_STACK_PUSH(&jargs.input, cuex_t, key);
    jargs.cb = cb;
    jargs.subst = cuex_subst_new_uw_clone(subst);
    jargs.do_lgr = cu_true;
    jargs.do_lgr_w = cu_true;
    jargs.do_mgr = cu_false;
    jargs.do_mgr_w = cu_false;
    jargs.minfo.is_wp = cu_true;
    jargs.sctr_most_specific = NULL;
    jargs.ctrl = cuex_ssfn_ctrl_unmatched;
    jargs.min_seqno = min_seqno;
    cuexP_ssfn_find_mgu(cu_upcast(cuex_ssfn_node_s, ssfn), &jargs);
    return jargs.ctrl;
}

cuex_ssfn_ctrl_t
cuex_ssfn_find_mgu(cuex_ssfn_t ssfn, cuex_subst_t subst, cuex_t key,
		  cuex_ssfn_seqno_t min_seqno,
		  cuex_ssfn_find_cb_t cb)
{
    struct cuexP_ssfn_find_mgu_jargs_s jargs;
    cucon_stack_init(&jargs.targets);
    cucon_stack_init(&jargs.input);
    cucon_stack_init(&jargs.output);
    cucon_stack_init(&jargs.sctr);
    CUCON_STACK_PUSH(&jargs.input, cuex_t, key);
    jargs.cb = cb;
    jargs.subst = cuex_subst_new_uw_clone(subst);
    jargs.do_lgr = cu_true;
    jargs.do_lgr_w = cu_true;
    jargs.do_mgr = cu_true;
    jargs.do_mgr_w = cu_true;
    jargs.minfo.is_wp = cu_true;
    jargs.sctr_most_specific = NULL;
    jargs.ctrl = cuex_ssfn_ctrl_unmatched;
    jargs.min_seqno = min_seqno;
    cuexP_ssfn_find_mgu(cu_upcast(cuex_ssfn_node_s, ssfn), &jargs);
    return jargs.ctrl;
}

cuex_ssfn_ctrl_t
cuex_ssfn_find_wpmgu(cuex_ssfn_t ssfn, cuex_subst_t subst, cuex_t key,
		    cuex_ssfn_seqno_t min_seqno,
		    cuex_ssfn_find_cb_t cb)
{
    struct cuexP_ssfn_find_mgu_jargs_s jargs;
    cucon_stack_init(&jargs.targets);
    cucon_stack_init(&jargs.input);
    cucon_stack_init(&jargs.output);
    cucon_stack_init(&jargs.sctr);
    CUCON_STACK_PUSH(&jargs.input, cuex_t, key);
    jargs.cb = cb;
    jargs.subst = cuex_subst_new_uw_clone(subst);
    jargs.do_lgr = cu_true;
    jargs.do_lgr_w = cu_false;
    jargs.do_mgr = cu_true;
    jargs.do_mgr_w = cu_false;
    jargs.minfo.is_wp = cu_true;
    jargs.sctr_most_specific = NULL;
    jargs.ctrl = cuex_ssfn_ctrl_unmatched;
    jargs.min_seqno = min_seqno;
    cuexP_ssfn_find_mgu(cu_upcast(cuex_ssfn_node_s, ssfn), &jargs);
    return jargs.ctrl;
}



/* Convenience
 * ----------- */

cu_clop_def(check_strength_cb2, void, cuex_veqv_t veqv)
{
    cu_debug_assert(cuex_veqv_qcode(veqv) == cuex_qcode_active_s);
}
cu_clop_def(check_strength_cb, cuex_ssfn_ctrl_t,
	    void *slot, cuex_subst_t subst,
	    cu_rank_t pat_arg_cnt, cuex_t *pat_arg_arr,
	    cuex_ssfn_matchinfo_t ign)
{
    cuex_subst_iter_veqv(subst, check_strength_cb2);
#ifdef SSFN_ENABLE_VERBOSE_DEBUG
    cuex_subst_dump(subst, stdout);
#endif
    return cuex_ssfn_ctrl_commit;
}

cu_bool_t
cuex_ssfn_have_wpmgr(cuex_ssfn_t ssfn, cuex_subst_t subst, cuex_t key,
		    cuex_ssfn_seqno_t seqno)
{
    return cuex_ssfn_find_wpmgr(ssfn, subst, key, seqno, check_strength_cb)
	== cuex_ssfn_ctrl_commit;
}

cu_bool_t
cuex_ssfn_have_wpmgu(cuex_ssfn_t ssfn, cuex_subst_t subst, cuex_t key,
		    cuex_ssfn_seqno_t seqno)
{
    return cuex_ssfn_find_wpmgu(ssfn, subst, key, seqno, check_strength_cb)
	== cuex_ssfn_ctrl_commit;
}

cu_clop_def(_ssfn_erase_cb, cuex_ssfn_ctrl_t,
	    void *slot, cuex_subst_t subst,
	    cu_rank_t pat_arg_cnt, cuex_t *pat_arg_arr,
	    cuex_ssfn_matchinfo_t minfo)
{
    return cuex_ssfn_ctrl_cont_delete;
}
void
cuex_ssfn_erase_lgr(cuex_ssfn_t ssfn, cuex_t key)
{
    cuex_ssfn_find_lgr(ssfn, NULL, key, 0, _ssfn_erase_cb);
}
void
cuex_ssfn_erase_mgr(cuex_ssfn_t ssfn, cuex_t key)
{
    cuex_ssfn_find_mgr(ssfn, NULL, key, 0, _ssfn_erase_cb);
}
void
cuex_ssfn_erase_wpmgr(cuex_ssfn_t ssfn, cuex_t key)
{
    cuex_ssfn_find_wpmgr(ssfn, NULL, key, 0, _ssfn_erase_cb);
}


/* Find Single
 * ----------- */

cu_clos_def(_ssfn_find_single_cb,
	    cu_prot(cuex_ssfn_ctrl_t,
		    void *slot, cuex_subst_t subst,
		    cu_rank_t pat_arg_cnt, cuex_t *pat_arg_arr,
		    cuex_ssfn_matchinfo_t minfo),
	    (void *slot;))
{
    cu_clos_self(_ssfn_find_single_cb);
    self->slot = slot;
    return cuex_ssfn_ctrl_commit;
}

void *
cuex_ssfn_find_single_wpmgr(cuex_ssfn_t ssfn, cuex_subst_t subst, cuex_t key)
{
    _ssfn_find_single_cb_t cb;
    if (cuex_ssfn_find_wpmgr(ssfn, subst, key, 0, _ssfn_find_single_cb_prep(&cb))
	== cuex_ssfn_ctrl_commit)
	return cb.slot;
    else
	return NULL;
}


/* Find equal within a renaming
 * ---------------------------- */

typedef struct cuexP_ssfn_find_eqr_jargs_s
{
    struct cucon_stack_s input;
    struct cucon_stack_s output;
    struct cucon_pmap_s blocked_key_vars;
    cuex_ssfn_ctrl_t ctrl;
    cu_clop(cb, cuex_ssfn_ctrl_t, void *);
} *cuexP_ssfn_find_eqr_jargs_t;

static void
cuexP_ssfn_node_check_deletable_eqr(cuex_ssfn_node_t node,
				   cuexP_ssfn_find_eqr_jargs_t jargs)
{
    if (node->var_cntn != NULL
	|| !cucon_pmap_is_empty(&node->match_exaddr)
	|| !cucon_umap_is_empty(&node->match_opr)) {
	jargs->ctrl = cuex_ssfn_ctrl_continue;
	return;
    }
    else {
	cu_rank_t N = CUCON_STACK_SIZE(&jargs->output, cuex_t);
	cu_rank_t n;
	for (n = 0; n < N; ++n)
	    if (ssfn_unify_cntn(node, n)) {
		jargs->ctrl = cuex_ssfn_ctrl_continue;
		return;
	    }
    }
    jargs->ctrl = cuex_ssfn_ctrl_cont_delete;
    return;
}

static cu_bool_t
cuexP_ssfn_find_eqr(cuex_ssfn_node_t node, cuexP_ssfn_find_eqr_jargs_t jargs)
{
    /* XXoptim: Return immediately on first and only match. */
    cuex_t key;
    cuex_meta_t key_meta;
    if (cucon_stack_is_empty(&jargs->input)) {
	cuex_ssfn_ctrl_t ctrl = cu_call(jargs->cb, (void *)node);
	if (ctrl != cuex_ssfn_ctrl_unmatched)
	    jargs->ctrl = ctrl;
	return cuex_ssfn_ctrl_do_cont(ctrl);
    }
    key = CUCON_STACK_TOP(&jargs->input, cuex_t);
    key_meta = cuex_meta(key);
    CUCON_STACK_POP(&jargs->input, cuex_t);
    switch (cuex_meta_kind(key_meta)) {
	cuex_ssfn_node_t subssfn;
    case cuex_meta_kind_opr:
	subssfn = cucon_umap_find_mem(&node->match_opr, key_meta);
	if (subssfn) {
	    cu_rank_t n = cuex_opn_r(cuex_opn_from_ex(key));
	    cu_rank_t i = n;
	    while (i > 0)
		CUCON_STACK_PUSH(&jargs->input, cuex_t,
			       cuex_opn_at(cuex_opn_from_ex(key), --i));
	    if (!cuexP_ssfn_find_eqr(subssfn, jargs))
		return cu_false;
	    if (cuex_ssfn_ctrl_do_delete(jargs->ctrl)) {
		cucon_umap_erase(&node->match_opr, key_meta);
		cuexP_ssfn_node_check_deletable_eqr(node, jargs);
	    }
	    cucon_stack_free(&jargs->input, sizeof(cuex_t)*n);
	}
	break;

    case cuex_meta_kind_type:
	subssfn = cucon_pmap_find_mem(&node->match_exaddr, key);
	if (subssfn) {
	    if (!cuexP_ssfn_find_eqr(subssfn, jargs))
		return cu_false;
	    if (cuex_ssfn_ctrl_do_delete(jargs->ctrl)) {
		cucon_pmap_erase(&node->match_exaddr, key);
		cuexP_ssfn_node_check_deletable_eqr(node, jargs);
	    }
	}
	break;

    default:
	if (cuex_is_varmeta(key_meta))
	    switch (cuex_varmeta_qcode(key_meta)) {
		cu_rank_t pat_arg_cnt;
		cu_rank_t i;
		cuex_ssfn_node_t subssfn;
	    case cuex_qcode_passive:
		subssfn = cucon_pmap_find_mem(&node->match_exaddr, key);
		if (subssfn) {
		    if (!cuexP_ssfn_find_eqr(subssfn, jargs))
			return cu_false;
		    if (cuex_ssfn_ctrl_do_delete(jargs->ctrl)) {
			cucon_pmap_erase(&node->match_exaddr, key);
			cuexP_ssfn_node_check_deletable_eqr(node, jargs);
		    }
		}
		break;
	    case cuex_qcode_active_s:
	    case cuex_qcode_active_w:
		pat_arg_cnt = cucon_stack_size(&jargs->output)/sizeof(cuex_t);
		for (i = 0; i < pat_arg_cnt; ++i)
		    if (ssfn_unify_cntn(node, i)) {
			cuex_t *arg_pos
			    = cucon_stack_at(&jargs->output, i*sizeof(cuex_t));
			if (cuex_eq(*arg_pos, key)) {
			    if (!cuexP_ssfn_find_eqr(ssfn_unify_cntn(node, i),
						    jargs))
				return cu_false;
			}
			if (cuex_ssfn_ctrl_do_delete(jargs->ctrl)) {
			    ssfn_unify_cntn(node, i) = NULL;
			    cuexP_ssfn_node_check_deletable_eqr(node, jargs);
			}
		    }
		if (node->var_cntn
		    && cucon_pmap_insert_void(&jargs->blocked_key_vars,
					    cuex_var_from_ex(key))) {
		    CUCON_STACK_PUSH(&jargs->output, cuex_t, key);
		    if (!cuexP_ssfn_find_eqr(node->var_cntn, jargs))
			return cu_false;
		    CUCON_STACK_POP(&jargs->output, cuex_t);
		    if (cuex_ssfn_ctrl_do_delete(jargs->ctrl)) {
			node->var_cntn = NULL;
			cuexP_ssfn_node_check_deletable_eqr(node, jargs);
		    }
		}
		break;
	    default:
		cu_debug_unreachable();
		break;
	    }
	else
	    cu_debug_unreachable();
	break;
    }
    CUCON_STACK_PUSH(&jargs->input, cuex_t, key);
    return cu_true;
}

cuex_ssfn_ctrl_t
cuex_ssfn_find_eqr(cuex_ssfn_t ssfn, cuex_t key,
		  cu_clop(cb, cuex_ssfn_ctrl_t, void *))
{
    struct cuexP_ssfn_find_eqr_jargs_s jargs;
    cucon_stack_init(&jargs.input);
    cucon_stack_init(&jargs.output);
    CUCON_STACK_PUSH(&jargs.input, cuex_t, key);
    jargs.ctrl = cuex_ssfn_ctrl_unmatched;
    jargs.cb = cb;
    cucon_pmap_init(&jargs.blocked_key_vars);
    cuexP_ssfn_find_eqr(cu_upcast(cuex_ssfn_node_s, ssfn), &jargs);
    return jargs.ctrl;
}


/* Dump
 * ---- */

cu_clos_def(_ssfn_dump_keys_cb,
	    cu_prot(cuex_ssfn_ctrl_t,
		    void *slot, cuex_subst_t subst,
		    cu_rank_t pat_arg_cnt, cuex_t *pat_arg_arr,
		    cuex_ssfn_matchinfo_t minfo),
	    (cuex_var_t var; FILE *out;))
{
    cu_clos_self(_ssfn_dump_keys_cb);
    cu_fprintf(self->out, "    %!\n",
	       cuex_subst_apply(subst, cuex_var_to_ex(self->var)));
    return cuex_ssfn_ctrl_continue;
}
void
cuex_ssfn_dump_keys(cuex_ssfn_t ssfn, FILE *out)
{
    _ssfn_dump_keys_cb_t cb;
    cb.var = cuex_var_new(cuex_qcode_active_s);
    cb.out = out;
    fprintf(out, "DUMP OF ssfn @ %p:\n", (void *)ssfn);
    cuex_ssfn_find_lgr(ssfn, NULL, cuex_var_to_ex(cb.var), 0,
		       _ssfn_dump_keys_cb_prep(&cb));
    fputc('\n', out);
}

/* Initialisation
 * -------------- */

#if 0
void
cuexP_ssfn_init_bindings(chi_sfn_t sfn)
{
    chi_sfn_def_cstr_cst(sfn, "ssfn_ctrl_t", chi_cls_to_ex(chi_clsof_uint()));
    chi_sfn_def_cstr_cst(sfn, "ssfn_ctrl_continue",
			 chi_obj_to_ex(
			     chi_obj_new_int(cuex_ssfn_ctrl_continue)));
    chi_sfn_def_cstr_cst(sfn, "ssfn_ctrl_unmatched",
			 chi_obj_to_ex(
			     chi_obj_new_int(cuex_ssfn_ctrl_unmatched)));
    chi_sfn_def_cstr_cst(sfn, "ssfn_ctrl_commit",
			 chi_obj_to_ex(
			     chi_obj_new_int(cuex_ssfn_ctrl_commit)));
    chi_sfn_def_cstr_cst(sfn, "ssfn_ctrl_fail",
			 chi_obj_to_ex(
			     chi_obj_new_int(cuex_ssfn_ctrl_fail)));
}
#endif
