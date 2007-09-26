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

#include <cuex/labelling.h>
#include <cuex/intf.h>
#include <cuex/compound.h>
#include <cuex/opn.h>
#include <cuex/oprdefs.h>
#include <cuex/atree.h>
#include <cuoo/halloc.h>
#include <cuoo/properties.h>
#include <cu/ptr_seq.h>

#define tuple(l, e) cuex_opn(CUEX_OR_TUPLE(2), l, e)

cu_clop_def(get_key, cu_word_t, cuex_t e)
{ return (cu_word_t)cuex_opn_at(e, 0); }

#define atree_find(T, l) cuex_atree_find(get_key, T, (cu_word_t)(l))
#define atree_insert(T, e) cuex_atree_insert(get_key, T, e)
#define atree_erase(T, l) cuex_atree_erase(get_key, T, (cu_word_t)(l))
#define atree_union(T0, T1) cuex_atree_union(get_key, T0, T1)
#define atree_isecn(T0, T1) cuex_atree_isecn(get_key, T0, T1)
#define atree_order(T0, T1) cuex_atree_order(get_key, T0, T1)
#define atree_subseteq(T0, T1) cuex_atree_subseteq(get_key, T0, T1)

cuoo_stdtype_t cuexP_labelling_type;

typedef struct cuex_labelling_s *cuex_labelling_t;
struct cuex_labelling_s
{
    CUOO_HCOBJ
    cuex_t atree;
};

#define LABELLING(e) ((cuex_labelling_t)(e))

static cuex_labelling_t
labelling(cuex_t atree)
{
    cuoo_hctem_decl(cuex_labelling, key);
    cuoo_hctem_init(cuex_labelling, key);
    cuoo_hctem_get(cuex_labelling, key)->atree = atree;
    return cuoo_hctem_new(cuex_labelling, key);
}

cuex_t
cuex_labelling_empty(void)
{
    return labelling(NULL);
}

cuex_t
cuex_labelling_singleton(cuex_t l, cuex_t e)
{
    return labelling(tuple(l, e));
}

cuex_t
cuex_labelling_by_arglist(cuex_t l, cuex_t e, ...)
{
    cuex_t L;
    va_list va;
    va_start(va, e);
    L = cuex_labelling_insert_valist(cuex_labelling_singleton(l, e), va);
    va_end(va);
    return L;
}

cuex_t
cuex_labelling_insert(cuex_t L, cuex_t l, cuex_t e)
{
    if (!cuex_is_labelling(L))
	cu_bugf("First argument of cuex_labelling_insert must be a "
		"labelling.");
    return labelling(atree_insert(LABELLING(L)->atree, tuple(l, e)));
}

cuex_t
cuex_labelling_insert_valist(cuex_t L, va_list va)
{
    cuex_t l, v;
    while ((l = va_arg(va, cuex_t)) != NULL) {
	v = va_arg(va, cuex_t);
	cu_debug_assert(v);
	L = cuex_labelling_insert(L, l, v);
    }
    return L;
}

cuex_t
cuex_labelling_insert_arglist(cuex_t L, ...)
{
    va_list va;
    va_start(va, L);
    L = cuex_labelling_insert_valist(L, va);
    va_end(va);
    return L;
}

cuex_t
cuex_labelling_deep_insert(cu_clop(merge, cuex_t, cuex_t, cuex_t),
			   cuex_t L, cuex_t l, cuex_t e)
{
    if (!cuex_is_labelling(L))
	cu_bugf("Second argument of cuex_labelling_insert must be a "
		"labelling.");
    return labelling(cuex_atree_deep_insert(get_key, merge, L, tuple(l, e)));
}

cuex_t
cuex_labelling_find(cuex_t L, cuex_t l)
{
    cuex_t e;
    if (!cuex_is_labelling(L))
	cu_bugf("First argument of cuex_labelling_find must be a labelling.");
    e = atree_find(LABELLING(L)->atree, l);
    if (e)
	return cuex_opn_at(e, 1);
    else
	return NULL;
}

