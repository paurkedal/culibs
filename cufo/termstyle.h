/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2009  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUFO_TERMSTYLE_H
#define CUFO_TERMSTYLE_H

#include <cufo/fwd.h>
#include <cucon/pmap.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cufo_termstyle_h cufo/termstyle.h: Terminal Style Specification
 *@{\ingroup cufo_mod */

#define CUFO_TERMFACE_FGCOLOUR 1
#define CUFO_TERMFACE_BGCOLOUR 2
#define CUFO_TERMFACE_ITALIC 4
#define CUFO_TERMFACE_UNDERLINE 8
#define CUFO_TERMFACE_BOLD 16
#define CUFO_TERMFACE_REVERSE 32

struct cufo_termface_s
{
    uint_least8_t enables;
    uint_least8_t booleans;
    uint_least8_t fgcolour, bgcolour;
};

void cufo_termface_init(cufo_termface_t face);

cufo_termface_t cufo_termface_new(void);

void cufo_termface_set_fgcolour(cufo_termface_t face, int colour);

void cufo_termface_set_bgcolour(cufo_termface_t face, int colour);

cu_bool_t cufo_termface_set_fgcolour_cstr(cufo_termface_t face,
					  char const *colour);

cu_bool_t cufo_termface_set_bgcolour_cstr(cufo_termface_t face,
					  char const *colour);

void cufo_termface_set_bool(cufo_termface_t face, unsigned int attr, cu_bool_t val);

CU_SINLINE void cufo_termface_set_italic(cufo_termface_t face, cu_bool_t val)
{ cufo_termface_set_bool(face, CUFO_TERMFACE_ITALIC, val); }

CU_SINLINE void cufo_termface_set_underline(cufo_termface_t face, cu_bool_t val)
{ cufo_termface_set_bool(face, CUFO_TERMFACE_UNDERLINE, val); }

CU_SINLINE void cufo_termface_set_bold(cufo_termface_t face, cu_bool_t val)
{ cufo_termface_set_bool(face, CUFO_TERMFACE_BOLD, val); }

CU_SINLINE void cufo_termface_set_reverse(cufo_termface_t face, cu_bool_t val)
{ cufo_termface_set_bool(face, CUFO_TERMFACE_REVERSE, val); }


struct cufo_termstyle_s
{
    struct cucon_pmap_s face_map;
};

void cufo_termstyle_init(cufo_termstyle_t style);

cufo_termstyle_t cufo_termstyle_new(void);

cu_bool_t cufo_termstyle_loadinto(cufo_termstyle_t style, cu_str_t name);

cufo_termface_t cufo_termstyle_get(cufo_termstyle_t style, cufo_tag_t tag);

cufo_termface_t cufo_termstyle_ref(cufo_termstyle_t style, cufo_tag_t tag);

/*!@}*/
CU_END_DECLARATIONS

#endif
