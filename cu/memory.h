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

/* By default, follow CU_NDEBUG, but let client explicitly switch on or off
 * memory debugging by defining CU_DEBUG_MEMORY or CU_NDEBUG_MEMORY,
 * respectively. */
#if !defined(CU_NDEBUG) && !defined(CU_NDEBUG_MEMORY)
#  define CU_DEBUG_MEMORY
#endif


CU_BEGIN_DECLARATIONS

#ifdef CUCONF_HAVE_STDINT_H
typedef uintptr_t cu_uintptr_t;
#else
typedef unsigned long cu_uintptr_t;
#endif
void cu_raise_out_of_memory(size_t size) CU_ATTR_NORETURN;
void cu_regh_out_of_memory(void (*f)(size_t));

/** \defgroup cu_memory cu/memory.h: Memory Allocation
 ** @{ \ingroup cu_base_mod */

/** \name Stack Allocation
 ** @{ */

/** Allocate memory on the stack.  (This is just an alias for alloca used in
 ** the library for quick replacement when debugging stack-related issues.) */
#define		cu_salloc(size) (alloca(size))

/** The counterpart of \ref cu_salloc, which is a noop. */
#define		cu_sfree(p) ((void)(p))

/** @}
 ** \name Dynamic Memory Allocation
 ** @{
 **
 ** Most of these functions call the Boehm-Demers-Weiser conservative garbage
 ** collector to do the real work, but also asserts that the result is non-\c
 ** NULL.
 **/

#if defined(CU_DEBUG_MEMORY) && !defined(CU_IN_DOXYGEN)

void *cuD_galloc(size_t size, char const *file, int line);
void *cuD_galloc_atomic(size_t size, char const *file, int line);
void *cuD_gallocz_atomic(size_t size, char const *file, int line);

void *cuD_ualloc(size_t size, char const *file, int line);
void *cuD_ualloc_atomic(size_t size, char const *file, int line);
void *cuD_uallocz_atomic(size_t size, char const *file, int line);

#define cu_galloc(size)		cuD_galloc(size, __FILE__, __LINE__)
#define cu_galloc_atomic(size)	cuD_galloc_atomic(size, __FILE__, __LINE__)
#define cu_gallocz(size)	cuD_galloc(size, __FILE__, __LINE__)
#define cu_gallocz_atomic(size)	cuD_gallocz_atomic(size, __FILE__, __LINE__)

#define cu_ualloc(size)		cuD_ualloc(size, __FILE__, __LINE__)
#define cu_ualloc_atomic(size)	cuD_ualloc_atomic(size, __FILE__, __LINE__)
#define cu_uallocz(size)	cuD_ualloc(size, __FILE__, __LINE__)
#define cu_uallocz_atomic(size)	cuD_uallocz_atomic(size, __FILE__, __LINE__)

#else /* !CU_DEBUG_MEMORY */

/** Returns \a size bytes of traced an collectable memory.
 ** \c GC_malloc with a check for \c NULL. */
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

/** Allocate a memory region of \a size bytes of untraced but collectable
 ** memory.  \c GC_malloc_atomic with a check for \c NULL. */
CU_SINLINE void *
cu_galloc_atomic(size_t size)
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

/** Allocate cleared traceable collectable memory. */
CU_SINLINE void *cu_gallocz(size_t size) { return cu_galloc(size); }

/** Allocate cleared atomic collectable memory. */
void *cu_gallocz_atomic(size_t size);

/** Returns \a size bytes of traced but uncollectable memory.
 ** \c GC_malloc_uncollectable with a check for \c NULL. */
CU_SINLINE void *
cu_ualloc(size_t size)
{
    void *p;
    p = GC_malloc_uncollectable(size);
    if (p == NULL)
	cu_raise_out_of_memory(size);
    return p;
}

/** Returns \a size bytes of untraced and uncollectable memory. May be
 ** implemented as \c GC_malloc_atomic_uncollectable, if available or \c
 ** malloc, so use \c cu_ufree_atomic to release it. */
CU_SINLINE void *
cu_ualloc_atomic(size_t size)
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

/** Allocate cleared traceable uncollectable memory. */
CU_SINLINE void *cu_uallocz(size_t size) { return cu_ualloc(size); }

/** Allocate cleared atomic uncollectable memory. */
void *cu_uallocz_atomic(size_t size);

#endif /* CU_DEBUG_MEMORY */


#if defined(CU_DEBUG_MEMORY) && !defined(CU_IN_DOXYGEN)

void cuD_gfree(void *ptr, char const *file, int line);
void cuD_ufree_atomic(void *ptr, char const *file, int line);
#  define cu_gfree(ptr)		cuD_gfree(ptr, __FILE__, __LINE__)
#  define cu_gfree_atomic	cu_gfree
#  define cu_ufree		cu_gfree
#  define cu_ufree_atomic(ptr)	cuD_ufree_atomic(ptr, __FILE__, __LINE__)

#else /* !CU_DEBUG_MEMORY */

/** Free traceable collectable memory (optional). */
#  define cu_gfree GC_free

/** Free atomic collectable memory (optional). */
#  define cu_gfree_atomic cu_gfree

/** Free traceable uncollectable memory. */
#  define cu_ufree cu_gfree

/** Free atomic uncollectable memory. */
#  ifdef CUCONF_HAVE_GC_MALLOC_ATOMIC_UNCOLLECTABLE
#    define cu_ufree_atomic cu_gfree
#  else
#    define cu_ufree_atomic free
#  endif

#endif

