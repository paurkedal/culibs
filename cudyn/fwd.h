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

#ifndef CUDYN_FWD_H
#define CUDYN_FWD_H

#include <cuoo/fwd.h>

CU_BEGIN_DECLARATIONS

typedef struct cudyn_proto_s	*cudyn_proto_t;
typedef struct cudyn_arrtype_s	*cudyn_arrtype_t;
typedef struct cudyn_ctortype_s	*cudyn_ctortype_t;
typedef struct cudyn_inltype_s	*cudyn_inltype_t;
typedef struct cudyn_elmtype_s	*cudyn_elmtype_t;
typedef struct cudyn_ptrtype_s	*cudyn_ptrtype_t;
typedef struct cudyn_sngtype_s	*cudyn_sngtype_t;
typedef struct cudyn_tuptype_s	*cudyn_tuptype_t;
typedef struct cudyn_duntype_s	*cudyn_duntype_t;

CU_END_DECLARATIONS

#endif
