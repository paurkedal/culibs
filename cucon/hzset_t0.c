/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2008  Petter Urkedal <urkedal@nbi.dk>
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

#include <cucon/hzset.h>
#include <cu/clos.h>
#include <cu/test.h>

cu_clos_def(divides, cu_prot(cu_bool_t, void const *key), (int j;))
{
    cu_clos_self(divides);
    return *((cu_word_t *)key) % self->j == 0;
}

void
test()
{
    int N = 10000;
    cu_word_t i;
    cu_word_t const *p;
    struct cucon_hzset_s set;
    divides_t divides;
    struct cucon_hzset_itr_s itr;
    int count;

    /* Build set. */
    cucon_hzset_init(&set, 1);
    for (i = 0; i < N; i += 1)
	cucon_hzset_insert(&set, &i);
    cu_test_assert(cucon_hzset_size(&set) == N);

    /* Test forall and filter */
    divides.j = 2;
    cu_test_assert(!cucon_hzset_forall(divides_prep(&divides), &set));
    cucon_hzset_filter(divides_ref(&divides), &set);
    cu_test_assert(cucon_hzset_size(&set) == N/2);
    cu_test_assert(cucon_hzset_forall(divides_ref(&divides), &set));

    /* Test iteration */
    cucon_hzset_itr_init(&itr, &set);
    count = 0;
    while ((p = cucon_hzset_itr_get(&itr))) {
	++count;
	cu_test_assert(*p % 2 == 0);
    }
    cu_test_assert(count == N/2);
}

int
main()
{
    cu_init();
    test();
    return 2*!!cu_test_bug_count();
}
