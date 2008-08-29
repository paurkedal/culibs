/* Part of the fform project, <http://www.eideticdew.org/culibs/>.
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

#include <cuex/str_algo.h>
#include <cuex/opn.h>
#include <cucon/stack.h>
#include <cu/buffer.h>
#include <cu/idr.h>
#include <cu/str.h>
#include <string.h>

cu_bool_t
cuex_left_idrjoin_charbuf(cu_buffer_t buf, cuex_meta_t opr, cuex_t e,
			  char const *delim)
{
    char const *name;
    struct cucon_stack_s stack;
    size_t delim_len = strlen(delim);
    cucon_stack_init(&stack);

    while (cuex_meta(e) == opr) {
	cu_idr_t idr = cuex_opn_at(e, 1);
	if (!cu_is_idr(idr))
	    return cu_false;
	cucon_stack_push_ptr(&stack, idr);
	e = cuex_opn_at(e, 0);
    }
    cu_buffer_init(buf, 32);
    if (!cu_is_idr(e))
	return cu_false;
    name = cu_idr_to_cstr(e);
    cu_buffer_write(buf, name, strlen(name));
    while (!cucon_stack_is_empty(&stack)) {
	cu_idr_t idr = cucon_stack_pop_ptr(&stack);
	name = cu_idr_to_cstr(idr);
	cu_buffer_write(buf, delim, delim_len);
	cu_buffer_write(buf, name, strlen(name));
    }
    return cu_true;
}

cu_str_t
cuex_left_idrjoin_str(cuex_meta_t opr, cuex_t e, char const *delim)
{
    struct cu_buffer_s buf;
    if (cuex_left_idrjoin_charbuf(&buf, opr, e, delim))
	return cu_str_new_charr(cu_buffer_content_start(&buf),
				cu_buffer_content_size(&buf));
    else
	return NULL;
}

char const *
cuex_left_idrjoin_cstr(cuex_meta_t opr, cuex_t e, char const *delim)
{
    struct cu_buffer_s buf;
    size_t len;
    char *cstr;

    if (cuex_left_idrjoin_charbuf(&buf, opr, e, delim)) {
	len = cu_buffer_content_size(&buf);
	cstr = cu_galloc(len + 1);
	memcpy(cstr, cu_buffer_content_start(&buf), len);
	cstr[len] = 0;
	return cstr;
    }
    else
	return NULL;
}
