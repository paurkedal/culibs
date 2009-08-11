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

#ifndef CUFO_FWD_H
#define CUFO_FWD_H

#include <cu/fwd.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cufo_fwd_h cufo/fwd.h: Forward Declarations
 *@{\ingroup cufo_mod */

typedef struct cufo_prispec_s *cufo_prispec_t;
typedef struct cufo_stream_s *cufo_stream_t;

typedef struct cufo_tag_s *cufo_tag_t;
typedef struct cufo_namespace_s *cufo_namespace_t;
typedef struct cufo_attr_s *cufo_attr_t;
typedef struct cufo_attrbind_s *cufo_attrbind_t;

typedef struct cufo_termface_s *cufo_termface_t;
typedef struct cufo_termstyle_s *cufo_termstyle_t;
typedef struct cufo_textsink_s *cufo_textsink_t;

typedef struct cufo_textstyle_s *cufo_textstyle_t;
typedef struct cufo_textstyler_s *cufo_textstyler_t;

/** Initialises \c libcufo, as is mandatory before before using the library.
 ** You may also want to call this function even if you don't use \c libcufo
 ** functions directly, because it installs a more powerful formatting engine
 ** for the \ref cu_logging_h "cu/logging.h" and \ref cu_diag_h "cu/diag.h"
 ** functions, including new format specifiers, line wrapping, and, if compiled
 ** with terminal support, some highlighting. */
void cufo_init(void);

/*!@}*/
CU_END_DECLARATIONS

#endif
