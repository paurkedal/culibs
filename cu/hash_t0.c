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

#include <cu/hash.h>
#include <cu/test.h>

int main()
{
    cu_word_t arr[2];
    int i, j;
    cu_init();
    for (i = 0; i < 1000; ++i) {
	cu_hash_t h0, h1;
	cu_hash_t init;
	for (j = 0; j < 2; ++j)
	    arr[j] = lrand48();

	init = lrand48();
	h0 = cu_wordarr_hash_bj(1, arr, init);
	h1 = cu_1word_hash_bj(arr[0], init);
	cu_test_assert(h0 == h1);
	h0 = cu_wordarr_hash_bj(2, arr, init);
	h1 = cu_2word_hash_bj(arr[0], arr[1], init);
	cu_test_assert(h0 == h1);

	h0 = cu_wordarr_hash_noinit_bj(1, arr);
	h1 = cu_1word_hash_noinit_bj(arr[0]);
	cu_test_assert(h0 == h1);
	h0 = cu_wordarr_hash_noinit_bj(2, arr);
	h1 = cu_2word_hash_noinit_bj(arr[0], arr[1]);
	cu_test_assert(h0 == h1);
    }
    return 2*!!cu_test_bug_count();
}
