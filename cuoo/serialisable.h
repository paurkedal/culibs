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

#ifndef CUOO_SERIALISABLE_H
#define CUOO_SERIALISABLE_H

#include <cuoo/fwd.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuoo_serialisable_h cuoo/serialisable.h: Interface for Serialisable Objects
 *@{\ingroup cuoo_mod */

struct cuoo_libinfo_s
{
    char const *uri;
    int version[8];
};

struct cuoo_intf_serialisable_s
{
    /* These are indexed at the start of the file, then integers are used.
     * Serialisable objects must register themselves under their URI in order
     * to be deserialisable.
     *
     * Examples:
     *     http://www.example.com/projects/mylib-1.2/mytype
     *     http://www.example.com/projects/mylib/mytype-23
     *
     * Or do we split up in origin URI (namespace) and type name?
     */
    cuoo_libinfo_t libinfo;
    char const *type_name;

    int revision;

    void (*write)(void *, FILE *);
    void *(*read)(FILE *, int revision);
};

/*!@}*/
CU_END_DECLARATIONS

#endif
