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

#include <cucon/stack.h>
#include <assert.h>
#include <string.h>

int
main()
{
    void *ptr;
    int i;
    struct cucon_stack_s stack;
    struct cucon_stack_s stack_cp;

    cu_init();

    cucon_stack_cct(&stack);

    for (i = 2; i < 32; ++i) {
	int j;
	for (j = 1; j < i; ++j) {
	    char *s = cucon_stack_alloc(&stack, i);
	    *s = 1;
	    cucon_stack_free(&stack, i);
	}
    }

    for (i = 0; i < 7000; ++i)
	CUCON_STACK_PUSH(&stack, int, i);
    i = 7000;
    while (i > 5000) {
	--i;
	assert(CUCON_STACK_TOP(&stack, int) == i);
	CUCON_STACK_POP(&stack, int);
    }
    cucon_stack_continuous_top(&stack, sizeof(int)*4500);
    while (i > 0) {
	--i;
	assert(CUCON_STACK_TOP(&stack, int) == i);
	CUCON_STACK_POP(&stack, int);
    }
    assert(cucon_stack_is_empty(&stack));


    for (i = 0; i < 7000; ++i)
	CUCON_STACK_PUSH(&stack, int, i);
    ptr = cucon_stack_continuous_top(&stack, sizeof(int)*6000);
    memset(ptr, 0, sizeof(int)*6000);
    i = 6000;
    while (i > 0) {
	assert(CUCON_STACK_TOP(&stack, int) == 0);
	CUCON_STACK_POP(&stack, int);
	--i;
    }
    i = 1000;
    cucon_stack_cct_copy(&stack_cp, &stack);
    while (i > 0) {
	--i;
	assert(CUCON_STACK_TOP(&stack, int) == i);
	assert(CUCON_STACK_TOP(&stack_cp, int) == i);
	CUCON_STACK_POP(&stack, int);
	CUCON_STACK_POP(&stack_cp, int);
    }
    return 0;
}
