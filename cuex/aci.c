/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2005--2007  Petter Urkedal <urkedal@nbi.dk>
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

#define ACI_SPILL_LINKS 0
#ifndef NDEBUG
#  define CUEX_ACI_DEBUG_SELF 1
#endif

#include <cuex/aci_priv.h>
#include <cu/algo.h>
#if !ACI_SPILL_LINKS
#include <cucon/priq.h>
#endif

#include <stdarg.h>
#include <limits.h>

#ifdef NDEBUG
#  define CUEX_ACI_NDEBUG
#endif

typedef enum {
    aci_on_conflict_null,
    aci_on_conflict_replace
} aci_on_conflict_t;

CU_SINLINE cuex_opn_t
aci_node(cuex_opn_t left, cuex_opn_t right,
	 cuex_meta_t elt_opr, cuex_t *elt_tpl)
{
    cu_rank_t i, r = cuex_opr_r(elt_opr);
    cuex_t *arr = cu_salloc(r*sizeof(cuex_t));
#if CUEX_ACI_DEBUG_SELF
    cu_debug_assert(left && right);
    cu_debug_assert(!aci_is_idy(left) || aci_is_idy(right));
    cu_debug_assert(aci_is_idy(left)  || (aci_key_t)elt_tpl[2] != aci_key(left));
    cu_debug_assert(aci_is_idy(right) || (aci_key_t)elt_tpl[2] != aci_key(right));
    if (!aci_is_idy(left)) {
	aci_key_t vkey = aci_key_clim((aci_key_t)elt_tpl[2], aci_key(left));
	cu_debug_assert(aci_key_covers(vkey, aci_key(left)));
	cu_debug_assert(aci_is_idy(aci_left(left))
			|| aci_key_covers(vkey, aci_key(aci_left(left))));
	cu_debug_assert(aci_is_idy(right)
			|| aci_key_covers(vkey, aci_key(right)));
    }
#endif
    arr[0] = left;
    arr[1] = right;
    for (i = 2; i < r; ++i)
	arr[i] = elt_tpl[i];
    return cuex_opn_by_arr(elt_opr, arr);
}

#if CUEX_ACI_DEBUG_SELF
static void
aci_debug_check_cover(aci_key_t vkey, cuex_opn_t tree)
{
    if (!aci_is_idy(tree)) {
	cuex_opn_t l = aci_left(tree);
	cuex_opn_t r = aci_right(tree);
	if (!aci_is_idy(l)) {
	    aci_key_t my_vkey = aci_key_clim(aci_key(tree), aci_key(l));
	    cu_debug_assert(vkey != my_vkey);
	    cu_debug_assert(aci_key_covers(vkey, my_vkey));
	    aci_debug_check_cover(my_vkey, l);
	    aci_debug_check_cover(my_vkey, r);
	}
    }
}
#endif

/* Update the left and right links of old. */
#define aci_update cuex_aci_rebranch
cuex_opn_t
cuex_aci_rebranch(cuex_opn_t g, cuex_opn_t new_left, cuex_opn_t new_right)
{
    cuex_meta_t opr = cuex_meta(g);
    cu_rank_t r = cuex_opr_r(opr);
    cuex_t *arr = cu_salloc(sizeof(cuex_t)*r);
    cuex_t *g_arr = cuex_opn_begin(g);
    cu_rank_t i;
#if CUEX_ACI_DEBUG_SELF
    aci_key_t vkey;
    cu_debug_assert(new_left && new_right);
    cu_debug_assert(!aci_is_idy(new_left) || aci_is_idy(new_right));
    cu_debug_assert(aci_is_idy(new_left)  || aci_key(g) != aci_key(new_left));
    cu_debug_assert(aci_is_idy(new_right) || aci_key(g) != aci_key(new_right));
    if (!aci_is_idy(new_left)) {
	vkey = aci_key_clim(aci_key(g), aci_key(new_left));
	cu_debug_assert(aci_key_covers(vkey, aci_key(new_left)));
	cu_debug_assert(aci_is_idy(aci_left(new_left))
			|| aci_key_covers(vkey, aci_key(aci_left(new_left))));
	cu_debug_assert(aci_is_idy(new_right)
			|| aci_key_covers(vkey, aci_key(new_right)));
	//aci_debug_check_cover(vkey, new_left);
	//aci_debug_check_cover(vkey, new_right);
    }
    cu_debug_assert(aci_is_idy(new_right) || aci_is_idy(aci_left(new_right))
		    || aci_key(g) != aci_key(aci_left(new_right)));
    cu_debug_assert(aci_is_idy(new_left)
		    || key_lt(aci_key(new_left), aci_key(g)));
    cu_debug_assert(aci_is_idy(new_right)
		    || key_gt(aci_key(new_right), aci_key(g)));
#endif
    arr[0] = new_left;
    arr[1] = new_right;
    for (i = 2; i < r; ++i)
	arr[i] = g_arr[i];
    return cuex_opn_by_arr(opr, arr);
}

cuex_opn_t
cuex_aci_find(cuex_opn_t e, cuex_t key)
{
    aci_key_t e_key;
    while (!aci_is_idy(e)) {
	e_key = aci_key(e);
	if (key_eq(e_key, (aci_key_t)key))
	    return e;
	if (key_lt((aci_key_t)key, e_key))
	    e = aci_left(e);
	else
	    e = aci_right(e);
    }
    return NULL;
}

cuex_opn_t
cuex_aci_generator(cuex_meta_t join, cuex_t key, ...)
{
    va_list vl;
    cu_rank_t i, r = cuex_opr_r(join);
    cuex_t *opd_arr = cu_salloc(sizeof(cuex_t)*r);
    opd_arr[0] = opd_arr[1] = aci_idy(join);
    opd_arr[2] = key;
    va_start(vl, key);
    for (i = 3; i < r; ++i)
	opd_arr[i] = va_arg(vl, cuex_t);
    va_end(vl);
    return cuex_opn_by_arr(join, opd_arr);
}

