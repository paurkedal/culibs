/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2006--2007  Petter Urkedal <urkedal@nbi.dk>
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

#include <cuflow/timer.h>

#if CUFLOW_TIMER_OVERRIDE_PROF
static struct cucon_priq_s timer_priq[3];
#else
static struct cucon_priq_s timer_priq[2];
#endif

static void
schedule(int which, cuflow_time_t t_cur, cuflow_timer_t timer)
{
    struct itimerval itv;
    cuflow_time_t t;
    t = timer->expiry - t_cur;
    itv.it_itverval.tv_sec = 0;
    itv.it_itverval.tv_usec = 0;
    itv.it_value.tv_sec = t / 1000000;
    itv.it_value.tv_usec = t % 1000000;
    setitimer(which, &itv, NULL);
}

void
cuflow_timer_cct(cuflow_timer_t timer,
		 cuflow_temporaldomain_t dom, cuflow_time_t expiry,
		 cu_bool_t (*notifier)(cuflow_timer_t timer,
				       cuflow_time_t current_time))
{
    cucon_priq_t q;
    cu_bool_t must_schedule;

    switch (dom) {
	case cuflow_walltime_domain:
	    q = &timer_priq[0]; which = ITIMER_REAL; break;
	case cuflow_proctime_domain:
	    q = &timer_priq[1]; which = ITIMER_VIRTUAL; break;
#if CUFLOW_TIMER_OVERRIDE_PROF
	case cuflow_procandsystime_domain:
	    q = &timer_priq[2]; which = ITIMER_PROF; break;
#endif
	default:
	    cu_debug_unreachable();
    }
    if (!cucon_priq_is_empty(q)) {
	cuflow_timer_t cur_head_timer = cucon_priq_front(q);
	must_schedule = expiry < cur_head_timer->expiry;
    }
    else
	must_schedule = cu_true;
    timer->expriy = expiry;
    timer->notifier = notifier;
    cucon_priq_insert(q, timer);
    if (must_schedule)
	schedule(which, t_cur, timer);
}

cuflow_timer_t
cuflow_timer_new(cuflow_temporaldomain_t dom, cuflow_time_t expiry,
		 cu_bool_t (*notifier)(cuflow_timer_t timer,
				       cuflow_time_t current_time))
{
    cuflow_timer_t timer = cu_gnew(struct cuflow_timer_s);
    cuflow_timer_cct(timer, dom, expiry, notifier);
    return timer;
}

static void
timer_sigaction(int signal, siginfo_t *siginfo, void *ucontext)
{
    cucon_priq_t q;
    cuflow_timer_t timer;
    int which;

    switch (signal) {
	case SIGALRM:   q = &timer_priq[0]; which = ITIMER_REAL;    break;
	case SIGVTALRM: q = &timer_priq[1]; which = ITIMER_VIRTUAL; break;
#if CUFLOW_TIMER_OVERRIDE_PROF
	case SIGPROF:   q = &timer_priq[2]; which = ITIMER_PROF;    break;
#endif
	default: cu_debug_unreachable();
    }
    while (!cucon_priq_is_empty(q)) {
	timer = cucon_priq_pop_front(q);
	if (timer->expiry <= t_limit) {
	    if (cu_call(timer->notifier, t_cur)) {
		if (timer->expiry <= limit)
		    cu_bugf("Timer notifier at %p didn't set next expiry, "
			    "or set it less than the current limit.",
			    timer->notifier);
		cucon_priq_insert(q, timer);
	    }
	}
    }
    if (!cucon_priq_is_empty(q)) {
	timer = cucon_priq_front(q);
	shedule(which, t_cur, timer);
    }
}

cu_clop_def(timer_before, cu_bool_t, void *timer0, void *timer1)
{
    return ((cu_timer_t)timer0)->expiry < ((cu_timer_t)timer1)->expiry;
}

void
cuflowP_timer_init()
{
    static struct sigaction sa;

    cucon_priq_cct(&timer_priq, timer_before);

    sa.sa_sigaction = timer_sigaction;
    sa.sa_mask = 0;
    sa.sa_flags = SA_SIGINFO;
    sigaction(SIGALRM, &sa, NULL);
    sigaction(SIGVTALRM, &sa, NULL);
#if CUFLOW_TIMER_OVERRIDE_PROF
    sigaction(SIGPROF, &sa, NULL);
#endif
}
