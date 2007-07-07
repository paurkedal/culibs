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

#include <cugra/fwd.h>
#include <cu/diag.h>
#ifdef CUCONF_HAVE_BUDDY
#include <bdd.h>
#endif

#ifdef CUCONF_HAVE_BUDDY
static void
bdd_error(int err)
{
    cu_bugf("BDD error %s", bdd_errstring(err));
}
#endif

void cugraP_graph_init(void);

void
cugra_init(void)
{
    static int done_init = 0;
    if (done_init)
	return;
    done_init = 1;

    cu_init();
    cugraP_graph_init();

#ifdef CUCONF_HAVE_BUDDY
    bdd_init(10000, 1000);
    bdd_error_hook(bdd_error);
    bdd_setcacheratio(10);
#endif
}
