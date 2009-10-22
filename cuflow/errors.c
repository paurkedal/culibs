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

#include <cuflow/fwd.h>
#include <cuflow/errors.h>
#include <cu/conf.h>
#include <cu/memory.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#ifdef CUCONF_HAVE_GC_GC_H
#  include <gc/gc.h>
#else
#  include <gc.h>
#endif


/* Throw exception on stack overflow
 * ================================= */

static char *stack_end_zone_low;
static char *stack_end_zone_high;

cuflow_xc_define(cuflow_err_stack_overflow, cu_prot0(void)) {}
static struct cuflow_err_stack_overflow_s stack_overflow_xc;

static void
segv_handler(int sig, siginfo_t *si, void *ucontext)
{
    char *err_addr;

    /* Any SIGV here causes an infinite loop, so better be
     * paranoid. */
    if (!si) {
	static char errmsg[] = "Missing siginfo argument.\n";
	CU_DISCARD(write(2, errmsg, sizeof(errmsg) - 1));
    }
    else if (!ucontext) {
	static char errmsg[] = "Missing ucontext argument.\n";
	CU_DISCARD(write(2, errmsg, sizeof(errmsg) - 1));
    }
    else {
	err_addr = (char *)si->si_addr;
	if (err_addr >= stack_end_zone_low &&
	    err_addr < stack_end_zone_high) {
	    static char errmsg[] = "warning: Stack overflow.\n";
	    CU_DISCARD(write(2, errmsg, sizeof(errmsg) - 1));
	    cuflow_throw_static(&stack_overflow_xc);
	}
    }

    /* Return control to the instruction which caused the coma, but now
     * let it hit the wall and dump core so we can gdb the fault. */
    signal(sig, SIG_DFL);
}

static void
cuflowP_signal_init_stack_overflow()
{
    struct sigaction act;
    stack_t sk;
    struct rlimit rlim;
    char *stack_end;

    /* Let GC detect the start of the stack, however it does it. */
    GC_malloc(1);

    if (getrlimit(RLIMIT_STACK, &rlim) < 0) {
	fprintf(stderr,
		"warning: Could not getrlimit for handling stack overflow: "
		"%s\n",
		strerror(errno));
	return;
    }
#if CUCONF_STACK_DIRECTION > 0
    stack_end = GC_stackbottom + rlim.rlim_cur; /* untested */
#elif CUCONF_STACK_DIRECTION < 0
    stack_end = GC_stackbottom - rlim.rlim_cur;
#else
#error Missing CUCONF_STACK_DIRECTION autoconf-igured variable.
#endif
    stack_end_zone_low = stack_end - 0x8000;
    stack_end_zone_high = stack_end + 0x8000;

    act.sa_sigaction = segv_handler;
    sigfillset(&act.sa_mask);
    act.sa_flags = SA_ONSTACK | SA_SIGINFO;
    if (sigaction(SIGSEGV, &act, NULL) < 0) {
	fprintf(stderr,
		"waring: Could not call sigaction for handling stack "
		"overflow: %s\n",
		strerror(errno));
	return;
    }
    sk.ss_flags = 0;
    sk.ss_sp = malloc(10*MINSIGSTKSZ);
    sk.ss_size = 10*MINSIGSTKSZ;
    if (sigaltstack(&sk, NULL) < 0) {
	fprintf(stderr,
		"warning: Could not call sigaltstack for handling stack "
		"overflow: %s\n",
		strerror(errno));
	signal(SIGSEGV, SIG_DFL);
    }
    cuflow_xc_cct0_m(&stack_overflow_xc, cuflow_err_stack_overflow_cct);
}


/* Out of memory exception
 * ======================= */

cuflow_xc_define(cuflow_err_out_of_memory, cu_prot0(void)) {}

static void out_of_memory(size_t size)
{
    /* Must use static struct since we can't allocate here. */
    static struct cuflow_err_out_of_memory_s xc;
    xc.windargs.xc_key = cuflow_err_out_of_memory_cct;
    cuflowP_throw(&xc.windargs);
    exit(1);
}


/* Init
 * ==== */

void
cuflowP_signal_init()
{
    cuflowP_signal_init_stack_overflow();
    cu_regh_out_of_memory(out_of_memory);
}
