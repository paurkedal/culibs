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

#define CHI_VAR_C
#include <cuex/opn.h>
#include <cuex/pvar.h>
#include <cu/memory.h>
#include <cu/str.h>
#include <assert.h>
#include <string.h>


/* Qcodes
 * ------ */

char const *
cuex_qcode_name(cuex_qcode_t q)
{
    /* switch is safer than array, and speed is not important. */
    switch (q) {
    case cuex_qcode_n: return "unquantified";
    case cuex_qcode_u: return "universal";
    case cuex_qcode_w: return "weak universal";
    case cuex_qcode_e: return "existential";
    default:
	assert(!"Not reached.");
	return "!!! INVALID !!!";
    }
}


/* Variables
 * --------- */

cuex_pvar_t
cuex_pvar_new(cuex_qcode_t q)
{
    cuex_pvar_t var = cuex_oalloc(cuex_pvarmeta(q),
				  sizeof(struct cuex_pvar_s));
    var->offset = (cu_offset_t)-1;
    var->module = NULL;
    var->idr = NULL;
    var->type = NULL; /* XXX was clsof_ex */
    var->var_chain = NULL;
    return var;
}

cuex_pvar_t
cuex_pvar_new_named(cuex_qcode_t q, char const* name)
{
    cuex_pvar_t var = cuex_pvar_new(q);
#if 0 /* XXX */
    char *xname;
    xname = cu_galloc(sizeof(char)*(strlen(name) + 1));
    strcpy(xname, name);
    chi_ex_set_print_str(cuex_pvar_to_ex(var), xname);
#endif
    return var;
}


cuex_pvar_t (cuex_pvar_from_ex)(cuex_t ex) { return cuex_pvar_from_ex(ex); }
cuex_t (cuex_pvar_to_ex)(cuex_pvar_t var) { return cuex_pvar_to_ex(var); }
cu_bool_t (cuex_pvar_has_offset)(cuex_pvar_t var)
{ return cuex_pvar_has_offset(var); }

void
cuex_pvar_set_module(cuex_pvar_t var, void *M, cu_idr_t idr)
{
    var->module = M;
    var->idr = idr;
}