cuex_t
cuex_labelling_left_union(cuex_t L0, cuex_t L1)
{
    if (!cuex_is_labelling(L0))
	cu_bugf("First argument of cuex_labelling_union must be a labelling.");
    if (!cuex_is_labelling(L1))
	cu_bugf("Second argument of cuex_labelling_union must be a "
		"labelling.");
    return labelling(cuex_atree_left_union(get_key,
					   LABELLING(L0)->atree,
					   LABELLING(L1)->atree));
}

cuex_t
cuex_labelling_deep_union(cu_clop(merge, cuex_t, cuex_t, cuex_t),
			  cuex_t L0, cuex_t L1)
{
    if (!cuex_is_labelling(L0))
	cu_bugf("First argument of cuex_labelling_deep_union must be a "
		"labelling.");
    if (!cuex_is_labelling(L1))
	cu_bugf("Second argument of cuex_labelling_deep_union must be a "
		"labelling.");
    return labelling(cuex_atree_deep_union_iv(get_key, 1, merge,
					      LABELLING(L0)->atree,
					      LABELLING(L1)->atree));
}

cuex_t
cuex_labelling_left_isecn(cuex_t L0, cuex_t L1)
{
    if (!cuex_is_labelling(L0))
	cu_bugf("First argument of cuex_labelling_left_isecn must be a "
		"labelling.");
    if (!cuex_is_labelling(L1))
	cu_bugf("Second argument of cuex_labelling_left_isecn must be a "
		"labelling.");
    return labelling(cuex_atree_left_isecn(get_key,
					   LABELLING(L0)->atree,
					   LABELLING(L1)->atree));
}

cuex_t
cuex_labelling_deep_isecn(cu_clop(merge, cuex_t, cuex_t, cuex_t),
			  cuex_t L0, cuex_t L1)
{
    if (!cuex_is_labelling(L0))
	cu_bugf("First argument of cuex_labelling_deep_isecn must be a "
		"labelling.");
    if (!cuex_is_labelling(L1))
	cu_bugf("Second argument of cuex_labelling_deep_isecn must be a "
		"labelling.");
    return labelling(cuex_atree_deep_isecn_iv(get_key, 1, merge,
					      LABELLING(L0)->atree,
					      LABELLING(L1)->atree));
}

cu_bool_t
cuex_labelling_conj_kv(cuex_t L, cu_clop(f, cu_bool_t, cuex_t l, cuex_t e))
{
    if (!cuex_is_labelling(L))
	cu_bugf("First argument of cuex_labelling_conj must be a labelling.");
    return cuex_atree_conj_kv(LABELLING(L)->atree, f);
}

cuex_t
cuex_labelling_image(cuex_t L, cu_clop(f, cuex_t, cuex_t v))
{
    if (!cuex_is_labelling(L))
	cu_bugf("First argument of cuex_labelling_image must be a labelling.");
    return labelling(cuex_atree_isokey_image_iv(LABELLING(L)->atree, 1, f));
}

cuex_t
cuex_labelling_image_kv(cuex_t L, cu_clop(f, cuex_t, cuex_t l, cuex_t e))
{
    if (!cuex_is_labelling(L))
	cu_bugf("First argument of cuex_labelling_image must be a labelling.");
    return labelling(cuex_atree_isokey_image_kv(LABELLING(L)->atree, f));
}

cu_clop_def(labelling_conj, cu_bool_t, cuex_t L, cu_clop(f, cu_bool_t, cuex_t))
{
    return cuex_atree_conj_iv(LABELLING(L)->atree, 1, f);
}

cu_clop_def(labelling_tran, cuex_t, cuex_t L, cu_clop(f, cuex_t, cuex_t))
{
    return labelling(cuex_atree_isokey_image_iv(LABELLING(L)->atree, 1, f));
}

cu_clos_def(labelling_print_elt, cu_prot(void, cuex_t e),
  ( FILE *out;
    int index; ))
{
    cuex_t l, x;
    cu_clos_self(labelling_print_elt);
    if (self->index++)
	fputs(", ", self->out);
    cu_debug_assert(cuex_meta_is_opr(cuex_meta(e)) &&
		    cuex_opr_r(cuex_meta(e)) >= 2);
    l = cuex_opn_at(e, 0);
    x = cuex_opn_at(e, 1);
    cu_fprintf(self->out, "%!: %!", l, x);
}

