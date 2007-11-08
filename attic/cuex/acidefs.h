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

#ifndef CUEX_ACIDEFS_H
#define CUEX_ACIDEFS_H

#include <cuex/fwd.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuex_acidefs_h cuex/acidefs.h: Extras for ACI Operators
 *@{\ingroup cuex_mod */

#define CUEX_O0ACI_PROD		cuex_o0aci(CUEX_O3ACI_PROD)
#define CUEX_O0ACI_SIGPROD	cuex_o0aci(CUEX_O4ACI_SIGPROD)
#define CUEX_O0ACI_STRUCTJOIN	cuex_o0aci(CUEX_O4ACI_STRUCTJOIN)
#define CUEX_O0ACI_DUNION	cuex_o0aci(CUEX_O4ACI_DUNION)
#define CUEX_O0ACI_SETJOIN	cuex_o0aci(CUEX_O3ACI_SETJOIN)

/*!@}*/
CU_END_DECLARATIONS

#endif