static cuex_opn_t
aci_insert(cuex_opn_t, cuex_meta_t, cuex_t *, aci_on_conflict_t);

static cuex_opn_t
aci_remove_min(cuex_opn_t e, cuex_opn_t *min_out)
{
    cuex_opn_t e_left = aci_left(e);
    cuex_opn_t e_right = aci_right(e);
    cu_debug_assert(e_left != e);
    if (aci_is_idy(e_left)) {
	*min_out = e;
	return e_right;
    }
    else {
	e_left = aci_remove_min(e_left, min_out);
	if (aci_is_idy(e_left) && !aci_is_idy(e_right))
	    return aci_insert(e_right, cuex_meta(e), cuex_opn_begin(e),
			      aci_on_conflict_null);
	else
	    return aci_update(e, e_left, e_right);
    }
}

static cuex_opn_t
aci_insert(cuex_opn_t e, cuex_meta_t elt_opr, cuex_t *elt_tpl,
	   aci_on_conflict_t on_conflict)
{
    aci_key_t key;
    aci_key_t e_key;
    aci_key_t e_vkey;
    if (aci_is_idy(e))
	return aci_node(e, e, elt_opr, elt_tpl);
    key = (aci_key_t)elt_tpl[2];
    e_key = aci_key(e);
    if (key_eq(e_key, key)) {
	cu_rank_t i;
	cu_rank_t r;
	r = cuex_opr_r(elt_opr);
	for (i = aci_val_start; i < r; ++i)
	    if (cuex_opn_at(e, i) != elt_tpl[i])
		switch (on_conflict) {
		    case aci_on_conflict_null:
			return NULL;
		    case aci_on_conflict_replace:
			return aci_node(aci_left(e), aci_right(e),
					elt_opr, elt_tpl);
		}
	return e;
    }
    if (aci_is_idy(aci_left(e))) {
	cuex_t idy = aci_right(e);
	cu_debug_assert(aci_is_idy(idy));
	if (key_lt(key, e_key))
	    return aci_update(e, aci_node(idy, idy, elt_opr, elt_tpl), idy);
	else
	    return aci_node(e, idy, elt_opr, elt_tpl);
    }
    e_vkey = aci_key_clim(aci_key(aci_left(e)), e_key);
    if (aci_key_covers(e_vkey, key)) {
	if (key_lt(key, e_vkey)) {
	    cuex_t new_left;
	    new_left = aci_insert(aci_left(e), elt_opr, elt_tpl, on_conflict);
	    if (new_left != aci_left(e))
		return aci_update(e, new_left, aci_right(e));
	    else
		return e;
	}
	else if (key_lt(key, e_key)) {
	    cuex_t new_right;
	    new_right = aci_insert(aci_right(e), cuex_meta(e),
				   cuex_opn_begin(e), on_conflict);
	    return aci_node(aci_left(e), new_right, elt_opr, elt_tpl);
	}
	else {
	    cuex_t new_right;
	    new_right = aci_insert(aci_right(e), elt_opr, elt_tpl,
				   on_conflict);
	    if (new_right != aci_right(e))
		return aci_update(e, aci_left(e), new_right);
	    else
		return e;
	}
    }
    if (key_lt(key, e_vkey)) {
	cuex_opn_t min;
	cuex_opn_t new_right = aci_remove_min(e, &min);
	cuex_opn_t idy = aci_idy(elt_opr);
	cuex_opn_t new_left = aci_node(idy, idy, elt_opr, elt_tpl);
	return aci_node(new_left, new_right, cuex_meta(min),
			cuex_opn_begin(min));
    }
    else
	return aci_node(e, aci_idy(elt_opr), elt_opr, elt_tpl);
}


/* Merging
 * ======= */

typedef struct aci_merge_state_s *aci_merge_state_t;
typedef struct aci_spill_link_s *aci_spill_link_t;
struct aci_spill_link_s
{
    cuex_opn_t g;
    aci_spill_link_t next;
};
struct aci_merge_state_s
{
    cuex_opn_t idy;
    cuex_aci_merge_fn_t merge;
#if ACI_SPILL_LINKS
    aci_spill_link_t spill_first;
    aci_spill_link_t spill_last;
#else
    struct cucon_priq_s spill;
#endif
    cuex_opn_t *get_min;
};

cu_clop_def(spill_prior_clop, cu_bool_t, cuex_opn_t g0, cuex_opn_t g1)
{
    return key_leq(aci_key(g0), aci_key(g1));
}

static void
aci_merge_state_cct(aci_merge_state_t st, cuex_meta_t join,
		    cuex_aci_merge_fn_t merge)
{
    st->idy = cuex_aci_identity(join);
    st->merge = merge;
#if ACI_SPILL_LINKS
    st->spill_first = NULL;
    st->spill_last = NULL;
#else
    cucon_priq_cct(&st->spill,
		   (cu_clop(, cu_bool_t, void *, void *))spill_prior_clop);
#endif
    st->get_min = NULL;
}

CU_SINLINE cu_bool_t
aci_have_spill(aci_merge_state_t st, aci_key_t rlim)
{
#if ACI_SPILL_LINKS
    return (st->spill_first && aci_key(st->spill_first->g) >= rlim);
#else
    return !cucon_priq_is_empty(&st->spill)
	&& key_lt(aci_key(cucon_priq_front(&st->spill)), rlim);
#endif
}

