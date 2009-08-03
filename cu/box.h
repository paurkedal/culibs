/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2009  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CU_BOX_H
#define CU_BOX_H

#include <cu/fwd.h>
#include <cu/conf.h>

#define CU_BOX_IS_UNION 1

CU_BEGIN_DECLARATIONS
/** \defgroup cu_box_h cu/box.h: Boxing and Unboxing Operations
 ** @{\ingroup cu_type_mod */

#define CU_BOX_SIZE CU_WORD_SIZE

#if CUCONF_SIZEOF_FLOAT <= CU_BOX_SIZE
#  define CU_BOX_FITS_FLOAT 1
#endif

#if CUCONF_SIZEOF_DOUBLE <= CU_BOX_SIZE
#  define CU_BOX_FITS_DOUBLE 1
#endif

#if CUCONF_SIZEOF_LONGDOUBLE <= CU_BOX_SIZE
#  define CU_BOX_FITS_LONGDOUBLE
#endif

#if defined(CU_BOX_IS_UNION) || defined(CU_IN_DOXYGEN)

/** A type which holds any other type either directly or though an allocated
 ** pointer, depending on the size of the type. */
typedef union {
    void *as_ptr;
    void (*as_fptr)(void);
    cu_word_t as_word;
#ifdef CU_BOX_FITS_FLOAT
    float as_float;
#endif
#ifdef CU_BOX_FITS_DOUBLE
    double as_double;
#endif
#ifdef CU_BOX_FITS_LONGDOUBLE
    long double as_longdouble;
#endif
} cu_box_t;

#  ifndef CU_IN_DOXYGEN
CU_SINLINE cu_box_t
cuP_box_ptr(void *p)
{
    cu_box_t box;
    box.as_ptr = p;
    return box;
}
CU_SINLINE cu_box_t
cuP_box_fptr(cu_fnptr_t f)
{
    cu_box_t box;
    box.as_fptr = f;
    return box;
}
CU_SINLINE cu_box_t
cuP_box_word(cu_word_t w)
{
    cu_box_t box;
    box.as_word = w;
    return box;
}
#    define cuP_unbox_ptr(box)		((box).as_ptr)
#    define cuP_unbox_fptr(box)		((box).as_fptr)
#    define cuP_unbox_word(box)		((box).as_word)
#    define CUP_BOX_PTR_INIT(ptr)	{ .as_ptr  = (ptr)  }
#    define CUP_BOX_FPTR_INIT(fptr)	{ .as_fptr = (fptr) }
#    define CUP_BOX_WORD_INIT(word)	{ .as_word = (word) }
#  endif /* !CU_IN_DOXYGEN */

/** A boilerplate to emit a definition of a function \a fn for boxing a large
 ** type \a type using garbage collected storage. */
#define CU_BOX_GNEW_TEMPLATE(fn, type)					\
    cu_box_t fn(type x)							\
    {									\
	cu_box_t box;							\
	box.as_ptr = cu_gnew(type);					\
	*(type *)box.as_ptr = x;					\
	return box;							\
    }									\
    CU_END_BOILERPLATE

#else /* !CU_BOX_IS_UNION */

typedef cu_word_t cu_box_t;
#  define cuP_box_ptr(val)		((cu_box_t)(val))
#  define cuP_box_fptr(val)		((cu_box_t)(val))
#  define cuP_box_word(val)		((cu_box_t)(val))
#  define cuP_unbox_ptr(box)		(box)
#  define cuP_unbox_fptr(box)		(box)
#  define cuP_unbox_word(box)		(box)
#  define CUP_BOX_PTR_INIT(ptr)		cuP_box_ptr(ptr)
#  define CUP_BOX_FPTR_INIT(fptr)	cuP_box_fptr(fptr)
#  define CUP_BOX_WORD_INIT(word)	cuP_box_word(word)

