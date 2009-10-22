/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2002--2007  Petter Urkedal <urkedal@nbi.dk>
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

#include <cu/clos.h>
#include <cucon/po.h>
#include <cucon/pmap.h>
#include <cu/memory.h>
#include <cucon/priq.h>
#include <cucon/algo_colour.h>
#include <cucon/stack.h>
#include <cu/thread.h>
#include <cu/debug.h>
#include <cu/str.h>
#include <cu/diag.h>

#include <assert.h>
#include <stdio.h>

void
cucon_po_cct_2elt(cucon_po_t po, cucon_poelt_t bot, cucon_poelt_t top)
{
    cu_debug_assert(cucon_list_is_empty(&bot->isuccs));
    cu_debug_assert(cucon_list_is_empty(&bot->ipreds));
    cu_debug_assert(cucon_list_is_empty(&top->isuccs));
    cu_debug_assert(cucon_list_is_empty(&top->ipreds));
    po->bot->level = 0;
    po->top->level = 1;
#ifdef CUCON_PO_ELT_LINKS_PO
    po->bot->po = po;
    po->top->po = po;
#endif
    cucon_list_prepend_ptr(&po->bot->isuccs, po->top);
    cucon_list_prepend_ptr(&po->top->ipreds, po->bot);
}

void
cucon_po_cct_2elt_cct(cucon_po_t po, cucon_poelt_t bot, cucon_poelt_t top)
{
    po->top = top;
    po->bot = bot;
    bot->level = 0;
    top->level = 1;
#ifdef CUCON_PO_ELT_LINKS_PO
    bot->po = po;
    top->po = po;
#endif
    cucon_list_init(&bot->isuccs);
    cucon_list_prepend_ptr(&bot->isuccs, top);
    cucon_list_init(&bot->ipreds);
    cucon_list_init(&top->isuccs);
    cucon_list_init(&top->ipreds);
    cucon_list_prepend_ptr(&top->ipreds, bot);
}

void
cucon_po_cct_mem(cucon_po_t po, size_t size_bot, size_t size_top)
{
    cucon_poelt_t bot = cu_galloc(sizeof(struct cucon_poelt_s) + size_bot);
    cucon_poelt_t top = cu_galloc(sizeof(struct cucon_poelt_s) + size_top);
    cucon_po_cct_2elt_cct(po, bot, top);
}

cucon_po_t
cucon_po_new_mem(size_t size_bot, size_t size_top)
{
    cucon_po_t po = cu_gnew(struct cucon_po_s);
    cucon_po_cct_mem(po, size_bot, size_top);
    return po;
}

void
cucon_po_cct_ptr(cucon_po_t po, void *bot, void *top)
{
    cucon_po_cct_mem(po, sizeof(void *), sizeof(void *));
    *(void **)cucon_poelt_get_mem(po->bot) = bot;
    *(void **)cucon_poelt_get_mem(po->top) = top;
}

cucon_po_t
cucon_po_new_ptr(void *bot, void *top)
{
    cucon_po_t po = cu_gnew(struct cucon_po_s);
    cucon_po_cct_ptr(po, bot, top);
    return po;
}

void     (cucon_po_cct)(cucon_po_t po) { cucon_po_cct(po); }
cucon_po_t (cucon_po_new)(void)        { return cucon_po_new(); }

void
cucon_poelt_cct(cucon_poelt_t e)
{
    cucon_list_init(&e->isuccs);
    cucon_list_init(&e->ipreds);
#if defined(CUCON_PO_ELT_LINKS_PO) && !defined(CU_NDEBUG)
    e->po = NULL;
#endif
}

cucon_poelt_t
cucon_poelt_new_alloc(size_t size)
{
    cucon_poelt_t elt = cu_galloc(sizeof(struct cucon_poelt_s) + size);
    cucon_list_init(&elt->isuccs);
    cucon_list_init(&elt->ipreds);
#if defined(CUCON_PO_ELT_LINKS_PO) && !defined(CU_NDEBUG)
    elt->po = NULL;
#endif
    return elt;
}

cucon_poelt_t
(cucon_poelt_of_data)(void *data)
{
    return cucon_poelt_of_data(data);
}

cucon_poelt_t
(cucon_po_bot)(cucon_po_t po)
{
    return cucon_po_bot(po);
}

cucon_poelt_t
(cucon_po_top)(cucon_po_t po)
{
    return cucon_po_top(po);
}

/* Fix level of 'e' to 'level', and fix level of successors accordingly. */
static void
_po_update_level_to(cucon_poelt_t e, unsigned int level)
{
    cucon_po_isucc_it_t it;
    e->level = level;
    ++level;
    for (it = cucon_po_isucc_begin(e); it != cucon_po_isucc_end(e);
	 it = cucon_po_isucc_it_next(it)) {
	cucon_poelt_t e_succ = cucon_po_isucc_it_get(it);
	if (e_succ->level < level)
	    _po_update_level_to(e_succ, level);
    }
}

/* Set level of 'e' according to its immediate predecessors, and fix
 * successors. */
static void
_po_update_level(cucon_poelt_t e)
{
    cucon_po_ipred_it_t it;
    unsigned int level = 0;
    for (it = cucon_po_ipred_begin(e); it != cucon_po_ipred_end(e);
	 it = cucon_po_ipred_it_next(it)) {
	unsigned int l = cucon_po_ipred_it_get(it)->level;
	if (l > level)
	    level = l;
    }
    e->level = ++level;
    ++level;
    for (it = cucon_po_isucc_begin(e); it != cucon_po_isucc_end(e);
	 it = cucon_po_isucc_it_next(it)) {
	cucon_poelt_t e_succ = cucon_po_isucc_it_get(it);
	if (e_succ->level < level)
	    _po_update_level_to(e_succ, level);
    }
}

void
cucon_po_insert_cct(cucon_po_t po, cucon_poelt_t e)
{
#ifdef CUCON_PO_ELT_LINKS_PO
    e->po = po;
#endif
    cucon_list_init(&e->ipreds);
    cucon_list_init(&e->isuccs);
    cucon_list_prepend_ptr(&e->ipreds, po->bot);
    cucon_list_prepend_ptr(&e->isuccs, po->top);
    cucon_list_prepend_ptr(&po->bot->isuccs, e);
    cucon_list_prepend_ptr(&po->top->ipreds, e);
    _po_update_level(e);
}

cucon_poelt_t
cucon_po_insert_mem(cucon_po_t po, size_t size)
{
    cucon_poelt_t e = cu_galloc(sizeof(struct cucon_poelt_s) + size);
    cucon_po_insert_cct(po, e);
    return e;
}

cucon_poelt_t
cucon_po_insert_ptr(cucon_po_t po, void *ptr)
{
    cucon_poelt_t e;
    e = cu_galloc(sizeof(struct cucon_poelt_s) + sizeof(void *));
    *(void **)CU_ALIGNED_PTR_END(e) = ptr;
    cucon_po_insert_cct(po, e);
    return e;
}