static cu_bool_t
aci_spill_push(aci_merge_state_t st, aci_spill_link_t sl, cuex_opn_t g)
{
#if ACI_SPILL_LINKS
    cu_debug_assert(!aci_is_idy(g));
    if (st->spill_last) {
	cu_debug_assert(st->spill_last->g);
	if (key_eq(aci_key(g), aci_key(st->spill_last->g))) {
	    g = cu_call(st->merge, st->idy, st->idy, g, st->spill_last->g);
	    st->spill_last->g = g;
	    if (!g)
		cu_bugf("unimplemented");
	    sl->g = NULL;
	    return cu_true;
	}
	else {
	    cu_debug_assert(key_gt(aci_key(g), aci_key(st->spill_last->g)));
	    st->spill_last->next = sl;
	}
    }
    else
	st->spill_first = sl;
    sl->g = aci_update(g, st->idy, st->idy);
    sl->next = NULL;
    st->spill_last = sl;
    return cu_true;
#else
    cucon_priq_insert(&st->spill, g);
    return cu_true;
#endif
}

#if ACI_SPILL_LINKS
static cu_bool_t
aci_spill_insert(aci_merge_state_t st, aci_spill_link_t sl, cuex_opn_t g)
{
#if 1
    aci_spill_link_t *nextptr = &st->spill_first;
    while (*nextptr && key_gt(aci_key(g), aci_key((*nextptr)->g)))
	nextptr = &(*nextptr)->next;
    if (*nextptr && key_eq(aci_key(g), aci_key((*nextptr)->g))) {
	g = cu_call(st->merge, st->idy, st->idy, (*nextptr)->g, g);
	if (!g)
	    return cu_false;
	(*nextptr)->g = g;
	sl->g = NULL;
	return cu_true;
    }
    sl->next = *nextptr;
    *nextptr = sl;
    if (!sl->next)
	st->spill_last = sl;
#else
    sl->next = st->spill_first;
    st->spill_first = sl;
    if (!st->spill_last)
	st->spill_last = sl;
#endif
    sl->g = aci_update(g, st->idy, st->idy);
    return cu_true;
}
#else
#define aci_spill_insert aci_spill_push
#endif

CU_SINLINE void
aci_spill_unpush(aci_merge_state_t st, aci_spill_link_t sl)
{
#if ACI_SPILL_LINKS
    cu_debug_assert(!sl->g);
#endif
}

CU_SINLINE cuex_opn_t
aci_spill_pop(aci_merge_state_t st, aci_key_t rlim)
{
#if ACI_SPILL_LINKS
    aci_spill_link_t head = st->spill_first;
    if (head) {
	cuex_opn_t g = head->g;
	cu_debug_assert(g);
	if (key_lt(aci_key(g), rlim)) {
	    head->g = NULL;
	    if (head->next)
		st->spill_first = head->next;
	    else {
		st->spill_first = NULL;
		st->spill_last = NULL;
	    }
	    return g;
	}
    }
    return NULL;
#else
    cuex_opn_t g;
    if (cucon_priq_is_empty(&st->spill))
	return NULL;
    g = cucon_priq_front(&st->spill);
    if (aci_key(g) >= rlim)
	return NULL;
    cucon_priq_pop_front(&st->spill);
    while (!cucon_priq_is_empty(&st->spill)
	   && key_eq(aci_key(g), aci_key(cucon_priq_front(&st->spill)))) {
	g = cu_call(st->merge, st->idy, st->idy, g,
		    cucon_priq_pop_front(&st->spill));
	if (!g)
	    cu_bugf("unimplemented");
    }
    return aci_update(g, st->idy, st->idy);
#endif
}
CU_SINLINE cuex_opn_t
aci_spill_pop_leq(aci_merge_state_t st, aci_key_t rlim)
{ return aci_spill_pop(st, rlim + 1); }

static cuex_opn_t
aci_merge_0t(aci_merge_state_t st, aci_key_t rlim)
{
    cuex_opn_t left = aci_spill_pop(st, rlim);
    if (st->get_min && left) {
	*st->get_min = left;
	st->get_min = NULL;
	left = aci_spill_pop(st, rlim);
    }
    if (left) {
	cuex_opn_t g;
	while ((g = aci_spill_pop(st, rlim)) != NULL) {
	    cuex_opn_t right;
	    aci_key_t sub_rlim = aci_key_rlim(aci_key(g), aci_key(left));
	    cu_debug_assert(key_leq(sub_rlim, rlim));
	    right = aci_merge_0t(st, sub_rlim);
	    left = aci_update(g, left, right);
	}
	return left;
    }
    else
	return st->idy;
}