#define CU_BOX_GNEW_TEMPLATE(fn, type)					\
    cu_box_t fn(type x)							\
    {									\
	void *p = cu_gnew(type);					\
	*(type *)p = x;							\
	return (cu_box_t)p;						\
    }									\
    CU_END_BOILERPLATE

#endif /* !CU_BOX_IS_UNION */

cu_box_t cu_box_galloc(size_t size, void *addr);
#define cu_box_gnew(type, obj) cu_box_galloc(sizeof(type), &(obj))

/** \name Pointers, Functions, and Enums
 ** @{ */

/*!Boxes a pointer of the specified type. */
#define cu_box_ptr(type, ptr)		cuP_box_ptr(1? (ptr) : (type)(ptr))
/*!Unboxes a pointer of the specified type. */
#define cu_unbox_ptr(type, box)		((type)cuP_unbox_ptr(box))
/** An initialiser for a pointer box. */
#define CU_BOX_PTR_INIT(type, ptr) CUP_BOX_PTR_INIT(1? (ptr) : (type)(ptr))
/** A \c NULL pointer box initialiser. */
#define CU_BOX_NULL_PTR_INIT		CUP_BOX_PTR_INIT(NULL)

/*!Boxes a function pointer of the specified type. */
#define cu_box_fptr(type, fp) cuP_box_fptr((cu_fnptr_t)(1? (fp) : (type)(fp)))
/*!Unboxes a function pointer of the specified type. */
#define cu_unbox_fptr(type, box)	((type)cuP_unbox_fptr(box))
/** An initialiser for a function pointer box. */
#define CU_BOX_FPTR_INIT(type, fptr) CUP_BOX_FPTR_INIT(1? (fptr) : (type)(fptr))
/** A \c NULL function pointer box initialiser. */
#define CU_BOX_NULL_FPTR_INIT		CUP_BOX_FPTR_INIT(NULL)

/*!Boxes a closure pointer of the specified type. */
#define cu_box_clop(type, cp)		cu_box_ptr(type,  cp)
/*!Unboxes a closure pointer of the specified type. */
#define cu_unbox_clop(type, box)	cu_unbox_ptr(type, box)

/*!Boxes an enum of the given type. */
#define cu_box_enum(type, i)		cuP_box_word(1? (i) : (type)(i))
/*!Unboxes an enum of the given type. */
#define cu_unbox_enum(type, box)	((type)cuP_unbox_word(box))
/** An initialiser for a box if enum type \a type. */
#define CU_BOX_ENUM_INIT(type, i)	CUP_BOX_WORD_INIT(1? (i) : (type)(i))

/** @}
 ** \name Integer Types
 ** @{ */

/*!Boxes an \c int or any shorter integer types. */
CU_SINLINE cu_box_t		cu_box_int(int i)
{ return cuP_box_word(i); }

/*!Unboxes an \c int or any shorter integer type. */
CU_SINLINE int			cu_unbox_int(cu_box_t box)
{ return cuP_unbox_word(box); }

/*!Boxes an <tt>unsigned int</tt> or any shorter unsigned integer type. */
CU_SINLINE cu_box_t		cu_box_uint(unsigned int i)
{ return cuP_box_word(i); }

/*!Unboxes an <tt>unsigned int</tt> or any shorter unsigned integer type. */
CU_SINLINE unsigned int		cu_unbox_uint(cu_box_t box)
{ return cuP_unbox_word(box); }

/*!Boxes a \c long. */
CU_SINLINE cu_box_t		cu_box_long(long i)
{ return cuP_box_word(i); }

/*!Unboxes a \c long. */
CU_SINLINE long			cu_unbox_long(cu_box_t box)
{ return cuP_unbox_word(box); }

/*!Boxes an <tt>unsigned long</tt>. */
CU_SINLINE cu_box_t		cu_box_ulong(unsigned long i)
{ return cuP_box_word(i); }

/*!Unboxes an <tt>unsigned long</tt>. */
CU_SINLINE unsigned long	cu_unbox_ulong(cu_box_t box)
{ return cuP_unbox_word(box); }

