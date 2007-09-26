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

#include <cudyn/misc.h>
#include <cuex/compound.h>
#include <cuex/oprdefs.h>
#include <cuex/opn.h>
#include <cu/diag.h>
#include <cu/memory.h>
#include <cu/ptr_seq.h>

/* == Commutative from Non-Commutative Source Adaptor == */

typedef struct comm_by_ncomm_iter_adaptor_s *comm_by_ncomm_iter_adaptor_t;
struct comm_by_ncomm_iter_adaptor_s
{
    cu_inherit (cu_ptr_source_s);
    cu_ptr_source_t subsource;
    int index;
};

static void *
comm_by_ncomm_iter_adaptor_get(cu_ptr_source_t source)
{
    cuex_t ncomm_e;
    comm_by_ncomm_iter_adaptor_t self
	= cu_from(comm_by_ncomm_iter_adaptor, cu_ptr_source, source);
    ncomm_e = cu_ptr_source_get(self->subsource);
    if (ncomm_e)
	return cuex_o2_metapair(cudyn_int(self->index++), ncomm_e);
    else
	return NULL;
}

static cu_ptr_source_t
comm_by_ncomm_iter_adaptor(cuex_intf_compound_t impl, cuex_t C)
{
    comm_by_ncomm_iter_adaptor_t self
	= cu_gnew(struct comm_by_ncomm_iter_adaptor_s);
    cu_ptr_source_init(cu_to(cu_ptr_source, self),
		       comm_by_ncomm_iter_adaptor_get);
    self->subsource = impl->ncomm_iter_source(impl, C);
    self->index = 0;
    return cu_to(cu_ptr_source, self);
}


/* == Commutative from Non-Commutative Image Junctor Adaptor == */

typedef struct comm_by_ncomm_image_adaptor_s *comm_by_ncomm_image_adaptor_t;
struct comm_by_ncomm_image_adaptor_s
{
    cu_inherit (cu_ptr_junctor_s);
    cu_ptr_junctor_t subjunctor;
    int index;
};

static void *
comm_by_ncomm_image_adaptor_get(cu_ptr_source_t source)
{
    cuex_t ncomm_e;
    comm_by_ncomm_image_adaptor_t self
	= cu_from3(comm_by_ncomm_image_adaptor,
		   cu_ptr_junctor, cu_ptr_junction, cu_ptr_source,
		   source);
    ncomm_e = cu_ptr_junctor_get(self->subjunctor);
    if (ncomm_e)
	return cuex_o2_metapair(cudyn_int(++self->index), ncomm_e);
    else
	return NULL;
}

static void
comm_by_ncomm_image_adaptor_put(cu_ptr_sink_t sink, void *elt)
{
    cuex_t elt_key, elt_val;
    cuex_meta_t e_meta = cuex_meta(elt);
    comm_by_ncomm_image_adaptor_t self
	= cu_from3(comm_by_ncomm_image_adaptor,
		   cu_ptr_junctor, cu_ptr_junction, cu_ptr_sink,
		   sink);
    cu_debug_assert(cuex_meta_is_opr(e_meta) && cuex_opr_r(e_meta) == 2);
    elt_key = cuex_opn_at(elt, 0);
    elt_val = cuex_opn_at(elt, 1);
    cu_debug_assert(cudyn_is_int(elt_key));
    cu_debug_assert(cudyn_to_int(elt_key) == self->index);
    cu_ptr_junctor_put(self->subjunctor, elt_val);
}

static void *
comm_by_ncomm_image_adaptor_finish(cu_ptr_junctor_t junctor)
{
    comm_by_ncomm_image_adaptor_t self
	= cu_from(comm_by_ncomm_image_adaptor, cu_ptr_junctor, junctor);
    return cu_ptr_junctor_finish(self->subjunctor);
}

static cu_ptr_junctor_t
comm_by_ncomm_image_adaptor(cuex_intf_compound_t impl, cuex_t C)
{
    comm_by_ncomm_image_adaptor_t self
	= cu_gnew(struct comm_by_ncomm_image_adaptor_s);
    cu_ptr_junctor_init(cu_to(cu_ptr_junctor, self),
			comm_by_ncomm_image_adaptor_get,
			comm_by_ncomm_image_adaptor_put,
			comm_by_ncomm_image_adaptor_finish);
    self->subjunctor = impl->ncomm_image_junctor(impl, C);
    self->index = -1;
    return cu_to(cu_ptr_junctor, self);
}