/* cucon_po_prec, cucon_po_preceq
 * ------------------------------ */

/* Spearches downwards from e1 for e0 and returns true iff it is found.
 * Visited elements are inserted into lspan1, so if e0 ≮ e1, then lspan1 will
 * contain the lower span of e1 upon exit. */
#ifdef CUCONP_USE_PO_COLLECT_LSPAN
static cu_bool_t
_po_prec_collect_lspan(cucon_poelt_t e0, cucon_poelt_t e1, cucon_pmap_t lspan1)
{
    cucon_po_ipred_it_t it;
    /* Can't use level here, since lspan is needed */
    for (it = cucon_po_ipred_begin(e1);
	 it != cucon_po_ipred_end(e1);
	 it = cucon_po_ipred_it_next(it)) {
	cucon_poelt_t e = cucon_po_ipred_it_get(it);
	if (e == e0)
	    return cu_true;
	if (!cucon_pmap_insert_void(lspan1, e))
	    continue;
	if (_po_prec_collect_lspan(e0, e, lspan1))
	    return cu_true;
    }
    return cu_false;
}
#endif

/* Searches upwards from e0 for e1 and returns true iff it is found.  Visited
 * elements are inserted into uspan0, so if e0 ≮ e1, then uspan0 will contain
 * the upper span of e0 upon exit. */
static cu_bool_t
_po_prec_collect_uspan(cucon_poelt_t e0, cucon_poelt_t e1, cucon_pmap_t uspan0)
{
    cucon_po_isucc_it_t it;
    /* Can't use level here, since uspan is needed */
    for (it = cucon_po_isucc_begin(e0);
	 it != cucon_po_isucc_end(e0);
	 it = cucon_po_isucc_it_next(it)) {
	cucon_poelt_t e = cucon_po_isucc_it_get(it);
	if (e == e1)
	    return cu_true;
	if (!cucon_pmap_insert_void(uspan0, e))
	    continue;
	if (_po_prec_collect_uspan(e, e1, uspan0))
	    return cu_true;
    }
    return cu_false;
}

static cu_bool_t
_po_prec(cucon_poelt_t e0, cucon_poelt_t e1, cucon_pmap_t uspan)
{
    cucon_listnode_t it;
    if (e0->level >= e1->level)
	return cu_false;
    for (it = cucon_po_isucc_begin(e0);
	 it != cucon_po_isucc_end(e0);
	 it = cucon_po_isucc_it_next(it)) {
	cucon_poelt_t e = cucon_po_isucc_it_get(it);
	if (e == e1)
	    return cu_true;
	if (!cucon_pmap_insert_void(uspan, e))
	    continue;
	if (_po_prec(e, e1, uspan))
	    return cu_true;
    }
    return cu_false;
}

cu_bool_t
cucon_po_prec(cucon_poelt_t e0, cucon_poelt_t e1)
{
    struct cucon_pmap_s done;
    cucon_pmap_init(&done);
    return _po_prec(e0, e1, &done);
}
#define _po_preceq cucon_po_preceq /* XX */


/* cucon_po_constain_prec
 * ---------------------- */

#ifdef CUCONP_USE_PO_COLLECT_LSPAN
static void
_po_for_succeq_remove_preds_in_pmap(cucon_poelt_t e, cucon_pmap_t rm,
				    cucon_pmap_t uspan)
{
    cucon_listnode_t it;
    it = cucon_po_ipred_begin(e);
    while (it != cucon_po_ipred_end(e)) {
	cucon_poelt_t e_pred = cucon_po_ipred_it_get(it);
	if (cucon_pmap_find_mem(rm, e_pred)) {
	    cucon_list_erase_ptr(&e_pred->isuccs, e);
	    it = cucon_list_erase_node(it);
	}
	else
	    it = cucon_po_ipred_it_next(it);
    }
    for (it = cucon_po_isucc_begin(e); it != cucon_po_isucc_end(e);
	 it = cucon_po_isucc_it_next(it)) {
	cucon_poelt_t e_succ = cucon_po_isucc_it_get(it);
	if (cucon_pmap_insert_void(uspan, e_succ))
	    _po_for_succeq_remove_preds_in_pmap(e_succ, rm, uspan);
    }
}
#else
static void
_po_for_preceq_remove_succs_in_pmap(cucon_poelt_t e, cucon_pmap_t rm,
				    cucon_pmap_t lspan)
{
    cucon_listnode_t it;
    it = cucon_po_isucc_begin(e);
    while (it != cucon_po_isucc_end(e)) {
	cucon_poelt_t e_succ = cucon_po_isucc_it_get(it);
	if (cucon_pmap_find_mem(rm, e_succ)) {
	    cucon_list_erase_ptr(&e_succ->ipreds, e);
	    it = cucon_list_erase_node(it);
	}
	else
	    it = cucon_po_isucc_it_next(it);
    }
    for (it = cucon_po_ipred_begin(e); it != cucon_po_ipred_end(e);
	 it = cucon_po_ipred_it_next(it)) {
	cucon_poelt_t e_pred = cucon_po_ipred_it_get(it);
	if (cucon_pmap_insert_void(lspan, e_pred))
	    _po_for_preceq_remove_succs_in_pmap(e_pred, rm, lspan);
    }
}
#endif

cu_bool_t
cucon_po_constrain_prec(cucon_po_t po, cucon_poelt_t e0, cucon_poelt_t e1)
{
    struct cucon_pmap_s lspan0, uspan1;
    if (e0 == e1)
	return cu_false;
    if (cucon_po_prec(e0, e1))
	return cu_true;
#ifdef CUCONP_USE_PO_COLLECT_LSPAN
    cucon_pmap_init(&lspan0);
    if (_po_prec_collect_lspan(e1, e0, &lspan0))
	return cu_false;
    cucon_pmap_insert_void(&lspan0, e0);
    cucon_pmap_init(&uspan1);
    _po_for_succeq_remove_preds_in_pmap(e1, &lspan0, &uspan1);
#else
    cucon_pmap_init(&uspan1);
    if (_po_prec_collect_uspan(e1, e0, &uspan1))
	return cu_false;
    cucon_pmap_insert_void(&uspan1, e1);
    cucon_pmap_init(&lspan0);
    _po_for_preceq_remove_succs_in_pmap(e0, &uspan1, &lspan0);
#endif
    cucon_list_prepend_ptr(&e0->isuccs, e1);
    cucon_list_prepend_ptr(&e1->ipreds, e0);
    _po_update_level(e1);
    return cu_true;
}


/* cucon_range_and_bounds_of_fn
 * ---------------------------- */

