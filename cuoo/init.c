/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2005--2007  Petter Urkedal <urkedal@nbi.dk>
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

#include <cu/debug.h>

void cuooP_hcons_init(void);
void cuooP_oalloc_init(void);
void cuooP_layout_init(void);
void cuooP_properties_init(void);
void cuooP_type_init(void);

void
cuooP_init()
{
    cu_debug_assert_once();

    cuooP_oalloc_init();
    cuooP_hcons_init();
    cuooP_type_init();

    cuooP_layout_init();
    cuooP_properties_init();
}
