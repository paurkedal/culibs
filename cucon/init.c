/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2004--2007  Petter Urkedal <urkedal@nbi.dk>
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

#include <cucon/fwd.h>

void cu_init(void);
void cuconP_logchain_init(void);
void cuconP_list_init(void);
void cuconP_pmap_init(void);
void cuconP_layout_init(void);
void cuconP_ucset_init(void);
void cuconP_ucmap_init(void);

void
cucon_init(void)
{
    static int done_init = 0;
    if (done_init)
	return;
    done_init = 1;

    cu_init();
    cuconP_layout_init();
    cuconP_list_init();
    cuconP_logchain_init();
    cuconP_ucset_init();
    cuconP_ucmap_init();
}
