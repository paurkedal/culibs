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

extern cufo_tagspace_t cufoP_culibs_tagspace;
extern cuoo_stdtype_t cufoP_tagspace_type, cufoP_tag_type;

/*!\defgroup cufo_tag_h cufo/tag.h: Tags for Light-Weight Markup
 *@{\ingroup cufo_mod */

struct cufo_tagspace_s;

CU_SINLINE cufo_tagspace_t cufo_tagspace(char const *url)
{ return (cufo_tagspace_t)cu_idr_by_cstr(url); }

CU_SINLINE cu_idr_t
cufo_tagspace_url_idr(cufo_tagspace_t tagspace)
{ return (cu_idr_t)tagspace; }

CU_SINLINE char const *
cufo_tagspace_url(cufo_tagspace_t tagspace)
{ return cu_idr_to_cstr(cufo_tagspace_url_idr(tagspace)); }

CU_SINLINE cufo_tagspace_t cufo_culibs_tagspace(void)
{ return cufoP_culibs_tagspace; }

struct cufo_tag_s
{
    CUOO_HCOBJ
    cufo_tagspace_t tagspace;
    cu_idr_t idr;
    unsigned int hints;
};

#define CUFO_TAGHINT_BLOCK 1
#define CUFO_TAGHINT_INLINE 2

CU_SINLINE cuoo_type_t
cufo_tag_type()
{ return cuoo_stdtype_to_type(cufoP_tag_type); }

cufo_tag_t
cufo_tag(cufo_tagspace_t tagspace, char const *name, unsigned int hints);

CU_SINLINE cufo_tagspace_t
cufo_tag_space(cufo_tag_t tag)
{ return tag->tagspace; }

CU_SINLINE char const *
cufo_tag_name(cufo_tag_t tag)
{ return cu_idr_to_cstr(tag->idr); }

CU_SINLINE cu_idr_t
cufo_tag_name_idr(cufo_tag_t tag)
{ return tag->idr; }

CU_SINLINE unsigned int
cufo_tag_hints(cufo_tag_t tag)
{ return tag->hints; }

/*!@}*/
CU_END_DECLARATIONS

#endif
