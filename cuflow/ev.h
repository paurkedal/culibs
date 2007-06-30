/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2003--2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CCF_EV_H
#define CCF_EV_H

#include <ccf/fwd.h>

CCF_BEGIN_DECLARATIONS

/* Exception vector indices. */
typedef unsigned int ccf_evi_t;
#define ccf_evi_logic 1
#define ccf_evi_runtime 2

typedef struct {
    ccf_closure_t on_xc;
    ccf_continuation_t cont;
} ccf_ev_elt_t;

/* Allocate 'n' slots in the exception vector and return the first
 * index.  IMPORTANT: This must be done before any threads are
 * created.  If you write a library, make sure that your
 * initialisation function does not create any threads, so that the
 * user have a chance to initialise other libraries using 'libccf', as
 * well. */
int ccf_ev_alloc(int n);

ccf_ev_elt_t *ccf_ev();

int ccf_ev_catch(ccf_closure_t trunk,
		 /* ccf_evi_t index_1, ccf_closure_t handler_1, */...
		 /* ccf_evi_t index_n, ccf_closure_t handler_n, 0 */);

void ccf_ev_throw(ccf_evi_t index, ...) CCF_ATTR_NORETURN;
int ccf_ev_throw_if_catched(ccf_evi_t index, ...);


CCF_END_DECLARATIONS

#endif
