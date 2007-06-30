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

#include <cuflow/gworkq.h>
#include <cuflow/gflexq.h>
#include <cu/thread.h>
#include <cu/memory.h>
#include <atomic_ops.h>

struct workq_head_s
{
    struct cuflow_gflexq_s gflexq;
};

static struct workq_head_s gworkq[cuflow_priority_postmax];
static AO_t gworkq_priority = 0;
pthread_mutex_t cuflowP_gworkq_mutex = CU_MUTEX_INITIALISER;
pthread_cond_t cuflowP_gworkq_cond = PTHREAD_COND_INITIALIZER;


/* gworkq functions
 * ---------------- */

cuflow_priority_t
cuflow_gworkq_current_priority()
{
    cuP_tstate_t tst = cuP_tstate();
    if (tst->gflexq)
	return ((cuflow_gflexq_t)tst->gflexq)->priority;
    else
	return tst->priority;
}

cuflow_priority_t
cuflow_gworkq_set_static_priority(cuflow_priority_t priority)
{
    cuP_tstate_t tst = cuP_tstate();
    cuflow_priority_t old = tst->priority;
    if (tst->gflexq)
	cu_bugf("Static priority can not be set while running with "
		"flexible-priority queue.");
    tst->priority = priority;
    return old;
}

static cu_bool_t
gworkq_yield_at(int cutoff_priority, cu_bool_t have_lock)
{
    int gwq_priority = gworkq_priority;
    if (!have_lock)
	cuflow_gworkq_lock();
    while (cutoff_priority <= gwq_priority) {
	/* Pop off the first flexible work queue, else the fixed work
	 * queue.  Unlink empty flexible queues. */
	struct workq_head_s *head = &gworkq[gwq_priority];
	cuflow_gflexq_t gflexq = head->gflexq.next;
	cuflow_workq_fn_t fn;
	fn = cuflow_workq_pop_front(cu_to(cuflow_workq, gflexq));
	while (!fn && gflexq != &head->gflexq) {
	    head->gflexq.next = gflexq->next;
	    gflexq->next->prev = &head->gflexq;
	    gflexq->next = gflexq->prev = NULL;
	    gflexq = head->gflexq.next;
	    fn = cuflow_workq_pop_front(cu_to(cuflow_workq, gflexq));
	}

	/* Drop lock, set correct priority, and run the job. */
	if (fn) {
	    cuP_tstate_t tst;
	    cuflow_priority_t saved_priority;
	    cuflow_gflexq_t saved_gflexq;
	    gworkq_priority = gwq_priority;
	    cuflow_gworkq_unlock();
	    tst = cuP_tstate();
	    saved_priority = tst->priority;
	    saved_gflexq = tst->gflexq;
	    tst->priority = gwq_priority;
	    tst->gflexq = gflexq == &head->gflexq? NULL : gflexq;
	    cu_call0(fn);
	    tst->priority = saved_priority;
	    tst->gflexq = saved_gflexq;
	    if (have_lock)
		cuflow_gworkq_lock();
	    return cu_true;
	}
	--gwq_priority;
    }
    gworkq_priority = gwq_priority;
    if (!have_lock)
	cuflow_gworkq_unlock();
    return cu_false;
}

cu_bool_t
cuflow_gworkq_yield_at(int cutoff_priority)
{
    int gwq_priority = AO_load_acquire_read(&gworkq_priority);
    if (cutoff_priority <= gwq_priority) {
	cu_bool_t r;
	r = gworkq_yield_at(cutoff_priority, cu_false);
	return r;
    }
    return cu_false;
}

cu_bool_t
cuflow_gworkq_yield_at_glck(int cutoff_priority)
{ return gworkq_yield_at(cutoff_priority, cu_true); }

cu_bool_t
cuflow_gworkq_yieldall_prioreq(void)
{
    cuflow_priority_t cutoff_priority = cuflow_gworkq_current_priority();
    if (cuflow_gworkq_yield_at(cutoff_priority)) {
	while (cuflow_gworkq_yield_at(cutoff_priority));
	return cu_true;
    }
    else
	return cu_false;
}

cu_bool_t
cuflow_gworkq_yieldall_prior(void)
{
    cuflow_priority_t cutoff_priority = cuflow_gworkq_current_priority() + 1;
    if (cuflow_gworkq_yield_at(cutoff_priority)) {
	while (cuflow_gworkq_yield_at(cutoff_priority));
	return cu_true;
    }
    else
	return cu_false;
}

