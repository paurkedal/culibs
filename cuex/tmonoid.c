/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2009  Petter Urkedal <urkedal@nbi.dk>
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

#include <cuex/tmonoid.h>
#include <cuex/compound.h>
#include <cuex/oprinfo.h>
#include <cufo/stream.h>
#include <cucon/list.h>

#define TMONOID(x) ((cuex_tmonoid_t)(x))

static cuex_t
_tmonoid_wrap(cuex_meta_t mult, cuex_t ltree)
{
    cuoo_hctem_decl(cuex_tmonoid, tem);
    cuoo_hctem_init(cuex_tmonoid, tem);
    cuoo_hctem_get(cuex_tmonoid, tem)->opr = mult;
    cuoo_hctem_get(cuex_tmonoid, tem)->ltree = ltree;
    return cuoo_hctem_new(cuex_tmonoid, tem);
}

cuex_t
cuex_tmonoid_identity(cuex_meta_t mult)
{
    return _tmonoid_wrap(mult, cuex_ltree_empty());
}

cuex_t
cuex_tmonoid_generator(cuex_meta_t mult, cuex_t x)
{
    return _tmonoid_wrap(mult, x);
}

cuex_t
cuex_tmonoid_product(cuex_meta_t mult, cuex_t x, cuex_t y)
{
    cu_debug_assert(cuex_tmonoid_contains(mult, x));
    cu_debug_assert(cuex_tmonoid_contains(mult, y));
    return _tmonoid_wrap(TMONOID(x)->opr, cuex_ltree_concat(TMONOID(x)->ltree,
							    TMONOID(y)->ltree));
}

cuex_t
cuex_any_tmonoid_product(cuex_t x, cuex_t y)
{
    cu_debug_assert(cuex_any_tmonoid_contains(x));
    cu_debug_assert(cuex_any_tmonoid_contains(y));
    cu_debug_assert(TMONOID(x)->opr == TMONOID(y)->opr);
    return _tmonoid_wrap(TMONOID(x)->opr, cuex_ltree_concat(TMONOID(x)->ltree,
							    TMONOID(y)->ltree));
}

cuex_t
cuex_tmonoid_rightmult(cuex_meta_t meta, cuex_t x, cuex_t y)
{
    cu_debug_assert(cuex_tmonoid_contains(meta, x));
    return _tmonoid_wrap(TMONOID(x)->opr,
			 cuex_ltree_concat(TMONOID(x)->ltree, y));
}

cuex_t
cuex_any_tmonoid_rightmult(cuex_t x, cuex_t y)
{
    cu_debug_assert(cuex_any_tmonoid_contains(x));
    return _tmonoid_wrap(TMONOID(x)->opr,
			 cuex_ltree_concat(TMONOID(x)->ltree, y));
}

cuex_t
cuex_tmonoid_from_valist(cuex_meta_t mult, va_list vl)
{
    return _tmonoid_wrap(mult, cuex_ltree_from_valist(vl));
}

cuex_t
cuex_tmonoid_from_args(cuex_meta_t mult, ...)
{
    cuex_t e;
    va_list vl;
    va_start(vl, mult);
    e = cuex_tmonoid_from_valist(mult, vl);
    va_end(vl);
    return e;
}

cuex_t
cuex_tmonoid_from_array(cuex_meta_t mult, cuex_t *array, size_t count)
{
    return _tmonoid_wrap(mult, cuex_ltree_from_array(array, count));
}

cuex_t
cuex_tmonoid_from_source(cuex_meta_t mult, cu_ptr_source_t source)
{
    return _tmonoid_wrap(mult, cuex_ltree_from_source(source));
}

cuex_t
cuex_tmonoid_rightmult_source(cuex_meta_t mult, cuex_t x,
			      cu_ptr_source_t y_source)
{
    cuex_t ltree;
    cu_debug_assert(cuex_tmonoid_contains(mult, x));
    ltree = cuex_ltree_append_from_source(TMONOID(x)->ltree, y_source);
    return _tmonoid_wrap(TMONOID(x)->opr, ltree);
}

