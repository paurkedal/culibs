/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2009  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUOO_HCTEM_H
#define CUOO_HCTEM_H

#include <cuoo/halloc.h>

CU_BEGIN_DECLARATIONS
/** \defgroup cuoo_hctem_h cuoo/hctem.h: Boilerplates for Hash-Consing
 ** @{ \ingroup cuoo_mod
 **
 ** These macros simplifies the allocation of hash-consed objects.  They are
 ** documented individually below, but how they work together is maybe best
 ** understood by a simple example, here the creation of a dynamically typed
 ** and hash-consed pair:
 **
 ** \code
 ** struct pair {
 **     CUOO_HCOBJ
 **     cuex_t left;
 **     cuex_t right;
 ** };
 **
 ** cuoo_type_t pair_type();
 **
 ** struct pair *pair_new(cuex_t left, cuex_t right)
 ** {
 **     cuoo_hctem_decl(pair, key);
 **     cuoo_hctem_init(pair, key);
 **     cuoo_hctem_get(pair, key)->left = left;
 **     cuoo_hctem_get(pair, key)->right = right;
 **     return cuoo_hctem_new(pair, key);
 ** }
 ** \endcode
 **
 ** The <tt>cuoo_hctem_*</tt> macros only work when following the above naming
 ** conventions for the struct and dynamic-type functions.  Otherwise, use the
 ** generic functions from \ref cuoo_halloc_h "cuoo/halloc.h".
 **/

/** Assuming a declaration of <tt>struct <em>prefix</em></tt> which starts
 ** with a \ref CUOO_HCOBJ statement, this macro returns the key-size to use
 ** for hash-consing object of the struct. */
#define cuoo_hctem_key_size(prefix) (sizeof(struct prefix) - CUOO_HCOBJ_SHIFT)

/** This macro emits a declaration of a key-template used for hash-consed
 ** allocation.  \a key must be initialised with \ref cuoo_hctem_init and
 ** assigned through the pointer returned by \a cuoo_hctem_get. */
#ifdef CUOO_HCTEM_EXCLUDE_HCOBJ_SHIFT
#  define cuoo_hctem_decl(prefix, key) char key[cuoo_hctem_key_size(prefix)]
#else
#  define cuoo_hctem_decl(prefix, tem) struct prefix tem
#endif

/** This macro initialises the template \a key, which must be declared with
 ** \ref cuoo_hctem_decl.  This macro should be invoked before assigning to the
 ** struct fields to ensure a predictable hash-key.  In particular, calling
 ** \ref cuoo_hctem_init zeros any padding at the end of the struct or holes
 ** within the struct due to alignment constraints, so that they do not contain
 ** arbitrary data which would affect the hash key.
 **
 ** Inspection of generated code generated by GCC at <tt>-O2</tt> suggests that
 ** it's able to optimise the cuoo_hctem_init invokation against subsequent
 ** unconditional \ref cuoo_hctem_get invocations. */
#define cuoo_hctem_init(prefix, key) \
	(memset(cuoo_hctem_key(prefix, key), 0, cuoo_hctem_key_size(prefix)))

/** Given a prefix and a template variable \a key, returns a pointer to the
 ** template struct, which has type <tt><em>prefix</em>_s *</tt>. */
#ifdef CUOO_HCTEM_EXCLUDE_HCOBJ_SHIFT
#  define cuoo_hctem_get(prefix, key)					\
	((struct prefix *)((char *)(key) - CUOO_HCOBJ_SHIFT))
#else
#  define cuoo_hctem_get(prefix, key) (&(key))
#endif

/** The address of the key of a template, in case you need to use \ref
 ** cuoo_halloc or \ref cuoo_hxalloc_init instead of cuoo_hctem_new. */
#ifdef CUOO_HCTEM_EXCLUDE_HCOBJ_SHIFT
#  define cuoo_hctem_key(prefix, key) (&(key))
#else
#  define cuoo_hctem_key(prefix, tem) cu_ptr_add(&(tem), CUOO_HCOBJ_SHIFT)
#endif

/** Given previous definition of a <tt>struct <em>prefix</em>_s</tt> and a
 ** function <tt><em>prefix</em>_type()</tt> returning the corresponding
 ** dynamic type, this macro returns a hash-consed object from the
 ** pre-initialised template \a key.  This macro is used in conjunction with
 ** \ref cuoo_hctem_decl, \ref cuoo_hctem_init, \ref cuoo_hctem_get. */
#define cuoo_hctem_new(prefix, key) \
    ((struct prefix *) \
     cuoo_halloc(prefix##_type(), cuoo_hctem_key_size(prefix), \
		 cuoo_hctem_key(prefix, key)))

#define cuoo_hctem_new_of_type(prefix, key, type) \
    ((struct prefix *) \
     cuoo_halloc(type, cuoo_hctem_key_size(prefix), \
		 cuoo_hctem_key(prefix, key)))


/** @} */
CU_END_DECLARATIONS

#endif
