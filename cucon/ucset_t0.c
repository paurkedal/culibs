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
#include <cu/int.h>
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

cu_clos_def(_filter_check, cu_prot(cu_bool_t, uintptr_t key),
    ( size_t count;
      cucon_ucset_t filtered; ))
{
    cu_clos_self(_filter_check);
    if (cu_ulong_bit_count(key) & 1) {
	self->filtered = cucon_ucset_insert(self->filtered, key);
	++self->count;
	return cu_true;
    }
    else
	return cu_false;
}

void
check()
{
    int i;
    static cu_word_t bitset[CHECK_MOD/WORD_WIDTH];
    for (i = 0; i < CHECK_REPEAT; ++i) {
	cucon_ucset_t U = NULL, Up, S[3] = {NULL, NULL, NULL}, S01, S12, S1p;
	int j;
	uintptr_t minkey = UINTPTR_MAX;
	uintptr_t maxkey = 0;
	_conj_check_t conj_cb;
	_iter_check_t iter_cb;
	_filter_check_t filter_cb;
	size_t count = 0;

	memset(bitset, 0, sizeof(bitset));
	for (j = 0; j < CHECK_SIZE; ++j) {
	    int choice;
	    unsigned long key = lrand48()%(CHECK_MOD/2 - 1);
	    cuex_t Up;
	    cu_bool_t got_it;

	    got_it = !!(bitset[key/WORD_WIDTH]
			& (CU_WORD_C(1) << key%WORD_WIDTH));
	    bitset[key/WORD_WIDTH] |= CU_WORD_C(1) << key%WORD_WIDTH;
	    Up = cucon_ucset_insert(U, key);
	    if (key < minkey) {
		minkey = key;
		cu_debug_assert(minkey == cucon_ucset_min_ukey(Up));
	    }
	    if (key > maxkey) {
		maxkey = key;
		cu_debug_assert(maxkey == cucon_ucset_max_ukey(Up));
	    }
	    cu_debug_assert(got_it == cucon_ucset_eq(Up, U));
	    cu_test_assert_int_eq(got_it, cucon_ucset_find(U, key));
	    cu_test_assert_size_eq(count + !got_it, cucon_ucset_card(Up));
	    if (got_it)
		cu_test_assert(cucon_ucset_eq(U, Up));
	    else {
		cu_test_assert(cucon_ucset_subeq(U, Up));
		cu_test_assert(cucon_ucset_eq(cucon_ucset_erase(Up, key), U));
		++count;
	    }
	    U = Up;
	    cu_test_assert(cucon_ucset_find(U, key));

	    if (!got_it) {
		choice = lrand48() % 3;
		S[choice] = cucon_ucset_insert(S[choice], key);
	    }
	}

	for (j = 0; j < CHECK_MOD; ++j) {
	    cu_bool_t bit = !!(bitset[j/WORD_WIDTH]
			       & (CU_WORD_C(1) << j%WORD_WIDTH));
	    cu_bool_t find = cucon_ucset_find(U, j);
	    if (bit != find) {
		cucon_ucset_dump(U, stdout);
		printf(bit
		       ? "Did not find inserted bit %d\n"
		       : "Found bit %d which was not inserted\n", j);
		abort();
	    }
	}

	cu_test_assert_size_eq(count, cucon_ucset_card(U));
	cu_test_assert(minkey == cucon_ucset_min_ukey(U));
	cu_test_assert(maxkey == cucon_ucset_max_ukey(U));
	for (j = 0; j < 3; ++j)
	    cu_test_assert(cucon_ucset_subeq(S[j], U));

	/* Check cucon_ucset_iter. */
	iter_cb.count = 0;
	iter_cb.seen = NULL;
	cucon_ucset_iter(U, _iter_check_prep(&iter_cb));
	cu_test_assert(cucon_ucset_eq(iter_cb.seen, U));
	cu_test_assert(iter_cb.count == count);

	/* Check cucon_ucset_conj. */
	conj_cb.count = 0;
	conj_cb.seen = NULL;
	cucon_ucset_conj(U, _conj_check_prep(&conj_cb));
	cu_test_assert(cucon_ucset_eq(conj_cb.seen, U));
	cu_test_assert(conj_cb.count == count);

	/* Check cucon_ucset_filter. */
	filter_cb.count = 0;
	filter_cb.filtered = NULL;
	Up = cucon_ucset_filter(U, _filter_check_prep(&filter_cb));
	cu_test_assert_size_eq(filter_cb.count, cucon_ucset_card(Up));
	cu_test_assert(cucon_ucset_eq(Up, filter_cb.filtered));
	cu_test_assert(cucon_ucset_subeq(Up, U));

	/* Check union. */
	S01 = cucon_ucset_union(S[0], S[1]);
	S12 = cucon_ucset_union(S[1], S[2]);
	cu_test_assert(cucon_ucset_subeq(S[0], S01));
	cu_test_assert(cucon_ucset_subeq(S[1], S01));
	cu_test_assert(cucon_ucset_subeq(S[1], S12));
	cu_test_assert(cucon_ucset_subeq(S[2], S12));
	Up = cucon_ucset_union(S01, S[2]);
	cu_test_assert(cucon_ucset_eq(Up, U));
	Up = cucon_ucset_union(S12, S[0]);
	cu_test_assert(cucon_ucset_eq(Up, U));

	/* Check intersection. */
	S1p = cucon_ucset_isecn(S01, S12);
	cu_test_assert(cucon_ucset_eq(S1p, S[1]));

	/* Check set minus. */
	cu_test_assert(cucon_ucset_eq(S01, cucon_ucset_compl(U, S[2])));
	cu_test_assert(cucon_ucset_eq(S12, cucon_ucset_compl(U, S[0])));
	cu_test_assert(cucon_ucset_eq(S[0], cucon_ucset_compl(S01, S[1])));
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
