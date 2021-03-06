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

#include <cuex/monoid.h>
#include <cuex/compound.h>
#include <cuex/oprinfo.h>
#include <cufo/stream.h>
#include <cufo/tagdefs.h>
#include <cucon/list.h>
#include <cuoo/hctem.h>

#define MONOID(x) ((cuex_monoid_t)(x))

static cuex_t
_monoid_wrap(cuex_meta_t mult, cuex_t ltree)
{
    cuoo_hctem_decl(cuex_monoid, tem);
    cuoo_hctem_init(cuex_monoid, tem);
    cuoo_hctem_get(cuex_monoid, tem)->opr = mult;
    cuoo_hctem_get(cuex_monoid, tem)->ltree = ltree;
    return cuoo_hctem_new(cuex_monoid, tem);
}

cuex_t 
cuex_monoid_identity(cuex_meta_t mult)
{
    return _monoid_wrap(mult, cuex_ltree_empty());
}

cuex_t
cuex_monoid_product(cuex_meta_t mult, cuex_t x, cuex_t y)
{
    cuex_t ltree;
    if (cuex_is_monoid_nongen(mult, x)) {
	if (cuex_ltree_is_empty(MONOID(x)->ltree))
	    return y;
	else if (cuex_is_monoid_nongen(mult, y)) {
	    if (cuex_ltree_is_empty(MONOID(y)->ltree))
		return x;
	    else
		ltree = cuex_ltree_concat(MONOID(x)->ltree, MONOID(y)->ltree);
	}
	else
	    ltree = cuex_ltree_concat(MONOID(x)->ltree, y);
    }
    else {
	if (cuex_is_monoid_nongen(mult, y)) {
	    if (cuex_ltree_is_empty(MONOID(y)->ltree))
		return x;
	    else
		ltree = cuex_ltree_concat(x, MONOID(y)->ltree);
	}
	else
	    ltree = cuex_ltree_concat(x, y);
    }
    return _monoid_wrap(mult, ltree);
}

cuex_t
cuex_monoid_rightmult(cuex_meta_t mult, cuex_t x, cuex_t y)
{
    cuex_t ltree;
    if (cuex_is_monoid_nongen(mult, x)) {
	if (cuex_ltree_is_empty(MONOID(x)->ltree))
	    return y;
	else
	    ltree = cuex_ltree_concat(MONOID(x)->ltree, y);
    }
    else
	ltree = cuex_ltree_concat(x, y);
    return _monoid_wrap(mult, ltree);
}

cuex_t
cuex_monoid_from_array(cuex_meta_t mult, cuex_t *array, size_t count)
{
    if (count == 1)
	return array[0];
    else
	return _monoid_wrap(mult, cuex_ltree_from_array(array, count));
}

cuex_t
cuex_monoid_from_source(cuex_meta_t mult, cu_ptr_source_t source)
{
    cuex_t ltree = cuex_ltree_from_source(source);
    if (cuex_ltree_is_singleton(ltree))
	return ltree;
    else
	return _monoid_wrap(mult, ltree);
}

cuex_t
cuex_monoid_rightmult_source(cuex_meta_t mult,
			     cuex_t x, cu_ptr_source_t y_source)
{
    if (cuex_is_monoid_nongen(mult, x)) {
	if (cuex_ltree_is_empty(MONOID(x)->ltree)) {
	    cuex_t ltree = cuex_ltree_from_source(y_source);
	    if (cuex_ltree_is_singleton(ltree))
		return ltree;
	    else
		return _monoid_wrap(mult, ltree);
	}
	else {
	    cuex_t ltree;
	    ltree = cuex_ltree_append_from_source(MONOID(x)->ltree, y_source);
	    return _monoid_wrap(mult, ltree);
	}
    }
    else {
	cuex_t ltree = cuex_ltree_append_from_source(x, y_source);
	if (cuex_ltree_is_singleton(ltree))
	    return ltree;
	else
	    return _monoid_wrap(mult, ltree);
    }
}

cuex_t
cuex_monoid_rightmult_array(cuex_meta_t mult, cuex_t x,
			    cuex_t *array, size_t count)
{
    struct cu_ptr_array_source src;
    cu_ptr_array_source_init(&src, array, array + count);
    return cuex_monoid_rightmult_source(mult, x, cu_to(cu_ptr_source, &src));
}

size_t
cuex_monoid_length(cuex_meta_t mult, cuex_t x)
{
    if (cuex_is_monoid_nongen(mult, x))
	return cuex_ltree_size(MONOID(x)->ltree);
    else
	return 1;
}

cuex_t
cuex_monoid_factor_at(cuex_meta_t mult, cuex_t x, ptrdiff_t i)
{
    if (cuex_is_monoid_nongen(mult, x))
	return cuex_ltree_at(MONOID(x)->ltree, i);
    else if (i == 0 || i == -1)
	return x;
    else
	cu_bugf("cuex_monoid_factor_at: "
		"Index %d out of range for monoid generator.", i);
}

cuex_t
cuex_monoid_factor_slice(cuex_meta_t mult, cuex_t x, ptrdiff_t i, ptrdiff_t j)
{
    if (cuex_is_monoid_nongen(mult, x)) {
	cuex_t e = cuex_ltree_slice(MONOID(x)->ltree, i, j);
	if (cuex_ltree_is_singleton(e))
	    return e;
	else
	    return _monoid_wrap(mult, e);
    }
    else {
	if (i < 0) ++i;
	if (j < 0) ++j;
	if (i == j)
	    return cuex_monoid_identity(mult);
	else if (i == 0 && (j == 1 || j == CUEX_MONOID_END))
	    return x;
	else
	    cu_bugf("cuex_monoid_factor_slice: Indices [%d, %d) out of range "
		    "for monoid generator.", i, j);
    }
}

