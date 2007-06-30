/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2006--2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUEX_QCODE_H
#define CUEX_QCODE_H

#include <cuex/fwd.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuex_qcode_h cuex/qcode.h: Quantisanion Specifiers
 * @{\ingroup cuex_mod */

/*!An enumeration of variable quantification. */
typedef enum {
    cuex_qcode_u = 0,	/*!< universal quantification */
    cuex_qcode_e = 1,	/*!< existential quantification */
    cuex_qcode_w = 2,	/*!< weak universal quantification for special use */
    cuex_qcode_n = 3	/*!< unquantified or special use */
} cuex_qcode_t;

/*!A set of quantifier codes. */
typedef enum {
    /*! the empty set */
    cuex_qcset_bot = 0,
    /*! <tt>{cuex_qcode_u}</tt> */
    cuex_qcset_u = (1 << cuex_qcode_u),
    /*! <tt>{cuex_qcode_e}</tt> */
    cuex_qcset_e = (1 << cuex_qcode_e),
    /*! <tt>{cuex_qcode_w}</tt> */
    cuex_qcset_w = (1 << cuex_qcode_w),
    /*! <tt>{cuex_qcode_n}</tt> */
    cuex_qcset_n = (1 << cuex_qcode_n),
    /*! <tt>{cuex_qcode_u, cuex_qcode_e}</tt> */
    cuex_qcset_ue = cuex_qcset_e | cuex_qcset_u,
    /*! <tt>{cuex_qcode_u, cuex_qcode_w}</tt> */
    cuex_qcset_uw = cuex_qcset_u | cuex_qcset_w,
    /*! <tt>{cuex_qcode_u, cuex_qcode_n}</tt> */
    cuex_qcset_un = cuex_qcset_u | cuex_qcset_n,
    /*! <tt>{cuex_qcode_e, cuex_qcode_w}</tt> */
    cuex_qcset_ew = cuex_qcset_e | cuex_qcset_w,
    /*! <tt>{cuex_qcode_e, cuex_qcode_n}</tt> */
    cuex_qcset_en = cuex_qcset_e | cuex_qcset_n,
    /*! <tt>{cuex_qcode_w, cuex_qcode_n}</tt> */
    cuex_qcset_wn = cuex_qcset_w | cuex_qcset_n,
    /*! <tt>{cuex_qcode_u, cuex_qcode_e, cuex_qcode_w}</tt> */
    cuex_qcset_uew = cuex_qcset_u | cuex_qcset_ew,
    /*! <tt>{cuex_qcode_u, cuex_qcode_e, cuex_qcode_n}</tt> */
    cuex_qcset_uen = cuex_qcset_u | cuex_qcset_en,
    /*! <tt>{cuex_qcode_u, cuex_qcode_w, cuex_qcode_n}</tt> */
    cuex_qcset_uwn = cuex_qcset_u | cuex_qcset_wn,
    /*! <tt>{cuex_qcode_e, cuex_qcode_w, cuex_qcode_n}</tt> */
    cuex_qcset_ewn = cuex_qcset_e | cuex_qcset_wn,
    /*! the universal set */
    cuex_qcset_uewn = cuex_qcset_ue | cuex_qcset_wn,
    /*! the universal set */
    cuex_qcset_top = cuex_qcset_uewn
} cuex_qcset_t;

/*!Returns the union \a S0 ∪ \a S1. */
CU_SINLINE cuex_qcset_t cuex_qcset_union(cuex_qcset_t S0, cuex_qcset_t S1)
{ return S0 | S1; }

/*!Returns the intersection \a S0 ∩ \a S1. */
CU_SINLINE cuex_qcset_t cuex_qcset_isecn(cuex_qcset_t S0, cuex_qcset_t S1)
{ return S0 & S1; }

/*!Returns the singular set {qc}. */
CU_SINLINE cuex_qcset_t cuex_qcset_singleton(cuex_qcode_t qc)
{ return 1 << qc; }

/*!True iff qc ∈ S. */
CU_SINLINE cu_bool_t cuex_qcset_contains(cuex_qcset_t S, cuex_qcode_t qc)
{ return (1 << qc) & S; }

/*!Returns cuex_qcode_top ∖ S. */
CU_SINLINE cuex_qcset_t cuex_qcset_complement(cuex_qcset_t S)
{ return ~S; }

/*!@}*/
CU_END_DECLARATIONS

#endif
