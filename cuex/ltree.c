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

#include <cuex/ltree.h>
#include <cufo/stream.h>
#include <cu/int.h>
#include <cu/ptr_seq.h>

#define LOG2_FANOUT	CUEX_LTREE_LOG2_FANOUT
#define FANOUT		CUEX_LTREE_FANOUT
#define FANOUT_MASK	CUEX_LTREE_FANOUT_MASK

/* Make a new tree from itr stopping at depth or when itr is empty. */
static cuex_opn_t
ltree_fresh(unsigned int depth, cuex_ltree_itr_t *itr)
{
    cuex_t v[FANOUT];
    cu_rank_t i;
    if (depth == 0) {
	cuex_t x = cuex_ltree_itr_get(itr);
	cu_debug_assert(x);
	return x;
    }
    for (i = 0; i < FANOUT && !cuex_ltree_itr_is_end(itr); ++i)
	v[i] = ltree_fresh(depth - 1, itr);
    if (i == 1)
	return v[0];
    else
	return cuex_opn_by_arr(CUEXP_OXR_LTREE(depth, i), v);
}

/* As above, but using source. */
static cuex_opn_t
ltree_fresh_source(unsigned int depth, cu_ptr_source_t source)
{
    cuex_t v[FANOUT];
    cu_rank_t i;
    if (depth == 0)
	return cu_ptr_source_get(source);
    for (i = 0; i < FANOUT; ++i) {
	v[i] = ltree_fresh_source(depth - 1, source);
	if (v[i] == NULL) {
	    if (i == 0)
		return NULL;
	    else
		break;
	}
    }
    if (i == 1)
	return v[0];
    else
	return cuex_opn_by_arr(CUEXP_OXR_LTREE(depth, i), v);
}

/*!Continue filling up \a x up to depth \a xdepth. */
static cuex_opn_t
ltree_fill(unsigned xdepth, cuex_t x, cuex_ltree_itr_t *itr)
{
    cuex_t v[FANOUT];
    cu_rank_t i, r;
    unsigned int depth;
    if (!cuexP_is_ltree_node(x)) {
	if (xdepth == 0)
	    return x;
	v[0] = x;
	i = r = 1;
	depth = 1;
    }
    else {
	r = cuex_opn_arity(x);
	cu_debug_assert(r > 0);
	--r;
	for (i = 0; i < r; ++i)
	    v[i] = cuex_opn_at(x, i);
	depth = cuexP_oa_ltree_depth(cuex_meta(x));
	cu_debug_assert(depth > 0);
	if (depth > 1)
	    v[i] = ltree_fill(depth - 1, cuex_opn_at(x, i), itr);
	else
	    v[i] = cuex_opn_at(x, i);
	++i;
    }
    do {
	while (i < FANOUT && !cuex_ltree_itr_is_end(itr))
	    v[i++] = ltree_fresh(depth - 1, itr);
	v[0] = cuex_opn_by_arr(CUEXP_OXR_LTREE(depth, i), v);
	i = 1;
	++depth;
    } while (depth <= xdepth && !cuex_ltree_itr_is_end(itr));
    return v[0];
}

/* As about, but using source. */
static cuex_opn_t
ltree_fill_source(unsigned xdepth, cuex_t x, cu_ptr_source_t source)
{
    cuex_t v[FANOUT];
    cu_rank_t i, r;
    unsigned int depth;
    if (!cuexP_is_ltree_node(x)) {
	if (xdepth == 0)
	    return x;
	v[0] = x;
	i = r = 1;
	depth = 1;
    }
    else {
	r = cuex_opn_arity(x);
	cu_debug_assert(r > 0);
	--r;
	for (i = 0; i < r; ++i)
	    v[i] = cuex_opn_at(x, i);
	depth = cuexP_oa_ltree_depth(cuex_meta(x));
	cu_debug_assert(depth > 0);
	if (depth > 1)
	    v[i] = ltree_fill_source(depth - 1, cuex_opn_at(x, i), source);
	else
	    v[i] = cuex_opn_at(x, i);
	++i;
    }
    do {
	while (i < FANOUT) {
	    v[i] = ltree_fresh_source(depth - 1, source);
	    if (!v[i])
		return cuex_opn_by_arr(CUEXP_OXR_LTREE(depth, i), v);
	    ++i;
	}
	v[0] = cuex_opn_by_arr(CUEXP_OXR_LTREE(depth, i), v);
	i = 1;
	++depth;
    } while (depth <= xdepth);
    return v[0];
}

