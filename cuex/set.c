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

#include <cuex/set.h>
#include <cuex/atree.h>
#include <cuex/compound.h>
#include <cuex/intf.h>
#include <cuoo/hctem.h>
#include <cuoo/intf.h>
#include <cuoo/type.h>
#include <cufo/stream.h>
#include <cufo/tagdefs.h>
#include <cu/ptr_seq.h>

typedef struct cuex_set *cuex_set_t;
struct cuex_set
{
    CUOO_HCOBJ
    cuex_t atree;
};

#define SET(x) ((cuex_set_t)(x))


cu_clop_def(_set_key, cu_word_t, cuex_t e)
{
    return (cu_word_t)e;
}

static cuex_t
_set_new(cuex_t atree)
{
    cuoo_hctem_decl(cuex_set, tem);
    cuoo_hctem_init(cuex_set, tem);
    cuoo_hctem_get(cuex_set, tem)->atree = atree;
    return cuoo_hctem_new(cuex_set, tem);
}

cuex_t
cuex_singleton_set(cuex_t e)
{
    return _set_new(cuex_atree_singleton(e));
}

cuex_t
cuex_set_insert(cuex_t S, cuex_t e)
{
    if (!cuex_is_set(S))
	cu_bugf("cuex_set_insert, first arg: expected a set, got %!.", S);
    return _set_new(cuex_atree_insert(_set_key, SET(S)->atree, e));
}

cuex_t
cuex_set_erase(cuex_t S, cuex_t e)
{
    if (!cuex_is_set(S))
	cu_bugf("cuex_set_erase, first arg: expected a set, got %!.", S);
    return _set_new(cuex_atree_erase(_set_key, SET(S)->atree, (cu_word_t)e));
}

cuex_t
cuex_set_union(cuex_t S0, cuex_t S1)
{
    if (!cuex_is_set(S0))
	cu_bugf("cuex_set_union, first arg: expected a set, got %!.", S0);
    if (!cuex_is_set(S1))
	cu_bugf("cuex_set_union, second arg: expected a set, got %!.", S1);
    return _set_new(cuex_atree_left_union(_set_key,
					  SET(S0)->atree, SET(S1)->atree));
}

cuex_t
cuex_set_isecn(cuex_t S0, cuex_t S1)
{
    if (!cuex_is_set(S0))
	cu_bugf("cuex_set_isecn, first arg: expected a set, got %!.", S0);
    if (!cuex_is_set(S1))
	cu_bugf("cuex_set_isecn, second arg: expected a set, got %!.", S1);
    return _set_new(cuex_atree_left_isecn(_set_key,
					  SET(S0)->atree, SET(S1)->atree));
}

cu_bool_t
cuex_set_contains(cuex_t S, cuex_t e)
{
    if (!cuex_is_set(S))
	cu_bugf("cuex_set_contains, first arg: expected a set, got %!.", S);
    return !!cuex_atree_find(_set_key, SET(S)->atree, (cu_word_t)e);
}

cu_bool_t
cuex_set_subeq(cuex_t S0, cuex_t S1)
{
    if (!cuex_is_set(S0))
	cu_bugf("cuex_set_subeq, first arg: expected a set, got %!.", S0);
    if (!cuex_is_set(S1))
	cu_bugf("cuex_set_subeq, second arg: expected a set, got %!.", S1);
    return cuex_atree_subseteq(_set_key, SET(S0)->atree, SET(S1)->atree);
}

cu_order_t
cuex_set_order(cuex_t S0, cuex_t S1)
{
    if (!cuex_is_set(S0))
	cu_bugf("cuex_set_order, first arg: expected a set, got %!.", S0);
    if (!cuex_is_set(S1))
	cu_bugf("cuex_set_order, second arg: expected a set, got %!.", S1);
    return cuex_atree_order(_set_key, SET(S0)->atree, SET(S1)->atree);
}


/* == Compound Implementation == */

static void *
_comm_iter_source_get(cu_ptr_source_t source)
{
    return cuex_atree_itr_get(source + 1);
}

