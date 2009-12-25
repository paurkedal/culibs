/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2002--2009  Petter Urkedal <urkedal@nbi.dk>
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

/** \defgroup cu_inherit_h cu/inherit.h: A Framework for Multiple Inheritance
 ** @{ \ingroup cu_base_mod */

#ifndef CU_MARG
#  ifndef CU_NDEBUG
#    define CU_MARG(ptr_t, ptr) ((ptr_t)(1? (ptr_t)(ptr) : (ptr)))
#  else
#    define CU_MARG(ptr_t, ptr) ((ptr_t)(ptr))
#  endif
#endif

#define CU_BASE_FIELD(base_s)		_base_##base_s
#define cu_inherit(base_s)		struct base_s CU_BASE_FIELD(base_s)
#define cu_inherit_virtual(base_s)	struct base_s *_vbase_##base_s
#define cu_set_virtual(base_s, ptr, vbase) \
	((void)((ptr)->_vbase_##base_s = vbase))

#define cu_base_offset(sup, base) \
	offsetof(struct sup, _base_##base)

#define cu_to(b1, o) (&(o)->CU_BASE_FIELD(b1))
#define cu_to2(b2, b1, o) cu_to(b2, cu_to(b1, o))
#define cu_to3(b3, b2, b1, o) cu_to(b3, cu_to2(b2, b1, o))
#define cu_to4(b4, b3, b2, b1, o) cu_to(b4, cu_to3(b3, b2, b1, o))
#define cu_to5(b5, b4, b3, b2, b1, o) cu_to(b5, cu_to4(b4, b3, b2, b1, o))
#define cu_to_virtual(base_s, o) ((o)->_vbase_##base_s)

#define cu_from(s1, s0, o) \
    ((struct s1 *)((char *)CU_MARG(struct s0 *, o) \
		   - offsetof(struct s1, _base_##s0)))
#define cu_from2(s2, s1, s0, o) \
    cu_from(s2, s1, cu_from(s1, s0, o))
#define cu_from3(s3, s2, s1, s0, o) \
    cu_from(s3, s2, cu_from2(s2, s1, s0, o))
#define cu_from4(s4, s3, s2, s1, s0, o) \
    cu_from(s4, s3, cu_from3(s3, s2, s1, s0, o))
#define cu_from5(s0, s1, s2, s3, s4, s5, o) \
    cu_sub(s0, s1, cu_sub4(s1, s2, s3, s4, s5, (o)))

/** @} */
#endif
