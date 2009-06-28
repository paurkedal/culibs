/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2007  Petter Urkedal <urkedal@nbi.dk>
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

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <cu/conf.h>
#include <cu/clos.h>
#include <cuflow/cont.h>
#ifdef CUCONF_HAVE_GC_GC_H
#  include <gc/gc.h>
#else
#  include <gc.h>
#endif

static int opt_decline = 0;

cu_clop_def0(thrower, void)
{
    printf("Throwing \"Take this\".\n");
    cuflow_throw("Take this.");
}

cu_clop_def0(relaxer, void)
{
    printf("I do nothing.\n");
}

cu_clop_def(catcher, void, char *xc)
{
    printf("Got exception \"%s\".  Accepting.\n", xc);
}

cu_clop_def(rethrower, void, char *xc)
{
    printf("Got exception \"%s\".  Rethrowing.\n", xc);
    cuflow_throw(xc);
    assert(!"Not reached.");
}

cu_clop_def(complainer, void, char *xc)
{
    assert(!"Not reached.");
}

cu_clop_def0(catch_and_rethrower, void)
{
    cuflow_catch(thrower,
		 (cu_clop(, void, void *))rethrower);
}

cu_clos_def(printer,
	    cu_prot0(void),
	    (char *str;))
{
    cu_clos_self(printer);
    printf(self->str);
}

cu_clos_def(winder,
	    cu_prot0(void),
	    (cu_clop0(trunk, void);))
{
    cu_clos_self(winder);
    printer_t pr0;
    printer_t pr1;
    pr0.str = "Winding in.\n";
    pr1.str = "Winding out.\n";
    cuflow_dynamic_wind(printer_prep(&pr0), self->trunk,
			printer_prep(&pr1), cuflow_current_mode());
}

cu_clos_def(cc_setter,
	    cu_prot0(void),
	    ( int is_reentry;
	      cu_clop(cont, void, void *); ))
{
    cu_clos_self(cc_setter);
    printf("Saving current continuation.\n");
    cuflow_set_cc(&self->cont,
	       &self->is_reentry, sizeof(self->is_reentry));
    assert(!cu_clop_is_null(self->cont));
    printf("Leaving cc_setter: is_reentry = %d\n", self->is_reentry);
}

cu_clop0(mk_winder(cu_clop0(wound, void)), void)
{
    winder_t *cl = GC_malloc(sizeof(winder_t));
    cl->trunk = wound;
    return winder_prep(cl);
}

cu_clop_def0(alt_main, void)
{
    cc_setter_t *setcc;
    printf("Entering alt_main.\n");
    cuflow_catch(relaxer,
		 (cu_clop(, void, void *))complainer);
    cuflow_catch(thrower,
		 (cu_clop(, void, void *))catcher);
    cuflow_catch(catch_and_rethrower,
		 (cu_clop(, void, void *))(void *)catcher);
    cu_call0(mk_winder(relaxer));
    printf("Current flow is %s.\n", cuflow_mode_name(cuflow_current_mode()));
    cuflow_catch(mk_winder(thrower),
	      (cu_clop(, void, void *))catcher);

    setcc = GC_malloc(sizeof(cc_setter_t));
    setcc->is_reentry = 0;
    setcc->cont = cu_clop_null;
    printf("Calling double winder.\n");
    cu_call0(mk_winder(mk_winder(cc_setter_prep(setcc))));
    if (setcc->is_reentry < 2) {
	int i = setcc->is_reentry + 1;
	printf("Reentering double winder.\n");
	assert(!cu_clop_is_null(setcc->cont));
	cu_call(setcc->cont, &i);
    }

    if (opt_decline) {
	cuflow_tstate()->opt_uncaught_backtrace = 1;
	cu_call0(mk_winder(thrower));
    }

    printf("Leaving alt_main.\n");
    cuflow_continuation_print_stats();
    exit(0);
}

int
main(int argc, char** argv)
{
    if (argc == 2)
	sscanf(argv[1], "%d", &opt_decline);
    cuflow_init();
    cuflow_call_in_root(alt_main, cuflow_mode_nondet);
    return 0;
}