cuex_t
cuex_ltree_concat(cuex_t x, cuex_t y)
{
    cuex_ltree_itr_t itr;
    unsigned int depth;
    if (cuex_ltree_is_empty(x))
	return y;
    if (cuex_ltree_is_empty(y))
	return x;
    cuex_ltree_itr_init_full(&itr, y);
    if (cuexP_is_ltree_node(x)) {
	depth = cuexP_oa_ltree_depth(cuex_meta(x));
	cu_debug_assert(depth > 0);
	x = ltree_fill(depth, x, &itr);
    }
    else
	depth = 0;
    while (!cuex_ltree_itr_is_end(&itr)) {
	cuex_t v[FANOUT];
	cu_rank_t i;
	++depth;
	v[0] = x;
	for (i = 1; i < FANOUT && !cuex_ltree_itr_is_end(&itr); ++i)
	    v[i] = ltree_fresh(depth - 1, &itr);
	x = cuex_opn_by_arr(CUEXP_OXR_LTREE(depth, i), v);
    }
    return x;
}

cuex_t
cuex_ltree_append_from_source(cuex_t x, cu_ptr_source_t source)
{
    unsigned int depth;
    if (cuex_ltree_is_empty(x)) {
	x = cu_ptr_source_get(source);
	if (x == NULL)
	    return cuex_ltree_empty();
	depth = 0;
    }
    else if (cuexP_is_ltree_node(x)) {
	depth = cuexP_oa_ltree_depth(cuex_meta(x));
	cu_debug_assert(depth > 0);
	x = ltree_fill_source(depth, x, source);
    }
    else
	depth = 0;
    for (;;) {
	cuex_t v[FANOUT];
	cu_rank_t i;
	++depth;
	v[0] = x;
	for (i = 1; i < FANOUT; ++i) {
	    v[i] = ltree_fresh_source(depth - 1, source);
	    if (!v[i]) {
		if (i == 1)
		    return v[0];
		else
		    return cuex_opn_by_arr(CUEXP_OXR_LTREE(depth, i), v);
	    }
	}
	x = cuex_opn_by_arr(CUEXP_OXR_LTREE(depth, FANOUT), v);
    }
}

cuex_t
cuex_ltree_append_from_array(cuex_t ltree, cuex_t *arr, size_t cnt)
{
    struct cu_ptr_array_source src;
    cu_ptr_array_source_init(&src, arr, arr + cnt);
    return cuex_ltree_append_from_source(ltree, cu_to(cu_ptr_source, &src));
}

cuex_t
cuex_ltree_from_valist(va_list vl)
{
    cuex_t ltree = cuex_ltree_empty();
    for (;;) {
	int i;
	cuex_t buf[32];
	for (i = 0; i < sizeof(buf)/sizeof(buf[0]); ++i) {
	    buf[i] = va_arg(vl, void *);
	    if (!buf[i])
		break;
	}
	ltree = cuex_ltree_append_from_array(ltree, buf, i);
	if (i < sizeof(buf)/sizeof(buf[0]))
	    return ltree;
    }
}

cuex_t
cuex_ltree_from_source(cu_ptr_source_t source)
{
    return cuex_ltree_append_from_source(cuex_ltree_empty(), source);
}

cuex_t
cuex_ltree_from_array(cuex_t *elt_array, size_t elt_count)
{
    struct cu_ptr_array_source src;
    cu_ptr_array_source_init(&src, elt_array, elt_array + elt_count);
    return cuex_ltree_append_from_source(cuex_ltree_empty(),
					 cu_to(cu_ptr_source, &src));
}

size_t
cuex_ltree_size(cuex_t x)
{
    size_t accu = 0;
    while (!cuex_ltree_is_empty(x)) {
	cuex_meta_t meta = cuex_meta(x);
	unsigned int k;
	cu_rank_t r;

	if (!cuex_is_oR_ltree(meta))
	    return accu + 1;
	k = cuexP_oa_ltree_depth(meta);
	r = cuex_opr_r(meta);
	cu_debug_assert(k > 0);
	cu_debug_assert(r > 0);
	--r;
	accu += r*(1 << (LOG2_FANOUT*(k - 1)));
	x = cuex_opn_at(x, r);
    }
    return accu;
}

cuex_t
cuex_ltree_at(cuex_t x, ptrdiff_t i)
{
    if (cuex_ltree_is_empty(x))
	cu_bugf("cuex_ltree_at called on empty tree.");
    if (i < 0) {
	i += cuex_ltree_size(x);
	cu_debug_assert(i >= 0);
    }
    while (cuexP_is_ltree_node(x)) {
	unsigned int k;
	size_t subnode_size, j;
	cuex_meta_t meta = cuex_meta(x);
	k = cuexP_oa_ltree_depth(meta);
	subnode_size = 1 << LOG2_FANOUT*(k - 1);
	j = i / subnode_size;
	x = cuex_opn_at(x, j);
	i = i % subnode_size;
	--k;
    }
    if (i != 0)
	cu_bugf("Tried to access element past the end of the tree.");
    return x;
}

