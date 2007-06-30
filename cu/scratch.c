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

#include <cu/scratch.h>
#include <cu/memory.h>
#include <string.h>

void
cuP_scratch_alloc_min(cu_scratch_t scr, size_t size)
{
    scr->ptr = cu_galloc(size);
    scr->size = size;
}

void
cuP_scratch_alloc_log(cu_scratch_t scr, size_t size)
{
    if (scr->size*2 > size)
	size = scr->size*2;
    scr->ptr = cu_galloc(size);
    scr->size = size;
}

void
cuP_scratch_realloc_min(cu_scratch_t scr, size_t size)
{
    void *ptr = cu_galloc(size);
    memcpy(ptr, scr->ptr, scr->size);
    scr->ptr = ptr;
    scr->size = size;
}

void
cuP_scratch_realloc_log(cu_scratch_t scr, size_t size)
{
    void *ptr;
    if (scr->size*2 > size)
	size = scr->size*2;
    ptr = cu_galloc(size);
    memcpy(ptr, scr->ptr, scr->size);
    scr->ptr = ptr;
    scr->size = size;
}
