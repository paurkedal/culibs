/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2009  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUEX_ITERATION_H
#define CUEX_ITERATION_H

#include <cuex/fwd.h>
#include <cu/clos.h>

CU_BEGIN_DECLARATIONS
/** \defgroup cuex_iteration_h cuex/iteration.h: Iteration over Subterms
 ** @{ \ingroup cuex_mod
 **
 ** The following implements various basic forms of iteration and imaging of
 ** expressions, taking into account both plain operations and compounds.  Some
 ** hints to remember the function names:
 **   - \c A — Like the ∀ operator, the predicate holds for all subexpressions.
 **   - \c E — Like the ∃ operator, the predicate holds for at least one
 **     subexpression.
 **   - \c img — The image under the given function
 **   - <tt>iter</tt>\e X — An iterative (sequential) variant of the algorithm \e X
 **   - <tt>bare</tt>\e X — A variant of \e X which takes a bare (context-free)
 **     function.
 **   - <em>X</em>\c k — A variant of \e X which pass the operand number as the
 **     first argument.
 **
 ** Functions ending with \c _view take a first argument \ref cuex_opview_t,
 ** which specifies how the operands are passed to the callback.  In
 ** particular, operations and non-commutative containers can be given a
 ** commutative view.  Each operand will then be replaced on the fly by
 ** <tt>cuex_o2_metapair(\e ei, \e eo)</tt> where \e ei is a \ref cudyn_int of
 ** the operand number and \e eo is the original operand.  Callbacks to image
 ** functions are expected to return the result in the same format.
 **/

/** The compound view specifies how to iterate over or reconstruct operations
 ** and compounds. */
typedef enum {
    /** Non-commutative view for plain operations, preferred view according to
     ** their implementation for compounds. */
    CUEX_PREF_VIEW,

    /** Semi-commutative view, meaning non-commutative for plain operations and
     ** commutative for all kinds of compounds. */
    CUEX_SCOMM_VIEW,

    /** Fully commutative view, meaning commutative for operations as well as
     ** compounds. */
    CUEX_FCOMM_VIEW
} cuex_opview_t;

#if defined(CUCONF_PARALLELIZE) || defined(CU_IN_DOXYGEN)

/** True iff \a f takes every immediate subexpression of \a e to true.  This is
 ** the same as \ref cuex_iterA_operands, but with unordered invocation, and
 ** the additional requirement that \a f is safe for parallel invocation.  */
cu_bool_t cuex_A_operands(cu_clop(f, cu_bool_t, cuex_t), cuex_t e);

/** Like \ref cuex_A_operands but with a specified view of operands. */
cu_bool_t cuex_A_operands_view(cuex_opview_t view,
			       cu_clop(f, cu_bool_t, cuex_t), cuex_t e);

/** Like \ref cuex_A_operands but also pass the operand number to \a f. */
cu_bool_t cuex_Ak_operands(cu_clop(f, cu_bool_t, int, cuex_t), cuex_t e);

/** Like \ref cuex_A_operands but also pass the operand number to \a f and use
 ** the specified view of the operands. */
cu_bool_t cuex_Ak_operands_view(cuex_opview_t view,
				cu_clop(f, cu_bool_t, int, cuex_t), cuex_t e);

/** True iff \a f takes some immediate subexpression of \a e to true.  This is
 ** the same as \ref cuex_iterE_operand, but with unordered invocation, and the
 ** additional requirement that \a f is safe for parallel invocation. */
cu_bool_t cuex_E_operand(cu_clop(f, cu_bool_t, cuex_t), cuex_t e);

#else
#  define cuex_A_operands	cuex_iterA_operands
#  define cuex_A_operands_view	cuex_iterA_operands_view
#  define cuex_Ak_operands	cuex_iterAk_operands
#  define cuex_Ak_operands_view	cuex_iterAk_operands_view
#  define cuex_E_operand	cuex_iterE_operand
#endif

/** A variant of \ref cuex_A_operands specialised for context-free callbacks. */
cu_bool_t cuex_bareA_operands(cu_bool_t (*f)(cuex_t), cuex_t e);