static void
_po_ucollect_preds(cucon_poelt_t low,
		   cu_clop(cmp_fn, cucon_pocmp_t, cucon_poelt_t),
		   cucon_pmap_t preds, cucon_pmap_t cache)
{
    cucon_po_isucc_it_t it;
    cu_bool_t have_pred = cu_false;
    for (it = cucon_po_isucc_begin(low);
	 it != cucon_po_isucc_end(low); it = cucon_po_isucc_it_next(it)) {
	cucon_pocmp_t *cmp;
	cucon_poelt_t e = cucon_po_isucc_it_get(it);
	if (cucon_pmap_insert_mem(cache, e, sizeof(cucon_pocmp_t), &cmp)) {
	    *cmp = cu_call(cmp_fn, e);
	    if (*cmp == cucon_pocmp_prec) {
		_po_ucollect_preds(e, cmp_fn, preds, cache);
		have_pred = cu_true;
	    }
	}
	else if (*cmp == cucon_pocmp_prec)
	    have_pred = cu_true;
    }
    if (!have_pred)
	cucon_pmap_insert_void(preds, low);
}

static void
_po_dcollect_range(cucon_poelt_t high,
		   cu_clop(cmp_fn, cucon_pocmp_t, cucon_poelt_t),
		   cucon_pmap_t range, cucon_pmap_t cache)
{
    cucon_po_ipred_it_t it;
    cucon_pmap_insert_void(range, high);
    for (it = cucon_po_ipred_begin(high);
	 it != cucon_po_ipred_end(high); it = cucon_po_ipred_it_next(it)) {
	cucon_pocmp_t *cmp;
	cucon_poelt_t e = cucon_po_ipred_it_get(it);
	if (cucon_pmap_insert_mem(cache, e, sizeof(cucon_pocmp_t), &cmp)) {
	    *cmp = cu_call(cmp_fn, e);
	    if (*cmp == cucon_pocmp_eq)
		_po_dcollect_range(e, cmp_fn, range, cache);
	}
    }
}

static void
_po_dcollect_range_and_succs(cucon_poelt_t high,
			     cu_clop(cmp_fn, cucon_pocmp_t, cucon_poelt_t),
			     cucon_pmap_t range, cucon_pmap_t succs,
			     cucon_pmap_t cache)
{
    cu_bool_t have_succ = cu_false;
    cucon_po_ipred_it_t it;
    for (it = cucon_po_ipred_begin(high);
	 it != cucon_po_ipred_end(high); it = cucon_po_ipred_it_next(it)) {
	cucon_pocmp_t *cmp;
	cucon_poelt_t e = cucon_po_ipred_it_get(it);
	if (cucon_pmap_insert_mem(cache, e, sizeof(cucon_pocmp_t), &cmp)) {
	    *cmp = cu_call(cmp_fn, e);
	    switch (*cmp) {
		case cucon_pocmp_prec:
		    _po_dcollect_range_and_succs(e, cmp_fn,
						 range, succs, cache);
		    have_succ = cu_true;
		    break;
		case cucon_pocmp_eq:
		    _po_dcollect_range(e, cmp_fn, range, cache);
		    break;
		default:
		    break;
	    }
	}
	else if (*cmp == cucon_pocmp_succ)
	    have_succ = cu_true;
    }
    if (!have_succ)
	cucon_pmap_insert_void(succs, high);
}

void
cucon_po_range_and_bounds_of_fn(cucon_poelt_t bot, cucon_poelt_t top,
				cu_clop(cmp_fn, cucon_pocmp_t, cucon_poelt_t),
				cucon_pmap_t range,
				cucon_pmap_t preds, cucon_pmap_t succs)
{
    cucon_pocmp_t *cmp;
    struct cucon_pmap_s cache;
    cucon_pmap_init(&cache);
    cucon_pmap_insert_mem(&cache, top, sizeof(cucon_pocmp_t), &cmp);
    *cmp = cu_call(cmp_fn, top);
    switch (*cmp) {
	case cucon_pocmp_succ:
	    cu_debug_unreachable();
	    break;
	case cucon_pocmp_eq:
	    _po_dcollect_range(top, cmp_fn, range, &cache);
	    break;
	case cucon_pocmp_prec:
	    _po_dcollect_range_and_succs(top, cmp_fn, range, succs, &cache);
	    break;
	case cucon_pocmp_unord:
	    break;
    }
    if (cucon_pmap_insert_mem(&cache, bot, sizeof(cucon_pocmp_t), &cmp))
	*cmp = cu_call(cmp_fn, bot);
    switch (*cmp) {
	case cucon_pocmp_prec:
	    cu_debug_unreachable();
	    break;
	case cucon_pocmp_eq:
	    break;
	case cucon_pocmp_succ:
	    _po_ucollect_preds(bot, cmp_fn, preds, &cache);
	    break;
	case cucon_pocmp_unord:
	    break;
    }
}

// TODO
#if 0
cucon_poelt_t
cucon_po_constrain_or_unify(cucon_po_t po, cucon_poelt_t e0, cucon_poelt_t e1,
			    cucon_pmap_t deleted)
{
    struct cucon_pmap_s range;
    if (e0 == e1)
	return NULL;
    if (cucon_po_prec(e0, e1))
	return cu_true;
    cucon_pmap_init(&deleted);
    if (cucon_po_open_range_accu(e1, e0, &deleted)) {
	cucon_pmap_insert_void(deleted, e0);
	cucon_pmap_insert_void(deleted, e1);
    }
    else {
	struct cucon_pmap_s lspan0, uspan0;
	cucon_pmap_init(&uspan1);
	cucon_po_uspan(e1, &uspan1);
	cucon_pmap_insert_void(&uspan1, e1);
	cucon_pmap_init(&lspan0);
	_po_for_preceq_remove_succs_in_pmap(e0, &uspan1, &lspan0);
	cucon_list_prepend_ptr(&e0->isuccs, e1);
	cucon_list_prepend_ptr(&e1->ipreds, e0);
	_po_update_level(e1);
    }
}
#endif

#if 0
/* cucon_po_insert_constrain
 * ----------------------- */

/* Assume 'e_low ≺ e' and insert ipreds of 'e' according to 'prec'
 * by seaching upwards from 'e_search'.  Elements less than 'e' are
 * coloured cucon_algo_colour_blue and the immediate elements above
 * without this property is coloured cucon_algo_colour_green. */
static void
_po_elt_insert_ipreds_aboveeq_C(cucon_poelt_t e,
				cu_clop(prec, cu_bool_t, void *, void *),
				cucon_poelt_t e_low)
{
    cucon_po_isucc_it_t it_isucc;
    void *value = cucon_poelt_get(e);
    cu_bool_t is_ipred = cu_true;
    cucon_poelt_t e_max = cucon_po_top(e->po);
    for (it_isucc = cucon_po_isucc_begin(e_low);
	 it_isucc != cucon_po_isucc_end(e_low);
	 it_isucc = cucon_po_isucc_it_next(it_isucc)) {
	cucon_poelt_t e_isucc = cucon_po_isucc_it_get(it_isucc);
	switch (e_isucc->algo_nonrec_colour) {
	case cucon_algo_colour_white:	/* unchecked */
	    if (e_isucc != e_max
		&& cu_call(prec, cucon_poelt_get(e_isucc), value)) {
		e_isucc->algo_nonrec_colour = cucon_algo_colour_blue;
		_po_elt_insert_ipreds_aboveeq_C(e, prec, e_isucc);
		is_ipred = cu_false;
	    }
	    else
		e_isucc->algo_nonrec_colour = cucon_algo_colour_green;
	    break;

	case cucon_algo_colour_blue:
	    is_ipred = cu_false;
	    break;

#ifndef CU_NDEBUG
	case cucon_algo_colour_green:	/* does not precede 'e'. */
	    break;
	default:
	    assert(!"Not reached.");
#else
	default:
	    break;
#endif
	}
    }

    /* Add constrait to 'e' only (will be completed later). */
    if (is_ipred)
	cucon_list_prepend_ptr(&e->ipreds, e_low);
}
/* The mirror of the above, but also take into account the colouring done by
 * the above algorithm. */