static cuex_opn_t
aci_merge_1t(aci_merge_state_t st, aci_key_t rlim, cuex_opn_t t0)
{
    cuex_opn_t g;
    aci_key_t llim0;
    cuex_opn_t left;
    cuex_opn_t right;
    if (!st->get_min && !aci_have_spill(st, rlim))
	return t0;
    if (aci_is_idy(t0))
	return aci_merge_0t(st, rlim);
    if (aci_is_idy(aci_left(t0))) {
	llim0 = aci_tree_llim(t0);
	left = aci_spill_pop_leq(st, aci_key(t0));
	if (left) {
	    if (key_eq(aci_key(left), aci_key(t0))) {
		left = cu_call(st->merge, st->idy, st->idy, t0, left);
		if (!left)
		    return NULL;
		else if (st->get_min) {
		    *st->get_min = left;
		    st->get_min = NULL;
		    return aci_merge_0t(st, rlim);
		}
		else
		    goto pop_rest;
	    }
	    /* else fall through */
	}
	else {
	    if (st->get_min) {
		*st->get_min = t0;
		st->get_min = NULL;
		return aci_merge_0t(st, rlim);
	    }
	    else {
		left = t0;
		goto pop_rest;
	    }
	}
    }
    else {
	llim0 = aci_tree_llim(t0);
	left = aci_spill_pop(st, llim0);
    }

    if (left && st->get_min) {
	*st->get_min = left;
	st->get_min = NULL;
	left = aci_spill_pop(st, llim0);
    }
    if (left) {
	aci_key_t sub_rlim;
	cu_debug_assert(aci_is_idy(aci_left(left)));
	while ((g = aci_spill_pop(st, llim0)) != NULL) {
	    sub_rlim = aci_key_rlim(aci_key(g), aci_key(left));
	    cu_debug_assert(key_leq(sub_rlim, rlim));
	    if (key_lt(llim0, sub_rlim)) {
		right = aci_merge_1t(st, sub_rlim, t0);
		left = aci_update(g, left, right);
		goto pop_rest;
	    }
	    right = aci_merge_0t(st, sub_rlim);
	    left = aci_update(g, left, right);
	}
	st->get_min = &g;
	sub_rlim = aci_key_rlim(aci_key(t0), aci_key(left));
	right = aci_merge_1t(st, sub_rlim, t0);
	left = aci_update(g, left, right);
    }
    else {
	aci_key_t clim0;
	aci_key_t rlim0 = aci_tree_rlim(t0);
	struct aci_spill_link_s sl;
	if (!st->get_min && !aci_have_spill(st, rlim0)) {
	    left = t0;
	    goto pop_rest;
	}
	clim0 = aci_tree_clim(t0);
	left = aci_merge_1t(st, clim0, aci_left(t0));
	g = aci_spill_pop_leq(st, aci_key(t0));
	if (aci_is_idy(left)) {
	    struct aci_spill_link_s sl;
	    if (g) {
		if (key_eq(aci_key(g), aci_key(t0))) {
		    g = cu_call(st->merge, st->idy, st->idy, t0, g);
		    if (!g) return NULL;
		}
		else
		    aci_spill_insert(st, &sl, t0);
		aci_spill_insert(st, &sl, g);
	    }
	    else
		aci_spill_insert(st, &sl, t0);
	    left = aci_merge_1t(st, rlim0, aci_right(t0));
	    goto pop_rest;
	}
	if (g) {
	    if (key_eq(aci_key(g), aci_key(t0))) {
		g = cu_call(st->merge, st->idy, st->idy, t0, g);
		if (!g) return NULL;
		right = aci_merge_1t(st, rlim0, aci_right(t0));
		left = aci_update(g, left, right);
	    }
	    else {
		cu_debug_assert(key_geq(aci_key(g), clim0));
		aci_spill_push(st, &sl, t0);
		right = aci_merge_1t(st, rlim0, aci_right(t0));
		aci_spill_unpush(st, &sl);
		left = aci_update(g, left, right);
	    }
	}
	else {
	    g = t0;
	    cu_debug_assert(st->get_min == NULL);
	    right = aci_merge_1t(st, rlim0, aci_right(t0));
	    left = aci_update(g, left, right);
	}
    }
pop_rest:
    cu_debug_assert(!st->get_min);
    while ((g = aci_spill_pop(st, rlim)) != NULL) {
	aci_key_t rlim0 = aci_key_rlim(aci_key(g), aci_key(left));
	cu_debug_assert(key_leq(rlim0, rlim));
	right = aci_merge_0t(st, rlim0);
	left = aci_update(g, left, right);
    }
    return left;
}

