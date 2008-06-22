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

#ifndef CUOO_HALLOC_H
#define CUOO_HALLOC_H

#include <cuoo/oalloc.h>
#include <cu/conf.h>
#include <cu/clos.h>
#include <cu/wordarr.h>

CU_BEGIN_DECLARATIONS

/*!\defgroup cuoo_hcons_h cuoo/halloc.h: Hash-Consing Allocation
 * @{\ingroup cuoo_mod */

#define CUOO_HCOBJ_KEY_SIZEW(struct_size) \
    (((struct_size) + CU_WORD_SIZE - 1 - CUOO_HCOBJ_SHIFT)/CU_WORD_SIZE)
#define CUOO_HCOBJ_KEY_SIZE(struct_size) \
    (CUOO_HCOBJ_KEY_SIZEW(struct_size)*CU_WORD_SIZE)
#define CUOO_HCOBJ_ALLOC_SIZEG(struct_size) \
    CUOO_OBJ_ALLOC_SIZEG(struct_size)

#ifndef CU_IN_DOXYGEN
void *cuexP_halloc_raw(cuex_meta_t meta, size_t key_sizew, void *key);
void *cuexP_hxalloc_raw(cuex_meta_t meta, size_t raw_alloc_sizeg,
			size_t key_sizew, void *key,
			cu_clop(init_nonkey, void, void *));
CU_SINLINE void *
cuooP_hxalloc_init_raw(cuoo_type_t type, size_t raw_alloc_sizeg,
		       size_t key_sizew, void *key,
		       cu_clop(init_nonkey, void, void *))
{
    return cuexP_hxalloc_raw(cuoo_type_to_meta(type), raw_alloc_sizeg,
			     key_sizew, key, init_nonkey);
}
void *cuooP_hxalloc_setao_raw(cuoo_type_t type, size_t raw_alloc_sizeg,
			      size_t key_sizew, void *key,
			      cu_offset_t ao_offset, AO_t ao_value);
void *cuooP_hxalloc_clear_raw(cuoo_type_t type, size_t raw_alloc_sizeg,
			      size_t key_sizew, void *key);
#endif

/*!Assuming a declaration of a struct <tt><em>prefix</em>_s</tt> which starts
 * with a \ref CUOO_HCOBJ statement, this macro returns the key-size to use for
 * hash-consing object of the struct. */
#define cuoo_hctem_key_size(prefix) (sizeof(struct prefix##_s) - CUOO_HCOBJ_SHIFT)

/*!This is part of several high-level hash-cosning macros summarised under \ref
 * cuoo_hctem_new.
 * This macro emits a declaration of a key-template used for hash-consed
 * allocation.  \a key must be initialised with \ref cuoo_hctem_init and
 * assigned through the pointer returned by \a cuoo_hctem_get. */
#define cuoo_hctem_decl(prefix, key) char key[cuoo_hctem_key_size(prefix)]

/*!This is part of several high-level hash-consing macros summarised under \ref
 * cuoo_hctem_new.
 * This macro initialises the template \a key, which must be declared with \ref
 * cuoo_hctem_decl.  Use this to zero the template before assigning to the
 * members.  Compiler optimisation typically means only the data which is not
 * subsequently assigned though the \ref cuoo_hctem_get pointer will be
 * zeroed.  In particular, calling \ref cuoo_hctem_init makes sure that
 * padding at the end of the struct and holes in the struct due to alignment
 * constraints do not contain arbitrary data which would invalidate the
 * hash-consing. */
#define cuoo_hctem_init(prefix, key) \
    (memset(&(key), 0, cuoo_hctem_key_size(prefix)))

/*!This is part of several high-level hash-consing macros summarised under \ref
 * cuoo_hctem_new.
 * Given a prefix and a template variable \a key, returns a pointer to the
 * template struct, which has type <tt><em>prefix</em>_s *</tt>. */
