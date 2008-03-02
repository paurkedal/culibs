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

#ifndef CU_CLOS_H
#define CU_CLOS_H

#include <cu/fwd.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cu_clos_h cu/clos.h:
 *@{\ingroup cu_mod */

#define cuPP_splice(argl...) argl
#define cuPP_argl_append(argl, arg) (cuPP_splice argl, arg)
#define cuPP_argl_prepend(argl, arg) (arg, cuPP_splice argl)
#define cuPP_argl0_append(argl, arg) (arg)
#define cuPP_argl0_prepend(argl, arg) (arg)

typedef struct cu_clos_self_s *cu_clos_self_t;

#if 1
#  define cuP_jargl(alops, argl, self) alops##append(argl, self)
#else
#  define cuP_jargl(alops, argl, self) alops##prepend(argl, self)
#endif

#define cuP_clos_formal(alops, argl) \
    cuP_jargl(alops, argl, cu_clos_self_t cuL_self)
#define cuP_xargl(argl...) cuP_jargl(cuPP_argl_, argl, cu_clos_self_t cuL_self)
#define cuP_xargl0() cu_clos_self_t cuL_self

/* Closure Pointers */

#define cu_clop(val, res_t, parl...)	res_t (**val)(parl, cu_clos_self_t)
#define cu_clop0(val, res_t)		res_t (**val)(cu_clos_self_t)

#define cu_clop_null (NULL)
#define cu_clop_is_null(clop) ((clop) == NULL)

#define cu_call(clop, args...) \
    ((**(clop))cuP_jargl(cuPP_argl_, (args), (cu_clos_self_t)(clop)))
#define cu_call0(clop) \
    ((**(clop))((cu_clos_self_t)(clop)))

#define cuP_clop_def(name, linkage, res_t, alops, xparl)		\
    linkage res_t name##_fn cuP_clos_formal(alops, xparl);		\
    linkage res_t (*name##_data) cuP_clos_formal(alops, xparl) = &name##_fn; \
    linkage res_t (**name) cuP_clos_formal(alops, xparl) = &name##_data; \
    linkage res_t name##_fn cuP_clos_formal(alops, xparl)
#define cu_clop_ref(name) (&name##_data)

#define cu_clop_def(name, res_t, parl...) \
    cuP_clop_def(name, static, res_t, cuPP_argl_, (parl))
#define cu_clop_def0(name, res_t) \
    cuP_clop_def(name, static, res_t, cuPP_argl0_, ())
#define cu_clop_edef(name, res_t, parl...) \
    cuP_clop_def(name,       , res_t, cuPP_argl_, (parl))
#define cu_clop_edef0(name, res_t) \
    cuP_clop_def(name,       , res_t, cuPP_argl0_, ())

#define cu_clop_call_fn(clop, argl...) clop##_fn(argl, NULL)
#define cu_clop_call_fn0(clop, argl...) clop##_fn(NULL)

/* Closures Proper */

#define cuP_clos_dec(name, linkage, cargs, res_t, alops, parl)		\
    typedef struct name##_s name##_t;					\
    struct name##_s {							\
	res_t (*cuL_fn) cuP_clos_formal(alops, parl);			\
	cuPP_splice cargs						\
    };									\
									\
    typedef res_t (**name##_clop_t) cuP_clos_formal(alops, parl);	\
									\
    linkage res_t name##_fn cuP_jargl(alops, parl, name##_t *);		\
									\
    CU_SINLINE void name##_cct(name##_t *self)				\
    { self->cuL_fn = (res_t (*)cuP_clos_formal(alops, parl))name##_fn;}	\
									\
    CU_SINLINE name##_clop_t name##_prep(name##_t *self)		\
    { name##_cct(self); return &self->cuL_fn; }				\
									\
    CU_SINLINE name##_clop_t name##_ref(name##_t *self)			\
    { return &self->cuL_fn; }

#define cuP_clos_fun(name, linkage, res_t, alops, parl)			\
    res_t name##_fn cuP_jargl(alops, parl, name##_t *cuL_self)

#define cu_clos_dec(name, prot, cargs)	cuP_clos_dec(name, static, cargs, prot)
#define cu_clos_edec(name, prot, cargs) cuP_clos_dec(name, extern, cargs, prot)
#define cu_clos_fun(name, prot)		cuP_clos_fun(name, static, prot)
#define cu_clos_efun(name, prot)	cuP_clos_fun(name, extern, prot)

#define cu_clos_def(name, prot, cargs)					\
    cuP_clos_dec(name, static, cargs, prot)				\
    cuP_clos_fun(name, static, prot)

#define cu_prot(res_t, parl...)	res_t, cuPP_argl_, (parl)
#define cu_prot0(res_t)		res_t, cuPP_argl0_, ()

#define cu_clos_self(name) name##_t *self = cuL_self

/*!@}*/
CU_END_DECLARATIONS

#endif
