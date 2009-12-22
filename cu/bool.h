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

#ifndef CU_BOOL_H
#define CU_BOOL_H

#include <cu/fwd.h>

CU_BEGIN_DECLARATIONS
/** \defgroup cu_bool_h cu/bool.h: Boolean Functions
 ** @{ \ingroup cu_type_mod */

/** An enumeration of unary boolean functions. */
typedef enum {
    CU_BOOL1F_FALSE	= 0x0,	/**< λx. false */
    CU_BOOL1F_TRUE	= 0x3,	/**< λx. true */
    CU_BOOL1F_IDENT	= 0x2,	/**< λx. x */
    CU_BOOL1F_NOT	= 0x1,	/**< λx. ¬x */
} cu_bool1f_t;

/** Evaluate \a f(\a x). */
CU_SINLINE cu_bool_t
cu_bool1f_apply(cu_bool1f_t f, cu_bool_t x)
{ return (f >> !!x) & 1; }

/** Return the function λ\e x. ¬\a f(\e x). */
CU_SINLINE cu_bool1f_t
cu_bool1f_not(cu_bool1f_t f)
{ return (cu_bool1f_t)(~f & 0x3); }

/** Return the composition \a f ∘ \a g. */
CU_SINLINE cu_bool1f_t
cu_bool1f_compose(cu_bool1f_t f, cu_bool1f_t g)
{ return (cu_bool1f_t)((3*(f & 1) & ~g) | (3*((f & 2) >> 1) & g)); }


/** An enumeration of binary boolean functions. */
typedef enum {

    /* Nullary */
    CU_BOOL2F_FALSE	= 0x0,	/**< λ(\e x, \e y). false */
    CU_BOOL2F_TRUE	= 0xf,	/**< λ(\e x, \e y). true */

    /* Unary */
    CU_BOOL2F_LEFT	= 0xa,	/**< λ(\e x, \e y). \e x */
    CU_BOOL2F_RIGHT	= 0xc,	/**< λ(\e x, \e y). \e y */
    CU_BOOL2F_NOT_LEFT	= 0x5,	/**< λ(\e x, \e y). ¬\e x */
    CU_BOOL2F_NOT_RIGHT	= 0x3,	/**< λ(\e x, \e y). ¬\e y */

    /* Commutative */
    CU_BOOL2F_NOR	= 0x1,	/**< λ(\e x, \e y). ¬(\e x ∨ \e y) */
    CU_BOOL2F_XOR	= 0x6,	/**< λ(\e x, \e y). \e x ⊻ \e y */
    CU_BOOL2F_NAND	= 0x7,	/**< λ(\e x, \e y). ¬(\e x ∧ \e y) */
    CU_BOOL2F_AND	= 0x8,	/**< λ(\e x, \e y). \e x ∧ \e y */
    CU_BOOL2F_IFF	= 0x9,	/**< λ(\e x, \e y). \e x ⇔ \e y */
    CU_BOOL2F_OR	= 0xe,	/**< λ(\e x, \e y). \e x ∨ \e y */

    /* Non-commutative */
    CU_BOOL2F_AND_NOT	= 0x2,	/**< λ(\e x, \e y). \e x ∧ ¬\e y */
    CU_BOOL2F_NOT_AND	= 0x4,	/**< λ(\e x, \e y). ¬\e x ∧ \e y */
    CU_BOOL2F_OR_NOT	= 0xb,	/**< λ(\e x, \e y). \e x ∨ ¬\e y */
    CU_BOOL2F_NOT_OR	= 0xd,	/**< λ(\e x, \e y). ¬\e x ∨ \e y */

} cu_bool2f_t;

/** Evaluate \a f(\a x, \a y). */
CU_SINLINE cu_bool_t
cu_bool2f_apply(cu_bool2f_t f, cu_bool_t x, cu_bool_t y)
{
    return (f >> ((!!x) | (2*!!y))) & 1;
}

/** Return the partial evaluation λ\e y. \a f(\a x, \e y). */
CU_SINLINE cu_bool1f_t
cu_bool2f_apply_left(cu_bool2f_t f, cu_bool_t x)
{
    int i = f >> !!x;
    return (cu_bool1f_t)((i & 1) | ((i >> 1) & 2));
}

/** Return the partial evaluation λ\e x. \a f(\e x, \a y). */
CU_SINLINE cu_bool1f_t
cu_bool2f_apply_right(cu_bool2f_t f, cu_bool_t y)
{
    return (cu_bool1f_t)((f >> (2*!!y)) & 3);
}

/** Return the function λ(\e x, \e y). ¬\a f(\e x, \e y). */
CU_SINLINE cu_bool2f_t
cu_bool2f_not(cu_bool2f_t f)
{
    return (cu_bool2f_t)(~f & 0xf);
}

CU_SINLINE cu_bool2f_t
cu_bool2f_not_left(cu_bool2f_t f)
{
    return (cu_bool2f_t)(((f & 5) << 1) | ((f & 10) >> 1));
}

CU_SINLINE cu_bool2f_t
cu_bool2f_not_right(cu_bool2f_t f)
{
    return (cu_bool2f_t)((f >> 2) | ((f & 3) << 2));
}

/** Swap the arguments, i.e. λ(\e x, \e y). \a f(\e y, \e x). */
CU_SINLINE cu_bool2f_t
cu_bool2f_swap(cu_bool2f_t f)
{
    return (cu_bool2f_t)((f & 9) | ((f & 2) << 1) | ((f & 4) >> 1));
}

/** @} */
CU_END_DECLARATIONS

#endif