static void
labelling_print(void *L, FILE *out)
{
    labelling_print_elt_t cl;
    fputc('(', out);
    cl.out = out;
    cl.index = 0;
    cuex_atree_iter(LABELLING(L)->atree, labelling_print_elt_prep(&cl));
    fputc(')', out);
}


/* == Compound Interface: Iteration == */

static void *
ncomm_iter_source_get(cu_ptr_source_t source)
{
    return cuex_atree_itr_get_at_1(source + 1);
}

static cu_ptr_source_t
ncomm_iter_source(cuex_intf_compound_t impl, cuex_t L)
{
    cu_ptr_source_t source;
    size_t atree_itr_size = cuex_atree_itr_size(LABELLING(L)->atree);
    source = cu_galloc(sizeof(struct cu_ptr_source_s) + atree_itr_size);
    cuex_atree_itr_init(source + 1, LABELLING(L)->atree);
    cu_ptr_source_init(source, ncomm_iter_source_get);
    return source;
}

static void *
comm_iter_source_get(cu_ptr_source_t source)
{
    return cuex_atree_itr_get(source + 1);
}

static cu_ptr_source_t
comm_iter_source(cuex_intf_compound_t impl, cuex_t L)
{
    cu_ptr_source_t source;
    size_t atree_itr_size = cuex_atree_itr_size(LABELLING(L)->atree);
    source = cu_galloc(sizeof(struct cu_ptr_source_s) + atree_itr_size);
    cuex_atree_itr_init(source + 1, LABELLING(L)->atree);
    cu_ptr_source_init(source, comm_iter_source_get);
    return source;
}


/* == Compound Interface: Non-commutative == */

typedef struct ncomm_image_junctor_s *ncomm_image_junctor_t;
struct ncomm_image_junctor_s
{
    cu_inherit (cu_ptr_junctor_s);
    cuex_t new_atree;
    cuex_t current_label;
    /* atree iterator follows */
};
#define IMAGE_ITR(itr) ((struct ncomm_image_junctor_s *)(itr))
#define IMAGE_ITR_ATREE_ITR(itr) \
    ((void *)((struct ncomm_image_junctor_s *)(itr) + 1))

static cuex_t
ncomm_image_junctor_get(cu_ptr_source_t source)
{
    ncomm_image_junctor_t self
	= cu_from3(ncomm_image_junctor,
		   cu_ptr_junctor, cu_ptr_junction, cu_ptr_source, source);
    cuex_t leaf = cuex_atree_itr_get(self + 1);
    if (leaf) {
	self->current_label = cuex_opn_at(leaf, 0);
	return cuex_opn_at(leaf, 1);
    }
    else
	return NULL;
}

static void
ncomm_image_junctor_put(cu_ptr_sink_t sink, void *elt)
{
    ncomm_image_junctor_t self
	= cu_from3(ncomm_image_junctor,
		   cu_ptr_junctor, cu_ptr_junction, cu_ptr_sink, sink);
    cuex_t l = self->current_label;
#ifndef CU_NDEBUG_CLIENT
    if (!l)
	cu_bugf("The put operation on the imageable-interface of a labelling "
		"does not correspond to a previous get.  Attempted to put "
		"%! into %!.", elt, self->new_atree);
    cu_debug_assert(l);
#endif
    self->new_atree = atree_insert(self->new_atree, tuple(l, elt));
#ifndef CU_NDEBUG_CLIENT
    self->current_label = NULL;
#endif
}

static cuex_t
ncomm_image_junctor_finish(cu_ptr_junctor_t junctor)
{
    ncomm_image_junctor_t self
	= cu_from(ncomm_image_junctor, cu_ptr_junctor, junctor);
    return labelling(self->new_atree);
}

