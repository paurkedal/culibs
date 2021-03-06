/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2002--2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUFLOW_FWD_H
#define CUFLOW_FWD_H

#include <stdlib.h>
#include <stdint.h>
#include <cu/conf.h>
#include <cu/fwd.h>

CU_BEGIN_DECLARATIONS

/** \defgroup cuflow_fwd_h cuflow/fwd.h: Forward Declarations
 ** @{ \ingroup cuflow_mod */

typedef struct cuflow_continuation *cuflow_continuation_t;
typedef struct cuflow_tstate	*cuflow_tstate_t;

typedef struct cuflow_cacheconf *cuflow_cacheconf_t;
typedef struct cuflow_gflexq	*cuflow_gflexq_t;	/* gworkq.h */
typedef struct cuflow_promise	*cuflow_promise_t;	/* promise.h*/
typedef struct cuflow_workq	*cuflow_workq_t;	/* workq.h */

/** Call this from the main thread to initialise the cuflow module before using
 ** it. */
void cuflow_init(void);

/** @} */

CU_END_DECLARATIONS

#ifndef CU_NCOMPAT
#  include <cuflow/compat.h>
#endif
#endif
