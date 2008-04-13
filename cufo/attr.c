/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2008  Petter Urkedal <urkedal@nbi.dk>
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

#include <cufo/attr.h>

void
cufo_attr_init(cufo_attr_t attr, cufo_namespace_t ns,
	       char const *name, cufo_attrtype_t type)
{
    attr->ns = ns;
    attr->idr = cu_idr_by_cstr(name);
    attr->type = type;
    attr->fixed_value = NULL;
}

void
cufo_attr_init_fixed(cufo_attr_t attr, cufo_namespace_t ns,
		     char const *name, char const *val)
{
    attr->ns = ns;
    attr->idr = cu_idr_by_cstr(name);
    attr->type = cufo_attrtype_fixed;
    attr->fixed_value = val;
}
