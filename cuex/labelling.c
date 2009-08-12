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
#include <cuex/monoid.h>
#include <cuex/iteration.h>
#include <cuoo/halloc.h>
#include <cuoo/properties.h>
#include <cufo/stream.h>
#include <cufo/tagdefs.h>
#include <cu/ptr_seq.h>

#define l_o2_label cuex_o2_metapair
#define L_O2_LABEL CUEX_O2_METAPAIR

cu_clop_def(get_key, cu_word_t, cuex_t e)
{ return (cu_word_t)cuex_opn_at(e, 0); }

cu_clop_def(value_merge, cuex_t, cuex_t e0, cuex_t e1)
{ return cuex_monoid_product(CUEX_O2_TUPLE, e0, e1); }

#define atree_find(T, l) cuex_atree_find(get_key, T, (cu_word_t)(l))
#define atree_find_index(T, l) \
	cuex_atree_find_index(get_key, T, (cu_word_t)(l))
#define atree_insert(T, e) cuex_atree_insert(get_key, T, e)
#define atree_erase(T, l) cuex_atree_erase(get_key, T, (cu_word_t)(l))
#define atree_find_erase(pT, l) \
	cuex_atree_find_erase(get_key, pT, (cu_word_t)(l))
#define atree_left_union(T0, T1) cuex_atree_left_union(get_key, T0, T1)
#define atree_isecn(T0, T1) cuex_atree_isecn(get_key, T0, T1)
#define atree_order(T0, T1) cuex_atree_order(get_key, T0, T1)
#define atree_subseteq(T0, T1) cuex_atree_subseteq(get_key, T0, T1)

cuoo_type_t cuexP_labelling_type;

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

cuex_t cuexP_labelling_empty;

