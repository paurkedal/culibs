/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2004--2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CU_TSTATE_H
#define CU_TSTATE_H

#include <cu/fwd.h>
#include <cu/conf.h>
#include <cu/threadlocal.h>
#ifdef CUCONF_HAVE_GC_GC_TINY_FL_H
#  include <gc/gc_tiny_fl.h>
#endif
#include <pthread.h>
#include <iconv.h>
#include <atomic_ops.h>

CU_BEGIN_DECLARATIONS
/** \file
 ** \internal */

#if defined(GC_TINY_FREELISTS) && defined(GC_GRANULE_BYTES)
#   define CU_GRAN_SIZE		GC_GRANULE_BYTES
#   define cuP_FL_CNT		GC_TINY_FREELISTS
#elif CUCONF_SIZEOF_VOID_P == 4
#   define CU_GRAN_SIZE 8
#   define cuP_FL_CNT 64
#elif CUCONF_SIZEOF_VOID_P == 8
#   define CU_GRAN_SIZE 16
#   define cuP_FL_CNT 48
#endif
#define CU_GRAN_SIZEW (CU_GRAN_SIZE/CU_WORD_SIZE)

typedef enum {
    cu_memkind_normal,
    cu_memkind_atomic,
    cu_memkind_uncoll,
    cu_memkind_atomic_uncoll,
    cu_memkind_object,
    cu_memkind_cnt
} cu_memkind_t;

typedef AO_t cu_rarex_t;

typedef struct cuP_tstate_s *cuP_tstate_t;
struct cuP_tstate_s
{
    cuP_tstate_t next;

    /* cu */
    struct cuP_thread_atexit_node_s *atexit_chain;
    void *ord_fl_arr[cuP_FL_CNT];
    void *unord_fl_arr[cuP_FL_CNT];
    cu_rarex_t *jammed_on_rarex;
    cu_bool_t jammed_on_write;

    /* cuflow */
    struct cuflowP_windstate_s *windstate;
    int priority;
    void *gflexq;

    /* cutext */
    iconv_t iconv_utf8_to_ucs4;
    iconv_t iconv_ucs4_to_utf8;
};

extern pthread_mutex_t cuP_global_mutex;
extern cuP_tstate_t cuP_tstate_chain;

CU_THREADLOCAL_DECL(cuP_tstate, cuP_tstate);
#define cuP_tstate_glck cuP_tstate

CU_END_DECLARATIONS

#endif
