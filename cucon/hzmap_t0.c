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

#include <cucon/hzmap.h>
#include <cu/inherit.h>
#include <cu/test.h>
#include <stdio.h>
#include <time.h>

#define SQR_N_INS 100
#define N_INS (SQR_N_INS*SQR_N_INS)
#define N_REP 800

typedef struct mynode_s *mynode_t;
struct mynode_s
{
    cu_inherit (cucon_hzmap_node_s);
    cu_word_t key[2];
    cu_word_t sum;
};

size_t  bench_n_insert = 0, bench_n_erase = 0, bench_n_find = 0;
clock_t bench_t_insert = 0, bench_t_erase = 0, bench_t_find = 0;

void
test(cucon_hzmap_t map, int verb)
{
    size_t i, j, n = 0;
    cucon_hzmap_init(map, 2);

    bench_n_insert += N_INS;
    bench_t_insert -= clock();
    for (i = 0; i < N_INS; ++i) {
	cucon_hzmap_node_t node;
	cu_word_t key[2];
	key[0] = (cu_word_t)mrand48() % SQR_N_INS;
	key[1] = (cu_word_t)mrand48() % SQR_N_INS;
	if (cucon_hzmap_insert(map, key, sizeof(struct mynode_s), &node)) {
#define node ((mynode_t)node)
	    node->sum = node->key[0] + node->key[1];
#undef node
	    ++n;
	}
    }
    bench_t_insert += clock();
    if (verb)
	printf("Inserted %d unique of %d random pairs, cap = %d.\n", n, N_INS,
	       cucon_hzmap_capacity(map));
    cu_test_assert_size_eq(cucon_hzmap_size(map), n);

    bench_n_find += N_INS;
    bench_t_find -= clock();
    for (i = 0; i < SQR_N_INS; ++i)
    for (j = 0; j < SQR_N_INS; ++j) {
	cu_word_t key[2] = {i, j};
	cucon_hzmap_node_t node;
	node = cucon_hzmap_2w_find(map, key);
	if (node) {
	    --n;
#define node ((mynode_t)node)
	    cu_test_assert_size_eq(node->key[0] + node->key[1], node->sum);
#undef node
	}
    }
    bench_t_find += clock();
    cu_test_assert_size_eq(n, 0);

    n = cucon_hzmap_size(map);
    bench_n_erase += 4*N_INS;
    bench_t_erase -= clock();
    for (i = 0; i < 4*N_INS; ++i) {
	cu_word_t key[2];
	key[0] = (cu_word_t)mrand48() % SQR_N_INS;
	key[1] = (cu_word_t)mrand48() % SQR_N_INS;
	if (cucon_hzmap_erase(map, key))
	    --n;
    }
    bench_t_erase += clock();
    if (verb)
	printf("Map shrunk to %d pairs, cap = %d.\n", n, cucon_hzmap_capacity(map));
    cu_test_assert_size_eq(n, cucon_hzmap_size(map));
}

int
main()
{
    int i;
    struct cucon_hzmap_s map;
    cu_init();
    for (i = 0; i < N_REP; ++i)
	test(&map, 0);
#define AVG(f) bench_t_##f/((double)CLOCKS_PER_SEC*bench_n_##f)
    printf("  find: %g\n"
	   "insert: %g\n"
	   " erase: %g\n",
	   AVG(find),
	   AVG(insert),
	   AVG(erase));
    return 2*!!cu_test_bug_count();
}
