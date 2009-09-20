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

void
cu_dlink_validate(cu_dlink_t l)
{
    if (l) {
	cu_dlink_t l0 = l;
	do {
	    cu_debug_assert(l->next->prev == l);
	    l = l->next;
	} while (l != l0);
    }
}

size_t
cu_dlink_card_lim(cu_dlink_t x, size_t limit)
{
    size_t n = 0;
    if (x && limit) {
	cu_dlink_t x0 = x;
	do {
	    ++n;
	    x = x->next;
	} while (x != x0 && n < limit);
    }
    return n;
}

void
cu_dlink_move_before(cu_dlink_t l, cu_dlink_t l_mv)
{
    cu_debug_assert(l != l_mv);
    l_mv->prev->next = l_mv->next;
    l_mv->next->prev = l_mv->prev;
    l_mv->prev = l->prev;
    l_mv->next = l;
    l_mv->prev->next = l_mv;
    l->prev = l_mv;
}

void
cu_dlink_move_after(cu_dlink_t l, cu_dlink_t l_mv)
{
    cu_debug_assert(l != l_mv);
    l_mv->prev->next = l_mv->next;
    l_mv->next->prev = l_mv->prev;
    l_mv->prev = l;
    l_mv->next = l->next;
    l->next = l_mv;
    l_mv->next->prev = l_mv;
}

void
cu_dlink_splice_before(cu_dlink_t x, cu_dlink_t y)
{
    cu_dlink_t xp = x->prev;
    cu_dlink_t yp = y->prev;
    x->prev = yp;
    y->prev = xp;
    yp->next = x;
    xp->next = y;
}

void
cu_dlink_splice_after(cu_dlink_t x, cu_dlink_t y)
{
    cu_dlink_t xn = x->next;
    cu_dlink_t yn = y->next;
    x->next = yn;
    y->next = xn;
    yn->prev = x;
    xn->prev = y;
}

cu_dlink_t
cu_dlink_cat_d(cu_dlink_t x, cu_dlink_t y)
{
    if (x == NULL)
	return y;
    else {
	if (y != NULL)
	    cu_dlink_splice_before(x, y);
	return x;
    }
}

void
cu_dlink_splice_complement_before(cu_dlink_t l, cu_dlink_t l_head)
{
    cu_debug_dlink_assert_valid(l);
    cu_debug_dlink_assert_valid(l_head);
    if (l_head->next != l_head) {
	l_head->prev->next = l;
	l_head->next->prev = l->prev;
	l->prev->next = l_head->next;
	l->prev = l_head->prev;
	cu_debug_dlink_invalidate(l_head);
    }
}

void
cu_dlink_splice_complement_after(cu_dlink_t l, cu_dlink_t l_head)
{
    cu_debug_dlink_assert_valid(l);
    cu_debug_dlink_assert_valid(l_head);
    if (l_head->next != l_head) {
	l_head->next->prev = l;
	l_head->prev->next = l->next;
	l->next->prev = l_head->prev;
	l->next = l_head->next;
	cu_debug_dlink_invalidate(l_head);
    }
}
