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

#ifndef CU_SIGNAL_H
#define CU_SIGNAL_H

#include <cuflow/fwd.h>
#include <cuflow/except.h>

CU_BEGIN_DECLARATIONS

cuflow_xc_extern(cuflow_err_stack_overflow, cu_prot0(void), ());
cuflow_xc_extern(cuflow_err_out_of_memory, cu_prot0(void), ());

CU_END_DECLARATIONS

#endif
