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

typedef struct comm_source_adaptor_s *comm_source_adaptor_t;
struct comm_source_adaptor_s
{
    cu_inherit (cu_ptr_source_s);
    cu_ptr_source_t subsource;
    int index;
};

static void *
comm_source_adaptor_get(cu_ptr_source_t source)
{
    cuex_t ncomm_e;
    comm_source_adaptor_t self
	= cu_from(comm_source_adaptor, cu_ptr_source, source);
    ncomm_e = cu_ptr_source_get(self->subsource);
    if (ncomm_e)
	return cuex_o2_metapair(cudyn_int(self->index++), ncomm_e);
    else
	return NULL;
}

static cu_ptr_source_t
comm_source_adaptor(cuex_intf_compound_t impl, cuex_t C)
{
    comm_source_adaptor_t self = cu_gnew(struct comm_source_adaptor_s);
    cu_ptr_source_init(cu_to(cu_ptr_source, self), comm_source_adaptor_get);
    self->subsource = impl->ncomm_source(impl, C);
    self->index = 0;
    return cu_to(cu_ptr_source, self);
}


/* == Commutative from Non-Commutative Image Junctor Adaptor == */

typedef struct comm_image_junctor_adaptor_s *comm_image_junctor_adaptor_t;
struct comm_image_junctor_adaptor_s
{
    cu_inherit (cu_ptr_junctor_s);
    cu_ptr_junctor_t subjunctor;
    int index;
};

static void *
comm_image_junctor_adaptor_get(cu_ptr_source_t source)
{
    cuex_t ncomm_e;
    comm_image_junctor_adaptor_t self
	= cu_from3(comm_image_junctor_adaptor,
		   cu_ptr_junctor, cu_ptr_junction, cu_ptr_source,
		   source);
    ncomm_e = cu_ptr_junctor_get(self->subjunctor);
    if (ncomm_e)
	return cuex_o2_metapair(cudyn_int(++self->index), ncomm_e);
    else
	return NULL;
}

static void
comm_image_junctor_adaptor_put(cu_ptr_sink_t sink, void *elt)
{
    cuex_t elt_key, elt_val;
    cuex_meta_t e_meta = cuex_meta(elt);
    comm_image_junctor_adaptor_t self
	= cu_from3(comm_image_junctor_adaptor,
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
comm_image_junctor_adaptor_finish(cu_ptr_junctor_t junctor)
{
    comm_image_junctor_adaptor_t self
	= cu_from(comm_image_junctor_adaptor, cu_ptr_junctor, junctor);
    return cu_ptr_junctor_finish(self->subjunctor);
}

static cu_ptr_junctor_t
comm_image_junctor_adaptor(cuex_intf_compound_t impl, cuex_t C)
{
    comm_image_junctor_adaptor_t self
	= cu_gnew(struct comm_image_junctor_adaptor_s);
    cu_ptr_junctor_init(cu_to(cu_ptr_junctor, self),
			comm_image_junctor_adaptor_get,
			comm_image_junctor_adaptor_put,
			comm_image_junctor_adaptor_finish);
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
    self->subsource = impl->comm_source(impl, C);
    self->subsinktor = impl->comm_build_sinktor(impl, C);
    return cu_to(cu_ptr_junctor, self);
}


/* == Interface Verification == */

void
cuex_intf_compound_verify(cuex_intf_compound_t impl)
{
    unsigned int flags = impl->flags;
    cu_bool_t has_comm = !!impl->comm_source;
    cu_bool_t has_ncomm = !!impl->ncomm_source;
    if (!has_comm && !has_ncomm)
	cu_bugf("You need to define either non-commutative or "
		"commutative iteration.");
    if (flags & CUEX_COMPOUNDFLAG_PREFER_COMM) {
	if (!has_comm)
	    cu_bugf("Flag CUEX_COMPOUNDFLAG_PREFER_COMM is set but no "
		    "commutative iteration view is provided.");
    }
    if (flags & CUEX_COMPOUNDFLAG_PREFER_NCOMM) {
	if (!has_ncomm)
	    cu_bugf("Flag CUEX_COMPOUNDFLAG_PREFER_NCOMM is set but no "
		    "non-commutative iteration view is provided.");
    }
    if (!(flags & (CUEX_COMPOUNDFLAG_PREFER_COMM |
		   CUEX_COMPOUNDFLAG_PREFER_NCOMM))) {
	if (!has_comm)
	    cu_bugf("You must set CUEX_COMPOUNDFLAGS_PREFER_NCOMM since only "
		    "non-commutative iteration view is provided.");
	if (!has_ncomm)
	    cu_bugf("You must set CUEX_COMPOUNDFLAGS_PREFER_COMM since only "
		    "commutative iteration view is provided.");
    }

    if (!impl->comm_source) {
	if (impl->ncomm_source)
	    impl->comm_source = comm_source_adaptor;
	else if (cu_true)
	    cu_bugf("Either ncomm_source or comm_source must be defined.");
    }
    if (!impl->comm_image_junctor) {
	if (impl->comm_source && impl->comm_build_sinktor)
	    impl->comm_image_junctor = comm_image_from_build_adaptor;
	else if (impl->ncomm_source && impl->ncomm_image_junctor)
	    impl->comm_image_junctor = comm_image_junctor_adaptor;
	else if (cu_false)
	    cu_bugf("Cannot synthesise comm_image_junctor.");
    }
}


/* == Algorithms == */

cu_bool_t
cuex_compound_conj(cuex_intf_compound_t impl, cuex_t C,
		   cu_clop(f, cu_bool_t, cuex_t))
{
    cuex_t e;
    cu_ptr_source_t source;
    if ((impl->flags & CUEX_COMPOUNDFLAG_PREFER_COMM))
	source = (*impl->comm_source)(impl, C);
    else
	source = (*impl->ncomm_source)(impl, C);
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
	cu_ptr_source_t source = (*impl->comm_source)(impl, C);
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
