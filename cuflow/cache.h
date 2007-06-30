/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2006--2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUFLOW_CACHE_H
#define CUFLOW_CACHE_H

#include <cuflow/fwd.h>
#include <cu/thread.h>
#include <cu/clos.h>
#include <cu/inherit.h>
#include <cu/dlink.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuflow_cache_h cuflow/cache.h: Function Call Cache (unfinished)
 *@{\ingroup cuflow_mod
 */

#define CUFLOWP_CACHE_LOG2_BIN_COUNT 5
#define CUFLOWP_CACHE_BIN_COUNT (1 << CUFLOWP_CACHE_LOG2_BIN_COUNT)

/*!Returns a function code identifying slot number \a index in a cache which
 * takes a key of \a key_wsize words size. */
#define CUFLOW_FNCODE(index, key_wsize) (((index) << 16) + (key_wsize))

/*!The key size in words of \a code. */
#define CUFLOW_FNCODE_KEY_SIZEW(code) ((code) & 0xffff)

#define CUFLOW_FNCODE_SLOT(code) ((code) >> 16)

#define CUFLOWP_CACHEOBJ_HDR(obj) ((cuflowP_cacheobjhdr_t)(obj) - 1)

typedef struct cuflowP_cachebin_s *cuflowP_cachebin_t;
typedef struct cuflow_cache_s *cuflow_cache_t;
typedef struct cuflowP_cacheobjhdr_s *cuflowP_cacheobjhdr_t;
typedef struct cuflow_cacheobj_s *cuflow_cacheobj_t;

struct cuflowP_cachebin_s
{
    cu_mutex_t mutex;
    size_t cap;
    size_t size;
    size_t access_since_pruned;
    cuflow_cacheobj_t *link_arr;
};

struct cuflow_cache_s
{
    cu_inherit (cu_dlink_s);
    cuflow_cacheconf_t conf;
    struct cuflowP_cachebin_s bin_arr[CUFLOWP_CACHE_BIN_COUNT];
    cuflow_cacheobj_t (**fn_arr)(cuflow_cacheobj_t key);
};

struct cuflowP_cacheobjhdr_s
{
    cuflow_cacheobj_t next;
    unsigned int access_ticks;
    unsigned long access_function;
    unsigned long gain;
};

/*!The base struct for both cache keys and cache objects. Typically use is to
 * \ref cu_inherit this in the cache key, and cu_inherit the cache key in the
 * full cache object.  This contains one field, \e fncode, which must be
 * assigned an integer obtaind with \ref CUFLOW_FNCODE, which identifies the
 * callback and key size. */
struct cuflow_cacheobj_s
{
    /*!The function code, identifying the slot number of the cache callback
     * and the size of the key, including this struct. */
    cu_word_t fncode;
};

/*!Creates a cache with a unique set of callbacks stored in \a fn_arr.
 * Normally a cache is called at program initialisation and kept for the
 * lifetime of the program, but if you want to despose of a cache, call \ref
 * cuflow_cache_dct, since \a cache will be linked into \a conf. */
void
cuflow_cache_cct(cuflow_cache_t cache, cuflow_cacheconf_t conf,
		 cuflow_cacheobj_t (**fn_arr)(cuflow_cacheobj_t key));

/*!Unlink \a cache from it's configuration. */
void cuflow_cache_dct(cuflow_cache_t cache);

/*!Return a pointer into the data area of a newly allocated cache object.  The
 * cache callbacks must use this to allocate objects and must call \ref
 * cuflow_cacheobj_set_gain before returning them. */
cuflow_cacheobj_t
cuflow_cacheobj_alloc(size_t full_size, cuflow_cacheobj_t key);

/*!Set the gain of \a obj to \a gain.  The gain is an estimate of
 * <i>C</i>/<i>S</i> where <i>C</i> is the cost of computing the retured
 * object in CPU cycles, and <i>S</i> is the size of the returned object in
 * bytes including a fixed overhead of about 5 words.  The <i>C</i> may
 * include cost of using other resources multiplied with suitable constants to
 * make them cycle-equivanent according to the desired resource utilization.
 *
 * These quastities are hard to determine precisely.  Available CPU clocks are
 * typically not precise enough to measure <i>C</i>, and computing <i>S</i>
 * may be expensive for tree-structures or even ambiguous when sharing is
 * involved.  Therefore, rule of thumb estimates will have to do.  Some
 * suggestions:
 * <ul>
 *   <li>If the complexity of the computation is linear in the size of
 *   \a obj, then <i>C</i>/<i>S</i> can be taken to be a constant.  Note that
 *   there is no need to know the size of \a obj, since it cancels out.</li>
 *
 *   <li>If the complexity of the computation is quadratic, make an estimate
 *   of the final size of \a obj and multiply with a constant to get
 *   \a gain.  Assuming that the size can be computed in linear time, the real
 *   computiation will dominate for sufficiently large input.  Alternatively,
 *   time the computation and use the square root to estimate the object size.
 *   If the time is not granular enough, then neglect the quadratic
 *   behaviour.</li>
 * </ul> */
CU_SINLINE void
cuflow_cacheobj_set_gain(cuflow_cacheobj_t obj, float gain)
{ CUFLOWP_CACHEOBJ_HDR(obj)->gain = gain; }

/*!Return the computed object with key-part equal to \a key.  \a key may be a
 * stack object or static storage.  The callback and key size is determined
 * from \e fncode.  The callback is only called if \a cache does not already
 * contain the requested object. */
cuflow_cacheobj_t
cuflow_cache_call(cuflow_cache_t cache, cu_word_t fncode,
		  cuflow_cacheobj_t key);

/*!Template macro for creating the type definitions of a cached function,
 * suited for use with the \c cuflow_cachetab command.  \a NAME is the
 * function name to be passed as the first argument of the CACHENAME_call
 * macro that is generated by <tt>cuflow_cachetab -p CACHENAME ...</tt>. */
#define cuflow_cached_edcl(NAME, key_struct, value_struct)		\
    struct NAME##_key_s							\
    {									\
	cu_inherit (cuflow_cacheobj_s);					\
	cuPP_splice key_struct						\
    };									\
    struct NAME##_obj_s							\
    {									\
	struct NAME##_key_s key;					\
	cuPP_splice value_struct					\
    };									\
    typedef struct NAME##_key_s NAME##_key_t;				\
    typedef struct NAME##_obj_s NAME##_obj_t;

/*!When cache header generated by \c cuflow_cachetab is included, this gives
 * the function index of \a NAME. */
#define cuflow_cached_fncode(NAME)					\
    CUFLOW_FNCODE(NAME##_index, sizeof(NAME##_key_t)/sizeof(cu_word_t));

/*!This is the template for the prototype of the definition of the \a NAME
 * cached function.  This should be followed by a function body surrounded by
 * <tt>{ }</tt>, which receives a parameter \e key.  The function body shall
 * return an object allocated with \ref cuflow_cached_new and tagged with
 * \ref cuflow_cacheobj_set_gain.  A previous \ref cuflow_cached_edcl must be
 * issued. */
#define cuflow_cached_edef(NAME)					\
    NAME##_obj_t *NAME##_fn(NAME##_key_t *key)

/*!Allocate an object suitable for returning from the cache function with name
 * \a NAME.  The returned value is a struct containing the members declared in
 * the \e value_struct parameter of the corresponding \ref cuflow_cached_edcl.
 * If \a extra_size is not 0, this number of extra bytes will be allocated
 * after the \e value_struct fields. */
#define cuflow_cached_new(NAME, extra_size)				\
    ((NAME##_obj_t *)							\
     cuflow_cacheobj_alloc(sizeof(struct NAME##_obj_s) + (extra_size),	\
			   (cuflow_cacheobj_t)(key)))

/*!@}*/
CU_END_DECLARATIONS

#endif
