/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2000--2007  Petter Urkedal <urkedal@nbi.dk>
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


#ifndef CU_MEMORY_H
#define CU_MEMORY_H

#include <stdlib.h>
#include <cu/fwd.h>
#include <cu/tstate.h>
#include <cu/conf.h>
#include <cu/debug.h>
#if CUCONF_HAVE_GC_GC_H
#  include <gc/gc.h>
#else
#  include <gc.h>
#endif
#ifdef CUCONF_HAVE_GC_LOCAL_MALLOC
#  ifdef CUCONF_HAVE_GC_GC_LOCAL_ALLOC_H
#    include <gc/gc_local_alloc.h>
#  elif defined(CUCONF_HAVE_GC_LOCAL_ALLOC_H)
#    include <gc_local_alloc.h>
#  endif
#endif
#ifdef CUCONF_HAVE_STDINT_H
#include <stdint.h>
#endif

#ifdef CU_NDEBUG
#  define CU_NDEBUG_MEMORY
#endif


CU_BEGIN_DECLARATIONS
/*!\defgroup cu_memory cu/memory.h: Memory Allocation
 * @{ \ingroup cu_mod */

#ifdef CUCONF_HAVE_STDINT_H
typedef uintptr_t cu_uintptr_t;
#else
typedef unsigned long cu_uintptr_t;
#endif

void cu_raise_out_of_memory(size_t size) CU_ATTR_NORETURN;
void cu_regh_out_of_memory(void (*f)(size_t));


/* Plain Dynamic Memory
 * ==================== */

#define		cu_malloc(size)	(malloc(size))
#define		cu_mfree(p)		free(p)


/* Stack-bound Memory
 * ================== */

/*!Allocate memory on the stack.  (This is just an alias for alloca used in
 * the library for quick replacement when debugging stack-related issues.) */
#define		cu_salloc(size) (alloca(size))
#define		cu_sfree(p) ((void)(p))


/* Garbage Collected Memory
 * ========================
 *
 * The following functions calls the Boehm-Demers-Weiser conservative
 * garbage collector functions, and adds an additional test to assert
 * non-NULL result.  It also adds more debugging info when not
 * switched off.  Another reason for this API indirection is to make it
 * simpler to plug in a different collector. */


/* alloc, avail and free
 * --------------------- */

#ifdef CUCONF_DEBUG_MEMORY

void *cu_galloc_D(size_t size, char const *file, int line);
void *cu_galloc_a_D(size_t size, char const *file, int line);
void *cu_galloc_u_D(size_t size, char const *file, int line);
void *cu_galloc_au_D(size_t size, char const *file, int line);
void cu_gfree_D(void *ptr, char const *file, int line);
void cu_gfree_au_D(void *ptr, char const *file, int line);
#define cu_galloc(size) cu_galloc_D(size, __FILE__, __LINE__)
#define cu_galloc_a(size) cu_galloc_a_D(size, __FILE__, __LINE__)
#define cu_galloc_u(size) cu_galloc_u_D(size, __FILE__, __LINE__)
#define cu_galloc_au(size) cu_galloc_au_D(size, __FILE__, __LINE__)
#define cu_gfree(ptr) cu_gfree_D(ptr, __FILE__, __LINE__)
#define cu_gfree_au(ptr) cu_gfree_au_D(ptr, __FILE__, __LINE__)

#else /* !CUCONF_DEBUG_MEMORY */

/*!Returns \a size bytes of traced an collectable memory.
 * \c GC_malloc with a check for \c NULL. */
CU_SINLINE void *
cu_galloc(size_t size)
{
    void *p;
#ifdef CUCONF_HAVE_GC_LOCAL_MALLOC
    p = GC_local_malloc(size);
#else
    p = GC_malloc(size);
#endif
    if (p == NULL)
	cu_raise_out_of_memory(size);
    return p;
}

/*!Allocate a memory region of \a size bytes of untraced but collectable
 * memory.  \c GC_malloc_atomic with a check for \c NULL. */
CU_SINLINE void *
cu_galloc_a(size_t size)
{
    void *p;
#ifdef CUCONF_HAVE_GC_LOCAL_MALLOC_ATOMIC
    p = GC_local_malloc_atomic(size);
#else
    p = GC_malloc_atomic(size);
#endif
    if (p == NULL)
	cu_raise_out_of_memory(size);
    return p;
}

