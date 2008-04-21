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

#include <cutext/wccat.h>
#include <cu/diag.h>
#include <cu/conf.h>
#include <string.h>

#define UNICODE_MAX 0x1fffff
#define BLOCK_SIZE 512
#define BLOCK_COUNT ((UNICODE_MAX + 1)/BLOCK_SIZE)

#if !defined(CUTEXT_UNIPREP_C) && !defined(CUCONF_ENABLE_WCCAT_SWITCH)

struct cutextP_wctype_table_entry_s
{
    char cat;
    char *sub;
};
extern struct cutextP_wctype_table_entry_s cutextP_wctype_table[];

cutext_wccat_t
cutext_wchar_wccat(cu_wint_t ch)
{
    int block = ch / BLOCK_SIZE;
    struct cutextP_wctype_table_entry_s *ent;
    if (ch > UNICODE_MAX)
	cu_bugf("Unicode character number %d is out of range.", (int)ch);
    ent = &cutextP_wctype_table[block];
    if (ent->sub)
	return ent->sub[ch % BLOCK_SIZE];
    else
	return ent->cat;
}

#endif /* !CUTEXT_UNIPREP_C && !CUCONF_ENABLE_WCCAT_SWITCH */

cutext_wccat_t
cutext_wccat_by_name(char *s)
{
    if (strlen(s) != 2)
	return CUTEXT_WCCAT_NONE;
    switch (s[0]) {
	case 'L': case 'l':
	    switch (s[1]) {
		case 'U': case 'u': return CUTEXT_WCCAT_LU;
		case 'L': case 'l': return CUTEXT_WCCAT_LL;
		case 'T': case 't': return CUTEXT_WCCAT_LT;
		case 'M': case 'm': return CUTEXT_WCCAT_LM;
		case 'O': case 'o': return CUTEXT_WCCAT_LO;
	    }
	    break;
	case 'M': case 'm':
	    switch (s[1]) {
		case 'N': case 'n': return CUTEXT_WCCAT_MN;
		case 'C': case 'c': return CUTEXT_WCCAT_MC;
		case 'E': case 'e': return CUTEXT_WCCAT_ME;
	    }
	    break;
	case 'N': case 'n':
	    switch (s[1]) {
		case 'D': case 'd': return CUTEXT_WCCAT_ND;
		case 'L': case 'l': return CUTEXT_WCCAT_NL;
		case 'O': case 'o': return CUTEXT_WCCAT_NO;
	    }
	    break;
	case 'P': case 'p':
	    switch (s[1]) {
		case 'C': case 'c': return CUTEXT_WCCAT_PC;
		case 'D': case 'd': return CUTEXT_WCCAT_PD;
		case 'S': case 's': return CUTEXT_WCCAT_PS;
		case 'E': case 'e': return CUTEXT_WCCAT_PE;
		case 'I': case 'i': return CUTEXT_WCCAT_PI;
		case 'F': case 'f': return CUTEXT_WCCAT_PF;
		case 'O': case 'o': return CUTEXT_WCCAT_PO;
	    }
	    break;
	case 'S': case 's':
	    switch (s[1]) {
		case 'M': case 'm': return CUTEXT_WCCAT_SM;
		case 'C': case 'c': return CUTEXT_WCCAT_SC;
		case 'K': case 'k': return CUTEXT_WCCAT_SK;
		case 'O': case 'o': return CUTEXT_WCCAT_SO;
	    }
	    break;
	case 'Z': case 'z':
	    switch (s[1]) {
		case 'S': case 's': return CUTEXT_WCCAT_ZS;
		case 'L': case 'l': return CUTEXT_WCCAT_ZL;
		case 'P': case 'p': return CUTEXT_WCCAT_ZP;
	    }
	    break;
	case 'C': case 'c':
	    switch (s[1]) {
		case 'C': case 'c': return CUTEXT_WCCAT_CC;
		case 'F': case 'f': return CUTEXT_WCCAT_CF;
		case 'S': case 's': return CUTEXT_WCCAT_CS;
		case 'O': case 'o': return CUTEXT_WCCAT_CO;
		case 'N': case 'n': return CUTEXT_WCCAT_CN;
	    }
	    break;
    }
    return CUTEXT_WCCAT_NONE;
}