static cu_ptr_source_t
_comm_iter_source(cuex_intf_compound_t impl, cuex_t S)
{
    cu_ptr_source_t source;
    size_t atree_itr_size = cuex_atree_itr_size(SET(S)->atree);
    source = cu_galloc(sizeof(struct cu_ptr_source) + atree_itr_size);
    cuex_atree_itr_init(source + 1, SET(S)->atree);
    cu_ptr_source_init(source, _comm_iter_source_get);
    return source;
}

cu_ptr_source_t
cuex_set_iter_source(cuex_t S)
{
    if (!cuex_is_set(S))
	cu_bugf("cuex_set_iter_source, first arg: expected a set, got %!.", S);
    return _comm_iter_source(NULL, S);
}

typedef struct _comm_build_sinktor *_comm_build_sinktor_t;
struct _comm_build_sinktor
{
    cu_inherit (cu_ptr_sinktor);
    cuex_t new_atree;
};

static void
_comm_build_sinktor_put(cu_ptr_sink_t sink, void *e)
{
    _comm_build_sinktor_t self
	= cu_from2(_comm_build_sinktor, cu_ptr_sinktor, cu_ptr_sink, sink);
    self->new_atree = cuex_atree_insert(_set_key, self->new_atree, e);
}

static void *
_comm_build_sinktor_finish(cu_ptr_sinktor_t sinktor)
{
    _comm_build_sinktor_t self
	= cu_from(_comm_build_sinktor, cu_ptr_sinktor, sinktor);
    return _set_new(self->new_atree);
}

static cu_ptr_sinktor_t
_comm_build_sinktor(cuex_intf_compound_t impl, cuex_t S)
{
    _comm_build_sinktor_t self = cu_gnew(struct _comm_build_sinktor);
    cu_ptr_sinktor_init(cu_to(cu_ptr_sinktor, self),
			_comm_build_sinktor_put,
			_comm_build_sinktor_finish);
    self->new_atree = cuex_atree_empty();
    return cu_to(cu_ptr_sinktor, self);
}

static cu_ptr_sinktor_t
_comm_union_sinktor(cuex_intf_compound_t impl, cuex_t S)
{
    _comm_build_sinktor_t self = cu_gnew(struct _comm_build_sinktor);
    cu_ptr_sinktor_init(cu_to(cu_ptr_sinktor, self),
			_comm_build_sinktor_put,
			_comm_build_sinktor_finish);
    self->new_atree = SET(S)->atree;
    return cu_to(cu_ptr_sinktor, self);
}

static struct cuex_intf_compound _set_compound = {
    .flags = CUEX_COMPOUNDFLAG_PREFER_COMM
	   | CUEX_COMPOUNDFLAG_COMM_IDEMPOTENT,
    .comm_iter_source = &_comm_iter_source,
    .comm_build_sinktor = &_comm_build_sinktor,
    .comm_union_sinktor = &_comm_union_sinktor,
};


/* == Interface Dispatch == */

static void
_set_print(cufo_stream_t fos, cufo_prispec_t spec, void *S)
{
    int i = 0;
    void *itr = cu_salloc(cuex_atree_itr_size(S));
    cuex_t e;
    cuex_atree_itr_init(itr, SET(S)->atree);
    cufo_tagputc(fos, cufoT_operator, '{');
    while ((e = cuex_atree_itr_get(itr))) {
	if (i++) {
	    cufo_tagputc(fos, cufoT_operator, ',');
	    cufo_putc(fos, ' ');
	}
	cufo_print_ex(fos, e);
    }
    cufo_tagputc(fos, cufoT_operator, '}');
}

static cu_box_t
_set_dispatch(cu_word_t intf_number, ...)
{
    switch (intf_number) {
	case CUOO_INTF_FOPRINT_FN:
	    return CUOO_INTF_FOPRINT_FN_BOX(&_set_print);
	case CUEX_INTF_COMPOUND:
	    return CUEX_INTF_COMPOUND_BOX(&_set_compound);
	default:
	    return CUOO_IMPL_NONE;
    }
}

/* == Initialisation == */

cuoo_type_t cuexP_set_type;
cuex_t cuexP_set_empty;

void
cuexP_set_init()
{
    cuex_intf_compound_finish(&_set_compound);
    cuexP_set_type = cuoo_type_new_opaque_hcs(
	_set_dispatch, sizeof(struct cuex_set) - CUOO_HCOBJ_SHIFT);
    cuexP_set_empty = _set_new(cuex_atree_empty());
}