/** @}
 ** \name Floating Point Types
 ** @{ */

#if defined(CU_BOX_FITS_FLOAT) || defined(CU_IN_DOXYGEN)
#  if defined(CU_BOX_IS_UNION) || defined(CU_IN_DOXYGEN)

/*!Boxes a \c float. */
CU_SINLINE cu_box_t		cu_box_float(float x)
{ cu_box_t box; box.as_float = x; return box; }

/*!Unboxes a \c float. */
CU_SINLINE float		cu_unbox_float(cu_box_t box)
{ return box.as_float; }

#  else /* !CU_BOX_IS_UNION */

CU_SINLINE cu_box_t		cu_box_float(float x)
{ union { cu_box_t box; float x; } u; u.x = x; return u.box; }

CU_SINLINE float		cu_unbox_float(cu_box_t box)
{ union { cu_box_t box; float x; } u; u.box = box; return u.x; }

#  endif /* !CU_BOX_IS_UNION */

#else /* !CU_BOX_FITS_FLOAT */
cu_box_t			cu_box_float(float x);
CU_SINLINE float		cu_unbox_float(cu_box_t box)
{ return *cu_unbox_ptr(float *, box); }
#endif /* !CU_BOX_FITS_FLOAT */


#if defined(CU_BOX_FITS_DOUBLE) || defined(CU_IN_DOXIGEN)
#  if defined(CU_BOX_IS_UNION) || defined(CU_IN_DOXYGEN)

/*!Boxes a \c double. */
CU_SINLINE cu_box_t		cu_box_double(double x)
{ cu_box_t box; box.as_double = x; return box; }

/*!Unboxes a \c double. */
CU_SINLINE double		cu_unbox_double(cu_box_t box)
{ return box.as_double; }

#  else /* !CU_BOX_IS_UNION */

CU_SINLINE cu_box_t		cu_box_double(double x)
{ union { cu_box_t box; double x; } u; u.x = x; return u.box; }

CU_SINLINE double		cu_unbox_double(cu_box_t box)
{ union { cu_box_t box; double x; } u; u.box = box; return u.x; }

#  endif /* !CU_BOX_IS_UNION */

#else /* !CU_BOX_FITS_DOUBLE */
cu_box_t			cu_box_double(double x);
CU_SINLINE double		cu_unbox_double(cu_box_t box)
{ return *cu_unbox_ptr(double *, box); }
#endif /* !CU_BOX_FITS_DOUBLE */


#if defined(CU_BOX_FITS_LONGDOUBLE) || defined(CU_IN_DOXIGEN)
#  if defined(CU_BOX_IS_UNION) || defined(CU_IN_DOXYGEN)

/*!Boxes a <tt>long double</tt>. */
CU_SINLINE cu_box_t		cu_box_longdouble(long double x)
{ cu_box_t box; box.as_longdouble = x; return box; }

/*!Unboxes a <tt>long double</tt>. */
CU_SINLINE long double		cu_unbox_longdouble(cu_box_t box)
{ return box.as_longdouble; }

#  else /* !CU_BOX_IS_UNION */

CU_SINLINE cu_box_t		cu_box_longdouble(long double x)
{ union { cu_box_t box; long double x; } u; u.x = x; return u.box; }

CU_SINLINE long double		cu_unbox_longdouble(cu_box_t box)
{ union { cu_box_t box; long double x; } u; u.box = box; return u.x; }

#  endif /* !CU_BOX_IS_UNION */

#else /* !CU_BOX_FITS_LONGDOUBLE */
cu_box_t			cu_box_longdouble(long double x);
CU_SINLINE long double		cu_unbox_longdouble(cu_box_t box)
{ return *cu_unbox_ptr(long double *, box); }
#endif /* !CU_BOX_FITS_LONGDOUBLE */

/** @}
 ** @} */
CU_END_DECLARATIONS

#endif
