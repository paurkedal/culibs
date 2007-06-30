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

#include <cutext/ucs4ctype.h>

#define BLOCK_SIZE 512

struct cutext_ucs4ctype_tbl_entry_s
{
    char cat;
    char *sub;
};
extern struct cutext_ucs4ctype_tbl_entry_s cutext_ucs4ctype_tbl[];

cutext_ucs4ctype_t
cutext_ucs4ctype(cutext_ucs4char_t ch)
{
    int block = ch / BLOCK_SIZE;
    struct cutext_ucs4ctype_tbl_entry_s *ent = &cutext_ucs4ctype_tbl[block];
    if (ent->sub)
	return ent->sub[ch % BLOCK_SIZE];
    else
	return ent->cat;
}
