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

#ifndef CUFO_ATTR_H
#define CUFO_ATTR_H

#include <cufo/fwd.h>
#include <cucon/fwd.h>
#include <cu/idr.h>
#include <cu/box.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cufo_attr_h cufo/attr.h: Markup Attributes
 *@{\ingroup cufo_mod */

typedef enum {
    cufo_attrtype_fixed,
    cufo_attrtype_enum,
    cufo_attrtype_cstr,
    cufo_attrtype_int,
} cufo_attrtype_t;

struct cufo_attr
{
    cufo_namespace_t ns;
    cu_idr_t idr;
    cufo_attrtype_t type;
    union {
	char const *fixed_value;
	char const *(*enum_name)(int);
    } extra;
};

void cufo_attr_init(cufo_attr_t attr, cufo_namespace_t ns,
		    char const *name, cufo_attrtype_t type);

void cufo_attr_init_fixed(cufo_attr_t attr, cufo_namespace_t ns,
			  char const *name, char const *val);

void cufo_attr_init_enum(cufo_attr_t attr, cufo_namespace_t ns,
			 char const *name, char const *(*enum_name)(int));

CU_SINLINE char const *
cufo_attr_name(cufo_attr_t attr)
{ return cu_idr_to_cstr(attr->idr); }

CU_SINLINE cufo_attrtype_t
cufo_attr_type(cufo_attr_t attr)
{ return attr->type; }

CU_SINLINE cu_box_t cufoP_cktype_int(int i)
{ return cu_box_int(i); }
CU_SINLINE cu_box_t cufoP_cktype_cstr(char const *s)
{ return cu_box_ptr(char *, (char *)s); }

#define CUFO_ATTR_INT(attr, i)	attr, cufoP_cktype_int(i)
#define CUFO_ATTR_CSTR(attr, s)	attr, cufoP_cktype_cstr(s)
#define CUFO_ATTR_ENUM(attr, i)	attr, cufoP_cktype_int(i)

struct cufo_attrbind
{
    cufo_attr_t attr;
    cu_box_t value;
};

/*!@}*/
CU_END_DECLARATIONS

#endif
