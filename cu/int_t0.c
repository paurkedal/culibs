/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2007  Petter Urkedal <urkedal@nbi.dk>
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

#include <cu/conf.h>
#include <cu/int.h>
#include <cu/size.h>
#include <cu/test.h>

int
ubit(unsigned long n)
{
    int i;
    for (i = sizeof(unsigned long)*8 - 1; i >= 0; --i)
	if (n & (1L << i))
	    return i;
    return -1;
}

int
lbit(unsigned long n)
{
    size_t i;
    for (i = 0; i < sizeof(unsigned long)*8;  ++i)
	if (n & (1 << i))
	    return i;
    return -1;
}

void
test_size(size_t i)
{
    unsigned int b = cu_size_floor_log2(i);
    unsigned int c = cu_size_ceil_log2(i);
    size_t j = cu_size_exp2floor(i);
    size_t jp = (size_t)1 << b;
    cu_test_assert(j == jp);
    cu_test_assert(j <= i);
    cu_test_assert(j > i/2);
    cu_test_assert((j & (j - 1)) == 0);
    cu_test_assert((i == j && b == c) || b + 1 == c);
}

int
main()
{
    unsigned long i;
    for (i = 1; i < 1000000; ++i) {
	unsigned long j = lrand48();
	if (j == 0)
	    continue;
	cu_test_assert(ubit(j) == cu_ulong_floor_log2(j));
	cu_test_assert(lbit(j) == cu_ulong_log2_lowbit(j));
    }

    for (i = 1; i < 1000; ++i)
	test_size(i);
    for (i = 0; i < 1000000; ++i)
	test_size(lrand48());
    test_size(SIZE_MAX/2 - 1);
    test_size(SIZE_MAX/2);
    test_size(SIZE_MAX/2 + 1);
    /* Last valid: cu_size_ceil_log2 overflows from SIZE_MAX/2 + 2. */

    return 0;
}
