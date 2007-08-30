/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2005--2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUDYN_TYPETPL_H
#define CUDYN_TYPETPL_H

#include <cudyn/fwd.h>

CU_BEGIN_DECLARATIONS
/* \defgroup cudyn_typetpl_h cudyn/typetpl.h: Type Definition Template Macros
 * @{ \ingroup cudyn_mod */

/* Elementary Type Templates
 * ------------------------- */

/*!Template to create the declarations for dynamic implementation of
 * \a type_t, using \a NAME as a basis for function names.
 * This declares the following functions:
 * <pre>
 *     cuex_t cudyn_<i>NAME</i>(\a type_t \a x);
 *     // Convert \a x to a (hashconsed) dynamically typed representation
 *     // of \a x.
 *
 *     cu_bool_t cudyn_is_<i>NAME</i>(cuex_t \a e);
 *     // Returns true iff \a e was created with cudyn_NAME, or an alias
 *     // defined with \c CUDYN_ETYPE_ALIAS.
 *
 *     \a type_t cudyn_to_<i>NAME</i>(cuex_t \a e);
 *     // If \a e was created with cudyn_<i>NAME</i>(\a x) for some \a x,
 *     // returns \a x, otherwise undefined behaviour.
 *
 *     cuoo_type_t cudyn_<i>NAME</i>_type(void);
 *     // Returns the dynamic type of \a type_t.
 * </pre>
 */
