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

#include <cuflow/wind.h>
#include <cu/conf.h>
#ifdef CUCONF_HAVE_GC_GC_H
#  include <gc/gc.h>
#else
#  include <gc.h>
#endif
#include <libunwind.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#ifdef CUCONF_ENABLE_THREADS
#  include <pthread.h>
#endif


/* A Stripped Down and Adapted chi/pmap.h
 * ====================================== */

typedef struct fni_map_s *fni_map_t;
struct fni_map_s
{
    size_t size; /* the number of elements in the map. */
    size_t mask; /* = capacity - 1 */
    cuflowP_fninfo_t *arr;
};
#define MIN_SIZE 8
#define MAX_FILL_NOM 2
#define MAX_FILL_DENOM 3
#define HASH(key) (((key) ^ ((key) >> 7)) - (((key) >> 11) ^ ((key) >> 19)))

static void
fni_map_set_capacity(fni_map_t pmap, size_t new_capacity)
{
    size_t i;
    cuflowP_fninfo_t *new_arr;
    cuflowP_fninfo_t *old_arr = pmap->arr;
    size_t old_capacity = pmap->mask + 1;
    if (old_capacity == new_capacity)
	return;
    new_arr = malloc(sizeof(cuflowP_fninfo_t)*new_capacity);
    pmap->mask = new_capacity - 1;
    pmap->arr = new_arr;
    memset(new_arr, 0, sizeof(cuflowP_fninfo_t)*new_capacity);
    for (i = 0; i < old_capacity; ++i) {
	cuflowP_fninfo_t node = old_arr[i];
	while (node) {
	    cuflowP_fninfo_t next = node->next;
	    unw_word_t hash = HASH(node->key) & pmap->mask;
	    node->next = new_arr[hash];
	    new_arr[hash] = node;
	    node = next;
	}
    }
    free(old_arr);
}
static void
fni_map_cct(fni_map_t pmap)
{
    pmap->arr = malloc(sizeof(cuflowP_fninfo_t)*MIN_SIZE);
    pmap->mask = MIN_SIZE - 1;
    pmap->size = 0;
    memset(pmap->arr, 0, MIN_SIZE*sizeof(cuflowP_fninfo_t));
}
static void
fni_map_dct(fni_map_t pmap)
{
    free(pmap->arr);
}
static void
fni_map_insert(fni_map_t pmap, unw_word_t key, cuflowP_fninfo_t fninfo)
{
    unw_word_t hash;
    ++pmap->size;
    if (pmap->size*MAX_FILL_DENOM > pmap->mask*MAX_FILL_NOM)
	fni_map_set_capacity(pmap, (pmap->mask + 1)*2);
    hash = HASH(key) & pmap->mask;
    fninfo->key = key;
    fninfo->next = pmap->arr[hash];
    pmap->arr[hash] = fninfo;
}
static cuflowP_fninfo_t
fni_map_find_mem(fni_map_t pmap, unw_word_t key)
{
    unw_word_t hash = HASH(key) & pmap->mask;
    cuflowP_fninfo_t node = pmap->arr[hash];
    while (node) {
	if (node->key == key)
	    return node;
	node = node->next;
    }
    return NULL;
}



/* Winding
 * ======= */

static int cuflowP_wind_done_init = 0;

#ifdef CUCONF_ENABLE_THREADS
static pthread_mutex_t cuflowP_windargs_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif
static struct fni_map_s cuflowP_ip_to_fninfo;

