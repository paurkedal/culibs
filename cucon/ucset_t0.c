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
#define CHECK_MOD CHECK_SIZE
#define CHECK_REPEAT 1000

#define WORD_WIDTH (sizeof(cu_word_t)*8)

cu_clos_def(_conj_check, cu_prot(cu_bool_t, uintptr_t key),
    ( size_t count;
      cucon_ucset_t seen; ))
{
    cu_clos_self(_conj_check);
    cucon_ucset_t seen = cucon_ucset_insert(self->seen, key);
    cu_debug_assert(seen != self->seen);
    self->seen = seen;
    ++self->count;
    return cu_true;
}

cu_clos_def(_iter_check, cu_prot(void, uintptr_t key),
    ( size_t count;
      cucon_ucset_t seen; ))
{
    cu_clos_self(_iter_check);
    cucon_ucset_t seen = cucon_ucset_insert(self->seen, key);
    self->seen = seen;
    ++self->count;
}

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
	_conj_check_t conj_cb;
	_iter_check_t iter_cb;
	size_t count = 0;

	memset(bitset, 0, sizeof(bitset));
	for (j = 0; j < CHECK_SIZE; ++j) {
	    unsigned long key = lrand48()%(CHECK_MOD/2 - 1);
	    cuex_t treep;
	    cu_bool_t got_it;
	    got_it = !!(bitset[key/WORD_WIDTH]
			& (CU_WORD_C(1) << key%WORD_WIDTH));
	    bitset[key/WORD_WIDTH] |= CU_WORD_C(1) << key%WORD_WIDTH;
	    if (key < minkey)
		minkey = key;
	    if (key > maxkey)
		maxkey = key;
	    treep = cucon_ucset_insert(tree, key);
#if CUCON_UCSET_ENABLE_HCONS
	    cu_debug_assert(got_it == (treep == tree));
#endif
	    cu_test_assert_int_eq(got_it, cucon_ucset_find(tree, key));
	    cu_test_assert_size_eq(count + !got_it, cucon_ucset_card(treep));
	    if (treep != tree) {
		++count;
		tree = treep;
	    }
	    cu_test_assert(cucon_ucset_find(tree, key));
	}

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

	cu_test_assert_size_eq(count, cucon_ucset_card(tree));
	cu_test_assert(minkey == cucon_ucset_min_ukey(tree));
	cu_test_assert(maxkey == cucon_ucset_max_ukey(tree));

	iter_cb.count = 0;
	iter_cb.seen = NULL;
	cucon_ucset_iter(tree, _iter_check_prep(&iter_cb));
#if CUCON_UCSET_ENABLE_HCONS
	cu_test_assert(iter_cb.seen == tree);
#endif
	cu_test_assert(iter_cb.count == count);

	conj_cb.count = 0;
	conj_cb.seen = NULL;
	cucon_ucset_conj(tree, _conj_check_prep(&conj_cb));
#if CUCON_UCSET_ENABLE_HCONS
	cu_test_assert(conj_cb.seen == tree);
#endif
	cu_test_assert(conj_cb.count == count);
    }
}

int
main()
{
    int i;
    cucon_ucset_t tree = NULL;
    cu_init();
    printf("Inserting ");
    for (i = 0; i < 0x100; ++i) {
	cucon_ucset_t treep;
	long key = lrand48()%0x200;
	treep = cucon_ucset_insert(tree, key);
	if (treep != tree)
	    printf(" %ld", key);
	tree = treep;
    }
    putc('\n', stdout);
    cucon_ucset_dump(tree, stdout);
    printf("card = %zd\n", cucon_ucset_card(tree));
    check();
    return 2*!!cu_test_bug_count();
}
