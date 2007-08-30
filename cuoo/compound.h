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

#ifndef CUOO_COMPOUND_H
#define CUOO_COMPOUND_H

#include <cuoo/fwd.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuoo_compound_h cuoo/compound.h: Compound Expressions
 *@{\ingroup cuoo_mod */

#define CUOO_COMPOUND_FLAG_COMMUTATIVE 1
#define CUOO_COMPOUND_FLAG_IDEMPOTENT 2

struct cuoo_intf_compound_s
{
    unsigned int flags;

    /* Iteration over subexpressions. */
    size_t itr_size;
    void (*itr_cct)(cuex_t obj, void *itr);
    void *(*itr_next)(void *itr);
    void (*itr_dct)(void *itr);

    /* Incremental construction. */
    size_t ctor_size;
    void (*ctor_cct)(void *ctor);
    void (*ctor_insert)(cuex_t obj, void *ctor);
    cuex_t (*ctor_finish)(void *ctor);

    /* Sequential conjunction. May be implemented in terms of the above. */
    cu_bool_t (*conj)(cuoo_intf_compound_t,
		      cu_clop(f, cu_bool_t, cuex_t), cuex_t);

    /* Image under a function. May be implemented in terms of the above. */
    cuex_t (*image)(cuoo_intf_compound_t,
		    cu_clop(f, cuex_t, cuex_t), cuex_t);
};

void
cuoo_intf_compound_set_itr(cuoo_intf_compound_t type,
			   size_t itr_size,
			   void (*itr_cct)(cuex_t obj, void *itr),
			   void *(*itr_next)(void *itr),
			   void (*itr_dct)(void *itr));

void
cuoo_intf_compound_set_ctor(cuoo_intf_compound_t type,
			    size_t ctor_size,
			    void (*ctor_cct)(void *ctor),
			    void (*ctor_insert)(cuex_t obj, void *ctor),
			    void (*ctor_finish)(void *ctor));

void
cuoo_intf_compound_set_conj(cuoo_intf_compound_t type,
			    cu_bool_t (*conj)(cu_clop(f, cu_bool_t, cuex_t),
					      cuex_t));

void
cuoo_intf_compound_set_image(cuoo_intf_compound_t type,
			     cuex_t (*image)(cu_clop(f, cuex_t, cuex_t),
					     cuex_t));

void
cuoo_intf_compound_finish(cuoo_intf_compound_t type);

/*!@}*/
CU_END_DECLARATIONS

#endif
