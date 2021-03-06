/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2003--2009  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUCON_RBTREE_H
#define CUCON_RBTREE_H

#include <cucon/fwd.h>
#include <cu/clos.h>
#include <cu/util.h>
#include <stdio.h>

CU_BEGIN_DECLARATIONS
/** \defgroup cucon_rbtree_h cucon/rbtree.h: Red-Black Trees
 ** @{ \ingroup cucon_maps_and_sets_mod
 **
 ** \note This is a somewhat unstable low-level API used to define more
 ** friendly APIs like \ref cucon_rbset_h and \ref cucon_rbmap_h, so you
 ** probably want to use those instead.
 **
 ** This is usual algorithm, but without back-links on the nodes to allow
 ** purely constructive updates if desired.  The comparison predicate is passed
 ** explicitely to functions which use them.  This makes it possible to define
 ** both sets and maps in terms of these trees, and it is also a way to allow
 ** keys to be stored directly on the slots.
 **
 ** \see cucon_rbset_h
 ** \see cucon_rbmap_h
 **/

/** A node of a red-black tree. */
struct cucon_rbnode
{
    size_t is_black; /* size_t due to root-node packing, see bottom of file. */
    struct cucon_rbnode *left;
    struct cucon_rbnode *right;
    /* variable size value */
};

/** Return a pointer to the slot of \a node. */
CU_SINLINE void *cucon_rbnode_mem(cucon_rbnode_t node)
{ return CU_ALIGNED_MARG_END(cucon_rbnode_t, node); }

/** Retrun the slot of \a node assuming it is a pointer. */
CU_SINLINE void *cucon_rbnode_ptr(cucon_rbnode_t node)
{ return *(void **)cucon_rbnode_mem(node); }

/** The left subtree of \a node. */
CU_SINLINE cucon_rbnode_t
cucon_rbnode_left(cucon_rbnode_t node) { return node->left; }

/** The right subtree of \a node. */
CU_SINLINE cucon_rbnode_t
cucon_rbnode_right(cucon_rbnode_t node) { return node->right; }

/** The red-black tree structure. */
struct cucon_rbtree
{
    struct cucon_rbnode *root;
};

/** Construct an empty tree. */
void cucon_rbtree_init(cucon_rbtree_t tree);

/** Return an empty tree. */
cucon_rbtree_t cucon_rbtree_new(void);

/** Construct a clone of \a src.  Cloned trees must be modified with \c _ctive
 ** functions only, to avoid interference. */
void cucon_rbtree_cct_copy_ctive(cucon_rbtree_t tree, cucon_rbtree_t src);

/** Return a clone of \a src.  Cloned trees must be modified with \c _ctive
 ** functions only, to avoid interference. */
cucon_rbtree_t cucon_rbtree_new_copy_ctive(cucon_rbtree_t src);

/** A node copier closure. */
    CU_DOXY_FAKED
typedef cu_clop(cucon_rbnode_copier_t, cucon_rbnode_t, cucon_rbnode_t)
    CU_DOXY_ENDFAKED(typedef __see_below__ cucon_rbnode_copier_t);

/** Make \a dst a deep copy of \a src, where \a node_new_copy\c (node) shall
 ** return a new copy of \c node, using \c cucon_rbnode_new_copy and filling in
 ** the slot. */
void cucon_rbtree_cct_copy_node(cucon_rbtree_t dst, cucon_rbtree_t src,
				cucon_rbnode_copier_t node_new_copy);

/** Return a deep copy of \a src, given that <code>\a node_new_copy(\e
 ** node)</code> returns a copy of \e node, using \ref cucon_rbnode_new_copy
 ** and filling in the slot. */
cucon_rbtree_t cucon_rbtree_new_copy_node(cucon_rbtree_t src,
					  cucon_rbnode_copier_t node_new_copy);

/** Construct \a dst as a deep copy of \a src, assuming the slots are
 ** pointers. */
void cucon_rbtree_cct_copy_ptr(cucon_rbtree_t dst, cucon_rbtree_t src);

/** Return a deep copy of \a src, assuming the slots are pointers. */
cucon_rbtree_t cucon_rbtree_new_copy_ptr(cucon_rbtree_t src);