static cuex_opn_t
aci_merge_2t(aci_merge_state_t st, aci_key_t rlim, cuex_opn_t t0, cuex_opn_t t1)
{
    cuex_opn_t g;
    aci_key_t llim0;
    aci_key_t llim1;
    aci_key_t llim01;
    cuex_opn_t left, right;
    if (aci_is_idy(t0))
	return aci_merge_1t(st, rlim, t1);
    if (aci_is_idy(t1))
	return aci_merge_1t(st, rlim, t0);
    cu_debug_assert(key_lt(aci_key(t0), rlim));
    cu_debug_assert(key_lt(aci_key(t1), rlim));
    if (aci_is_idy(aci_left(t0))) {
	struct aci_spill_link_s sl;
	aci_spill_push(st, &sl, t0);
	t1 = aci_merge_1t(st, rlim, t1);
	aci_spill_unpush(st, &sl);
	return t1;
    }
    if (aci_is_idy(aci_left(t1))) {
	struct aci_spill_link_s sl;
	aci_spill_push(st, &sl, t1);
	t0 = aci_merge_1t(st, rlim, t0);
	aci_spill_unpush(st, &sl);
	return t0;
    }
    llim0 = aci_tree_llim(t0);
    llim1 = aci_tree_llim(t1);
    llim01 = key_min(llim0, llim1);
    left = aci_spill_pop(st, llim01);
    if (st->get_min && left) {
	*st->get_min = left;
	left = aci_spill_pop(st, llim01);
	st->get_min = NULL;
    }
    if (left) {
	aci_key_t rlim0, rlim1;
	while ((g = aci_spill_pop(st, llim01)) != NULL) {
	    aci_key_t sub_rlim = aci_key_rlim(aci_key(g), aci_key(left));
	    cu_debug_assert(key_leq(sub_rlim, rlim));
	    if (key_lt(llim01, sub_rlim)) {
		if (llim1 >= sub_rlim) {	/* only t0 covered */
		    cu_debug_assert(t0);
		    right = aci_merge_1t(st, sub_rlim, t0);
		    left = aci_update(g, left, right);
		    llim0 = rlim;
		    llim01 = llim1;
		    t0 = NULL;
		    if (!t1)
			goto pop_rest;
		}
		else if (llim0 >= sub_rlim) {	/* only t1 covered */
		    cu_debug_assert(t1);
		    right = aci_merge_1t(st, sub_rlim, t1);
		    left = aci_update(g, left, right);
		    llim1 = rlim;
		    llim01 = llim0;
		    t1 = NULL;
		    if (!t0)
			goto pop_rest;
		}
		else {
		    right = aci_merge_2t(st, sub_rlim, t0, t1);
		    left = aci_update(g, left, right);
		    goto pop_rest;
		}
	    }
	    else {
		right = aci_merge_0t(st, sub_rlim);
		left = aci_update(g, left, right);
	    }
	}
	if (t0 && t1) {
	    rlim0 = aci_key_rlim(aci_key(left), aci_key(t0));
	    rlim1 = aci_key_rlim(aci_key(left), aci_key(t1));
	    if (key_lt(rlim1, rlim0)) {
		cu_ptr_swap(&t0, &t1);
		cu_ptr_swap(&rlim0, &rlim1);
	    }
	    if (key_lt(rlim0, rlim1)) {
		st->get_min = &g;
		right = aci_merge_1t(st, rlim0, t0);
		left = aci_update(g, left, right);
		st->get_min = &g;
		right = aci_merge_1t(st, rlim1, t1);
		left = aci_update(g, left, right);
	    }
	    else {
		st->get_min = &g;
		right = aci_merge_2t(st, rlim0, t0, t1);
		left = aci_update(g, left, right);
	    }
	}
	else {
	    if (!t0) {
		cu_debug_assert(t1);
		t0 = t1;
	    }
	    rlim0 = aci_key_rlim(aci_key(left), aci_key(t0));
	    right = aci_merge_1t(st, rlim0, t0);
	    left = aci_update(g, left, right);
	}
    }
    else {
	aci_key_t clim0 = aci_tree_clim(t0);
	aci_key_t clim1 = aci_tree_clim(t1);
	struct aci_spill_link_s sl0, sl1;
	if (key_eq(clim0, clim1)) {
	    aci_key_t rlim0 = aci_tree_rlim(t0);
	    left = aci_merge_2t(st, clim0, aci_left(t0), aci_left(t1));
	    if (aci_is_idy(left)) {
		if (key_lt(aci_key(t0), aci_key(t1))) {
		    aci_spill_push(st, &sl0, t0);
		    aci_spill_push(st, &sl1, t1);
		}
		else {
		    aci_spill_push(st, &sl1, t1);
		    aci_spill_push(st, &sl0, t0);
		}
		left = aci_merge_2t(st, rlim0, aci_right(t0), aci_right(t1));
		aci_spill_unpush(st, &sl1);
		aci_spill_unpush(st, &sl0);
		return left;
	    }
	    st->get_min = &g;
	    if (key_eq(aci_key(t0), aci_key(t1))) {
		g = cu_call(st->merge, st->idy, st->idy, t0, t1);
		if (!g)
		    return NULL;
		cu_debug_assert(key_lt(aci_key(g), rlim0));
		aci_spill_push(st, &sl0, g);
		right = aci_merge_2t(st, rlim0, aci_right(t0), aci_right(t1));
		aci_spill_unpush(st, &sl0);
	    }
	    else if (key_lt(aci_key(t0), aci_key(t1))) {
		cuex_t t0p, t1p;
		aci_spill_push(st, &sl0, t0);
		aci_spill_push(st, &sl1, t1);
		right = aci_merge_2t(st, rlim0, aci_right(t0), aci_right(t1));
//		t1p = aci_spill_pop(st, rlim);
//		t0p = aci_spill_pop(st, rlim);
//		cu_debug_assert(t0p == t0 && t1p == t1);
	    }
	    else {
		cuex_t t0p, t1p;
		aci_spill_push(st, &sl1, t1);
		aci_spill_push(st, &sl0, t0);
		right = aci_merge_2t(st, rlim0, aci_right(t0), aci_right(t1));
//		t1p = aci_spill_pop(st, rlim);
//		t0p = aci_spill_pop(st, rlim);
//		cu_debug_assert(t0p == t0 && t1p == t1);
	    }
	}
	else {
	    if (key_lt(clim1, clim0)) {
		cu_ptr_swap(&t0, &t1);
		cu_ptr_swap(&clim0, &clim1);
	    }
	    if (aci_key_covers(clim0, clim1)) {
		aci_key_t rlim0 = aci_tree_rlim(t0);
		left = aci_merge_1t(st, clim0, aci_left(t0));
		if (aci_is_idy(left)) {
		    aci_spill_push(st, &sl0, t0);
		    t0 = aci_merge_2t(st, rlim, aci_right(t0), t1);
		    aci_spill_unpush(st, &sl0);
		    return t0;
		}
		st->get_min = &g;
		aci_spill_push(st, &sl0, t0);
		right = aci_merge_2t(st, rlim0, aci_right(t0), t1);
		aci_spill_unpush(st, &sl0);
	    }
	    else if (aci_key_covers(clim1, clim0)) {
		aci_key_t rlim1 = aci_tree_rlim(t1);
		left = aci_merge_2t(st, clim1, t0, aci_left(t1));
		if (aci_is_idy(left)) {
		    aci_spill_push(st, &sl1, t1);
		    t0 = aci_merge_1t(st, rlim, aci_right(t1));
		    aci_spill_unpush(st, &sl1);
		    return t0;
		}
		st->get_min = &g;
		aci_spill_push(st, &sl1, t1);
		right = aci_merge_1t(st, rlim1, aci_right(t1));
		aci_spill_unpush(st, &sl1);
	    }
	    else {
		aci_key_t clim01 = aci_key_clim(aci_key(t0), aci_key(t1));
		aci_key_t rlim01;
		left = aci_merge_1t(st, clim01, t0);
		if (aci_is_idy(left))
		    return aci_merge_1t(st, rlim, t1);
		st->get_min = &g;
		rlim01 = aci_key_rlim(aci_key(left), aci_key(t1));
		right = aci_merge_1t(st, rlim01, t1);
	    }
	}
#if 1
	aci_key_t vkey = aci_key_clim(aci_key(g), aci_key(left));
	if (!(aci_is_idy(right) || aci_key_covers(vkey, aci_key(right)))) {
	    cu_fprintf(stdout, "\n%p %p %! %! %!\n", vkey, g, aci_key(right), left, right);
	    abort();
	}
#endif
	cu_debug_assert(!aci_is_idy(left) || aci_is_idy(right));
	left = aci_update(g, left, right);
    }
pop_rest:
    while ((g = aci_spill_pop(st, rlim)) != NULL) {
	aci_key_t rlim0 = aci_key_rlim(aci_key(g), aci_key(left));
	cu_debug_assert(key_leq(rlim0, rlim));
	right = aci_merge_0t(st, rlim0);
	left = aci_update(g, left, right);
    }
    return left;
}