/** A variant of \ref cuex_E_operand specialised for context-free callbacks. */
cu_bool_t cuex_bareE_operand(cu_bool_t (*f)(cuex_t), cuex_t e);

/** Calls \a f on each immediate subexpression of \a e in operand order. */
void cuex_iter_operands(cu_clop(f, void, cuex_t), cuex_t e);

/** Calls \a f on each subexpression of \a e in operand order and builds the
 ** conjunction of the results, exiting as soon as \a f returns false. */
cu_bool_t cuex_iterA_operands(cu_clop(f, cu_bool_t, cuex_t), cuex_t e);

/** Like \ref cuex_iterA_operands but using specified view of operands. */
cu_bool_t cuex_iterA_operands_view(cuex_opview_t view,
				   cu_clop(f, cu_bool_t, cuex_t), cuex_t e);

/** Like \a ref cuex_iterA_operands but also pass the operand number to
 ** \a f. */
cu_bool_t cuex_iterAk_operands(cu_clop(f, cu_bool_t, int, cuex_t), cuex_t e);

/** Like \ref cuex_iterA_operands but also pass the operand number to \a f and
 ** use the specified view. */
cu_bool_t cuex_iterAk_operands_view(cuex_opview_t view,
				    cu_clop(f, cu_bool_t, int, cuex_t),
				    cuex_t e);

/** Calls \a f on each subexpression of \a e in operand order and builds the
 ** disjunction of the results, exiting as soon as \a f returns true. */
cu_bool_t cuex_iterE_operand(cu_clop(f, cu_bool_t, cuex_t), cuex_t e);

#if defined(CUCONF_PARALLELIZE) || defined(CU_IN_DOXYGEN)

/** Returns the result of replacing each immediate subexpression of \a e with
 ** its mapping under \a f.  This is the same as \ref cuex_iterimg_operands,
 ** but with unordered invocation, and the additional requirement that \a f is
 ** safe for parallel invocation. */
cuex_t cuex_img_operands(cu_clop(f, cuex_t, cuex_t), cuex_t e);

/** A variant of \ref cuex_img_operands with a specified view of operands. */
cuex_t cuex_img_operands_view(cuex_opview_t view,
			      cu_clop(f, cuex_t, cuex_t), cuex_t e);

/** As \ref cuex_img_operands but also pass the operand number as the first
 ** argument to \a f. */
cuex_t cuex_imgk_operands(cu_clop(f, cuex_t, int, cuex_t), cuex_t e);

/** A variant of \ref cuex_imgk_operands with a specified view of operands. */
cuex_t cuex_imgk_operands_view(cuex_opview_t view,
			       cu_clop(f, cuex_t, int, cuex_t), cuex_t e);

#else
#  define cuex_img_operands		cuex_iterimg_operands
#  define cuex_img_operands_view	cuex_iterimg_operands_view
#  define cuex_imgk_operands		cuex_iterimgk_operands
#  define cuex_imgk_operands_view	cuex_iterimgk_operands_view
#endif

/** A variant of \ref cuex_img_operands specialised for context-free callbacks. */
cuex_t cuex_bareimg_operands(cuex_t (*f)(cuex_t), cuex_t e);

/** Returns the result of replacing each immediate subexpression of \a e with
 ** its mapping under \a f, where \a f is called in operand order.  If \a f
 ** returns \c NULL, this function immediately returns \c NULL, as well. */
cuex_t cuex_iterimg_operands(cu_clop(f, cuex_t, cuex_t), cuex_t e);

/** As \ref cuex_iterimg_operands but using a specified of operands. */
cuex_t cuex_iterimg_operands_view(cuex_opview_t view,
				  cu_clop(f, cuex_t, cuex_t), cuex_t e);

/** Like \ref cuex_iterimg_operands but also pass the operand number as the
 ** first argument to \a f. */
cuex_t cuex_iterimgk_operands(cu_clop(f, cuex_t, int, cuex_t), cuex_t e);

/** As \ref cuex_iterimgk_operands but using a specified of operands. */
cuex_t cuex_iterimgk_operands_view(cuex_opview_t view,
				   cu_clop(f, cuex_t, int, cuex_t), cuex_t e);

/** @} */
CU_END_DECLARATIONS

#endif