static void
_po_elt_insert_isuccs_beloweq_C(cucon_poelt_t e,
				cu_clop(prec, cu_bool_t, void *, void *),
				cucon_poelt_t e_high)
{
    cucon_po_ipred_it_t it_ipred;
    void *value = cucon_poelt_get(e);
    cu_bool_t is_isucc = cu_true;
    cucon_poelt_t e_min = cucon_po_min(e->po);
    for (it_ipred = cucon_po_ipred_begin(e_high);
	 it_ipred != cucon_po_ipred_end(e_high);
	 it_ipred = cucon_po_ipred_it_next(it_ipred)) {
	cucon_poelt_t e_ipred = cucon_po_ipred_it_get(it_ipred);
	switch (e_ipred->algo_nonrec_colour) {
	case cucon_algo_colour_white:	/* unchecked */
	case cucon_algo_colour_green:	/* checked with opposite predicate */
	    if (e_ipred != e_min
		&& cu_call(prec, value, cucon_poelt_get(e_ipred))) {
		e_ipred->algo_nonrec_colour = cucon_algo_colour_red;
		_po_elt_insert_isuccs_beloweq_C(e, prec, e_ipred);
		is_isucc = cu_false;
	    }
	    else
		e_ipred->algo_nonrec_colour = cucon_algo_colour_yellow;
	    break;

	case cucon_algo_colour_red:	/* confirmed to succeed 'e' */
	    is_isucc = cu_false;
	    break;

#ifndef CU_NDEBUG
	case cucon_algo_colour_blue:	/* does not succeed (precedes) 'e' */
	case cucon_algo_colour_yellow:	/* does not succeed 'e' */
	    break;
	default:
	    assert(!"Not reached.");
#else
	default:
	    break;
#endif
	}
    }

    /* Add constrait to 'e' only (will be completed later). */
    if (is_isucc)
	cucon_list_prepend_ptr(&e->isuccs, e_high);
}
void
cucon_po_insert_constrain(cucon_po_t po, cucon_poelt_t e,
			  cu_clop(prec, cu_bool_t, void *, void *))
{
    cucon_po_isucc_it_t it_isucc;
    cucon_po_ipred_it_t it_ipred;
    /*cuflow_mode_t flow;*/
#ifdef CUCON_PO_ELT_LINKS_PO
    e->po = po;
    e->algo_colours = 0;
    e->algo_nonrec_colour = cucon_algo_colour_white;
#endif
    /* Protect from nonlinear control flow. */
    /*cuflow_set_save_current_flow(cuflow_mode_det, &flow);*/

    ASSERT_UNCOLOURED(po);
    _po_elt_insert_ipreds_aboveeq_C(e, prec, cucon_po_min(po));
    _po_elt_insert_isuccs_beloweq_C(e, prec, cucon_po_top(po));

    /* Now, all nodes which are ordered with respect to 'e' are
     * coloured red or blue.  Insertion of 'e' will make all links
     * between a red and a blue node redundant, so remove these. */
    assert(e->algo_nonrec_colour == cucon_algo_colour_white);
    remove_red_isuccs_below_use_gray(e);

    /* Finish linking. */
    assert(cucon_po_ipred_begin(e) != cucon_po_ipred_end(e));
    assert(cucon_po_isucc_begin(e) != cucon_po_isucc_end(e));
    for (it_ipred = cucon_po_ipred_begin(e);
	 it_ipred != cucon_po_ipred_end(e);
	 it_ipred = cucon_po_ipred_it_next(it_ipred)) {
	cucon_poelt_t e_ipred = cucon_po_ipred_it_get(it_ipred);
	cucon_list_prepend_ptr(&e_ipred->isuccs, e);
    }
    for (it_isucc = cucon_po_isucc_begin(e);
	 it_isucc != cucon_po_isucc_end(e);
	 it_isucc = cucon_po_isucc_it_next(it_isucc)) {
	cucon_poelt_t e_isucc = cucon_po_isucc_it_get(it_isucc);
	cucon_list_prepend_ptr(&e_isucc->ipreds, e);
    }

    /* Clear colours. */
    e->algo_nonrec_colour = cucon_algo_colour_red; /* close hole */
    colour_up_to_same_colour(cucon_po_min(po), cucon_algo_colour_white);
    colour_down_to_same_colour(cucon_po_top(po), cucon_algo_colour_white);
    ASSERT_UNCOLOURED(po);

    _po_update_level(e);

    /* No we are back in a consistent state. */
    /*cuflow_set_current_mode(flow);*/
}
#endif


/* cucon_po_iter_open_range, cucon_po_iter_closed_range
 * cucon_po_iter_left_range, cucon_po_iter_right_range
 * ---------------------------------------------------- */

static int
_po_for_open_range_dw(cucon_poelt_t low, cucon_poelt_t high,
		      cu_clop(f, void, cucon_poelt_t), cucon_pmap_t visit_set)
{
    cucon_po_ipred_it_t it_ipred;
    int reach = 0;
    for (it_ipred = cucon_po_ipred_begin(high);
	 it_ipred != cucon_po_ipred_end(high);
	 it_ipred = cucon_po_ipred_it_next(it_ipred)) {
	int *visit;
	cucon_poelt_t e_ipred = cucon_po_ipred_it_get(it_ipred);
	if (e_ipred == low)
	    return 1;
	if (!cucon_pmap_insert_mem(visit_set, e_ipred,
				   sizeof(int), &visit)) {
	    reach += *visit;
	    continue;
	}
	if (_po_for_open_range_dw(low, e_ipred, f, visit_set)) {
	    *visit = 1;
	    ++reach;
	    cu_call(f, e_ipred);
	}
	else
	    *visit = 0;
    }
    return reach;
}

void
cucon_po_iter_open_range(cucon_poelt_t low, cucon_poelt_t high,
			 cu_clop(f, void, cucon_poelt_t))
{
    struct cucon_pmap_s visit_set;
    cucon_pmap_init(&visit_set);
    _po_for_open_range_dw(low, high, f, &visit_set);
}

