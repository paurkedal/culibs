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

typedef uint_fast16_t cuoo_shape_t;

/*!To be or-ed into shapes for types which use a custom hash function in place
 * of a fixed size. */
#define CUOO_SHAPEFLAG_HCV		UINT16_C(0x8000)

/*!Or-ed into shapes of types which may define finalisers. */
#define CUOO_SHAPEFLAG_FIN		UINT16_C(0x4000)

/*!Mask of all the above flags. */
#define CUOO_SHAPEFLAG_MASK		UINT16_C(0xc000)

/*!Main classification of types. */
#define CUOO_SHAPE_NONE			0x00
#define CUOO_SHAPE_OPAQUE		0x01
#define CUOO_SHAPE_OPAQUE_HCV		(CUOO_SHAPE_OPAQUE|CUOO_SHAPEFLAG_HCV)
#define CUOO_SHAPE_METATYPE		0x02
#define CUOO_SHAPE_TVAR			0x03
#define CUOO_SHAPE_PROTO		0x04
#define CUOO_SHAPE_CTORTYPE		0x05
#define CUOO_SHAPE_BY_EXPR		0x06

/* inline types */
#define CUOO_SHAPE_PTRTYPE		0x08
#define CUOO_SHAPE_ARRTYPE		0x09
#define CUOO_SHAPE_TUPTYPE		0x0a

/* inline types: elementary */
#define CUOO_SHAPE_SCALAR_MIN		0x10
#define CUOO_SHAPE_SCALAR_BOOL		0x10
#define CUOO_SHAPE_SCALAR_CHAR		0x11
#define CUOO_SHAPE_SCALAR_METAINT	0x12
#define CUOO_SHAPE_SCALAR_UINT8		0x16
#define CUOO_SHAPE_SCALAR_INT8		0x17
#define CUOO_SHAPE_SCALAR_UINT16	0x18
#define CUOO_SHAPE_SCALAR_INT16		0x19
#define CUOO_SHAPE_SCALAR_UINT32	0x1a
#define CUOO_SHAPE_SCALAR_INT32		0x1b
#define CUOO_SHAPE_SCALAR_UINT64	0x1c
#define CUOO_SHAPE_SCALAR_INT64		0x1d
#define CUOO_SHAPE_SCALAR_FLOAT		0x1e
#define CUOO_SHAPE_SCALAR_DOUBLE	0x1f
#define CUOO_SHAPE_SCALAR_MAX		0x1f

#define CUOO_SHAPE_CULIBS_END		0x100

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

    uint_least16_t shape;
    uint_least16_t key_sizew;

    cu_word_t (*impl)(cu_word_t intf_number, ...); /* FIXME set */
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
	&& cuoo_type_from_meta(m)->shape == CUOO_SHAPE_METATYPE;
}

/*!Cast \a e to a \ref cuoo_type_t, assuming \ref cuoo_is_type(\a e) is true. */
CU_SINLINE cuoo_type_t cuoo_type_from_ex(cuex_t e)
{ return (cuoo_type_t)e; }

/*!The expression form of \a t.  For elementary and opaque types, the type
 * itself.  For types of parametric kind, the instantiated form. */
CU_SINLINE cuex_t cuoo_type_as_expr(cuoo_type_t t)
{ return t->as_expr? t->as_expr : t; }

/*!Returns the \ref cuoo_shape_t "shape" of \a type. */
CU_SINLINE cuoo_shape_t cuoo_type_shape(cuoo_type_t type)
{ return type->shape; }

#define cuoo_type_impl(type, ...) ((type)->impl(__VA_ARGS__))
#define cuoo_type_impl_ptr(type, ...) ((void *)(type)->impl(__VA_ARGS__))

CU_SINLINE cu_bool_t cuoo_type_is_hctype(cuoo_type_t type)
{ return type->key_sizew; }

CU_SINLINE cu_bool_t cuoo_type_is_metatype(cuoo_type_t type)
{ return type->shape == CUOO_SHAPE_METATYPE; }

CU_SINLINE cu_bool_t cuoo_type_is_inltype(cuoo_type_t type)
{ return (type->shape & ~CUOO_SHAPEFLAG_MASK) >= CUOO_SHAPE_PTRTYPE; }

CU_SINLINE cu_bool_t cuoo_type_is_nonptr_inltype(cuoo_type_t type)
{ return (type->shape & ~CUOO_SHAPEFLAG_MASK) > CUOO_SHAPE_PTRTYPE; }

CU_SINLINE cu_bool_t cuoo_type_is_proto(cuoo_type_t type)
{ return type->shape == CUOO_SHAPE_PROTO; }

/*!Initialise \a type to be used for non-hash-consed objects. */
void cuoo_type_init_general(cuoo_type_t type, cuoo_shape_t shape,
			    cuoo_impl_t impl, cuex_t e);
/*!Initialise \a type to be used for hash-consed objects of size
 * \a key_size, excluding the CU_HCOBJ header. */
void cuoo_type_init_general_hcs(cuoo_type_t type, cuoo_shape_t shape,
				cuoo_impl_t impl, cuex_t e, size_t key_size);
/*!Initialise \a type to be used for hash-consed objects with hash function \a
 * key_hash_fn. */
void cuoo_type_init_general_hcv(cuoo_type_t type, cuoo_shape_t shape,
				cuoo_impl_t impl, cuex_t e);

/*!Construct \a type as an opaque type for non-hash-consed objects. */
void cuoo_type_init_opaque(cuoo_type_t type, cuoo_impl_t impl);

/*!Construct \a type as an opaque type for hash-consed objects of key size \a
 * key_size. */
void cuoo_type_init_opaque_hcs(cuoo_type_t type, cuoo_impl_t impl,
			       size_t key_size);

/*!Construct \a type as an opaque type for hash-consed object with hash
 * function \a key_hash_fn. */
void cuoo_type_init_opaque_hcv(cuoo_type_t type, cuoo_impl_t impl);

/*!Creates a type for non-hash-consed objects. */
cuoo_type_t cuoo_type_new_opaque(cuoo_impl_t impl);

/*!Creates a type for hash-consed objects of size \a key_size, excluding the \c
 * CU_HCOBJ header. */
cuoo_type_t cuoo_type_new_opaque_hcs(cuoo_impl_t impl, size_t key_size);

/*!Creates a type for hash-consed objects of with hash function
 * \a key_hash_fn. */
cuoo_type_t cuoo_type_new_opaque_hcv(cuoo_impl_t impl);


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
