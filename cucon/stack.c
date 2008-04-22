/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2003--2007  Petter Urkedal <urkedal@nbi.dk>
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
#include <cu/memory.h>
#include <cu/util.h>
#include <cu/diag.h>

#include <assert.h>
#include <string.h>
#include <stdio.h>

#define cuconP_STACK_CHUNK_SIZE 4096

void
cucon_stack_init(cucon_stack_t stack)
{
    stack->prev = NULL;
    stack->begin = stack->sp = stack->end = cuconP_STACK_NOADDRESS;
}

void
cucon_stack_init_copy(cucon_stack_t dst, cucon_stack_t src)
{
    char *sp_dst;
    cucon_stack_init(dst);
    sp_dst = cucon_stack_alloc(dst, cucon_stack_size(src));
    while (src) {
	memcpy(sp_dst, src->sp, src->end - src->sp);
	sp_dst += src->end - src->sp;
	src = src->prev;
    }
    cu_debug_assert(sp_dst == dst->end);
}

void *
cuconP_stack_expand(cucon_stack_t stack, size_t size)
{
    cucon_stack_t prev;
    size_t chunk_size;
    stack->sp += size;	/* Restore sp after shift in inline fn. */
    if (size > cuconP_STACK_CHUNK_SIZE)
	chunk_size = size;
    else
	chunk_size = cuconP_STACK_CHUNK_SIZE;
    prev = cu_galloc(CU_ALIGNED_SIZEOF(struct cucon_stack_s) + chunk_size);
    prev->prev = stack->prev;
    prev->begin = stack->begin;
    prev->sp = stack->sp;
    prev->end = stack->end;
    stack->prev = prev;
    stack->begin = (char *)(prev + 1);
    stack->end = (char *)stack->begin + chunk_size;
    stack->sp = (char *)stack->end - size;
    assert(stack->sp != cuconP_STACK_NOADDRESS);
    return stack->sp;
}

void
cuconP_stack_shrink(cucon_stack_t stack)
{
    if (cucon_stack_is_empty(stack))
	cu_bugf("cucon_stack_free called on an empty stack.");
    do {
	cucon_stack_t prev = stack->prev;
	size_t size = (char *)stack->sp - (char *)stack->end;
	if (!prev) {
	    if (size == 0)
		return;
	    else
		cu_bugf("cucon_stack_free more elements than on stack.");
	}
	stack->prev = prev->prev;
	stack->begin = prev->begin;
	stack->sp = prev->sp;
	stack->end = prev->end;
	stack->sp += size;
    } while (stack->sp >= stack->end);
}

void
cucon_stack_swap(cucon_stack_t stack0, cucon_stack_t stack1)
{
    struct cucon_stack_s tmp;
    memcpy(&tmp, stack0, sizeof(struct cucon_stack_s));
    memcpy(stack0, stack1, sizeof(struct cucon_stack_s));
    memcpy(stack1, &tmp, sizeof(struct cucon_stack_s));
}

size_t
cucon_stack_size(cucon_stack_t stack)
{
    size_t size = 0;
    do {
	size += stack->end - stack->sp;
	stack = stack->prev;
    } while (stack);
    return size;
}

void *
cucon_stack_at(cucon_stack_t stack, size_t pos)
{
    char *sp = stack->sp + pos;
    while (sp >= stack->end) {
	pos = (char *)sp - (char *)stack->end;
#ifndef CU_NDEBUG
	if (!stack->prev)
	    cu_bugf("cucon_stack_at requested to access element past the "
		     "stack.");
#endif
	stack = stack->prev;
	sp = (char*)stack->sp + pos;
    }
    return sp;
}

void *
cucon_stack_continuous_top(cucon_stack_t stack, size_t size)
{
    if (stack->end < stack->sp + size) {
	void *new_begin;
	void *new_end;
	void *sp;
	cucon_stack_t stack0 = stack, stack1;
	ptrdiff_t n = size - (stack->end - stack->sp);
	do {
	    stack0 = stack0->prev;
#ifndef CU_NDEBUG
	    if (stack0 == NULL)
		cu_bugf("cucon_stack_continuous_top requests more continuous"
			 " storage than the stack size.");
#endif
	    n -= stack0->end - stack0->sp;
	} while (n > 0);
	size -= n;
	new_begin = cu_galloc(size);
	new_end = new_begin + size;
	sp = new_begin;
	stack1 = stack;
	do {
	    n = stack1->end - stack1->sp;
	    memcpy(sp, stack1->sp, n);
	    sp += n;
	    stack1 = stack1->prev;
	} while (stack1 != stack0);
	memcpy(sp, stack1->sp, stack1->end - stack1->sp);
	stack->begin = new_begin;
	stack->sp = new_begin;
	stack->end = new_end;
	stack->prev = stack0->prev;
    }
    return stack->sp;
}

void
cuconP_stack_it_fix(cucon_stack_it_t *it)
{
    for (;;) {
	cucon_stack_t prev = it->stack->prev;
	size_t left = it->stack->end - it->sp;
#ifndef CU_NDEBUG
	if (!prev)
	    cu_bugf("cucon_stack_it_next moves the interator past the end "
		     "and more.");
#endif
	it->stack = prev;
	it->sp = prev->sp;
	if (left == 0)
	    break;
	it->sp += left;
    }
}

void
cucon_stack_unwind_to_mark(cucon_stack_t sk, cucon_stack_mark_t mark)
{
    while ((char *)mark < sk->begin || sk->end < (char *)mark) {
	cucon_stack_t prev = sk->prev;
	if (!prev)
	    cu_bugf("cucon_stack_unwind_to_mark can not find mark.");
	sk->prev = prev->prev;
	sk->begin = prev->begin;
	sk->sp = prev->sp;
	sk->end = prev->end;
    }
    sk->sp = (char *)mark;
}
