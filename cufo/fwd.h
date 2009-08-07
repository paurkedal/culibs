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

typedef struct cufo_target_s *cufo_target_t;
typedef struct cufo_prispec_s *cufo_prispec_t;
typedef struct cufo_stream_s *cufo_stream_t;

typedef struct cufo_tag_s *cufo_tag_t;
typedef struct cufo_namespace_s *cufo_namespace_t;
typedef struct cufo_attr_s *cufo_attr_t;

typedef struct cufo_termface_s *cufo_termface_t;
typedef struct cufo_termstyle_s *cufo_termstyle_t;
typedef struct cufo_textstream_s *cufo_textstream_t;

typedef struct cufo_textstyle_s *cufo_textstyle_t;
typedef struct cufo_textstyler_s *cufo_textstyler_t;

void cufo_init(void);

/*!@}*/
CU_END_DECLARATIONS

#endif