/* == Commutative Image Junctor from Build Sinktor Adaptor == */

typedef struct comm_image_from_build_adaptor_s
	*comm_image_from_build_adaptor_t;
struct comm_image_from_build_adaptor_s
{
    cu_inherit (cu_ptr_junctor_s);
    cu_ptr_source_t subsource;
    cu_ptr_sinktor_t subsinktor;
};

static void *
comm_image_from_build_adaptor_get(cu_ptr_source_t source)
{
    comm_image_from_build_adaptor_t self
	= cu_from3(comm_image_from_build_adaptor,
		   cu_ptr_junctor, cu_ptr_junction, cu_ptr_source, source);
    return cu_ptr_source_get(self->subsource);
}

static void
comm_image_from_build_adaptor_put(cu_ptr_sink_t sink, void *elt)
{
    comm_image_from_build_adaptor_t self
	= cu_from3(comm_image_from_build_adaptor,
		   cu_ptr_junctor, cu_ptr_junction, cu_ptr_sink, sink);
    cu_ptr_sinktor_put(self->subsinktor, elt);
}

static void *
comm_image_from_build_adaptor_finish(cu_ptr_junctor_t junctor)
{
    comm_image_from_build_adaptor_t self
	= cu_from(comm_image_from_build_adaptor, cu_ptr_junctor, junctor);
    return cu_ptr_sinktor_finish(self->subsinktor);
}

static cu_ptr_junctor_t
comm_image_from_build_adaptor(cuex_intf_compound_t impl, cuex_t C)
{
    comm_image_from_build_adaptor_t self
	= cu_gnew(struct comm_image_from_build_adaptor_s);
    cu_ptr_junctor_init(cu_to(cu_ptr_junctor, self),
			comm_image_from_build_adaptor_get,
			comm_image_from_build_adaptor_put,
			comm_image_from_build_adaptor_finish);
    self->subsource = impl->comm_iter_source(impl, C);
    self->subsinktor = impl->comm_build_sinktor(impl, C);
    return cu_to(cu_ptr_junctor, self);
}


/* == Other Defaults == */

static size_t
compound_size(cuex_intf_compound_t impl, cuex_t C)
{
    cu_ptr_source_t source = cuex_compound_pref_iter_source(impl, C);
    return cu_ptr_source_count(source);
}


/* == Interface Verification == */

