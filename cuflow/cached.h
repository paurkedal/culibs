/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUFLOW_CACHED_H
#define CUFLOW_CACHED_H

#include <cuflow/fwd.h>
#include <cuflow/sched.h>
#include <cuflow/cdisj.h>
#include <cu/conf.h>
#include <cu/inherit.h>
#include <atomic_ops.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuflow_cached_h cuflow/cached.h: Cached Function Calls
 *@{\ingroup cuflow_mod */

typedef unsigned long cuflow_gain_t;
typedef cu_clop0(cuflow_cached_clop_t, void);

typedef struct cuflowP_cached_node_s cuflowP_cached_node_t;
struct cuflowP_cached_node_s
{
    cuflowP_cached_node_t *next;
    AO_t cdisj;
    unsigned int access_ticks;
    cuflow_gain_t access_function;
    cuflow_gain_t access_gain;
};

#define CUFLOW_CACHED_GRAN_SIZEW 1
#define CUFLOW_CACHED_GRAN_SIZEB (CU_WORD_SIZE*CUFLOW_CACHED_GRAN_SIZEW)
#define CUFLOW_CACHED_ARG_SIZEG(NAME)					\
    ((sizeof(struct NAME##_arg_s) + CUFLOW_CACHED_GRAN_SIZEB - 1) /	\
     CUFLOW_CACHED_GRAN_SIZEB)
#define CUFLOW_CACHED_ARG_SIZE(NAME)					\
    (CUFLOW_CACHED_ARG_SIZEG(NAME)*CUFLOW_CACHED_GRAN_SIZEB)

#define cuflowP_cached_decls(NAME, arg_members, res_members, linkage)	\
    typedef struct NAME##_node_s NAME##_node_t;				\
									\
    typedef struct NAME##_arg_s {					\
	void (*fn)(struct NAME##_arg_s *);				\
	cuPP_splice arg_members						\
    } NAME##_arg_t;							\
									\
    typedef union NAME##_u {						\
	struct NAME##_arg_s arg;					\
	char cuL_padding[CUFLOW_CACHED_ARG_SIZE(NAME)];			\
    } NAME##_t;								\
									\
    typedef struct NAME##_result_s {					\
	cuPP_splice res_members						\
    } *NAME##_result_t;							\
									\
    struct NAME##_node_s {						\
	cu_inherit (cuflowP_cached_node_s);				\
	NAME##_t padded_arg;						\
	struct NAME##_result_s res;					\
    };									\
									\
    linkage void NAME##_fn(NAME##_arg_t *arg);				\
									\
    CU_SINLINE void							\
    NAME##_cct(NAME##_t *padded_arg)					\
    {									\
	memset(padded_arg, 0, CUFLOW_CACHED_ARG_SIZE(NAME));		\
	padded_arg->arg.fn = NAME##_fn;					\
    }									\
									\
    typedef NAME##_node_t *NAME##_promise_t;				\
									\
    CU_SINLINE NAME##_result_t						\
    NAME##_fulfill(NAME##_promise_t promise)				\
    {									\
	cuflow_cdisj_wait_while(&cu_to(cuflowP_cached_node, promise)->cdisj);\
	return &promise->res;						\
    }									\
									\
    CU_END_BOILERPLATE

/*!Emits type definitions and declarations for a cached function with
 * external linkage. */
#define cuflow_cached_edecl(NAME, arg_members, res_members)		\
    cuflowP_cached_decls(NAME, arg_members, res_members, extern)
/*!Emits the prototype of the cached function with external linkage.  This
 * shall be followed by the body of the function. */
#define cuflow_cached_edef(NAME)					\
    extern void NAME##_fn(NAME##_arg_t *arg)

/*!Emits type definitions and declarations for a cached function with
 * static linkage. */
#define cuflow_cached_sdecl(NAME, arg_members, res_members)		\
    cuflowP_cached_decls(NAME, arg_members, res_members, static)
/*!Emits the prototype of the cached function with static linkage.  This
 * shall be followed by the body of the function. */
#define cuflow_cached_sdef(NAME)					\
    static void NAME##_fn(NAME##_arg_t *arg)

/*!Emits type definitions and declarations for a cached function with static
 * linkage, and the prototype of the function.  Shall be followed by the body
 * of the function. */
#define cuflow_cached_def(NAME, arg_members, res_members)		\
    cuflow_cached_sdecl(NAME, arg_members, res_members);		\
    cuflow_cached_sdef(NAME)

#define CUFLOW_CACHED_NODE_FROM_ARG(NAME, arg)				\
    ((NAME##_node_t *)							\
     ((char *)(arg) - offsetof(struct NAME##_node_s, padded_arg)))

/*!Used in a cached function definition, inserts declarations and
 * initialisation of two local variables \e arg and \e res.  \e arg is the
 * initialised argument struct from the caller, and \e res is the result
 * struct to be filled out. */
#define cuflow_cached_arg_res(NAME)					\
    NAME##_result_t res = &CUFLOW_CACHED_NODE_FROM_ARG(NAME, arg)->res

#define cuflow_cached_set_gain(NAME, x)					\
    (cu_to(cuflowP_cached_node,						\
	   CUFLOW_CACHED_NODE_FROM_ARG(NAME, arg))->access_gain = x)

void *cuflowP_cached_call(void *arg, size_t arg_sizeg, size_t full_size);
void *cuflowP_sched_cached_call(void *arg, size_t, size_t);

#define cuflow_cached_call(NAME, arg_block, result_out)			\
    do {								\
	NAME##_t cuL_padded_arg;					\
	NAME##_cct(&cuL_padded_arg);					\
	NAME##_arg_t *arg = &cuL_padded_arg.arg;			\
	{ cuPP_splice arg_block }					\
	{								\
	    NAME##_node_t *cuL_node = cuflowP_cached_call(		\
		    &cuL_padded_arg, CUFLOW_CACHED_ARG_SIZEG(NAME),	\
		    sizeof(struct NAME##_node_s));			\
	    *(result_out) = &cuL_node->res;				\
	}								\
    } while (0)

#define cuflow_cached_sched_call(NAME, arg_block, promise_out)		\
    do {								\
	NAME##_t cuL_padded_arg;					\
	NAME##_cct(&cuL_padded_arg);					\
	NAME##_arg_t *arg = &cuL_padded_arg.arg;			\
	{ cuPP_splice arg_block }					\
	*(promise_out) = (NAME##_promise_t)cuflowP_sched_cached_call(	\
		    &cuL_padded_arg, CUFLOW_CACHED_ARG_SIZEG(NAME),	\
		    sizeof(struct NAME##_node_s));			\
    } while (0)

/*!@}*/
CU_END_DECLARATIONS

#endif
