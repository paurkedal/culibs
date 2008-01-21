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

#include <cucon/ucset.h>
#include <cu/debug.h>
#include <cu/test.h>
#include <string.h>

#define CHECK_SIZE 0x100
#define CHECK_MOD 4*CHECK_SIZE
#define CHECK_REPEAT 1000

#define WORD_WIDTH (sizeof(cu_word_t)*8)

void
check()
{
    int i;
    static cu_word_t bitset[CHECK_MOD/WORD_WIDTH];
    for (i = 0; i < CHECK_REPEAT; ++i) {
	cucon_ucset_t tree = NULL;
	int j;
	uintptr_t minkey = UINTPTR_MAX;
	uintptr_t maxkey = 0;
	memset(bitset, 0, sizeof(bitset));
	for (j = 0; j < CHECK_SIZE; ++j) {
	    unsigned long key = lrand48()%(CHECK_MOD/2 - 1);
	    bitset[key/WORD_WIDTH] |= CU_WORD_C(1) << key%WORD_WIDTH;
	    if (key < minkey)
		minkey = key;
	    if (key > maxkey)
		maxkey = key;
	    tree = cucon_ucset_insert(tree, key);
	    cu_test_assert(cucon_ucset_find(tree, key));
	}
	cu_test_assert(minkey == cucon_ucset_min_ukey(tree));
	cu_test_assert(maxkey == cucon_ucset_max_ukey(tree));
	for (j = 0; j < CHECK_MOD; ++j) {
	    cu_bool_t bit = !!(bitset[j/WORD_WIDTH]
			       & (CU_WORD_C(1) << j%WORD_WIDTH));
	    cu_bool_t find = cucon_ucset_find(tree, j);
	    if (bit != find) {
		cucon_ucset_dump(tree, stdout);
		printf(bit
		       ? "Did not find inserted bit %d\n"
		       : "Found bit %d which was not inserted\n", j);
		abort();
	    }
	}
    }
}

int
main()
{
    int i;
    cucon_ucset_t tree = NULL;
    cu_init();
    printf("inserting ");
    for (i = 0; i < 40; ++i) {
	long key = lrand48()%40;
	printf(" %ld", key);
	tree = cucon_ucset_insert(tree, key);
    }
    putc('\n', stdout);
    cucon_ucset_dump(tree, stdout);
    check();
    return 2*!!cu_test_bug_count();
}
