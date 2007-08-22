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

#ifndef CU_OBJ_H
#define CU_OBJ_H

#include <cu/fwd.h>
#include <cu/thread.h>
#include <cu/inherit.h>
#include <cu/hcobj.h>
#include <cu/conf.h>
#include <cu/memory.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cu_dyn_h cu/dyn.h: Dynamically Typed Objects Basics
 * @{ \ingroup cudyn_mod */

/* Meta Descriptor Format
 *
 *   Objects:
 *	bit0 = 0
 *	bit1 = 0 (reserved)
 *	bit2..bitN = address of type descriptor divided by 4
 *
 *   For operations and other expression constructs the whole meta-word
 *   is considered to for the operator (opcode), and is composed of
 *	bit0 = 1
 *	bit1 = 0 (reserved)
 *	bit3..bit14 = arity
 *	bit14..bitN = distinguises operators of the same arity
 *   OBS. Leave arity less significant than the rest of the operator,
 *   so that client code can use '<' to distinuish special constructs
 *   from normal operators.
 */

#define CUEX_PRIoMETA PRIoPTR
#define CUEX_PRIuMETA PRIuPTR
#define CUEX_PRIxMETA PRIxPTR
#define CUEX_PRIXMETA PRIXPTR

#define CUEX_META_MASK(shift, width) \
    (((CUEX_META_C(1) << (width)) - CUEX_META_C(1)) << (shift))

#define CUEX_METAKIND_WIDTH	 2
#define CUEX_METAKIND_MASK	 3

#define CUEX_OPR_FLAGS_WIDTH	 1
#if CUCONF_SIZEOF_VOID_P == 4
#  define CUEX_OPR_ARITY_WIDTH	 9
#  define CUEX_OPR_SELECT_WIDTH	20
#elif CUCONF_SIZEOF_VOID_P == 8
#  define CUEX_OPR_ARITY_WIDTH	24
#  define CUEX_OPR_SELECT_WIDTH	37
#else
#  error Unexpected word size, hand-tune new case here.
#endif

#define CUEX_OPR_FLAGS_SHIFT	CUEX_METAKIND_WIDTH
#define CUEX_OPR_ARITY_SHIFT	(CUEX_OPR_FLAGS_SHIFT + CUEX_OPR_FLAGS_WIDTH)
#define CUEX_OPR_SELECT_SHIFT	(CUEX_OPR_ARITY_SHIFT + CUEX_OPR_ARITY_WIDTH)

#define CUEX_META_WIDTH (CUEX_OPR_SELECT_SHIFT + CUEX_OPR_SELECT_WIDTH)
#if CUEX_META_WIDTH != CUCONF_WIDTHOF_VOID_P
#  error CUEX_OPR_META_WIDTH does not add up to the width of a pointer
#endif

#define CUEX_OPR_FLAGS_MASK \
    CUEX_META_MASK(CUEX_OPR_FLAGS_SHIFT, CUEX_OPR_FLAGS_WIDTH)
#define CUEX_OPR_ARITY_MASK \
    CUEX_META_MASK(CUEX_OPR_ARITY_SHIFT, CUEX_OPR_ARITY_WIDTH)
#define CUEX_OPR_SELECT_MASK \
    CUEX_META_MASK(CUEX_OPR_SELECT_SHIFT, CUEX_OPR_SELECT_WIDTH)

#define CUEX_OPRFLAG_CTOR (CUEX_META_C(1) << CUEX_OPR_FLAGS_SHIFT)

typedef enum {
    cuex_meta_kind_type = 0,
    cuex_meta_kind_opr = 1,
    cuex_meta_kind_other = 2,	/* varmeta or extension */
    cuex_meta_kind_ignore = 3,	/* Collides with GC internal freelist link. */
} cuex_meta_kind_t;

#define cuex_meta_kind(meta) ((cuex_meta_kind_t)((meta) & 3))

#define cudyn_type_from_meta(meta) ((cudyn_type_t)(meta))
#define cudyn_type_to_meta(ptr) ((cuex_meta_t)(ptr))

/* Meta codes: other.varmeta */
#define CUEXP_VARMETA_SELECT_WIDTH 3
#define CUEXP_VARMETA_SELECT_MASK  7
#define CUEXP_VARMETA_SELECT_VALUE 6
#define CUEXP_VARMETA_WSIZE_SHIFT CUEXP_VARMETA_SELECT_WIDTH
#define CUEXP_VARMETA_WSIZE_WIDTH 2
#define CUEXP_VARMETA_WSIZE_MASK \
    CUEX_META_MASK(CUEXP_VARMETA_WSIZE_SHIFT, CUEXP_VARMETA_WSIZE_WIDTH)


/* Operators
 * ---------
 *
 * Operators are the same type as cuex_meta_t, no need for cast, but before
 * using cuex_opr_r, check that it is really an operator. */

/*!Returns the arity of the operator \a opr. */
#define cuex_opr_r(opr)							\
    ((int)(((opr) & CUEX_OPR_ARITY_MASK) >> CUEX_OPR_ARITY_SHIFT))


CU_SINLINE cuex_meta_t
cuex_meta(void *obj)
{
    return *((cuex_meta_t *)obj - 1) - 1;
}


/* Types  XXX to be moved to cudyn/type.h
 * ===== */

typedef enum {
    cudyn_typekind_none,
    cudyn_typekind_tvar,
    cudyn_typekind_stdtype,		/* Inherits cudyn_stdtype_s. */
    cudyn_typekind_stdtypeoftypes,	/* Inherits cudyn_stdtype_s. */
    cudyn_typekind_proto,
    cudyn_typekind_ctortype,
    cudyn_typekind_by_expr,

    /* inline types */
    cudyn_typekind_ptrtype,
    cudyn_typekind_arrtype,
    cudyn_typekind_tuptype,
    cudyn_typekind_sigtype,
    cudyn_typekind_duntype,
    cudyn_typekind_sngtype,

    /* inline types: elementary */
    cudyn_typekind_elmtype_MIN,
    cudyn_typekind_elmtype_bool = cudyn_typekind_elmtype_MIN,
    cudyn_typekind_elmtype_uint8,
    cudyn_typekind_elmtype_int8,
    cudyn_typekind_elmtype_uint16,
    cudyn_typekind_elmtype_int16,
    cudyn_typekind_elmtype_uint32,
    cudyn_typekind_elmtype_int32,
    cudyn_typekind_elmtype_uint64,
    cudyn_typekind_elmtype_int64,
    cudyn_typekind_elmtype_metaint,
    cudyn_typekind_elmtype_float,
    cudyn_typekind_elmtype_double,
    cudyn_typekind_elmtype_char
} cudyn_typekind_t;

typedef enum {
    cudyn_hcmethod_none = 0,	/* Not hashconsed. */
    cudyn_hcmethod_by_size,	/* Sets cudyn_type_s.u0.key_size */
    cudyn_hcmethod_by_size_fn,	/* Sets cudyn_type_s.u0.key_size_fn */
    cudyn_hcmethod_by_hash_fn	/* Sets cudyn_type_s.u0.key_hash_fn */
} cudyn_hcmethod_t;

struct cudyn_type_s
{
    CU_HCOBJ		/* used optionally */

    /* If non-null, as_expr is the expression-form of the type.  Since
     * it uniquely identifies the type, it is, when present, sufficient for
     * the HC key.
     * NB! This must be right after CU_HCOBJ, due to cudynP_type_cct_hce and
     * cudyn_type_glck. */
    cuex_t as_expr;

    cudyn_typekind_t typekind : 5;
    cudyn_hcmethod_t members_hcmethod : 2;

    union {
	size_t key_size;
	cu_clop(key_size_fn, size_t, void *obj);
	cu_clop(key_hash_fn, cu_hash_t, void *obj);
    } u0;
};
#define cuex_meta_is_type(meta) (cuex_meta_kind(meta) == cuex_meta_kind_type)

void cudyn_type_cct(cudyn_type_t type, cuex_t as_expr, cudyn_typekind_t kind);

CU_SINLINE cu_bool_t cudyn_is_type(cuex_t e)
{
    cuex_meta_t m = cuex_meta(e);
    return cuex_meta_is_type(m)
	&& cudyn_type_from_meta(m)->typekind == cudyn_typekind_stdtypeoftypes;
}

CU_SINLINE cudyn_type_t cudyn_type_from_ex(cuex_t e)
{ return (cudyn_type_t)e; }

/*!The expression form of \a t.  For elementary and opaque types, the type
 * itself.  For types of parametric kind, the instantiated form. */
CU_SINLINE cuex_t cudyn_type_as_expr(cudyn_type_t t)
{ return t->as_expr? t->as_expr : t; }

CU_SINLINE cudyn_typekind_t cudyn_type_typekind(cudyn_type_t type)
{ return type->typekind; }

CU_SINLINE cu_bool_t cudyn_type_is_hctype(cudyn_type_t type)
{ return type->members_hcmethod; }

CU_SINLINE cu_bool_t cudyn_type_is_stdtype(cudyn_type_t type)
{ return type->typekind <= cudyn_typekind_stdtypeoftypes; }

CU_SINLINE cu_bool_t cudyn_type_is_inltype(cudyn_type_t type)
{ return type->typekind >= cudyn_typekind_ptrtype; }

CU_SINLINE cu_bool_t cudyn_type_is_nonptr_inltype(cudyn_type_t type)
{ return type->typekind > cudyn_typekind_ptrtype; }

CU_SINLINE cu_bool_t cudyn_type_is_proto(cudyn_type_t type)
{ return type->typekind == cudyn_typekind_proto; }

CU_SINLINE cu_bool_t cudyn_type_is_typeoftypes(cudyn_type_t type)
{ return type->typekind == cudyn_typekind_stdtypeoftypes; }

void cudynP_type_cct_nonhc(cudyn_type_t type, cuex_t as_expr,
			   cudyn_typekind_t kind);
void cudynP_type_cct_hcs(cudyn_type_t type, cuex_t as_expr,
			 cudyn_typekind_t kind, size_t key_size);
void cudynP_type_cct_hce(cudyn_type_t type, cuex_t as_expr,
			 cudyn_typekind_t kind);
void cudynP_type_cct_hcv(cudyn_type_t type, cuex_t as_expr,
			 cudyn_typekind_t kind,
			 cu_clop(key_size_fn, size_t, void *));
void cudynP_type_cct_hcf(cudyn_type_t type, cuex_t as_expr,
			 cudyn_typekind_t kind,
			 cu_clop(key_hash_fn, cu_hash_t, void *));

struct cudyn_stdtype_s
{
    cu_inherit (cudyn_type_s);
    cu_clop(finalise, void, void *);
    cu_clop(conj, cu_bool_t, void *, cu_clop(, cu_bool_t, void *));
    cu_clop(tran, void *, void *, cu_clop(, void *, void *));
};

#define cudyn_stdtype_from_type(type) cu_from(cudyn_stdtype, cudyn_type, type)
#define cudyn_stdtype_to_type(stdtype) cu_to(cudyn_type, stdtype)
#define cudyn_stdtype_from_meta(meta) \
    cudyn_stdtype_from_type(cudyn_type_from_meta(meta))
#define cudyn_stdtype_to_meta(stdtype) \
    cudyn_type_to_meta(cudyn_stdtype_to_type(stdtype))

extern cudyn_stdtype_t cudynP_stdtype_type;
#define cudyn_stdtype_type() cudyn_stdtype_to_type(cudynP_stdtype_type)

/* Create a type with standard method slots. */
void cudyn_stdtype_cct(cudyn_stdtype_t, cudyn_typekind_t kind);
cudyn_stdtype_t cudyn_stdtype_new(void);
cudyn_stdtype_t cudyn_stdtypeoftypes_new(void);

/*!A new type of types.  If the instances are hash-consed, the expression
 * for of the type will be used as the key. */
cudyn_stdtype_t cudyn_stdtypeoftypes_new_hce(void);

/* Create a type for hash constructed objects. */
void cudyn_stdtype_cct_hcs(cudyn_stdtype_t, cudyn_typekind_t, size_t key_size);
cudyn_stdtype_t cudyn_stdtype_new_hcs(size_t key_size);
cudyn_stdtype_t cudyn_stdtypeoftypes_new_hcs(size_t key_size);

/* Create a type for hash constructed objects with variable size. */
void cudyn_stdtype_cct_hcv(cudyn_stdtype_t, cudyn_typekind_t,
			   cu_clop(key_size, size_t, void *));
cudyn_stdtype_t cudyn_stdtype_new_hcv(cu_clop(key_size, size_t, void *));


/* Hashconsed Objects
 * ================== */

size_t cuex_key_size(cuex_meta_t meta, void *obj);
cu_hash_t cuex_key_hash(void *);


/* Properties
 * ========== */

#ifdef CUDYN_ENABLE_KEYED_PROP
cudyn_propkey_t cudyn_propkey_create(void);

void cudyn_prop_set(cuex_t ex, cudyn_propkey_t key, void *val);
void *cudyn_prop_get(cuex_t ex, cudyn_propkey_t key);
#endif

/*!@}*/
CU_END_DECLARATIONS

#endif