/* Subset Tests
 * ============ */

cu_clop_def(aci_leq_priq_prior_clop, cu_bool_t, aci_key_t xk, aci_key_t yk)
{
    return key_lt(xk, yk);
}

CU_SINLINE void
aci_leq_priq_cct(cucon_priq_t q)
{
    cucon_priq_cct(q,
	    (cu_clop(, cu_bool_t, void *, void *))aci_leq_priq_prior_clop);
}

static cu_bool_t
aci_leq_0t(cucon_priq_t xq, cucon_priq_t yq)
{
    while (!cucon_priq_is_empty(xq)) {
	aci_key_t xk = *(aci_key_t *)cucon_priq_front(xq);
	while (!cucon_priq_is_empty(yq)) {
	    aci_key_t yk = cucon_priq_front(yq);
	    if (key_lt(xk, yk))
		return cu_false;
	    if (key_eq(xk, yk))
		break;
	    cucon_priq_pop_front(yq);
	}
	cucon_priq_pop_front(xq);
    }
    return cu_true;
}

static cu_bool_t
aci_leq_0t_lim(cucon_priq_t xq, cucon_priq_t yq, aci_key_t lim)
{
    while (!cucon_priq_is_empty(xq)) {
	aci_key_t xk = cucon_priq_front(xq);
	if (xk >= lim)
	    return cu_true;
	while (!cucon_priq_is_empty(yq)) {
	    aci_key_t yk = cucon_priq_front(yq);
	    if (key_lt(xk, yk))
		return cu_false;
	    if (key_eq(xk, yk))
		break;
	    cucon_priq_pop_front(yq);
	}
	cucon_priq_pop_front(xq);
    }
    return cu_true;
}

/* Returns false if keys is xq within y's domain are not in y, else pop
 * off keys in y's domain and return true. */
static cu_bool_t
aci_leq_1r(cuex_opn_t y, cucon_priq_t xq, cucon_priq_t yq)
{
    if (cucon_priq_is_empty(xq))
	return cu_true;
    if (aci_is_idy(y))
	return cu_true;
    if (!aci_leq_1r(aci_left(y), xq, yq))
	return cu_false;
    while (!cucon_priq_is_empty(xq)) {
	aci_key_t xk = cucon_priq_front(xq);
	if (key_geq(xk, aci_key(y))) {
	    if (key_eq(xk, aci_key(y)))
		cucon_priq_pop_front(xq);
	    return aci_leq_1r(aci_right(y), xq, yq);
	}
	while (!cucon_priq_is_empty(yq)) {
	    aci_key_t yk = cucon_priq_front(yq);
	    if (key_lt(xk, yk))
		return cu_false;
	    if (key_eq(xk, yk))
		break;
	    cucon_priq_pop_front(yq);
	}
	cucon_priq_pop_front(xq);
    }
    return cu_true;
}

static cu_bool_t
aci_leq_1l(cuex_opn_t x, cucon_priq_t xq, cucon_priq_t yq)
{
tailcall:
    if (aci_is_idy(x))
	return cu_true;
    if (!aci_leq_1l(aci_left(x), xq, yq))
	return cu_false;
    while (!cucon_priq_is_empty(yq)
	    && key_lt(cucon_priq_front(yq), aci_key(x))) {
	aci_key_t yk = cucon_priq_front(yq);
	if (!cucon_priq_is_empty(xq)) {
	    aci_key_t xk = cucon_priq_front(xq);
	    if (key_lt(xk, yk))
		return cu_false;
	    if (key_eq(xk, yk))
		cucon_priq_pop_front(xq);
	}
	cucon_priq_pop_front(yq);
    }
    if (cucon_priq_is_empty(yq) || cucon_priq_front(yq) != aci_key(x))
	return cu_false;
    x = aci_right(x);
    goto tailcall;
}

static cu_bool_t
aci_leq_2t(cuex_opn_t x, cuex_opn_t y, cucon_priq_t xq, cucon_priq_t yq)
{
    aci_key_t x_clim, y_clim;
tailcall:
    if (key_eq(x, y))
	return cucon_priq_is_empty(xq);
    if (aci_is_idy(x))
	return aci_leq_1r(y, xq, yq);
    if (aci_is_idy(y))
	return cu_false;
    if (aci_is_idy(aci_left(x))) {
	cucon_priq_insert(xq, aci_key(x));
	return aci_leq_1r(y, xq, yq);
    }
    if (aci_is_idy(aci_left(y))) {
	cucon_priq_insert(yq, aci_key(y));
	return aci_leq_1l(x, xq, yq);
    }

    x_clim = aci_tree_clim(x);
    y_clim = aci_tree_clim(y);
    if (key_eq(x_clim, y_clim)) {
	if (key_lt(aci_key(x), aci_key(y)))
	    return cu_false;
	if (!aci_leq_2t(aci_left(x), aci_left(y), xq, yq))
	    return cu_false;
	if (key_lt(aci_key(y), aci_key(x))) {
	    cucon_priq_insert(yq, aci_key(y));
	    cucon_priq_insert(xq, aci_key(x));
	}
	x = aci_right(x);
	y = aci_right(y);
	goto tailcall;
    }
    else if (aci_key_covers(y_clim, x_clim)) {
	aci_key_t xk;
	if (key_lt(x_clim, y_clim)) {
	    if (!aci_leq_2t(x, aci_left(y), xq, yq))
		return cu_false;
	    if (!cucon_priq_is_empty(xq)) {
		xk = cucon_priq_front(xq);
		if (key_lt(xk, aci_key(y)))
		    return cu_false;
		else if (key_eq(xk, aci_key(y)))
		    cucon_priq_pop_front(xq);
	    }
	    cucon_priq_insert(yq, aci_key(y));
	    return aci_leq_1r(aci_right(y), xq, yq);
	}
	else {
	    if (!aci_leq_1r(aci_left(y), xq, yq))
		return cu_false;
	    if (!cucon_priq_is_empty(xq)) {
		xk = cucon_priq_front(xq);
		if (key_lt(xk, aci_key(y)))
		    return cu_false;
		if (key_eq(xk, aci_key(y)))
		    cucon_priq_pop_front(xq);
	    }
	    cucon_priq_insert(yq, aci_key(y));
	    return aci_leq_2t(x, aci_right(y), xq, yq);
	}
    }
    else if (aci_key_covers(x_clim, y_clim)) {
	if (key_lt(y_clim, x_clim)) {
	    if (!aci_leq_2t(aci_left(x), y, xq, yq))
		return cu_false;
	    cucon_priq_insert(xq, aci_key(x));
	    return aci_leq_1l(aci_right(x), xq, yq);
	}
	else {
	    if (!aci_leq_1l(aci_left(x), xq, yq))
		return cu_false;
	    cucon_priq_insert(xq, aci_key(x));
	    return aci_leq_2t(aci_right(x), y, xq, yq);
	}
    }
    else {
	if (!aci_leq_0t_lim(xq, yq, aci_tree_llim(x)))
	    return cu_false;
	return aci_leq_1l(x, xq, yq);
    }
    return cu_false;
}

