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

#ifndef CUDYN_PROP_H
#define CUDYN_PROP_H

#include <cuex/fwd.h>
#include <cudyn/fwd.h>
#include <cucon/pmap.h>
#include <cu/rarex.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cudyn_prop cudyn/prop.h: Thread-Safe Properties
 * @{ \ingroup cudyn_mod
 *
 * \note Storing the property globally will prevent the expressions
 * and values from being recycled by the garbage collector.  For variables
 * the alternative is to use \c cuex_pvar_t which has internally stored
 * properties.  Local properties which is used within a single thread is
 * more efficiently stored in a \c cucon_pmap_t.  */

struct cudyn_prop_s
{
    struct cucon_pmap_s pmap;
    cu_rarex_t rarex;
};

/* Construct a property. */
void cudyn_prop_cct(cudyn_prop_t prop);
cudyn_prop_t cudyn_prop_new(void);


/* Easy Interface
 * --------------
 *
 * Set pre-constructed properties and get them without worrying about
 * locking.  No unlocked modification to the value after setting it, but you
 * can replace it. */

/*!Sets a pointer value for \a prop of \a ex.  Returns true iff the
 * property did not exist in advance for \a ex. */
cu_bool_t cudyn_prop_replace_ptr(cudyn_prop_t key, cuex_t ex, void *value);

/*!Sets a pointer value for \a prop of \a ex and returns true iff the
 * property did not exist in advance for \a ex. */
cu_bool_t cudyn_prop_condset_ptr(cudyn_prop_t key, cuex_t ex, void *value);

/*!Sets a pointer value for \a prop of \a ex.  Fails with an error if
 * property exists for \a ex. */
void cudyn_prop_define_ptr(cudyn_prop_t key, cuex_t ex, void *value);

/*!Returns the value for \a prop of \a ex, assuming the slot contains a
 * pointer. */
void *cudyn_prop_get_ptr(cudyn_prop_t key, cuex_t ex);


/* Property-Slot Interface
 * -----------------------
 *
 * Set and modify properties in internal value slots at the expense of
 * dealing with locking.  */

/*!Set '*slot' to point to the property \a prop of \a ex, allocating a
 * slot of size \a size if it does not yet exist.  Returns true iff the
 * property does not exist, but locks \a prop in either case. */
cu_bool_t cudyn_prop_set_mem_lock(cudyn_prop_t key, cuex_t ex,
				  size_t size, cu_ptr_ptr_t slot);

/*!Set '*slot' to point to the property \a prop of \a ex, allocating a
 * slot of size \a size if it does not yet exist.  Returns true and locks
 * \a prop iff the property does not exists.  */
cu_bool_t cudyn_prop_set_mem_condlock(cudyn_prop_t key, cuex_t ex,
				      size_t size, cu_ptr_ptr_t slot);

/*!Call this when you are done modifying a slot obtained by a call to
 * \ref cudyn_prop_set_mem_lock or a true-returning call to
 * \ref cudyn_prop_set_mem_condlock. */
CU_SINLINE void cudyn_prop_set_mem_unlock(cudyn_prop_t key, cuex_t ex)
{ cu_rarex_unlock_write(&key->rarex); }

/*!Return property \a key of \a ex and lock \a key for reading even if NULL
 * is returned. */
void *cudyn_prop_get_mem_lock(cudyn_prop_t key, cuex_t ex);

/*!If \a ex has a property \a key, (read-)lock it and return a pointer
 * to its slot. */
void *cudyn_prop_get_mem_condlock(cudyn_prop_t key, cuex_t ex);

/*!Call this after reading a slot returned by a call to
 * \ref cudyn_prop_get_mem_condlock which returned non-NULL. */
CU_SINLINE void cudyn_prop_get_mem_unlock(cudyn_prop_t key, cuex_t ex)
{ cu_rarex_unlock_read(&key->rarex); }

/*!@}*/
CU_END_DECLARATIONS

#endif
