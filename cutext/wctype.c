/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2008  Petter Urkedal <urkedal@nbi.dk>
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

#include <cutext/wctype.h>
#include <cutext/wccat.h>

typedef uint_least64_t cutext_wctype_t;

#define cutext_wctype_singleton(cat) (UINT64_C(1) << (cat))

#define CUTEXT_WCTYPE_UPPER	( cutext_wctype_singleton(CUTEXT_WCCAT_LU) \
				| cutext_wctype_singleton(CUTEXT_WCCAT_LT) )
#define CUTEXT_WCTYPE_LOWER	( cutext_wctype_singleton(CUTEXT_WCCAT_LL) )
#define CUTEXT_WCTYPE_ALPHA	( CUTEXT_WCTYPE_UPPER | CUTEXT_WCTYPE_LOWER \
				| cutext_wctype_singleton(CUTEXT_WCCAT_LM) \
				| cutext_wctype_singleton(CUTEXT_WCCAT_LO) )
#define CUTEXT_WCTYPE_NUMBER	( cutext_wctype_singleton(CUTEXT_WCCAT_ND) \
				| cutext_wctype_singleton(CUTEXT_WCCAT_NL) \
				| cutext_wctype_singleton(CUTEXT_WCCAT_NO) )
#define CUTEXT_WCTYPE_MARK	( cutext_wctype_singleton(CUTEXT_WCCAT_MN) \
				| cutext_wctype_singleton(CUTEXT_WCCAT_MC) \
				| cutext_wctype_singleton(CUTEXT_WCCAT_ME) )
#define CUTEXT_WCTYPE_ALNUM	( CUTEXT_WCTYPE_ALPHA | CUTEXT_WCTYPE_NUMBER )
#define CUTEXT_WCTYPE_BLANK	( cutext_wctype_singleton(CUTEXT_WCCAT_ZS) )
#define CUTEXT_WCTYPE_SPACE	( CUTEXT_WCTYPE_BLANK \
				| cutext_wctype_singleton(CUTEXT_WCCAT_ZL) \
				| cutext_wctype_singleton(CUTEXT_WCCAT_ZP) )
#define CUTEXT_WCTYPE_PUNCT	( cutext_wctype_singleton(CUTEXT_WCCAT_PC) \
				| cutext_wctype_singleton(CUTEXT_WCCAT_PD) \
				| cutext_wctype_singleton(CUTEXT_WCCAT_PS) \
				| cutext_wctype_singleton(CUTEXT_WCCAT_PE) \
				| cutext_wctype_singleton(CUTEXT_WCCAT_PI) \
				| cutext_wctype_singleton(CUTEXT_WCCAT_PF) \
				| cutext_wctype_singleton(CUTEXT_WCCAT_PO) \
				| cutext_wctype_singleton(CUTEXT_WCCAT_SM) \
				| cutext_wctype_singleton(CUTEXT_WCCAT_SC) \
				| cutext_wctype_singleton(CUTEXT_WCCAT_SK) \
				| cutext_wctype_singleton(CUTEXT_WCCAT_SO) )

#define CUTEXT_WCTYPE_GRAPH	( CUTEXT_WCTYPE_ALNUM | CUTEXT_WCTYPE_PUNCT )
#define CUTEXT_WCTYPE_PRINT	( CUTEXT_WCTYPE_GRAPH | CUTEXT_WCTYPE_SPACE )
#define CUTEXT_WCTYPE_CNTRL	( cutext_wctype_singleton(CUTEXT_WCCAT_CC) )

/* NOTE. The "mark" and some of the "other" categories are not included in
 * either cntrl or print. */

cu_bool_t
cutext_iswxdigit(cu_wint_t wc)
{
    return cutext_iswdigit(wc)
	|| (0x41 <= wc && wc <= 0x46)
	|| (0x61 <= wc && wc <= 0x66);
}

cu_bool_t
cutext_iswblank(cu_wint_t wc)
{
    cutext_wccat_t cat = cutext_wchar_wccat(wc);
    return !!(cutext_wctype_singleton(cat) & CUTEXT_WCTYPE_BLANK)
	|| 0x9 == wc;
}

cu_bool_t
cutext_iswspace(cu_wint_t wc)
{
    cutext_wccat_t cat = cutext_wchar_wccat(wc);
    return !!(cutext_wctype_singleton(cat) & CUTEXT_WCTYPE_SPACE)
	|| (0x9 <= wc && wc <= 0xd);
}

#define DEF_ISX(fn, wctype) \
    cu_bool_t \
    fn(cu_wint_t wc) \
    { \
	cutext_wccat_t cat = cutext_wchar_wccat(wc); \
	return !!(cutext_wctype_singleton(cat) & wctype); \
    }

DEF_ISX(cutext_iswalnum, CUTEXT_WCTYPE_ALNUM)
DEF_ISX(cutext_iswalpha, CUTEXT_WCTYPE_ALPHA)
DEF_ISX(cutext_iswcntrl, CUTEXT_WCTYPE_CNTRL)
DEF_ISX(cutext_iswgraph, CUTEXT_WCTYPE_GRAPH)
DEF_ISX(cutext_iswlower, CUTEXT_WCTYPE_LOWER)
DEF_ISX(cutext_iswprint, CUTEXT_WCTYPE_PRINT)
DEF_ISX(cutext_iswpunct, CUTEXT_WCTYPE_PUNCT)
DEF_ISX(cutext_iswupper, CUTEXT_WCTYPE_UPPER)
