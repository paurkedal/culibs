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
#include <cuoo/halloc.h>
#include <cuoo/intf.h>
#include <cuoo/type.h>
#include <cu/ptr_seq.h>

typedef struct cuex_set_s *cuex_set_t;
struct cuex_set_s
{
    CUOO_HCOBJ
    cuex_t atree;
};

#define SET(x) ((cuex_set_t)(x))


cu_clop_def(set_key, cu_word_t, cuex_t e)
{
    return (cu_word_t)e;
}

static cuex_t
set_new(cuex_t atree)
{
    cuoo_hctem_decl(cuex_set, tem);
    cuoo_hctem_init(cuex_set, tem);
    cuoo_hctem_get(cuex_set, tem)->atree = atree;
    return cuoo_hctem_new(cuex_set, tem);
}

cuex_t
cuex_singleton_set(cuex_t e)
{
    return set_new(cuex_atree_singleton(e));
}

cuex_t
cuex_set_insert(cuex_t S, cuex_t e)
{
    if (!cuex_is_set(S))
	cu_bugf("cuex_set_insert, first arg: expected a set, got %!.", S);
    return set_new(cuex_atree_insert(set_key, SET(S)->atree, e));
}

cuex_t
cuex_set_erase(cuex_t S, cuex_t e)
{
    if (!cuex_is_set(S))
	cu_bugf("cuex_set_erase, first arg: expected a set, got %!.", S);
    return set_new(cuex_atree_erase(set_key, SET(S)->atree, (cu_word_t)e));
}

cuex_t
cuex_set_union(cuex_t S0, cuex_t S1)
{
    if (!cuex_is_set(S0))
	cu_bugf("cuex_set_union, first arg: expected a set, got %!.", S0);
    if (!cuex_is_set(S1))
	cu_bugf("cuex_set_union, second arg: expected a set, got %!.", S1);
    return set_new(cuex_atree_left_union(set_key,
					 SET(S0)->atree, SET(S1)->atree));
}

cuex_t
cuex_set_isecn(cuex_t S0, cuex_t S1)
{
    if (!cuex_is_set(S0))
	cu_bugf("cuex_set_isecn, first arg: expected a set, got %!.", S0);
    if (!cuex_is_set(S1))
	cu_bugf("cuex_set_isecn, second arg: expected a set, got %!.", S1);
    return set_new(cuex_atree_left_isecn(set_key,
					 SET(S0)->atree, SET(S1)->atree));
}

cu_bool_t
cuex_set_contains(cuex_t S, cuex_t e)
{
    if (!cuex_is_set(S))
	cu_bugf("cuex_set_contains, first arg: expected a set, got %!.", S);
    return !!cuex_atree_find(set_key, SET(S)->atree, (cu_word_t)e);
}

cu_bool_t
cuex_set_subeq(cuex_t S0, cuex_t S1)
{
    if (!cuex_is_set(S0))
	cu_bugf("cuex_set_subeq, first arg: expected a set, got %!.", S0);
    if (!cuex_is_set(S1))
	cu_bugf("cuex_set_subeq, second arg: expected a set, got %!.", S1);
    return cuex_atree_subseteq(set_key, SET(S0)->atree, SET(S1)->atree);
}

cu_order_t
cuex_set_order(cuex_t S0, cuex_t S1)
{
    if (!cuex_is_set(S0))
	cu_bugf("cuex_set_order, first arg: expected a set, got %!.", S0);
    if (!cuex_is_set(S1))
	cu_bugf("cuex_set_order, second arg: expected a set, got %!.", S1);
    return cuex_atree_order(set_key, SET(S0)->atree, SET(S1)->atree);
}


/* == Compound Implementation == */

static void *
comm_iter_source_get(cu_ptr_source_t source)
{
    return cuex_atree_itr_get(source + 1);
}

