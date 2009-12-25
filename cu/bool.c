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

#include <cu/bool.h>
#include <cu/debug.h>

static cu_bool_t _b1f_false(cu_bool_t x) { return cu_false; }
static cu_bool_t _b1f_ident(cu_bool_t x) { return x; }
static cu_bool_t _b1f_not(cu_bool_t x) { return !x; }
static cu_bool_t _b1f_true(cu_bool_t x) { return cu_true; }

cu_bool_t (*cu_bool1f_to_func(cu_bool1f_t f))(cu_bool_t)
{
    switch (f) {
	case CU_BOOL1F_FALSE: return _b1f_false;
	case CU_BOOL1F_IDENT: return _b1f_ident;
	case CU_BOOL1F_NOT:   return _b1f_not;
	case CU_BOOL1F_TRUE:  return _b1f_true;
    }
    cu_debug_unreachable();
}

cu_bool1f_t
cu_bool1f_from_func(cu_bool_t (*f)(cu_bool_t))
{
    cu_bool_t yF = (*f)(cu_false) ? 1 : 0;
    cu_bool_t yT = (*f)(cu_true)  ? 2 : 0;
    return (cu_bool1f_t)(yF + yT);
}

static cu_bool_t _b2f_false(cu_bool_t x, cu_bool_t y) { return cu_false; }
static cu_bool_t _b2f_nor(cu_bool_t x, cu_bool_t y) { return !(x || y); }
static cu_bool_t _b2f_and_not(cu_bool_t x, cu_bool_t y) { return x && !y; }
static cu_bool_t _b2f_not_right(cu_bool_t x, cu_bool_t y) { return !y; }
static cu_bool_t _b2f_not_and(cu_bool_t x, cu_bool_t y) { return !x && y; }
static cu_bool_t _b2f_not_left(cu_bool_t x, cu_bool_t y) { return !x; }
static cu_bool_t _b2f_xor(cu_bool_t x, cu_bool_t y) { return !x != !y; }
static cu_bool_t _b2f_nand(cu_bool_t x, cu_bool_t y) { return !(x && y); }
static cu_bool_t _b2f_and(cu_bool_t x, cu_bool_t y) { return x && y; }
static cu_bool_t _b2f_iff(cu_bool_t x, cu_bool_t y) { return !x == !y; }
static cu_bool_t _b2f_left(cu_bool_t x, cu_bool_t y) { return x; }
static cu_bool_t _b2f_or_not(cu_bool_t x, cu_bool_t y) { return x || !y; }
static cu_bool_t _b2f_right(cu_bool_t x, cu_bool_t y) { return y; }
static cu_bool_t _b2f_not_or(cu_bool_t x, cu_bool_t y) { return !x || y; }
static cu_bool_t _b2f_or(cu_bool_t x, cu_bool_t y) { return x || y; }
static cu_bool_t _b2f_true(cu_bool_t x, cu_bool_t y) { return cu_true; }

static cu_bool_t (*_b2f_arr[16])(cu_bool_t, cu_bool_t) = {
    _b2f_false, _b2f_nor, _b2f_and_not, _b2f_not_right,
    _b2f_not_and, _b2f_not_left, _b2f_xor, _b2f_nand,
    _b2f_and, _b2f_iff, _b2f_left, _b2f_or_not,
    _b2f_right, _b2f_not_or, _b2f_or, _b2f_true,
};

cu_bool_t (*cu_bool2f_to_func(cu_bool2f_t f))(cu_bool_t, cu_bool_t)
{
    return _b2f_arr[f];
}

cu_bool2f_t
cu_bool2f_from_func(cu_bool_t (*f)(cu_bool_t, cu_bool_t))
{
    int r;
    r  = (*f)(cu_false, cu_false) ? 1 : 0;
    r |= (*f)(cu_true,  cu_false) ? 2 : 0;
    r |= (*f)(cu_false, cu_true)  ? 4 : 0;
    r |= (*f)(cu_true,  cu_true)  ? 8 : 0;
    return (cu_bool1f_t)r;
}
