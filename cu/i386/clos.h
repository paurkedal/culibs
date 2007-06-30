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

/* NOTE.  DON'T USE THIS IMPLEMENTATION.  My benchmarks shows that it
 * is very inefficint when calling closures, contrary to what a naive
 * count of instructions suggests.  My guess is it breaks instruction
 * caching or pipelining.
 *
 *
 * Portability
 * -----------
 *
 * This variant implementation works on IA32 only, and may also depend
 * an OS and break with exotic compilers or compiler flags.  Similar
 * implementations should be possible for most architectures, though.
 * The implementation also requires a 'typeof' extension from both the
 * compiler used both for bulidnig the library and for that used to
 * compile client code.
 *
 * The libccf distribution also includes a generic replacement.
 *
 *
 * Type Safety
 * -----------
 *
 * This variant is fully type-safe for if the compiler has a typeof
 * keyword.
 *
 * Note, however, that there is no distinction between C function
 * pointers and pointers to corresponding closures, as they can be
 * mixed and 'cu_call' is redundant.  To be sure your usage is
 * portable, be careful not to mix C functions an closure pointers.
 * If in doubt, switch to the generic implementation now an then.
 */

#ifndef CU_X86_CLOS_BITS_H
#define CU_X86_CLOS_BITS_H

#include <cuflow/fwd.h>
#include <stdint.h>

CU_BEGIN_DECLARATIONS

/* The two upper bytes of cuP_CLOS_MAGIC are padding.  For
 * 'cu_clptr_is_clos' to be reliable, these two bytes should be set
 * to some non-instruction, which are also not produced by the
 * compiler for padding.  When I checked, the compiler pads with a
 * valid instruction,
 *
 * #define cuP_CLOS_MAGIC UINT64_C(0xf689c300000000e8)
 *
 * so we use */
#define cuP_CLOS_MAGIC UINT64_C(0xff00c300000000e8)


#if 1
#  define cuP_TYPEOF_OR_VOIDPTR(obj) typeof(obj)
#  define cuP_CLSCT(s) struct s
#else
#  define cuP_TYPEOF_OR_VOIDPTR(obj) void *
#  define cuP_CLSCT(s) void
#endif

#define cuP_clos_base(clos_s, result_t, alops, argl)			\
    struct {								\
	result_t (*cuP_like_full) 					\
	    alops##prepend(argl, cuP_CLSCT(clos_s) *);			\
	result_t (*cuP_like_call) argl;					\
    } cuP_trampoline

#define cuP_clos_cct(clos, fn)						\
    ((void)(1								\
	    ? (*(uint64_t*)&(clos)->cuP_trampoline			\
	       = cuP_CLOS_MAGIC						\
	       + ((uint64_t)((uint32_t)(fn) - 5				\
			   - (uint32_t)&(clos)->cuP_trampoline) << 8))	\
	    : (clos)->cuP_trampoline.cuP_like_full == (fn)))

#define cu_clos_ref(clos)						\
     ((cuP_TYPEOF_OR_VOIDPTR((clos)->cuP_trampoline.cuP_like_call)) 	\
      &(clos)->cuP_trampoline)
#define cu_clos_ref_cast(proto, clos)					\
    cuP_clos_ref_cast(proto, cu_clos_ref(clos))
#define cuP_clos_ref_cast(result_t, alops, argl, fn) ((result_t (*)argl)(fn))

#define cu_clos_from_clptr(clptr) ((void *)(clptr))

#define cuP_clos_formal(clos_s, alops, argl)				\
    alops##prepend(argl, cuP_CLSCT(clos_s) *cuP_dummyarg)

#define cu_clos_arg_dcln(clos_s, var)					\
    struct clos_s *var =						\
	(1								\
	 ? *((void **)(uint32_t)&cuP_dummyarg - 1) - 5			\
	 : (void *)(cuP_dummyarg == var))

#define cu_nonclos(nonclos) (*nonclos)
#define cu_nonclos0(nonclos) (*nonclos)cuP_clptr_formal0
#define cu_nonclos_cct(nonclos, fn) (*(nonclos) = (fn))
#define cu_nonclos_ref(nonclos) (*(nonclos))

#define cu_func(fn, proto) struct cuP_dummy_s
#define cu_func_init_e(fn, proto) struct cuP_dummy_s
#define cu_static_func(fn, proto) struct cuP_dummy_s
#define cu_func_init(fn, proto) struct cuP_dummay_s
#define cu_func_ref(fn) (fn)
#define cu_func_ref_cast(proto, fn) cuP_func_ref_cast(proto, fn)
#define cuP_func_ref_cast(result_t, alops, argl, fn) ((result_t (*)argl)(fn))

#define cuP_clop(fn, result_t, alops, argl) result_t (*fn)argl
#define cu_clptr(clptr) (*clptr)
#define cu_clptr0(clptr) (*clptr)cuP_clptr_formal0
#define cu_clop_null (NULL)
#define cu_clop_is_null(clptr) ((clptr) == NULL)
#define cu_clptr_fnptr_m(clptr)						\
    ((cu_fnptr_t)((uint32_t)((*(uint64_t*)(clptr) >> 8) & 0xffffffff)	\
		    + 5 + (uint32_t)(clptr)))
#if 0
#define cu_clptr_is_clos(clptr)						\
    (((*(uint64_t*)(clptr)) & UINT64_C(0xffffff00000000ff))		\
     == cuP_CLOS_MAGIC)
#else
#define cu_clptr_is_clos(clptr) (0)
#endif

#define cu_call(clptr, args...) (clptr)(args)
#define cu_call0(clptr) (clptr)()
#if 0
#define cu_call_cast(clptr, result_t, args...)				\
	(((result_t (*)())(clptr))(args))
#define cu_call0_cast(clptr, result_t)					\
	(((result_t (*)())(clptr))())
#endif

#define cuP_clptr_formal0() (void)

CU_END_DECLARATIONS

#endif
