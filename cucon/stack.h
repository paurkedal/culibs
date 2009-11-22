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

#ifndef CUCON_STACK_H
#define CUCON_STACK_H

#include <cucon/fwd.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cucon_stack_h cucon/stack.h: Stacks of Memory
 * @{\ingroup cucon_linear_mod
 *
 * This is a stack in which you can push and pop objects of any size.
 * Memory will be allocated in chunks of fixed size or the size of the
 * object if it is bigger than the predefined size.  This implementation
 * is more low-level that the GNU obstacks.  It does not take care of
 * alignment, and it does not store object sizes on the stack.  On the
 * other hand, that makes it efficient.
 *
 * \see cucon_frame_h
 */
struct cucon_stack
{
    struct cucon_stack *prev;
    char *begin;	/* lowest address for current capacity */
    char *sp;		/* lowest address in use */
    char *end;		/* boundary to prev */
};
void *cuconP_stack_expand(cucon_stack_t stack, size_t size);
void cuconP_stack_shrink(cucon_stack_t stack);
#define cuconP_STACK_NOADDRESS NULL

/*!Construct \a stack as an empty stack. */
void cucon_stack_init(cucon_stack_t stack);

/*!Construct \a dst as a copy of \a src. */
void cucon_stack_init_copy(cucon_stack_t dst, cucon_stack_t src);

/*!True iff stack is empty. */
CU_SINLINE cu_bool_t cucon_stack_is_empty(cucon_stack_t stack)
{ return stack->begin == cuconP_STACK_NOADDRESS; }

/*!Swap the contents of \a stack0 and \a stack1. */
void cucon_stack_swap(cucon_stack_t stack0, cucon_stack_t stack1);

/*!Number of bytes on the stack. Linear time! If you only need to know if the
 * size is 0, use the faster \ref cucon_stack_is_empty. */
size_t cucon_stack_size(cucon_stack_t stack);

/*!Stack size in units of <tt>sizeof(void *)</tt>. If you only need to known
 * if the size is 0, use the faster \ref cucon_stack_is_empty. */
CU_SINLINE size_t cucon_stack_size_ptr(cucon_stack_t stack)
{ return cucon_stack_size(stack)/sizeof(void *); }

CU_SINLINE size_t cucon_stack_size_int(cucon_stack_t stack)
{ return cucon_stack_size(stack)/sizeof(int); }

/*!Returns a pointer to an object \a pos bytes down the stack. */
void *cucon_stack_at(cucon_stack_t stack, size_t pos);

CU_SINLINE void *cucon_stack_at_ptr(cucon_stack_t stack, size_t pos)
{ return *(void **)cucon_stack_at(stack, pos*sizeof(void *)); }

CU_SINLINE int cucon_stack_at_int(cucon_stack_t stack, size_t pos)
{ return *(int *)cucon_stack_at(stack, pos*sizeof(int)); }

/*!Returns a pointer to the top object on \a stack. */
CU_SINLINE void *cucon_stack_top(cucon_stack_t stack)
{ return stack->sp; }

/*!Returns the top object on \a stack, assuming it is a pointer. */
CU_SINLINE void *cucon_stack_top_ptr(cucon_stack_t stack)
{ return *(void **)cucon_stack_top(stack); }

CU_SINLINE int cucon_stack_top_int(cucon_stack_t stack)
{ return *(int *)cucon_stack_top(stack); }

/*!Make sure at least \a size bytes from the top of the stack is
 * continuous, reallocating if necessary, and return a pointer to the
 * top of the stack.  */
void *cucon_stack_continuous_top(cucon_stack_t stack, size_t size);

/*!Allocate \a size bytes of space on \a stack.  If you need alignment for
 * subsequent allocations, \a size must be padded to preserve alignment. */
CU_SINLINE void *
cucon_stack_alloc(cucon_stack_t stack, size_t size)
{
    char *sp = stack->sp -= size;
    if (sp - stack->begin < 0)
	return cuconP_stack_expand(stack, size);
    else
	return sp;
}

/*!Free \a size bytes off the top of \a stack. */
CU_SINLINE void
cucon_stack_free(cucon_stack_t stack, size_t size)
{
    if (size) {
	stack->sp += size;
	if (stack->sp - stack->end >= 0)
	    cuconP_stack_shrink(stack);
    }
}

/*!Free <tt>sizeof(void *)</tt> from \a stack. */
CU_SINLINE void cucon_stack_free_ptr(cucon_stack_t stack)
{ cucon_stack_free(stack, sizeof(void *)); }

/*!Push \a ptr onto the top of \a stack. */
CU_SINLINE void cucon_stack_push_ptr(cucon_stack_t stack, void *ptr)
{ *(void **)cucon_stack_alloc(stack, sizeof(void *)) = ptr; }

CU_SINLINE void cucon_stack_push_int(cucon_stack_t stack, int i)
{ *(int *)cucon_stack_alloc(stack, sizeof(int)) = i; }

/*!Pop and return a pointer from the top of \a stack. */
CU_SINLINE void *cucon_stack_pop_ptr(cucon_stack_t stack)
{
    void *p = cucon_stack_top_ptr(stack);
    cucon_stack_free(stack, sizeof(void *));
    return p;
}

CU_SINLINE int cucon_stack_pop_int(cucon_stack_t stack)
{
    int i = cucon_stack_top_int(stack);
    cucon_stack_free(stack, sizeof(int));
    return i;
}


/* == Marks == */

/*!An indicator of a level on a stack. */
typedef struct cucon_stack_mark *cucon_stack_mark_t;

/*!A mark which can be used with \ref cucon_stack_unwind_to_mark. Note that
 * marks don't survive operations which re-allocate the stack, such as \ref
 * cucon_stack_continuous_top. */
CU_SINLINE cucon_stack_mark_t cucon_stack_mark(cucon_stack_t stack)
{ return (cucon_stack_mark_t)stack->sp; }

/*!Unwind the stack to the state it had when \a mark was created from it.
 * \pre cucon_stack_continuous_top must not have been called since \a mark was
 * created. */
void cucon_stack_unwind_to_mark(cucon_stack_t sk, cucon_stack_mark_t mark);

/* == Iteration == */

/*!Stack iterator struct. */
struct cucon_stack_itr
{
    /* Turning to next fragment is eager, except that the last one is kept. */
    cucon_stack_t stack;
    char *sp;
};

/*!Initialise \a itr for iterating from top to bottom of \a stack. */
void cucon_stack_itr_init(cucon_stack_itr_t itr, cucon_stack_t stack);

/*!Get the next \a size bytes stack fragment referred by \a itr and increment
 * \a itr. */
void *cucon_stack_itr_get(cucon_stack_itr_t itr, size_t size);

/*!Get the next pointer at \a itr and increment \a itr by the pointer size. */
void *cucon_stack_itr_get_ptr(cucon_stack_itr_t itr);

void cucon_stack_itr_advance(cucon_stack_itr_t itr, size_t size);

/*!Assuming the elements of \a stack are pointers, returns a pointer source of
 * all the elements. */
cu_ptr_source_t cucon_stack_ptr_source(cucon_stack_t stack);


/* == Iterators == */

/*!\deprecated Use \ref cucon_stack_itr. */
typedef struct cucon_stack_it_s
{
    cucon_stack_t stack;
    char *sp;
} cucon_stack_it_t;

/*!\deprecated Use \ref cucon_stack_itr. */
CU_SINLINE cucon_stack_it_t
cucon_stack_begin(cucon_stack_t stack)
{
    cucon_stack_it_t it;
    it.stack = stack;
    it.sp = stack->sp;
    return it;
}

/*!\deprecated Use \ref cucon_stack_itr. */
CU_SINLINE cu_bool_t cucon_stack_it_is_end(cucon_stack_it_t it)
{ return it.sp == cuconP_STACK_NOADDRESS; }

void cuconP_stack_it_fix(cucon_stack_it_t *it);

/*!\deprecated Use \ref cucon_stack_itr. */
CU_SINLINE void
cucon_stack_it_advance(cucon_stack_it_t *it, size_t size)
{
    it->sp += size;
    if (it->sp >= it->stack->end)
	cuconP_stack_it_fix(it);
}

/*!\deprecated Use \ref cucon_stack_itr. */
CU_SINLINE void *cucon_stack_it_get(cucon_stack_it_t it) { return it.sp; }
/*!\deprecated Use \ref cucon_stack_itr. */
#define cucon_stack_it_get_ptr(it) CUCON_STACK_IT_GET(it, void *)


/* == Typed Data Macros == */

#define CUCON_STACK_SIZE(stack, elt_t) (cucon_stack_size(stack)/sizeof(elt_t))

/* Return the top object on the stack, assuming it has type 'elt_t'. */
#define CUCON_STACK_TOP(stack, elt_t) (*(elt_t*)cucon_stack_top(stack))

/* Allocate 'sizeof(elt_t)' bytes and store 'obj' there.  Observe the
 * note on alignment under 'cucon_stack_alloc'. */
#define CUCON_STACK_PUSH(stack, elt_t, obj) \
	((void)(*(elt_t*)cucon_stack_alloc((stack), sizeof(elt_t)) = (obj)))

/* Free 'sizeof(elt_t)' bytes. */
#define CUCON_STACK_POP(stack, elt_t) cucon_stack_free((stack), sizeof(elt_t))

/*!\deprecated Use \ref cucon_stack_itr. */
#define CUCON_STACK_IT_ADVANCE(it, elt_t) \
	cucon_stack_it_advance(it, sizeof(elt_t))

/*!\deprecated Use \ref cucon_stack_itr. */
#define CUCON_STACK_IT_GET(it, elt_t) (*(elt_t*)cucon_stack_it_get(it))

/*!@}*/
CU_END_DECLARATIONS

#endif
