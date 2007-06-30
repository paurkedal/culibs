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

#include <cudyn/type.h>
#include <cu/idr.h>
#include <stdio.h>

cu_clos_def(tuptype_print_elt,
	    cu_prot(cu_bool_t, cu_idr_t label, cu_offset_t bitoff,
			       cudyn_type_t t),
    ( int i;
      FILE *out; ))
{
    cu_clos_self(tuptype_print_elt);
    if (self->i++)
	fputs(" × ", self->out);
    if (label) {
	fputc('~', self->out);
	fputs(cu_idr_to_cstr(label), self->out);
    }
    cu_fprintf(self->out, "@%d:%!", bitoff, t);
    ++self->i;
    return cu_true;
}


cu_bool_t
cudyn_type_print(cudyn_type_t t, FILE *out)
{
    if (cudyn_type_is_tuptype(t)) {
	tuptype_print_elt_t cb;
	cb.i = 0;
	cb.out = out;
	fprintf(out, "#[cudyn_tuptype_t size=%ld: ", (long)cudyn_type_size(t));
	cudyn_tuptype_conj(cudyn_tuptype_from_type(t),
			   tuptype_print_elt_prep(&cb));
	fputc(']', out);
	return cu_true;
    }
    return cu_false;
}