static cu_ptr_source_t
comm_iter_source(cuex_intf_compound_t impl, cuex_t S)
{
    cu_ptr_source_t source;
    size_t atree_itr_size = cuex_atree_itr_size(SET(S)->atree);
    source = cu_galloc(sizeof(struct cu_ptr_source_s) + atree_itr_size);
    cuex_atree_itr_init(source + 1, SET(S)->atree);
    cu_ptr_source_init(source, comm_iter_source_get);
    return source;
}

cu_ptr_source_t
cuex_set_iter_source(cuex_t S)
{
    if (!cuex_is_set(S))
	cu_bugf("cuex_set_iter_source, first arg: expected a set, got %!.", S);
    return comm_iter_source(NULL, S);
}

typedef struct comm_build_sinktor_s *comm_build_sinktor_t;
struct comm_build_sinktor_s
{
    cu_inherit (cu_ptr_sinktor_s);
    cuex_t new_atree;
};

static void
comm_build_sinktor_put(cu_ptr_sink_t sink, void *e)
{
    comm_build_sinktor_t self
	= cu_from2(comm_build_sinktor, cu_ptr_sinktor, cu_ptr_sink, sink);
    self->new_atree = cuex_atree_insert(set_key, self->new_atree, e);
}

static void *
comm_build_sinktor_finish(cu_ptr_sinktor_t sinktor)
{
    comm_build_sinktor_t self
	= cu_from(comm_build_sinktor, cu_ptr_sinktor, sinktor);
    return set_new(self->new_atree);
}

static cu_ptr_sinktor_t
comm_build_sinktor(cuex_intf_compound_t impl, cuex_t S)
{
    comm_build_sinktor_t self = cu_gnew(struct comm_build_sinktor_s);
    cu_ptr_sinktor_init(cu_to(cu_ptr_sinktor, self),
			comm_build_sinktor_put,
			comm_build_sinktor_finish);
    self->new_atree = cuex_atree_empty();
    return cu_to(cu_ptr_sinktor, self);
}

static cu_ptr_sinktor_t
comm_union_sinktor(cuex_intf_compound_t impl, cuex_t S)
{
    comm_build_sinktor_t self = cu_gnew(struct comm_build_sinktor_s);
    cu_ptr_sinktor_init(cu_to(cu_ptr_sinktor, self),
			comm_build_sinktor_put,
			comm_build_sinktor_finish);
    self->new_atree = SET(S)->atree;
    return cu_to(cu_ptr_sinktor, self);
}

static struct cuex_intf_compound_s set_compound = {
    .flags = CUEX_COMPOUNDFLAG_PREFER_COMM
	   | CUEX_COMPOUNDFLAG_COMM_IDEMPOTENT,
    .comm_iter_source = &comm_iter_source,
    .comm_build_sinktor = &comm_build_sinktor,
    .comm_union_sinktor = &comm_union_sinktor,
};


/* == Interface Dispatch == */

void
set_print(void *S, FILE *out)
{
    int i = 0;
    void *itr = cu_salloc(cuex_atree_itr_size(S));
    cuex_t e;
    cuex_atree_itr_init(itr, SET(S)->atree);
    fputc('{', out);
    while ((e = cuex_atree_itr_get(itr)))
	cu_fprintf(out, i++? ", %!" : "%!", e);
    fputc('}', out);
}

static cu_word_t
set_dispatch(cu_word_t intf_number, ...)
{
    switch (intf_number) {
	case CUOO_INTF_PRINT_FN:
	    return (cu_word_t)&set_print;
	case CUEX_INTF_COMPOUND:
	    return (cu_word_t)&set_compound;
	default:
	    return CUOO_IMPL_NONE;
    }
}

/* == Initialisation == */

cuoo_stdtype_t cuexP_set_type;
cuex_t cuexP_set_empty;

void
cuexP_set_init()
{
    cuexP_set_type = cuoo_stdtype_new_hcs(
	set_dispatch, sizeof(struct cuex_set_s) - CUOO_HCOBJ_SHIFT);
    cuexP_set_empty = set_new(cuex_atree_empty());
}