cuex_t
cuex_ltree_last(cuex_t x)
{
    if (cuex_ltree_is_empty(x))
	cu_bugf("cuex_ltree_last called on empty tree.");
    while (cuexP_is_ltree_node(x)) {
	size_t r = cuex_opr_r(cuex_meta(x));
	x = cuex_opn_at(x, r - 1);
    }
    return x;
}

static cuex_t
cuex_ltree_prefix(cuex_t x, ptrdiff_t n)
{
    if (n < 0)
	n += cuex_ltree_size(x);
    if (n == 0)
	return cuex_ltree_empty();
tailcall:
    if (cuexP_is_ltree_node(x)) {
	int k = cuexP_oa_ltree_depth(cuex_meta(x));
	cuex_t v[FANOUT];
	ptrdiff_t subnode_size = 1 << LOG2_FANOUT*(k - 1);
	ptrdiff_t m = n / subnode_size;
	ptrdiff_t l = n % subnode_size;
	ptrdiff_t i;
	if (m == 0) {
	    x = cuex_opn_at(x, 0);
	    goto tailcall;
	}
	if (m == 1 && l == 0)
	    return cuex_opn_at(x, 0);
	cu_debug_assert(m <= FANOUT);
	for (i = 0; i < m; ++i)
	    v[i] = cuex_opn_at(x, i);
	if (l) {
	    v[i] = cuex_ltree_prefix(cuex_opn_at(x, i), l);
	    ++i;
	}
	return cuex_opn_by_arr(CUEXP_OXR_LTREE(k, i), v);
    }
    cu_debug_assert(n == 1);
    return x;
}

cuex_t
cuex_ltree_slice(cuex_t x, ptrdiff_t i, ptrdiff_t j)
{
    cuex_ltree_itr_t it;
    if (i < 0) {
	size_t n = cuex_ltree_size(x);
	i += n;
	if (j < 0)
	    j += n;
    }
    else if (j < 0)
	j += cuex_ltree_size(x);

    if (i == j)
	return cuex_ltree_empty();
    if (i == 0) {
	if (j == CUEX_LTREE_END)
	    return x;
	else
	    return cuex_ltree_prefix(x, j);
    }
    cuex_ltree_itr_init_slice(&it, x, i, j);
    return ltree_fresh(CUEXP_OA_LTREE_DEPTH_MAXP - 1, &it);
}

static cu_bool_t
ltree_forall(cu_clop(fn, cu_bool_t, cuex_t), cuex_t x)
{
    cuex_meta_t x_meta;
    while (cuex_is_oR_ltree(x_meta = cuex_meta(x))) {
	cu_rank_t i, r = cuex_opr_r(x_meta);
	cu_debug_assert(r > 0);
	--r;
	for (i = 0; i < r; ++i)
	    if (!ltree_forall(fn, cuex_opn_at(x, i)))
		return cu_false;
	x = cuex_opn_at(x, r);
    }
    return cu_call(fn, x);
}

cu_bool_t
cuex_ltree_forall(cu_clop(fn, cu_bool_t, cuex_t), cuex_t x)
{
    if (cuex_ltree_is_empty(x))
	return cu_true;
    else if (cuexP_is_ltree_node(x))
	return ltree_forall(fn, x);
    else
	return cu_call(fn, x);
}

void
cuex_ltree_itr_init_full(cuex_ltree_itr_t *itr, cuex_t x)
{
    if (cuex_ltree_is_empty(x))
	itr->i_end = 0;
    else if (cuexP_is_ltree_node(x)) {
	unsigned int k = cuexP_oa_ltree_depth(cuex_meta(x));
	cu_debug_assert(k > 0);
	itr->i_end = cuex_ltree_size(x);
	while (k) {
	    itr->stack[k] = x;
	    cu_debug_assert(cuexP_oa_ltree_depth(cuex_meta(x)) == k);
	    x = cuex_opn_at(x, 0);
	    --k;
	}
    }
    else
	itr->i_end = 1;
    itr->stack[0] = x;
    itr->i_cur = 0;
}

void
cuex_ltree_itr_init_slice(cuex_ltree_itr_t *itr, cuex_t x,
			  ptrdiff_t i, ptrdiff_t j)
{
    if (i < 0) {
	size_t n = cuex_ltree_size(x);
	i += n;
	if (j < 0)
	    j += n;
    }
    else if (j < 0)
	j += cuex_ltree_size(x);
    cu_debug_assert(0 <= i && i <= j);

    itr->i_cur = i;
    if (cuex_ltree_is_empty(x))
	itr->i_end = 0;
    else if (cuexP_is_ltree_node(x)) {
	unsigned int k = cuexP_oa_ltree_depth(cuex_meta(x));
	cu_debug_assert(k > 0);
	itr->i_end = cuex_ltree_size(x);
	if (i == itr->i_end)
	    return;
	cu_debug_assert(i <= itr->i_end);
	while (k) {
	    size_t index = (i >> LOG2_FANOUT*(k - 1)) & FANOUT_MASK;
	    itr->stack[k] = x;
	    if (cuexP_is_ltree_node(x)
		    && cuexP_oa_ltree_depth(cuex_meta(x)) == k)
		x = cuex_opn_at(x, index);
	    else
		cu_debug_assert(index == 0);
	    --k;
	}
    }
    else
	itr->i_end = 1;
    if (j < itr->i_end)
	itr->i_end = j;
    else
	cu_debug_assert(j == itr->i_end || j == CUEX_LTREE_END);
    itr->stack[0] = x;
}

