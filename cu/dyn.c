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

#include <cu/dyn.h>
#include <cu/halloc.h>
#include <cu/memory.h>
#include <cu/debug.h>
#include <cu/wordarr.h>
#include <cu/oalloc.h>

#ifndef NDEBUG
#  define CUEX_DEBUG_HCONS
#endif


/* Types
 * ===== */

void
cudyn_type_cct(cudyn_type_t t, cuex_t expr, cudyn_typekind_t kind)
{
    t->typekind = kind;
    t->members_hcmethod = cudyn_hcmethod_none;
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
cudynP_hctype_cct_nonhc(cudyn_hctype_t type, cuex_t expr, cudyn_typekind_t kind)
{
    cudyn_hctype_to_type(type)->typekind = kind;
    cudyn_hctype_to_type(type)->members_hcmethod = cudyn_hcmethod_none;
    cudyn_hctype_to_type(type)->as_expr = expr;
    type->u0.key_size = 0;
}

void
cudynP_hctype_cct_hcs(cudyn_hctype_t type, cuex_t expr, cudyn_typekind_t kind,
		      size_t key_size)
{
    cudyn_hctype_to_type(type)->typekind = kind;
    cudyn_hctype_to_type(type)->members_hcmethod = cudyn_hcmethod_by_size;
    cudyn_hctype_to_type(type)->as_expr = expr;
    type->u0.key_size = key_size;
}

void
cudynP_hctype_cct_hcv(cudyn_hctype_t t, cuex_t expr, cudyn_typekind_t kind,
		      cu_clop(key_size_fn, size_t, void *))
{
    cudyn_hctype_to_type(t)->typekind = kind;
    cudyn_hctype_to_type(t)->members_hcmethod = cudyn_hcmethod_by_size_fn;
    cudyn_hctype_to_type(t)->as_expr = expr;
    t->u0.key_size_fn = key_size_fn;
}

void
cudynP_hctype_cct_hcf(cudyn_hctype_t t, cuex_t expr, cudyn_typekind_t kind,
		      cu_clop(key_hash_fn, cu_hash_t, void *))
{
    cudyn_hctype_to_type(t)->typekind = kind;
    cudyn_hctype_to_type(t)->members_hcmethod = cudyn_hcmethod_by_hash_fn;
    cudyn_hctype_to_type(t)->as_expr = expr;
    t->u0.key_hash_fn = key_hash_fn;
}


/* Standard Type
 * ------------- */

void
cudyn_stdtype_cct(cudyn_stdtype_t type, cudyn_typekind_t kind)
{
    cudynP_hctype_cct_nonhc(cudyn_stdtype_to_hctype(type), NULL, kind);
    type->finalise = cu_clop_null;
    type->conj = stdobj_conj_default_clop;
    type->tran = stdobj_tran_default_clop;
}

cudyn_stdtype_t
cudyn_stdtype_new()
{
    cudyn_stdtype_t type = cudyn_onew(cudyn_stdtype);
    cudyn_stdtype_cct(type, cudyn_typekind_stdtype);
    return type;
}

cudyn_stdtype_t
cudyn_stdtypeoftypes_new()
{
    cudyn_stdtype_t t = cudyn_onew(cudyn_stdtype);
    cudyn_stdtype_cct(t, cudyn_typekind_stdtypeoftypes);
    return t;
}

cudyn_stdtype_t
cudyn_stdtype_new_self_instance(cudyn_typekind_t kind)
{
    cudyn_stdtype_t type
	= cudyn_oalloc_self_instance(sizeof(struct cudyn_stdtype_s));
    cudyn_stdtype_cct(type, kind);
    return type;
}

void
cudyn_stdtype_cct_hcs(cudyn_stdtype_t type, cudyn_typekind_t kind,
		      size_t key_size)
{
    cudynP_hctype_cct_hcs(cudyn_stdtype_to_hctype(type), NULL, kind, key_size);
    type->finalise = cu_clop_null;
    type->conj = stdobj_conj_default_clop;
    type->tran = stdobj_tran_default_clop;
}

void
cudyn_stdtype_cct_hcv(cudyn_stdtype_t type, cudyn_typekind_t kind,
		      cu_clop(key_size_fn, size_t, void *))
{
    cudynP_hctype_cct_hcv(cudyn_stdtype_to_hctype(type), NULL, kind,
			  key_size_fn);
    type->finalise = cu_clop_null;
    type->conj = stdobj_conj_default_clop;
    type->tran = stdobj_tran_default_clop;
}

cudyn_stdtype_t
cudyn_stdtype_new_hcs(size_t key_size)
{
    cudyn_stdtype_t type = cudyn_onew(cudyn_stdtype);
    cudyn_stdtype_cct_hcs(type, cudyn_typekind_stdtype, key_size);
    return type;
}

cudyn_stdtype_t
cudyn_stdtypeoftypes_new_hcs(size_t key_size)
{
    cudyn_stdtype_t type = cudyn_onew(cudyn_stdtype);
    cudyn_stdtype_cct_hcs(type, cudyn_typekind_stdtypeoftypes, key_size);
    return type;
}

cudyn_stdtype_t
cudyn_stdtypeoftypes_new_hce(void)
{
    cudyn_stdtype_t type = cudyn_onew(cudyn_stdtype);
    cudyn_stdtype_cct_hcs(type, cudyn_typekind_stdtypeoftypes, sizeof(cuex_t));
    return type;
}

cudyn_stdtype_t
cudyn_stdtype_new_hcv(cu_clop(key_size_fn, size_t, void *))
{
    cudyn_stdtype_t type = cudyn_onew(cudyn_stdtype);
    cudyn_stdtype_cct_hcv(type, cudyn_typekind_stdtype, key_size_fn);
    return type;
}


/* Hashconsed Objects
 * ================== */

size_t
cuex_key_size(cuex_meta_t meta, void *obj)
{
    cu_hash_t key_size;
    switch (cuex_meta_kind(meta)) {
	cudyn_type_t t;
    case cuex_meta_kind_type:
	t = cudyn_type_from_meta(meta);
	if (t->members_hcmethod == cudyn_hcmethod_by_size)
	    key_size = cudyn_hctype_from_type(t)->u0.key_size;
	else {
	    cu_debug_assert(t->members_hcmethod == cudyn_hcmethod_by_size_fn);
	    key_size = cu_call(cudyn_hctype_from_type(t)->u0.key_size_fn,
			       obj);
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
	    cudyn_type_t t;
	case cuex_meta_kind_type:
	    t =  cudyn_type_from_meta(meta);
	    cu_debug_assert(cudyn_is_type(t) && cudyn_type_is_hctype(t));
	    switch (t->members_hcmethod) {
		case cudyn_hcmethod_by_size:
		    key_size = cudyn_hctype_from_type(t)->u0.key_size;
		    break;
		case cudyn_hcmethod_by_size_fn:
		    key_size =
			cu_call(cudyn_hctype_from_type(t)->u0.key_size_fn,
				obj);
		    break;
		case cudyn_hcmethod_by_hash_fn:
		    return cu_call(cudyn_hctype_from_type(t)->u0.key_hash_fn,
				   obj);
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
    return cu_hc_key_hash(key_size/sizeof(cu_word_t), obj + CU_HCOBJ_SHIFT, meta);
}



/* Properties
 * ========== */

#ifdef CUDYN_ENABLE_KEYED_PROP
pthread_mutex_t cudynP_property_mutex = CU_MUTEX_INITIALISER;
struct cucon_umap_s cudynP_property_map;
AO_t cudynP_next_propkey = 1;

cudyn_propkey_t
cudyn_propkey_create(void)
{
    return AO_fetch_and_add1(&cudynP_next_propkey);
}

void
cudyn_prop_set(cuex_t ex, cudyn_propkey_t key, void *val)
{
    uintptr_t *props;
    size_t req_size = sizeof(uintptr_t)*cudynP_next_propkey;
    cu_hcobj_set_has_prop(ex);
    cu_mutex_lock(&cudynP_property_mutex);
    if (cucon_umap_insert_mem(&cudynP_property_map, ~(uintptr_t)ex,
			      req_size, &props)) {
	memset(props, 0, req_size);
	props[0] = req_size;
    }
    else if (props[0] <= sizeof(uintptr_t)*key) {
	uintptr_t *old_props = props;
	cucon_umap_erase(&cudynP_property_map, ~(uintptr_t)ex);
	cucon_umap_insert_mem(&cudynP_property_map, ~(uintptr_t)ex,
			      req_size, &props);
	memcpy(props, old_props, req_size);
	props[0] = req_size;
    }
    props[key] = (uintptr_t)val;
    cu_mutex_unlock(&cudynP_property_mutex);
}

void *
cudyn_prop_get(cuex_t ex, cudyn_propkey_t key)
{
    void *r;
    uintptr_t *props;
    cu_mutex_lock(&cudynP_property_mutex);
    props = cucon_umap_find_mem(&cudynP_property_map, ~(uintptr_t)ex);
    if (!props || props[0] <= sizeof(uintptr_t)*key)
	r = NULL;
    else
	r = (void *)props[key];
    cu_mutex_unlock(&cudynP_property_mutex);
    return r;
}
#endif


/* Init
 * ==== */

cudyn_stdtype_t cudynP_stdtype_type;

void
cuP_dyn_init()
{
    static stdobj_conj_default_t stdobj_conj_default;
    static stdobj_tran_default_t stdobj_tran_default;
    stdobj_conj_default_clop = stdobj_conj_default_prep(&stdobj_conj_default);
    stdobj_tran_default_clop = stdobj_tran_default_prep(&stdobj_tran_default);
#if CUDYN_ENABLE_KEYED_PROP
    cucon_umap_cct(&cudynP_property_map);
#endif
    cudynP_stdtype_type =
	cudyn_stdtype_new_self_instance(cudyn_typekind_stdtypeoftypes);
}
