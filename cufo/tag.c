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

#include <cufo/tag.h>
#include <cuoo/type.h>
#include <cuoo/hctem.h>
#include <cuoo/intf.h>
#include <cu/str.h>

cufo_namespace_t cufoP_culibs_namespace;
cuoo_type_t cufoP_tag_type;

cufo_tag_t
cufo_tag(cufo_namespace_t ns, char const *name)
{
    cufo_tag_t tag;
    cuoo_hctem_decl(cufo_tag, tem);
    cuoo_hctem_init(cufo_tag, tem);
    tag = cuoo_hctem_get(cufo_tag, tem);
    tag->ns = ns;
    tag->idr = cu_idr_by_cstr(name);
    return cuoo_hctem_new(cufo_tag, tem);
}

static cu_str_t
_tag_to_str(cuex_t tag)
{
    return cu_str_new_fmt("<%s/>", cufo_tag_name(tag));
}

static cu_box_t
_tag_dispatch(cu_word_t intf_number, ...)
{
    switch (intf_number) {
	case CUOO_INTF_TO_STR_FN:
	    return CUOO_INTF_TO_STR_FN_BOX(_tag_to_str);
	default:
	    return CUOO_IMPL_NONE;
    }
}

void
cufoP_tag_init()
{
    cufoP_tag_type = cuoo_type_new_opaque_hcs(
	_tag_dispatch, sizeof(struct cufo_tag) - CUOO_HCOBJ_SHIFT);
    cufoP_culibs_namespace = cufo_namespace("http://www.eideticdew.org/culibs");
}
