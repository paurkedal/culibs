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

#ifndef CUEX_LTREE_H
#define CUEX_LTREE_H

#include <cuex/opn.h>
#include <cuex/oprdefs.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuex_ltree_h cuex/ltree.h: Left-Packed Trees (Low-Level API)
 *@{\ingroup cuex_mod
 *
 * This header implements a tree-based hash-consed container of expressions
 * representing an sequence of elements.  It is used to implement \ref
 * cuex_monoid_h "monoid expressions".
 */

#define CUEX_LTREE_LOG2_FANOUT 2
#define CUEX_LTREE_FANOUT (1 << CUEX_LTREE_LOG2_FANOUT)
#define CUEX_LTREE_FANOUT_MASK (CUEX_LTREE_FANOUT - 1)
#define CUEXP_OXR_LTREE(depth, arity) \
    (CUEXP_OR_LTREE(arity) | CUEXP_OA_LTREE_DEPTH(depth))
#define CUEXP_OA_LTREE_DEPTH_MAXP (1 << CUEXP_OA_LTREE_DEPTH_WIDTH)

CU_SINLINE cu_bool_t
cuex_is_oR_ltree(cuex_meta_t opr)
{
    return cuex_opr_sans_arity(cuexP_og_ltree_strip(opr)) == CUEXP_OR_LTREE(0);
}

struct cuex_ltree_node_s
{
    CUOO_HCOBJ
    cuex_t sub[CUEX_LTREE_FANOUT];
};

/*!The empty tree. */
CU_SINLINE cuex_t cuex_ltree_empty() { return NULL; }

/*!True iff \a tree is the empty tree. */
CU_SINLINE cu_bool_t cuex_ltree_is_empty(cuex_t tree) { return tree == NULL; }

CU_SINLINE cu_bool_t cuex_ltree_is_singleton(cuex_t tree)
{ return tree != NULL && !cuex_is_oR_ltree(cuex_meta(tree)); }

CU_SINLINE cu_bool_t cuexP_is_ltree_node(cuex_t tree)
{ return cuex_is_oR_ltree(cuex_meta(tree)); }

/*!Returns element number \a i of \a tree.
 * \pre \a tree must be non-empty and \a i must be within its range. */
cuex_t cuex_ltree_at(cuex_t tree, size_t i);

/*!Returns the last element of \a tree.
 * \pre \a tree must be non-empty. */
cuex_t cuex_ltree_last(cuex_t tree);

/*!Returns the concatenation of \a tree0 and \a tree1. */
cuex_t cuex_ltree_concat(cuex_t tree0, cuex_t tree1);

/*!Returns the slice of \a tree for indices [\a start, \a end). */
cuex_t cuex_ltree_slice(cuex_t tree, size_t start, size_t end);

/*!Return the number of elements in \a tree. */
size_t cuex_ltree_size(cuex_t tree);

/*!Calls \a fn on each element of \a tree sequentially as long as the result is
 * true, and returns true iff \a fn mapped all elements to true. */
cu_bool_t cuex_ltree_forall(cu_clop(fn, cu_bool_t, cuex_t), cuex_t tree);


/*!Opaque iterator type to be initialised with \ref cuex_ltree_itr_init_full or
 * \ref cuex_ltree_itr_init_slice. */
struct cuex_ltree_itr_s
{
    size_t i_cur;
    size_t i_end;
    cuex_t stack[CUEXP_OA_LTREE_DEPTH_MAXP];
};
typedef struct cuex_ltree_itr_s cuex_ltree_itr_t;

/*!Construct \a itr to iterate over all elements of \a tree. */
void cuex_ltree_itr_init_full(cuex_ltree_itr_t *itr, cuex_t tree);

/*!Construct \a itr to iterate over elements \a start to \a end of \a tree. */
void cuex_ltree_itr_init_slice(cuex_ltree_itr_t *itr, cuex_t tree,
			       size_t start, size_t end);

/*!True iff \a itr is at the end of its range. */
CU_SINLINE cu_bool_t cuex_ltree_itr_is_end(cuex_ltree_itr_t *itr)
{ return itr->i_cur == itr->i_end; }

/*!If \a itr is at its end, returns \c NULL, else returns the next element of
 * the sequence. */
cuex_t cuex_ltree_itr_get(cuex_ltree_itr_t *itr);

cu_ptr_source_t cuex_ltree_full_source(cuex_t x);

cu_ptr_source_t cuex_ltree_slice_source(cuex_t x, size_t i, size_t j);

cuex_t cuex_ltree_from_source(cu_ptr_source_t source);

cuex_t cuex_ltree_from_array(cuex_t *elt_array, size_t elt_count);

cuex_t cuex_ltree_append_from_source(cuex_t tree0, cu_ptr_source_t source1);

/*!@}*/
CU_END_DECLARATIONS

#endif