/** Create a copy of \a src, but with an uninitialised value slot of \a
 ** slot_size bytes. */
cucon_rbnode_t cucon_rbnode_new_copy(cucon_rbnode_t src, size_t slot_size);

/** Insert all elements of \a src into \a dst, assuming slots are pointers. */
void cucon_rbtree_merge2p(cucon_rbtree_t dst, cucon_rbtree_t src,
			  cu_clop(cmp, int, void *, void *));

/** Constructively insert all elements of \a src into \a dst, assuming slots
 ** are pointers. */
void cucon_rbtree_cmerge2p(cucon_rbtree_t dst, cucon_rbtree_t src,
			   cu_clop(cmp, int, void *, void *));

/** Return number of elements in the tree. */
CU_SINLINE size_t
cucon_rbtree_size(cucon_rbtree_t tree)
{
    return tree->root? tree->root->is_black : 0;
}

/** True iff \a tree is empty. */
CU_SINLINE cu_bool_t
cucon_rbtree_is_empty(cucon_rbtree_t tree) { return tree->root == NULL; }

/** The root node of \a tree. */
CU_SINLINE cucon_rbnode_t
cucon_rbtree_root(cucon_rbtree_t tree) { return tree->root; }

/** Insert into \a tree an element which compares according to \a cmp1 where a
 ** negative, zero and positive return value means than the key is less than,
 ** equal to and greater than \a other_key, respectively.  If \a cmp1 maps an
 ** existing element to 0, then <code>*\a slot</code> is set to point to its
 ** value slot, and false is returned.  Otherwise, a new node is created with
 ** value of \a slot_size bytes at an address stored in \c *\a slot and true is
 ** returned.  In the latter case, you should construct an object at \c *\a
 ** slot for which \a cmp1 gives 0.  */
cu_bool_t
cucon_rbtree_insert1m_mem(cucon_rbtree_t tree,
			  cu_clop(cmp1, int, void *other_key),
			  size_t slot_size, cu_ptr_ptr_t slot);

/** A contructive version of \ref cucon_rbtree_insert1m_mem. */
cu_bool_t
cucon_rbtree_cinsert1m_mem(cucon_rbtree_t tree,
			   cu_clop(cmp1, int, void *),
			   cucon_rbnode_copier_t node_new_copy,
			   size_t slot_size, cu_ptr_ptr_t slot);

/** Insert \a key into \a tree if not present, assuming the keys are stored in
 ** a leading pointer-valued field of the slots.  Nodes are allocated with \a
 ** node_size bytes with cucon_rbnode_t at offset \a node_offset.  An allocated
 ** or present node with key equal to \a key is stored at <code>*\a
 ** node_out</code>.  If insertion was done, returns true, in which case only
 ** the key part of the returned slot is initialised. */
cu_bool_t
cucon_rbtree_insert2p_node(cucon_rbtree_t tree,
			   cu_clop(cmp2, int, void *, void *), void *key,
			   size_t node_size, size_t node_offset,
			   cucon_rbnode_t *node_out);

/* FIXME. Bad API. */
cu_bool_t
cucon_rbtree_insert2p_ptr(cucon_rbtree_t tree,
			  cu_clop(cmp2, int, void *, void *),
			  cu_ptr_ptr_t ptr_io);
cu_bool_t
cucon_rbtree_cinsert2p_ptr(cucon_rbtree_t tree,
			   cu_clop(cmp2, int, void *, void *),
			   cu_ptr_ptr_t ptr_io);

/** If \a tree has a mapping for \a key set <code>*\a slot_o</code> to its
 ** mapping and return false, else create a new mapping with key initialised to
 ** \a key, set <code>*\a slot_o</code> to its slot, and return true.
 **
 ** \pre The slots of \a tree are \a slot_size bytes which can be copied with
 ** \c memcpy, and starts with a \a key_size bytes key which can be compared
 ** with memcmp.  (“imem” for “immediate memory”.) */
cu_bool_t
cucon_rbtree_insert_imem_ctive(cucon_rbtree_t tree,
			       size_t key_size, void *key,
			       size_t slot_size, cu_ptr_ptr_t slot_o);