void
cuflowP_wind_init_function(int *done_init, cuflowP_windstate_t wst,
			cuflowP_fninfo_t fninfo)
{
    static struct cuflowP_windargs windargs_normal
	= { cuflow_wind_direction_normal };
    unw_context_t ucontext;
    unw_cursor_t cursor;
    unw_word_t sp;
    unw_word_t ip;

    assert(cuflowP_wind_done_init);
    if (unw_getcontext(&ucontext) < 0)
	goto fail;
    if (unw_init_local(&cursor, &ucontext) < 0)
	goto fail;
    if (unw_step(&cursor) < 0)
	goto fail;
    if (unw_get_reg(&cursor, UNW_REG_SP, &sp) < 0)
	goto fail;
#ifdef CUCONF_ENABLE_THREADS
    if (pthread_mutex_lock(&cuflowP_windargs_mutex) != 0)
	assert(!"Not reachable.");
    if (*done_init) {
	pthread_mutex_unlock(&cuflowP_windargs_mutex);
	return;
    }
#endif
#ifdef CUCONF_UNW_START_IP_WORKS
    {
	unw_proc_info_t pi;
	if (unw_get_proc_info(&cursor, &pi) < 0)
	    goto fail;
	ip = pi.start_ip;
    }
#else
    {
	char buf[1];
	unw_word_t offset;
	if (unw_get_reg(&cursor, UNW_REG_IP, &ip) < 0)
	    goto unlock_fail;
	unw_get_proc_name(&cursor, buf, 0, &offset);
	ip -= offset;
    }
#endif
    {
	fprintf(stderr, "DXX 0x%x\n", ip);
    }
    fni_map_insert(&cuflowP_ip_to_fninfo, ip, fninfo);
    fninfo->windargs_offset = (unw_word_t)&wst->windargs - sp;
    if (unw_get_reg(&cursor, UNW_REG_IP, &fninfo->ip) < 0)
	goto unlock_fail;
#ifdef CUCONF_ENABLE_THREADS
    *done_init = 1;
    pthread_mutex_unlock(&cuflowP_windargs_mutex);
#endif
    wst->windargs = &windargs_normal;
    return;
unlock_fail:
#ifdef CUCONF_ENABLE_THREADS
    pthread_mutex_unlock(&cuflowP_windargs_mutex);
#endif
fail:
    fprintf(stderr,
	    "error: Cound not initialise function information.\n"
	    "error: In libccf/wind_unw.c/cuflowP_wind_init_function, "
	    "calling libunwind.\n");
    abort();
}

void
cuflowP_unwind(volatile cuflowP_windargs_t windargs)
{
    unw_context_t ucontext;
    unw_cursor_t cursor;
    int st;

    assert(cuflowP_wind_done_init);
    unw_getcontext(&ucontext);
    unw_init_local(&cursor, &ucontext);
    while ((st = unw_step(&cursor)) > 0) {
	unw_word_t ip;
	cuflowP_fninfo_t fninfo;
#ifdef CUCONF_UNW_START_IP_WORKS
	unw_proc_info_t pi;
	unw_get_proc_info(&cursor, &pi);
	ip = pi.start_ip;
#else
	{
	    char buf[1];
	    unw_word_t offset;
	    if (unw_get_reg(&cursor, UNW_REG_IP, &ip) < 0)
		goto fail;
	    unw_get_proc_name(&cursor, buf, 0, &offset);
	    ip -= offset;
	}
#endif
	fninfo = fni_map_find_mem(&cuflowP_ip_to_fninfo, ip);
	{
	    char buf[1024];
	    unw_get_proc_name(&cursor, buf, 1024, NULL);
	    fprintf(stderr, "FXX 0x%x %s\n", ip, buf);
	}
	if (fninfo) {
	    unw_word_t sp;
	    if (unw_get_reg(&cursor, UNW_REG_SP, &sp) < 0)
		goto fail;
	    *(cuflowP_windargs_t *)(sp + fninfo->windargs_offset) = windargs;
	    if (unw_set_reg(&cursor, UNW_REG_IP, fninfo->ip) < 0)
		goto fail;
	    /* There might be an adjustment to sp to be done.  This
	     * may be detected and stored in fninfo.  */
	    unw_resume(&cursor);
	}
    }
    fprintf(stderr,
	    "error: Unwinding reached primordial state.  Aborting.\n");
    abort();
fail:
    fprintf(stderr,
	    "error: Cound not initialise function information.\n"
	    "error: In libccf/wind_unw.c/cuflowP_unwind, calling libunwind.\n");
    abort();
}

void
cuflowP_rewind(volatile cuflowP_windargs_t windargs)
{
    assert(!"Not implemented.");
    abort();
}

void
cuflow_unwind()
{
    cuflowP_windargs_t wi = GC_malloc(sizeof(struct cuflowP_windargs));
    assert(cuflowP_wind_done_init);
    wi->direction = cuflow_wind_direction_unwind;
    cuflowP_unwind(wi);
}

void
cuflowP_throw(volatile cuflowP_windargs_t windargs)
{
    windargs->direction = cuflow_wind_direction_except;
    cuflowP_unwind(windargs);
}

static void
cuflowP_wind_cleanup()
{
    fni_map_dct(&cuflowP_ip_to_fninfo);
}
void
cuflowP_wind_init()
{
    if (cuflowP_wind_done_init)
	return;
    cuflowP_wind_done_init = 1;
    fni_map_cct(&cuflowP_ip_to_fninfo);
    atexit(cuflowP_wind_cleanup);
}
