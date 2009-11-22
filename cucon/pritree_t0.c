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

#include <cucon/pritree.h>
#include <cu/memory.h>
#include <assert.h>

int
main()
{
    size_t N = 100000;
    int n;
    struct cucon_pritree pritree;
    double x = 1.0;
    cucon_prinode_t *tv;

    cu_init();

    tv = cu_salloc(sizeof(cucon_prinode_t)*N);
    cucon_pritree_init(&pritree);
    for (n = 0; n < N; ++n) {
	tv[n] = cu_gnew(struct cucon_prinode);
	cucon_pritree_insert_init(&pritree, tv[n], drand48());
    }
    for (n = 0; n < N; ++n) {
	cucon_prinode_t prinode = cucon_pritree_front(&pritree);
	assert(prinode->priority <= x);
	x = prinode->priority;
	cucon_pritree_pop(&pritree);
    }
    assert(cucon_pritree_is_empty(&pritree));

    for (n = 0; n < N; ++n) {
	tv[n] = cu_gnew(struct cucon_prinode);
	cucon_pritree_insert_init(&pritree, tv[n], drand48());
    }
    for (n = 0; n < N/2; ++n)
	cucon_pritree_change_priority(&pritree, tv[2*n], drand48());
    x = 1.0;
    for (n = 0; n < N; ++n) {
	cucon_prinode_t prinode = cucon_pritree_front(&pritree);
	assert(prinode->priority <= x);
	x = prinode->priority;
	if (n % 2 == 0)
	    cucon_pritree_change_priority(&pritree,
					cucon_pritree_front(&pritree), -1.0);
	else
	    cucon_pritree_pop(&pritree);
    }
    for (n = 0; n < N/2; ++n)
	cucon_pritree_pop(&pritree);
    assert(cucon_pritree_is_empty(&pritree));

    return 0;
}
