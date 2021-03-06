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

#ifndef CUEX_OPN_H
#define CUEX_OPN_H

#include <cuex/fwd.h>
#include <cuex/ex.h>
#include <cuoo/halloc.h>
#include <cu/clos.h>
#include <cu/ptr_seq.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuex_opn_h cuex/opn.h: Additional Interface for Operations
 * @{\ingroup cuex_mod */

struct cuex_opn
{
    CUOO_HCOBJ
    cuex_t operand_arr[1];
};
#define cuex_sizeof_opn(N) \
	(sizeof(struct cuex_opn) - sizeof(void *) + (N)*sizeof(void *))

/*!Create an operation of \a opr applied to the vararg arguments.  The
 * function expects \c cuex_opr_r(opr) vararg arguments of type \c cuex_t,
 * which should be dynamically typed objects and operations. */
cuex_opn_t cuex_opn(cuex_meta_t opr, /*operand*/...);

/*!As \ref cuex_opn, but with varargs prepared by caller. */
cuex_opn_t cuex_opn_by_valist(cuex_meta_t opr, va_list operand_valist);

cuex_opn_t cuexP_opn_by_arr_with_ctor(cuex_meta_t opr, cuex_t *arr);

/*!Create an operation of \a opr applied to \a arr.  The array
 * should contain \c cuex_opr_r(opr) values of dynamically typed objects
 * and operations. */
CU_SINLINE cuex_opn_t
cuex_opn_by_arr(cuex_meta_t opr, cuex_t *arr)
{
    if (cu_expect_false(cuex_opr_has_ctor(opr)))
	return cuexP_opn_by_arr_with_ctor(opr, arr);
    return (cuex_opn_t)cuexP_halloc_raw(
	opr,
	CUOO_HCOBJ_KEY_SIZEW(cuex_opr_r(opr)*sizeof(cuex_t) + CUOO_HCOBJ_SHIFT),
	arr);
}

/*!Returns the arity of the operator of \a opn. */
CU_SINLINE cu_rank_t cuex_opn_arity(cuex_t opn)
{ return cuex_opr_r(cuex_meta(opn)); }

/*!Returns operand \a i of \a opn, counting from zero. */
CU_SINLINE cuex_t cuex_opn_at(cuex_t opn, int i)
{ return ((cuex_opn_t)opn)->operand_arr[i]; }

/*!Returns the start of the operand array of \a opn. */
CU_SINLINE cuex_t *cuex_opn_begin(cuex_t opn)
{ return ((cuex_opn_t)opn)->operand_arr; }

/*!Returns past-the-end of the operand array. */
CU_SINLINE cuex_t *cuex_opn_end(cuex_t opn)
{ return ((cuex_opn_t)opn)->operand_arr + cuex_opr_r(cuex_meta(opn)); }

/*!A template for iterating over operands of an operation.
 * Given an operation \a opn with operator \a opr, bind each operand to
 * \a sub and call \a STMT.  This macro declares \a sub as <tt>cuex_t</tt>
 * such that it is in scope when \a STMT is evaluated.
 * 
 * \note The \a opr parameter is redundant since it must be exactly
 * <tt>cuex_meta(\a opn)</tt>.  It is passed as an optimisation, since
 * the caller usually must have obtained it before using this template,
 * in order to determine that \a opn is in fact an operation. */
#define CUEX_OPN_ITER(opr, opn, sub, STMT)				\
do {									\
    int cuexL_i, cuexL_r = cuex_opr_r(opr);				\
    for (cuexL_i = 0; cuexL_i < cuexL_r; ++cuexL_i) {			\
	cuex_t sub = cuex_opn_at(opn, cuexL_i);				\
	STMT;								\
    }									\
} while (0)

/*!A template for iterating over operands of an operaton as long as a
 * condition holds.
 * Given an operation \a opn with operator \a opr, bind each operand to
 * \a sub and evaluate \a EXPR.  If \a EXPR returns false, immediately
 * return false from the surrounding function.  This macro declares \a sub
 * as <tt>cuex_t</tt> such that it is in scope when \a EXPR is evaluated. */
#define CUEX_OPN_CONJ_RETURN(opr, opn, sub, EXPR)			\
do {									\
    int cuexL_i, cuexL_r = cuex_opr_r(opr);				\
    for (cuexL_i = 0; cuexL_i < cuexL_r; ++cuexL_i) {			\
	cuex_t sub = cuex_opn_at(opn, cuexL_i);				\
	if (!(EXPR))							\
	    return cu_false;						\
    }									\
} while (0)

/*!Analogous to \ref CUEX_OPN_CONJ_RETURN, but return true if \a EXPR evaluates
 * to true. */
#define CUEX_OPN_DISJ_RETURN(opr, opn, sub, EXPR)			\
do {									\
    int cuexL_i, cuexL_r = cuex_opr_r(opr);				\
    for (cuexL_i = 0; cuexL_i < cuexL_r; ++cuexL_i) {			\
	cuex_t sub = cuex_opn_at(opn, cuexL_i);				\
	if (EXPR)							\
	    return cu_true;						\
    }									\
} while (0)

/*!A template to transform an operation.
 * Reassign \a opn with an operation of operator \a opr, which must be the
 * same as the operator of \a opn, with operands obtained as follows.  For
 * each operand of \a opn, bind it to \a old_sub and then evaluate
 * \a EXPR to obtain the new operand.  \a old_sub is declared locally
 * by this macro and is in scope where \a EXPR is evaluated.  */
#define CUEX_OPN_TRAN(opr, opn, old_sub, EXPR)				\
do {									\
    int cuexL_i, cuexL_r = cuex_opr_r(opr);				\
    for (cuexL_i = 0; cuexL_i < cuexL_r; ++cuexL_i) {			\
	cuex_t old_sub, cuexL_new_sub;					\
	old_sub = cuex_opn_at(opn, cuexL_i);				\
	cuexL_new_sub = EXPR;						\
	if (old_sub != cuexL_new_sub) {					\
	    cuex_t *new_arr = cu_salloc(sizeof(cuex_t)*cuexL_r);	\
	    memcpy(new_arr, cuex_opn_begin(opn), sizeof(cuex_t)*cuexL_r); \
	    new_arr[cuexL_i] = cuexL_new_sub;				\
	    for (++cuexL_i; cuexL_i < cuexL_r; ++cuexL_i) {		\
		old_sub = cuex_opn_at(opn, cuexL_i);			\
		new_arr[cuexL_i] = EXPR;				\
	    }								\
	    opn = cuex_opn_by_arr(opr, new_arr);			\
	    break;							\
	}								\
    }									\
} while (0)

/*!A template to transform an operation, with loop-termination on failure.
 * Same as \ref CUEX_OPN_TRAN, except that \a EXPR is allowed to return
 * \c NULL, in which case the loop terminates an \a opn is assigned
 * \c NULL. */
#define CUEX_OPN_TRAN_NULL(opr, opn, old_sub, EXPR)			\
do {									\
    int cuexL_i, cuexL_r = cuex_opr_r(opr);				\
    for (cuexL_i = 0; cuexL_i < cuexL_r; ++cuexL_i) {			\
	cuex_t old_sub, cuexL_new_sub;					\
	old_sub = cuex_opn_at(opn, cuexL_i);				\
	cuexL_new_sub = EXPR;						\
	if (!cuexL_new_sub) {						\
	    opn = NULL;							\
	    break;							\
	}								\
	if (old_sub != cuexL_new_sub) {					\
	    cuex_t *new_arr = cu_salloc(sizeof(cuex_t)*cuexL_r);	\
	    memcpy(new_arr, cuex_opn_begin(opn), sizeof(cuex_t)*cuexL_r); \
	    new_arr[cuexL_i] = cuexL_new_sub;				\
	    for (++cuexL_i; cuexL_i < cuexL_r; ++cuexL_i) {		\
		old_sub = cuex_opn_at(opn, cuexL_i);			\
		new_arr[cuexL_i] = EXPR;				\
		if (new_arr[cuexL_i] == NULL)				\
		    break;						\
	    }								\
	    if (cuexL_i == cuexL_r)					\
		opn = cuex_opn_by_arr(opr, new_arr);			\
	    else							\
		opn = NULL;						\
	    break;							\
	}								\
    }									\
} while (0)

/*!Return left-associated application of \a opr on \a x and \a y.  Time
 * complexity is linear in the number of top-level \a opr in \a y.
 * \pre \a opr is a binary operator. */
cuex_opn_t cuex_opn2_left(cuex_meta_t opr, cuex_t x, cuex_t y);

/*!Return right-associated application of \a opr on \a x and \a y.  Time
 * complexity is linear in the number of top-level \a opr in \a x.
 * \pre \a opr is a binary operator. */
cuex_opn_t cuex_opn2_right(cuex_meta_t opr, cuex_t x, cuex_t y);

/*!The arity if \a e is an operator, the compound size if \a e is a compound,
 * otherwise 0. */
cu_rank_t cuex_arity(cuex_t e);

#define cuex_opn_comm_A		cuex_opn_comm_iterA
#define cuex_opn_comm_Ak	cuex_opn_comm_iterAk
#define cuex_opn_comm_img	cuex_opn_comm_iterimg
#define cuex_opn_comm_imgk	cuex_opn_comm_iterimgk

cu_bool_t cuex_opn_comm_iterA(cu_clop(f, cu_bool_t, cuex_t), cuex_t e);
cu_bool_t cuex_opn_comm_iterAk(cu_clop(f, cu_bool_t, int, cuex_t), cuex_t e);
cuex_t cuex_opn_comm_iterimg(cu_clop(f, cuex_t, cuex_t), cuex_t e);
cuex_t cuex_opn_comm_iterimgk(cu_clop(f, cuex_t, int, cuex_t), cuex_t e);

struct cuex_opn_source
{
    cu_inherit (cu_ptr_source);
    int i;
    cuex_t e;
};

void cuex_opn_ncomm_source_init(cuex_opn_source_t src, cuex_t e);

cu_ptr_source_t cuex_opn_ncomm_source(cuex_t e);

void cuex_opn_comm_source_init(cuex_opn_source_t src, cuex_t e);

cu_ptr_source_t cuex_opn_comm_source(cuex_t e);

/*!@}*/
CU_END_DECLARATIONS

#endif
