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

#ifndef CUOO_TYPE_H
#define CUOO_TYPE_H

#include <cuoo/hcobj.h>
#include <cuoo/meta.h>
#include <cu/thread.h>
#include <cu/inherit.h>
#include <cu/conf.h>
#include <cu/memory.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuoo_type_h cuoo/type.h: Operations and Dynamically Typed Objects
 * @{ \ingroup cuoo_mod */


/* Types
 * ===== */

typedef enum {
    cuoo_typekind_none,
    cuoo_typekind_tvar,
    cuoo_typekind_stdtype,		/* Inherits cuoo_stdtype_s. */
    cuoo_typekind_stdtypeoftypes,	/* Inherits cuoo_stdtype_s. */
    cuoo_typekind_proto,
    cuoo_typekind_ctortype,
    cuoo_typekind_by_expr,

    /* inline types */
    cuoo_typekind_ptrtype,
    cuoo_typekind_arrtype,
    cuoo_typekind_tuptype,
    cuoo_typekind_sigtype,
    cuoo_typekind_duntype,
    cuoo_typekind_sngtype,

    /* inline types: elementary */
    cuoo_typekind_elmtype_MIN,
    cuoo_typekind_elmtype_bool = cuoo_typekind_elmtype_MIN,
    cuoo_typekind_elmtype_uint8,
    cuoo_typekind_elmtype_int8,
    cuoo_typekind_elmtype_uint16,
    cuoo_typekind_elmtype_int16,
    cuoo_typekind_elmtype_uint32,
    cuoo_typekind_elmtype_int32,
    cuoo_typekind_elmtype_uint64,
    cuoo_typekind_elmtype_int64,
    cuoo_typekind_elmtype_metaint,
    cuoo_typekind_elmtype_float,
    cuoo_typekind_elmtype_double,
    cuoo_typekind_elmtype_char
} cuoo_typekind_t;

typedef enum {
    cuoo_hcmethod_none = 0,	/* Not hashconsed. */
    cuoo_hcmethod_by_size,	/* Sets cuoo_type_s.u0.key_size */
    cuoo_hcmethod_by_size_fn,	/* Sets cuoo_type_s.u0.key_size_fn */
    cuoo_hcmethod_by_hash_fn	/* Sets cuoo_type_s.u0.key_hash_fn */
} cuoo_hcmethod_t;

struct cuoo_type_s
{
    CUOO_HCOBJ		/* used optionally */

    /* If non-null, as_expr is the expression-form of the type.  Since
     * it uniquely identifies the type, it is, when present, sufficient for
     * the HC key.
     * NB! This must be right after CUOO_HCOBJ, due to cuooP_type_cct_hce and
     * cuoo_type_glck. */
    cuex_t as_expr;

    cu_word_t (*impl)(cu_word_t intf_number, ...); /* FIXME set */

    cuoo_typekind_t typekind : 5;
    cuoo_hcmethod_t members_hcmethod : 2;

    union {
	size_t key_size;
	cu_clop(key_size_fn, size_t, void *obj);
	cu_clop(key_hash_fn, cu_hash_t, void *obj);
    } u0;
};
#define cuex_meta_is_type(meta) (cuex_meta_kind(meta) == cuex_meta_kind_type)

void cuoo_type_cct(cuoo_type_t type, cuex_t as_expr, cuoo_typekind_t kind);

CU_SINLINE cu_bool_t cuoo_is_type(cuex_t e)
{
    cuex_meta_t m = cuex_meta(e);
    return cuex_meta_is_type(m)
	&& cuoo_type_from_meta(m)->typekind == cuoo_typekind_stdtypeoftypes;
}

CU_SINLINE cuoo_type_t cuoo_type_from_ex(cuex_t e)
{ return (cuoo_type_t)e; }

/*!The expression form of \a t.  For elementary and opaque types, the type
 * itself.  For types of parametric kind, the instantiated form. */
CU_SINLINE cuex_t cuoo_type_as_expr(cuoo_type_t t)
{ return t->as_expr? t->as_expr : t; }

CU_SINLINE cuoo_typekind_t cuoo_type_typekind(cuoo_type_t type)
{ return type->typekind; }

#define cuoo_type_impl(type, args...) ((type)->impl(args))
#define cuoo_type_impl_ptr(type, args...) ((void *)(type)->impl(args))

CU_SINLINE cu_bool_t cuoo_type_is_hctype(cuoo_type_t type)
{ return type->members_hcmethod; }

CU_SINLINE cu_bool_t cuoo_type_is_stdtype(cuoo_type_t type)
{ return type->typekind <= cuoo_typekind_stdtypeoftypes; }

CU_SINLINE cu_bool_t cuoo_type_is_inltype(cuoo_type_t type)
{ return type->typekind >= cuoo_typekind_ptrtype; }

CU_SINLINE cu_bool_t cuoo_type_is_nonptr_inltype(cuoo_type_t type)
{ return type->typekind > cuoo_typekind_ptrtype; }

CU_SINLINE cu_bool_t cuoo_type_is_proto(cuoo_type_t type)
{ return type->typekind == cuoo_typekind_proto; }

CU_SINLINE cu_bool_t cuoo_type_is_typeoftypes(cuoo_type_t type)
{ return type->typekind == cuoo_typekind_stdtypeoftypes; }

void cuooP_type_cct_nonhc(cuoo_type_t type, cuoo_impl_t impl, cuex_t as_expr,
			  cuoo_typekind_t kind);
void cuooP_type_cct_hcs(cuoo_type_t type, cuoo_impl_t impl, cuex_t as_expr,
			cuoo_typekind_t kind, size_t key_size);
void cuooP_type_cct_hce(cuoo_type_t type, cuoo_impl_t impl, cuex_t as_expr,
			cuoo_typekind_t kind);
void cuooP_type_cct_hcv(cuoo_type_t type, cuoo_impl_t impl, cuex_t as_expr,
			cuoo_typekind_t kind,
			cu_clop(key_size_fn, size_t, void *));
void cuooP_type_cct_hcf(cuoo_type_t type, cuoo_impl_t impl, cuex_t as_expr,
			cuoo_typekind_t kind,
			cu_clop(key_hash_fn, cu_hash_t, void *));

struct cuoo_stdtype_s
{
    cu_inherit (cuoo_type_s);
    cu_clop(finalise, void, void *);
};

#define cuoo_stdtype_from_type(type) cu_from(cuoo_stdtype, cuoo_type, type)
#define cuoo_stdtype_to_type(stdtype) cu_to(cuoo_type, stdtype)
#define cuoo_stdtype_from_meta(meta) \
    cuoo_stdtype_from_type(cuoo_type_from_meta(meta))
#define cuoo_stdtype_to_meta(stdtype) \
    cuoo_type_to_meta(cuoo_stdtype_to_type(stdtype))

extern cuoo_stdtype_t cuooP_stdtype_type;
#define cuoo_stdtype_type() cuoo_stdtype_to_type(cuooP_stdtype_type)

/* Create a type with standard method slots. */
void cuoo_stdtype_cct(cuoo_stdtype_t, cuoo_typekind_t kind,
		      cuoo_impl_t impl);

cuoo_stdtype_t cuoo_stdtype_new(cuoo_impl_t impl);

/* Create a type for hash constructed objects. */
void cuoo_stdtype_cct_hcs(cuoo_stdtype_t, cuoo_typekind_t,
			  cuoo_impl_t impl, size_t key_size);

cuoo_stdtype_t cuoo_stdtype_new_hcs(cuoo_impl_t impl, size_t key_size);

/* Create a type for hash constructed objects with variable size. */
void cuoo_stdtype_cct_hcv(cuoo_stdtype_t, cuoo_typekind_t, cuoo_impl_t impl,
			  cu_clop(key_size, size_t, void *));

cuoo_stdtype_t cuoo_stdtype_new_hcv(cuoo_impl_t impl,
				    cu_clop(key_size, size_t, void *));


cuoo_stdtype_t cuoo_stdtypeoftypes_new(cuoo_impl_t impl);

/*!A new type of types.  If the instances are hash-consed, the expression
 * for of the type will be used as the key. */
cuoo_stdtype_t cuoo_stdtypeoftypes_new_hce(cuoo_impl_t impl);

cuoo_stdtype_t cuoo_stdtypeoftypes_new_hcs(cuoo_impl_t impl, size_t key_size);


/* Objects
 * ======= */

void *cuoo_impl_ptr(cuex_t obj, cu_word_t intf);


/* Hashconsed Objects
 * ================== */

size_t cuex_key_size(cuex_meta_t meta, void *obj);
cu_hash_t cuex_key_hash(void *);


/* Properties
 * ========== */

#ifdef CUOO_ENABLE_KEYED_PROP
cuoo_propkey_t cuoo_propkey_create(void);

void cuoo_prop_set(cuex_t ex, cuoo_propkey_t key, void *val);
void *cuoo_prop_get(cuex_t ex, cuoo_propkey_t key);
#endif

/*!@}*/
CU_END_DECLARATIONS

#endif
