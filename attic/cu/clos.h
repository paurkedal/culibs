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

#ifndef CU_CLOS_H
#define CU_CLOS_H

#include <cu/fwd.h>

CU_BEGIN_DECLARATIONS

/* Preprocessor Tools
 * ================== */

#define cuPP_splice(argl...) argl
#define cuPP_argl_append(argl, arg) (cuPP_splice argl, arg)
#define cuPP_argl_prepend(argl, arg) (arg, cuPP_splice argl)
#define cuPP_argl0_append(argl, arg) (arg)
#define cuPP_argl0_prepend(argl, arg) (arg)


/* Implementation
 * ============== */

/* The definition of a variant must provide
 *
 *     cuP_clos_formal(clos_s, alops, argl)
 *         Expands to the formal argument list, given
 *             'clos_s'	the struct name of the closure
 *             'alops'	the prefix to the argument list macros, either
 *                      'cuPP_argl_' or 'cuPP_argl0_'.
 *             'argl'	the formal argument list.
 *     cuP_clos_base(clos_s, result_t, alops, argl)
 *     cuP_clos_cct(clos, fn)
 *     cuP_clos_ref(clos)
 *     cuP_clos_arg_dcln(clos_s, var)
 *
 *     cu_clop_null
 *     cu_clop_is_null(clptr)
 *     cu_clop_def(clop, res_t, argl...)
 *     cu_clop_def0(clop, res_t)
 *     cu_clop_edef(clop, res_t, argl...)
 *     cu_clop_edef0(clop, res_t)
 *
 *     cu_call(clop, args...)
 *     cu_call0(clop)
 */
#include <cu/conf.h>
#define CU_CLOS_VARIANT_GENERIC	1
#define CU_CLOS_VARIANT_I386		2
#if CUCONF_CLOS_VARIANT == CU_CLOS_VARIANT_GENERIC
#  include <cu/generic/clos.h>
#elif CUCONF_CLOS_VARIANT == CU_CLOS_VARIANT_I386
#  include <cu/i386/clos.h>
#else
#  error Missing a valid definition of CUCONF_CLOS_VARIANT in config.h.
#endif

#define cu_clos_base(self_s, proto) cuP_clos_base(self_s, proto)
#define cu_clos_formal(self_s, args...)					\
    cuPP_splice cuP_clos_formal(self_s, cuPP_argl_, (args))
#define cu_clos_formal0(self_s)						\
    cuPP_splice cuP_clos_formal(self_s, cuPP_argl0_, ())


/* High-level interface
 * ==================== */

#define cuP_clos_common(PFX, linkage, result_t, alops, argl, cargs)	\
    struct PFX##_s {							\
	cuP_clos_base (PFX##_s, result_t, alops, argl);			\
	cuPP_splice cargs						\
    };									\
    typedef struct PFX##_s PFX##_t;					\
    typedef cuP_clop(PFX##_clop_t, result_t, alops, argl);		\
									\
    linkage result_t PFX##_fn cuP_clos_formal(PFX##_s, alops, argl);	\
							    		\
    CU_SINLINE void PFX##_cct(PFX##_t *clos)				\
    {									\
	cuP_clos_cct(clos, PFX##_fn);					\
    }									\
									\
    CU_SINLINE PFX##_clop_t PFX##_prep(PFX##_t *clos)			\
    {									\
	cuP_clos_cct(clos, PFX##_fn);					\
	return (PFX##_clop_t)clos;					\
    }									\
									\
    CU_SINLINE PFX##_clop_t PFX##_ref(PFX##_t *clos)			\
    {									\
	return (PFX##_clop_t)clos;					\
    }

/*!Forward declare and define closure struct for a closure with static
 * linkage. */
#define cu_clos_dec(PFX, proto, cargs) cuP_clos_dec(PFX, proto, cargs)
#define cuP_clos_dec(PFX, result_t, alops, argl, cargs)		\
    cuP_clos_common(PFX, static, result_t, alops, argl, cargs)

/*!Provide the function definition for a closure which was previously
 * declared with \ref cu_clos_dec. */
#define cu_clos_fun(PFX, proto) cuP_clos_fun(PFX, proto)
#define cuP_clos_fun(PFX, result_t, alops, argl)			\
    result_t PFX##_fn cuP_clos_formal(PFX##_s, alops, argl)

/*!Forward declare and define closure struct for a closure with extern
 * linkage. */
#define cu_clos_edec(PFX, proto, cargs) cuP_clos_edecl(PFX, proto, cargs)
#define cuP_clos_edecl(PFX, result_t, alops, argl, cargs)		\
    cuP_clos_common(PFX, extern, result_t, alops, argl, cargs)

/*!Provide the function definition for a closure which was previously
 * declared with \ref cu_clos_edec. */
#define cu_clos_efun(PFX, proto) cuP_clos_efun(PFX, proto)
#define cuP_clos_efun(PFX, result_t, alops, argl)			\
    result_t PFX##_fn cuP_clos_formal(PFX##_s, alops, argl)

/*!Define a closure (function and struct) with static linkage which has
 * no previous forward declaration. */
#define cu_clos_def(PFX, proto, cargs)					\
    cuP_clos_dec(PFX, proto, cargs)					\
    cuP_clos_fun(PFX, proto)

#define cu_prot(res_t, argl...) res_t, cuPP_argl_, (argl)
#define cu_prot0(res_t) res_t, cuPP_argl0_, ()

#define cu_clop(cptr, result_t, argl...) \
    cuP_clop(cptr, result_t, cuPP_argl_, (argl))
#define cu_clop0(cptr, result_t) \
    cuP_clop(cptr, result_t, cuPP_argl0_, ())

#define cu_clos_self(PFX) cuP_clos_arg_dcln(PFX##_s, self)

/* A generic pointer. Must be cast before call. */
typedef cu_clop0(cu_clop_generic_t, void);

CU_END_DECLARATIONS

#endif