/*!Returns \a size bytes of traced but uncollectable memory.
 * \c GC_malloc_uncollectable with a check for \c NULL. */
CU_SINLINE void *
cu_galloc_u(size_t size)
{
    void *p;
    p = GC_malloc_uncollectable(size);
    if (p == NULL)
	cu_raise_out_of_memory(size);
    return p;
}

/*!Returns \a size bytes of untraced and uncollectable memory. May
 * be implemented as \c GC_malloc_atomic_uncollectable, if available
 * or \c malloc, so use \c cu_gfree_au to release it. */
CU_SINLINE void *
cu_galloc_au(size_t size)
{
    void *p;
#ifdef CUCONF_HAVE_GC_MALLOC_ATOMIC_UNCOLLECTABLE
    p = GC_malloc_atomic_uncollectable(size);
#else
    p = malloc(size);
#endif
    if (p == NULL)
	cu_raise_out_of_memory(size);
    return p;
}

/*!Allocate cleared collected memory. */
#define cu_cgalloc cu_galloc

/*!Allocate cleared atomic collected memory. */
#define cu_cgalloc_a cu_galloc_a

/*!Allocate cleared traced but uncollected memory. */
#define cu_cgalloc_u cu_galloc_u

/*!Frees memory allocated with \c cu_galloc, \c cu_galloc_a, or
 * \c cu_galloc_u, but only the latter really needs to be freed
 * explicitely. */
#define cu_gfree GC_free
#ifdef CUCONF_HAVE_GC_MALLOC_ATOMIC_UNCOLLECTABLE
/*!Frees memory allocated with \c cu_galloc_au. */
#  define cu_gfree_au GC_free
#else
#  define cu_gfree_au free
#endif

#endif /* CUCONF_DEBUG_MEMORY */

#define cu_gfree_u cu_gfree
#define cu_gfree_a cu_gfree

/*!Shortcut to allocate an object of type \a type using \ref cu_galloc. */
#define	cu_gnew(type) ((type*)cu_galloc(sizeof(type)))

/*!Shortcut to allocate an object of type \a type using \ref cu_galloc_a. */
#define cu_gnew_a(type) ((type *)cu_galloc_a(sizeof(type)))

/*!Shortcut to allocate an object of type \a type using \ref cu_galloc_u. */
#define cu_gnew_u(type) ((type *)cu_galloc_u(sizeof(type)))

/*!Shortcut to allocate an object of type \a type using \ref cu_galloc_au. */
#define cu_gnew_au(type) ((type *)cu_galloc_au(sizeof(type)))

/*!A shortcut to allocate an cleared object of type \a type using \ref
 * cu_cgalloc. */
#define cu_cgnew(type) ((type *)cu_cgalloc(sizeof(type)))


/* Other GC facilities
 * ------------------- */

/*!A type which represents a pointer that is hidden to the garbage
 * collector. */
typedef struct cu_hidden_ptr_s *cu_hidden_ptr_t;

/*!Hide a pointer from the garbage collector. */
#define cu_hide_ptr(ptr) ((cu_hidden_ptr_t)~(cu_uintptr_t)(ptr))
/*!Reveal a pointer that was hidden with \c cu_hide_ptr.  If you don't
 * know how determine if the pointer is still valid, don't use this. */
#define cu_reveal_ptr(hptr) \
	((void*)~(cu_uintptr_t)CU_MARG(cu_hidden_ptr_t, hptr))

#ifdef CUCONF_DEBUG_MEMORY
void cuD_gc_register_finaliser(void *ptr, GC_finalization_proc, void *,
				    GC_finalization_proc *, void **);
void cuD_gc_register_finaliser_no_order(void *ptr, GC_finalization_proc,
					     void *, GC_finalization_proc *,
					     void**);
void cu_gc_base(void *);
#else
#define cu_gc_register_finaliser GC_register_finalizer
#define cu_gc_register_finaliser_no_order GC_register_finalizer_no_order
#define cu_gc_base GC_base
#endif
void *cu_gc_base_D(void *);

#ifdef CU_NDEBUG_MEMORY
#  define cu_gc_ptr_assign(p, q) (*(p) = (q))
#else
#  define cu_gc_ptr_assign(p, q) \
	(1 ? *((void**)GC_is_visible(p)) = GC_is_valid_displacement(q) \
	   : *(p) = (q))
#endif

/*!@}*/
CU_END_DECLARATIONS

#endif
