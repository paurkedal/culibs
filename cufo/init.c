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

#include <cu/util.h>

void cufoP_tag_init(void);
void cufoP_tagdefs_init(void);
void cufoP_attrdefs_init(void);

void
cufo_init(void)
{
    CU_RETURN_UNLESS_FIRST_CALL;
    cu_init();
    cufoP_tag_init();
    cufoP_tagdefs_init();
    cufoP_attrdefs_init();
}
