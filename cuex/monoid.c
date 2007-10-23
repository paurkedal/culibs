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
#include <cuex/intf.h>
#include <cuex/oprinfo.h>
#include <cucon/list.h>
#include <cu/ptr_seq.h>

#define MONOID(x) ((cuex_monoid_t)(x))

static cuex_t
make_monoid(cuex_meta_t mult, cuex_t ltree)
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
    cuoo_hctem_decl(cuex_monoid, tem);
    cuoo_hctem_init(cuex_monoid, tem);
    cuoo_hctem_get(cuex_monoid, tem)->opr = mult;
    cuoo_hctem_get(cuex_monoid, tem)->ltree = cuex_ltree_empty();
    return cuoo_hctem_new(cuex_monoid, tem);
}

cuex_t
cuex_monoid_product(cuex_meta_t mult, cuex_t x, cuex_t y)
{
    cuex_t ltree;
    if (cuex_is_monoid(mult, x)) {
	if (cuex_ltree_is_empty(MONOID(x)->ltree))
	    return y;
	else if (cuex_is_monoid(mult, y)) {
	    if (cuex_ltree_is_empty(MONOID(y)->ltree))
		return x;
	    else
		ltree = cuex_ltree_concat(MONOID(x)->ltree, MONOID(y)->ltree);
	}
	else
	    ltree = cuex_ltree_concat(MONOID(x)->ltree, y);
    }
    else {
	if (cuex_is_monoid(mult, y)) {
	    if (cuex_ltree_is_empty(MONOID(y)->ltree))
		return x;
	    else
		ltree = cuex_ltree_concat(x, MONOID(y)->ltree);
	}
	else
	    ltree = cuex_ltree_concat(x, y);
    }
    return make_monoid(mult, ltree);
}

cuex_t
cuex_monoid_product_over_source(cuex_meta_t mult, cu_ptr_source_t source)
{
    cuex_t ltree = cuex_ltree_from_source(source);
    if (cuex_ltree_is_singleton(ltree))
	return ltree;
    else
	return make_monoid(mult, ltree);
}

cuex_t
cuex_monoid_right_multiply_source(cuex_meta_t mult,
				  cuex_t x, cu_ptr_source_t y_source)
{
    if (cuex_is_monoid(mult, x)) {
	if (cuex_ltree_is_empty(MONOID(x)->ltree)) {
	    cuex_t ltree = cuex_ltree_from_source(y_source);
	    if (cuex_ltree_is_singleton(ltree))
		return ltree;
	    else
		return make_monoid(mult, ltree);
	}
	else {
	    cuex_t ltree;
	    ltree = cuex_ltree_append_from_source(MONOID(x)->ltree, y_source);
	    return make_monoid(mult, ltree);
	}
    }
    else {
	cuex_t ltree = cuex_ltree_append_from_source(x, y_source);
	if (cuex_ltree_is_singleton(ltree))
	    return ltree;
	else
	    return make_monoid(mult, ltree);
    }
}

size_t
cuex_monoid_factor_count(cuex_meta_t mult, cuex_t x)
{
    if (cuex_is_monoid(mult, x))
	return cuex_ltree_size(MONOID(x)->ltree);
    else
	return 1;
}

cuex_t
cuex_monoid_factor_at(cuex_meta_t mult, cuex_t x, size_t i)
{
    if (cuex_is_monoid(mult, x))
	return cuex_ltree_at(MONOID(x)->ltree, i);
    else if (i == 0)
	return x;
    else
	cu_bugf("cuex_monoid_factor_at: "
		"Index %d out of range for monoid generator.", i);
}

cuex_t
cuex_monoid_factor_slice(cuex_meta_t mult, cuex_t x, size_t i, size_t j)
{
    if (i == j)
	return cuex_monoid_identity(mult);
    else if (cuex_is_monoid(mult, x)) {
	cuex_t e = cuex_ltree_slice(MONOID(x)->ltree, i, j);
	if (i + 1 == j)
	    return e;
	else
	    return make_monoid(mult, e);
    }
    else if (i == 0 && j == 1)
	return x;
    else
	cu_bugf("cuex_monoid_factor_slice: "
		"Indices [%d, %d) out of range for monoid generator.", i, j);
}

void
cuex_monoid_itr_init_full(cuex_meta_t mult,
			  cuex_monoid_itr_t *itr, cuex_t e)
{
    if (cuex_is_monoid(mult, e))
	cuex_ltree_itr_init_full(&itr->sub, ((cuex_monoid_t)e)->ltree);
    else
	cuex_ltree_itr_init_full(&itr->sub, e);
}

