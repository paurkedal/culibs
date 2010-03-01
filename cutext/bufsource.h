/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2010  Petter Urkedal <paurkedal@eideticdew.org>
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

#ifndef CUTEXT_BUFSOURCE_H
#define CUTEXT_BUFSOURCE_H

#include <cutext/source.h>
#include <cu/inherit.h>
#include <cu/buffer.h>

CU_BEGIN_DECLARATIONS
/** \defgroup cutext_bufsource_h cutext/bufsource.h:
 ** @{ \ingroup cutext_mod */

struct cutext_bufsource
{
    cu_inherit (cutext_source);
    cutext_source_t subsrc;
    struct cu_buffer buf;
};

void cutext_bufsource_init(cutext_bufsource_t bsrc, cutext_source_t subsrc);

size_t cutext_bufsource_read(cutext_bufsource_t bsrc, void *buf, size_t size);

void const *cutext_bufsource_look(cutext_bufsource_t bsrc,
				  size_t req_size, size_t *act_size);

CU_SINLINE void cutext_bufsource_close(cutext_bufsource_t bsrc)
{ cutext_source_close(bsrc->subsrc); }

/** @} */
CU_END_DECLARATIONS

#endif
