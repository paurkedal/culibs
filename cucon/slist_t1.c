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

#include <cucon/slist.h>
#include <stdio.h>

int
main()
{
    int i;
    ccg_slist l;
    cucon_slist_cct(&l);
    for (i = 0; i < 2; ++i) {
	cucon_slist_iterator it;
	cucon_slist_prepend_ptr(&l, "beta");
	cucon_slist_append_ptr(&l, "gamma");
	cucon_slist_prepend_ptr(&l, "alpha");
	cucon_slist_append_ptr(&l, "delta");
	printf("+ { ");
	for (it = cucon_slist_begin(&l); it != cucon_slist_end(&l);
	     it = cucon_slist_it_next(it))
	    printf("%s; ", (char*)cucon_slist_it_get_ptr(it));
	printf("}\n");
	it = cucon_slist_begin(&l);
	it = cucon_slist_it_next(cucon_slist_it_next(it));
	cucon_slist_it_erase_free(it);
	cucon_slist_it_erase_free(cucon_slist_begin(&l));
	it = cucon_slist_begin(&l);
	it = cucon_slist_it_next(it);
	cucon_slist_it_erase_free(it);
	printf("- { ");
	for (it = cucon_slist_begin(&l); it != cucon_slist_end(&l);
	     it = cucon_slist_it_next(it))
	    printf("%s; ", (char*)cucon_slist_it_get_ptr(it));
	printf("}\n");
	cucon_slist_clear_free(&l);
    }
    cucon_slist_dct_free(&l);
    return 0;
}

