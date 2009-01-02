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

#define CHECK_REPEAT	500

#define FULL_COUNT	0x140
#define DENSE_COUNT	0x100
#define DENSE_MIN	-0x80
#define DENSE_MOD	0x200
#define DENSE_MAX	(DENSE_MIN + DENSE_MOD)

#define TC_DIFF_MIN	-512
#define TC_DIFF_SPAN	1024
#define TC_CUT_MIN	-512
#define TC_CUT_SPAN	1024

#define WORD_WIDTH (sizeof(cu_word_t)*8)

cu_clos_def(_conj_check, cu_prot(cu_bool_t, uintptr_t key),
    ( size_t count;
      cucon_ucset_t seen; ))
{
    cu_clos_self(_conj_check);
    cucon_ucset_t seen = cucon_ucset_insert(self->seen, key);
    cu_test_assert(seen != self->seen);
    self->seen = seen;
    ++self->count;
    return cu_true;
}

cu_clos_def(_iter_check, cu_prot(void, uintptr_t key),
    ( size_t count;
      intptr_t diff;
      uintptr_t cut_min, cut_max;
      cucon_ucset_t seen;
      cucon_ucset_t cut_add_out;
      cucon_ucset_itr_t itr; ))
{
    cu_clos_self(_iter_check);
    cucon_ucset_t seen = cucon_ucset_insert(self->seen, key);
    uintptr_t keyp;
    self->seen = seen;
    ++self->count;
    cu_test_assert(!cucon_ucset_itr_at_end(self->itr));
    keyp = cucon_ucset_itr_get(self->itr);
    cu_test_assert(keyp == key);
    if (self->cut_min <= key && key <= self->cut_max)
	self->cut_add_out = cucon_ucset_insert(self->cut_add_out,
					       key + self->diff);
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
    static cu_word_t bitset[DENSE_MOD/WORD_WIDTH];
    for (i = 0; i < CHECK_REPEAT; ++i) {
	cucon_ucset_t U = NULL, Ut, Uc, Uct, Up, Upp;
	cucon_ucset_t S[3] = {NULL, NULL, NULL}, S01, S12, S1p;
	int j;
	uintptr_t key;
	uintptr_t umin = UINTPTR_MAX;
	uintptr_t umax = 0;
	intptr_t smin = INTPTR_MAX;
	intptr_t smax = INTPTR_MIN;
	uintptr_t cut_min, cut_max;
	_conj_check_t conj_cb;
	_iter_check_t iter_cb;
	_filter_check_t filter_cb;
	size_t count = 0;

	memset(bitset, 0, sizeof(bitset));
	for (j = 0; j < FULL_COUNT; ++j) {
	    int choice;
	    uintptr_t ukey;
	    cucon_ucset_t Up;
	    cu_bool_t got_it;

	    if (j < DENSE_COUNT) {
		ukey = lrand48() % DENSE_MOD;
		key = ukey + DENSE_MIN;
		got_it = !!(bitset[ukey/WORD_WIDTH]
			    & (CU_WORD_C(1) << ukey%WORD_WIDTH));
	    }
	    else {
		key = lrand48();
		got_it = cucon_ucset_find(U, key);
	    }
	    if (DENSE_MIN <= (intptr_t)key && (intptr_t)key < DENSE_MAX) {
		ukey = key - DENSE_MIN;
		bitset[ukey/WORD_WIDTH] |= CU_WORD_C(1) << ukey%WORD_WIDTH;
	    }

	    Up = cucon_ucset_insert(U, key);

	    if (key < umin)		umin = key;
	    if (key > umax)		umax = key;
	    if ((intptr_t)key < smin)	smin = key;
	    if ((intptr_t)key > smax)	smax = key;
	    cu_test_assert(umin == cucon_ucset_umin(Up));
	    cu_test_assert(umax == cucon_ucset_umax(Up));
	    cu_test_assert(smin == cucon_ucset_smin(Up));
	    cu_test_assert(smax == cucon_ucset_smax(Up));

	    if (j == 0) {
		cu_test_assert(cucon_ucset_is_singleton(Up));
		cu_test_assert(cucon_ucset_eq(cucon_ucset_singleton(key), Up));
	    }
	    else if (!got_it)
		cu_test_assert(!cucon_ucset_is_singleton(Up));
	    cu_test_assert_int_eq(got_it, cucon_ucset_find(U, key));
	    cu_test_assert(got_it == cucon_ucset_eq(Up, U));
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

	for (j = 0; j < DENSE_MOD; ++j) {
	    cu_bool_t bit = !!(bitset[j/WORD_WIDTH]
			       & (CU_WORD_C(1) << j%WORD_WIDTH));
	    uintptr_t key = j + DENSE_MIN;
	    cu_bool_t find = cucon_ucset_find(U, key);
	    if (bit != find) {
		cucon_ucset_dump(U, stdout);
		printf(bit
		       ? "Did not find inserted bit %d\n"
		       : "Found bit %d which was not inserted\n", j);
		abort();
	    }
	}

	cu_test_assert_size_eq(count, cucon_ucset_card(U));
	for (j = 0; j < 3; ++j)
	    cu_test_assert(cucon_ucset_subeq(S[j], U));

	/* Check cucon_ucset_iter. */
	iter_cb.count = 0;
	iter_cb.seen = NULL;
	iter_cb.itr = cucon_ucset_itr_new(U);
	iter_cb.diff = lrand48() % TC_DIFF_SPAN + TC_DIFF_MIN;
	iter_cb.cut_min = cut_min = lrand48() % TC_CUT_SPAN + TC_CUT_MIN;
	iter_cb.cut_max = cut_max = lrand48() % TC_CUT_SPAN - TC_CUT_MIN;
	iter_cb.cut_add_out = NULL;
	cucon_ucset_iter(U, _iter_check_prep(&iter_cb));
	cu_test_assert(cucon_ucset_eq(iter_cb.seen, U));
	cu_test_assert(iter_cb.count == count);
	cu_test_assert(cucon_ucset_itr_at_end(iter_cb.itr));
	Up = cucon_ucset_translate_uclip(U, iter_cb.diff, cut_min, cut_max);
	Ut = cucon_ucset_translate(U, iter_cb.diff);
	cu_test_assert(cucon_ucset_eq(Up, iter_cb.cut_add_out));
	Uc = cucon_ucset_uclip(U, cut_min, cut_max);
	Uct = cucon_ucset_translate(Uc, iter_cb.diff);
	cu_test_assert(cucon_ucset_eq(Up, Uct));
	if (((intptr_t)cut_min < 0) == ((intptr_t)cut_max < 0)) {
	    Upp = cucon_ucset_translate_sclip(U, iter_cb.diff,
					      cut_min, cut_max);
	    cu_test_assert(cucon_ucset_eq(Upp, Up));
	    Uc = cucon_ucset_sclip(U, cut_min, cut_max);
	    Uct = cucon_ucset_translate(Uc, iter_cb.diff);
	    cu_test_assert(cucon_ucset_eq(Uct, Up));
	}
	else if ((intptr_t)cut_max < (intptr_t)cut_min) {
	    Upp = cucon_ucset_translate_sclip(U, iter_cb.diff,
					      cut_max + 1, cut_min - 1);
	    cu_test_assert(cucon_ucset_eq(Upp, cucon_ucset_compl(Ut, Up)));
	    Uc = cucon_ucset_sclip(U, cut_max + 1, cut_min - 1);
	    Uct = cucon_ucset_translate(Uc, iter_cb.diff);
	    cu_test_assert(cucon_ucset_eq(Uct, Upp));
	}

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

	/* Disjoint */
	cu_test_assert(cucon_ucset_disjoint(S[0], S[1]));
	cu_test_assert(cucon_ucset_is_empty(S[1]) ==
		       cucon_ucset_disjoint(S01, S12));
	key = lrand48();
	cu_test_assert(!cucon_ucset_disjoint(cucon_ucset_insert(S[0], key),
					     cucon_ucset_insert(S[1], key)));
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