#define CUDYN_ETYPE_DCLN(NAME, type_t)					\
    extern cudyn_elmtype_t cudynP_##NAME##_type;			\
									\
    CU_SINLINE cuoo_type_t						\
    cudyn_##NAME##_type()						\
    {									\
	return cudyn_elmtype_to_type(cudynP_##NAME##_type);		\
    }									\
									\
    CU_SINLINE cu_bool_t						\
    cudyn_is_##NAME(cuex_t e)						\
    {									\
	return cuex_meta(e) == cuoo_type_to_meta(cudyn_##NAME##_type());\
    }									\
									\
    CU_SINLINE cuex_t							\
    cudyn_##NAME(type_t x)						\
    {									\
	if (sizeof(type_t) >= sizeof(cu_word_t))			\
	    return cuoo_halloc(cudyn_##NAME##_type(), sizeof(type_t), &x);\
	else {								\
	    struct {							\
		type_t xp;						\
		char padding[sizeof(cu_word_t) > sizeof(type_t)		\
			     ? sizeof(cu_word_t) - sizeof(type_t) : 1]; \
	    } tpl = { x };						\
	    return cuoo_halloc(cudyn_##NAME##_type(),			\
				sizeof(cu_word_t), &tpl);		\
	}								\
    }									\
									\
    CU_SINLINE type_t							\
    cudyn_to_##NAME(cuex_t e)						\
    {									\
	return *(type_t *)((void *)e + CUOO_HCOBJ_SHIFT);			\
    }

#define CUDYN_ETYPE_DEFN(NAME, type_t)					\
    cudyn_elmtype_t cudynP_##NAME##_type;				\
    struct cudynP_##NAME##_aligntest { char ch; type_t x; };

#define CUDYN_ETYPE_INIT(NAME, type_t, kind, ffitype)			\
    cudynP_##NAME##_type =						\
	cudyn_elmtype_new(cuoo_typekind_elmtype_##NAME, sizeof(type_t),\
			  offsetof(struct cudynP_##NAME##_aligntest, x),\
			  ffitype);					\
    cuoo_prop_define_ptr(cuoo_raw_c_name_prop(),			\
			  cudynP_##NAME##_type,				\
			  cu_idr_by_cstr(#type_t));

#define CUDYN_ETYPE_DEFN_PRINT(NAME, type_t, format)			\
    CUDYN_ETYPE_DEFN(NAME, type_t)					\
    static void cudynP_##NAME##_print(cuex_t e, FILE *out)		\
    { fprintf(out, format, cudyn_to_##NAME(e)); }

#define CUDYN_ETYPE_INIT_PRINT(NAME, type_t, kind, ffitype)		\
    CUDYN_ETYPE_INIT(NAME, type_t, kind, ffitype)			\
    cuoo_prop_condset_ptr(cuoo_raw_print_fn_prop(), cudyn_##NAME##_type(),\
			   &cudynP_##NAME##_print);

/*!Template to set up function aliases for dynamic implemetation of
 * \a type_t with name \a NAME in terms of \a IMPLNAME. */
#define CUDYN_ETYPE_ALIAS(NAME, type_t, IMPLNAME)			\
    CU_SINLINE cuoo_type_t cudyn_##NAME##_type()			\
    { return cudyn_##IMPLNAME##_type(); }				\
    CU_SINLINE cu_bool_t cudyn_is_##NAME(cuex_t e)			\
    { return cudyn_is_##IMPLNAME(e); }					\
    CU_SINLINE cuex_t cudyn_##NAME(type_t x)				\
    { return cudyn_##IMPLNAME(x); }					\
    CU_SINLINE type_t cudyn_to_##NAME(cuex_t e)				\
    { return cudyn_to_##IMPLNAME(e); }

#define cudyn_condsize_none ((size_t)-1)


/* Array Type Templates
 * -------------------- */

/*!Template to create the declarations for dynamic implementations of
 * \a type_t including array variants, using \a NAME as a basis for
 * function names.  In addition to what \ref CUDYN_ETYPE_DCLN declares,
 * <pre>
 *     cuex_t cudyn_<i>NAME</i>arr(size_t \a cnt, \a type_t *\a arr);
 *     // Convert an array \a arr of \a cnt elements to a dynamic
 *     // representation.
 *
 *     size_t cudyn_<i>NAME</i>arr_condsize(cuex_t \a e)
 *     // If \a e is an array of \a type_t, return its size, else return
 *     // cudyn_condsize_none.
 *
 *     \a type_t *cudyn_to_<i>NAME</i>arr(cuex_t \a e);
 *     // If \a e was created with cudyn_<i>NAME</i>arr(size, arr) for some
 *     // size and arr, return arr, else undefined behaviour.
 *
 *     cuoo_type_t cudyn_<i>NAME</i>arr_type(size_t \a cnt);
 *     // Returns the dynamic type of \a type_t[\a cnt]
 * </pre>
 */
#define CUDYN_ARRTYPE_DCLN(NAME, type_t)				\
    CU_SINLINE cuoo_type_t						\
    cudyn_##NAME##arr_type(size_t size)					\
    {									\
	return cudyn_arrtype_to_type(					\
		   cudyn_arrtype(cudyn_##NAME##_type(), size));		\
    }									\
									\
    size_t cudyn_##NAME##arr_condsize(cuex_t);				\
									\
    CU_SINLINE size_t							\
    cudyn_##NAME##arr_size(cuex_t ex)					\
    {									\
	return cudyn_arrtype_elt_cnt(cudyn_arrtype_from_meta(cuex_meta(ex)));\
    }									\
									\
    CU_SINLINE cuex_t							\
    cudyn_##NAME##arr(size_t cnt, type_t *arr)				\
    {									\
	if (sizeof(type_t) >= sizeof(cu_word_t))			\
	    return cuoo_halloc(cudyn_##NAME##arr_type(cnt),		\
				sizeof(type_t)*cnt, arr);		\
	else								\
	    cu_bugf("unimplemented");					\
    }									\
									\
    CU_SINLINE type_t *							\
    cudyn_##NAME##arr_arr(cuex_t e)					\
    { return (type_t *)((void *)e + CUOO_HCOBJ_SHIFT); }			\
									\
    CU_SINLINE type_t							\
    cudyn_##NAME##arr_at(cuex_t e, size_t i)				\
    { return cudyn_##NAME##arr_arr(e)[i]; }

#define CUDYN_ARRTYPE_DEFN(NAME, type_t)				\
    size_t								\
    cudyn_##NAME##arr_condsize(cuex_t e)				\
    {									\
	cuex_meta_t meta = cuex_meta(e);				\
	cudyn_arrtype_t t;						\
	if (!cuex_meta_is_type(meta)					\
	    ||!cuoo_type_is_arrtype(cuoo_type_from_meta(meta)))	\
	    return cudyn_condsize_none;					\
	t = cudyn_arrtype_from_meta(meta);				\
	if (cudyn_arrtype_elt_type(t) != cudyn_##NAME##_type())		\
	    return cudyn_condsize_none;					\
	return cudyn_arrtype_elt_cnt(t);				\
    }

#define CUDYN_ETYPEARR_DCLN(NAME, type_t)				\
    CUDYN_ETYPE_DCLN(NAME, type_t)					\
    CUDYN_ARRTYPE_DCLN(NAME, type_t)

#define CUDYN_ETYPEARR_DEFN(NAME, type_t)				\
    CUDYN_ETYPE_DEFN(NAME, type_t)					\
    CUDYN_ARRTYPE_DEFN(NAME, type_t)

#define CUDYN_ETYPEARR_INIT CUDYN_ETYPE_INIT

#define CUDYN_ETYPEARR_DEFN_PRINT CUDYN_ETYPE_DEFN_PRINT

#define CUDYN_ETYPEARR_INIT_PRINT CUDYN_ETYPE_INIT_PRINT

/*!Template to set up function aliases for dynamic implementation of
 * \a type_t with name \a NAME in terms of \a IMPLNAME including array
 * versions.  The two types must have the same size and be layout
 * compatible. */
#define CUDYN_ETYPEARR_ALIAS(NAME, type_t, IMPLNAME)			\
    CUDYN_ETYPE_ALIAS(NAME, type_t, IMPLNAME)				\
    CU_SINLINE cuoo_type_t cudyn_##NAME##arr_type(size_t cnt)		\
    { return cudyn_##IMPLNAME##arr_type(cnt); }				\
    CU_SINLINE int cudyn_##NAME##_condsize(cuex_t e)			\
    { return cudyn_##IMPLNAME##arr_condsize(e); }			\
    CU_SINLINE cuex_t cudyn_##NAME##arr(size_t cnt, type_t *arr)	\
    { return cudyn_##IMPLNAME##arr(cnt, (void *)arr); }			\
    CU_SINLINE type_t *cudyn_##NAME##arr_arr(cuex_t e)			\
    { return (type_t *)cudyn_##IMPLNAME##arr_arr(e); }

/*!@}*/
CU_END_DECLARATIONS

#endif
