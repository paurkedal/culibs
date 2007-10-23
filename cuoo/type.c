/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2005--2007  Petter Urkedal <urkedal@nbi.dk>
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

#include <cuoo/type.h>
#include <cuoo/halloc.h>
#include <cuoo/oalloc.h>
#include <cuoo/intf.h>
#include <cu/memory.h>
#include <cu/debug.h>
#include <cu/wordarr.h>

#ifndef NDEBUG
#  define CUEX_DEBUG_HCONS
#endif


/* Types
 * ===== */

void
cuoo_type_cct(cuoo_type_t t, cuex_t expr, cuoo_typekind_t kind)
{
    t->typekind = kind;
    t->members_hcmethod = cuoo_hcmethod_none;
    t->as_expr = expr;
}


/* HC Type
 * ------- */

cu_clos_def(stdobj_conj_default,
	    cu_prot(cu_bool_t, void *obj, cu_clop(fn, cu_bool_t, void *)), ())
{
    return cu_true;
}
static cu_clop(stdobj_conj_default_clop,
	       cu_bool_t, void *, cu_clop(, cu_bool_t, void *));
cu_clos_def(stdobj_tran_default,
	    cu_prot(void *, void *obj, cu_clop(fn, void *, void *)), ())
{
    return obj;
}
static cu_clop(stdobj_tran_default_clop,
	       void *, void *, cu_clop(, void *, void *));

void
cuooP_type_cct_nonhc(cuoo_type_t type, cuoo_impl_t impl, cuex_t expr,
		     cuoo_typekind_t kind)
{
    type->typekind = kind;
    type->impl = impl;
    type->members_hcmethod = cuoo_hcmethod_none;
    type->as_expr = expr;
    type->u0.key_size = 0;
}

void
cuooP_type_cct_hcs(cuoo_type_t type, cuoo_impl_t impl, cuex_t expr,
		   cuoo_typekind_t kind, size_t key_size)
{
    type->typekind = kind;
    type->impl = impl;
    type->members_hcmethod = cuoo_hcmethod_by_size;
    type->as_expr = expr;
    type->u0.key_size = key_size;
}

void
cuooP_type_cct_hcv(cuoo_type_t type, cuoo_impl_t impl, cuex_t expr,
		   cuoo_typekind_t kind, cu_clop(key_size_fn, size_t, void *))
{
    type->typekind = kind;
    type->impl = impl;
    type->members_hcmethod = cuoo_hcmethod_by_size_fn;
    type->as_expr = expr;
    type->u0.key_size_fn = key_size_fn;
}

void
cuooP_type_cct_hcf(cuoo_type_t type, cuoo_impl_t impl, cuex_t expr,
		   cuoo_typekind_t kind,
		   cu_clop(key_hash_fn, cu_hash_t, void *))
{
    type->typekind = kind;
    type->impl = impl;
    type->members_hcmethod = cuoo_hcmethod_by_hash_fn;
    type->as_expr = expr;
    type->u0.key_hash_fn = key_hash_fn;
}


/* Standard Type
 * ------------- */

void
cuoo_stdtype_cct(cuoo_stdtype_t type, cuoo_typekind_t kind, cuoo_impl_t impl)
{
    cuooP_type_cct_nonhc(cuoo_stdtype_to_type(type), impl, NULL, kind);
    type->finalise = cu_clop_null;
    type->conj = stdobj_conj_default_clop;
    type->tran = stdobj_tran_default_clop;
}

cuoo_stdtype_t
cuoo_stdtype_new(cuoo_impl_t impl)
{
    cuoo_stdtype_t type = cuoo_onew(cuoo_stdtype);
    cuoo_stdtype_cct(type, cuoo_typekind_stdtype, impl);
    return type;
}

cuoo_stdtype_t
cuoo_stdtypeoftypes_new(cuoo_impl_t impl)
{
    cuoo_stdtype_t t = cuoo_onew(cuoo_stdtype);
    cuoo_stdtype_cct(t, cuoo_typekind_stdtypeoftypes, impl);
    return t;
}

