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

#include <cu/clos.h>
#include <assert.h>

cu_clos_def(my_clos, cu_prot(int, int k, double x),
    ( int i; int j; ))
{
    cu_clos_self(my_clos);
    assert(self->i == 1001);
    assert(self->j == 1002);
    assert(k == 1003);
    assert(x == 1.1);
    return 1004;
}

cu_clop_def(plain, int, int k, double x)
{
    assert(k == 1010);
    assert(x == 1.2);
    return 1011;
}

cu_clos_def(testfn0,
	    cu_prot(int, int i, int j),
	    (int k; int l;))
{
    cu_clos_self(testfn0);
    return i*self->k + j*self->l;
}

cu_clos_dec(testfn1,
	    cu_prot(int, char *s),
	    (int n;));

cu_clos_fun(testfn1, cu_prot(int, char *s))
{
    cu_clos_self(testfn1);
    return atoi(s) + self->n;
}


int
main()
{
    int l;
    my_clos_t clos;
    testfn0_t fn0;
    testfn1_t fn1;

    clos.i = 1001;
    clos.j = 1002;
    l = cu_call(my_clos_prep(&clos), 1003, 1.1);
    assert(l == 1004);

    l = cu_call(plain, 1010, 1.2);
    assert(l == 1011);

    testfn0_cct(&fn0);
    fn0.k = 10;
    fn0.l = 100;
    assert(cu_call(testfn0_prep(&fn0), 2, 3) == 320);

    testfn1_cct(&fn1);
    fn1.n = 6;
    assert(cu_call(testfn1_prep(&fn1), "4") == 10);

    return 0;
}
