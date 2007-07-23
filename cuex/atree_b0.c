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

#include <cuex/atree.h>
#include <cuex/oprdefs.h>
#include <cudyn/misc.h>

cu_clop_def(opn0word, cu_word_t, cuex_t e)
{
    return (cu_word_t)cuex_opn_at(e, 0);
}

void
bench(int N, int R)
{
    cuex_t e = cuex_atree_empty();
    clock_t t_insert, t_erase;
    int i;
    for (i = 0; i < N; ++i)
	e = cuex_atree_insert(opn0word, e, cuex_o2_apply(cudyn_int(i), cuex_o0_null()));
    t_insert = -clock();
    for (i = 0; i < R; ++i)
	cuex_atree_insert(opn0word, e, cuex_o2_apply(cudyn_int(i + N), cuex_o0_null()));
    t_insert += clock();
    t_erase = -clock();
    for (i = 0; i < R; ++i)
	cuex_atree_erase(opn0word, e, (cu_word_t)cudyn_int(i % N));
    t_erase += clock();
    printf("%d %lg %lg\n", N,
	   t_insert/((double)CLOCKS_PER_SEC*R),
	   t_erase/((double)CLOCKS_PER_SEC*R));
}

int main()
{
    clock_t t_tot = -clock();
    double i, z = 1.0;
    cuex_init();
    for (i = 2.0; i < 40000; i *= 1.4142) {
	bench(i, 80000/z);
	z += 0.5;
    }
    t_tot += clock();
    fprintf(stderr, "Total time: %lg\n", t_tot/(double)CLOCKS_PER_SEC);
    return 0;
}
