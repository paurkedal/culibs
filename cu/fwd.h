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

#ifndef CU_FWD_H
#define CU_FWD_H

#include <stdlib.h>
#include <stdint.h>
#include <cu/conf.h>

#ifdef __cplusplus
#  define CU_BEGIN_DECLARATIONS extern "C" {
#  define CU_END_DECLARATIONS }
#else
#  define CU_BEGIN_DECLARATIONS
#  define CU_END_DECLARATIONS
#endif


CU_BEGIN_DECLARATIONS
/** \defgroup cu_fwd_h cu/fwd.h: Forward Declarations
 ** @{ \ingroup cu_mod */

/* The AC_C_INLINE will defined "inline" to "__inline__" or "__inline", leave
 * it, or define it blank. */
#define CU_SINLINE static inline

#ifndef CU_MARG
#  ifndef CU_NDEBUG
#    define CU_MARG(ptr_t, ptr) ((ptr_t)(1? (ptr_t)(ptr) : (ptr)))
#  else
#    define CU_MARG(ptr_t, ptr) ((ptr_t)(ptr))
#  endif
#endif

CU_SINLINE void cu_noop() {}

/* Used to prevent warnings when C compiler can not predict
 * initialisation, and informatively to assert the code is ok. May be
 * disabled for slightly more optimal code. */
#define CU_NOINIT(...) = __VA_ARGS__

/** Used to silence warnings about the result of \a call being discarded. */
#define CU_DISCARD(call) ((call)? cu_noop() : cu_noop())

/* Compiler-dependent features.  These have no semantic effect if used
 * correctly, but may give optimalization hints or provide additional
 * debugging.  See GCC info nodes for details. */
#ifdef __GNUC__
#  define CU_ATTR_NORETURN	__attribute__ ((noreturn))
#  define CU_ATTR_PURE		__attribute__ ((pure))
#  define CU_ATTR_CONST		__attribute__ ((const))
#  define CU_ATTR_UNUSED	__attribute__ ((unused))
#  define CU_ATTR_DEPRECATED	__attribute__ ((deprecated))
#  define CU_ATTR_MALLOC	__attribute__ ((malloc))
#  define cu_expect(e, c)	__builtin_expect(e, c)
#else
#  define CU_ATTR_NORETURN
#  define CU_ATTR_PURE
#  define CU_ATTR_CONST
#  define CU_ATTR_UNUSED
#  define CU_ATTR_DEPRECATED
#  define CU_ATTR_MALLOC
#  define cu_expect(e, c)	(e)
#endif
#define cu_expect_false(e) cu_expect(e, cu_false)
#define cu_expect_true(e) cu_expect(e, cu_true)

#define CU_END_BOILERPLATE struct cuP_expecting_semicolor_after_boilerplate

#define CU_DOXY_FAKED
#define CU_DOXY_ENDFAKED(...)
#ifdef CU_IN_DOXYGEN
#  define CU_DOXY_AID(fake, ...)					\
/** \par "Real Definition"						*/\
/** The following is the actual definition.  It was omitted above	*/\
/** since it is unparsable to the documentation generator.		*/\
/** \code __VA_ARGS__ \endcode						*/\
    fake
#else
#  define CU_DOXY_AID(fake, ...) __VA_ARGS__
#endif


/* Scalar Types
 * ------------ */

/** The integer type used to pass around booleans. */
typedef unsigned int cu_bool_t;

/** \deprecated Use \ref cu_bool_t. */
typedef unsigned int cu_bool_fast_t;

/** The smallest integer type which can hold a boolean. */
typedef unsigned char cu_bool_least_t;

/** A non-zero integer value used to represent true. */
#define cu_true 1

/** The integer value 0 used to represent false. */
#define cu_false 0

typedef void *cu_ptr_t;

/* Used in argument list instead of 'void **' to avoid cast warnings. */
typedef void *cu_ptr_ptr_t;

/** The type conventionally used for generic function pointers.  This shall be
 ** cast back to the actual function pointer type when used. */
typedef void (*cu_fnptr_t)();

/* A type which can hold the size of bounded-size objects.  Holds the size of
 * structures, but not necessarily variable sized arrays.  May be 32 bit on 64
 * bit platforms. */
