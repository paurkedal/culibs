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

#include <cuex/fwd.h>
#include <cugra/fwd.h>
#include <cuex/dunion.h>
#include <cuex/tpvar.h>

void cudynP_init(void);
void cuexP_atree_init(void);
void cuexP_ex_init(void);
void cuexP_labelling_init(void);
void cuexP_sig_init(void);
void cuexP_type_init(void);
void cuexP_print_init(void);
void cuex_oprdefs_init(void);

cuex_opn_t cuexP_dunion_empty;

void
cuex_init()
{
    static int done_init = 0;
    if (done_init)
	return;
    done_init = 1;

    cu_debug_assert(sizeof(struct cuex_tpvar_s)/sizeof(cu_word_t) <=
		    (1 << CUEXP_VARMETA_WSIZE_WIDTH));

    cugra_init();
    cudynP_init();
    cuexP_ex_init();
    cuex_oprdefs_init();
    cuexP_atree_init();
    cuexP_labelling_init();
    cuexP_sig_init();
    cuexP_type_init();
    cuexP_print_init();
    cuexP_dunion_empty = cuex_aci_identity(CUEX_O4ACI_DUNION);
}