static cu_bool_t
aci_leq(cuex_opn_t x, cuex_opn_t y)
{
    struct cucon_priq_s xq, yq;
    aci_leq_priq_cct(&xq);
    aci_leq_priq_cct(&yq);
    if (!aci_leq_2t(x, y, &xq, &yq))
	return cu_false;
    return aci_leq_0t(&xq, &yq);
}

static cu_order_t
aci_order(cuex_opn_t x, cuex_opn_t y)
{
    aci_key_t x_clim, y_clim;
tailcall:
    if (key_eq(x, y))
	return cu_order_eq;
    if (aci_is_idy(x))
	return cu_order_lt;
    if (aci_is_idy(y))
	return cu_order_gt;
    if (aci_is_idy(aci_left(x))) {
	if (aci_is_idy(aci_left(y)))
	    return cu_order_none;
	else if (cuex_aci_find(y, aci_key(x)))
	    return cu_order_lt;
	else
	    return cu_order_none;
    }
    if (aci_is_idy(aci_left(y))) {
	if (cuex_aci_find(x, aci_key(y)))
	    return cu_order_gt;
	else
	    return cu_order_none;
    }

    x_clim = aci_tree_clim(x);
    y_clim = aci_tree_clim(y);
    if (key_eq(x_clim, y_clim)) {
	if (key_lt(aci_key(x), aci_key(y))) { /* possible order: > */
	    if (aci_leq(aci_left(y), aci_left(x))) {
		struct cucon_priq_s xq, yq;
		aci_leq_priq_cct(&xq);
		aci_leq_priq_cct(&yq);
		cucon_priq_insert(&yq, aci_key(y));
		if (aci_leq_2t(aci_right(y), aci_right(x), &yq, &xq)
			&& aci_leq_0t(&yq, &xq))
		    return cu_order_gt;
	    }
	    return cu_order_none;
	}
	else if (key_lt(aci_key(y), aci_key(x))) { /* possible order: < */
	    if (aci_leq(aci_left(x), aci_left(y))) {
		struct cucon_priq_s xq, yq;
		aci_leq_priq_cct(&xq);
		aci_leq_priq_cct(&yq);
		cucon_priq_insert(&xq, aci_key(x));
		if (aci_leq_2t(aci_right(x), aci_right(y), &xq, &yq)
			&& aci_leq_0t(&xq, &yq))
		    return cu_order_lt;
	    }
	    return cu_order_none;
	}
	else				/* possible order: <, =, > */
	    switch (aci_order(aci_left(x), aci_left(y))) {
		case cu_order_none:
		    return cu_order_none;
		case cu_order_lt:
		    if (aci_leq(aci_right(x), aci_right(y)))
			return cu_order_lt;
		    return cu_order_none;
		case cu_order_gt:
		    if (aci_leq(aci_right(y), aci_right(x)))
			return cu_order_gt;
		    return cu_order_none;
		case cu_order_eq:
		    x = aci_right(x);
		    y = aci_right(y);
		    goto tailcall;
	    }
    }
    else if (aci_key_covers(x_clim, y_clim)) {
	if (aci_leq(y, x))
	    return cu_order_gt;
    }
    else if (aci_key_covers(y_clim, x_clim)) {
	if (aci_leq(x, y))
	    return cu_order_lt;
    }
    return cu_order_none;
}


/* Iteration
 * ========= */

static cu_bool_t
aci_conj(cuex_meta_t join, cuex_opn_t e,
	 cu_clop(cb, cu_bool_t, cuex_opn_t))
{
tailcall:
    if (cuex_is_aci_identity(join, e))
	return cu_true;
    else {
	if (!aci_conj(join, aci_left(e), cb))
	    return cu_false;
	if (!cu_call(cb, e))
	    return cu_false;
	e = aci_right(e);
	goto tailcall;
    }
}

cu_bool_t
cuex_aci_conj(cuex_meta_t join, cuex_opn_t e,
	      cu_clop(cb, cu_bool_t, cuex_opn_t))
{
    if (cuex_is_aci(join, e))
	return aci_conj(join, e, cb);
    else
	return cu_call(cb, e);
}