/** @}
 ** \name Typed Memory Allocation Macros
 ** @{
 **
 ** These are straight-forward convenience macros which passes the size of the
 ** given type to the corresponding allocation functions, and cast the result
 ** to the given type. */

/** Shortcut to allocate an object of type \a type using \ref cu_salloc. */
#define cu_snew(type, n)	((type *)cu_salloc(sizeof(type)))

/** Shortcut to allocate an object of type \a type using \ref cu_galloc. */
#define	cu_gnew(type) ((type*)cu_galloc(sizeof(type)))

/** Shortcut to allocate an object of type \a type using \ref cu_galloc_atomic. */
#define cu_gnew_atomic(type) ((type *)cu_galloc_atomic(sizeof(type)))

/** Shortcut to allocate an object of type \a type using \ref cu_ualloc. */
#define cu_unew(type) ((type *)cu_ualloc(sizeof(type)))

/** Shortcut to allocate an object of type \a type using \ref cu_ualloc_atomic. */
#define cu_unew_atomic(type) ((type *)cu_ualloc_atomic(sizeof(type)))

#define cu_gnewz(type)		((type *)cu_gallocz(sizeof(type)))
#define cu_gnewz_atomic(type)	((type *)cu_gallocz_atomic(sizeof(type)))
#define cu_unewz(type)		((type *)cu_uallocz(sizeof(type)))
#define cu_unewz_atomic(type)	((type *)cu_uallocz_atomic(sizeof(type)))

#define cu_snewarr(type, n)	   ((type *)cu_salloc(sizeof(type)*(n)))
#define cu_gnewarr(type, n)        ((type *)cu_galloc(sizeof(type)*(n)))
#define cu_gnewarr_atomic(type, n) ((type *)cu_galloc_atomic(sizeof(type)*(n)))
#define cu_unewarr(type, n)        ((type *)cu_ualloc(sizeof(type)*(n)))
#define cu_unewarr_atomic(type, n) ((type *)cu_ualloc_atomic(sizeof(type)*(n)))

#define cu_gnewarrz(type, n)        ((type *)cu_gallocz(sizeof(type)*(n)))
#define cu_gnewarrz_atomic(type, n) ((type *)cu_gallocz_atomic(sizeof(type)*(n)))
#define cu_unewarrz(type, n)        ((type *)cu_uallocz(sizeof(type)*(n)))
#define cu_unewarrz_atomic(type, n) ((type *)cu_uallocz_atomic(sizeof(type)*(n)))

/** @}
 ** \name Supplementary Definitions
 ** @{ */

/** Notify that the pointer \a ptr_lvalue in traceable memory is no longer
 ** needed.  It will be left as a tuning option whether to zero the pointer or
 ** not.  Clients may want to use their own analogous macro for local tuning. */
#define CU_GWIPE(ptr_lvalue) ((ptr_lvalue) = NULL)

#define CU_GCLEAR_INT(lvalue) (0? (void)((lvalue) = 0) : (void)0)
#define CU_GCLEAR_PTR(lvalue) (0? (void)((lvalue) = NULL) : (void)0)

/** A type which represents a pointer that is hidden to the garbage
 ** collector. */
typedef struct cu_hidden_ptr *cu_hidden_ptr_t;

/** Hide a pointer from the garbage collector. */
#define cu_hide_ptr(ptr) ((cu_hidden_ptr_t)~(cu_uintptr_t)(ptr))

/** Reveal a pointer that was hidden with \c cu_hide_ptr.  If you don't know
 ** how determine if the pointer is still valid, don't use this. */
#define cu_reveal_ptr(hptr) \
	((void*)~(cu_uintptr_t)CU_MARG(cu_hidden_ptr_t, hptr))

#ifdef CU_DEBUG_MEMORY
void *cuD_gc_base(void *);
#  define cu_gc_base cuD_gc_base
#else
#  define cu_gc_base GC_base
#endif
/* These used to be distinct from libgc due to additional debug headers. */
#define cu_gc_register_finaliser GC_register_finalizer
#define cu_gc_register_finaliser_no_order GC_register_finalizer_no_order

#ifndef CU_DEBUG_MEMORY
#  define cu_gc_ptr_assign(p, q) (*(p) = (q))
#else
#  define cu_gc_ptr_assign(p, q) \
	(1 ? *((void**)GC_is_visible(p)) = GC_is_valid_displacement(q) \
	   : *(p) = (q))
#endif

/** @}
 ** @} */

#if defined(CU_COMPAT) && CU_COMPAT < 20091115
#  define cu_malloc	malloc
#  define cu_mfree	free
#  define cu_galloc_a	cu_galloc_atomic
#  define cu_galloc_u	cu_ualloc
#  define cu_galloc_au	cu_ualloc_atomic
#  define cu_cgalloc	cu_gallocz
#  define cu_cgalloc_a	cu_gallocz_atomic
#  define cu_cgalloc_u	cu_uallocz
#  define cu_gfree_a	cu_gfree_atomic
#  define cu_gfree_u	cu_ufree
#  define cu_gfree_au	cu_ufree_atomic
#  define cu_gnew_a	cu_gnew_atomic
#  define cu_gnew_u	cu_unew
#  define cu_gnew_au	cu_unew_atomic
#  define cu_cgnew	cu_gnewz
#  define cu_gnewarr_a	cu_gnewarr_atomic
#  define cu_gnewarr_u	cu_unewarr
#  define cu_gnewarr_au	cu_unewarr_atomic
#endif

CU_END_DECLARATIONS

#endif
