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

cu_dlog_def(_file, "dtag=cu.thread");

cu_mutex_t cuP_pmutex_arr[cuP_PMUTEX_CNT];

typedef struct _thread_data {
    void *(*cf)(void *);
    void *cd;
} *_thread_data_t;

typedef struct cuP_thread_atexit_node *_thread_atexit_node_t;
struct cuP_thread_atexit_node
{
    _thread_atexit_node_t next;
    cu_clop0(fn, void);
};

static struct cu_iter_hook _thread_entry_hook;
static struct cu_iter_hook _thread_exit_hook;

#ifdef CUCONF_HAVE_THREAD_KEYWORD
static __thread cu_bool_t _thread_init_done = cu_false;
#endif
static pthread_key_t _thread_init_key;


static void
_thread_cleanup(void *null)
{
    cuP_tstate_t tls = cuP_tstate();
    _thread_atexit_node_t node, node_next;

    node = tls->atexit_chain;
    while (node) {
	cu_call0(node->fn);
	node_next = node->next;
	cu_ufree(node);
	node = node_next;
    }
    cu_iter_hook_call(&_thread_exit_hook);
}

static void *
_thread_start(void *subcd)
{
    void *ret;
#ifdef CUCONF_HAVE_THREAD_KEYWORD
    cu_debug_assert(!_thread_init_done);
    _thread_init_done = cu_true;
#endif
    cu_dlogf(_file, "Running thread init hook from cu_pthread_create.");
    cu_iter_hook_call(&_thread_entry_hook);
    pthread_cleanup_push(_thread_cleanup, NULL);
    ret = (*((_thread_data_t)subcd)->cf)(((_thread_data_t)subcd)->cd);
    pthread_cleanup_pop(1);
    return ret;
}

int
cu_pthread_create(pthread_t *th_out, pthread_attr_t const *attrs,
		  void *(*cf)(void *), void *cd)
{
    _thread_data_t subcd = cu_gnew(struct _thread_data);
    subcd->cf = cf;
    subcd->cd = cd;
    return GC_pthread_create(th_out, attrs, _thread_start, subcd);
}

void
cu_thread_init(void)
{
#ifdef CUCONF_HAVE_THREAD_KEYWORD
    if (_thread_init_done)
	return;
    _thread_init_done = cu_true;
#else
    if (cuP_tstate() != NULL)
	return;
#endif
    cu_dlogf(_file, "Running thread init hook from cu_thread_init.");
    cu_iter_hook_call(&_thread_entry_hook);
    pthread_setspecific(_thread_init_key, &_thread_init_key);
}

void
cu_assert_thread_init(void)
{
#ifdef CUCONF_HAVE_THREAD_KEYWORD
    if (!_thread_init_done)
#else
    if (cuP_tstate() == NULL)
#endif
	cu_bugf("Initaliasation has not not been called for this thread.  "
		"This can be fixed by using cu_pthread_create or calling "
		"cu_thread_init at a suitable place before this point.");
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

void
cu_thread_atexit(cu_clop0(fn, void))
{
    cuP_tstate_t tstate = cuP_tstate();
    _thread_atexit_node_t node;
    node = cu_unew(struct cuP_thread_atexit_node);
    node->fn = fn;
    node->next = tstate->atexit_chain;
    tstate->atexit_chain = node;
}

void
cu_pthread_key_create(pthread_key_t *key_out, void (*destructor)(void *))
{
    int err = pthread_key_create(key_out, destructor);
    if (err != 0)
	cu_handle_syserror(err, "pthread_key_create");
}

void
cu_pthread_setspecific(pthread_key_t key, void *data)
{
    int err = pthread_setspecific(key, data);
    if (err != 0)
	cu_handle_syserror(err, "pthread_setspecific");
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
_thread_atexit(void)
{
    _thread_cleanup(NULL);
}

void
cuP_thread_init(void)
{
    int i;

    cu_pthread_key_create(&_thread_init_key, _thread_cleanup);

    cu_iter_hook_init(&_thread_entry_hook);
    cu_iter_hook_init(&_thread_exit_hook);
    atexit(_thread_atexit);

    for (i = 0; i < cuP_PMUTEX_CNT; ++i)
	cu_mutex_init(&cuP_pmutex_arr[i]);
}