typedef unsigned int cu_shortsize_t;
typedef unsigned int cu_rank_t;
#define CU_RANK_MAX UINT_MAX

/* A fastest and shortest types which can hold 'log2(size)' for any
 * 'size' in 'size_t'. */
typedef unsigned int cu_logsize_fast_t;
typedef unsigned char cu_logsize_least_t;
#define cu_logsize_width 6

/** An unsigned integer used for weak (non-cryptographic) hashes. */
typedef unsigned long int cu_hash_t;

/** \c sizeof(cu_hash_t) as a const literal. */
#define CUCONF_SIZEOF_CU_HASH_T CUCONF_SIZEOF_LONG

/** An integer of the biggest width which can be efficiently handled by the
 ** architecture.  This must also be wide enough to hold a pointer, so if there
 ** are architetures where pointers are wider than the maximum natural integer
 ** with, a less efficient integer type must me chosen. */
typedef uintptr_t cu_word_t;

#define CU_WORD_WIDTH CUCONF_WIDTHOF_INTPTR_T
#define CU_WORD_P2WIDTH CUCONF_P2WIDTHOF_INTPTR_T
#define CU_WORD_LOG2_P2WIDTH CUCONF_LOG2_P2WIDTHOF_INTPTR_T
#define CU_WORD_SIZE CUCONF_SIZEOF_INTPTR_T
#define CU_WORD_C(x) ((cu_word_t)x##UL)
#define CU_PRIdWORD PRIdPTR
#define CU_PRIuWORD PRIuPTR
#define CU_PRIxWORD PRIxPTR
#define CU_SCNdWORD SCNdPTR
#define CU_SCNiWORD SCNiPTR
#define CU_SCNuWORD SCNuPTR
#define CU_SCNxWORD SCNxPTR

/* Missing from stdint.h */
#define CU_UINTPTR_C(x) ((uintptr_t)x##UL)

/* And int which can hold any offset in a structure. */
typedef int cu_offset_t;
#define CU_OFFSET_NAME CUP_UINT_NAME

#ifdef CUCONF_ENABLE_LARGE_DATASETS
typedef size_t cu_count_t;
typedef ptrdiff_t cu_countdiff_t;
#else
typedef unsigned int cu_count_t;
typedef int cu_countdiff_t;
#endif


/* Compound Types
 * -------------- */

typedef struct cu_buffer_s	*cu_buffer_t;		/* buffer.h */
typedef struct cu_dlink_s	*cu_dlink_t;		/* dlink.h */
typedef struct cu_dbufsink_s	*cu_dbufsink_t;		/* dsink.h */
typedef struct cu_dcountsink_s	*cu_dcountsink_t;	/* dsink.h */
typedef struct cu_dsink_s	*cu_dsink_t;		/* dsink.h */
typedef struct cu_dsource_s	*cu_dsource_t;		/* dsource.h */
typedef struct cu_idr_s		*cu_idr_t;		/* idr.h */
typedef struct cu_log_facility_s*cu_log_facility_t;	/* logging.h */
typedef struct cu_ptr_array_source_s *cu_ptr_array_source_t; /* ptr_seq.h */
typedef struct cu_ptr_source_s	*cu_ptr_source_t;	/* ptr_seq.h */
typedef struct cu_ptr_sink_s	*cu_ptr_sink_t;		/* ptr_seq.h */
typedef struct cu_ptr_junction_s*cu_ptr_junction_t;	/* ptr_seq.h */
typedef struct cu_ptr_sinktor_s *cu_ptr_sinktor_t;	/* ptr_seq.h */
typedef struct cu_ptr_junctor_s *cu_ptr_junctor_t;	/* ptr_seq.h */
typedef struct cu_sref_s	*cu_sref_t;		/* srcref.h */
typedef struct cu_str_s		*cu_str_t;		/* str.h */
typedef struct cu_wstring_s	*cu_wstring_t;		/* wstring.h */


/* Declarations of Some Common Functions
 * ------------------------------------- */

/** Initialises the \ref cu_mod "cu" module.  This must be called at least once
 ** before using cu_* functions.  It is automatically called by the
 ** initialisation functions of dependent modules. */
void cu_init(void);

/** @} */
CU_END_DECLARATIONS

#endif