cuex_t
cuex_any_tmonoid_rightmult_source(cuex_t x, cu_ptr_source_t y_source)
{
    cuex_t ltree;
    cu_debug_assert(cuex_any_tmonoid_contains(x));
    ltree = cuex_ltree_append_from_source(TMONOID(x)->ltree, y_source);
    return _tmonoid_wrap(TMONOID(x)->opr, ltree);
}

cuex_t
cuex_tmonoid_rightmult_array(cuex_meta_t mult, cuex_t x,
			     cuex_t *array, size_t count)
{
    struct cu_ptr_array_source_s src;
    cu_ptr_array_source_init(&src, array, array + count);
    return cuex_tmonoid_rightmult_source(mult, x, cu_to(cu_ptr_source, &src));
}

cuex_t
cuex_any_tmonoid_rightmult_array(cuex_t x, cuex_t *array, size_t count)
{
    struct cu_ptr_array_source_s src;
    cu_ptr_array_source_init(&src, array, array + count);
    return cuex_any_tmonoid_rightmult_source(x, cu_to(cu_ptr_source, &src));
}

size_t
cuex_tmonoid_length(cuex_meta_t mult, cuex_t x)
{
    cu_debug_assert(cuex_tmonoid_contains(mult, x));
    return cuex_ltree_size(TMONOID(x)->ltree);
}

size_t
cuex_any_tmonoid_length(cuex_t x)
{
    cu_debug_assert(cuex_any_tmonoid_contains(x));
    return cuex_ltree_size(TMONOID(x)->ltree);
}

cuex_t
cuex_tmonoid_factor_at(cuex_meta_t mult, cuex_t x, ptrdiff_t i)
{
    cu_debug_assert(cuex_tmonoid_contains(mult, x));
    return cuex_ltree_at(TMONOID(x)->ltree, i);
}

cuex_t
cuex_any_tmonoid_factor_at(cuex_t x, ptrdiff_t i)
{
    cu_debug_assert(cuex_any_tmonoid_contains(x));
    return cuex_ltree_at(TMONOID(x)->ltree, i);
}

cuex_t
cuex_tmonoid_factor_slice(cuex_meta_t mult, cuex_t x, ptrdiff_t i, ptrdiff_t j)
{
    cu_debug_assert(cuex_tmonoid_contains(mult, x));
    return _tmonoid_wrap(TMONOID(x)->opr,
			 cuex_ltree_slice(TMONOID(x)->ltree, i, j));
}

cuex_t
cuex_any_tmonoid_factor_slice(cuex_t x, ptrdiff_t i, ptrdiff_t j)
{
    cu_debug_assert(cuex_any_tmonoid_contains(x));
    return _tmonoid_wrap(TMONOID(x)->opr,
			 cuex_ltree_slice(TMONOID(x)->ltree, i, j));
}

void
cuex_tmonoid_itr_init_full(cuex_meta_t mult, cuex_tmonoid_itr_t *itr, cuex_t x)
{
    cu_debug_assert(cuex_tmonoid_contains(mult, x));
    cuex_ltree_itr_init_full(&itr->sub, TMONOID(x)->ltree);
}

void
cuex_any_tmonoid_itr_init_full(cuex_tmonoid_itr_t *itr, cuex_t x)
{
    cu_debug_assert(cuex_any_tmonoid_contains(x));
    cuex_ltree_itr_init_full(&itr->sub, TMONOID(x)->ltree);
}

void
cuex_tmonoid_itr_init_slice(cuex_meta_t mult, cuex_tmonoid_itr_t *itr, cuex_t x,
			    ptrdiff_t i, ptrdiff_t j)
{
    cu_debug_assert(cuex_tmonoid_contains(mult, x));
    cuex_ltree_itr_init_slice(&itr->sub, TMONOID(x)->ltree, i, j);
}

void
cuex_any_tmonoid_itr_init_slice(cuex_tmonoid_itr_t *itr, cuex_t x,
				ptrdiff_t i, ptrdiff_t j)
{
    cu_debug_assert(cuex_any_tmonoid_contains(x));
    cuex_ltree_itr_init_slice(&itr->sub, TMONOID(x)->ltree, i, j);
}