void
cucon_po_iter_left_range(cucon_poelt_t min, cucon_poelt_t high,
			 cu_clop(f, void, cucon_poelt_t))
{
    struct cucon_pmap_s visit_set;
    cucon_pmap_init(&visit_set);
    if (_po_for_open_range_dw(min, high, f, &visit_set))
	cu_call(f, min);
}
void
cucon_po_iter_right_range(cucon_poelt_t low, cucon_poelt_t max,
			  cu_clop(f, void, cucon_poelt_t))
{
    struct cucon_pmap_s visit_set;
    cucon_pmap_init(&visit_set);
    if (_po_for_open_range_dw(low, max, f, &visit_set))
	cu_call(f, max);
}
void
cucon_po_iter_closed_range(cucon_poelt_t min, cucon_poelt_t max,
			   cu_clop(f, void, cucon_poelt_t))
{
    struct cucon_pmap_s visit_set;
    cucon_pmap_init(&visit_set);
    if (min == max)
	cu_call(f, min);
    else if (_po_for_open_range_dw(min, max, f, &visit_set)) {
	cu_call(f, min);
	cu_call(f, max);
    }
}

/* cucon_po_iter_ipreds, cucon_po_iter_isuccs
 * ------------------------------------------ */
void
cucon_po_iter_ipreds(cucon_poelt_t e, cu_clop(f, void, cucon_poelt_t))
{
    cucon_po_ipred_it_t it;
    for (it = cucon_po_ipred_begin(e); it != cucon_po_ipred_end(e);
	 it = cucon_po_ipred_it_next(it))
	cu_call(f, cucon_po_ipred_it_get(it));
}

void
cucon_po_iter_isuccs(cucon_poelt_t e, cu_clop(f, void, cucon_poelt_t))
{
    cucon_po_isucc_it_t it;
    for (it = cucon_po_isucc_begin(e); it != cucon_po_isucc_end(e);
	 it = cucon_po_isucc_it_next(it))
	cu_call(f, cucon_po_isucc_it_get(it));
}

/* cucon_po_topological_succs
 * cucon_po_topological_preds
 * ------------------------ */

cu_clop_def(cucon_po_topological_prec, cu_bool_t,
	    cucon_poelt_t e0, cucon_poelt_t e1)
{
    return e0->level < e1->level;
}
cu_clop_def(cucon_po_topological_succ, cu_bool_t,
	    cucon_poelt_t e0, cucon_poelt_t e1)
{
    return e0->level < e1->level;
}

void
cucon_po_topological_succs(cucon_poelt_t e, cu_clop(f, void, cucon_poelt_t))
{
    struct cucon_priq_s priq;
    struct cucon_pmap_s processed;
    cucon_priq_init(&priq,
	(cu_clop(, cu_bool_t, void *, void *))cucon_po_topological_prec);
    cucon_pmap_init(&processed);
    for (;;) {
	cucon_po_isucc_it_t it;
	cu_call(f, e);
	for (it = cucon_po_isucc_begin(e); it != cucon_po_isucc_end(e);
	     it = cucon_po_isucc_it_next(it)) {
	    cucon_poelt_t elt = cucon_po_isucc_it_get(it);
	    if (cucon_pmap_insert_void(&processed, elt))
		cucon_priq_insert(&priq, elt);
	}
	if (cucon_priq_is_empty(&priq))
	    break;
	e = cucon_priq_pop_front(&priq);
    }
}

void
cucon_po_topological_preds(cucon_poelt_t e, cu_clop(f, void, cucon_poelt_t))
{
    struct cucon_priq_s priq;
    struct cucon_pmap_s processed;
    cucon_priq_init(&priq,
	(cu_clop(, cu_bool_t, void *, void *))cucon_po_topological_succ);
    cucon_pmap_init(&processed);
    for (;;) {
	cucon_po_ipred_it_t it;
	cu_call(f, e);
	for (it = cucon_po_ipred_begin(e); it != cucon_po_ipred_end(e);
	     it = cucon_po_ipred_it_next(it)) {
	    cucon_poelt_t elt = cucon_po_ipred_it_get(it);
	    if (cucon_pmap_insert_void(&processed, elt))
		cucon_priq_insert(&priq, elt);
	}
	if (cucon_priq_is_empty(&priq))
	    break;
	e = cucon_priq_pop_front(&priq);
    }
}


/* cucon_po_pruned_lspanning
 * ----------------------- */

static void
_po_pruned_lspanning_accu(cucon_poelt_t elt, cucon_pmap_t pruned_S,
			  cucon_pmap_t uspan)
{
    cucon_po_isucc_it_t it;
    for (it = cucon_po_isucc_begin(elt); it != cucon_po_isucc_end(elt);
	 it = cucon_po_isucc_it_next(it)) {
	cucon_poelt_t elt_succ = cucon_po_isucc_it_get(it);
	if (cucon_pmap_insert_void(uspan, elt_succ))
	    _po_pruned_lspanning_accu(elt_succ, pruned_S, uspan);
	else
	    cucon_pmap_erase(pruned_S, elt_succ);
    }
}

cu_clos_def(_po_pruned_lspanning_cb,
	    cu_prot(void, void const *elt),
	    ( cucon_pmap_t pruned_S;
	      struct cucon_pmap_s uspan; ))
{
#define elt ((cucon_poelt_t)elt)
    cu_clos_self(_po_pruned_lspanning_cb);
    if (cucon_pmap_insert_void(&self->uspan, elt)) {
	cucon_pmap_insert_void(self->pruned_S, elt);
	_po_pruned_lspanning_accu(elt, self->pruned_S, &self->uspan);
    }
#undef elt
}

cucon_pmap_t
cucon_po_pruned_lspanning(cucon_po_t po, cucon_pmap_t S)
{
    _po_pruned_lspanning_cb_t cb;
    cb.pruned_S = cucon_pmap_new();
    cucon_pmap_init(&cb.uspan);
    cucon_pmap_iter_keys(S, _po_pruned_lspanning_cb_prep(&cb));
    return cb.pruned_S;
}


/* cucon_po_conj_lspan
 * ----------------- */

static cu_bool_t
conj_lspan_helper(cucon_poelt_t elt,
		  cu_clop(cb, cu_bool_t, cucon_poelt_t elt),
		  cucon_pmap_t lspan)
{
    cucon_po_ipred_it_t it;
    if (!cucon_pmap_insert_void(lspan, elt))
	return cu_true;
    if (!cu_call(cb, elt))
	return cu_false;
    for (it = cucon_po_ipred_begin(elt);
	 it != cucon_po_ipred_end(elt);
	 it = cucon_po_ipred_it_next(it)) {
	cucon_poelt_t elt_pred = cucon_po_ipred_it_get(it);
	if (!conj_lspan_helper(elt_pred, cb, lspan))
	    return cu_false;
    }
    return cu_true;
}
cu_clos_def(_conj_lspan_pmap_cb,
	    cu_prot(cu_bool_t, void const *elt),
	    ( cu_clop(cb, cu_bool_t, cucon_poelt_t elt);
	      struct cucon_pmap_s lspan; ))
{
#define elt ((cucon_poelt_t)elt)
    cu_clos_self(_conj_lspan_pmap_cb);
    return conj_lspan_helper(elt, self->cb, &self->lspan);
#undef elt
}
cu_bool_t
cucon_po_conj_lspan(cucon_po_t po, cucon_pmap_t span,
		  cu_clop(cb, cu_bool_t, cucon_poelt_t elt))
{
    cu_bool_t r;
    _conj_lspan_pmap_cb_t pmap_cb;
    pmap_cb.cb = cb;
    cucon_pmap_init(&pmap_cb.lspan);
    r = cucon_pmap_conj_keys(span, _conj_lspan_pmap_cb_prep(&pmap_cb));
    return r;
}


