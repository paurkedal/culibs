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

#include <cuex/compound.h>
#include <cu/diag.h>
#include <cu/memory.h>

void
cuex_intf_compound_verify(cuex_intf_compound_t impl)
{
    unsigned int flags = impl->flags;
    cu_bool_t has_comm = !!impl->comm_iterable;
    cu_bool_t has_ncomm = !!impl->ncomm_iterable;
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
}

cu_bool_t
cuex_compound_conj(cuex_intf_compound_t impl, cuex_t compound,
		   cu_clop(f, cu_bool_t, cuex_t))
{
    void *itr;
    cuex_t e;
    cuex_intf_iterable_t src_impl;
    if ((impl->flags & CUEX_COMPOUNDFLAG_PREFER_COMM) || !impl->ncomm_iterable)
	src_impl = impl->comm_iterable;
    else
	src_impl = impl->ncomm_iterable;
    itr = cu_salloc(src_impl->itr_size(compound));
    src_impl->itr_init(itr, compound);
    while ((e = src_impl->itr_get(itr)))
	if (!cu_call(f, e))
	    return cu_false;
    return cu_true;
}

cuex_t
cuex_compound_image(cuex_intf_compound_t impl, cuex_t compound,
		    cu_clop(f, cuex_t, cuex_t))
{
    cuex_t e;
    if ((impl->flags & CUEX_COMPOUNDFLAG_PREFER_COMM)
	    || !impl->ncomm_imageable) {
	cuex_intf_iterable_t src_impl = impl->comm_iterable;
	cuex_intf_growable_t dst_impl = impl->comm_growable;    
	void *src_itr = cu_salloc(src_impl->itr_size(compound));
	void *dst_itr = cu_salloc(dst_impl->itr_size);
	src_impl->itr_init(src_itr, compound);
	dst_impl->itr_init_empty(dst_itr, compound);
	while ((e = src_impl->itr_get(src_itr))) {
	    e = cu_call(f, e);
	    if (!e)
		return NULL;
	    dst_impl->itr_put(dst_itr, e);
	}
	return dst_impl->itr_finish(dst_itr);
    }
    else {
	cuex_intf_imageable_t img_impl = impl->ncomm_imageable;
	void *img_itr = cu_salloc(img_impl->itr_size(compound));
	img_impl->itr_init(img_itr, compound);
	while ((e = img_impl->itr_get(img_itr))) {
	    e = cu_call(f, e);
	    if (!e)
		return NULL;
	    img_impl->itr_put(img_itr, e);
	}
	return img_impl->itr_finish(img_itr);
    }
}
