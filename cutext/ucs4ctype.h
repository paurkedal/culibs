/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2004--2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUTEXT_UCS4CTYPE_H
#define CUTEXT_UCS4CTYPE_H

#include <cu/fwd.h>
#include <cutext/ucs4.h>

CU_BEGIN_DECLARATIONS

typedef enum {
    cutext_ucs4ctype_none,
    /* letter */
    cutext_ucs4ctype_lu = 8, /* uppercase */
    cutext_ucs4ctype_ll, /* lowercase */
    cutext_ucs4ctype_lt, /* titlecase */
    cutext_ucs4ctype_lm, /* modifier */
    cutext_ucs4ctype_lo, /* other */
    /* mark */
    cutext_ucs4ctype_mn = 16, /* nonspacing */
    cutext_ucs4ctype_mc, /* spacing combining */
    cutext_ucs4ctype_me, /* enclosing */
    /* number */
    cutext_ucs4ctype_nd = 24, /* decimal digit */
    cutext_ucs4ctype_nl, /* letter */
    cutext_ucs4ctype_no, /* other */
    /* punctuation */
    cutext_ucs4ctype_pc = 32, /* connector */
    cutext_ucs4ctype_pd, /* dash */
    cutext_ucs4ctype_ps, /* open */
    cutext_ucs4ctype_pe, /* close */
    cutext_ucs4ctype_pi, /* initial quote */
    cutext_ucs4ctype_pf, /* final quote */
    cutext_ucs4ctype_po, /* other */
    /* symbol */
    cutext_ucs4ctype_sm = 40, /* math */
    cutext_ucs4ctype_sc, /* currency */
    cutext_ucs4ctype_sk, /* modifier */
    cutext_ucs4ctype_so, /* other */
    /* separator */
    cutext_ucs4ctype_zs = 48, /* space */
    cutext_ucs4ctype_zl, /* line */
    cutext_ucs4ctype_zp, /* paragraph */
    /* other */
    cutext_ucs4ctype_cc = 56, /* control */
    cutext_ucs4ctype_cf, /* format */
    cutext_ucs4ctype_cs, /* surrogate */
    cutext_ucs4ctype_co, /* private use */
    cutext_ucs4ctype_cn, /* not assigned */
} cutext_ucs4ctype_t;

cutext_ucs4ctype_t cutext_ucs4ctype(cutext_ucs4char_t ch);

CU_SINLINE cu_bool_t
cutext_ucs4ctype_is_letter(cutext_ucs4ctype_t ct) { return (ct >> 3) == 1; }
CU_SINLINE cu_bool_t
cutext_ucs4ctype_is_mark(cutext_ucs4ctype_t ct) { return (ct >> 3) == 2; }
CU_SINLINE cu_bool_t
cutext_ucs4ctype_is_number(cutext_ucs4ctype_t ct) { return (ct >> 3) == 3; }
CU_SINLINE cu_bool_t
cutext_ucs4ctype_is_punctuation(cutext_ucs4ctype_t ct) { return (ct >> 3) == 4; }
CU_SINLINE cu_bool_t
cutext_ucs4ctype_is_symbol(cutext_ucs4ctype_t ct) { return (ct >> 3) == 5; }
CU_SINLINE cu_bool_t
cutext_ucs4ctype_is_separator(cutext_ucs4ctype_t ct) { return (ct >> 3) == 6; }
CU_SINLINE cu_bool_t
cutext_ucs4ctype_is_otherucscat(cutext_ucs4ctype_t ct) { return (ct >> 3) == 7; }

CU_SINLINE cu_bool_t
cutext_ucs4char_is_letter(cutext_ucs4char_t ch)
{
    return cutext_ucs4ctype_is_letter(cutext_ucs4ctype(ch));
}
CU_SINLINE cu_bool_t
cutext_ucs4char_is_mark(cutext_ucs4char_t ch)
{
    return cutext_ucs4ctype_is_mark(cutext_ucs4ctype(ch));
}
CU_SINLINE cu_bool_t
cutext_ucs4char_is_number(cutext_ucs4char_t ch)
{
    return cutext_ucs4ctype_is_number(cutext_ucs4ctype(ch));
}
CU_SINLINE cu_bool_t
cutext_ucs4char_is_punctuation(cutext_ucs4char_t ch)
{
    return cutext_ucs4ctype_is_punctuation(cutext_ucs4ctype(ch));
}
CU_SINLINE cu_bool_t
cutext_ucs4char_is_symbol(cutext_ucs4char_t ch)
{
    return cutext_ucs4ctype_is_symbol(cutext_ucs4ctype(ch));
}
CU_SINLINE cu_bool_t
cutext_ucs4char_is_separator(cutext_ucs4char_t ch)
{
    return cutext_ucs4ctype_is_separator(cutext_ucs4ctype(ch));
}
CU_SINLINE cu_bool_t
cutext_ucs4char_is_otherucscat(cutext_ucs4char_t ch)
{
    return cutext_ucs4ctype_is_otherucscat(cutext_ucs4ctype(ch));
}

CU_SINLINE cu_bool_t
cutext_ucs4char_is_vertical_space(cutext_ucs4char_t ch)
{
    return ch == '\n' || ch == '\v' || ch == '\f';
}
CU_SINLINE cu_bool_t
cutext_ucs4char_is_space(cutext_ucs4char_t ch)
{
    return cutext_ucs4char_is_separator(ch) || cutext_ucs4char_is_vertical_space(ch)
	|| ch == '\t';
}
CU_SINLINE cu_bool_t
cutext_ucs4char_is_printable(cutext_ucs4char_t ch)
{
    return !cutext_ucs4char_is_otherucscat(ch)
	&& !cutext_ucs4char_is_vertical_space(ch);
}

CU_END_DECLARATIONS

#endif
