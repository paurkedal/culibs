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
#include <cu/test.h>

void
_test1f()
{
    int i;
    for (i = 0; i < 2; ++i) {
	cu_bool_t x = i;
	cu_test_assert(!cu_bool1f_apply(CU_BOOL1F_FALSE, x));
	cu_test_assert(cu_bool1f_apply(CU_BOOL1F_TRUE, x));
	cu_test_assert(cu_bool1f_apply(CU_BOOL1F_IDENT, x) == x);
	cu_test_assert(cu_bool1f_apply(CU_BOOL1F_NOT, x) == !x);
	cu_test_assert(!cu_bool1f_apply(cu_bool1f_konst(cu_false), x));
	cu_test_assert(cu_bool1f_apply(cu_bool1f_konst(cu_true), x));
    }

    cu_test_assert(cu_bool1f_not(CU_BOOL1F_FALSE) == CU_BOOL1F_TRUE);
    cu_test_assert(cu_bool1f_not(CU_BOOL1F_TRUE) == CU_BOOL1F_FALSE);
    cu_test_assert(cu_bool1f_not(CU_BOOL1F_IDENT) == CU_BOOL1F_NOT);
    cu_test_assert(cu_bool1f_not(CU_BOOL1F_NOT) == CU_BOOL1F_IDENT);

    for (i = 0; i < 4; ++i) {
	cu_bool1f_t g = (cu_bool1f_t)i;
	cu_test_assert(cu_bool1f_compose(CU_BOOL1F_FALSE, g)== CU_BOOL1F_FALSE);
	cu_test_assert(cu_bool1f_compose(CU_BOOL1F_TRUE, g) == CU_BOOL1F_TRUE);
	cu_test_assert(cu_bool1f_compose(CU_BOOL1F_IDENT, g) == g);
	cu_test_assert(cu_bool1f_compose(CU_BOOL1F_NOT, g) == cu_bool1f_not(g));
    }
}