cuex_t
cuex_ltree_itr_get(cuex_ltree_itr_t *itr)
{
    size_t i = itr->i_cur;
    size_t j = i + 1;
    unsigned int k;
    cuex_t r;
    if (j >= itr->i_end) {
	if (j > itr->i_end)
	    return NULL;
	else {
	    itr->i_cur = j;
	    return itr->stack[0];
	}
    }
    k = (cu_ulong_bit_count(i ^ j) - 1)/LOG2_FANOUT;
    r = itr->stack[0];
    itr->i_cur = j;
    ++k;
    while (k > 0) {
	size_t index = (j >> LOG2_FANOUT*(k - 1)) & FANOUT_MASK;
	if (!cuexP_is_ltree_node(itr->stack[k])) {
	    cu_debug_assert(index == 0);
	    --k;
	    itr->stack[k] = itr->stack[k + 1];
	}
	else if (cuexP_oa_ltree_depth(cuex_meta(itr->stack[k])) < k) {
	    cu_debug_assert(index == 0);
	    --k;
	    itr->stack[k] = itr->stack[k + 1];
	}
	else {
	    --k;
	    itr->stack[k] = cuex_opn_at(itr->stack[k + 1], index);
	}
    }
    return r;
}

typedef struct _iter_source *_iter_source_t;
struct _iter_source
{
    cu_inherit (cu_ptr_source);
    struct cuex_ltree_itr itr;
};

static void *
_iter_source_get(cu_ptr_source_t source)
{
    _iter_source_t self = cu_from(_iter_source, cu_ptr_source, source);
    return cuex_ltree_itr_get(&self->itr);
}

cu_ptr_source_t
cuex_ltree_full_source(cuex_t x)
{
    _iter_source_t self = cu_gnew(struct _iter_source);
    cu_ptr_source_init(cu_to(cu_ptr_source, self), _iter_source_get);
    cuex_ltree_itr_init_full(&self->itr, x);
    return cu_to(cu_ptr_source, self);
}

cu_ptr_source_t
cuex_ltree_slice_source(cuex_t x, ptrdiff_t i, ptrdiff_t j)
{
    _iter_source_t self = cu_gnew(struct _iter_source);
    cu_ptr_source_init(cu_to(cu_ptr_source, self), _iter_source_get);
    cuex_ltree_itr_init_slice(&self->itr, x, i, j);
    return cu_to(cu_ptr_source, self);
}


/* == Printing == */

cu_clos_def(_ltree_fprint_helper, cu_prot(cu_bool_t, cuex_t elt),
    ( FILE *out;
      int count;
      char const *fmt_init, *fmt_cont; ))
{
    cu_clos_self(_ltree_fprint_helper);
    if (self->count++ == 0)
	cu_fprintf(self->out, self->fmt_init, elt);
    else
	cu_fprintf(self->out, self->fmt_cont, elt);
    return cu_true;
}

void
cuex_ltree_fprint(FILE *out, cuex_t ltree,
		  char const *fmt_init, char const *fmt_cont)
{
    _ltree_fprint_helper_t cb;
    cb.out = out;
    cb.count = 0;
    cb.fmt_init = fmt_init;
    cb.fmt_cont = fmt_cont;
    cuex_ltree_forall(_ltree_fprint_helper_prep(&cb), ltree);
}

cu_clos_def(_ltree_foprint_helper, cu_prot(cu_bool_t, cuex_t elt),
    ( cufo_stream_t fos;
      int count;
      char const *fmt_init, *fmt_cont; ))
{
    cu_clos_self(_ltree_foprint_helper);
    if (self->count++ == 0)
	cufo_printf(self->fos, self->fmt_init, elt);
    else
	cufo_printf(self->fos, self->fmt_cont, elt);
    return cu_true;
}

void
cuex_ltree_foprint(cufo_stream_t fos, cuex_t ltree,
		   char const *fmt_init, char const *fmt_cont)
{
    _ltree_foprint_helper_t cb;
    cb.fos = fos;
    cb.count = 0;
    cb.fmt_init = fmt_init;
    cb.fmt_cont = fmt_cont;
    cuex_ltree_forall(_ltree_foprint_helper_prep(&cb), ltree);
}
