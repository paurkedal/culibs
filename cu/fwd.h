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
/*!\defgroup cu_fwd_h cu/fwd.h: Forward Declarations
 * @{\ingroup cu_mod */

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

#define cu_noop() ((void)0)

/* Used to prevent warnings when C compiler can not predict
 * initialisation, and informatively to assert the code is ok. May be
 * disabled for slightly more optimal code. */
#define CU_NOINIT(val) = val

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


/* Scalar Types
 * ------------ */

/* Booleans */
typedef unsigned int cu_bool_t;
typedef unsigned int cu_bool_fast_t;
typedef unsigned char cu_bool_least_t;
#define cu_true 1
#define cu_false 0

typedef void *cu_ptr_t;

/* Used in argument list instead of 'void **' to avoid cast warnings. */
typedef void *cu_ptr_ptr_t;

/* Used as generic function pointer. */
typedef void (*cu_fnptr_t)();

/* A type which can hold the size of bounded-size objects.  Holds the size of
 * structures, but not necessarily variable sized arrays.  May be 32 bit on 64
 * bit platforms. */
typedef unsigned int cu_shortsize_t;
typedef unsigned int cu_rank_t;

/* A fastest and shortest types which can hold 'log2(size)' for any
 * 'size' in 'size_t'. */
typedef unsigned int cu_logsize_fast_t;
typedef unsigned char cu_logsize_least_t;
#define cu_logsize_width 6

/* Used for fast and weak hashes. */
typedef unsigned long int cu_hash_t;
#define CUCONF_SIZEOF_CU_HASH_T CUCONF_SIZEOF_LONG

/* Machine word. */
typedef uintptr_t cu_word_t;
#define CU_WORD_WIDTH CUCONF_WIDTHOF_INTPTR_T
#define CU_WORD_SIZE CUCONF_SIZEOF_INTPTR_T
#define CU_WORD_C(x) ((cu_word_t)x##UL)

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

typedef struct cu_idr_s		*cu_idr_t;		/* idr.h */
typedef struct cu_ptr_source_s	*cu_ptr_source_t;	/* ptr_seq.h */
typedef struct cu_ptr_sink_s	*cu_ptr_sink_t;		/* ptr_seq.h */
typedef struct cu_ptr_junction_s*cu_ptr_junction_t;	/* ptr_seq.h */
typedef struct cu_ptr_sinktor_s *cu_ptr_sinktor_t;	/* ptr_seq.h */
typedef struct cu_ptr_junctor_s *cu_ptr_junctor_t;	/* ptr_seq.h */
typedef struct cu_sref_s	*cu_sref_t;		/* srcref.h */
typedef struct cu_str_s		*cu_str_t;		/* str.h */


/* Declarations of Some Common Functions
 * ------------------------------------- */

/* init.c */
void cu_init(void);

/* cstr.c */
char const *cu_struniq(char const *str);
char const *cu_struniq_int(int);
size_t cu_struniq_string_count(void);
char *cu_wcstombs(wchar_t const *wcs);
wchar_t *cu_mbstowcs(char const *str);

/*!@}*/
CU_END_DECLARATIONS

#endif
