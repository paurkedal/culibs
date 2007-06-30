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

#define CUCON_SLINK_C
#include <cucon/slink.h>

cucon_slink_t
cucon_slink_prepend_ptr(cucon_slink_t slink, void *ptr)
{
    cucon_slink_t l = cu_galloc(cu_aligned_ceil(sizeof(struct cucon_slink_s))
			       + sizeof(void *));
    l->next = slink;
    *(void**)CU_ALIGNED_PTR_END(l) = ptr;
    return l;
}