cu_ptr_source_t
cuex_tmonoid_factor_source(cuex_meta_t mult, cuex_t x, ptrdiff_t i, ptrdiff_t j)
{
    cu_debug_assert(cuex_tmonoid_contains(mult, x));
    return cuex_ltree_slice_source(x, i, j);
}

cu_ptr_source_t
cuex_any_tmonoid_factor_source(cuex_t x, ptrdiff_t i, ptrdiff_t j)
{
    cu_debug_assert(cuex_any_tmonoid_contains(x));
    return cuex_ltree_slice_source(x, i, j);
}

/* == Factor Sources == */

static cu_ptr_source_t
_factor_source(cuex_intf_compound_t impl, cuex_t x)
{
    return cuex_ltree_full_source(TMONOID(x)->ltree);
}

/* == Build Sinktor == */

typedef struct _build_sinktor_s *_build_sinktor_t;
struct _build_sinktor_s
{
    cu_inherit (cu_ptr_sinktor_s);
    cuex_meta_t mult;
    struct cucon_list_s l;
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
    cu_ptr_source_t source = cucon_list_source_ptr(&self->l);
    return cuex_tmonoid_from_source(self->mult, source);
}

static cu_ptr_sinktor_t
_build_sinktor(cuex_intf_compound_t impl, void *tpl)
{
    _build_sinktor_t self = cu_gnew(struct _build_sinktor_s);
    cu_ptr_sinktor_init(cu_to(cu_ptr_sinktor, self),
			_build_sinktor_put, _build_sinktor_finish);
    cu_debug_assert(cuex_any_tmonoid_contains(tpl));
    self->mult = TMONOID(tpl)->opr;
    cucon_list_init(&self->l);
    return cu_to(cu_ptr_sinktor, self);
}


/* == Printing == */

static void
_tmonoid_print(void *x, FILE *out)
{
    cuex_oprinfo_t oprinfo;
    oprinfo = cuex_oprinfo(TMONOID(x)->opr);
    cu_debug_assert(oprinfo);
    fprintf(out, "%s:[", cuex_oprinfo_name(oprinfo));
    cuex_ltree_fprint(out, TMONOID(x)->ltree, "%!", ", %!");
    fputs("]", out);
}

static void
_tmonoid_foprint(cufo_stream_t fos, cufo_prispec_t spec, void *x)
{
    cuex_oprinfo_t oprinfo;
    oprinfo = cuex_oprinfo(TMONOID(x)->opr);
    cu_debug_assert(oprinfo);
    cufo_printf(fos, "%s:[", cuex_oprinfo_name(oprinfo));
    cuex_ltree_foprint(fos, TMONOID(x)->ltree, "%!", ", %!");
    cufo_putc(fos, ']');
}


/* == Dispatch == */

static struct cuex_intf_compound_s _compound_impl = {
    .flags = CUEX_COMPOUNDFLAG_PREFER_NCOMM
	   | CUEX_COMPOUNDFLAG_NCOMM_FILTERABLE_IMAGE
	   | CUEX_COMPOUNDFLAG_NCOMM_EXPANSIVE_IMAGE,
    .ncomm_iter_source = _factor_source,
    .ncomm_build_sinktor = _build_sinktor,
};

static cu_word_t
_tmonoid_dispatch(cu_word_t intf_number, ...)
{
    switch (intf_number) {
	case CUEX_INTF_COMPOUND:
	    return (cu_word_t)&_compound_impl;
	case CUOO_INTF_PRINT_FN:
	    return (cu_word_t)_tmonoid_print;
	case CUOO_INTF_FOPRINT_FN:
	    return (cu_word_t)_tmonoid_foprint;
	default:
	    return CUOO_IMPL_NONE;
    }
}

cuoo_type_t cuexP_tmonoid_type;

void
cuexP_tmonoid_init(void)
{
    cuex_intf_compound_finish(&_compound_impl);
    cuexP_tmonoid_type = cuoo_type_new_opaque_hcs(
	    _tmonoid_dispatch,
	    sizeof(struct cuex_tmonoid_s) - CUOO_HCOBJ_SHIFT);
}