cuoo_stdtype_t
cuoo_stdtype_new_self_instance(cuoo_typekind_t kind, cuoo_impl_t impl)
{
    cuoo_stdtype_t type
	= cuoo_oalloc_self_instance(sizeof(struct cuoo_stdtype_s));
    cuoo_stdtype_cct(type, kind, impl);
    return type;
}

void
cuoo_stdtype_cct_hcs(cuoo_stdtype_t type, cuoo_typekind_t kind,
		     cuoo_impl_t impl, size_t key_size)
{
    cuooP_type_cct_hcs(cuoo_stdtype_to_type(type), impl, NULL, kind, key_size);
    type->finalise = cu_clop_null;
    type->conj = stdobj_conj_default_clop;
    type->tran = stdobj_tran_default_clop;
}

void
cuoo_stdtype_cct_hcv(cuoo_stdtype_t type, cuoo_typekind_t kind,
		     cuoo_impl_t impl, cu_clop(key_size_fn, size_t, void *))
{
    cuooP_type_cct_hcv(cuoo_stdtype_to_type(type), impl, NULL, kind, key_size_fn);
    type->finalise = cu_clop_null;
    type->conj = stdobj_conj_default_clop;
    type->tran = stdobj_tran_default_clop;
}

cuoo_stdtype_t
cuoo_stdtype_new_hcs(cuoo_impl_t impl, size_t key_size)
{
    cuoo_stdtype_t type = cuoo_onew(cuoo_stdtype);
    cuoo_stdtype_cct_hcs(type, cuoo_typekind_stdtype, impl, key_size);
    return type;
}

cuoo_stdtype_t
cuoo_stdtypeoftypes_new_hcs(cuoo_impl_t impl, size_t key_size)
{
    cuoo_stdtype_t type = cuoo_onew(cuoo_stdtype);
    cuoo_stdtype_cct_hcs(type, cuoo_typekind_stdtypeoftypes, impl, key_size);
    return type;
}

cuoo_stdtype_t
cuoo_stdtypeoftypes_new_hce(cuoo_impl_t impl)
{
    cuoo_stdtype_t type = cuoo_onew(cuoo_stdtype);
    cuoo_stdtype_cct_hcs(type, cuoo_typekind_stdtypeoftypes,
			 impl, sizeof(cuex_t));
    return type;
}

cuoo_stdtype_t
cuoo_stdtype_new_hcv(cuoo_impl_t impl, cu_clop(key_size_fn, size_t, void *))
{
    cuoo_stdtype_t type = cuoo_onew(cuoo_stdtype);
    cuoo_stdtype_cct_hcv(type, cuoo_typekind_stdtype, impl, key_size_fn);
    return type;
}


/* Objects
 * ======= */

void *
cuoo_impl_ptr(cuex_t obj, cu_word_t intf)
{
    cuex_meta_t obj_meta = cuex_meta(obj);
    if (cuex_meta_is_type(obj_meta))
	return cuoo_type_impl_ptr(cuoo_type_from_meta(obj_meta), intf);
    return NULL;
}


/* Hashconsed Objects
 * ================== */

size_t
cuex_key_size(cuex_meta_t meta, void *obj)
{
    cu_hash_t key_size;
    switch (cuex_meta_kind(meta)) {
	cuoo_type_t t;
    case cuex_meta_kind_type:
	t = cuoo_type_from_meta(meta);
	if (t->members_hcmethod == cuoo_hcmethod_by_size)
	    key_size = t->u0.key_size;
	else {
	    cu_debug_assert(t->members_hcmethod == cuoo_hcmethod_by_size_fn);
	    key_size = cu_call(t->u0.key_size_fn, obj);
	}
	cu_debug_assert(key_size % sizeof(cu_word_t) == 0);
	break;
    case cuex_meta_kind_opr:
	key_size = cuex_opr_r(meta)*sizeof(void *);
	break;
    case cuex_meta_kind_other:
	cu_debug_assert((meta & CUEXP_VARMETA_SELECT_MASK) ==
			CUEXP_VARMETA_SELECT_VALUE);
	key_size = ((meta & CUEXP_VARMETA_WSIZE_MASK)
		    >> CUEXP_VARMETA_WSIZE_SHIFT)*sizeof(cu_word_t);
	break;
    default:
	cu_debug_unreachable();
    }
    return key_size;
}

