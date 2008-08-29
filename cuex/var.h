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

#ifndef CUEX_VAR_H
#define CUEX_VAR_H

#include <cu/fwd.h>
#include <cuex/fwd.h>
#include <cuex/qcode.h>
#include <cu/conf.h>
#include <cuoo/oalloc.h>
#include <cuoo/halloc.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuex_var_h cuex/var.h: Variables
 * @{\ingroup cuex_mod
 *
 * Variables are structureless expressions.  They have no properties except
 * for pointer equality, quantisation, and their type.  Their main use is as
 * placeholders for values to be \ref cuex_subst_h "substituted" or as to
 * express patterns (cf \ref cuex_ssfn_h).
 *
 * Quantisation can not be changed once a variable is created.  Instead,
 * after analysis, create new variables of the correct quantisation,
 * optionally put the in a \c cucon_pmap_t or \c cuex_subst_t, and
 * transform the expressions accordingly.
 */

typedef enum {
    cuex_varkind_svar,
    cuex_varkind_tvar,
    cuex_varkind_tpvar,
    cuex_varkind_fpvar,
    cuex_varkind_pvar,
    cuex_varkind_ivar,
    cuex_varkind_rvar,
    cuex_varkind_xvar,
} cuex_varkind_t;

/* Variable meta format: (index, kind, qcode, wsize, 2'b10) */
#define CUEXP_VARMETA_QCODE_WIDTH	 2
#define CUEXP_VARMETA_KIND_WIDTH	 3
#if CUCONF_SIZEOF_VOID_P == 4
#  define CUEXP_VARMETA_INDEX_WIDTH	22
#elif CUCONF_SIZEOF_VOID_P == 8
#  define CUEXP_VARMETA_INDEX_WIDTH	54
#else
#  error Unexpected word size, hand-tune new case here.
#endif

#define CUEXP_VARMETA_QCODE_SHIFT \
    (CUEXP_VARMETA_WSIZE_SHIFT + CUEXP_VARMETA_WSIZE_WIDTH)
#define CUEXP_VARMETA_KIND_SHIFT \
    (CUEXP_VARMETA_QCODE_SHIFT + CUEXP_VARMETA_QCODE_WIDTH)
#define CUEXP_VARMETA_INDEX_SHIFT \
    (CUEXP_VARMETA_KIND_SHIFT + CUEXP_VARMETA_KIND_WIDTH)
#define CUEXP_VARMETA_QCODE_MASK \
    CUEX_META_MASK(CUEXP_VARMETA_QCODE_SHIFT, CUEXP_VARMETA_QCODE_WIDTH)
#define CUEXP_VARMETA_KIND_MASK	 \
    CUEX_META_MASK(CUEXP_VARMETA_KIND_SHIFT, CUEXP_VARMETA_KIND_WIDTH)
#define CUEXP_VARMETA_INDEX_MASK \
    CUEX_META_MASK(CUEXP_VARMETA_INDEX_SHIFT, CUEXP_VARMETA_INDEX_WIDTH)

#if CUEXP_VARMETA_INDEX_SHIFT+CUEXP_VARMETA_INDEX_WIDTH!=CUCONF_WIDTHOF_VOID_P
#  error Widths for variable meta fields do not add up to word size.
#endif

/* Create a varmeta with non-zero key size. */
#define cuex_varmeta_kqis(kind, qcode, index, wsize)			\
    (CUEXP_VARMETA_SELECT_VALUE						\
     | ((wsize) << CUEXP_VARMETA_WSIZE_SHIFT)				\
     | ((kind) << CUEXP_VARMETA_KIND_SHIFT)				\
     | ((qcode) << CUEXP_VARMETA_QCODE_SHIFT)				\
     | ((index) << CUEXP_VARMETA_INDEX_SHIFT) )

/* Create a varmeta with zero key size. */
#define cuex_varmeta_kqi(kind, qcode, index) 				\
    cuex_varmeta_kqis(kind, qcode, index, 0)

/* Select on (kind, qcode, index, wsize) (full select) */
#define cuex_is_varmeta_kqis(meta, kind, qcode, index, wsize)		\
    ((meta) == cuex_varmeta_kqis(kind, qcode, index, wsize))

/* Select on (kind, qcode, index, wsize) where wsize == 0 (full select) */
#define cuex_is_varmeta_kqi(meta, kind, qcode, index)			\
    ((meta) == cuex_varmeta_kqi(kind, qcode, index))

/* Select on (kind, qcode), match any (index, wsize) */
#define cuex_is_varmeta_kq(meta, kind, qcode)				\
    ((( CUEXP_VARMETA_SELECT_MASK					\
      | CUEXP_VARMETA_KIND_MASK						\
      | CUEXP_VARMETA_QCODE_MASK ) & (meta))				\
     == cuex_varmeta_kqi(kind, qcode, 0))

#define cuex_is_varmeta_ki(meta, kind, index)				\
    ((( CUEXP_VARMETA_SELECT_MASK					\
      | CUEXP_VARMETA_KIND_MASK						\
      | CUEXP_VARMETA_INDEX_MASK ) & (meta))				\
     == cuex_varmeta_kqi(kind, 0, index))

/* Select on kind, match any (qcode, index, wsize) */
#define cuex_is_varmeta_k(meta, kind)					\
    ((( CUEXP_VARMETA_SELECT_MASK					\
      | CUEXP_VARMETA_KIND_MASK ) & (meta))				\
     == cuex_varmeta_kqi(kind, 0, 0))

/* Select on qcode, match any (kind, index, wsize) */
#define cuex_is_varmeta_q(meta, qcode)					\
    ((( CUEXP_VARMETA_SELECT_MASK					\
      | CUEXP_VARMETA_QCODE_MASK ) & (meta))				\
     == cuex_varmeta_kqi(0, qcode, 0))


/* Varmeta Interface
 * ----------------- */

/*!The meta of a simple variable quantified as \a qcode. */
#define cuex_is_varmeta(meta)						\
    (((CUEXP_VARMETA_SELECT_MASK) & (meta)) == cuex_varmeta_kqi(0, 0, 0))

/*!The quantification of variables of \a meta. */
#define cuex_varmeta_qcode(meta)					\
    ((cuex_qcode_t)							\
     (((meta) & CUEXP_VARMETA_QCODE_MASK) >> CUEXP_VARMETA_QCODE_SHIFT))


#define cuex_varmeta_index(meta)					\
    (((meta) & CUEXP_VARMETA_INDEX_MASK ) >> CUEXP_VARMETA_INDEX_SHIFT)

#define cuex_is_varmeta_u(meta) cuex_is_varmeta_q(meta, cuex_qcode_u)
#define cuex_is_varmeta_e(meta) cuex_is_varmeta_q(meta, cuex_qcode_e)
#define cuex_is_varmeta_w(meta) cuex_is_varmeta_q(meta, cuex_qcode_w)
#define cuex_is_varmeta_n(meta) cuex_is_varmeta_q(meta, cuex_qcode_n)


/* Plain Variables
 * --------------- */

/*!True iff \a ex is a variable, either cuex_var_t or cuex_pvar_t. */
CU_SINLINE cu_bool_t	cuex_is_var(cuex_t ex)
{ return cuex_is_varmeta(cuex_meta(ex)); }

#define cuex_var_to_ex(var) ((cuex_t)(var))
#define cuex_var_from_ex(ex) ((cuex_var_t)(ex))

/*!Returns the quantisation of \a var. */
#define cuex_var_qcode(var) cuex_varmeta_qcode(cuex_meta(var))

/*!Returns a new variable with quantisation \a qcode.  The variable has
 * no properties besides its type, and pointer equality. */
CU_SINLINE cuex_var_t	cuex_var_new(cuex_qcode_t qcode)
{ return (cuex_var_t)cuexP_oalloc(cuex_varmeta_kqi(cuex_varkind_svar,
						   qcode, 0), 0); }

/*!Creates an existential variable. */
#define cuex_var_new_e() cuex_var_new(cuex_qcode_e)

/*!Creates a universal variable. */
#define cuex_var_new_u() cuex_var_new(cuex_qcode_u)

/*!Creates a weak universal (or parametric) variable. Roughly, this is a
 * variable which is quantified to the extent that the expression it
 * occurs in do not contradict more specialised expressions. The precise
 * definition may be application dependent. */
#define cuex_var_new_w() cuex_var_new(cuex_qcode_w)

/*!Create an unquantified variable.  Used before quantisation is analysed
 * or in cases where it does not apply. */
#define cuex_var_new_n() cuex_var_new(cuex_qcode_n)

/*!\defgroup cuex_ivar_rvar_group cuex_ivar, cuex_rvar: Indexed Variables
 * @{\ingroup cuex_var_h_mod
 * The indexed variables are hash-consed on their meta, which means that two
 * variables of the same kind, quantisation and index are equal.
 * The cuex_rvar variant is used to refer back to components of \ref
 * CUEX_OR_TUPLE on the LHS of a \ref CUEX_O2_RBIND.
 * nodes in recursive expressions.
 * The cuex_ivar variant is useful for de Bruijn indexing of lambda
 * variables.
 */
/*!The meta of an variable quantified as \a qcode and indexed with
 * \a index. */
#define cuex_ivarmeta(qcode, index) \
    cuex_varmeta_kqi(cuex_varkind_ivar, qcode, index)

/*!True iff \a meta is of an indexed variable of any quantisation. */
#define cuex_is_ivarmeta(meta) cuex_is_varmeta_k(meta, cuex_varkind_ivar)

/*!True iff \a meta is of an indexed variable of quantisation \a qcode. */
#define cuex_is_ivarmeta_q(meta, qcode)					\
    cuex_is_varmeta_kq(meta, cuex_varkind_ivar, qcode)

/*!Create a variable with quantisation \a quant and an associated integer
 * \a index. */
CU_SINLINE cuex_var_t cuex_ivar(cuex_qcode_t qcode, unsigned int index)
{ return (cuex_var_t)cuexP_halloc(cuex_ivarmeta(qcode, index), 0, NULL); }
#define cuex_ivar_e(i) cuex_ivar(cuex_qcode_e, i)
#define cuex_ivar_u(i) cuex_ivar(cuex_qcode_u, i)
#define cuex_ivar_w(i) cuex_ivar(cuex_qcode_w, i)
#define cuex_ivar_n(i) cuex_ivar(cuex_qcode_n, i)

/*!Gives the index of the meta of \a var. */
CU_SINLINE unsigned int	cuex_ivar_index(cuex_var_t var)
{ return cuex_varmeta_index(cuex_meta(var)); }


/*!The meta of a variable used to refer to an operand of
 * \ref CUEX_OR_TUPLE on the LHS of a \ref CUEX_O2_RBIND. */
#define cuex_rvarmeta(index) \
    cuex_varmeta_kqi(cuex_varkind_rvar, cuex_qcode_n, index)

/*!True iff \a meta is \ref cuex_rvarmeta(\e index) for some \e index. */
#define cuex_is_rvarmeta(meta) cuex_is_varmeta_k(meta, cuex_varkind_rvar)

/*!A variable which refers to component \a index of the LHS \ref
 * CUEX_OR_TUPLE of the closest surrounding \ref CUEX_O2_RBIND node. */
CU_SINLINE cuex_var_t cuex_rvar(unsigned int index)
{ return (cuex_var_t)cuexP_halloc(cuex_rvarmeta(index), 0, NULL); }

/*!\copydoc cuex_ivar_index. */
#define cuex_rvar_index(var) cuex_ivar_index(var)

/*!@}*/

typedef struct cuex_xvarops_s *cuex_xvarops_t;
struct cuex_xvarops_s
{
    void (*print)(void *, FILE *);
};

extern struct cucon_umap_s cuexP_xvarops;

/*!Compose a meta for an extended variable kind from the given quantisation,
 * subkind and slot size.  The slot size \a wsize is in words
 * (\ref cu_word_t). */
#define cuex_xvarmeta(qcode, subkind, wsize) \
    cuex_varmeta_kqis(cuex_varkind_xvar, qcode, subkind, wsize)

/*!Call this at startup to register new variable subkinds.  This checks that \a
 * subkind is unique and registers the callbacks \a ops for variables of this
 * kind.  It is up to the client to ensure that \a subkind is unique across the
 * codebase.  Alternatively, pass \a subkind = \c (cuex_meta_t)(-1) to
 * dynamically allocate a free subkind.  The registered subkind is returned.
 *
 * This shall be called from the main thread. */
cuex_meta_t cuex_register_xvarkind(cuex_meta_t subkind, unsigned int wsize,
				   cuex_xvarops_t ops);

/*!True iff \a meta is describes an extended variable kind. */
CU_SINLINE cu_bool_t
cuex_is_xvarmeta(cuex_meta_t meta)
{ return cuex_is_varmeta_k(meta, cuex_varkind_xvar); }

/*!True iff \a meta is an extended variable kind meta of with the given subkind. */
CU_SINLINE cu_bool_t
cuex_is_xvarmeta_k(cuex_meta_t meta, cuex_varkind_t subkind)
{ return cuex_is_varmeta_ki(meta, cuex_varkind_xvar, subkind); }

/*!True iff \a meta is an extended variable kind meta of the given subkind and
 * quantification. */
CU_SINLINE cu_bool_t
cuex_is_xvarmeta_kq(cuex_meta_t meta, cuex_varkind_t subkind,
		    cuex_qcode_t qcode)
{ return cuex_is_varmeta_kqi(meta, cuex_varkind_xvar, qcode, subkind); }

/*!Extracts the subkind of \a meta which is assumed to describe an extended
 * variable kind. */
CU_SINLINE cuex_meta_t
cuex_xvarmeta_subkind(cuex_meta_t meta)
{ return cuex_varmeta_index(meta); }

/*!Allocate a variable of extended kind with meta \a xvarmeta. */
CU_SINLINE cuex_t
cuex_xvar_alloc(cuex_meta_t xvarmeta)
{
    return cuexP_oalloc(xvarmeta,
			cuexP_varmeta_wsize(xvarmeta)*sizeof(cu_word_t));
}

/*!@}*/
CU_END_DECLARATIONS

#endif
