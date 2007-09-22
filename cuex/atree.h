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

#ifndef CUEX_ATREE_H
#define CUEX_ATREE_H

#include <cuex/fwd.h>
#include <cuoo/type.h>
#include <cu/algo.h>
#include <cu/clos.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuex_atree_h cuex/atree.h: Associative Trees of Expressions
 *@{\ingroup cuex_mod
 *
 * This is a low-level interface for implementing semi-lattices, sets, maps,
 * and other collections of expressions which are keyed on a machine word
 * computed form the expressions.
 *
 * The internal structure of the tree is hidden.  The leafs of the tree are the
 * inserted expressions.
 * Most of the functions below takes a \a get_key argument, which when passed a
 * leaf node shall compute a \c cu_word_t typed key.
 * Since \c cu_word_t is at least the size of pointers, to implement a set of
 * hash-consed objects, cast the leaf value pointer to \c cu_word_t, and to
 * implement a map, use a binary operation as leafs, and cast the first operand
 * to \c cu_word_t.
 * The same \a get_key must be used consistently when operating on the same
 * tree, which also means that union and intersections can only be used on
 * trees of the same \a get_key.
 * A high-level interface will typically embed these trees in a top-level node
 * and either use a fixed \a get_key or one stored on the top-level node.  */

/*!\defgroup cuex_atree_iv_mod Indexed Value Variants
 * The following functions are specialised variants of the corresponding
 * functions from the \ref cuex_atree_h "primary interface", where the leafs are
 * assumed to be operations of sufficient arity, and callbacks acting on leaves
 * are replaced by callbacks action on a given operand of the leaves.  The
 * caller is responsible of ensuring that all leafs have the proper form.
 * \ingroup cuex_atree_h */

/*!\defgroup cuex_atree_kv_mod Key-Value Variants
 * The following functions are specialised variants of the corresponding
 * functions from the \ref cuex_atree_h "primary interface", where the leafs
 * are assumed to be binary operations where the first operand serves as a key
 * and the second operand serves as a value.  Callbacks generally receive the
 * key as the first argument and one or two values as the next arguments.
 * \ingroup cuex_atree_h */

extern cuoo_stdtype_t cuexP_anode_type;

CU_SINLINE cuoo_type_t
cuex_anode_type()
{ return cuoo_stdtype_to_type(cuexP_anode_type); }

/*!An empty container. */
CU_SINLINE cuex_t
cuex_atree_empty()
{ return NULL; }

/*!True iff \a tree is the empty container. */
CU_SINLINE cu_bool_t
cuex_atree_is_empty(cuex_t tree)
{ return tree == NULL; }

/*!Find \a find_key in \a tree, assuming \a tree is keyed by \a get_key.
 * Returns the matching element, i.e. the element \e e such that
 * <tt>cu_call(get_key, e)</tt> equals \a find_key.  Returns \c NULL if no such
 * element exists.  */
cuex_t
cuex_atree_find(cu_clop(get_key, cu_word_t, cuex_t),
		cuex_t tree, cu_word_t key);

/*!If the key of \a value is not already in \a tree, returns the result of
 * inserting \a value into \a tree, else returns \a tree. */
cuex_t
cuex_atree_insert(cu_clop(get_key, cu_word_t, cuex_t),
		  cuex_t tree, cuex_t value);

/*!If the key of \a value is in \a tree, then replace the element with \a
 * value, otherwise insert \a value. */
cuex_t
cuex_atree_replace(cu_clop(get_key, cu_word_t, cuex_t),
		   cuex_t tree, cuex_t value);

/*!As \ref cuex_atree_insert, but if a key-equal element \e e is present in \a
 * tree, replace it with <tt>\a merge_values(\e e, \a value)</tt>. */
cuex_t
cuex_atree_deep_insert(cu_clop(get_key, cu_word_t, cuex_t leaf),
		       cu_clop(merge, cuex_t, cuex_t leaf0, cuex_t leaf1),
		       cuex_t tree, cuex_t value);

/*!As \ref cuex_atree_deep_insert, but merge only operand \a merge_index of
 * values.
 * \ingroup cuex_atree_iv_mod */
cuex_t
cuex_atree_deep_insert_iv(cu_clop(get_key, cu_word_t, cuex_t),
			  cu_rank_t merge_index,
			  cu_clop(merge_fn, cuex_t, cuex_t val0, cuex_t val1),
			  cuex_t tree, cuex_t value);

/*!As \ref cuex_atree_deep_insert, but assume leafs are key-value binary
 * operations.  \a merge receives the common first operand (key) and each of
 * the second operands (values) and shall return the merged value.
 * \ingroup cuex_atree_kv_mod */
cuex_t
cuex_atree_deep_insert_kv(cu_clop(merge, cuex_t,
				  cuex_t key, cuex_t val0, cuex_t val1),
			  cuex_t tree, cuex_t value);

/*!If \a erase_key is in \a tree, returns \a tree with the value corresponding
 * to \a erase_key erased, otherwise returns \a tree. */
cuex_t
cuex_atree_erase(cu_clop(get_key, cu_word_t, cuex_t),
		 cuex_t tree, cu_word_t erase_key);

/*!Returns the union of \a tree0 and \a tree1, considering two elements equal
 * if the corresponding values returned by \a get_key are equal.  For common
 * nodes, those from \a tree0 are used in the result. */
cuex_t
cuex_atree_left_union(cu_clop(get_key, cu_word_t, cuex_t),
		      cuex_t tree0, cuex_t tree1);

/*!As \ref cuex_atree_left_union, except merge any duplicate element with \a
 * merge_values. */
cuex_t
cuex_atree_deep_union(cu_clop(get_key, cu_word_t, cuex_t),
		      cu_clop(merge, cuex_t, cuex_t leaf0, cuex_t leaf1),
		      cuex_t tree0, cuex_t tree1);

/*!As \ref cuex_atree_deep_union, except that \a merge_fn only acts on operand
 * \a merge_index of the leaves.
 * \ingroup cuex_atree_iv_mod */
cuex_t
cuex_atree_deep_union_iv(cu_clop(get_key, cu_word_t, cuex_t),
			 cu_rank_t merge_index,
			 cu_clop(merge_fn, cuex_t, cuex_t val0, cuex_t val1),
			 cuex_t tree0, cuex_t tree1);

/*!As \ref cuex_atree_deep_union, except that \a merge_fn only acts on the
 * second operand of leafs and also receives the first operand (the key).
 * \ingroup cuex_atree_kv_mod */
cuex_t
cuex_atree_deep_union_kv(cu_clop(merge, cuex_t,
				 cuex_t key, cuex_t val0, cuex_t val1),
			 cuex_t tree0, cuex_t tree1);

/*!Returns the intersection of \a tree0 and \a tree1, considering two
 * elements equal if the corresponding values returned by \a get_key are
 * equal.  The nodes from \a tree0 are used in the result. */
cuex_t
cuex_atree_left_isecn(cu_clop(get_key, cu_word_t, cuex_t),
		      cuex_t tree0, cuex_t tree1);

/*!As \a cuex_atree_isecn, but merge intersecting elements from \a tree0 and \a
 * tree1 using \a merge_values. */
cuex_t
cuex_atree_deep_isecn(cu_clop(get_key, cu_word_t, cuex_t),
		      cu_clop(merge, cuex_t, cuex_t leaf0, cuex_t leaf1),
		      cuex_t tree0, cuex_t tree1);

/*!As \ref cuex_atree_deep_isecn, but \a merge_fn only merges operand \a
 * merge_index.
 * \ingroup cuex_atree_iv_mod */
cuex_t
cuex_atree_deep_isecn_iv(cu_clop(get_key, cu_word_t, cuex_t),
			 cu_rank_t merge_index,
			 cu_clop(merge_fn, cuex_t, cuex_t val0, cuex_t val1),
			 cuex_t tree0, cuex_t tree1);

/*!As \ref cuex_atree_deep_isecn, except it assumes the first operand of leaves
 * are keys and the second oprand are values.  \a merge receives the common
 * keys and the two values to be merged.
 * \ingroup cuex_atree_kv_mod */
cuex_t
cuex_atree_deep_isecn_kv(cu_clop(merge, cuex_t,
				 cuex_t key, cuex_t val0, cuex_t val1),
			 cuex_t tree0, cuex_t tree1);

/*!True iff \a tree0 ⊆ \a tree1 where elements are considered equal iff their
 * \a get_key values are equal. */
cu_bool_t cuex_atree_subseteq(cu_clop(get_key, cu_word_t, cuex_t),
			      cuex_t tree0, cuex_t tree1);

/*!As \ref cuex_atree_subseteq, except for any pair (\e e0, \e e1) of elements
 * of (\a tree0, \a tree1) with equal keys, return false unless <tt>\a
 * value_subseteq(\e e0, \e e1)</tt> returns true. */
cu_bool_t
cuex_atree_deep_subseteq(cu_clop(get_key, cu_word_t, cuex_t),
			 cu_clop(value_subseteq, cu_bool_t, cuex_t, cuex_t),
			 cuex_t tree0, cuex_t tree1);

/*!Returns the ordering of \a tree0 and \a tree1, where elements are
 * considered equal iff their \a get_key values are equal. */
cu_order_t cuex_atree_order(cu_clop(get_key, cu_word_t, cuex_t),
			    cuex_t tree0, cuex_t tree1);

/*!A variant of \a cuex_atree_order which also takes into account value
 * ordering.  Whenever the keys of two elements are equal, either \a
 * value_subseteq or \a value_order is used to determine their ordering.  The
 * latter function is used when one of the two possible orderings of the trees
 * have been excluded.  The use of two ordering callbacks is purely an
 * optimisation which may be hidden by a high-level interface; either callback
 * can be implemented in terms of the other. */
cu_order_t
cuex_atree_deep_order(cu_clop(get_key, cu_word_t, cuex_t),
		      cu_clop(value_subseteq, cu_bool_t, cuex_t, cuex_t),
		      cu_clop(value_order, cu_order_t, cuex_t, cuex_t),
		      cuex_t tree0, cuex_t tree1);

/*!As \ref cuex_atree_deep_order, but only pass operand \a value_index of
 * the leaves to the ordering functions.
 * \ingroup cuex_atree_iv_mod */
cu_order_t
cuex_atree_deep_order_iv(cu_clop(get_key, cu_word_t, cuex_t),
			 cu_rank_t value_index,
			 cu_clop(value_subseteq, cu_bool_t, cuex_t, cuex_t),
			 cu_clop(value_order, cu_order_t, cuex_t, cuex_t),
			 cuex_t tree0, cuex_t tree1);

/*!As \ref cuex_atree_deep_order, but assume leafs are operations where the
 * first operand are keys and the second are values.  The common key and the
 * values to compare are passed to the ordering functions.
 * \ingroup cuex_atree_kv_mod */
cu_order_t
cuex_atree_deep_order_kv(cu_clop(value_subseteq, cu_bool_t,
				 cuex_t key, cuex_t val0, cuex_t val1),
			 cu_clop(value_order, cu_order_t,
				 cuex_t key, cuex_t val0, cuex_t val1),
			 cuex_t tree0, cuex_t tree1);

/*!Call \a f with each value in \a tree. */
void cuex_atree_iter(cuex_t tree, cu_clop(f, void, cuex_t leaf));

/*!Returns the conjunction af \a f applied to all keys in \a tree.  Stops as
 * soon as \a f returns false. */
cu_bool_t cuex_atree_conj(cuex_t tree, cu_clop(f, cu_bool_t, cuex_t leaf));

/*!A variant of \a cuex_atree_conj which expects elements to be operations of
 * at least \a value_index + 1 operands, and calls \a f with operand \a
 * value_index.
 * \ingroup cuex_atree_iv_mod */
cu_bool_t cuex_atree_conj_iv(cuex_t tree, cu_rank_t value_index,
			     cu_clop(f, cu_bool_t, cuex_t val));

/*!A variant of \a cuex_atree_conj which expects that elements are operations
 * of at least 2 operands, and calls back \a f with the first two operands of
 * the elements.  This specialisation applies to implementations of map-like
 * types such as \ref cuex_labelling_h "labellings".
 * \ingroup cuex_atree_kv_mod */
cu_bool_t cuex_atree_conj_kv(cuex_t tree,
			     cu_clop(f, cu_bool_t, cuex_t key, cuex_t val));

/*!Returns the image of \a tree under \a f.  Note that \a get_key is only
 * used for building the result, and may in fact be different from the \a
 * get_key which was used to build \a tree (thus its order in the argument
 * list).  If \a f leaves the key of its argument unchanged, see \ref
 * cuex_atree_isokey_image for a faster alternative. */
cuex_t cuex_atree_image(cuex_t tree, cu_clop(f, cuex_t, cuex_t),
			cu_clop(get_key, cu_word_t, cuex_t leaf));

/*!Returns the image of \a tree under \a f assuming \a f does not change the
 * key of it's argument.  The key referred to is the result of the \e get_key
 * used to build the container \ref cuex_atree_insert, so the condition is that
 * \e get_key(\a f(\e e)) = \e get_key(\e e) for all elements \e e in \a
 * tree.  If this condition is not met see \ref cuex_atree_image. */
cuex_t cuex_atree_isokey_image(cuex_t tree, cu_clop(f, cuex_t, cuex_t leaf));

/*!A variant of \ref cuex_atree_isokey_image which assumes that elements are
 * operations of arity at least \a value_index + 1, and transforms only operand
 * \a value_index of the elements.  This specialisation applies to map-like
 * types with inert keys, such as \ref cuex_labelling_h "labellings".
 * \ingroup cuex_atree_iv_mod */
cuex_t cuex_atree_isokey_image_iv(cuex_t tree, cu_rank_t value_index,
				  cu_clop(f, cuex_t, cuex_t val));

/*!A variant of \ref cuex_atree_isokey_image which assumes elements to be
 * operations of arity at least 2, and transforms the second operand.  Both the
 * first and second operands are passed to \a f in order.  This is suited for
 * map-like types where the first operand is an inert key and the second
 * operand is a transformable value.
 * \ingroup cuex_atree_kv_mod */
cuex_t cuex_atree_isokey_image_kv(cuex_t tree,
				  cu_clop(f, cuex_t, cuex_t key, cuex_t val));

/*!The depth of the deepest tree node of \a tree.  The empty tree and
 * singletons have depth 0.  A tree node has depth one plus the maximum of the
 * depths of its branches. */
int cuex_atree_depth(cuex_t tree);

size_t cuex_atree_itr_size(cuex_t tree);
void cuex_atree_itr_init(void *itr, cuex_t tree);
cuex_t cuex_atree_itr_get(void *itr);
cuex_t cuex_atree_itr_get_at_1(void *itr);

/*!@}*/
CU_END_DECLARATIONS

#endif