cu_hash_t
cuex_key_hash(void *obj)
{
    cuex_meta_t meta = cuex_meta(obj);
    size_t key_size;
    switch (cuex_meta_kind(meta)) {
	    cuoo_type_t t;
	case cuex_meta_kind_type:
	    t =  cuoo_type_from_meta(meta);
	    cu_debug_assert(cuoo_is_type(t) && cuoo_type_is_hctype(t));
	    switch (t->members_hcmethod) {
		case cuoo_hcmethod_by_size:
		    key_size = t->u0.key_size;
		    break;
		case cuoo_hcmethod_by_size_fn:
		    key_size = cu_call(t->u0.key_size_fn, obj);
		    break;
		case cuoo_hcmethod_by_hash_fn:
		    return cu_call(t->u0.key_hash_fn, obj);
		default:
		    cu_debug_unreachable();
	    }
	    break;
	case cuex_meta_kind_opr:
	    key_size = cuex_opr_r(meta)*sizeof(void *);
	    break;
	case cuex_meta_kind_other:
	    cu_debug_assert((meta & CUEXP_VARMETA_SELECT_MASK) ==
			    CUEXP_VARMETA_SELECT_VALUE);
	    key_size = 0;
	    break;
	default:
	    cu_debug_unreachable();
    }
    return cu_wordarr_hash(key_size/sizeof(cu_word_t),
			   obj + CUOO_HCOBJ_SHIFT, meta);
}



/* Properties
 * ========== */

#ifdef CUOO_ENABLE_KEYED_PROP
pthread_mutex_t cuooP_property_mutex = CU_MUTEX_INITIALISER;
struct cucon_umap_s cuooP_property_map;
AO_t cuooP_next_propkey = 1;

cuoo_propkey_t
cuoo_propkey_create(void)
{
    return AO_fetch_and_add1(&cuooP_next_propkey);
}

void
cuoo_prop_set(cuex_t ex, cuoo_propkey_t key, void *val)
{
    uintptr_t *props;
    size_t req_size = sizeof(uintptr_t)*cuooP_next_propkey;
    cu_hcobj_set_has_prop(ex);
    cu_mutex_lock(&cuooP_property_mutex);
    if (cucon_umap_insert_mem(&cuooP_property_map, ~(uintptr_t)ex,
			      req_size, &props)) {
	memset(props, 0, req_size);
	props[0] = req_size;
    }
    else if (props[0] <= sizeof(uintptr_t)*key) {
	uintptr_t *old_props = props;
	cucon_umap_erase(&cuooP_property_map, ~(uintptr_t)ex);
	cucon_umap_insert_mem(&cuooP_property_map, ~(uintptr_t)ex,
			      req_size, &props);
	memcpy(props, old_props, req_size);
	props[0] = req_size;
    }
    props[key] = (uintptr_t)val;
    cu_mutex_unlock(&cuooP_property_mutex);
}

void *
cuoo_prop_get(cuex_t ex, cuoo_propkey_t key)
{
    void *r;
    uintptr_t *props;
    cu_mutex_lock(&cuooP_property_mutex);
    props = cucon_umap_find_mem(&cuooP_property_map, ~(uintptr_t)ex);
    if (!props || props[0] <= sizeof(uintptr_t)*key)
	r = NULL;
    else
	r = (void *)props[key];
    cu_mutex_unlock(&cuooP_property_mutex);
    return r;
}
#endif


/* Init
 * ==== */

cuoo_stdtype_t cuooP_stdtype_type;

void
cuP_dyn_init()
{
    static stdobj_conj_default_t stdobj_conj_default;
    static stdobj_tran_default_t stdobj_tran_default;
    stdobj_conj_default_clop = stdobj_conj_default_prep(&stdobj_conj_default);
    stdobj_tran_default_clop = stdobj_tran_default_prep(&stdobj_tran_default);
#if CUOO_ENABLE_KEYED_PROP
    cucon_umap_cct(&cuooP_property_map);
#endif
    cuooP_stdtype_type = cuoo_stdtype_new_self_instance(
	cuoo_typekind_stdtypeoftypes, cuoo_impl_none);
}
