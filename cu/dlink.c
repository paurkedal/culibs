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
cu_dlink_insert_list_before(cu_dlink_t l, cu_dlink_t l_head)
{
    cu_debug_dlink_assert_valid(l);
    cu_debug_dlink_assert_valid(l_head);
    l_head->prev->next = l;
    l_head->next->prev = l->prev;
    l->prev->next = l_head->next;
    l->prev = l_head->prev;
    cu_debug_dlink_invalidate(l_head);
}

void
cu_dlink_insert_list_after(cu_dlink_t l, cu_dlink_t l_head)
{
    cu_debug_dlink_assert_valid(l);
    cu_debug_dlink_assert_valid(l_head);
    l_head->next->prev = l;
    l_head->prev->next = l->next;
    l->next->prev = l_head->prev;
    l->next = l_head->next;
    cu_debug_dlink_invalidate(l_head);
}
