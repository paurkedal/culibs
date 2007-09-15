/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2004--2007  Petter Urkedal <urkedal@nbi.dk>
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

/* Portability
 * -----------
 *
 * Consider the three declarations
 *
 *   RESULT_TYPE f(ARGLIST);
 *   RESULT_TYPE g(ARGLIST, void *carg);
 *   typedef RESULT_TYPE (*h)(ARGLIST, ...);
 *
 * where RESULT_TYPE is an arbitrary type and ARGLIST is an arbitrary
 * list of argument declarations.  For the following hack to work,
 *
 *   (1) 'f' must be callable when cast to 'h' with an extra pointer
 *       argument and simply ignore the last argument.
 *
 *   (2) 'g' must be callable when cast to 'h' and carg passed in the
 *       first vararg position.
 *
 *   (3) A function pointer must be preserved when converted to
 *       '(void *)' and back.
 *
 * Considering that C allows calls to functions with no prototype
 * declaration, including vararg, assumptions (1) and (2) must hold
 * given a plain implementation that does not include runtime
 * information about the actual prototypes.
 *
 * Assumption (3) is only used to cast away the prototype in
 * 'cu_clos_cct' and can be avoided if necessary.
 *
 *
 * Type Safety
 * -----------
 * 
 * 'cu_clos_cct' and 'cu_nonclos_cct' are not type-safe.  Other
 * functions are type-safe, except for a reasonably unlikely
 * coincidence with pointers to pointers to functions.
 */

#ifndef CU_CLOS_BITS_H
#define CU_CLOS_BITS_H

#include <cu/fwd.h>
#include <cu/conf.h>

CU_BEGIN_DECLARATIONS

#define cuP_clos_base(clos_s, result_t, alops, argl) \
	result_t (*cuP_clos_fn)alops##append(argl, void *)
#define cuP_clos_cct(clos, fn) ((clos)->cuP_clos_fn = (void *)(fn))
#ifdef CUCONF_ENABLE_CLOSURE_TYPE_HACK
#  define cuP_clos_ref(clos) &(clos)->cuP_clos_fn
#else
#  define cuP_clos_ref(clos) ((void *)(clos))
#endif

#define cuP_clos_formal(clos_s, alops, argl) \
	alops##append(argl, struct clos_s *cuP_clarg)
#define cuP_clos_arg_dcln(clos_s, var) struct clos_s *var = cuP_clarg

#define cuP_clop_def(PFX, linkage, res_t, alops, argl)			\
    cuP_clos_dec(PFX, res_t, alops, argl, ())				\
    static PFX##_t PFX##__clos = {					\
	(res_t (*)alops##append(argl, void *))PFX##_fn			\
    };									\
    linkage cuP_clop(PFX, res_t, alops, argl) = cuP_clos_ref(&PFX##__clos); \
    linkage cuP_clos_fun(PFX, res_t, alops, argl)
#define cu_clop_def(clop, res_t, argl...)				\
    cuP_clop_def(clop, static, res_t, cuPP_argl_, (argl))
#define cu_clop_def0(clop, res_t)					\
    cuP_clop_def(clop, static, res_t, cuPP_argl0_, ())
#define cu_clop_edef(clop, res_t, argl...)				\
    cuP_clop_def(clop,, res_t, cuPP_argl_, (argl))
#define cu_clop_edef0(clop, res_t)					\
    cuP_clop_def(clop,, res_t, cuPP_argl0_, ())

#define cu_clop_call_fn(clop, argl...) clop##_fn(argl, NULL)
#define cu_clop_call_fn0(clop) clop##_fn(NULL)

/* Pointers to closures. */
#define cuP_clop(fn, result_t, alops, argl)			\
    result_t (*const *fn)alops##append(argl, void *)
#define cu_clop_null (NULL)
#define cu_clop_is_null(clop) ((clop) == NULL)

#define cu_call(clop, args...) ((**(clop))(args, (void *)(clop)))
#define cu_call0(clop) ((**(clop))((void *)(clop)))

CU_END_DECLARATIONS

#endif