/* cucon_po_lspan_accu, cucon_po_lspan, cucon_po_lspan_isecn
 * cucon_po_uspan_accu, cucon_po_uspan, cucon_po_uspan_isecn
 * ---------------------------------------------------- */

void
cucon_po_lspan_accu(cucon_poelt_t max, cucon_pmap_t accum)
{
    cucon_listnode_t it;
    if (!cucon_pmap_insert_void(accum, max))
	return;
    for (it = cucon_po_ipred_begin(max); it != cucon_po_ipred_end(max);
	 it = cucon_po_ipred_it_next(it))
	cucon_po_lspan_accu(cucon_po_ipred_it_get(it), accum);
}

void
cucon_po_uspan_accu(cucon_poelt_t min, cucon_pmap_t accum)
{
    cucon_listnode_t it;
    if (!cucon_pmap_insert_void(accum, min))
	return;
    for (it = cucon_po_isucc_begin(min); it != cucon_po_isucc_end(min);
	 it = cucon_po_isucc_it_next(it))
	cucon_po_uspan_accu(cucon_po_isucc_it_get(it), accum);
}

cucon_pmap_t
cucon_po_lspan(cucon_poelt_t max)
{
    cucon_pmap_t R = cucon_pmap_new();
    cucon_po_lspan_accu(max, R);
    return R;
}

cucon_pmap_t
cucon_po_uspan(cucon_poelt_t min)
{
    cucon_pmap_t R = cucon_pmap_new();
    cucon_po_uspan_accu(min, R);
    return R;
}

cucon_pmap_t
cucon_po_lspan_isecn(cucon_poelt_t max, cucon_pmap_t S)
{
    cucon_pmap_t R = cucon_po_lspan(max);
    cucon_pmap_assign_isecn(R, S);
    return R;
}

cucon_pmap_t
cucon_po_uspan_isecn(cucon_poelt_t min, cucon_pmap_t S)
{
    cucon_pmap_t R = cucon_po_uspan(min);
    cucon_pmap_assign_isecn(R, S);
    return R;
}

static cu_bool_t
_po_open_range_accu(cucon_poelt_t low, cucon_poelt_t high, cucon_pmap_t S,
		    cucon_pmap_t visit)
{
    cucon_listnode_t it;
    cu_bool_t low_in_range = cu_false;
    if (low->level >= high->level)
	return cu_true;
    for (it = cucon_po_isucc_begin(low); it != cucon_po_isucc_end(low);
	    it = cucon_po_isucc_it_next(it)) {
	cucon_poelt_t succ = cucon_listnode_ptr(it);
	cu_bool_t *succ_in_range;
	if (succ == high)
	    return cu_true;
	if (cucon_pmap_insert_mem(visit, succ, sizeof(cu_bool_t),
				  &succ_in_range))
	    *succ_in_range = _po_open_range_accu(succ, high, S, visit);
	if (*succ_in_range) {
	    low_in_range = cu_true;
	    cucon_pmap_insert_void(S, succ);
	}
    }
    return low_in_range;
}

cu_bool_t
cucon_po_open_range_accu(cucon_poelt_t low, cucon_poelt_t high,
			 cucon_pmap_t S)
{
    struct cucon_pmap_s visit;
    cucon_pmap_init(&visit);
    return _po_open_range_accu(low, high, S, &visit);
}

cucon_pmap_t
cucon_po_open_range(cucon_poelt_t low, cucon_poelt_t high)
{
    cucon_pmap_t S;
    struct cucon_pmap_s visit;
    S = cucon_pmap_new();
    cucon_pmap_init(&visit);
    _po_open_range_accu(low, high, S, &visit);
    return S;
}

static cu_bool_t
_po_closed_range_and_succs(cucon_poelt_t min, cucon_poelt_t max,
			   cucon_pmap_t range, cucon_pmap_t succs,
			   cucon_pmap_t cache)
{
    cucon_listnode_t it;
    cu_bool_t min_in_range;
    struct cucon_stack_s maybe_ins_succs, maybe_rm_succs;
    /* Can't use level here. */
    if (min == max) {
	cucon_pmap_insert_void(range, min);
	for (it = cucon_po_isucc_begin(min);
	     it != cucon_po_isucc_end(min); it = cucon_listnode_next(it)) {
	    cucon_poelt_t succ = cucon_listnode_ptr(it);
	    cu_bool_t *in_range;
	    if (cucon_pmap_insert_mem(cache, succ,
				      sizeof(cu_bool_t), &in_range)) {
		*in_range = cu_false;
		cucon_pmap_insert_void(succs, succ);
	    }
	}
	return cu_true;
    }
    cucon_stack_init(&maybe_ins_succs);
    cucon_stack_init(&maybe_rm_succs);
    min_in_range = cu_false;
    for (it = cucon_po_isucc_begin(min);
	 it != cucon_po_isucc_end(min); it = cucon_listnode_next(it)) {
	cucon_poelt_t succ = cucon_listnode_ptr(it);
	cu_bool_t *in_range;
	if (cucon_pmap_insert_mem(cache, succ, sizeof(cu_bool_t), &in_range)) {
	    if (_po_closed_range_and_succs(succ, max, range, succs, cache))
		*in_range = min_in_range = cu_true;
	    else {
		*in_range = cu_false;
		CUCON_STACK_PUSH(&maybe_ins_succs, cucon_poelt_t, succ);
	    }
	}
	else {
	    if (*in_range)
		min_in_range = cu_true;
	    else
		CUCON_STACK_PUSH(&maybe_rm_succs, cucon_poelt_t, succ);
	}
    }
    if (min_in_range) {
	cucon_pmap_insert_void(range, min);
	while (!cucon_stack_is_empty(&maybe_ins_succs)) {
	    cucon_pmap_insert_void(succs,
		    CUCON_STACK_TOP(&maybe_ins_succs, cucon_poelt_t));
	    CUCON_STACK_POP(&maybe_ins_succs, cucon_poelt_t);
	}
	return cu_true;
    }
    else {
	while (!cucon_stack_is_empty(&maybe_rm_succs)) {
	    cucon_pmap_erase(succs,
		    CUCON_STACK_TOP(&maybe_rm_succs, cucon_poelt_t));
	    CUCON_STACK_POP(&maybe_rm_succs, cucon_poelt_t);
	}
	return cu_false;
    }
}

