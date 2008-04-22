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

#include <cuex/subst.h>
#include <cuex/opn.h>
#include <cucon/pmap.h>


cuex_subst_t sig;
struct cucon_pmap_s ex_to_var;

void
ins(cuex_t ex)
{
    cuex_var_t var = cuex_subst_insert_fragment_accum(sig, &ex_to_var, ex, NULL);
    cu_verbf(0, "%! ↦ %!\n", cuex_var_to_ex(var), ex);
}

int
main()
{
    cuex_t v0, v1;
    cuex_init();
    sig = cuex_subst_new_uw();
    v0 = cuex_var_to_ex(cuex_var_new(cuex_qcode_active_s));
    v1 = cuex_var_to_ex(cuex_var_new(cuex_qcode_active_s));
    cucon_pmap_init(&ex_to_var);
    ins(cuex_opn(cuex_opr(0x100, 2),
		 cuex_opn(cuex_opr(0x101, 2), v0, v1),
		 cuex_opn(cuex_opr(0x101, 2), v0, v1)));
    ins(cuex_opn(cuex_opr(0x101, 2), v0, v1));
    cuex_subst_dump(sig, stdout);
    return 0;
}