void
_test2f()
{
    int i;
    for (i = 0; i < 4; ++i) {
	cu_bool_t x = i & 1;
	cu_bool_t y = i >> 1;

	cu_test_assert(!cu_bool2f_apply(CU_BOOL2F_FALSE, x, y));
	cu_test_assert(cu_bool2f_apply(CU_BOOL2F_TRUE, x, y));
	cu_test_assert(cu_bool2f_apply(CU_BOOL2F_LEFT, x, y) == x);
	cu_test_assert(cu_bool2f_apply(CU_BOOL2F_RIGHT, x, y) == y);
	cu_test_assert(cu_bool2f_apply(CU_BOOL2F_NOT_LEFT, x, y) == !x);
	cu_test_assert(cu_bool2f_apply(CU_BOOL2F_NOT_RIGHT, x, y) == !y);
	cu_test_assert(cu_bool2f_apply(CU_BOOL2F_NOR, x, y) == !(x | y));
	cu_test_assert(cu_bool2f_apply(CU_BOOL2F_XOR, x, y) == (x ^ y));
	cu_test_assert(cu_bool2f_apply(CU_BOOL2F_NAND, x, y) == !(x & y));
	cu_test_assert(cu_bool2f_apply(CU_BOOL2F_AND, x, y) == (x & y));
	cu_test_assert(cu_bool2f_apply(CU_BOOL2F_IFF, x, y) == (x == y));
	cu_test_assert(cu_bool2f_apply(CU_BOOL2F_OR, x, y) == (x | y));
	cu_test_assert(cu_bool2f_apply(CU_BOOL2F_AND_NOT, x, y) == (x & !y));
	cu_test_assert(cu_bool2f_apply(CU_BOOL2F_NOT_AND, x, y) == ((!x) & y));
	cu_test_assert(cu_bool2f_apply(CU_BOOL2F_OR_NOT, x, y) == (x | !y));
	cu_test_assert(cu_bool2f_apply(CU_BOOL2F_NOT_OR, x, y) == ((!x) | y));
    }
    cu_test_assert(cu_bool2f_konst(cu_true) == CU_BOOL2F_TRUE);
    cu_test_assert(cu_bool2f_konst(cu_false) == CU_BOOL2F_FALSE);

    for (i = 0; i < 0x40; ++i) {
	cu_bool_t x = i & 1;
	cu_bool_t y = i >> 1;
	cu_bool2f_t f = (cu_bool2f_t)(i >> 2);

	cu_bool1f_t fL = cu_bool2f_apply_left(f, x);
	cu_bool1f_t fR = cu_bool2f_apply_right(f, y);
	cu_test_assert(cu_bool1f_apply(fL, y) == cu_bool2f_apply(f, x, y));
	cu_test_assert(cu_bool1f_apply(fL, y) == cu_bool1f_apply(fR, x));

	cu_test_assert(cu_bool2f_apply(cu_bool2f_not(f), x, y)
		       == !cu_bool2f_apply(f, x, y));
	cu_test_assert(cu_bool2f_apply(cu_bool2f_not_left(f), x, y)
		       == cu_bool2f_apply(f, !x, y));
	cu_test_assert(cu_bool2f_apply(cu_bool2f_not_right(f), x, y)
		       == cu_bool2f_apply(f, x, !y));
	cu_test_assert(cu_bool2f_apply(cu_bool2f_swap(f), x, y)
		       == cu_bool2f_apply(f, y, x));

	/* cu_bool2f_compose */
	cu_test_assert(cu_bool2f_apply(cu_bool2f_compose(CU_BOOL1F_NOT, f),
				       x, y) == !cu_bool2f_apply(f, x, y));

	/* cu_bool2f_compose_left */
	cu_test_assert(cu_bool2f_apply(
		cu_bool2f_compose_left(f, CU_BOOL1F_NOT), x, y)
	    == cu_bool2f_apply(f, !x, y));
	cu_test_assert(cu_bool2f_apply(
		cu_bool2f_compose_left(f, CU_BOOL1F_FALSE), x, y)
	    == cu_bool2f_apply(f, cu_false, y));
	cu_test_assert(cu_bool2f_apply(
		cu_bool2f_compose_left(f, CU_BOOL1F_TRUE), x, y)
	    == cu_bool2f_apply(f, cu_true, y));

	/* cu_bool2f_compose_right */
	cu_test_assert(cu_bool2f_apply(
		cu_bool2f_compose_right(f, CU_BOOL1F_NOT), x, y)
	    == cu_bool2f_apply(f, x, !y));
	cu_test_assert(cu_bool2f_apply(
		cu_bool2f_compose_right(f, CU_BOOL1F_FALSE), x, y)
	    == cu_bool2f_apply(f, x, cu_false));
	cu_test_assert(cu_bool2f_apply(
		cu_bool2f_compose_right(f, CU_BOOL1F_TRUE), x, y)
	    == cu_bool2f_apply(f, x, cu_true));
    }

    for (i = 0; i < 4; ++i) {
	cu_bool2f_t f = (cu_bool2f_t)i;

	/* cu_bool2f_compose */
	cu_test_assert(cu_bool2f_compose(CU_BOOL1F_FALSE, f)
		       == CU_BOOL2F_FALSE);
	cu_test_assert(cu_bool2f_compose(CU_BOOL1F_TRUE, f)
		       == CU_BOOL2F_TRUE);
	cu_test_assert(cu_bool2f_compose(CU_BOOL1F_IDENT, f) == f);

	/* cu_bool2f_compose_left, cu_bool2f_compose_right */
	cu_test_assert(cu_bool2f_compose_left(f, CU_BOOL1F_IDENT) == f);
	cu_test_assert(cu_bool2f_compose_right(f, CU_BOOL1F_IDENT) == f);
    }
}

int
main()
{
    cu_init();

    _test1f();
    _test2f();

    return 2*!!cu_test_bug_count();
}