#define cuoo_hctem_get(prefix, key)					\
    ((struct prefix##_s *)((char *)(key) - CUOO_HCOBJ_SHIFT))

/*!The key address of a template in case you need to use \ref cuoo_halloc or
 * \ref cuoo_hxalloc_init instead of cuoo_hctem_new. */
#define cuoo_hctem_key(prefix, key) (&(key))

/*!This is part of several high-level hash-consing macros.
 * Given previous definition of a <tt>struct <em>prefix</em>_s</tt> and a
 * function <tt><em>prefix</em>_type()</tt> returning the corresponding dynamic
 * type, this macro returns a hash-constructed object from the initialised
 * template \a key.  This macro is used in conjunction with \ref
 * cuoo_hctem_decl, \ref cuoo_hctem_init, \ref cuoo_hctem_get, and it's
 * maybe best understood from an example, here creating a dynamic pair:
 * \code
 * struct pair_s {
 *     CUOO_HCOBJ
 *     cuex_t left;
 *     cuex_t right;
 * };
 *
 * cuoo_type_t pair_type();
 *
 * struct pair_s *pair_new(cuex_t left, cuex_t right)
 * {
 *     cuoo_hctem_decl(pair, key);
 *     cuoo_hctem_init(pair, key);
 *     cuoo_hctem_get(pair, key)->left = left;
 *     cuoo_hctem_get(pair, key)->right = right;
 *     return cuoo_hctem_new(pair, key);
 * }
 * \endcode
 * The cuoo_hctem_* macros only work when following the above naming conventions
 * for the struct and dynamic-type functions.  Otherwise, use \ref
 * cuoo_halloc.  */
#define cuoo_hctem_new(prefix, key) \
    ((struct prefix##_s *) \
     cuoo_halloc(prefix##_type(), cuoo_hctem_key_size(prefix), &(key)))

#define cuoo_hctem_new_of_type(prefix, key, type) \
    ((struct prefix##_s *) \
     cuoo_halloc(type, cuoo_hctem_key_size(prefix), &(key)))

#ifndef CU_IN_DOXYGEN
CU_SINLINE void *
cuexP_halloc(cuex_meta_t meta, size_t key_size, void *key)
{
    return cuexP_halloc_raw(meta,
			    CUOO_HCOBJ_KEY_SIZEW(key_size + CUOO_HCOBJ_SHIFT),
			    key);
}
#endif

/*!General hash-consing allocation, allowing more control than \ref
 * cuoo_hctem_new. */
CU_SINLINE void *
cuoo_halloc(cuoo_type_t type, size_t key_size, void *key)
{
    return cuexP_halloc_raw(cuoo_type_to_meta(type),
			    CUOO_HCOBJ_KEY_SIZEW(key_size + CUOO_HCOBJ_SHIFT),
			    key);
}

/*!General hash-consing allocation with extra non-keyed memory.  This generally
 * takes more GC cycles to clean up and is therefore more expensive than \ref
 * cuoo_halloc, but it allows caching computations and associating properties
 * to live objects. */
CU_SINLINE void *
cuoo_hxalloc_init(cuoo_type_t type, size_t struct_size,
		  size_t key_size, void *key,
		  cu_clop(init_nonkey, void, void *obj))
{
    cu_debug_assert(key_size % CU_WORD_SIZE == 0);
    return cuooP_hxalloc_init_raw(
	type, CUOO_HCOBJ_ALLOC_SIZEG(struct_size),
	CUOO_HCOBJ_KEY_SIZEW(key_size + CUOO_HCOBJ_SHIFT),
	key, init_nonkey);
}

/*!As \ref cuoo_hxalloc_init specialised with initialisation that simply sets
 * an \a AO_t typed field at offset \a oa_offset to \a ao_value. */
CU_SINLINE void *
cuoo_hxalloc_setao(cuoo_type_t type, size_t struct_size,
		   size_t key_size, void *key,
		   cu_offset_t ao_offset, AO_t ao_value)
{
    cu_debug_assert(key_size % CU_WORD_SIZE == 0);
    return cuooP_hxalloc_setao_raw(
	type, CUOO_HCOBJ_ALLOC_SIZEG(struct_size),
	CUOO_HCOBJ_KEY_SIZEW(key_size + CUOO_HCOBJ_SHIFT),
	key, ao_offset, ao_value);
}

/*!As \ref cuoo_hxalloc_init specialised with initialisation that only clear
 * the non-key data. */
CU_SINLINE void *
cuoo_hxalloc_clear(cuoo_type_t type, size_t struct_size,
		   size_t key_size, void *key)
{
    cu_debug_assert(key_size % CU_WORD_SIZE == 0);
    return cuooP_hxalloc_clear_raw(
	type, CUOO_HCOBJ_ALLOC_SIZEG(struct_size),
	CUOO_HCOBJ_KEY_SIZEW(key_size + CUOO_HCOBJ_SHIFT), key);
}

/*!Macro to call \ref cuoo_hxalloc_init based on an identifier prefix.  It is
 * assumed that the following are defined:
 * <ul>
 *   <li><tt>struct <i>prefix</i>_s { CU_HCOBJ ... }</tt> — The struct of the
 *     object, used for size calculation and the return type.</li>
 *   <li><tt>cuoo_type_t <i>prefix</i>_type(void)</tt> — A function or
 *     function-like macro which shall return the dynamic type of the
 *     object.</li>
 * </ul> */
#define cuoo_hxnew_init(prefix, key_size, key, init_nonkey) \
    ((struct prefix##_s *) \
     cuoo_hxalloc_init(prefix##_type(), sizeof(struct prefix##_s), \
		       key_size, key, init_nonkey))

/*!Macro to call \ref cuoo_hxalloc_setao based on an indentifier prefix.  See
 * \ref cuoo_hxnew_init for details. */
#define cuoo_hxnew_setao(prefix, key_size, key, ao_offset, ao_value) \
    ((struct prefix##_s *) \
     cuoo_hxalloc_setao(prefix##_type(), sizeof(struct prefix##_s), \
			key_size, key, ao_offset, ao_value))

/*!Macro to call \ref cuoo_hxalloc_clear based on an indentifier prefix.  See
 * \ref cuoo_hxnew_init for details. */
#define cuoo_hxnew_clear(prefix, key_size, key) \
    ((struct prefix##_s *) \
     cuoo_hxalloc_clear(prefix##_type(), sizeof(struct prefix##_s), \
			key_size, key))

/*!@}*/

#if CU_COMPAT < 20080207
#  define cuoo_halloc_extra		cuoo_hxalloc_init
#  define cuoo_halloc_extra_setao	cuoo_hxalloc_setao
#  define cuoo_hnew_extra		cuoo_hxnew_init
#  define cuoo_hnew_extra_setao		cuoo_hxnew_setao
#endif

CU_END_DECLARATIONS

#endif
