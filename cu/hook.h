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

#ifndef CU_HOOK_H
#define CU_HOOK_H

#include <cu/fwd.h>
#include <cu/inherit.h>
#include <cu/dlink.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cu_hook_h cu/hook.h: Hooks; Templates and Common Instances
 *@{\ingroup cu_mod */

/*!A node of a hook for use in hook implementations. */
typedef struct cu_hook_node_s *cu_hook_node_t;
struct cu_hook_node_s
{
    cu_inherit (cu_dlink_s);
    cu_clop0(f, void);
};

#ifndef CU_IN_DOXYGEN
void cuP_hook_init(cu_hook_node_t hook);
void cuP_hook_prepend(cu_hook_node_t hook, cu_clop0(f, void));
void cuP_hook_append(cu_hook_node_t hook, cu_clop0(f, void));
#endif

/*!A template for defining a new hook with name prefix \a name.  \a f_decl must
 * declare a closure pointer parameter \e f.  This template defines an opaque
 * pointer type which can be cast to \ref cu_hook_node_t and functions
 * with suffixes _init, _prepend, and _append.  The _call function is hook
 * dependent, and can be defined using \ref CU_HOOK_FOR. */
#define CU_HOOK_TEMPLATE(name, f_decl)					\
    typedef struct name##_s *name##_t; /* abstract */			\
    struct name##_s { cu_inherit (cu_hook_node_s); };			\
									\
    /*!Initialise \a hook as an empty hook. */				\
    CU_SINLINE void name##_init(name##_t hook)				\
    { cuP_hook_init((cu_hook_node_t)hook); }				\
									\
    /*!True iff \a hook is empty. */					\
    CU_SINLINE cu_bool_t name##_is_empty(name##_t hook)			\
    { return cu_dlink_is_singleton(cu_to2(cu_dlink, cu_hook_node, hook)); }\
									\
    /*!Arrange for \a f to be called before current entries of \a hook. */\
    CU_SINLINE void name##_prepend(name##_t hook, f_decl)		\
    { cuP_hook_prepend((cu_hook_node_t)hook, (cu_clop0(, void))f); }	\
									\
    /*!Arrange for \a f to be called after current entries of \a hook. */\
    CU_SINLINE void name##_append(name##_t hook, f_decl)		\
    { cuP_hook_append((cu_hook_node_t)hook, (cu_clop0(, void))f); }

#define CU_HOOK_NEXT(node) \
    cu_from(cu_hook_node, cu_dlink, cu_to(cu_dlink, node)->next)

/*!A template for a for-loop header which iterates over entries of \a hook.
 * Each entry is assigned to \a node, which bust be declared with type \ref
 * cu_hook_node_t. */
#define CU_HOOK_FOR(node, hook)						\
    for (node = CU_HOOK_NEXT(cu_to(cu_hook_node, hook));		\
	 node != cu_to(cu_hook_node, hook);				\
	 node = CU_HOOK_NEXT(node))

CU_HOOK_TEMPLATE(cu_iter_hook, cu_clop0(f, void))
/*!Calls each entry of \a hook in order. */
void cu_iter_hook_call(cu_iter_hook_t hook);

CU_HOOK_TEMPLATE(cu_iterA_hook, cu_clop0(f, cu_bool_t))
/*!Calls each entry of \a hook in order as long as true is returned, and
 * returns the conjunction of the return values. */
cu_bool_t cu_iterA_hook_call(cu_iterA_hook_t hook);

CU_HOOK_TEMPLATE(cu_iter_ptr_hook, cu_clop(f, void, void *))
/*!Calls each entry of \a hook in order. */
void cu_iter_ptr_hook_call(cu_iter_ptr_hook_t hook, void *arg);

CU_HOOK_TEMPLATE(cu_iterA_ptr_hook, cu_clop(f, cu_bool_t, void *))
/*!Calls each entry of \a hook in order as long as true is returned, and
 * returns the conjunction of the return values. */
cu_bool_t cu_iterA_ptr_hook_call(cu_iterA_ptr_hook_t hook, void *arg);

CU_HOOK_TEMPLATE(cu_ptr_compose_hook, cu_clop(f, void *, void *))
/*!Evaluates the composition of the functions in \a hook applied to \a arg, or
 * returns \c NULL as soon as an entry returns \c NULL. */
void *cu_ptr_compose_hook_call(cu_ptr_compose_hook_t hook, void *arg);

/*!@}*/
CU_END_DECLARATIONS

#endif