/** This function applies when slots of \a tree start with pointers to their
 ** keys.  If \a tree contains a node with a key equal to \a key according to
 ** \a cmp2, then erase it from \a tree and return it, else return \c NULL. */
cucon_rbnode_t
cucon_rbtree_erase2p(cucon_rbtree_t tree,
		     cu_clop(cmp2, int, void *, void *), void *key);

/** Erase from \a tree the element which compares 0 according to \a cmp1 and
 ** return it's slot, or return \c NULL if not found. */
void *cucon_rbtree_erase1m(cucon_rbtree_t tree, cu_clop(cmp1, int, void *));

/** This function applies when slots of \a tree start with pointers to their
 ** keys.  Return the node from \a tree with a key equal to \a key according to
 ** \a cmp2, or \c NULL if none. */
cucon_rbnode_t
cucon_rbtree_find2p_node(cucon_rbtree_t tree,
			 cu_clop(cmp2, int, void *, void *), void *key);

/** Return a pointer to the slot of the node where \a cmp1 returns 0, or \c
 ** NULL if not found. */
void *cucon_rbtree_find1m_mem(cucon_rbtree_t tree,
			      cu_clop(cmp1, int, void *));

/** Return the slot, assuming it is a pointer, of the node where \a cmp2
 ** returns 0, or \c NULL if not found. */
void *cucon_rbtree_find2p_ptr(cucon_rbtree_t tree,
			      cu_clop(cmp2, int, void *, void *), void *key);

void
cucon_rbtree_nearest2p(cucon_rbtree_t tree,
		       cu_clop(cmp2, int, void *, void *), void *key,
		       cucon_rbnode_t *below_out,
		       cucon_rbnode_t *equal_out,
		       cucon_rbnode_t *above_out);

/** Applies \a cb to the pointers to slots of \a tree in order. */
void cucon_rbtree_iter_mem(cucon_rbtree_t tree, cu_clop(cb, void, void *val));

/** Applies \a cb to pointers stored in the slots of \a tree in order. */
void cucon_rbtree_iter_ptr(cucon_rbtree_t tree, cu_clop(cb, void, void *val));

/** Applies \a cb to the pointers to slots of \a tree in reverse order. */
void cucon_rbtree_rev_iter_mem(cucon_rbtree_t tree, cu_clop(cb, void, void *));

/** Applies \a cb to pointers stored in the slots of \a tree in reverse
 ** order. */
void cucon_rbtree_rev_iter_ptr(cucon_rbtree_t tree, cu_clop(cb, void, void *));

/** Applies \a cb on each value of \a tree in order, exiting immediately with
 ** false as soon as \a cb returns false, otherwise returns true. */
cu_bool_t cucon_rbtree_conj_mem(cucon_rbtree_t tree,
				cu_clop(cb, cu_bool_t, void *value));

/** Applies \a cb on each pointer stored in the value slots of \a tree in
 ** order, exiting immediately with false as soon as \c cb returns false,
 ** otherwise returns true. */
cu_bool_t cucon_rbtree_conj_ptr(cucon_rbtree_t tree,
				cu_clop(cb, cu_bool_t, void *value));

/** Applies \a cb on each value of \a tree in reverse order, exiting
 ** immediately with false as soon as \a cb returns false, otherwise returns
 ** true. */
cu_bool_t cucon_rbtree_rev_conj_mem(cucon_rbtree_t tree,
				    cu_clop(cb, cu_bool_t, void *value));

/** Applies \a cb on each pointer stored in the value slots of \a tree in
 ** reverse order, exiting immediately with false as soon as \c cb returns
 ** false, otherwise returns true. */
cu_bool_t cucon_rbtree_rev_conj_ptr(cucon_rbtree_t tree,
				    cu_clop(cb, cu_bool_t, void *value));

/** Debug dump of \a tree an graphviz format. */
void cucon_rbtree_dump_as_graphviz(cucon_rbtree_t tree,
				   cu_clop(print_label, void, void *, FILE *),
				   FILE *out);

/** @} */
CU_END_DECLARATIONS

#endif
