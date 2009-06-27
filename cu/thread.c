/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2005--2007  Petter Urkedal <urkedal@nbi.dk>
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

#include <cu/thread.h>
#include <cu/tstate.h>
#include <cu/hook.h>
#include <cu/hash.h>
#include <cu/memory.h>

cu_mutex_t cuP_pmutex_arr[cuP_PMUTEX_CNT];

typedef struct _thread_data_s {
    void *(*cf)(void *);
    void *cd;
} *_thread_data_t;

static struct cu_iter_hook_s _thread_entry_hook;
static struct cu_iter_hook_s _thread_exit_hook;

static void *
_thread_start(void *subcd)
{
    if (!cu_iter_hook_is_empty(&_thread_exit_hook))
	cuP_tstate();
    cu_iter_hook_call(&_thread_entry_hook);
    return (*((_thread_data_t)subcd)->cf)(((_thread_data_t)subcd)->cd);
}

int
cu_thread_create(pthread_t *th_out, pthread_attr_t const *attrs,
		 void *(*cf)(void *), void *cd)
{
    _thread_data_t subcd = cu_gnew(struct _thread_data_s);
    subcd->cf = cf;
    subcd->cd = cd;
    return GC_pthread_create(th_out, attrs, _thread_start, subcd);
}

void
cu_register_thread_init(cu_clop0(on_entry, void), cu_clop0(on_exit, void))
{
    if (!cu_clop_is_null(on_entry)) {
	cu_call0(on_entry);
	cu_iter_hook_append(&_thread_entry_hook, on_entry);
    }
    if (!cu_clop_is_null(on_exit))
	cu_iter_hook_prepend(&_thread_exit_hook, on_exit);
}


typedef struct cuP_thread_atexit_node_s *cuP_thread_atexit_node_t;
struct cuP_thread_atexit_node_s
{
    cuP_thread_atexit_node_t next;
    cu_clop0(fn, void);
};

void
cu_thread_atexit(cu_clop0(fn, void))
{
    cuP_tstate_t tstate = cuP_tstate();
    cuP_thread_atexit_node_t node;
    node = cu_gnew_u(struct cuP_thread_atexit_node_s);
    node->fn = fn;
    node->next = tstate->atexit_chain;
    tstate->atexit_chain = node;
}

void
cuP_thread_run_atexit(cuP_tstate_t st)
{
    cuP_thread_atexit_node_t node, node_next;
    node = st->atexit_chain;
    while (node) {
	cu_call0(node->fn);
	node_next = node->next;
	cu_gfree_u(node);
	node = node_next;
    }
    cu_iter_hook_call(&_thread_exit_hook);
}


/* Mutex Locks
 * =========== */

void
cu_pmutex_lock(void *ptr)
{
    cu_mutex_lock(cu_pmutex_mutex(ptr));
}

cu_bool_t
cu_pmutex_trylock(void *ptr)
{
    return cu_mutex_trylock(cu_pmutex_mutex(ptr));
}

void
cuP_pmutex_unlock(void *ptr)
{
    cu_mutex_unlock(cu_pmutex_mutex(ptr));
}


/* Init
 * ==== */

static void
_thread_uninit(void)
{
    cu_iter_hook_call(&_thread_exit_hook);
}

void
cuP_thread_init(void)
{
    int i;

    cu_iter_hook_init(&_thread_entry_hook);
    cu_iter_hook_init(&_thread_exit_hook);
    atexit(_thread_uninit);

    for (i = 0; i < cuP_PMUTEX_CNT; ++i)
	cu_mutex_init(&cuP_pmutex_arr[i]);
}
