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

#include <cuoo/halloc.h>
#include <cuoo/support.h>
#include <cu/clos.h>
#include <cu/ptr.h>
#include <atomic_ops.h>

cu_clos_def(_setao_clos, cu_prot(void, void *obj),
	    ( size_t ao_offset; AO_t ao_value; ))
{
    cu_clos_self(_setao_clos);
    AO_store((AO_t *)cu_ptr_add(obj, self->ao_offset), self->ao_value);
}

void *
cuooP_hxalloc_setao_raw(cuoo_type_t type, size_t struct_size,
			size_t key_size, void *key,
			cu_offset_t ao_offset, AO_t ao_value)
{
    _setao_clos_t init;
    init.ao_offset = ao_offset;
    init.ao_value = ao_value;
    return cuooP_hxalloc_init_raw(type, struct_size, key_size, key,
				  _setao_clos_prep(&init));
}

cu_clop_def(init_noop_clos, void, void *obj) {}

void *
cuooP_hxalloc_clear_raw(cuoo_type_t type, size_t struct_size,
			size_t key_size, void *key)
{
    return cuooP_hxalloc_init_raw(type, struct_size, key_size, key,
				  init_noop_clos);
}
