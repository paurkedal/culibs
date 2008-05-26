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

/*!Main classification of types. */
typedef enum {
    cuoo_typekind_none,
    cuoo_typekind_tvar,
    cuoo_typekind_opaque,
    cuoo_typekind_metatype,
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

/*!The dynamic type (base) struct. */
struct cuoo_type_s
{
    CUOO_HCOBJ		/* used optionally */

    /* If non-null, as_expr is the expression-form of the type.  Since
     * it uniquely identifies the type, it is, when present, sufficient for
     * the HC key.
     * NB! This must be right after CUOO_HCOBJ, due to cuoo_type_init_general_hce and
     * cuoo_type_glck. */
    cuex_t as_expr;

    cu_word_t (*impl)(cu_word_t intf_number, ...); /* FIXME set */

    cuoo_typekind_t typekind : 5;
    cuoo_hcmethod_t members_hcmethod : 2;
#ifdef CUOO_ENABLE_FINALISERS
    cu_bool_t has_finaliser : 1;
#endif

    union {
	size_t key_size;
	cu_clop(key_size_fn, size_t, void *obj);
	cu_clop(key_hash_fn, cu_hash_t, void *obj);
    } u0;
};

extern cuoo_type_t cuooP_type_type;

/*!The type of types. */
CU_SINLINE cuoo_type_t cuoo_type_type()
{ return cuooP_type_type; }

/*!If true then \a meta is a type and can be converted with \ref
 * cuoo_type_from_meta. */
CU_SINLINE cu_bool_t cuex_meta_is_type(cuex_meta_t meta)
{ return cuex_meta_kind(meta) == cuex_meta_kind_type; }

/*!True iff \a e is a type. */
CU_SINLINE cu_bool_t cuoo_is_type(cuex_t e)
{
    cuex_meta_t m = cuex_meta(e);
    return cuex_meta_is_type(m)
	&& cuoo_type_from_meta(m)->typekind == cuoo_typekind_metatype;
}

/*!Cast \a e to a \ref cuoo_type_t, assuming \ref cuoo_is_type(\a e) is true. */
CU_SINLINE cuoo_type_t cuoo_type_from_ex(cuex_t e)
{ return (cuoo_type_t)e; }

/*!The expression form of \a t.  For elementary and opaque types, the type
 * itself.  For types of parametric kind, the instantiated form. */
CU_SINLINE cuex_t cuoo_type_as_expr(cuoo_type_t t)
{ return t->as_expr? t->as_expr : t; }

/*!Returns the \ref cuoo_typekind_t "type-kind" of \a type. */
CU_SINLINE cuoo_typekind_t cuoo_type_typekind(cuoo_type_t type)
{ return type->typekind; }

#define cuoo_type_impl(type, ...) ((type)->impl(__VA_ARGS__))
#define cuoo_type_impl_ptr(type, ...) ((void *)(type)->impl(__VA_ARGS__))

CU_SINLINE cu_bool_t cuoo_type_is_hctype(cuoo_type_t type)
{ return type->members_hcmethod; }

CU_SINLINE cu_bool_t cuoo_type_is_metatype(cuoo_type_t type)
{ return type->typekind <= cuoo_typekind_metatype; }

CU_SINLINE cu_bool_t cuoo_type_is_inltype(cuoo_type_t type)
{ return type->typekind >= cuoo_typekind_ptrtype; }

CU_SINLINE cu_bool_t cuoo_type_is_nonptr_inltype(cuoo_type_t type)
{ return type->typekind > cuoo_typekind_ptrtype; }

CU_SINLINE cu_bool_t cuoo_type_is_proto(cuoo_type_t type)
{ return type->typekind == cuoo_typekind_proto; }

CU_SINLINE cu_bool_t cuoo_type_is_typeoftypes(cuoo_type_t type)
{ return type->typekind == cuoo_typekind_metatype; }

/*!Initialise \a type to be used for non-hash-consed objects. */
void cuoo_type_init_general(cuoo_type_t type, cuoo_typekind_t kind,
			    cuoo_impl_t impl, cuex_t e);
/*!Initialise \a type to be used for hash-consed objects of size
 * \a key_size, excluding the CU_HCOBJ header. */
void cuoo_type_init_general_hcs(cuoo_type_t type, cuoo_typekind_t kind,
				cuoo_impl_t impl, cuex_t e, size_t key_size);
/*!Initialise \a type to be used for hash-consed objects with hash function \a
 * key_hash_fn. */
void cuoo_type_init_general_hcf(cuoo_type_t type, cuoo_typekind_t kind,
				cuoo_impl_t impl, cuex_t e,
				cu_clop(key_hash_fn, cu_hash_t, void *));

/*!Construct \a type as an opaque type for non-hash-consed objects. */
void cuoo_type_init_opaque(cuoo_type_t type, cuoo_impl_t impl);

/*!Construct \a type as an opaque type for hash-consed objects of key size \a
 * key_size. */
void cuoo_type_init_opaque_hcs(cuoo_type_t type, cuoo_impl_t impl,
			       size_t key_size);

/*!Construct \a type as an opaque type for hash-consed object with hash
 * function \a key_hash_fn. */
void cuoo_type_init_opaque_hcf(cuoo_type_t type, cuoo_impl_t impl,
			       cu_clop(key_hash_fn, cu_hash_t, void *));

/*!Creates a type for non-hash-consed objects. */
cuoo_type_t cuoo_type_new_opaque(cuoo_impl_t impl);

/*!Creates a type for hash-consed objects of size \a key_size, excluding the \c
 * CU_HCOBJ header. */
cuoo_type_t cuoo_type_new_opaque_hcs(cuoo_impl_t impl, size_t key_size);

/*!Creates a type for hash-consed objects of with hash function
 * \a key_hash_fn. */
cuoo_type_t cuoo_type_new_opaque_hcf(cuoo_impl_t impl,
				     cu_clop(key_hash_fn, cu_hash_t, void *));


cuoo_type_t cuoo_type_new_metatype(cuoo_impl_t impl);

/*!A new type of types.  If the instances are hash-consed, the expression
 * for of the type will be used as the key. */
cuoo_type_t cuoo_type_new_metatype_hce(cuoo_impl_t impl);

cuoo_type_t cuoo_type_new_metatype_hcs(cuoo_impl_t impl, size_t key_size);


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

/*!\deprecated*/
#define cuoo_stdtype_from_type(type) (type)
/*!\deprecated*/
#define cuoo_stdtype_to_type(type) (type)

CU_END_DECLARATIONS

#endif
