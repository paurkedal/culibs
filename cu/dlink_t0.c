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

#include <cu/dlink.h>
#include <cu/test.h>
#include <cu/memory.h>

typedef struct _link *_link_t;
struct _link
{
    cu_inherit (cu_dlink_s);
    int value;
};

static cu_dlink_t
_new_singleton(int value)
{
    _link_t l = cu_gnew(struct _link);
    l->value = value;
    cu_dlink_init_singleton(cu_to(cu_dlink, l));
    return cu_to(cu_dlink, l);
}

static cu_dlink_t
_new_cycle_upto(int n)
{
    int i;
    cu_dlink_t l;

    if (n == 0)
	return NULL;
    l = _new_singleton(0);
    for (i = 1; i < n; ++i) {
	_link_t lp = cu_gnew(struct _link);
	lp->value = i;
	cu_dlink_init_singleton(cu_to(cu_dlink, lp));
	cu_dlink_insert_before(l, cu_to(cu_dlink, lp));
    }
    return l;
}

static int
_value(cu_dlink_t l)
{
    return cu_downcast(_link, cu_dlink_s, l)->value;
}

int
main()
{
    cu_dlink_t x, y;
    cu_init();

    /* Tests on empty cycles. */
    cu_test_assert(!cu_dlink_is_singleton(NULL));
    cu_test_assert(cu_dlink_card_leq_1(NULL));
    cu_test_assert(!cu_dlink_card_eq_2(NULL));
    cu_test_assert(cu_dlink_card_leq_2(NULL));
    cu_test_assert(cu_dlink_card_lim(NULL, SIZE_MAX) == 0);
    cu_test_assert(cu_dlink_cat_d(NULL, NULL) == NULL);

    /* Tests on singleton cycles. */
    x = _new_singleton(1);
    cu_dlink_validate(x);
    cu_test_assert(cu_dlink_is_singleton(x));
    cu_test_assert(cu_dlink_card_leq_1(x));
    cu_test_assert(!cu_dlink_card_eq_2(x));
    cu_test_assert(cu_dlink_card_leq_2(x));
    cu_test_assert(cu_dlink_card_lim(x, 0) == 0);
    cu_test_assert(cu_dlink_card_lim(x, 1) == 1);
    cu_test_assert(cu_dlink_card_lim(x, SIZE_MAX) == 1);
    cu_test_assert(cu_dlink_cat_d(x, NULL) == x);
    cu_test_assert(cu_dlink_cat_d(NULL, x) == x);

    /* Test splicing of singletons. */
    y = _new_singleton(2);
    cu_dlink_splice_before(x, y);
    cu_dlink_card_eq_2(x);
    cu_dlink_splice_before(x, y);
    cu_dlink_is_singleton(x);

    /* Tests on two-link cycles. */
    y = _new_singleton(2);
    cu_dlink_insert_before(x, y);
    cu_dlink_validate(x);
    cu_test_assert(!cu_dlink_is_singleton(x));
    cu_test_assert(!cu_dlink_card_leq_1(x));
    cu_test_assert(cu_dlink_card_eq_2(x));
    cu_test_assert(cu_dlink_card_leq_2(x));
    cu_test_assert(cu_dlink_card_lim(x, 0) == 0);
    cu_test_assert(cu_dlink_card_lim(x, 1) == 1);
    cu_test_assert(cu_dlink_card_lim(x, 2) == 2);
    cu_test_assert(cu_dlink_card_lim(x, SIZE_MAX) == 2);

    /* Tests on longer cycles, including erase. */
    cu_test_assert(cu_dlink_cat_d(x, _new_singleton(3)) == x);
    cu_dlink_validate(x);
    cu_test_assert(cu_dlink_card_lim(x, SIZE_MAX) == 3);
    cu_dlink_erase(x->prev);
    cu_dlink_validate(x);
    cu_test_assert(_value(x) == 1);
    cu_test_assert(_value(x->prev) == 2);
    cu_test_assert(cu_dlink_card_eq_2(x));

    /* Test splicing of two-link cycles. */
    y = _new_singleton(11);
    cu_dlink_move_after(y, _new_singleton(12));
    cu_dlink_validate(x);
    cu_dlink_splice_before(x, y); /* concatenate to [1, 2, 11, 12] */
    cu_dlink_validate(x);
    cu_test_assert(_value(x->prev) == 12);
    cu_dlink_splice_after(x, y); /* split into [1, 12], [11, 2] */
    cu_dlink_validate(x);
    cu_dlink_validate(y);
    cu_test_assert(cu_dlink_card_eq_2(x));
    cu_test_assert(cu_dlink_card_eq_2(y));
    cu_test_assert(_value(x) == 1);
    cu_test_assert(_value(x->next) == 12);
    cu_test_assert(_value(y) == 11);
    cu_test_assert(_value(y->next) == 2);

    /* Splice complements */
    x = _new_cycle_upto(5);
    cu_dlink_splice_complement_before(x, _new_singleton(1000));
    cu_dlink_validate(x);
    cu_dlink_splice_complement_after(x, _new_singleton(1000));
    cu_dlink_validate(x);
    cu_test_assert(cu_dlink_card_lim(x, SIZE_MAX) == 5);
    y = cu_dlink_cat_d(_new_singleton(1000), _new_singleton(-80));
    cu_dlink_splice_complement_before(x, y);
    y = cu_dlink_cat_d(_new_singleton(1000), _new_singleton(80));
    cu_dlink_splice_complement_after(x, y);
    cu_test_assert(_value(x) == 0);
    cu_test_assert(_value(x->next) == 80);
    cu_test_assert(_value(x->next->next) == 1);
    cu_test_assert(_value(x->prev) == -80);
    cu_test_assert(_value(x->prev->prev) == 4);

    return 2*!!cu_test_bug_count();
}