cu_bool_t
cuflow_gworkq_yield_idle(void)
{
    cuflow_priority_t cutoff_priority = cuflow_gworkq_current_priority() + 1;
    if (cuflow_gworkq_yield_at(cutoff_priority)) {
	while (cuflow_gworkq_yield_at(cutoff_priority));
	return cu_true;
    }
    else
	return cuflow_gworkq_yield_at(cutoff_priority - 1);
}

void
cuflow_gworkq_yield_gcond_glck(void)
{
    if (!gworkq_yield_at(cuflow_gworkq_current_priority(), cu_true))
	pthread_cond_wait(&cuflowP_gworkq_cond, &cuflowP_gworkq_mutex);
}

void
cuflow_gworkq_sched_at(cuflow_workq_fn_t fn, cuflow_priority_t priority)
{
    cuflow_gflexq_t gflexq = &gworkq[priority].gflexq;
    cuflow_gworkq_lock();
    if (gflexq->next == NULL)
	pthread_cond_broadcast(&cuflowP_gworkq_cond);
    cuflow_workq_append(cu_to(cuflow_workq, gflexq), fn);
    if (priority > gworkq_priority)
	gworkq_priority = priority;
    cuflow_gworkq_unlock();
}

void
cuflow_gworkq_sched(cuflow_workq_fn_t fn)
{
    cuP_tstate_t tst = cuP_tstate();
    cuflow_gflexq_t gflexq = tst->gflexq;
    cuflow_priority_t priority;
    cuflow_gworkq_lock();
    if (!gflexq) {
	priority = tst->priority;
	gflexq = &gworkq[priority].gflexq;
	if (gflexq->next == NULL)
	    pthread_cond_broadcast(&cuflowP_gworkq_cond);
	cuflow_workq_append(cu_to(cuflow_workq, gflexq), fn);
    }
    else {
	priority = gflexq->priority;
	cuflow_gflexq_sched_glck(gflexq, fn);
    }
    if (priority > gworkq_priority)
	gworkq_priority = priority;
    cuflow_gworkq_unlock();
}

void
cuflow_gworkq_dump(FILE *out)
{
    int i;
    fprintf(out, "Non-empty global work queues:\n");
    cuflow_gworkq_lock();
    for (i = cuflow_priority_postmax; i;) {
	cuflow_gflexq_t head;
	cuflow_gflexq_t gflexq;
	size_t size;
	--i;
	head = &gworkq[i].gflexq;
	gflexq = head->next;
	size = cuflow_workq_size(cu_to(cuflow_workq, head));
	if (size > 0 || gflexq != head)
	    fprintf(out, "\tPriority %d has %ld tasks.\n", i, (long)size);
	while (gflexq != head) {
	    fprintf(out, "\t    Flexqueue with %ld tasks.\n",
		    (long)cuflow_workq_size(cu_to(cuflow_workq, gflexq)));
	    gflexq = gflexq->next;
	}
    }
    cuflow_gworkq_unlock();
}

/* gflexq functions
 * ---------------- */

void
cuflow_gflexq_cct(cuflow_gflexq_t gflexq, cuflow_priority_t initpri)
{
    cuflow_workq_cct(cu_to(cuflow_workq, gflexq));
    gflexq->priority = initpri;
    gflexq->next = gflexq->prev = NULL;
    cucon_list_cct(&gflexq->subqueue_list);
}

cuflow_gflexq_t
cuflow_gflexq_new(cuflow_priority_t initpri)
{
    cuflow_gflexq_t gfq = cu_gnew(struct cuflow_gflexq_s);
    cuflow_gflexq_cct(gfq, initpri);
    return gfq;
}

void
cuflow_gflexq_sched_glck(cuflow_gflexq_t gflexq, cuflow_workq_fn_t fn)
{
    cuflow_workq_append(cu_to(cuflow_workq, gflexq), fn);
    if (gflexq->next == NULL) {
	cuflow_priority_t priority = gflexq->priority;
	cuflow_gflexq_t head = &gworkq[priority].gflexq;
	gflexq->next = head;
	gflexq->prev = head->prev;
	head->prev = gflexq;
	gflexq->prev->next = gflexq;
	if (priority > gworkq_priority)
	    gworkq_priority = priority;
	pthread_cond_broadcast(&cuflowP_gworkq_cond);
    }
}

