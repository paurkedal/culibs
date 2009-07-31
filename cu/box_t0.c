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

#include <cu/pack.h>
#include <cu/test.h>
#include <string.h>
#include <limits.h>
#include <math.h>

#define uint unsigned int
#define ulong unsigned long
#define longdouble long double
#define TEST_BOX(type, x) \
    { \
	cu_box_t p; \
	type y; \
	memset(&p, 0xff, sizeof(p)); \
	p = cu_box_##type(x); \
	y = cu_unbox_##type(p); \
	cu_test_assert(x == y); \
    }

int
main()
{
    cu_init();
    TEST_BOX(int, 0);
    TEST_BOX(int, INT_MIN);
    TEST_BOX(int, INT_MAX);
    TEST_BOX(int, 0x5566aabb);
    TEST_BOX(uint, 0);
    TEST_BOX(uint, UINT_MAX);
    TEST_BOX(uint, 0xaabb5566);
    TEST_BOX(float, (float)M_PI);
    TEST_BOX(double, M_PI);
    TEST_BOX(longdouble, (long double)M_PI);
    return 2*!!cu_test_bug_count();
}