static cuex_opn_t
aci_tran(cuex_meta_t join, cuex_opn_t idy, cuex_opn_t e, cuex_opn_t acc,
	 cu_clop(cb, cuex_opn_t, cuex_opn_t))
{
    while (e != idy) {
	cuex_opn_t v;
	cu_debug_assert(cuex_meta(e) == join);
	acc = aci_tran(join, idy, aci_left(e), acc, cb);
	if (!acc)
	    return NULL;
	v = cu_call(cb, aci_update(e, idy, idy));
	if (!v)
	    return NULL;
	acc = cuex_aci_join(join, acc, v);
	e = aci_right(e);
    }
    return acc;
}

cuex_opn_t
cuex_aci_tran(cuex_meta_t join, cuex_opn_t e,
	      cu_clop(cb, cuex_opn_t, cuex_opn_t))
{
    cuex_opn_t idy = cuex_aci_identity(join);
    if (cuex_is_aci(join, e))
	return aci_tran(join, idy, e, idy, cb);
    else
	return cu_call(cb, e);
}


/* Interface
 * ========= */

CU_SINLINE cuex_opn_t
aci_insert_vl(cuex_opn_t e, cuex_meta_t opr, cuex_t key, va_list vl,
	      aci_on_conflict_t on_conflict)
{
    cu_rank_t i, r = cuex_opr_r(opr);
    cuex_t *tpl = cu_salloc(r*sizeof(cuex_t));
    cu_debug_assert(cuex_opr_index(cuex_meta(e)) == cuex_opr_index(opr));
    tpl[2] = key;
    for (i = 3; i < r; ++i)
	tpl[i] = va_arg(vl, cuex_t);
    return aci_insert(e, opr, tpl, on_conflict);
}

cuex_opn_t
cuex_aci_insert(cuex_meta_t opr, cuex_opn_t e, cuex_t key, ...)
{
    va_list vl;
    va_start(vl, key);
    e = aci_insert_vl(e, opr, key, vl, aci_on_conflict_null);
    va_end(vl);
    return e;
}

cuex_opn_t
cuex_aci_replace(cuex_meta_t opr, cuex_opn_t e, cuex_t key, ...)
{
    va_list vl;
    va_start(vl, key);
    e = aci_insert_vl(e, opr, key, vl, aci_on_conflict_replace);
    va_end(vl);
    return e;
}

cu_clop_def(aci_merge_by_replace_clop,
	    cuex_opn_t, cuex_opn_t l, cuex_opn_t r, cuex_opn_t x, cuex_opn_t y)
{
    return aci_update(x, l, r);
}

cuex_opn_t
cuex_aci_join(cuex_meta_t join, cuex_t x, cuex_t y)
{
    struct aci_merge_state_s st;
    aci_merge_state_cct(&st, join, aci_merge_by_replace_clop);
    if (!cuex_is_aci(join, x))
	x = cuex_aci_generator(join, x);
    if (!cuex_is_aci(join, y))
	y = cuex_aci_generator(join, y);
    return aci_merge_2t(&st, ACI_KEY_MAX, x, y);
}

cu_order_t
cuex_aci_order(cuex_meta_t join, cuex_t x, cuex_t y)
{
    if (!cuex_is_aci(join, x))
	x = cuex_aci_generator(join, x);
    if (!cuex_is_aci(join, y))
	y = cuex_aci_generator(join, y);
    return aci_order(x, y);
}

cu_bool_t
cuex_aci_leq(cuex_meta_t join, cuex_t x, cuex_t y)
{
    if (!cuex_is_aci(join, x))
	x = cuex_aci_generator(join, x);
    if (!cuex_is_aci(join, y))
	y = cuex_aci_generator(join, y);
    return aci_leq(x, y);
}

static void
aci_stats(cuex_opn_t e, cuex_aci_stats_t stats, int depth)
{
tailcall:
    if (aci_is_idy(e)) {
	++stats->leaf_cnt;
	stats->leaf_depth_sum += depth;
	if (depth > stats->max_depth)
	    stats->max_depth = depth;
	if (depth < stats->min_depth)
	    stats->min_depth = depth;
    }
    else {
	++stats->node_cnt;
	stats->node_depth_sum += depth;
	++depth;
	aci_stats(aci_left(e), stats, depth);
	e = aci_right(e);
	goto tailcall;
    }
}

void
cuex_aci_stats(cuex_opn_t e, cuex_aci_stats_t stats)
{
    stats->max_depth = 0;
    stats->min_depth = INT_MAX;
    stats->node_cnt = 0;
    stats->node_depth_sum = 0;
    stats->leaf_cnt = 0;
    stats->leaf_depth_sum = 0;
    aci_stats(e, stats, 0);
}

size_t
cuex_aci_count(cuex_opn_t e)
{
    struct cuex_aci_stats_s st;
    cuex_aci_stats(e, &st);
    return st.node_cnt;
}

static void
aci_dump(cuex_opn_t e, FILE *out, int lev)
{
    if (aci_is_idy(e)) {
#if 0
	cu_findent(out, lev*4);
	fputs("leaf\n", out);
#endif
    }
    else {
	cu_rank_t i, r = cuex_opn_r(e);
	cuex_opn_t left = aci_left(e);
	aci_dump(left, out, lev + 1);
	cu_findent(out, lev*4);
	if (aci_is_idy(left))
	    fprintf(out, "node %"ACI_KEY_PRIp, aci_key(e));
	else
	    fprintf(out, "node %"ACI_KEY_PRIp"/%"ACI_KEY_PRIp, aci_key(e),
		    aci_key_clim(aci_key(e), aci_key(left)));
	for (i = 2; i < r; ++i)
	    cu_fprintf(out, " (%!)", cuex_opn_at(e, i));
	fputc('\n', out);
	aci_dump(aci_right(e), out, lev + 1);
    }
}
void
cuex_aci_dump(cuex_opn_t e, FILE *out)
{
    fprintf(out, "ACI @ %p\n", e);
    aci_dump(e, out, 1);
}
