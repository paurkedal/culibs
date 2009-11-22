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
#include <cu/ptr.h>
#include <cu/str.h>

char const *
cuoo_shape_name(cuoo_shape_t shape)
{
    switch (shape) {
	case CUOO_SHAPE_OPAQUE:		return "opaque";
	case CUOO_SHAPE_OPAQUE_HCV:	return "opaque_hcv";
	case CUOO_SHAPE_METATYPE:	return "metatype";
	case CUOO_SHAPE_UNIT:		return "unit";
	case CUOO_SHAPE_SCALAR_BOOL:	return "bool";
	case CUOO_SHAPE_SCALAR_CHAR:	return "char";
	case CUOO_SHAPE_SCALAR_METAINT:	return "cuex_meta_t";
	case CUOO_SHAPE_SCALAR_UINT8:	return "uint8_t";
	case CUOO_SHAPE_SCALAR_INT8:	return "int8_t";
	case CUOO_SHAPE_SCALAR_UINT16:	return "uint16_t";
	case CUOO_SHAPE_SCALAR_INT16:	return "int16_t";
	case CUOO_SHAPE_SCALAR_UINT32:	return "uint32_t";
	case CUOO_SHAPE_SCALAR_INT32:	return "int32_t";
	case CUOO_SHAPE_SCALAR_UINT64:	return "uint64_t";
	case CUOO_SHAPE_SCALAR_INT64:	return "int64_t";
	case CUOO_SHAPE_SCALAR_FLOAT:	return "float";
	case CUOO_SHAPE_SCALAR_DOUBLE:	return "double";
	default:			return NULL;
    }
}

size_t
cuoo_shape_valsize(cuoo_shape_t shape)
{
    switch (shape) {
	case CUOO_SHAPE_UNIT:
	    return 0;
	case CUOO_SHAPE_PTRTYPE:
	    return sizeof(void *);
	case CUOO_SHAPE_SCALAR_INT8:  case CUOO_SHAPE_SCALAR_UINT8:
	    return sizeof(uint8_t);
	case CUOO_SHAPE_SCALAR_INT16: case CUOO_SHAPE_SCALAR_UINT16:
	    return sizeof(uint16_t);
	case CUOO_SHAPE_SCALAR_INT32: case CUOO_SHAPE_SCALAR_UINT32:
	    return sizeof(uint32_t);
	case CUOO_SHAPE_SCALAR_INT64: case CUOO_SHAPE_SCALAR_UINT64:
	    return sizeof(uint64_t);
	default:
	    return (size_t)-1;
    }
}

void
cuoo_type_init_general(cuoo_type_t type, cuoo_shape_t shape,
		       cuoo_impl_t impl, cuex_t expr)
{
    cu_debug_assert(!(shape & CUOO_SHAPEFLAG_HCV));
    type->shape = shape;
    type->impl = impl;
    type->as_expr = expr;
    type->key_sizew = 0;
}

void
cuoo_type_init_general_hcs(cuoo_type_t type, cuoo_shape_t shape,
			   cuoo_impl_t impl, cuex_t expr, size_t key_size)
{
    cu_debug_assert(!(shape & CUOO_SHAPEFLAG_HCV));
    cu_debug_assert(key_size % sizeof(cu_word_t) == 0);
    type->shape = shape;
    type->impl = impl;
    type->as_expr = expr;
    type->key_sizew = key_size / sizeof(cu_word_t);
}

void
cuoo_type_init_general_hcv(cuoo_type_t type, cuoo_shape_t shape,
			   cuoo_impl_t impl, cuex_t expr)
{
    cu_debug_assert(shape & CUOO_SHAPEFLAG_HCV);
    type->shape = shape;
    type->impl = impl;
    type->as_expr = expr;
    type->key_sizew = (uint_least16_t)-1;
}


/* Opaque Types
 * ------------ */

void
cuoo_type_init_opaque(cuoo_type_t type, cuoo_impl_t impl)
{
    cuoo_type_init_general(type, CUOO_SHAPE_OPAQUE, impl, NULL);
}

void
cuoo_type_init_opaque_hcs(cuoo_type_t type, cuoo_impl_t impl, size_t key_size)
{
    cuoo_type_init_general_hcs(type, CUOO_SHAPE_OPAQUE, impl, NULL,
			       key_size);
}

void
cuoo_type_init_opaque_hcv(cuoo_type_t type, cuoo_impl_t impl)
{
    cuoo_type_init_general_hcv(type, CUOO_SHAPE_OPAQUE_HCV, impl, NULL);
}

cuoo_type_t
cuoo_type_new_opaque(cuoo_impl_t impl)
{
    cuoo_type_t type = cuoo_onew(cuoo_type);
    cuoo_type_init_general(type, CUOO_SHAPE_OPAQUE, impl, NULL);
    return type;
}

