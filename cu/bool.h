/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2009--2010  Petter Urkedal <paurkedal@eideticdew.org>
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

/** Returns -1 if ¬\a x ∧ \a y, 0 if \a x ⇔ \a y, and 1 if \a x ∧ ¬\a y. */
CU_SINLINE int
cu_bool_cmp(cu_bool_t x, cu_bool_t y)
{ return x? (y? 0 : 1) : (y? -1 : 0); }

/** Return the enumerated boolean function corresponding to \a f.  \a f will be
 ** called twice upon construction. */
cu_bool1f_t cu_bool1f_from_func(cu_bool_t (*f)(cu_bool_t));

/** Return the C function which at \e x returns <code>cu_bool1f_apply(\a f, \e
 ** x)</code>. */
cu_bool_t (*cu_bool1f_to_func(cu_bool1f_t f))(cu_bool_t);

/** Evaluate \a f(\a x). */
CU_SINLINE cu_bool_t
cu_bool1f_apply(cu_bool1f_t f, cu_bool_t x)
{ return (f >> !!x) & 1; }

/** Return the constant function, λ\e y. \a x. */
CU_SINLINE cu_bool1f_t
cu_bool1f_konst(cu_bool_t x)
{ return (cu_bool1f_t)(3*!!x); }

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

/** Return the enumerated boolean function which agrees with \a f on its
 ** domain.  This samples \a f 4 times. */
cu_bool2f_t cu_bool2f_from_func(cu_bool_t (*f)(cu_bool_t, cu_bool_t));

/** The C function which agrees with \a f on its domain. */
cu_bool_t (*cu_bool2f_to_func(cu_bool2f_t f))(cu_bool_t, cu_bool_t);

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

/** The constant function λ(\e x, \e y). \a v. */
CU_SINLINE cu_bool2f_t
cu_bool2f_konst(cu_bool_t v)
{
    return (cu_bool2f_t)(v? 15 : 0);
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

/** Return the composition \a f ∘ \a g, i.e. the function λ(\e x, \e y). \a f
 ** (\a g (\e x, \e y)). */
CU_SINLINE cu_bool2f_t
cu_bool2f_compose(cu_bool1f_t f, cu_bool2f_t g)
{
    return (cu_bool2f_t)((15*(f & 1) & ~g) | (15*((f & 2) >> 1) & g));
}

/** Return the composition of \a f on the left operand with \a g, λ(\e x, \e
 ** y). \a f (\a g(\e x), \e y). */
CU_SINLINE cu_bool2f_t
cu_bool2f_compose_left(cu_bool2f_t f, cu_bool1f_t g)
{
    int gg = g | (g << 2);
    int fL = f & 5;
    int fH = (f >> 1) & 5;
    return (cu_bool2f_t)((3*fL & ~gg) | (3*fH & gg));
}

/** Return the composition of \a f on the right operand with \a g, λ(\e x, \e
 ** y). \a f (\e x, \a g(\e y)). */
CU_SINLINE cu_bool2f_t
cu_bool2f_compose_right(cu_bool2f_t f, cu_bool1f_t g)
{
    int gg = 3*(g + (g & 2));
    int fL = f & 3;
    int fH = f >> 2;
    return (cu_bool2f_t)((5*fL & ~gg) | (5*fH & gg));
}

/** @} */
CU_END_DECLARATIONS

#endif