cuex_t
cuex_labelling_singleton(cuex_t l, cuex_t e)
{
    return labelling(l_o2_label(l, e));
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
    return labelling(atree_insert(LABELLING(L)->atree, l_o2_label(l, e)));
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
    return labelling(cuex_atree_deep_insert(get_key, merge, L,
					    l_o2_label(l, e)));
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

size_t
cuex_labelling_find_index(cuex_t L, cuex_t l)
{
    if (!cuex_is_labelling(L))
	cu_bugf("First argument of cuex_labelling_find_index must be a "
		"labelling.");
    return atree_find_index(LABELLING(L)->atree, l);
}

size_t
cuex_labelling_card(cuex_t L)
{
    if (!cuex_is_labelling(L))
	cu_bugf("First argument of cuex_labelling_card must be a labelling.");
    return cuex_atree_card(LABELLING(L)->atree);
}

cuex_t
cuex_labelling_erase(cuex_t L, cuex_t l)
{
    if (!cuex_is_labelling(L))
	cu_bugf("First argument of cuex_labelling_erase must be a labelling.");
    return labelling(atree_erase(LABELLING(L)->atree, l));
}

cuex_t
cuex_labelling_find_erase(cuex_t *L, cuex_t l)
{
    cuex_t tree, node;
    if (!cuex_is_labelling(*L))
	cu_bugf("First argument of cuex_labelling_erase must be a labelling.");
    tree = LABELLING(*L)->atree;
    node = atree_find_erase(&tree, l);
    if (node) {
	*L = labelling(tree);
	return cuex_opn_at(node, 1);
    }
    else
	return NULL;
}

cuex_t
cuex_labelling_left_union(cuex_t L0, cuex_t L1)
{
    if (!cuex_is_labelling(L0))
	cu_bugf("First argument of cuex_labelling_left_union must be a "
		"labelling.");
    if (!cuex_is_labelling(L1))
	cu_bugf("Second argument of cuex_labelling_left_union must be a "
		"labelling.");
    return labelling(cuex_atree_left_union(get_key,
					   LABELLING(L0)->atree,
					   LABELLING(L1)->atree));
}

cuex_t
cuex_labelling_disjoint_union(cuex_t L0, cuex_t L1)
{
    cuex_t tree;
    if (!cuex_is_labelling(L0))
	cu_bugf("First argument of cuex_labelling_disjoint_union must be a "
		"labelling.");
    if (!cuex_is_labelling(L1))
	cu_bugf("Second argument of cuex_labelling_disjoint_union must be a "
		"labelling.");
    if (!LABELLING(L0)->atree)
	return L1;
    tree = cuex_atree_disjoint_union(get_key,
				     LABELLING(L0)->atree,
				     LABELLING(L1)->atree);
    if (!tree)
	return NULL; /* not disjoint */
    return labelling(tree);
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


/* == Iterative Functions == */

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


/* == Expand and Reduce == */

cuex_t
cuex_labelling_expand_all(cuex_t e)
{
    if (cuex_is_labelling(e)) {
	cu_ptr_source_t source = cuex_labelling_comm_iter_source(e);
	cuex_t ep = cu_ptr_source_get(source);
	if (ep) {
	    cuex_t r = cuex_labelling_expand_all(ep);
	    while ((ep = cu_ptr_source_get(source)))
		r = cuex_monoid_product(CUEX_O2_TUPLE,
					r, cuex_labelling_expand_all(ep));
	    return r;
	} else
	    return e;
    } else
	return cuex_bareimg_operands(cuex_labelling_expand_all, e);
}

cuex_t
cuex_labelling_contract_all(cuex_t e)
{
    if (cuex_is_monoid_nongen(CUEX_O2_TUPLE, e)) {
	cuex_t ep, L, M;
	struct cuex_monoid_itr_s itr;
	cuex_monoid_itr_init_full(CUEX_O2_TUPLE, &itr, e);
	L = cuex_labelling_empty();
	M = cuex_monoid_identity(CUEX_O2_TUPLE);
	while ((ep = cuex_monoid_itr_get(&itr))) {
	    cuex_meta_t ep_meta = cuex_meta(ep);
	    if (ep_meta == L_O2_LABEL) {
		cuex_t l = cuex_opn_at(ep, 0);
		cuex_t v = cuex_labelling_contract_all(cuex_opn_at(ep, 1));
		L = cuex_labelling_insert(L, l, v);
	    }
	    else if (cuex_is_labelling(ep))
		L = cuex_labelling_deep_union(value_merge, L, ep);
	    else {
		cuex_t epp = cuex_labelling_contract_all(ep);
		M = cuex_monoid_product(CUEX_O2_TUPLE, M, epp);
	    }
	}
	if (cuex_is_labelling_empty(L))
	    return M;
	else if (cuex_is_monoid_identity(CUEX_O2_TUPLE, M))
	    return L;
	else
	    return cuex_monoid_product(CUEX_O2_TUPLE, M, L);
    } else if (cuex_meta(e) == L_O2_LABEL) {
	cuex_t l = cuex_opn_at(e, 0);
	cuex_t v = cuex_opn_at(e, 1);
	return cuex_labelling_singleton(l, v);
    } else
	return cuex_bareimg_operands(cuex_labelling_contract_all, e);
}


/* == Print == */

cu_clos_def(_labelling_print_elt, cu_prot(void, cuex_t e),
  ( FILE *out;
    int index; ))
{
    cuex_t l, x;
    cu_clos_self(_labelling_print_elt);
    if (self->index++)
	fputs(", ", self->out);
    cu_debug_assert(cuex_meta_is_opr(cuex_meta(e)) &&
		    cuex_opr_r(cuex_meta(e)) >= 2);
    l = cuex_opn_at(e, 0);
    x = cuex_opn_at(e, 1);
    cu_fprintf(self->out, "%!: %!", l, x);
}

static void
_labelling_print(cuex_t L, FILE *out)
{
    _labelling_print_elt_t cl;
    fputc('{', out);
    cl.out = out;
    cl.index = 0;
    cuex_atree_iter(LABELLING(L)->atree, _labelling_print_elt_prep(&cl));
    fputc('}', out);
}

cu_clos_def(_labelling_foprint_elt, cu_prot(void, cuex_t e),
  ( cufo_stream_t fos;
    int index; ))
{
    cu_clos_self(_labelling_foprint_elt);
    cuex_t l, x;
    if (self->index++) {
	cufo_enter(self->fos, cufoT_operator);
	cufo_putc(self->fos, ',');
	cufo_leave(self->fos, cufoT_operator);
	cufo_putc(self->fos, ' ');
    }
    cu_debug_assert(cuex_meta_is_opr(cuex_meta(e)) &&
		    cuex_opr_r(cuex_meta(e)) >= 2);
    l = cuex_opn_at(e, 0);
    x = cuex_opn_at(e, 1);
    cufo_enter(self->fos, cufoT_label);
    cufo_print_ex(self->fos, l);
    cufo_putc(self->fos, ':');
    cufo_leave(self->fos, cufoT_label);
    cufo_putc(self->fos, ' ');
    cufo_print_ex(self->fos, x);
}

static void
_labelling_foprint(cufo_stream_t fos, cufo_prispec_t spec, cuex_t L)
{
    _labelling_foprint_elt_t cl;
    cufo_enter(fos, cufoT_operator);
    cufo_putc(fos, '{');
    cufo_leave(fos, cufoT_operator);
    cl.fos = fos;
    cl.index = 0;
    cuex_atree_iter(LABELLING(L)->atree, _labelling_foprint_elt_prep(&cl));
    cufo_enter(fos, cufoT_operator);
    cufo_putc(fos, '}');
    cufo_leave(fos, cufoT_operator);
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

cu_ptr_source_t
cuex_labelling_ncomm_iter_source(cuex_t L)
{
    cu_debug_assert(cuex_is_labelling(L));
    return ncomm_iter_source(NULL, L);
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

cu_ptr_source_t
cuex_labelling_comm_iter_source(cuex_t L)
{
    cu_debug_assert(cuex_is_labelling(L));
    return comm_iter_source(NULL, L);
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
    self->new_atree = atree_insert(self->new_atree, l_o2_label(l, elt));
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

cu_ptr_junctor_t
cuex_labelling_ncomm_image_junctor(cuex_t L)
{
    return ncomm_image_junctor(NULL, L);
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
    if (cuex_meta(elt) == L_O2_LABEL)
	self->new_atree = atree_insert(self->new_atree, elt);
    else if (cuex_is_labelling(elt))
	self->new_atree
	    = atree_left_union(self->new_atree, LABELLING(elt)->atree);
    else
	cu_bugf("Tried to insert unlabelled element into a "
		"cuex_labelling_comm_build_sinktor.");
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

cu_ptr_sinktor_t
cuex_labelling_comm_build_sinktor(void)
{ return comm_build_sinktor(NULL, NULL); }

cu_ptr_sinktor_t
cuex_labelling_comm_union_sinktor(cuex_t L)
{ return comm_union_sinktor(NULL, L); }

#if 0
static cuex_t
comm_find(cuex_t L, cuex_t l)
{
    return atree_find(LABELLING(L)->atree, l);
}
#endif


/* == Compound Interface == */

static struct cuex_intf_compound_s _labelling_compound = {
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

static cu_box_t
_labelling_impl(cu_word_t intf_number, ...)
{
    switch (intf_number) {
	case CUOO_INTF_PRINT_FN:
	    return CUOO_INTF_PRINT_FN_BOX(_labelling_print);
	case CUOO_INTF_FOPRINT_FN:
	    return CUOO_INTF_FOPRINT_FN_BOX(_labelling_foprint);
	case CUEX_INTF_COMPOUND:
	    return CUEX_INTF_COMPOUND_BOX(&_labelling_compound);
	default:
	    return CUOO_IMPL_NONE;
    }
}


/* == Initialisation == */

void
cuexP_labelling_init(void)
{
    cuex_intf_compound_finish(&_labelling_compound);
    cuexP_labelling_type = cuoo_type_new_opaque_hcs(
	_labelling_impl, sizeof(struct cuex_labelling_s) - CUOO_HCOBJ_SHIFT);
    cuexP_labelling_empty = labelling(NULL);
}
