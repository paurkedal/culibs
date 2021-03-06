/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2005--2010  Petter Urkedal <paurkedal@eideticdew.org>
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
#include <cu/box.h>
#include <stdint.h>

CU_BEGIN_DECLARATIONS
/** \defgroup cuoo_type_h cuoo/type.h: Operations and Dynamically Typed Objects
 ** \ingroup cuoo_mod
 ** @{ */

/** \name Type Shapes
 **
 ** The shape of a type is the major classification to determine the
 ** representation of objects of that type.
 **  @{ */

/** To be or-ed into shapes for types which use a custom hash function in place
 ** of a fixed size. */
#define CUOO_SHAPEFLAG_HCV		UINT16_C(0x8000)

/** Or-ed into shapes of types which may define finalisers. */
#define CUOO_SHAPEFLAG_FIN		UINT16_C(0x4000)

/** Mask of all the above flags. */
#define CUOO_SHAPEFLAG_MASK		UINT16_C(0xc000)

/* Main classification of types. */
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
#define CUOO_SHAPE_UNIT			0x0f

/* inline types: elementary */
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

#define CUOO_SHAPE_MIN_SCALAR_INT	CUOO_SHAPE_SCALAR_UINT8
#define CUOO_SHAPE_MAX_SCALAR_INT	CUOO_SHAPE_SCALAR_INT64
#define CUOO_SHAPE_MIN_SCALAR_FP	CUOO_SHAPE_SCALAR_FLOAT
#define CUOO_SHAPE_MAX_SCALAR_FP	CUOO_SHAPE_SCALAR_DOUBLE
#define CUOO_SHAPE_MIN_SCALAR		CUOO_SHAPE_SCALAR_BOOL
#define CUOO_SHAPE_MAX_SCALAR		CUOO_SHAPE_MAX_SCALAR_FP

#define CUOO_SHAPE_CULIBS_END		0x100

/** A textual name for \a shape, or \c NULL if unimplemented. Mainly useful for
 ** the scalar types where the returned string corresponds with the C type
 ** name, other cases may be unimplemented. */
char const *cuoo_shape_name(cuoo_shape_t shape);

/** The size of values of types with shape \a shape, or \c (size_t)-1 if
 ** unknown.  This is defined for scalar shapes. */
size_t cuoo_shape_valsize(cuoo_shape_t shape);

/** True if \a shape indicates an opaque struct. */
CU_SINLINE cu_bool_t
cuoo_shape_is_opaque(cuoo_shape_t shape)
{
    shape &= ~CUOO_SHAPEFLAG_MASK;
    return shape == CUOO_SHAPE_OPAQUE;
}

/** True if \a shape indicates a scalar type. */
CU_SINLINE cu_bool_t
cuoo_shape_is_scalar(cuoo_shape_t shape)
{
    shape &= ~CUOO_SHAPEFLAG_MASK;
    return shape >= CUOO_SHAPE_MIN_SCALAR
	&& shape <= CUOO_SHAPE_MAX_SCALAR;
}

/** True if \a shape indicates an integer type, excluding bool and char. */
CU_SINLINE cu_bool_t
cuoo_shape_is_scalar_int(cuoo_shape_t shape)
{
    shape &= ~CUOO_SHAPEFLAG_MASK;
    return shape >= CUOO_SHAPE_MIN_SCALAR_INT
	&& shape <= CUOO_SHAPE_MAX_SCALAR_INT;
}

/** True if \a shape indicates a floating point type. */
CU_SINLINE cu_bool_t
cuoo_shape_is_scalar_fp(cuoo_shape_t shape)
{
    shape &= ~CUOO_SHAPEFLAG_MASK;
    return shape >= CUOO_SHAPE_MIN_SCALAR_FP
	&& shape <= CUOO_SHAPE_MAX_SCALAR_FP;
}

/** True iff there is exactly one type for this shape. */
CU_SINLINE cu_bool_t
cuoo_shape_is_singleton(cuoo_shape_t shape)
{
    shape &= ~CUOO_SHAPEFLAG_MASK;
    return shape == CUOO_SHAPE_UNIT
	|| (shape >= CUOO_SHAPE_MIN_SCALAR && shape <= CUOO_SHAPE_MAX_SCALAR);
}

/**  @}
 ** \name Types
 **  @{ */

/** The dynamic type (base) struct. */
struct cuoo_type
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

    cu_box_t (*impl)(cu_word_t intf_number, ...); /* FIXME set */
};

extern cuoo_type_t cuooP_type_type;

/** The type of types. */
CU_SINLINE cuoo_type_t cuoo_type_type()
{ return cuooP_type_type; }

/** If true then \a meta is a type and can be converted with \ref
 * cuoo_type_from_meta. */
CU_SINLINE cu_bool_t cuex_meta_is_type(cuex_meta_t meta)
{ return cuex_meta_kind(meta) == cuex_meta_kind_type; }

