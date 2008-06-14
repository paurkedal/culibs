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

#ifndef CUSTO_BINARY_IO_H
#define CUSTO_BINARY_IO_H

#include <cucon/fwd.h>
#include <stdint.h>
#include <stdio.h>

CU_BEGIN_DECLARATIONS

cu_bool_t custo_fwrite_uintmax(uintmax_t i, FILE *file);
cu_bool_t custo_fread_uintmax(uintmax_t *i, FILE *file);
cu_bool_t custo_fwrite_intmax(intmax_t i, FILE *file);
cu_bool_t custo_fread_intmax(intmax_t *i, FILE *file);

/* Serialisation of strings */
cu_bool_t custo_fread_str_init(cu_str_t str, FILE *file);
cu_str_t custo_fread_str_new(FILE *file);
cu_bool_t custo_fwrite_str(cu_str_t str, FILE *file);

CU_END_DECLARATIONS

#endif