cu_bool_t
cucon_po_closed_range_and_succs(cucon_poelt_t min, cucon_poelt_t max,
				cucon_pmap_t range, cucon_pmap_t succs)
{
    struct cucon_pmap_s cache;
    cucon_pmap_init(&cache);
    return _po_closed_range_and_succs(min, max, range, succs, &cache);
}


/* cucon_po_connected_prune_to_max
 * cucon_po_connected_prune_to_min
 * ----------------------------- */

void
cucon_po_connected_prune_to_max(cucon_pmap_t S, cucon_poelt_t elt)
{
    cucon_listnode_t it;
    cu_bool_t elt_is_max = cu_true;
    for (it = cucon_po_isucc_begin(elt); it != cucon_po_isucc_end(elt);
	 it = cucon_listnode_next(it)) {
	cucon_poelt_t succ = cucon_po_isucc_it_get(it);
	if (cucon_pmap_find_mem(S, succ)) {
	    cucon_po_connected_prune_to_max(S, succ);
	    elt_is_max = cu_false;
	}
    }
    if (!elt_is_max)
	cucon_pmap_erase(S, elt);
}

void
cucon_po_connected_prune_to_min(cucon_pmap_t S, cucon_poelt_t elt)
{
    cucon_listnode_t it;
    cu_bool_t elt_is_min = cu_true;
    for (it = cucon_po_ipred_begin(elt); it != cucon_po_ipred_end(elt);
	 it = cucon_listnode_next(it)) {
	cucon_poelt_t pred = cucon_po_ipred_it_get(it);
	if (cucon_pmap_find_mem(S, pred)) {
	    cucon_po_connected_prune_to_min(S, pred);
	    elt_is_min = cu_false;
	}
    }
    if (!elt_is_min)
	cucon_pmap_erase(S, elt);
}


/* cucon_po_inf_of_list
 * cucon_po_sup_of_list
 * ------------------ */

cucon_pmap_t
cucon_po_inf_of_list(cucon_list_t L)
{
    cucon_listnode_t it_L;
    cucon_pmap_t R;
    cu_debug_assert(!cucon_list_is_empty(L));
    it_L = cucon_list_begin(L);
    if (cucon_list_is_singleton(L)) {
	R = cucon_pmap_new();
	cucon_pmap_insert_void(R, cucon_listnode_ptr(it_L));
    }
    else {
	cucon_poelt_t elt0 = cucon_listnode_ptr(it_L);
	cucon_po_t po = elt0->po;
	R = cucon_po_lspan(elt0);
	while ((it_L = cucon_listnode_next(it_L)) != cucon_list_end(L))
	    R = cucon_po_lspan_isecn(cucon_listnode_ptr(it_L), R);
	cucon_po_connected_prune_to_max(R, cucon_po_bot(po));
    }
    return R;
}

cucon_pmap_t
cucon_po_sup_of_list(cucon_list_t L)
{
    cucon_listnode_t it_L;
    cucon_pmap_t R;
    cu_debug_assert(!cucon_list_is_empty(L));
    it_L = cucon_list_begin(L);
    if (cucon_list_is_singleton(L)) {
	R = cucon_pmap_new();
	cucon_pmap_insert_void(R, cucon_listnode_ptr(it_L));
    }
    else {
	cucon_poelt_t elt0 = cucon_listnode_ptr(it_L);
	cucon_po_t po = elt0->po;
	R = cucon_po_uspan(elt0);
	while ((it_L = cucon_listnode_next(it_L)) != cucon_list_end(L))
	    R = cucon_po_uspan_isecn(cucon_listnode_ptr(it_L), R);
	cucon_po_connected_prune_to_min(R, cucon_po_top(po));
    }
    return R;
}


/* cucon_po_ljunctions
 * ----------------- */

#if 0 /* XX trick, trick, analyse */
cucon_pmap_t
cucon_po_ljunctions(cucon_poelt_t e)
{
    struct cucon_priq_s Q;
    struct cucon_pmap_s reach_map;
    cucon_pmap_init(&reach_map);
    cucon_priq_init(&Q, cucon_po_topological_succ);
    cucon_priq_insert(&Q, e);
    while (!cucon_priq_is_empty(&Q)) {
	cucon_listnode_t it;
	e = cucon_priq_pop_front(&Q);
	if (cucon_list_is_empty(&e->ipreds))
	    continue;
	for (it = cucon_po_ipred_begin(e); it != cucon_po_ipred_end(e);
	     it = cucon_listnode_next(it)) {
	    cucon_pmap_t e_reach = cucon_pmap_find_ptr(&reach_map, e);
	    cucon_poelt_t pred = cucon_po_ipred_it_get(it);
	    cucon_pmap_t *pred_reach;
	    if (cucon_pmap_insert_mem(&reach_map, pred,
				    sizeof(cucon_pmap_t), &pred_reach)) {
		*pred_reach = e_reach;
		cucon_priq_insert(&Q, pred);
	    }
	    else {
		cucon_pmap_t S = cucon_pmap_new_copy_void(*pred_reach);
		cucon_pmap_t U = cucon_pmap_new_copy_void(e_reach);
		cucon_pmap_isecn_union(S, U);
		*pred_reach = U;
	    }
	}
    }
}

cucon_pmap_t
cucon_po_pairwise_inf_of_list(cucon_list_t L)
{
    size_t i;
    cucon_listnode_t it_L;
    size_t N = cucon_list_count(L);
    struct cucon_priq_s Q;
    cu_debug_assert(!cucon_list_is_empty(L) && !cucon_list_is_singleton(L));
    cucon_priq_init(&Q, cucon_po_topological_succ);
    i = 0;
    for (it_L = cucon_list_begin(L); it_L != cucon_list_end(L);
	 it_L = cucon_listnode_next(it_L)) {
	cucon_poelt_t e = cucon_listnode_ptr(L);
	cucon_bitvect_t tagvect;
	if (cucon_pmap_insert_mem(&elt_to_tagvect, e,
				sizeof(struct cucon_bitvect_s), &tagvect)) {
	    cucon_bitvect_init_fill(&tagvect, N);
	    cucon_bitvect_set_at(&tagvect, i, 1);
	    cucon_priq_insert(&Q, e);
	    ++i;
	}
	else
	    cu_debug_error("Two identical elements in argument.");
    }
    while (!cucon_priq_is_empty(&Q)) {
	cucon_poelt_t e = cucon_priq_pop_front(&Q);
	cucon_listnode_t it_P;
	for (it_P = cucon_po_ipred_begin(e); it_P != cucon_po_ipred_end(e);
	     it_P = cucon_listnode_next(it_P)) {
	    cucon_poelt_t e_P = cucon_po_ipred_it_get(it_P);
	    cucon_bitvect_t tagvect_P;
	    if (cucon_pmap_insert_mem(&elt_to_tagvect, e_P,
				    sizeof(struct cucon_bitvect_s); &tagvect_P)) {
		cucon_bitvect_init_copy(tagvect_P, tagvect);
		cucon_priq_insert(&Q, e_P);
	    }
	    else {
	    }
	}
    }
}
#endif

