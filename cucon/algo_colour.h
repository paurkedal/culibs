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

#ifndef CUCON_BITS_ALGO_COLOUR_H
#define CUCON_BITS_ALGO_COLOUR_H

#include <cucon/fwd.h>

CU_BEGIN_DECLARATIONS

/* This may seem like an overkill, but 'cucon_po_insert_constrain' needs
 * more than 4 colours, so we need 3 bits. */
typedef enum {
    cucon_algo_colour_white,
    cucon_algo_colour_grey,
    cucon_algo_colour_gray = cucon_algo_colour_grey,
    cucon_algo_colour_black,
    cucon_algo_colour_red,
    cucon_algo_colour_yellow,
    cucon_algo_colour_green,
    cucon_algo_colour_blue
} cucon_algo_colour_t;

char const *cucon_algo_colour_name(cucon_algo_colour_t colour);
cucon_algo_colour_t cucon_algo_colour_from_name(char const *name);

#define cucon_algo_colour_bits 3

CU_END_DECLARATIONS

#endif
