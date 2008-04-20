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

#define UNICODE_MAX 0x1fffff
#define BLOCK_SIZE 512
#define BLOCK_COUNT ((UNICODE_MAX + 1)/BLOCK_SIZE)

struct cutextP_wctype_table_entry_s
{
    char cat;
    char *sub;
};
extern struct cutextP_wctype_table_entry_s cutextP_wctype_table[];

cutext_wccat_t
cutext_wchar_wccat(cu_wchar_t ch)
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