cucon_pmap_t
cucon_po_ucollect_reachable_ljunctions(cucon_poelt_t elt)
{
    struct cucon_priq_s Q;
    cucon_pmap_t R = cucon_pmap_new();
    cucon_pmap_t *ljset;
    cucon_priq_init(&Q,
	(cu_clop(, cu_bool_t, void *, void *))cucon_po_topological_prec);
    cucon_priq_insert(&Q, elt);
    cucon_pmap_insert_mem(R, elt, sizeof(cucon_pmap_t), &ljset);
    *ljset = cucon_pmap_new();
    while (!cucon_priq_is_empty(&Q)) {
	cucon_listnode_t it;
	cucon_pmap_t ljset0;
	elt = cucon_priq_pop_front(&Q);
	ljset0 = cucon_pmap_find_ptr(R, elt);
	if (cucon_list_is_empty_or_singleton(&elt->isuccs)) {
	    if (cucon_list_is_empty(&elt->isuccs))
		continue;
	}
	else {
	    ljset0 = cucon_pmap_new_copy_void(ljset0);
	    cucon_pmap_insert_void(ljset0, elt);
	}
	for (it = cucon_po_isucc_begin(elt); it != cucon_po_isucc_end(elt);
	     it = cucon_po_isucc_it_next(it)) {
	    cucon_poelt_t e_succ = cucon_po_isucc_it_get(it);
	    if (cucon_pmap_insert_mem(R, e_succ, sizeof(cucon_pmap_t), &ljset)) {
		*ljset = ljset0;
		cucon_priq_insert(&Q, e_succ);
	    }
	    else {
		cucon_pmap_t S = *ljset;
		size_t size_S = cucon_pmap_size(S);
		S = cucon_pmap_new_copy_void(S);
		cucon_pmap_assign_union_void(S, ljset0);
		if (cucon_pmap_size(S) > size_S) {
		    cucon_priq_insert(&Q, e_succ);
		    *ljset = S;
		}
	    }
	}
    }
    return R;
}

/* cucon_po_write_gviz
 * ----------------- */

cu_clos_def(_print_gviz_cb,
	    cu_prot(void, cucon_poelt_t elt),
	    ( FILE *out;
	      cu_clop(label, cu_str_t, cucon_poelt_t); ))
{
    cu_clos_self(_print_gviz_cb);
    cucon_listnode_t it;
    fprintf(self->out, "v%p[label=\"%s\"];\n", (void *)elt,
	    cu_str_to_cstr(cu_call(self->label, elt)));
    for (it = cucon_po_ipred_begin(elt); it != cucon_po_ipred_end(elt);
	 it = cucon_listnode_next(it)) {
	cucon_poelt_t elt_pred = cucon_po_ipred_it_get(it);
	fprintf(self->out, "v%p -> v%p;\n", (void *)elt, (void *)elt_pred);
    }
}
void
cucon_po_print_gviz(cucon_po_t po, cu_clop(label, cu_str_t, cucon_poelt_t),
		    FILE *out)
{
    _print_gviz_cb_t cb;
    cb.out = out;
    cb.label = label;
    fprintf(out, "digraph po {\n");
    cucon_po_topological_preds(cucon_po_top(po), _print_gviz_cb_prep(&cb));
    fprintf(out, "}\n");
}


/* Debugging
 * --------- */

cu_clos_def(_count_connections_cb,
	    cu_prot(void, cucon_poelt_t elt),
	    ( size_t cnt_succ;
	      size_t cnt_pred; ))
{
    cu_clos_self(_count_connections_cb);
    self->cnt_succ += cucon_list_count(&elt->isuccs);
    self->cnt_pred += cucon_list_count(&elt->ipreds);
/*     return cu_true; */
}

size_t
cucon_po_debug_count_connections(cucon_po_t po)
{
    _count_connections_cb_t cb;
    cucon_pmap_t S = cucon_pmap_new();
    cb.cnt_succ = 0;
    cb.cnt_pred = 0;
    cucon_pmap_insert_void(S, cucon_po_top(po));
/*     cucon_po_conj_lspan(po, S, cu_clos_ref(&cb)); */
    cucon_po_topological_preds(cucon_po_top(po),
			       _count_connections_cb_prep(&cb));
    cu_debug_assert(cb.cnt_succ == cb.cnt_pred);
    return cb.cnt_succ;
}

cu_clos_def(_check_links_cb,
	    cu_prot(void, cucon_poelt_t elt),
	    ( struct cucon_pmap_s uspan;
	      cucon_poelt_t elt_center; ))
{
    cu_clos_self(_check_links_cb);
    cucon_listnode_t it;
    if (elt == self->elt_center)
	return;
    for (it = cucon_po_isucc_begin(elt); it != cucon_po_isucc_end(elt);
	 it = cucon_listnode_next(it)) {
	cucon_poelt_t elt_succ = cucon_po_isucc_it_get(it);
/* 	cu_debug_assert(!cucon_pmap_find_mem(&self->uspan, elt_succ)); */
	if (cucon_pmap_find_mem(&self->uspan, elt_succ)) {
	    FILE *out = fopen("tmp.debug.gviz", "w");
	    if (out) {
		cucon_po_debug_dump_gviz(elt->po, out);
		fclose(out);
		cu_bugf("Redundant link in graph. "
			 "Graph dumped to tmp.debug.gviz.");
	    }
	    else
		cu_bugf("Redundant link in graph. "
			 "Could not dump graph.");
	}
    }
}
void
cucon_po_debug_check_nonredundant(cucon_poelt_t elt)
{
    _check_links_cb_t cb;
    cucon_pmap_init(&cb.uspan);
    _po_prec_collect_uspan(elt, cucon_po_bot(elt->po), &cb.uspan);
    cb.elt_center = elt;
    cucon_po_topological_preds(elt, _check_links_cb_prep(&cb));
}

cu_clos_def(_dump_gviz_cb,
	    cu_prot(void, cucon_poelt_t elt),
	    ( FILE *out; ))
{
    cu_clos_self(_dump_gviz_cb);
    cucon_listnode_t it;
    fprintf(self->out, "v%p[label=\"%d\"];\n", (void *)elt, elt->level);
    for (it = cucon_po_ipred_begin(elt); it != cucon_po_ipred_end(elt);
	 it = cucon_listnode_next(it)) {
	cucon_poelt_t elt_pred = cucon_po_ipred_it_get(it);
	fprintf(self->out, "v%p -> v%p;\n", (void *)elt, (void *)elt_pred);
    }
}
void
cucon_po_debug_dump_gviz(cucon_po_t po, FILE *out)
{
    _dump_gviz_cb_t cb;
    cb.out = out;
    fprintf(out, "digraph po {\n");
    cucon_po_topological_preds(cucon_po_top(po), _dump_gviz_cb_prep(&cb));
    fprintf(out, "}\n");
}
