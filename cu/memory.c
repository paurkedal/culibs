/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2000--2010  Petter Urkedal <paurkedal@eideticdew.org>
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


#include <cu/conf.h>
#include <cu/memory.h>
#include <cu/debug.h>
#include <cu/tstate.h>
#include <cu/util.h>
#include <cu/diag.h>
#include <cu/ptr.h>

#include <gc/gc_mark.h>

#include <stdio.h>
#include <assert.h>
#include <string.h>

#if 0
#  include <gc/private/gc_priv.h>
#else
#  ifndef PTRFREE
#    define PTRFREE 0
#  endif
#  ifndef NORMAL
#    define NORMAL 1
#  endif
#  ifndef ATOMIC
#    define UNCOLLECTABLE 2
#  endif
#endif

static void
_out_of_memory(size_t size)
{
    fprintf(stderr, "Allocation of %ld bytes failed.\n", (long)size);
    abort();
}

static void (*cuP_raise_out_of_memory)(size_t size) = _out_of_memory;

void
cu_raise_out_of_memory(size_t size)
{
    (*cuP_raise_out_of_memory)(size);
    abort();
}

void
cu_regh_out_of_memory(void (*f)(size_t))
{
    cuP_raise_out_of_memory = f;
}


/* Garbage Collected Memory
 * ======================== */

static ptrdiff_t cuD_gc_base_shift = -1;

void *
cuD_gc_base(void *ptr)
{
    /* This compensates for the GC_debug_* allocators.  It isn't
     * needed at the moment, since we can't use these allocators with
     * the kind-general local alloc. */
    if (cuD_gc_base_shift == -1)
	cu_bugf("Library is not initialised.");
    ptr = GC_base(ptr);
    if (ptr == NULL)
	return ptr;
    else
	return cu_ptr_add(ptr, cuD_gc_base_shift);
}

#define DBG_MAX_ALLOC_SIZE 100000000

CU_SINLINE void
cuD_check_size(size_t size, char const *file, int line)
{
    if (size > DBG_MAX_ALLOC_SIZE) {
	cu_errf("Trying to allocate %d bytes, which is above debug limit.",
		 size);
	cu_debug_unreachable();
    }
}

void *
cuD_galloc(size_t size, char const *file, int line)
{
    void *p = GC_debug_malloc(size, file, line);
    if (p == NULL)
	cu_raise_out_of_memory(size);
    return p;
}

void *
cuD_galloc_atomic(size_t size, char const *file, int line)
{
    void *p = GC_debug_malloc_atomic(size, file, line);
    if (p == NULL)
	cu_raise_out_of_memory(size);
    return p;
}

void *
cuD_gallocz_atomic(size_t size, char const *file, int line)
{
    void *p = GC_debug_malloc_atomic(size, file, line);
    if (p == NULL)
	cu_raise_out_of_memory(size);
    memset(p, 0, size);
    return p;
}

void *
cuD_ualloc(size_t size, char const *file, int line)
{
    void *p = GC_debug_malloc_uncollectable(size, file, line);
    if (p == NULL)
	cu_raise_out_of_memory(size);
    return p;
}

void *
cuD_ualloc_atomic(size_t size, char const *file, int line)
{
#ifdef CUCONF_HAVE_GC_MALLOC_ATOMIC_UNCOLLECTABLE
    void *p = GC_malloc_atomic_uncollectable(size);
#else
    void *p = malloc(size);
#endif
    if (p == NULL)
	cu_raise_out_of_memory(size);
    return p;
}

void *
cuD_uallocz_atomic(size_t size, char const *file, int line)
{
#ifdef CUCONF_HAVE_GC_MALLOC_ATOMIC_UNCOLLECTABLE
    void *p = GC_malloc_atomic_uncollectable(size);
#else
    void *p = malloc(size);
#endif
    if (p == NULL)
	cu_raise_out_of_memory(size);
    memset(p, 0, size);
    return p;
}

void
cuD_gfree(void *ptr, char const *file, int line)
{
    void *base;
    if (!ptr)
	return;
    base = cuD_gc_base(ptr);
    if (base != ptr)
	cu_bugf("Invalid pointer passed to cu_gfree: %p (base = %p)",
		ptr, base);
    GC_debug_free(ptr);
}

void
cuD_ufree_atomic(void *ptr, char const *file, int line)
{
#ifdef CUCONF_HAVE_GC_MALLOC_ATOMIC_UNCOLLECTABLE
    GC_free(ptr);
#else
    free(ptr);
#endif
}

void *
(cu_gallocz_atomic)(size_t size)
{
    void *p = GC_malloc_atomic(size);
    if (!p) cu_raise_out_of_memory(size);
    memset(p, 0, size);
    return p;
}

void *
(cu_uallocz_atomic)(size_t size)
{
#ifdef CUCONF_HAVE_GC_MALLOC_ATOMIC_UNCOLLECTABLE
    void *p = GC_malloc_atomic_uncollectable(size);
#else
    void *p = malloc(size);
#endif
    if (!p) cu_raise_out_of_memory(size);
    memset(p, 0, size);
    return p;
}


/* Other GC facilities
 * ------------------- */

void *
cuP_weakptr_get_locked(void *link)
{
    if (*(cu_hidden_ptr_t *)link)
	return cu_reveal_ptr(*(cu_hidden_ptr_t *)link);
    else
	return NULL;
}


/* Initialisation
 * ============== */

void
cuP_memory_init()
{
    void *ptr;

    cu_debug_assert_once();

    /* Determine size of debug header. */
    ptr = cuD_galloc(1, __FILE__, __LINE__);
    cuD_gc_base_shift = (char *)ptr - (char *)GC_base(ptr);
    cuD_gfree(ptr, __FILE__, __LINE__);
}
