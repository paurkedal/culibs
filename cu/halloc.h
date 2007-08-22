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

#ifndef CU_HCONS_H
#define CU_HCONS_H

#include <cu/fwd.h>
#include <cu/conf.h>
#include <cu/wordarr.h>
#include <cu/oalloc.h>

CU_BEGIN_DECLARATIONS

/*!\defgroup cu_hcons_h cu/halloc.h: Hash-Consing Allocation
 * @{\ingroup cu_mod */

#define CUDYN_HCOBJ_KEY_SIZEW(struct_size) \
    (((struct_size) + CU_WORD_SIZE - 1 - CU_HCOBJ_SHIFT)/CU_WORD_SIZE)
#define CUDYN_HCOBJ_ALLOC_SIZEG(struct_size) \
    CUDYN_OBJ_ALLOC_SIZEG(struct_size)

void *cuexP_halloc_raw(cuex_meta_t meta, size_t key_sizew, void *key);
void *cuexP_halloc_extra_raw(cuex_meta_t meta, size_t raw_alloc_sizeg,
			     size_t key_sizew, void *key,
			     cu_clop(init_nonkey, void, void *));
CU_SINLINE void *
cudynP_halloc_extra_raw(cudyn_type_t type, size_t raw_alloc_sizeg,
			size_t key_sizew, void *key,
			cu_clop(init_nonkey, void, void *))
{
    return cuexP_halloc_extra_raw(cudyn_type_to_meta(type), raw_alloc_sizeg,
				  key_sizew, key, init_nonkey);
}

/*!Assuming a declaration of a struct <tt><em>prefix</em>_s</tt> which starts
 * with a \ref CU_HCOBJ statement, this macro returns the key-size to use for
 * hash-consing object of the struct. */
#define cudyn_hctem_key_size(prefix) (sizeof(struct prefix##_s) - CU_HCOBJ_SHIFT)

/*!This is part of several high-level hash-cosning macros summarised under \ref
 * cudyn_hctem_new.
 * This macro emits a declaration of a key-template used for hash-consed
 * allocation.  \a key must be initialised with \ref cudyn_hctem_init and
 * assigned through the pointer returned by \a cudyn_hctem_get. */
#define cudyn_hctem_decl(prefix, key) char key[cudyn_hctem_key_size(prefix)]

/*!This is part of several high-level hash-consing macros summarised under \ref
 * cudyn_hctem_new.
 * This macro initialises the template \a key, which must be declared with \ref
 * cudyn_hctem_decl.  Use this to zero the template before assigning to the
 * members.  Compiler optimisation typically means only the data which is not
 * subsequently assigned though the \ref cudyn_hctem_get pointer will be
 * zeroed.  In particular, calling \ref cudyn_hctem_init makes sure that
 * padding at the end of the struct and holes in the struct due to alignment
 * constraints do not contain arbitrary data which would invalidate the
 * hash-consing. */
#define cudyn_hctem_init(prefix, key) \
    (memset(&(key), 0, cudyn_hctem_key_size(prefix)))

/*!This is part of several high-level hash-consing macros summarised under \ref
 * cudyn_hctem_new.
 * Given a prefix and a template variable \a key, returns a pointer to the
 * template struct, which has type <tt><em>prefix</em>_s *</tt>. */
#define cudyn_hctem_get(prefix, key)					\
    ((struct prefix##_s *)((char *)(key) - CU_HCOBJ_SHIFT))

/*!The key address of a template in case you need to use \ref cudyn_halloc or
 * \ref cudyn_halloc_extra instead of cudyn_hctem_new. */
#define cudyn_hctem_key(prefix, key) (&(key))

/*!This is part of several high-level hash-consing macros.
 * Given previous definition of a <tt>struct <em>prefix</em>_s</tt> and a
 * function <tt><em>prefix</em>_type()</tt> returning the corresponding dynamic
 * type, this macro returns a hash-constructed object from the initialised
 * template \a key.  This macro is used in conjunction with \ref
 * cudyn_hctem_decl, \ref cudyn_hctem_init, \ref cudyn_hctem_get, and it's
 * maybe best understood from an example, here creating a dynamic pair:
 * \code
 * struct pair_s {
 *     CU_HCOBJ
 *     cuex_t left;
 *     cuex_t right;
 * };
 *
 * cudyn_type_t pair_type();
 *
 * struct pair_s *pair_new(cuex_t left, cuex_t right)
 * {
 *     cudyn_hctem_decl(pair, key);
 *     cudyn_hctem_init(pair, key);
 *     cudyn_hctem_get(pair, key)->left = left;
 *     cudyn_hctem_get(pair, key)->right = right;
 *     return cudyn_hctem_new(pair, key);
 * }
 * \endcode
 * The cudyn_hctem_* macros only work when following the above naming conventions
 * for the struct and dynamic-type functions.  Otherwise, use \ref
 * cudyn_halloc.  */
#define cudyn_hctem_new(prefix, key) \
    ((struct prefix##_s *) \
     cudyn_halloc(prefix##_type(), \
			 cudyn_hctem_key_size(prefix), &(key)))

#define cu_hc_key_hash cu_wordarr_hash

CU_SINLINE cu_hash_t
cu_hash_ptr_arr(void **start, void **end)
{
    cu_hash_t hash = 0;
    while (start != end) {
	hash = cu_hash_mix(hash + (uintptr_t)*start);
	++start;
    }
    return hash;
}

CU_SINLINE cu_bool_t
cu_ptr_arr_eq(void **start0, void **end0, void **start1)
{
    while (start0 != end0) {
	if (*start0 != *start1)
	    return cu_false;
	++start0;
	++start1;
    }
    return cu_true;
}

CU_SINLINE void *
cuexP_halloc(cuex_meta_t meta, size_t key_size, void *key)
{
    return cuexP_halloc_raw(meta,
			    CUDYN_HCOBJ_KEY_SIZEW(key_size + CU_HCOBJ_SHIFT),
			    key);
}

/*!General hash-consing allocation, allowing more control than \ref
 * cudyn_hctem_new. */
CU_SINLINE void *
cudyn_halloc(cudyn_type_t type, size_t key_size, void *key)
{
    return cuexP_halloc_raw(cudyn_type_to_meta(type),
			    CUDYN_HCOBJ_KEY_SIZEW(key_size + CU_HCOBJ_SHIFT),
			    key);
}

/*!General hash-consing allocation with extra non-keyed memory.  This generally
 * takes more GC cycles to clean up and is therefore more expensive than \ref
 * cudyn_halloc, but it allows caching computations and associating properties
 * to live objects. */
CU_SINLINE void *
cudyn_halloc_extra(cudyn_type_t type, size_t struct_size,
		   size_t key_size, void *key,
		   cu_clop(init_nonkey, void, void *obj))
{
    cu_debug_assert(key_size % CU_WORD_SIZE == 0);
    return cudynP_halloc_extra_raw(type,
				   CUDYN_HCOBJ_ALLOC_SIZEG(struct_size),
				   CUDYN_HCOBJ_KEY_SIZEW(key_size
							 + CU_HCOBJ_SHIFT),
				   key,
				   init_nonkey);
}

#define cudyn_hnew_extra(prefix, key_size, key, init_nonkey) \
    ((struct prefix##_s *) \
     cudyn_halloc_extra(prefix##_type(), sizeof(struct prefix##_s), \
			key_size, key, init_nonkey))

/*!@}*/

CU_END_DECLARATIONS

#endif
