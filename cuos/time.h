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

#ifndef CUOS_TIME_H
#define CUOS_TIME_H

#include <cucon/fwd.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuos_time cuos/time.h: System and process time and related.
 * @{ \ingroup cuos_mod */

double cuos_time_utc(void);
double cuos_time_proc(void);
double cuos_sleep(double);

/* @} */
CU_END_DECLARATIONS

#endif
