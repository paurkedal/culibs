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

#include <cucon/list.h>
#include <stdio.h>


int
main()
{
    struct cucon_list_s list;
    cucon_listnode_t it;

    cu_init();
    cucon_list_cct(&list);
    cucon_list_append_ptr(&list, "beta");
    cucon_list_prepend_ptr(&list, "alpha");
    it = cucon_list_append_mem(&list, sizeof(void*));
    *(void**)cucon_listnode_mem(it) = "gamma";

    for (it = cucon_list_begin(&list); it != cucon_list_end(&list);
	 it = cucon_listnode_next(it))
	printf("%s\n", (char*)cucon_listnode_ptr(it));

    return 0;
}