void
cuex_monoid_itr_init_full(cuex_meta_t mult,
			  cuex_monoid_itr_t *itr, cuex_t e)
{
    if (cuex_is_monoid_nongen(mult, e))
	cuex_ltree_itr_init_full(&itr->sub, ((cuex_monoid_t)e)->ltree);
    else
	cuex_ltree_itr_init_full(&itr->sub, e);
}

void
cuex_monoid_itr_init_slice(cuex_meta_t mult,
			   cuex_monoid_itr_t *itr, cuex_t e,
			   ptrdiff_t i, ptrdiff_t j)
{
    if (cuex_is_monoid_nongen(mult, e))
	cuex_ltree_itr_init_slice(&itr->sub, ((cuex_monoid_t)e)->ltree, i, j);
    else
	cuex_ltree_itr_init_slice(&itr->sub, e, i, j);
}


/* == Factor Sources == */

static cu_ptr_source_t
_factor_source(cuex_intf_compound_t impl, cuex_t x)
{
    return cuex_ltree_full_source(MONOID(x)->ltree);
}

cu_ptr_source_t
cuex_any_monoid_factor_source(cuex_t x, ptrdiff_t i, ptrdiff_t j)
{
    if (cuex_is_any_monoid_nongen(x))
	return cuex_ltree_slice_source(MONOID(x)->ltree, i, j);
    else
	return cuex_ltree_slice_source(x, i, j);
}


/* == Build Sinktor == */

typedef struct _build_sinktor *_build_sinktor_t;
struct _build_sinktor
{
    cu_inherit (cu_ptr_sinktor);
    cuex_meta_t mult;
    struct cucon_list l;
};

static void
_build_sinktor_put(cu_ptr_sink_t sink, void *x)
{
    _build_sinktor_t self;
    self = cu_from2(_build_sinktor, cu_ptr_sinktor, cu_ptr_sink, sink);
    cucon_list_append_ptr(&self->l, x);
}

static void *
_build_sinktor_finish(cu_ptr_sinktor_t sinktor)
{
    _build_sinktor_t self = cu_from(_build_sinktor, cu_ptr_sinktor, sinktor);
    if (cucon_list_is_singleton(&self->l))
	return cucon_list_front_ptr(&self->l);
    else {
	cu_ptr_source_t source = cucon_list_source_ptr(&self->l);
	return cuex_monoid_from_source(self->mult, source);
    }
}

static cu_ptr_sinktor_t
_build_sinktor(cuex_intf_compound_t impl, void *tpl)
{
    _build_sinktor_t self = cu_gnew(struct _build_sinktor);
    cu_ptr_sinktor_init(cu_to(cu_ptr_sinktor, self),
			_build_sinktor_put, _build_sinktor_finish);
    cu_debug_assert(cuex_is_any_monoid_nongen(tpl));
    self->mult = MONOID(tpl)->opr;
    cucon_list_init(&self->l);
    return cu_to(cu_ptr_sinktor, self);
}


/* == Printing == */

static void
_monoid_print(void *x, FILE *out)
{
    cuex_oprinfo_t oprinfo;
    oprinfo = cuex_oprinfo(MONOID(x)->opr);
    cu_debug_assert(oprinfo);
    fprintf(out, "%s:[", cuex_oprinfo_name(oprinfo));
    cuex_ltree_fprint(out, MONOID(x)->ltree, "%!", ", %!");
    fputs("]", out);
}

static void
_monoid_foprint(cufo_stream_t fos, cufo_prispec_t spec, void *x)
{
    cuex_oprinfo_t oprinfo;
    oprinfo = cuex_oprinfo(MONOID(x)->opr);
    cu_debug_assert(oprinfo);
    cufo_enter(fos, cufoT_operator);
    cufo_printf(fos, "%s:[", cuex_oprinfo_name(oprinfo));
    cufo_leave(fos, cufoT_operator);
    cuex_ltree_foprint(fos, MONOID(x)->ltree, "%!", ", %!");
    cufo_enter(fos, cufoT_operator);
    cufo_putc(fos, ']');
    cufo_leave(fos, cufoT_operator);
}


/* == Dispatch == */

static struct cuex_intf_compound _compound_impl = {
    .flags = CUEX_COMPOUNDFLAG_PREFER_NCOMM
	   | CUEX_COMPOUNDFLAG_NCOMM_FILTERABLE_IMAGE
	   | CUEX_COMPOUNDFLAG_NCOMM_EXPANSIVE_IMAGE,
    .ncomm_iter_source = _factor_source,
    .ncomm_build_sinktor = _build_sinktor,
};

static cu_box_t
_monoid_dispatch(cu_word_t intf_number, ...)
{
    switch (intf_number) {
	case CUEX_INTF_COMPOUND:
	    return CUEX_INTF_COMPOUND_BOX(&_compound_impl);
	case CUOO_INTF_PRINT_FN:
	    return CUOO_INTF_PRINT_FN_BOX(_monoid_print);
	case CUOO_INTF_FOPRINT_FN:
	    return CUOO_INTF_FOPRINT_FN_BOX(_monoid_foprint);
	default:
	    return CUOO_IMPL_NONE;
    }
}

cuoo_type_t cuexP_monoid_type;

void
cuexP_monoid_init(void)
{
    cuex_intf_compound_finish(&_compound_impl);
    cuexP_monoid_type = cuoo_type_new_opaque_hcs(
	_monoid_dispatch, sizeof(struct cuex_monoid) - CUOO_HCOBJ_SHIFT);
}
