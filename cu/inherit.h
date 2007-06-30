/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2002--2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CU_INHERIT_H
#define CU_INHERIT_H

#include <stddef.h> /* defines offsetof */

/* !! No CU_BEGIN_DECLARATIONS here !!
 * These are only macros, and we don't need to include cu/fwd.h */

/*!\defgroup cu_inherit cu/inherit.h: A Framework for Multiple Inheritance
 * @{ \ingroup cu_mod */

#ifndef CU_MARG
#  ifndef CU_NDEBUG
#    define CU_MARG(ptr_t, ptr) ((ptr_t)(1? (ptr_t)(ptr) : (ptr)))
#  else
#    define CU_MARG(ptr_t, ptr) ((ptr_t)(ptr))
#  endif
#endif

#define cu_inherit(base_s)		struct base_s _base_##base_s
#define cu_inherit_virtual(base_s)	struct base_s *_vbase_##base_s
#define cu_set_virtual(base_s, ptr, vbase) \
	((void)((ptr)->_vbase_##base_s = vbase))

#define cu_base_offset(sub_s, base_s) \
	offsetof(struct sub_s, _base_##base_s)

#define cu_to(b1, o) (&(o)->_base_##b1##_s)
#define cu_to2(b2, b1, o) cu_to(b2, cu_to(b1, o))
#define cu_to3(b3, b2, b1, o) cu_to(b3, cu_to2(b2, b1, o))
#define cu_to4(b4, b3, b2, b1, o) cu_to(b4, cu_to3(b3, b2, b1, o))

#define cu_from(s1, s0, o)						\
    ((struct s1##_s *)((char *)CU_MARG(struct s0##_s *, o)		\
		       - offsetof(struct s1##_s, _base_##s0##_s)))
#define cu_from2(s2, s1, s0, o)					\
    cu_from(s2, s1, cu_from(s1, s0, o))
#define cu_from3(s3, s2, s1, s0, o)					\
    cu_from(s3, s2, cu_from2(s2, s1, s0, o))
#define cu_from4(s4, s3, s2, s1, s0, o)				\
    cu_from(s4, s3, cu_from3(s3, s2, s1, s0, o))

#define cu_upcast(base_s, o) (&(o)->_base_##base_s)
#define cu_upcast2(t0, t1, o) \
    cu_upcast(t0, cu_upcast(t1, (o)))
#define cu_upcast3(t0, t1, t2, o) \
    cu_upcast(t0, cu_upcast2(t1, t2, (o)))
#define cu_upcast4(t0, t1, t2, t3, o) \
    cu_upcast(t0, cu_upcast3(t1, t2, t3, (o)))
#define cu_upcast5(t0, t1, t2, t3, t4, o) \
    cu_upcast(t0, cu_upcast4(t1, t2, t3, t4, (o)))
#define cu_upcast_virtual(base_s, o) ((o)->_vbase_##base_s)

#define cu_downcast(sub_s, base_s, o)					\
    ((struct sub_s *)((char*)CU_MARG(struct base_s *, o)		\
		      - offsetof(struct sub_s, _base_##base_s)))
#define cu_downcast2(s0, s1, s2, o) \
    cu_downcast(s0, s1, cu_downcast(s1, s2, (o)))
#define cu_downcast3(s0, s1, s2, s3, o) \
    cu_downcast(s0, s1, cu_downcast2(s1, s2, s3, (o)))
#define cu_downcast4(s0, s1, s2, s3, s4, o) \
    cu_downcast(s0, s1, cu_downcast3(s1, s2, s3, s4, (o)))
#define cu_downcast5(s0, s1, s2, s3, s4, s5, o) \
    cu_downcast(s0, s1, cu_downcast4(s1, s2, s3, s4, s5, (o)))
/* For cu_downcast_virtual(sub_s, base_s, o) see "old/object.h". */

/*!@}*/
#endif
