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

#include <cu/word.h>
#include <cu/wordarr.h>

cu_word_t
cu_word_bitimg2(cu_bool2f_t f, cu_word_t x, cu_word_t y)
{
    cu_bool_t not_r;
    cu_word_t r;

    if (f >= 8) {
	not_r = cu_true;
	f = (cu_bool2f_t)(15 - f);
    }
    else
	not_r = cu_false;

    switch (f) {
	case CU_BOOL2F_FALSE:	r = CU_WORD_C(0); break;
	case CU_BOOL2F_NOR:	r = ~x & ~y;	break;
	case CU_BOOL2F_AND_NOT:	r = x & ~y;	break;
	case CU_BOOL2F_NOT_RIGHT: r = ~y;	break;
	case CU_BOOL2F_NOT_AND:	r = ~x & y;	break;
	case CU_BOOL2F_NOT_LEFT:r = ~x;		break;
	case CU_BOOL2F_XOR:	r = x ^ y;	break;
	case CU_BOOL2F_NAND:	r = ~(x & y);	break;
	default:		cu_debug_unreachable();
    }
    return not_r? ~r : r;
}