void
cuex_monoid_itr_init_slice(cuex_meta_t mult,
			   cuex_monoid_itr_t *itr, cuex_t e,
			   size_t i, size_t j)
{
    if (cuex_is_monoid(mult, e))
	cuex_ltree_itr_init_slice(&itr->sub, ((cuex_monoid_t)e)->ltree, i, j);
    else
	cuex_ltree_itr_init_slice(&itr->sub, e, i, j);
}


/* == Factor Sources == */

static cu_ptr_source_t
factor_source(cuex_intf_compound_t impl, cuex_t x)
{
    return cuex_ltree_full_source(MONOID(x)->ltree);
}

cu_ptr_source_t
cuex_any_monoid_factor_source(cuex_t x, size_t i, size_t j)
{
    if (cuex_is_any_monoid(x))
	return cuex_ltree_slice_source(MONOID(x)->ltree, i, j);
    else
	return cuex_ltree_slice_source(x, i, j);
}


/* == Build Sinktor == */

typedef struct build_sinktor_s *build_sinktor_t;
struct build_sinktor_s
{
    cu_inherit (cu_ptr_sinktor_s);
    cuex_meta_t mult;
    struct cucon_list_s l;
};

static void
build_sinktor_put(cu_ptr_sink_t sink, void *x)
{
    build_sinktor_t self;
    self = cu_from2(build_sinktor, cu_ptr_sinktor, cu_ptr_sink, sink);
    cucon_list_append_ptr(&self->l, x);
};

static void *
build_sinktor_finish(cu_ptr_sinktor_t sinktor)
{
    build_sinktor_t self = cu_from(build_sinktor, cu_ptr_sinktor, sinktor);
    if (cucon_list_is_singleton(&self->l))
	return cucon_list_front_ptr(&self->l);
    else {
	cu_ptr_source_t source = cucon_list_source_ptr(&self->l);
	return cuex_monoid_product_over_source(self->mult, source);
    }
}

static cu_ptr_sinktor_t
build_sinktor(cuex_intf_compound_t impl, void *tpl)
{
    build_sinktor_t self = cu_gnew(struct build_sinktor_s);
    cu_ptr_sinktor_init(cu_to(cu_ptr_sinktor, self),
			build_sinktor_put, build_sinktor_finish);
    cu_debug_assert(cuex_is_any_monoid(tpl));
    self->mult = MONOID(tpl)->opr;
    cucon_list_cct(&self->l);
    return cu_to(cu_ptr_sinktor, self);
}


/* == Printing == */

cu_clos_def(monoid_print_item, cu_prot(cu_bool_t, cuex_t factor),
	    (FILE *out; int count;))
{
    cu_clos_self(monoid_print_item);
    if (self->count++)
	fputs(", ", self->out);
    cu_fprintf(self->out, "%!", factor);
    return cu_true;
}

static void
monoid_print(void *x, FILE *out)
{
    monoid_print_item_t cb;
    cuex_oprinfo_t oprinfo;
    cb.out = out;
    cb.count = 0;
    oprinfo = cuex_oprinfo(MONOID(x)->opr);
    cu_debug_assert(oprinfo);
    fprintf(out, "%s:[", cuex_oprinfo_name(oprinfo));
    cuex_ltree_forall(monoid_print_item_prep(&cb), MONOID(x)->ltree);
    fputs("]", out);
}


/* == Dispatch == */

struct cuex_intf_compound_s compound_impl = {
    .flags = CUEX_COMPOUNDFLAG_PREFER_NCOMM
	   | CUEX_COMPOUNDFLAG_NCOMM_FILTERABLE_IMAGE
	   | CUEX_COMPOUNDFLAG_NCOMM_EXPANSIVE_IMAGE,
    .ncomm_iter_source = factor_source,
    .ncomm_build_sinktor = build_sinktor,
};

cu_word_t
monoid_dispatch(cu_word_t intf_number, ...)
{
    switch (intf_number) {
	case CUEX_INTF_COMPOUND:
	    return (cu_word_t)&compound_impl;
	case CUOO_INTF_PRINT_FN:
	    return (cu_word_t)monoid_print;
	default:
	    return CUOO_IMPL_NONE;
    }
}

cuoo_stdtype_t cuexP_monoid_type;

void
cuexP_monoid_init(void)
{
    cuex_intf_compound_finish(&compound_impl);
    cuexP_monoid_type = cuoo_stdtype_new_hcs(
	monoid_dispatch, sizeof(struct cuex_monoid_s) - CUOO_HCOBJ_SHIFT);
}
