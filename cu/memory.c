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


#define CU_DEBUG 1
#include <cu/conf.h>
#include <cu/memory.h>
#include <cu/debug.h>
#include <cu/tstate.h>
#include <cu/halloc.h>
#include <cu/util.h>

#include <gc/gc_mark.h>

#include <stdio.h>
#include <assert.h>

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

static cu_bool_t memory_init_done = 0;
#if 0
#ifdef CUCONF_ENABLE_GC_DISCLAIM
static int object_kind;
static void **object_freelist;
#endif
#endif

static void
out_of_memory(size_t size)
{
    fprintf(stderr, "Allocation of %ld bytes failed.\n", (long)size);
    abort();
}

static void (*cuP_raise_out_of_memory)(size_t size) = out_of_memory;

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

#ifdef CUCONF_DEBUG_MEMORY
void *
cu_gc_base(void *ptr)
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
	return ptr + cuD_gc_base_shift;
}

CU_SINLINE void
cuP_dbginfo_cct(cuP_dbginfo_t hdr, char const *file, int line)
{
    hdr->name = file;
    hdr->line = line;
    hdr->prev = NULL;
    hdr->next = NULL;
    hdr->cminfo_chain = NULL;
}
#endif

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

#ifdef CUCONF_DEBUG_MEMORY

void *cu_galloc_D(size_t size, char const *file, int line)
{
    void *p;
    p = GC_debug_malloc(size, file, line);
    if (p == NULL)
	cu_raise_out_of_memory(size);
}

void
cu_gfree_D(void *ptr, char const *file, int line)
{
    void *base;
    if (!ptr)
	return;
    base = cu_gc_base(ptr);
    if (base != ptr)
	cu_bugf("Invalid pointer passed to cu_gfree: %p (base = %p)",
		 ptr, base);
    GC_debug_free(ptr);
}

#ifndef CUCONF_HAVE_GC_MALLOC_ATOMIC_UNCOLLECTABLE
void
cu_gfree_au_D(void *ptr, char const *file, int line)
{
    void *base;
    if (!ptr)
	return;
    base = cu_gc_base(ptr);
    if (base != ptr)
	cu_bugf_fl(file, line,
		    "Invalid pointer passed to cu_gfree_au: %p (base = %p)",
		    ptr, base);
    free(ptr);
}
#endif

#endif /* CUCONF_DEBUG_MEMORY */



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

#ifdef CUCONF_DEBUG_MEMORY
struct cuP_debug_finaliser_cd_s
{
    GC_finalization_proc fn;
    void *cd;
};
static void
cuP_debug_finaliser(void *obj, void *cd)
{
    obj += CU_ALIGNED_SIZEOF(struct cuP_dbginfo_s);
    assert(obj == cu_gc_base(obj));
    (*((struct cuP_debug_finaliser_cd_s *)cd)->fn)(
	obj,
	((struct cuP_debug_finaliser_cd_s *)cd)->cd);
}
void
cuD_gc_register_finaliser(void *ptr, GC_finalization_proc fn, void *cd,
			       GC_finalization_proc *ofn, void **ocd)
{
#ifndef CU_NDEBUG
    if (ptr != cu_gc_base(ptr)) {
	cu_errf("Non-base pointer %p vs %p passed for registration "
		 "of finaliser.", ptr, cu_gc_base(ptr));
	cu_debug_abort();
    }
#endif
    GC_register_finalizer(ptr, fn, cd, ofn, ocd);
}
void
cuD_gc_register_finaliser_no_order(void *ptr,
				       GC_finalization_proc fn, void *cd,
				       GC_finalization_proc *ofn, void **ocd)
{
#ifndef CU_NDEBUG
    if (ptr != cu_gc_base(ptr)) {
	cu_errf("Non-base pointer %p vs %p passed for registration "
		 "of finaliser.", ptr, cu_gc_base(ptr));
	cu_debug_abort();
    }
#endif
    GC_register_finalizer_no_order(ptr, fn, cd, ofn, ocd);
}
#endif /* CUCONF_DEBUG_MEMORY */


/* Initialisation
 * ============== */

void
cuP_memory_init()
{
    void *ptr;

    cu_debug_assert(!memory_init_done);
    memory_init_done = 1;

    /* Determine size of debug header. */
    ptr = cu_galloc(1);
    cuD_gc_base_shift = (char*)ptr - (char*)GC_base(ptr);
    cu_gfree(ptr);
}
