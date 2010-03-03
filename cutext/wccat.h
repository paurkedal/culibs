/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2004--2010  Petter Urkedal <paurkedal@eideticdew.org>
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

#ifndef CUTEXT_WCCAT_H
#define CUTEXT_WCCAT_H

#include <cu/wchar.h>

CU_BEGIN_DECLARATIONS

/** \defgroup cutext_wccat_h cutext/wccat.h: Unicode General Categories
 ** @{\ingroup cutext_mod */

/** Enumeration of the Unicode 5.1.0 character categories. */
typedef enum {
    CUTEXT_WCCAT_NONE,		/*!< not a category */
    CUTEXT_WCCAT_LU = 8,	/*!< letter, uppercase */
    CUTEXT_WCCAT_LL,		/*!< letter, lowercase */
    CUTEXT_WCCAT_LT,		/*!< letter, titlecase */
    CUTEXT_WCCAT_LM,		/*!< letter, modifier */
    CUTEXT_WCCAT_LO,		/*!< letter, other */
    CUTEXT_WCCAT_MN = 16,	/*!< mark, nonspacing */
    CUTEXT_WCCAT_MC,		/*!< mark, spacing combining */
    CUTEXT_WCCAT_ME,		/*!< mark, enclosing */
    CUTEXT_WCCAT_ND = 24,	/*!< number, decimal digit */
    CUTEXT_WCCAT_NL,		/*!< number, letter */
    CUTEXT_WCCAT_NO,		/*!< number, other */
    CUTEXT_WCCAT_PC = 32,	/*!< punctuation, connector */
    CUTEXT_WCCAT_PD,		/*!< punctuation, dash */
    CUTEXT_WCCAT_PS,		/*!< punctuation, open */
    CUTEXT_WCCAT_PE,		/*!< punctuation, close */
    CUTEXT_WCCAT_PI,		/*!< punctuation, initial quote */
    CUTEXT_WCCAT_PF,		/*!< punctuation, final quote */
    CUTEXT_WCCAT_PO,		/*!< punctuation, other */
    CUTEXT_WCCAT_SM = 40,	/*!< symbol, math */
    CUTEXT_WCCAT_SC,		/*!< symbol, currency */
    CUTEXT_WCCAT_SK,		/*!< symbol, modifier */
    CUTEXT_WCCAT_SO,		/*!< symbol, other */
    CUTEXT_WCCAT_ZS = 48,	/*!< separator, space */
    CUTEXT_WCCAT_ZL,		/*!< separator, line */
    CUTEXT_WCCAT_ZP,		/*!< separator, paragraph */
    CUTEXT_WCCAT_CC = 56,	/*!< other, control */
    CUTEXT_WCCAT_CF,		/*!< other, format */
    CUTEXT_WCCAT_CS,		/*!< other, surrogate */
    CUTEXT_WCCAT_CO,		/*!< other, private use */
    CUTEXT_WCCAT_CN,		/*!< other, not assigned */
} cutext_wccat_t;

/** Returns the general category denoted by its two-letter name \a name, or
 ** \ref CUTEXT_WCCAT_NONE if \a name is invalid. */
cutext_wccat_t cutext_wccat_by_name(char *name);

/** Returns the general category of \a ch. */
cutext_wccat_t cutext_wchar_wccat(cu_wint_t ch);

/** True iff \a ct is in the "letter" ("L*") main category. */
CU_SINLINE cu_bool_t
cutext_wccat_is_letter(cutext_wccat_t ct) { return (ct >> 3) == 1; }

/** True iff \a ct is in the "mark" ("M*") main category. */
CU_SINLINE cu_bool_t
cutext_wccat_is_mark(cutext_wccat_t ct) { return (ct >> 3) == 2; }

/** True iff \a ct is in the "number" ("N*") main category. */
CU_SINLINE cu_bool_t
cutext_wccat_is_number(cutext_wccat_t ct) { return (ct >> 3) == 3; }

/** True iff \a ct is in the "punctuation" ("P*") main category. */
CU_SINLINE cu_bool_t
cutext_wccat_is_punctuation(cutext_wccat_t ct) { return (ct >> 3) == 4; }

/** True iff \a ct is in the "symbol" ("S*") main category. */
CU_SINLINE cu_bool_t
cutext_wccat_is_symbol(cutext_wccat_t ct) { return (ct >> 3) == 5; }

/** True iff \a ct is in the "separator" ("Z*") main category. */
CU_SINLINE cu_bool_t
cutext_wccat_is_separator(cutext_wccat_t ct) { return (ct >> 3) == 6; }

/** True iff \a ct is in the "other" ("C*") main category. */
CU_SINLINE cu_bool_t
cutext_wccat_is_other(cutext_wccat_t ct) { return (ct >> 3) == 7; }

CU_SINLINE cu_bool_t
cutext_wchar_is_letter(cu_wchar_t ch)
{
    return cutext_wccat_is_letter(cutext_wchar_wccat(ch));
}
CU_SINLINE cu_bool_t
cutext_wchar_is_mark(cu_wchar_t ch)
{
    return cutext_wccat_is_mark(cutext_wchar_wccat(ch));
}
CU_SINLINE cu_bool_t
cutext_wchar_is_number(cu_wchar_t ch)
{
    return cutext_wccat_is_number(cutext_wchar_wccat(ch));
}
CU_SINLINE cu_bool_t
cutext_wchar_is_punctuation(cu_wchar_t ch)
{
    return cutext_wccat_is_punctuation(cutext_wchar_wccat(ch));
}
CU_SINLINE cu_bool_t
cutext_wchar_is_symbol(cu_wchar_t ch)
{
    return cutext_wccat_is_symbol(cutext_wchar_wccat(ch));
}
CU_SINLINE cu_bool_t
cutext_wchar_is_separator(cu_wchar_t ch)
{
    return cutext_wccat_is_separator(cutext_wchar_wccat(ch));
}
CU_SINLINE cu_bool_t
cutext_wchar_is_other(cu_wchar_t ch)
{
    return cutext_wccat_is_other(cutext_wchar_wccat(ch));
}

CU_SINLINE cu_bool_t
cutext_wchar_is_vertical_space(cu_wchar_t ch)
{
    return ch == '\n' || ch == '\v' || ch == '\f';
}
CU_SINLINE cu_bool_t
cutext_wchar_is_space(cu_wchar_t ch)
{
    return cutext_wchar_is_separator(ch) || cutext_wchar_is_vertical_space(ch)
	|| ch == '\t';
}
CU_SINLINE cu_bool_t
cutext_wchar_is_print(cu_wchar_t ch)
{
    return !cutext_wchar_is_other(ch)
	&& !cutext_wchar_is_vertical_space(ch);
}

/** @} */

CU_END_DECLARATIONS

#endif