void
cuflow_gflexq_sched(cuflow_gflexq_t gflexq, cuflow_workq_fn_t fn)
{
    cuflow_gworkq_lock();
    cuflow_gflexq_sched_glck(gflexq, fn);
    cuflow_gworkq_unlock();
}

static void
gflexq_set_priority_glck(cuflow_gflexq_t gflexq, cuflow_priority_t priority)
{
    if (gflexq->next != NULL) {
	cuflow_gflexq_t head = &gworkq[priority].gflexq;
	gflexq->prev->next = gflexq->next;
	gflexq->next->prev = gflexq->prev;
	gflexq->next = head;
	gflexq->prev = head->prev;
	head->prev = gflexq;
	gflexq->prev->next = gflexq;
	if (priority > gworkq_priority)
	    gworkq_priority = priority;
    }
    AO_store_release_write(&gflexq->priority, priority);
}

void
cuflow_gflexq_raise_priority_glck(cuflow_gflexq_t gflexq, cuflow_priority_t pri)
{
    cucon_listnode_t it;
    gflexq_set_priority_glck(gflexq, pri);
    for (it = cucon_list_begin(&gflexq->subqueue_list);
	    it != cucon_list_end(&gflexq->subqueue_list);
	    it = cucon_listnode_next(it)) {
	gflexq = cucon_listnode_ptr(it);
	if (gflexq->priority < pri)
	    cuflow_gflexq_raise_priority_glck(gflexq, pri);
    }
}

void
cuflow_gflexq_raise_priority(cuflow_gflexq_t gflexq, cuflow_priority_t pri)
{
    if (AO_load_acquire_read(&gflexq->priority) < pri) {
	cuflow_gworkq_lock();
	cuflow_gflexq_raise_priority_glck(gflexq, pri);
	cuflow_gworkq_unlock();
    }
}

void
cuflow_gflexq_enter(cuflow_gflexq_t gflexq, cuflow_gflexq_entry_t entry)
{
    cuP_tstate_t tst = cuP_tstate();
    cuflow_gflexq_t old_gflexq = tst->gflexq;
    if (old_gflexq) {
	cuflow_gworkq_lock();
	if (old_gflexq->priority > gflexq->priority)
	    cuflow_gflexq_raise_priority_glck(gflexq, old_gflexq->priority);
	entry->subqueue_it =
	    cucon_list_append_ptr(&old_gflexq->subqueue_list, gflexq);
	cuflow_gworkq_unlock();
    }
    else if (tst->priority > gflexq->priority)
	cuflow_gflexq_raise_priority(gflexq, tst->priority);
    entry->gflexq = old_gflexq;
    tst->gflexq = gflexq;
}

void
cuflow_gflexq_enter_glck(cuflow_gflexq_t gflexq, cuflow_gflexq_entry_t entry)
{
    cuP_tstate_t tst = cuP_tstate();
    cuflow_gflexq_t old_gflexq = tst->gflexq;
    if (old_gflexq) {
	if (old_gflexq->priority > gflexq->priority)
	    cuflow_gflexq_raise_priority_glck(gflexq, old_gflexq->priority);
	entry->subqueue_it =
	    cucon_list_append_ptr(&old_gflexq->subqueue_list, gflexq);
    }
    else if (tst->priority > gflexq->priority)
	cuflow_gflexq_raise_priority_glck(gflexq, tst->priority);
    entry->gflexq = old_gflexq;
    tst->gflexq = gflexq;
}

void
cuflow_gflexq_leave_glck(cuflow_gflexq_entry_t entry)
{
    cuP_tstate_t tst = cuP_tstate();
    tst->gflexq = entry->gflexq;
    if (entry->gflexq)
	cucon_list_erase_node(entry->subqueue_it);
}

void
cuflow_gflexq_leave(cuflow_gflexq_entry_t entry)
{
    cuP_tstate_t tst = cuP_tstate();
    tst->gflexq = entry->gflexq;
    if (entry->gflexq) {
	cuflow_gworkq_lock();
	cucon_list_erase_node(entry->subqueue_it);
	cuflow_gworkq_unlock();
    }
}


/* Init
 * ---- */

void
cuflowP_gworkq_init()
{
    int priority;
    for (priority = 0; priority < cuflow_priority_postmax; ++priority) {
	cuflow_gflexq_t gflexq = &gworkq[priority].gflexq;
	cuflow_workq_cct(cu_to(cuflow_workq, gflexq));
	gflexq->priority = priority;
	gflexq->next = gflexq;
	gflexq->prev = gflexq;
    }
}