void
cuex_intf_compound_finish(cuex_intf_compound_t impl)
{
    unsigned int flags = impl->flags;
    cu_bool_t has_comm_iter = !!impl->comm_iter_source;
    cu_bool_t has_ncomm_iter = !!impl->ncomm_iter_source;

    /* Check that we have at least one iteration source, and that the flags
     * points to the correct one. */
    if (!has_comm_iter && !has_ncomm_iter)
	cu_bugf("You need to define either non-commutative or "
		"commutative iteration.");
    if (flags & CUEX_COMPOUNDFLAG_PREFER_COMM) {
	if (!has_comm_iter)
	    cu_bugf("Flag CUEX_COMPOUNDFLAG_PREFER_COMM is set but no "
		    "commutative iteration view is provided.");
    }
    if (flags & CUEX_COMPOUNDFLAG_PREFER_NCOMM) {
	if (!has_ncomm_iter)
	    cu_bugf("Flag CUEX_COMPOUNDFLAG_PREFER_NCOMM is set but no "
		    "non-commutative iteration view is provided.");
    }
    if (!(flags & (CUEX_COMPOUNDFLAG_PREFER_COMM |
		   CUEX_COMPOUNDFLAG_PREFER_NCOMM))) {
	if (!has_comm_iter)
	    cu_bugf("You must set CUEX_COMPOUNDFLAGS_PREFER_NCOMM since only "
		    "non-commutative iteration view is provided.");
	if (!has_ncomm_iter)
	    cu_bugf("You must set CUEX_COMPOUNDFLAGS_PREFER_COMM since only "
		    "commutative iteration view is provided.");
    }

    /* Check that any constructing callbacks have a corresponding iteration
     * source. */
    if (!has_ncomm_iter)
	if (impl->ncomm_image_junctor)
	    cu_bugf("The non-commutative image junctor (ncomm_image_junctor) "
		    "must have a corresponding iterator source "
		    "(ncomm_iter_source).");
    if (!has_comm_iter) {
	if (impl->comm_image_junctor)
	    cu_bugf("The commutative image junctor (comm_image_junctor) "
		    "must have a corresponding iterator source "
		    "(comm_iter_source).");
	if (impl->comm_build_sinktor)
	    cu_bugf("The commutative build sinktor (comm_build_sinktor) "
		    "must have a corresponding iterator source "
		    "(comm_iter_source).");
	if (impl->comm_union_sinktor)
	    cu_bugf("The commutative union sinktor (comm_union_sinktor) "
		    "must have a corresponding iterator source ");
    }

    /* Synthesis of commutative view if not present. */
    if (!impl->comm_iter_source) {
	cu_debug_assert(impl->ncomm_iter_source);
	impl->comm_iter_source = comm_by_ncomm_iter_adaptor;
    }
    if (!impl->comm_image_junctor) {
	if (impl->comm_build_sinktor) {
	    /* Use the more general construction from the commutative view if
	     * present. */
	    impl->comm_image_junctor = comm_image_from_build_adaptor;
	    impl->flags |= CUEX_COMPOUNDFLAG_COMM_FILTERABLE_IMAGE
			 | CUEX_COMPOUNDFLAG_COMM_EXPANSIVE_IMAGE;
	}
	else if (impl->ncomm_image_junctor) {
	    if (has_comm_iter)
		cu_bugf("Cannot synthesise the commutative image junctor "
			"(comm_image_junctor) from non-commutative view "
			"since commutative iteration (comm_iter_source) "
			"is already defined.  Either define "
			"comm_image_junctor, or remove comm_iter_source.");
	    /* The commutative iteration source is synthesised, so we can
	     * synthesise a matching commutative image junctor. */
	    impl->comm_image_junctor = comm_by_ncomm_image_adaptor;
	    if (impl->flags & CUEX_COMPOUNDFLAG_NCOMM_FILTERABLE_IMAGE)
		impl->flags |= CUEX_COMPOUNDFLAG_COMM_FILTERABLE_IMAGE;
	    if (impl->flags & CUEX_COMPOUNDFLAG_NCOMM_EXPANSIVE_IMAGE)
		impl->flags |= CUEX_COMPOUNDFLAG_COMM_EXPANSIVE_IMAGE;
	}
    }

    /* Synthesis of various methods. */
    if (!impl->size)
	impl->size = compound_size;
}


/* == Algorithms == */

cu_ptr_source_t
cuex_compound_pref_iter_source(cuex_intf_compound_t impl, cuex_t C)
{
    if (impl->flags & CUEX_COMPOUNDFLAG_PREFER_NCOMM)
	return impl->ncomm_iter_source(impl, C);
    else
	return impl->comm_iter_source(impl, C);
}

cu_ptr_junctor_t
cuex_compound_pref_image_junctor(cuex_intf_compound_t impl, cuex_t C)
{
    if (impl->flags & CUEX_COMPOUNDFLAG_PREFER_NCOMM)
	return impl->ncomm_image_junctor(impl, C);
    else
	return impl->comm_image_junctor(impl, C);
}

#if 0
cu_bool_t
cuex_compound_conj(cuex_intf_compound_t impl, cuex_t C,
		   cu_clop(f, cu_bool_t, cuex_t))
{
    cuex_t e;
    cu_ptr_source_t source = cuex_compound_preferred_iter_source(impl, C);
    while ((e = cu_ptr_source_get(source)))
	if (!cu_call(f, e))
	    return cu_false;
    return cu_true;
}

cuex_t
cuex_compound_image(cuex_intf_compound_t impl, cuex_t C,
		    cu_clop(f, cuex_t, cuex_t))
{
    cuex_t e;
    if ((impl->flags & CUEX_COMPOUNDFLAG_PREFER_COMM)) {
	cu_ptr_source_t source = (*impl->comm_iter_source)(impl, C);
	cu_ptr_sinktor_t sinktor = (*impl->comm_build_sinktor)(impl, C);
	while ((e = cu_ptr_source_get(source))) {
	    e = cu_call(f, e);
	    if (!e)
		return NULL;
	    cu_ptr_sinktor_put(sinktor, e);
	}
	return cu_ptr_sinktor_finish(sinktor);
    }
    else {
	cu_ptr_junctor_t junctor = (*impl->ncomm_image_junctor)(impl, C);
	while ((e = cu_ptr_junctor_get(junctor))) {
	    e = cu_call(f, e);
	    if (!e)
		return NULL;
	    cu_ptr_junctor_put(junctor, e);
	}
	return cu_ptr_junctor_finish(junctor);
    }
}
#endif