/** True iff \a e is a type. */
CU_SINLINE cu_bool_t cuoo_is_type(cuex_t e)
{
    cuex_meta_t m = cuex_meta(e);
    return cuex_meta_is_type(m)
	&& cuoo_type_from_meta(m)->shape == CUOO_SHAPE_METATYPE;
}

/** Cast \a e to a \ref cuoo_type "cuoo_type_t", assuming \ref cuoo_is_type(\a
 * e) is true. */
CU_SINLINE cuoo_type_t cuoo_type_from_ex(cuex_t e)
{ return (cuoo_type_t)e; }

/** The analytic type of \a t.  For elementary and opaque types, the type
 * itself. */
CU_SINLINE cuex_t cuoo_type_as_expr(cuoo_type_t t)
{ return t->as_expr? t->as_expr : t; }

/** Returns the \ref cuoo_shape_t "shape" of \a type. */
CU_SINLINE cuoo_shape_t cuoo_type_shape(cuoo_type_t type)
{ return type->shape; }

#define cuoo_type_impl(type, ...) ((type)->impl(__VA_ARGS__))
#define cuoo_type_impl_ptr(type, ...) \
    cu_unbox_ptr(void *, (type)->impl(__VA_ARGS__))
#define cuoo_type_impl_fptr(T, type, ...) \
    cu_unbox_fptr(T, (type)->impl(__VA_ARGS__))

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

/** Initialise \a type to be used for non-hash-consed objects.  \a shape is a
 * tag indicating the encoding of objects of this type.  \a atype is an
 * optional analytic type of the stored objects. */
void cuoo_type_init_general(cuoo_type_t type, cuoo_shape_t shape,
			    cuoo_impl_t impl, cuex_t atype);
/** Initialise \a type to be used for hash-consed objects of size
 * \a key_size, excluding the CU_HCOBJ header. */
void cuoo_type_init_general_hcs(cuoo_type_t type, cuoo_shape_t shape,
				cuoo_impl_t impl, cuex_t atype,
				size_t key_size);
/** Initialise \a type to be used for hash-consed objects with hash function \a
 * key_hash_fn. */
void cuoo_type_init_general_hcv(cuoo_type_t type, cuoo_shape_t shape,
				cuoo_impl_t impl, cuex_t atype);

/** Construct \a type as an opaque type for non-hash-consed objects. */
void cuoo_type_init_opaque(cuoo_type_t type, cuoo_impl_t impl);

/** Construct \a type as an opaque type for hash-consed objects of key size \a
 * key_size. */
void cuoo_type_init_opaque_hcs(cuoo_type_t type, cuoo_impl_t impl,
			       size_t key_size);

/** Construct \a type as an opaque type for hash-consed object with hash
 * function \a key_hash_fn. */
void cuoo_type_init_opaque_hcv(cuoo_type_t type, cuoo_impl_t impl);

/** Creates a type for non-hash-consed objects. */
cuoo_type_t cuoo_type_new_opaque(cuoo_impl_t impl);

/** Creates a type for hash-consed objects of size \a key_size, excluding the \c
 * CU_HCOBJ header. */
cuoo_type_t cuoo_type_new_opaque_hcs(cuoo_impl_t impl, size_t key_size);

/** Creates a type for hash-consed objects of with hash function
 * \a key_hash_fn. */
cuoo_type_t cuoo_type_new_opaque_hcv(cuoo_impl_t impl);

/** A new type of non-hash-consed types. */
cuoo_type_t cuoo_type_new_metatype(cuoo_impl_t impl);

/** A new type of types which are hash-consed based on the analytic expression
 * of the type. */
cuoo_type_t cuoo_type_new_metatype_hce(cuoo_impl_t impl);

/** A new type of types which are hash-consed with key size \a key_size. */
cuoo_type_t cuoo_type_new_metatype_hcs(cuoo_impl_t impl, size_t key_size);

/** A default implementation of \ref CUOO_INTF_TO_STR_FN for types. */
cu_str_t cuoo_type_to_str_default(cuex_t type);

/**  @}
 ** \name Type-Related Functions on Objects
 **  @{ */

void *cuoo_impl_ptr(cuex_t obj, cu_word_t intf);

cu_hash_t cuex_key_hash(void *obj);

#ifdef CUOO_ENABLE_KEYED_PROP
cuoo_propkey_t cuoo_propkey_create(void);

void cuoo_prop_set(cuex_t ex, cuoo_propkey_t key, void *val);
void *cuoo_prop_get(cuex_t ex, cuoo_propkey_t key);
#endif

/**  @}
 ** @} */

/*!\deprecated*/
#define cuoo_stdtype_from_type(type) (type)
/*!\deprecated*/
#define cuoo_stdtype_to_type(type) (type)

CU_END_DECLARATIONS

#endif
