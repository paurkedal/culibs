/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUEX_COMPOUND_H
#define CUEX_COMPOUND_H

#include <cuoo/type.h>
#include <cuex/fwd.h>
#include <cuex/intf.h>
#include <cu/clos.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuex_compound_h cuex/compound.h: Compound Expressions
 *@{\ingroup cuex_mod
 *
 * Compound expressions, or compounds for short, are containers of expressions
 * (dynamic objects and operations).
 * Compounds are also themselves expressions.
 * One thing you can do an compounds without knowing more about their structure
 * is to iterate over their subexpressions.
 * Further, compounds may support creation of their image under a function or
 * iterable construction.
 *
 * The following terminology will reflect that compounds are geared towards
 * representing associative binary operations, though it is just as useful for
 * containers.
 * We distinguish between non-commutative and commutative views.  A compound
 * may bind one or both views.
 * For example, a map may be an commutative compound of key-value pairs, or a
 * non-commutative compound of just the values where the keys are treated
 * inertly.
 * Conversely, a vector may have a commutative iterator where the values are
 * index-value pairs.
 */

typedef struct cuex_intf_compound_s *cuex_intf_compound_t;

/*!Set if the non-commutative interface is preferable when an algorithm
 * supports both.
 * This flag \e must be set if the commutative interface is unbound. */
#define CUEX_COMPOUNDFLAG_PREFER_NCOMM 1

/*!Set if the commutative interface is preferable when an algorithm supports
 * both.
 * This flag \e must be set if the non-commutative interface is unbound. */
#define CUEX_COMPOUNDFLAG_PREFER_COMM 2

/*!Set iff successive elements which are equal reduces to a single occurrence
 * for the non-commutative interface. */
#define CUEX_COMPOUNDFLAG_NCOMM_IDEMPOTENT 4

/*!Set iff equal elements reduces to a single occurrence for the commutative
 * interface.  */
#define CUEX_COMPOUNDFLAG_COMM_IDEMPOTENT 8

/*!Set iff \ref cuex_intf_compound_s::ncomm_image_junctor allows dropping
 * elements by omitting some calls to \ref cu_ptr_junction_put. */
#define CUEX_COMPOUNDFLAG_NCOMM_FILTERABLE_IMAGE 16

/*!Set iff \ref cuex_intf_compound_s::comm_image_junctor allows dropping
 * elements by omitting some calls to \ref cu_ptr_junction_put. */
#define CUEX_COMPOUNDFLAG_COMM_FILTERABLE_IMAGE 32

/*!Set iff \ref cuex_intf_compound_s::ncomm_image_junctor allows adding extra
 * elements by calling \ref cu_ptr_junction_put more than once after a call to
 * \ref cu_ptr_junction_get. */
#define CUEX_COMPOUNDFLAG_NCOMM_EXPANSIVE_IMAGE 64

/*!Set iff \ref cuex_intf_compound_s::comm_image_junctor allows adding extra
 * elements by calling \ref cu_ptr_junction_put more than once after a call to
 * \ref cu_ptr_junction_get. */
#define CUEX_COMPOUNDFLAG_COMM_EXPANSIVE_IMAGE 128

/*!Interface for compound expressions.
 * This struct is meant to be declared with static storage and initialised
 * using C99 struct member designators and must have static storage so that any
 * future added fields become \c NULL.
 * An alternative to using member designators is to provide an initialisation
 * function.
 * In any case, \ref cuex_intf_compound_finish must be called on the interface
 * at some point before the first time it is returned by an interface
 * dispatcher, so that the implied fields can be computed.
 * Some valid assignments for the most important members are,
 *
 * <table class="normal">
 *   <tr><th>assigned</th><th>synthesised</th><th>comment</th></tr>
 *   <tr>
 *     <td>\ref ncomm_iter_source, \ref ncomm_image_junctor</td>
 *     <td>\ref comm_iter_source, \ref comm_image_junctor</td>
 *     <td>A non-commutative compound with an implied commutative view.</td>
 *   </tr>
 *   <tr>
 *     <td>\ref comm_iter_source, \ref comm_image_junctor</td>
 *     <td></td>
 *     <td>A purely commutative compound with limited construction.</td>
 *   </tr>
 *   <tr>
 *     <td>\ref comm_iter_source, \ref comm_build_sinktor</td>
 *     <td>\ref comm_image_junctor (or explicit)</td>
 *     <td>A purely commutative compound with full construction.</td>
 *   </tr>
 *   <tr>
 *     <td>\ref ncomm_iter_source, \ref ncomm_image_junctor<br>
 *         \ref comm_iter_source, \ref comm_image_junctor</td>
 *     <td></td>
 *     <td>A compound with explicit non-commutative and commutative views.</td>
 *   </tr>
 *   <tr>
 *     <td>\ref ncomm_iter_source, \ref ncomm_image_junctor<br>
 *         \ref comm_iter_source, \ref comm_build_sinktor</td>
 *     <td>\ref comm_image_junctor (or explicit)</td>
 *     <td>A compound with explicit non-commutative and commutative views,
 *         where the commutative view has full construction.</td>
 *   </tr>
 * </table>
 *
 * The \ref comm_union_sinktor may optionally be assigned whenever \ref
 * comm_build_sinktor is defined.
 */
struct cuex_intf_compound_s
{
    unsigned int flags;

    /*!Source sequence of elements for the non-commutative view. This can be
     * left unassigned for purely commutative compounds.  It will not be
     * synthesised. */
    cu_ptr_source_t (*ncomm_iter_source)(cuex_intf_compound_t, cuex_t C);

    /*!Shall return a \ref cu_ptr_seq_h "junctor" which allows the construction
     * of an image of \a C.  The elements must have the same order as that
     * returned by \ref ncomm_iter_source.   This may be left unassigned, in
     * which case it will be synthesised if \ref ncomm_build_sinktor is
     * defined.  If assigned, then \ref ncomm_iter_source must also be
     * assigned.  */
    cu_ptr_junctor_t (*ncomm_image_junctor)(cuex_intf_compound_t, cuex_t C);

    /*!Shall return a \ref cu_ptr_seq_h "sinktor" to construct a container from
     * scratch using the non-commutative view. */
    cu_ptr_sinktor_t (*ncomm_build_sinktor)(cuex_intf_compound_t, cuex_t C);

    /*!Source sequence of elements for the commutative view.  If not assigned,
     * it will be synthesised from \ref ncomm_iter_source. */
    cu_ptr_source_t (*comm_iter_source)(cuex_intf_compound_t, cuex_t C);

    /*!Shall return a \ref cu_ptr_seq_h "junctor" which allows the construction
     * of an image of \a C.  The element order must be the same as that of \ref
     * comm_iter_source.  This can be synthesised from \ref comm_iter_source
     * and \ref comm_build_sinktor.  Otherwise, if \ref comm_iter_source is
     * synthetic, then it can also be synthesised from \ref
     * ncomm_image_junctor.  Otherwise, no construction is allowed on this
     * compound.  */
    cu_ptr_junctor_t (*comm_image_junctor)(cuex_intf_compound_t, cuex_t C);
    
    /*!Shall return a \ref cu_ptr_seq_h "sinktor" which allows the construction
     * of a container from scratch, using \a C as a template for any additional
     * properties. */
    cu_ptr_sinktor_t (*comm_build_sinktor)(cuex_intf_compound_t, cuex_t C);

    /*!Shall return a \ref cu_ptr_seq_h "sinktor" which allows the construction
     * of a copy of \a C with additional elements put into the sink. */
    cu_ptr_sinktor_t (*comm_union_sinktor)(cuex_intf_compound_t, cuex_t C);

    /*!Shall return the number of elements. This must correspond to the number
     * of elements provided by \ref ncomm_iter_source and \ref
     * comm_iter_source. If not defined, it will be synthesised from \ref
     * comm_iter_source or \ref ncomm_iter_source. */
    size_t (*size)(cuex_intf_compound_t, cuex_t C);

#if 0
    cuex_t (*comm_find)(cuex_intf_compound_t, cuex_t C, cuex_t key);
#endif
};

/*!Returns the compound implementation of \a type, or \c NULL if not
 * implemented for this type. */
CU_SINLINE cuex_intf_compound_t cuex_type_compound(cuoo_type_t type)
{ return (cuex_intf_compound_t)cuoo_type_impl_ptr(type, CUEX_INTF_COMPOUND); }

/*!Verifies that \a impl has been correctly initialised, and may synthesise
 * some missing fields as indicated in \ref cuex_intf_compound_s. */
void cuex_intf_compound_finish(cuex_intf_compound_t impl);

/*!Returns the number of elements in \a C. */
CU_SINLINE size_t
cuex_compound_size(cuex_intf_compound_t impl, cuex_t C)
{ return impl->size(impl, C); }

/*!Returns a source for iterating through \a C using either non-commutative or
 * commutative view as preferred by the compound. */
cu_ptr_source_t
cuex_compound_pref_iter_source(cuex_intf_compound_t impl, cuex_t C);

/*!Returns a source for iterating through \a C using the commutative view. */
CU_SINLINE cu_ptr_source_t
cuex_compound_comm_iter_source(cuex_intf_compound_t impl, cuex_t C)
{ return impl->comm_iter_source(impl, C); }

/*!Returns a junctor for constructing an image of \a C using either
 * non-commutative or commutative view as preferred by the compound. */
cu_ptr_junctor_t
cuex_compound_pref_image_junctor(cuex_intf_compound_t impl, cuex_t C);

/*!Returns a junctor for constructing an image of \a C using the commutative
 * view. */
CU_SINLINE cu_ptr_junctor_t
cuex_compound_comm_image_junctor(cuex_intf_compound_t impl, cuex_t C)
{ return impl->comm_image_junctor(impl, C); }

#if 0
cu_bool_t cuex_compound_conj(cuex_intf_compound_t impl, cuex_t C,
			     cu_clop(f, cu_bool_t, cuex_t));

cuex_t cuex_compound_image(cuex_intf_compound_t impl, cuex_t C,
			   cu_clop(f, cuex_t, cuex_t));
#endif

/*!Returns an ordered sequence over immediate subexpressions of \a e.  For
 * operations, yields the opreands in order (as returned by \ref cuex_opn_at).
 * For compounds, returns the non-commutative source of the compound, or \c
 * NULL if the compound only has a commutative view.  Returns the empty
 * interator for atomic expressions. */
cu_ptr_source_t cuex_ncomm_source(cuex_t e);

/*!Returns an unordered sequence over immediate subexpressions of \a e.  For
 * operations, a sequence of cuex_o2_metapair are returned, where the first
 * operand is an cudyn_int of the position, and the second is the original
 * operand.  For compounds, the (possibly synthesised) commutative view is
 * returned.  Returns the empty iterator for atomic expressions. */
cu_ptr_source_t cuex_comm_source(cuex_t e);

/*!Returns a sequence over \a e, using the preferred iteration source.  For
 * operations, the operands are returned in order, as with \ref
 * cuex_ncomm_source.  For compounds, \ref cuex_compound_pref_iter_source is
 * used.  Returns the empty iterator for atomic expressions. */
cu_ptr_source_t cuex_pref_source(cuex_t e);

/*!@}*/
CU_END_DECLARATIONS

#endif
