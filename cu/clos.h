/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2008--2018  Petter A. Urkedal <paurkedal@gmail.com>
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
#include <cu/util.h>

#ifndef CU_IN_DOXYGEN /* Put documentation in cu/clos.doxy */
#define CU_CLOS_CONTEXT_FIRST 1

CU_BEGIN_DECLARATIONS

#define cuPP_splice(...) __VA_ARGS__
#define cuPP_argl_append(argl, arg) (cuPP_splice argl, arg)
#define cuPP_argl_prepend(argl, arg) (arg, cuPP_splice argl)
#define cuPP_argl0_append(argl, arg) (arg)
#define cuPP_argl0_prepend(argl, arg) (arg)

typedef struct cu_clos_self_s *cu_clos_self_t;

#if CU_CLOS_CONTEXT_FIRST
#  define cuP_jargl(alops, argl, self) alops##prepend(argl, self)
#else
#  define cuP_jargl(alops, argl, self) alops##append(argl, self)
#endif

#define cuP_clos_formal(alops, argl) \
    cuP_jargl(alops, argl, cu_clos_self_t cuL_self)
#define cuP_xargl(...) \
    cuP_jargl(cuPP_argl_, __VA_ARGS__, cu_clos_self_t cuL_self)
#define cuP_xargl0() cu_clos_self_t cuL_self

/* Closure Pointers */

#define cu_clop(val, res_t, ...) \
    res_t (**val)cuP_jargl(cuPP_argl_, (__VA_ARGS__), cu_clos_self_t)
#define cu_clop0(val, res_t) \
    res_t (**val)(cu_clos_self_t)
typedef void (**cu_clop_generic_t)();

#define cu_clop_null (NULL)
#define cu_clop_is_null(clop) ((clop) == NULL)

#define cu_call(clop, ...) \
    ((**(clop))cuP_jargl(cuPP_argl_, (__VA_ARGS__), (cu_clos_self_t)(clop)))
#define cu_call0(clop) \
    ((**(clop))cuP_jargl(cuPP_argl0_, (), ((cu_clos_self_t)(clop))))

#define cuP_clop_def(name, linkage, res_t, alops, xparl)		\
    linkage res_t name##_fn cuP_clos_formal(alops, xparl);		\
    linkage res_t (*name##_data) cuP_clos_formal(alops, xparl) = &name##_fn; \
    linkage res_t (**name) cuP_clos_formal(alops, xparl)		\
	    CU_ATTR_UNUSED = &name##_data;				\
    linkage res_t name##_fn cuP_clos_formal(alops, xparl)
#define cu_clop_ref(name) (&name##_data)

#define cu_clop_def(name, res_t, ...) \
    cuP_clop_def(name, static, res_t, cuPP_argl_, (__VA_ARGS__))
#define cu_clop_def0(name, res_t) \
    cuP_clop_def(name, static, res_t, cuPP_argl0_, ())
#define cu_clop_edef(name, res_t, ...) \
    cuP_clop_def(name,       , res_t, cuPP_argl_, (__VA_ARGS__))
#define cu_clop_edef0(name, res_t) \
    cuP_clop_def(name,       , res_t, cuPP_argl0_, ())

#define cu_clop_call_fn(clop, ...) clop##_fn(__VA_ARGS__, NULL)
#define cu_clop_call_fn0(clop) clop##_fn(NULL)

/* Closures Implementations as Standalone Objects */

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
    CU_SINLINE void name##_init(name##_t *self)				\
    { self->cuL_fn = (res_t (*)cuP_clos_formal(alops, parl))name##_fn;}	\
									\
    CU_SINLINE name##_clop_t name##_prep(name##_t *self)		\
    { name##_init(self); return &self->cuL_fn; }			\
									\
    CU_SINLINE name##_clop_t name##_ref(name##_t *self)			\
    { return &self->cuL_fn; }

#define cuP_clos_fun(name, linkage, res_t, alops, parl)			\
    res_t name##_fn cuP_jargl(alops, parl, name##_t *cuL_self)

#define cu_clos_dec(name, prot, cargs)					\
    cuP_clos_dec(name, static, cargs, prot)				\
    CU_END_BOILERPLATE

#define cu_clos_edec(name, prot, cargs)					\
    cuP_clos_dec(name, extern, cargs, prot)				\
    CU_END_BOILERPLATE

#define cu_clos_fun(name, prot)		cuP_clos_fun(name, static, prot)
#define cu_clos_efun(name, prot)	cuP_clos_fun(name, extern, prot)

#define cu_clos_def(name, prot, cargs)					\
    cuP_clos_dec(name, static, cargs, prot)				\
    cuP_clos_fun(name, static, prot)

#define cu_prot(res_t, ...)	res_t, cuPP_argl_, (__VA_ARGS__)
#define cu_prot0(res_t)		res_t, cuPP_argl0_, ()

#define cu_clos_self(name) name##_t *self = cuL_self

/* Closure Implementations as Struct Members */

#define cu_clof_fun(fn, res_t, ...) \
    static res_t fn cuP_jargl(cuPP_argl_,(__VA_ARGS__),cu_clos_self_t cuL_self)
#define cu_clof_fun0(fn, res_t) \
    static res_t fn cuP_jargl(cuPP_argl0_, (), cu_clos_self_t cuL_self)
#define cu_clof_decl(f, res_t, ...) \
    res_t (*f)cuP_jargl(cuPP_argl_, (__VA_ARGS__), cu_clos_self_t)
#define cu_clof_decl0(f, res_t) \
    res_t (*f)cuP_jargl(cuPP_argl0_, (), cu_clos_self_t)
#define cu_clof_self(name, field) \
    struct name *self = cu_ptr_context(struct name, field, cuL_self)
#define cu_clof_init(obj, field, fn) ((obj)->field = (fn))
#define cu_clof_ref(obj, field) &(obj)->field

CU_END_DECLARATIONS

#endif /* !CU_IN_DOXYGEN */

#endif
