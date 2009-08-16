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

#ifndef CU_EXCEPT_H
#define CU_EXCEPT_H

#include <cuflow/fwd.h>
#include <cuflow/wind.h>
#include <cu/clos.h>
#include <cu/conf.h>
#ifdef CUCONF_HAVE_GC_GC_H
#  include <gc/gc.h>
#else
#  include <gc.h>
#endif

CU_BEGIN_DECLARATIONS

#define cuflow_xc_extern(name, proto, sct) cuflowP_xc_extern(name, proto, sct)
#define cuflow_xc_define(name, proto) cuflowP_xc_define(name, proto)
#define cuflowP_xc_extern(name, ign_t, alops, argl, sct)		\
    typedef struct name##_s *name##_t;					\
    void name##_cct alops##prepend(argl, name##_t);			\
    struct name##_s {							\
	struct cuflowP_windargs_s windargs;				\
	cuPP_splice sct							\
    }
#define cuflowP_xc_define(name, ign_t, alops, argl)			\
    void name##_cct alops##prepend(argl, name##_t self)


#define cuflow_xc_cct_m(xc, fn, ...) \
    ((fn)(xc, __VA_ARGS__), (xc)->windargs.xc_key = (cu_fnptr_t)(fn))
#define cuflow_xc_cct0_m(xc, fn) \
    ((fn)(xc), (xc)->windargs.xc_key = (cu_fnptr_t)(fn))

#define cuflow_throw(name, ...)					\
    do {								\
	name##_t xc = GC_malloc(sizeof(struct name##_s));		\
	cuflow_xc_cct_m(xc, name##_cct, __VA_ARGS__);			\
	cuflowP_throw(&xc->windargs);					\
    } while (0)
#define cuflow_throw0(name)						\
    do {								\
	name##_t xc = GC_malloc(sizeof(struct name##_s));		\
	cuflow_xc_cct0_m(xc, name##_cct);				\
	cuflowP_throw(&xc->windargs);					\
    } while (0)
#define cuflow_throw_static(xc) cuflowP_throw(&(xc)->windargs)

#define cuflow_xc_is(name) (cuflowP_windstate.windargs->xc_key == name##_cct)
#define cuflow_xc_dcln(name)						\
	name##_t xc = (name##_t)((char *)cuflowP_windstate.windargs	\
				 - offsetof(struct name##_s, windargs))

CU_END_DECLARATIONS

#endif
