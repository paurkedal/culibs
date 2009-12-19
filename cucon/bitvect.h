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

#ifndef CUCON_BITVECT_H
#define CUCON_BITVECT_H

#include <cucon/bitarray.h>

#define cucon_bitvect			cucon_bitarray
#define cucon_bitvect_init_uninit	cucon_bitarray_init
#define cucon_bitvect_new_uninit	cucon_bitarray_new
#define cucon_bitvect_init_fill		cucon_bitarray_init_fill
#define cucon_bitvect_new_fill		cucon_bitarray_new_fill
#define cucon_bitvect_init_copy		cucon_bitarray_init_copy
#define cucon_bitvect_new_copy		cucon_bitarray_new_copy
#define cucon_bitvect_fill		cucon_bitarray_fill
#define cucon_bitvect_size		cucon_bitarray_size
#define cucon_bitvect_set_at		cucon_bitarray_set_at
#define cucon_bitvect_at		cucon_bitarray_at
#define cucon_bitvect_find		cucon_bitarray_find
#define cucon_bitvect_find2		cucon_bitarray_find2

#endif
