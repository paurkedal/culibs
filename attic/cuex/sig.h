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

#ifndef CUEX_SIG_H
#define CUEX_SIG_H

#include <cuex/fwd.h>
#include <cuex/aci.h>
#include <cuex/oprdefs.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuex_sig_h cuex/sig.h: Signature Expressions
 * @{\ingroup cuex_mod
 * Signatures represents types of structures, which are objects
 * containing other objects and in particular functions, all indexed
 * by labels.  To make them convenient to manipulate, they are
 * represented as a \ref cuex_aci_h "semilattice", where the
 * generators \f$\{(l\mapsto t_l)\}_l\f$ are identified by a
 * label and have an associated type.
 * Thus, a signature has the form
 * \f[
 *   (l_1 \mapsto t_{l_1}) \times \cdots \times (l_N \mapsto t_{l_N})
 * \f]
 *
 * where × is associative, commutative, and idempotent.  The underlying
 * representation used to enforce the axioms is a map from labels to types,
 * so for consistency, the client must make sure that identical labels
 * have the same associated type.  Signature expressions can be
 * converted to a type, at which point offsets into the corresponding
 * structure are calculated. */

extern cuex_opn_t cuexP_sig_identity;

/*!The identity signature.  Contains exactly one empty stucture. */
CU_SINLINE cuex_opn_t cuex_sig_identity(void) { return cuexP_sig_identity; }

/*!True iff \a sig is the identity signature. */
CU_SINLINE cu_bool_t cuex_is_sig_identity(cuex_opn_t sig)
{ return sig == cuexP_sig_identity; }

/*!True iff \a sig is a signature. */
CU_SINLINE cu_bool_t cuex_is_sig(cuex_opn_t sig)
{
    return cuex_is_aci(CUEX_O4ACI_SIGPROD, sig)
	|| cuex_meta(sig) == CUEX_O2_SIGBASE;
}

/*!A generator of signatures, identified by \a label and associated with
 * \a type.  This represents a structure field indexed by \a label which
 * contains an object of \a type.  Products involving generators with
 * the same label but with different associated types is undefined. */
CU_SINLINE cuex_opn_t cuex_sig_generator(cu_idr_t label, cuex_t type)
{ return cuex_aci_generator(CUEX_O4ACI_SIGPROD, label, type); }

/*!The signature product of \a lhs and \a rhs.  This represents the
 * set of structures which a both members of \a lhs and \a rhs.
 * \pre Any duplicate label between the generators of \a lhs and \a rhs
 * must map to the same type. */
CU_SINLINE cuex_opn_t
cuex_sigprod(cuex_opn_t lhs, cuex_opn_t rhs)
{
    cu_debug_assert(cuex_is_aci(CUEX_O4ACI_SIGPROD, lhs));
    cu_debug_assert(cuex_is_aci(CUEX_O4ACI_SIGPROD, rhs));
    return cuex_aci_join(CUEX_O4ACI_SIGPROD, lhs, rhs);
}

/*!Same as <tt>cuex_sigprod(\a sig, cuex_generator(\a label, \a type))</tt>
 * but faster. */
CU_SINLINE cuex_opn_t
cuex_sigprod_generator(cuex_opn_t sig, cu_idr_t label, cuex_t type)
{
    return cuex_aci_insert(CUEX_O4ACI_SIGPROD, sig, label, type);
}

/*!Returns the type associated with \a label in \a sig, or \c NULL if none. */
cuex_t cuex_sig_find_type(cuex_opn_t sig, cu_idr_t label);

cu_bool_t cuex_sig_find(cuex_opn_t sig, cu_idr_t label,
			cuex_t *type_out, cu_offset_t *bitoffset_out);

cu_bool_t cuex_sig_conj(cuex_opn_t sig,
			cu_clop(cb, cu_bool_t, cu_idr_t, cuex_t, cu_offset_t));

cu_bool_t
cuex_sig_conj_byoffset(cuex_t sig,
		       cu_clop(cb, cu_bool_t, cu_idr_t, cuex_t, cu_offset_t));

/*!Size of \a sig in bits. */
cu_offset_t cuex_sig_bitsize(cuex_opn_t sig);

/*!Size of \a sig in bytes, rounded upwards from the bit size. */
CU_SINLINE cu_offset_t
cuex_sig_size(cuex_opn_t sig)
{
    cu_offset_t s = cuex_sig_bitsize(sig);
    return (s + 7)/8;
}

/*!Alignment of \a sig in bits. */
cu_offset_t cuex_sig_bitalign(cuex_opn_t sig);

/*!Alignment of \a sig in bytes. */
CU_SINLINE cu_offset_t
cuex_sig_align(cuex_opn_t sig)
{
    cu_offset_t a = cuex_sig_bitalign(sig);
    return (a + 7)/8;
}

/*!For convenience, make a signature from \a n alternating
 * <tt>char const *</tt> and <tt>cuex_t</tt> arguments, each pair
 * representing the label and type of a component of the signature. */
cuex_opn_t cuex_sig_v(int n, ...);

/*!Same as \ref cuex_sig_v, except that a prepared argument list \a va
 * is passed. */
cuex_opn_t cuex_sig_va(int n, va_list va);

/*!Dump \a sig to \a out. Mostly for debugging. */
void cuex_sig_dump(cuex_t sig, FILE *out);

/*!@}*/
CU_END_DECLARATIONS

#endif
