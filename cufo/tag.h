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

#ifndef CUFO_TAG_H
#define CUFO_TAG_H

#include <cufo/fwd.h>
#include <cuoo/fwd.h>
#include <cu/idr.h>

CU_BEGIN_DECLARATIONS

extern cufo_namespace_t cufoP_culibs_namespace;
extern cuoo_type_t cufoP_namespace_type, cufoP_tag_type;

/*!\defgroup cufo_tag_h cufo/tag.h: Tags for Light-Weight Markup
 *@{\ingroup cufo_mod */

struct cufo_namespace_s;

CU_SINLINE cufo_namespace_t cufo_namespace(char const *url)
{ return (cufo_namespace_t)cu_idr_by_cstr(url); }

CU_SINLINE cu_idr_t
cufo_namespace_url_idr(cufo_namespace_t namespace)
{ return (cu_idr_t)namespace; }

CU_SINLINE char const *
cufo_namespace_url(cufo_namespace_t namespace)
{ return cu_idr_to_cstr(cufo_namespace_url_idr(namespace)); }

CU_SINLINE cufo_namespace_t cufo_culibs_namespace(void)
{ return cufoP_culibs_namespace; }

struct cufo_tag_s
{
    CUOO_HCOBJ
    cufo_namespace_t namespace;
    cu_idr_t idr;
};

CU_SINLINE cuoo_type_t
cufo_tag_type()
{ return cufoP_tag_type; }

cufo_tag_t
cufo_tag(cufo_namespace_t namespace, char const *name);

CU_SINLINE cufo_namespace_t
cufo_tag_space(cufo_tag_t tag)
{ return tag->namespace; }

CU_SINLINE char const *
cufo_tag_name(cufo_tag_t tag)
{ return cu_idr_to_cstr(tag->idr); }

CU_SINLINE cu_idr_t
cufo_tag_name_idr(cufo_tag_t tag)
{ return tag->idr; }

/*!@}*/
CU_END_DECLARATIONS

#endif