static cu_ptr_junctor_t
ncomm_image_junctor(cuex_intf_compound_t impl, cuex_t L)
{
    size_t atree_itr_size = cuex_atree_itr_size(LABELLING(L)->atree);
    ncomm_image_junctor_t self
	= cu_galloc(sizeof(struct ncomm_image_junctor_s) + atree_itr_size);
    cuex_atree_itr_init(self + 1, LABELLING(L)->atree);
    self->new_atree = cuex_atree_empty();
    self->current_label = NULL;
    cu_ptr_junctor_init(cu_to(cu_ptr_junctor, self),
			ncomm_image_junctor_get,
			ncomm_image_junctor_put,
			ncomm_image_junctor_finish);
    return cu_to(cu_ptr_junctor, self);
}


/* == Compound Interface: Commutative */

typedef struct comm_build_sinktor_s *comm_build_sinktor_t;
struct comm_build_sinktor_s
{
    cu_inherit (cu_ptr_sinktor_s);
    cuex_t new_atree;
};

static void
comm_build_sinktor_put(cu_ptr_sink_t sink, void *elt)
{
    comm_build_sinktor_t self
	= cu_from2(comm_build_sinktor, cu_ptr_sinktor, cu_ptr_sink, sink);
    self->new_atree = atree_insert(self->new_atree, elt);
}

static void *
comm_build_sinktor_finish(cu_ptr_sinktor_t sinktor)
{
    comm_build_sinktor_t self
	= cu_from(comm_build_sinktor, cu_ptr_sinktor, sinktor);
    return labelling(self->new_atree);
}

static cu_ptr_sinktor_t
comm_build_sinktor(cuex_intf_compound_t impl, cuex_t L)
{
    comm_build_sinktor_t self = cu_gnew(struct comm_build_sinktor_s);
    cu_ptr_sinktor_init(cu_to(cu_ptr_sinktor, self),
			comm_build_sinktor_put,
			comm_build_sinktor_finish);
    self->new_atree = cuex_atree_empty();
    return cu_to(cu_ptr_sinktor, self);
}

static cu_ptr_sinktor_t
comm_union_sinktor(cuex_intf_compound_t impl, cuex_t L)
{
    comm_build_sinktor_t self = cu_gnew(struct comm_build_sinktor_s);
    cu_ptr_sinktor_init(cu_to(cu_ptr_sinktor, self),
			comm_build_sinktor_put,
			comm_build_sinktor_finish);
    self->new_atree = LABELLING(L)->atree;
    return cu_to(cu_ptr_sinktor, self);
}

#if 0
static cuex_t
comm_find(cuex_t L, cuex_t l)
{
    return atree_find(LABELLING(L)->atree, l);
}
#endif


/* == Compound Interface == */

static struct cuex_intf_compound_s labelling_compound = {
    .flags = CUEX_COMPOUNDFLAG_PREFER_NCOMM
	   | CUEX_COMPOUNDFLAG_NCOMM_FILTERABLE_IMAGE
	   | CUEX_COMPOUNDFLAG_COMM_IDEMPOTENT,
    .ncomm_iter_source = &ncomm_iter_source,
    .ncomm_image_junctor = &ncomm_image_junctor,
    .comm_iter_source = &comm_iter_source,
    .comm_build_sinktor = &comm_build_sinktor,
    .comm_union_sinktor = &comm_union_sinktor,
#if 0
    .comm_find = comm_find,
#endif
};


/* == Implementation Dispatcher == */

static cu_word_t
labelling_impl(cu_word_t intf_number, ...)
{
    switch (intf_number) {
	case CUOO_INTF_PRINT_FN:
	    return (cu_word_t)labelling_print;
	case CUEX_INTF_COMPOUND:
	    return (cu_word_t)&labelling_compound;
	default:
	    return CUOO_IMPL_NONE;
    }
}


/* == Initialisation == */

void
cuexP_labelling_init(void)
{
    cuex_intf_compound_finish(&labelling_compound);
    cuexP_labelling_type = cuoo_stdtype_new_hcs(
	labelling_impl, sizeof(struct cuex_labelling_s) - CUOO_HCOBJ_SHIFT);
    cuexP_labelling_type->conj = labelling_conj;
    cuexP_labelling_type->tran = labelling_tran;
}