cuoo_type_t
cuoo_type_new_opaque_hcs(cuoo_impl_t impl, size_t key_size)
{
    cuoo_type_t type = cuoo_onew(cuoo_type);
    cuoo_type_init_opaque_hcs(type, impl, key_size);
    return type;
}

cuoo_type_t
cuoo_type_new_opaque_hcv(cuoo_impl_t impl)
{
    cuoo_type_t type = cuoo_onew(cuoo_type);
    cuoo_type_init_opaque_hcv(type, impl);
    return type;
}

cu_str_t
cuoo_type_to_str_default(cuex_t type)
{
    cuoo_shape_t shape = cuoo_type_shape(type);
    char const *name = cuoo_shape_name(shape);
    if (cuoo_shape_is_singleton(shape))
	return cu_str_new_cstr(name);
    else if (name)
	return cu_str_new_fmt("__%s_%p", name, type);
    else
	return cu_str_new_fmt("__type_%p", type);
}

cu_box_t cuooP_type_foprint;

static cu_box_t
_type_impl(cu_word_t intf_number, ...)
{
    switch (intf_number) {
	case CUOO_INTF_TO_STR_FN:
	    return CUOO_INTF_TO_STR_FN_BOX(cuoo_type_to_str_default);
	case CUOO_INTF_FOPRINT_FN:
	    return cuooP_type_foprint;
	default:
	    return CUOO_IMPL_NONE;
    }
}


/* Meta Types
 * ---------- */

cuoo_type_t
cuoo_type_new_metatype(cuoo_impl_t impl)
{
    cuoo_type_t t = cuoo_onew(cuoo_type);
    cuoo_type_init_general(t, CUOO_SHAPE_METATYPE, impl, NULL);
    return t;
}

static cuoo_type_t
cuoo_type_new_self_instance_hcb(cuoo_shape_t shape, cuoo_impl_t impl)
{
    cuoo_type_t type = cuoo_oalloc_self_instance(sizeof(struct cuoo_type));
    cuoo_type_init_general_hcs(type, shape, impl, NULL,
			       sizeof(struct cuoo_type) - CUOO_HCOBJ_SHIFT);
    return type;
}

cuoo_type_t
cuoo_type_new_metatype_hcs(cuoo_impl_t impl, size_t key_size)
{
    cuoo_type_t type = cuoo_onew(cuoo_type);
    cuoo_type_init_general_hcs(type, CUOO_SHAPE_METATYPE, impl, NULL, key_size);
    return type;
}

cuoo_type_t
cuoo_type_new_metatype_hce(cuoo_impl_t impl)
{
    cuoo_type_t type = cuoo_onew(cuoo_type);
    cuoo_type_init_general_hcs(type, CUOO_SHAPE_METATYPE,
			       impl, NULL, sizeof(cuex_t));
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

cu_hash_t
cuex_key_hash(void *obj)
{
    cuex_meta_t meta = cuex_meta(obj);
    size_t key_sizew;
    switch (cuex_meta_kind(meta)) {
	    cuoo_type_t t;
	case cuex_meta_kind_type:
	    t =  cuoo_type_from_meta(meta);
	    cu_debug_assert(cuoo_is_type(t) && cuoo_type_is_hctype(t));
	    if (t->shape & CUOO_SHAPEFLAG_HCV) {
		key_sizew = *(cu_word_t *)cu_ptr_add(obj, CUOO_HCOBJ_SHIFT);
		key_sizew = (key_sizew + CU_WORD_SIZE - 1)/CU_WORD_SIZE;
	    }
	    else
		key_sizew = t->key_sizew;
	    break;
	case cuex_meta_kind_opr:
	    key_sizew = cuex_opr_r(meta);
	    break;
	case cuex_meta_kind_other:
	    cu_debug_assert((meta & CUEXP_VARMETA_SELECT_MASK) ==
			    CUEXP_VARMETA_SELECT_VALUE);
	    key_sizew = 0;
	    break;
	default:
	    cu_bug_unreachable();
    }
    return cu_wordarr_hash(key_sizew, cu_ptr_add(obj, CUOO_HCOBJ_SHIFT), meta);
}



/* Properties
 * ========== */

#ifdef CUOO_ENABLE_KEYED_PROP
pthread_mutex_t cuooP_property_mutex = CU_MUTEX_INITIALISER;
struct cucon_umap cuooP_property_map;
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
    cuooP_hcobj_set_has_prop(ex);
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

cuoo_type_t cuooP_type_type;

void
cuooP_type_init()
{
#if CUOO_ENABLE_KEYED_PROP
    cucon_umap_init(&cuooP_property_map);
#endif
    cuooP_type_type = cuoo_type_new_self_instance_hcb(CUOO_SHAPE_METATYPE,
						      _type_impl);
}
