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

#include <cuex/atree.h>
#include <cuex/opn.h>
#include <cufo/stream.h>
#include <cufo/tagdefs.h>
#include <cuoo/prop.h>
#include <cuoo/properties.h>
#include <cuoo/intf.h>
#include <cuoo/hctem.h>
#include <cu/int.h>
#include <cu/algo.h>

static size_t atree_card(cuex_t tree);

cu_clop_def(get_key_0, cu_word_t, cuex_t e)
{
    cu_debug_assert(cuex_meta_is_opr(cuex_meta(e)) &&
		    cuex_opr_r(cuex_meta(e)) > 0);
    return (cu_word_t)cuex_opn_at(e, 0);
}

cu_clop_def(_merge_as_first, cuex_t, cuex_t e0, cuex_t e1)
{
    return e0;
}

cu_clop_def(_merge_as_second, cuex_t, cuex_t e0, cuex_t e1)
{
    return e1;
}

cu_clop_def(_merge_disjoin, cuex_t, cuex_t e0, cuex_t e1)
{
    return NULL;
}

cu_clos_def(_merge_iv_adaptor,
	    cu_prot(cuex_t, cuex_t e0, cuex_t e1),
  ( cu_rank_t value_index;
    cu_clop(value_merge, cuex_t, cuex_t, cuex_t); ))
{
    cu_clos_self(_merge_iv_adaptor);
    cu_debug_assert(cuex_meta_is_opr(cuex_meta(e0)) &&
		    cuex_meta_is_opr(cuex_meta(e1)) &&
		    cuex_opr_r(cuex_meta(e0)) > self->value_index &&
		    cuex_opr_r(cuex_meta(e1)) > self->value_index);
    return cu_call(self->value_merge,
		   cuex_opn_at(e0, self->value_index),
		   cuex_opn_at(e1, self->value_index));
}

cu_clos_def(_merge_kv_adaptor,
	    cu_prot(cuex_t, cuex_t e0, cuex_t e1),
    (cu_clop(merge, cuex_t, cuex_t, cuex_t, cuex_t);))
{
    cu_clos_self(_merge_kv_adaptor);
    cu_debug_assert(cuex_meta_is_opr(cuex_meta(e0)) &&
		    cuex_meta_is_opr(cuex_meta(e1)) &&
		    cuex_opr_r(cuex_meta(e0)) >= 2 &&
		    cuex_opr_r(cuex_meta(e1)) >= 2);
    cu_debug_assert(cuex_opn_at(e0, 0) == cuex_opn_at(e1, 0));
    return cu_call(self->merge, cuex_opn_at(e0, 0),
		   cuex_opn_at(e0, 1), cuex_opn_at(e1, 1));
}

cu_clop_def(_subseteq_true, cu_bool_t, cuex_t e0, cuex_t e1)
{
    return cu_true;
}

cu_clos_def(_subseteq_iv_adaptor,
	    cu_prot(cu_bool_t, cuex_t e0, cuex_t e1),
  ( cu_rank_t value_index;
    cu_clop(value_subseteq, cu_bool_t, cuex_t, cuex_t); ))
{
    cu_clos_self(_subseteq_iv_adaptor);
    cu_debug_assert(cuex_meta_is_opr(cuex_meta(e0)) &&
		    cuex_meta_is_opr(cuex_meta(e1)) &&
		    cuex_opr_r(cuex_meta(e0)) > self->value_index &&
		    cuex_opr_r(cuex_meta(e1)) > self->value_index);
    return cu_call(self->value_subseteq,
		   cuex_opn_at(e0, self->value_index),
		   cuex_opn_at(e1, self->value_index));
}

cu_clos_def(_subseteq_kv_adaptor,
	    cu_prot(cu_bool_t, cuex_t e0, cuex_t e1),
  ( cu_clop(value_subseteq, cu_bool_t, cuex_t key, cuex_t v0, cuex_t v1); ))
{
    cu_clos_self(_subseteq_kv_adaptor);
    cu_debug_assert(cuex_meta_is_opr(cuex_meta(e0)) &&
		    cuex_meta_is_opr(cuex_meta(e1)) &&
		    cuex_opr_r(cuex_meta(e0)) >= 2 &&
		    cuex_opr_r(cuex_meta(e1)) >= 2);
    cu_debug_assert(cuex_opn_at(e0, 0) == cuex_opn_at(e1, 0));
    return cu_call(self->value_subseteq, cuex_opn_at(e0, 0),
		   cuex_opn_at(e0, 1), cuex_opn_at(e1, 1));
}

cu_clop_def(order_eq, cu_order_t, cuex_t e0, cuex_t e1)
{
    return cu_order_eq;
}

cu_clos_def(_order_iv_adaptor,
	    cu_prot(cu_order_t, cuex_t e0, cuex_t e1),
  ( cu_rank_t value_index;
    cu_clop(value_order, cu_order_t, cuex_t, cuex_t); ))
{
    cu_clos_self(_order_iv_adaptor);
    cu_debug_assert(cuex_meta_is_opr(cuex_meta(e0)) &&
		    cuex_meta_is_opr(cuex_meta(e1)) &&
		    cuex_opr_r(cuex_meta(e0)) > self->value_index &&
		    cuex_opr_r(cuex_meta(e1)) > self->value_index);
    return cu_call(self->value_order,
		   cuex_opn_at(e0, self->value_index),
		   cuex_opn_at(e1, self->value_index));
}

cu_clos_def(_order_kv_adaptor,
	    cu_prot(cu_order_t, cuex_t e0, cuex_t e1),
  ( cu_clop(value_order, cu_order_t, cuex_t, cuex_t, cuex_t); ))
{
    cu_clos_self(_order_kv_adaptor);
    cu_debug_assert(cuex_meta_is_opr(cuex_meta(e0)) &&
		    cuex_meta_is_opr(cuex_meta(e1)) &&
		    cuex_opr_r(cuex_meta(e0)) >= 2 &&
		    cuex_opr_r(cuex_meta(e1)) >= 2);
    cu_debug_assert(cuex_opn_at(e0, 0) == cuex_opn_at(e1, 0));
    return cu_call(self->value_order, cuex_opn_at(e0, 0),
		   cuex_opn_at(e0, 1), cuex_opn_at(e1, 1));
}

typedef struct cuex_anode *cuex_atree_t;

struct cuex_anode
{
    CUOO_HCOBJ
    cu_word_t centre;
    cuex_t left, right;
};

cuoo_type_t cuexP_anode_type;

#define NODE(tree) ((cuex_atree_t)(tree))

#define PAIR_CENTRE(left, right) \
    pair_centre((cu_word_t)(left), (cu_word_t)(right))
#define anode_meta cuoo_type_to_meta(cuexP_anode_type)

CU_SINLINE cu_word_t
pair_centre(cu_word_t left, cu_word_t right)
{
    cu_word_t mask;
    mask = cu_word_dcover(left ^ right);
    return (left & ~mask) | ((mask + CU_WORD_C(1)) >> 1);
}

CU_SINLINE cu_word_t
centre_to_min(cu_word_t centre)
{
    cu_word_t mask = centre ^ (centre - 1);
    return centre & ~mask;
}

CU_SINLINE cu_word_t
centre_to_max(cu_word_t centre)
{
    cu_word_t mask = centre ^ (centre - 1);
    return centre | mask;
}

static cuex_atree_t
node_new(cu_word_t centre, cuex_t left, cuex_t right)
{
    cuoo_hctem_decl(cuex_anode, key);
    cuoo_hctem_init(cuex_anode, key);
    cuoo_hctem_get(cuex_anode, key)->centre = centre;
    cuoo_hctem_get(cuex_anode, key)->left = left;
    cuoo_hctem_get(cuex_anode, key)->right = right;
    return cuoo_hctem_new(cuex_anode, key);
}

cuex_t
cuex_atree_find(cu_clop(get_key, cu_word_t, cuex_t),
		cuex_t tree, cu_word_t key)
{
    cuex_meta_t tree_meta;
    if (cuex_atree_is_empty(tree))
	return NULL;
tail_call:
    tree_meta = cuex_meta(tree);
    if (tree_meta != anode_meta) {
	if (cu_call(get_key, tree) == key)
	    return tree;
	else
	    return NULL;
    } else {
	cu_word_t centre = NODE(tree)->centre;
	if ((cu_word_t)key < centre) {
	    if ((cu_word_t)key < centre_to_min(centre))
		return NULL;
	    else {
		tree = NODE(tree)->left;
		goto tail_call;
	    }
	} else {
	    if (key > centre_to_max(centre))
		return NULL;
	    else {
		tree = NODE(tree)->right;
		goto tail_call;
	    }
	}
    }
}

size_t
cuex_atree_find_index(cu_clop(get_key, cu_word_t, cuex_t),
		      cuex_t tree, cu_word_t key)
{
    size_t index = 0;
    if (cuex_atree_is_empty(tree))
	return (size_t)-1;
    while (cuex_meta(tree) == anode_meta) {
	cu_word_t centre = NODE(tree)->centre;
	if ((cu_word_t)key < centre) {
	    if ((cu_word_t)key < centre_to_min(centre))
		return (size_t)-1;
	    tree = NODE(tree)->left;
	} else {
	    if (key > centre_to_max(centre))
		return (size_t)-1;
	    index += atree_card(NODE(tree)->left);
	    tree = NODE(tree)->right;
	}
    }
    if (cu_call(get_key, tree) != key)
	return -1;
    return index;
}

static cuex_t
atree_insert(cu_clop(get_key, cu_word_t, cuex_t),
	     cu_clop(merge_values, cuex_t, cuex_t, cuex_t),
	     cuex_t tree, cuex_t value)
{
    cu_word_t value_key = cu_call(get_key, value);
    cuex_meta_t tree_meta;
    tree_meta = cuex_meta(tree);
    if (tree_meta != anode_meta) {
	cu_word_t atree_key;
	atree_key = cu_call(get_key, tree);
	if (value_key == atree_key)
	    return cu_call(merge_values, tree, value);
	else if (value_key < atree_key)
	    return node_new(PAIR_CENTRE(value_key, atree_key), value, tree);
	else
	    return node_new(PAIR_CENTRE(atree_key, value_key), tree, value);
    } else {
	cu_word_t centre = NODE(tree)->centre;
	if ((cu_word_t)value_key < centre) {
	    if ((cu_word_t)value_key < centre_to_min(centre))
		return node_new(PAIR_CENTRE(value_key, centre), value, tree);
	    else {
		cuex_t left = atree_insert(get_key, merge_values,
					   NODE(tree)->left, value);
		if (!left) return NULL;
		return node_new(centre, left, NODE(tree)->right);
	    }
	} else {
	    if ((cu_word_t)value_key > centre_to_max(centre))
		return node_new(PAIR_CENTRE(centre, value_key), tree, value);
	    else {
		cuex_t right = atree_insert(get_key, merge_values,
					    NODE(tree)->right, value);
		if (!right) return NULL;
		return node_new(centre, NODE(tree)->left, right);
	    }
	}
    }
}

/* As above, but calls merge_values with arguments reversed.  This is used to
 * get the argument ordering right in atree_union. */
static cuex_t
atree_insert_rev(cu_clop(get_key, cu_word_t, cuex_t),
		 cu_clop(merge_values, cuex_t, cuex_t, cuex_t),
		 cuex_t tree, cuex_t value)
{
    cu_word_t value_key = cu_call(get_key, value);
    cuex_meta_t tree_meta;
    tree_meta = cuex_meta(tree);
    if (tree_meta != anode_meta) {
	cu_word_t atree_key;
	atree_key = cu_call(get_key, tree);
	if (value_key == atree_key)
	    return cu_call(merge_values, value, tree);
	else if (value_key < atree_key)
	    return node_new(PAIR_CENTRE(value_key, atree_key), value, tree);
	else
	    return node_new(PAIR_CENTRE(atree_key, value_key), tree, value);
    } else {
	cu_word_t centre = NODE(tree)->centre;
	if ((cu_word_t)value_key < centre) {
	    if ((cu_word_t)value_key < centre_to_min(centre))
		return node_new(PAIR_CENTRE(value_key, centre), value, tree);
	    else {
		cuex_t left = atree_insert_rev(get_key, merge_values,
					       NODE(tree)->left, value);
		if (!left) return NULL;
		return node_new(centre, left, NODE(tree)->right);
	    }
	} else {
	    if ((cu_word_t)value_key > centre_to_max(centre))
		return node_new(PAIR_CENTRE(centre, value_key), tree, value);
	    else {
		cuex_t right = atree_insert_rev(get_key, merge_values,
						NODE(tree)->right, value);
		if (!right) return NULL;
		return node_new(centre, NODE(tree)->left, right);
	    }
	}
    }
}

cuex_t
cuex_atree_insert(cu_clop(get_key, cu_word_t, cuex_t),
		  cuex_t tree, cuex_t value)
{
    if (cuex_atree_is_empty(tree))
	return value;
    return atree_insert(get_key, _merge_as_first, tree, value);
}

cuex_t
cuex_atree_replace(cu_clop(get_key, cu_word_t, cuex_t),
		   cuex_t tree, cuex_t value)
{
    if (cuex_atree_is_empty(tree))
	return value;
    return atree_insert(get_key, _merge_as_second, tree, value);
}

cuex_t
cuex_atree_deep_insert(cu_clop(get_key, cu_word_t, cuex_t),
		       cu_clop(merge_values, cuex_t, cuex_t, cuex_t),
		       cuex_t tree, cuex_t value)
{
    if (cuex_atree_is_empty(tree))
	return value;
    return atree_insert(get_key, merge_values, tree, value);
}

cuex_t
cuex_atree_deep_insert_iv(cu_clop(get_key, cu_word_t, cuex_t),
			  cu_rank_t i,
			  cu_clop(merge_values, cuex_t, cuex_t, cuex_t),
			  cuex_t tree, cuex_t value)
{
    _merge_iv_adaptor_t adaptor;
    adaptor.value_index = i;
    adaptor.value_merge = merge_values;
    return cuex_atree_deep_insert(get_key, _merge_iv_adaptor_prep(&adaptor),
				  tree, value);
}

cuex_t
cuex_atree_deep_insert_kv(cu_clop(merge_values, cuex_t,
				  cuex_t, cuex_t, cuex_t),
			  cuex_t tree, cuex_t value)
{
    _merge_kv_adaptor_t adaptor;
    adaptor.merge = merge_values;
    return cuex_atree_deep_insert(get_key_0, _merge_kv_adaptor_prep(&adaptor),
				  tree, value);
}

static cuex_t
atree_erase(cu_clop(get_key, cu_word_t, cuex_t), cuex_t tree, cu_word_t key,
	    cuex_t *elt_out)
{
    cuex_meta_t tree_meta;
    tree_meta = cuex_meta(tree);
    if (tree_meta != anode_meta) {
	cu_word_t atree_key;
	atree_key = cu_call(get_key, tree);
	if (key == atree_key) {
	    *elt_out = tree;
	    return NULL;
	}
	else
	    return tree;
    } else {
	cu_word_t centre = NODE(tree)->centre;
	if ((cu_word_t)key < centre) {
	    if ((cu_word_t)key < centre_to_min(centre))
		return tree;
	    else {
		cuex_t new_left;
		new_left = atree_erase(get_key, NODE(tree)->left, key, elt_out);
		if (new_left == NULL)
		    return NODE(tree)->right;
		else
		    return node_new(centre, new_left, NODE(tree)->right);
	    }
	} else {
	    if ((cu_word_t)key > centre_to_max(centre))
		return tree;
	    else {
		cuex_t new_right;
		new_right = atree_erase(get_key, NODE(tree)->right, key,
					elt_out);
		if (new_right == NULL)
		    return NODE(tree)->left;
		else
		    return node_new(centre, NODE(tree)->left, new_right);
	    }
	}
    }
}

cuex_t
cuex_atree_erase(cu_clop(get_key, cu_word_t, cuex_t),
		 cuex_t tree, cu_word_t key)
{
    cuex_t elt;
    cuex_t res;
    if (cuex_atree_is_empty(tree))
	return tree;
    res = atree_erase(get_key, tree, key, &elt);
    if (res == NULL)
	return cuex_atree_empty();
    else
	return res;
}

cuex_t
cuex_atree_find_erase(cu_clop(get_key, cu_word_t, cuex_t),
		      cuex_t *tree, cu_word_t key)
{
    cuex_t elt = NULL;
    if (cuex_atree_is_empty(*tree))
	return NULL;
    *tree = atree_erase(get_key, *tree, key, &elt);
    if (*tree == NULL)
	*tree = cuex_atree_empty();
    return elt;
}

static cuex_t
atree_union(cu_clop(get_key, cu_word_t, cuex_t),
	    cu_clop(merge_values, cuex_t, cuex_t, cuex_t),
	    cuex_t tree0, cuex_t tree1)
{
    cuex_meta_t meta0 = cuex_meta(tree0);
    cuex_meta_t meta1 = cuex_meta(tree1);
    if (meta0 != anode_meta)
	return atree_insert_rev(get_key, merge_values, tree1, tree0);
    else if (meta1 != anode_meta)
	return atree_insert(get_key, merge_values, tree0, tree1);
    else {
	cu_word_t centre0 = NODE(tree0)->centre;
	cu_word_t centre1 = NODE(tree1)->centre;
	if (centre0 == centre1) {
	    cuex_t left, right;
	    left = atree_union(get_key, merge_values,
			       NODE(tree0)->left, NODE(tree1)->left);
	    if (!left) return NULL;
	    right = atree_union(get_key, merge_values,
				NODE(tree0)->right, NODE(tree1)->right);
	    if (!right) return NULL;
	    return node_new(centre0, left, right);
	}
	else if (centre0 < centre1) {
	    cu_word_t max0 = centre_to_max(centre0);
	    if (max0 < centre1) {
		cu_word_t min1 = centre_to_min(centre1);
		if (max0 < min1)		/* [---] [---] */
		    return node_new(pair_centre(centre0, centre1),
				    tree0, tree1);
		else {				/* [[---]---] */
		    cuex_t left = atree_union(get_key, merge_values, tree0,
					      NODE(tree1)->left);
		    if (!left) return NULL;
		    return node_new(centre1, left, NODE(tree1)->right);
		}
	    }
	    else {				/* [---[---]] */
		cuex_t right = atree_union(get_key, merge_values,
					   NODE(tree0)->right, tree1);
		if (!right) return NULL;
		return node_new(centre0, NODE(tree0)->left, right);
	    }
	}
	else {  /* As above but with tree0 and tree1 swapped. */
	    cu_word_t max1 = centre_to_max(centre1);
	    if (max1 < centre0) {
		cu_word_t min0 = centre_to_min(centre0);
		if (max1 < min0)
		    return node_new(pair_centre(centre1, centre0),
				    tree1, tree0);
		else {
		    cuex_t left = atree_union(get_key, merge_values,
					      NODE(tree0)->left, tree1);
		    if (!left) return NULL;
		    return node_new(centre0, left, NODE(tree0)->right);
		}
	    }
	    else {
		cuex_t right = atree_union(get_key, merge_values,
					   tree0, NODE(tree1)->right);
		if (!right) return NULL;
		return node_new(centre1, NODE(tree1)->left, right);
	    }
	}
    }
}

cuex_t
cuex_atree_left_union(cu_clop(get_key, cu_word_t, cuex_t),
		      cuex_t tree0, cuex_t tree1)
{
    if (cuex_atree_is_empty(tree0))
	return tree1;
    if (cuex_atree_is_empty(tree1))
	return tree0;
    return atree_union(get_key, _merge_as_first, tree0, tree1);
}

cuex_t
cuex_atree_disjoint_union(cu_clop(get_key, cu_word_t, cuex_t),
			  cuex_t tree0, cuex_t tree1)
{
    if (cuex_atree_is_empty(tree0))
	return tree1;
    if (cuex_atree_is_empty(tree1))
	return tree0;
    return atree_union(get_key, _merge_disjoin, tree0, tree1);
}

cuex_t
cuex_atree_deep_union(cu_clop(get_key, cu_word_t, cuex_t),
		      cu_clop(merge_values, cuex_t, cuex_t, cuex_t),
		      cuex_t tree0, cuex_t tree1)
{
    if (cuex_atree_is_empty(tree0))
	return tree1;
    if (cuex_atree_is_empty(tree1))
	return tree0;
    return atree_union(get_key, merge_values, tree0, tree1);
}

cuex_t
cuex_atree_deep_union_iv(cu_clop(get_key, cu_word_t, cuex_t),
			 cu_rank_t value_index,
			 cu_clop(value_merge, cuex_t, cuex_t, cuex_t),
			 cuex_t tree0, cuex_t tree1)
{
    _merge_iv_adaptor_t adaptor;
    if (cuex_atree_is_empty(tree0))
	return tree1;
    if (cuex_atree_is_empty(tree1))
	return tree0;
    adaptor.value_index = value_index;
    adaptor.value_merge = value_merge;
    return atree_union(get_key, _merge_iv_adaptor_prep(&adaptor),
		       tree0, tree1);
}

cuex_t
cuex_atree_deep_union_kv(cu_clop(merge, cuex_t, cuex_t, cuex_t, cuex_t),
			 cuex_t tree0, cuex_t tree1)
{
    _merge_kv_adaptor_t adaptor;
    if (cuex_atree_is_empty(tree0))
	return tree1;
    if (cuex_atree_is_empty(tree1))
	return tree0;
    adaptor.merge = merge;
    return atree_union(get_key_0, _merge_kv_adaptor_prep(&adaptor),
		       tree0, tree1);
}

static cuex_t
atree_isecn(cu_clop(get_key, cu_word_t, cuex_t),
	    cu_clop(merge_values, cuex_t, cuex_t, cuex_t),
	    cuex_t tree0, cuex_t tree1)
{
    cuex_meta_t meta0 = cuex_meta(tree0);
    cuex_meta_t meta1 = cuex_meta(tree1);
    if (meta0 != anode_meta) {
	cuex_t leaf1;
	leaf1 = cuex_atree_find(get_key, tree1, cu_call(get_key, tree0));
	if (leaf1)
	    return cu_call(merge_values, tree0, leaf1);
	else
	    return NULL;
    }
    else if (meta1 != anode_meta) {
	cuex_t leaf0;
	leaf0 = cuex_atree_find(get_key, tree0, cu_call(get_key, tree1));
	if (leaf0)
	    return cu_call(merge_values, leaf0, tree1);
	else
	    return NULL;
    }
    else {
	cu_word_t centre0 = NODE(tree0)->centre;
	cu_word_t centre1 = NODE(tree1)->centre;
	if (centre0 == centre1) {
	    cuex_t new_left;
	    cuex_t new_right;
	    new_left = atree_isecn(get_key, merge_values,
				   NODE(tree0)->left, NODE(tree1)->left);
	    new_right = atree_isecn(get_key, merge_values,
				    NODE(tree0)->right, NODE(tree1)->right);
	    if (!new_left)
		return new_right;
	    if (!new_right)
		return new_left;
	    return node_new(centre0, new_left, new_right);
	}
	else if (centre0 < centre1) {
	    cu_word_t max0 = centre_to_max(centre0);
	    if (max0 < centre1) {
		cu_word_t min1 = centre_to_min(centre1);
		if (max0 < min1)		/* [---] [---] */
		    return NULL;
		else				/* [[---]---] */
		    return atree_isecn(get_key, merge_values,
				       tree0, NODE(tree1)->left);
	    }
	    else				/* [---[---]] */
		return atree_isecn(get_key, merge_values,
				   NODE(tree0)->right, tree1);
	}
	else {  /* As above but with tree0 and tree1 swapped. */
	    cu_word_t max1 = centre_to_max(centre1);
	    if (max1 < centre0) {
		cu_word_t min0 = centre_to_min(centre0);
		if (max1 < min0)
		    return NULL;
		else
		    return atree_isecn(get_key, merge_values,
				       NODE(tree0)->left, tree1);
	    }
	    else
		return atree_isecn(get_key, merge_values,
				   tree0, NODE(tree1)->right);
	}
    }
}

cuex_t
cuex_atree_left_isecn(cu_clop(get_key, cu_word_t, cuex_t),
		      cuex_t tree0, cuex_t tree1)
{
    if (cuex_atree_is_empty(tree0))
	return tree0;
    if (cuex_atree_is_empty(tree1))
	return tree1;
    return atree_isecn(get_key, _merge_as_first, tree0, tree1);
}

cuex_t
cuex_atree_deep_isecn(cu_clop(get_key, cu_word_t, cuex_t),
		      cu_clop(merge_values, cuex_t, cuex_t, cuex_t),
		      cuex_t tree0, cuex_t tree1)
{
    if (cuex_atree_is_empty(tree0))
	return tree0;
    if (cuex_atree_is_empty(tree1))
	return tree1;
    return atree_isecn(get_key, merge_values, tree0, tree1);
}

cuex_t
cuex_atree_deep_isecn_iv(cu_clop(get_key, cu_word_t, cuex_t),
			 cu_rank_t value_index,
			 cu_clop(value_merge, cuex_t, cuex_t, cuex_t),
			 cuex_t tree0, cuex_t tree1)
{
    _merge_iv_adaptor_t adaptor;
    if (cuex_atree_is_empty(tree0))
	return tree0;
    if (cuex_atree_is_empty(tree1))
	return tree1;
    adaptor.value_index = value_index;
    adaptor.value_merge = value_merge;
    return atree_isecn(get_key, _merge_iv_adaptor_prep(&adaptor), tree0, tree1);
}

cuex_t
cuex_atree_deep_isecn_kv(cu_clop(merge, cuex_t, cuex_t, cuex_t, cuex_t),
			 cuex_t tree0, cuex_t tree1)
{
    _merge_kv_adaptor_t adaptor;
    if (cuex_atree_is_empty(tree0))
	return tree0;
    if (cuex_atree_is_empty(tree1))
	return tree1;
    adaptor.merge = merge;
    return atree_isecn(get_key_0, _merge_kv_adaptor_prep(&adaptor),
		       tree0, tree1);
}

static cu_bool_t
atree_subseteq(cu_clop(get_key, cu_word_t, cuex_t),
	       cu_clop(value_subseteq, cu_bool_t, cuex_t, cuex_t),
	       cuex_t tree0, cuex_t tree1)
{
    cuex_meta_t meta0, meta1;
    if (tree0 == tree1)
	return cu_true;
    meta0 = cuex_meta(tree0);
    meta1 = cuex_meta(tree1);
    if (meta0 != anode_meta) {
	cuex_t leaf1;
	leaf1 = cuex_atree_find(get_key, tree1, cu_call(get_key, tree0));
	if (leaf1)
	    return cu_call(value_subseteq, tree0, leaf1);
	else
	    return cu_false;
    }
    else if (meta1 != anode_meta)
	return cu_false;
    else {
	cu_word_t centre0 = NODE(tree0)->centre;
	cu_word_t centre1 = NODE(tree1)->centre;
	if (centre0 == centre1) {
	    if (!atree_subseteq(get_key, value_subseteq,
				NODE(tree0)->left, NODE(tree1)->left))
		return cu_false;
	    else
		return atree_subseteq(get_key, value_subseteq,
				      NODE(tree0)->right, NODE(tree1)->right);
	} else if (centre0 < centre1) {
	    cu_word_t min1 = centre_to_min(centre1);
	    if (min1 < centre0)		/* [[0]1---] */
		return atree_subseteq(get_key, value_subseteq,
				      tree0, NODE(tree1)->left);
	    else
		return cu_false;
	} else { /* centre1 < centre0 */
	    cu_word_t max1 = centre_to_max(centre1);
	    if (max1 > centre0)		/* [---1[0]] */
		return atree_subseteq(get_key, value_subseteq,
				      tree0, NODE(tree1)->right);
	    else
		return cu_false;
	}
    }
}

static cu_order_t
atree_order(cu_clop(get_key, cu_word_t, cuex_t),
	    cu_clop(value_subseteq, cu_bool_t, cuex_t, cuex_t),
	    cu_clop(value_order, cu_order_t, cuex_t, cuex_t),
	    cuex_t tree0, cuex_t tree1)
{
    cuex_meta_t meta0, meta1;
    if (tree0 == tree1)
	return cu_order_eq;
    meta0 = cuex_meta(tree0);
    meta1 = cuex_meta(tree1);
    if (meta0 != anode_meta) {
	if (meta1 != anode_meta) {
	    if (cu_call(get_key, tree0) == cu_call(get_key, tree1))
		return cu_call(value_order, tree0, tree1);
	    else
		return cu_order_none;
	} else {
	    cuex_t leaf1;
	    leaf1 = cuex_atree_find(get_key, tree1, cu_call(get_key, tree0));
	    if (leaf1 && cu_call(value_subseteq, tree0, leaf1))
		return cu_order_lt;
	    else
		return cu_order_none;
	}
    } else if (meta1 != anode_meta) {
	cuex_t leaf0;
	leaf0 = cuex_atree_find(get_key, tree0, cu_call(get_key, tree1));
	if (leaf0 && cu_call(value_subseteq, tree1, leaf0))
	    return cu_order_gt;
	else
	    return cu_order_none;
    } else {
	cu_word_t centre0 = NODE(tree0)->centre;
	cu_word_t centre1 = NODE(tree1)->centre;
	if (centre0 == centre1) {
	    cu_order_t left_order, right_order;
	    left_order = atree_order(get_key, value_subseteq, value_order,
				     NODE(tree0)->left, NODE(tree1)->left);
	    if (left_order == cu_order_none)
		return cu_order_none;
	    right_order = atree_order(get_key, value_subseteq, value_order,
				      NODE(tree0)->right, NODE(tree1)->right);
	    if (left_order == right_order)
		return left_order;
	    else if (left_order == cu_order_eq)
		return right_order;
	    else if (right_order == cu_order_eq)
		return left_order;
	    else
		return cu_order_none;
	} else if (centre0 < centre1) {
	    cu_word_t max0 = centre_to_max(centre0);
	    if (max0 < centre1) {
		cu_word_t min1 = centre_to_min(centre1);
		if (max0 < min1)	/* [---] [---] */
		    return cu_order_none;
		else {			/* [[---]---] */
		    if (atree_subseteq(get_key, value_subseteq,
				       tree0, NODE(tree1)->left))
			return cu_order_lt;
		    else
			return cu_order_none;
		}
	    } else {			/* [---[---]] */
		if (atree_subseteq(get_key, value_subseteq,
				   tree1, NODE(tree0)->right))
		    return cu_order_gt;
		else
		    return cu_order_none;
	    }
	} else { /* centre1 < centre0 */
	    cu_word_t max1 = centre_to_max(centre1);
	    if (max1 < centre0) {
		cu_word_t min0 = centre_to_min(centre0);
		if (max1 < min0)
		    return cu_order_none;
		else {
		    if (atree_subseteq(get_key, value_subseteq,
				       tree1, NODE(tree0)->left))
			return cu_order_gt;
		    else
			return cu_order_none;
		}
	    } else {
		if (atree_subseteq(get_key, value_subseteq,
				   tree0, NODE(tree1)->right))
		    return cu_order_lt;
		else
		    return cu_order_none;
	    }
	}
    }
}

cu_bool_t
cuex_atree_subseteq(cu_clop(get_key, cu_word_t, cuex_t),
		    cuex_t tree0, cuex_t tree1)
{
    if (tree0 == tree1)
	return cu_true;
    else if (cuex_atree_is_empty(tree0))
	return cu_true;
    else if (cuex_atree_is_empty(tree1))
	return cu_false;
    else
	return atree_subseteq(get_key, _subseteq_true, tree0, tree1);
}

cu_bool_t
cuex_atree_deep_subseteq(cu_clop(get_key, cu_word_t, cuex_t),
			 cu_clop(value_subseteq, cu_bool_t, cuex_t, cuex_t),
			 cuex_t tree0, cuex_t tree1)
{
    if (tree0 == tree1)
	return cu_true;
    else if (cuex_atree_is_empty(tree0))
	return cu_true;
    else if (cuex_atree_is_empty(tree1))
	return cu_false;
    else
	return atree_subseteq(get_key, value_subseteq, tree0, tree1);
}

cu_bool_t
cuex_atree_deep_subseteq_iv(cu_clop(get_key, cu_word_t, cuex_t),
			    cu_rank_t value_index,
			    cu_clop(value_subseteq, cu_bool_t, cuex_t, cuex_t),
			    cuex_t tree0, cuex_t tree1)
{
    if (tree0 == tree1)
	return cu_true;
    else if (cuex_atree_is_empty(tree0))
	return cu_true;
    else if (cuex_atree_is_empty(tree1))
	return cu_false;
    else {
	_subseteq_iv_adaptor_t adaptor;
	adaptor.value_index = value_index;
	adaptor.value_subseteq = value_subseteq;
	return atree_subseteq(get_key, _subseteq_iv_adaptor_prep(&adaptor),
			      tree0, tree1);
    }
}

cu_bool_t
cuex_atree_deep_subseteq_kv(cu_clop(value_subseteq, cu_bool_t,
				    cuex_t, cuex_t, cuex_t),
			    cuex_t tree0, cuex_t tree1)
{
    if (tree0 == tree1)
	return cu_true;
    else if (cuex_atree_is_empty(tree0))
	return cu_true;
    else if (cuex_atree_is_empty(tree1))
	return cu_false;
    else {
	_subseteq_kv_adaptor_t adaptor;
	adaptor.value_subseteq = value_subseteq;
	return atree_subseteq(get_key_0, _subseteq_kv_adaptor_prep(&adaptor),
			      tree0, tree1);
    }
}

cu_order_t
cuex_atree_order(cu_clop(get_key, cu_word_t, cuex_t),
		 cuex_t tree0, cuex_t tree1)
{
    if (tree0 == tree1)
	return cu_order_eq;
    else if (cuex_atree_is_empty(tree0))
	return cu_order_lt;
    else if (cuex_atree_is_empty(tree1))
	return cu_order_gt;
    else
	return atree_order(get_key, _subseteq_true, order_eq, tree0, tree1);
}

cu_order_t
cuex_atree_deep_order_iv(cu_clop(get_key, cu_word_t, cuex_t),
			 cu_rank_t value_index,
			 cu_clop(value_subseteq, cu_bool_t, cuex_t, cuex_t),
			 cu_clop(value_order, cu_order_t, cuex_t, cuex_t),
			 cuex_t tree0, cuex_t tree1)
{
    if (tree0 == tree1)
	return cu_order_eq;
    else if (cuex_atree_is_empty(tree0))
	return cu_order_lt;
    else if (cuex_atree_is_empty(tree1))
	return cu_order_gt;
    else {
	_subseteq_iv_adaptor_t subseteq_adaptor;
	_order_iv_adaptor_t order_adaptor;
	subseteq_adaptor.value_index = value_index;
	subseteq_adaptor.value_subseteq = value_order;
	order_adaptor.value_index = value_index;
	order_adaptor.value_order = value_order;
	return atree_order(get_key,
			   _subseteq_iv_adaptor_prep(&subseteq_adaptor),
			   _order_iv_adaptor_prep(&order_adaptor),
			   tree0, tree1);
    }
}

cu_order_t
cuex_atree_deep_order_kv(cu_clop(value_subseteq, cu_bool_t,
				 cuex_t, cuex_t, cuex_t),
			 cu_clop(value_order, cu_order_t,
				 cuex_t, cuex_t, cuex_t),
			 cuex_t tree0, cuex_t tree1)
{
    if (tree0 == tree1)
	return cu_order_eq;
    else if (cuex_atree_is_empty(tree0))
	return cu_order_lt;
    else if (cuex_atree_is_empty(tree1))
	return cu_order_gt;
    else {
	_subseteq_kv_adaptor_t subseteq_adaptor;
	_order_kv_adaptor_t order_adaptor;
	subseteq_adaptor.value_subseteq = value_subseteq;
	order_adaptor.value_order = value_order;
	return atree_order(get_key_0,
			   _subseteq_kv_adaptor_prep(&subseteq_adaptor),
			   _order_kv_adaptor_prep(&order_adaptor),
			   tree0, tree1);
    }
}

static void
atree_iter(cuex_t tree, cu_clop(f, void, cuex_t))
{
    cuex_meta_t tree_meta;
tail_call:
    tree_meta = cuex_meta(tree);
    if (tree_meta == anode_meta) {
	atree_iter(NODE(tree)->left, f);
	tree = NODE(tree)->right;
	goto tail_call;
    } else
	cu_call(f, tree);
}

void
cuex_atree_iter(cuex_t tree, cu_clop(f, void, cuex_t))
{
    if (cuex_atree_is_empty(tree))
	return;
    atree_iter(tree, f);
}

static cu_bool_t
atree_conj(cuex_t tree, cu_clop(f, cu_bool_t, cuex_t))
{
    cuex_meta_t tree_meta;
tail_call:
    tree_meta = cuex_meta(tree);
    if (tree_meta == anode_meta) {
	if (!atree_conj(NODE(tree)->left, f))
	    return cu_false;
	tree = NODE(tree)->right;
	goto tail_call;
    } else
	return cu_call(f, tree);
}

cu_bool_t
cuex_atree_conj(cuex_t tree, cu_clop(f, cu_bool_t, cuex_t))
{
    if (cuex_atree_is_empty(tree))
	return cu_true;
    return atree_conj(tree, f);
}

static cu_bool_t
atree_conj_iv(cuex_t tree, cu_rank_t i, cu_clop(f, cu_bool_t, cuex_t))
{
    cuex_meta_t tree_meta;
tail_call:
    tree_meta = cuex_meta(tree);
    if (tree_meta == anode_meta) {
	if (!atree_conj_iv(NODE(tree)->left, i, f))
	    return cu_false;
	tree = NODE(tree)->right;
	goto tail_call;
    } else {
	cu_debug_assert(cuex_meta_is_opr(tree_meta) &&
			cuex_opr_r(tree_meta) > i);
	return cu_call(f, cuex_opn_at(tree, i));
    }
}

cu_bool_t
cuex_atree_conj_iv(cuex_t tree, cu_rank_t i, cu_clop(f, cu_bool_t, cuex_t))
{
    if (cuex_atree_is_empty(tree))
	return cu_true;
    return atree_conj_iv(tree, i, f);
}

static cu_bool_t
atree_conj_kv(cuex_t tree, cu_clop(f, cu_bool_t, cuex_t, cuex_t))
{
    cuex_meta_t tree_meta;
tail_call:
    tree_meta = cuex_meta(tree);
    if (tree_meta == anode_meta) {
	if (!atree_conj_kv(NODE(tree)->left, f))
	    return cu_false;
	tree = NODE(tree)->right;
	goto tail_call;
    } else {
	cu_debug_assert(cuex_meta_is_opr(tree_meta) &&
			cuex_opr_r(tree_meta) >= 2);
	return cu_call(f, cuex_opn_at(tree, 0), cuex_opn_at(tree, 1));
    }
}

cu_bool_t
cuex_atree_conj_kv(cuex_t tree, cu_clop(f, cu_bool_t, cuex_t, cuex_t))
{
    if (cuex_atree_is_empty(tree))
	return cu_true;
    return atree_conj_kv(tree, f);
}

static void
atree_image(cuex_t tree, cu_clop(f, cuex_t, cuex_t),
	    cu_clop(get_key, cu_word_t, cuex_t), cuex_t *accu)
{
    cuex_meta_t tree_meta;
tail_call:
    tree_meta = cuex_meta(tree);
    if (tree_meta == anode_meta) {
	atree_image(NODE(tree)->left, f, get_key, accu);
	tree = NODE(tree)->right;
	goto tail_call;
    } else {
	cuex_t e = cu_call(f, tree);
	if (!*accu)
	    *accu = e;
	else
	    *accu = atree_insert(get_key, _merge_as_first, *accu, e);
    }
}

cuex_t
cuex_atree_image(cuex_t tree, cu_clop(f, cuex_t, cuex_t),
		 cu_clop(get_key, cu_word_t, cuex_t))
{
    cuex_t accu;
    if (cuex_atree_is_empty(tree))
	return tree;
    accu = NULL;
    atree_image(tree, f, get_key, &accu);
    return accu;
}

static cuex_t
atree_isokey_image(cuex_t tree, cu_clop(f, cuex_t, cuex_t))
{
    cuex_meta_t tree_meta = cuex_meta(tree);
    if (tree_meta == anode_meta)
	return node_new(NODE(tree)->centre,
			atree_isokey_image(NODE(tree)->left, f),
			atree_isokey_image(NODE(tree)->right, f));
    else
	return cu_call(f, tree);
}

cuex_t
cuex_atree_isokey_image(cuex_t tree, cu_clop(f, cuex_t, cuex_t))
{
    if (cuex_atree_is_empty(tree))
	return tree;
    else
	return atree_isokey_image(tree, f);
}

static cuex_t
atree_isokey_image_iv(cuex_t tree, cu_rank_t i, cu_clop(f, cuex_t, cuex_t))
{
    cuex_meta_t tree_meta = cuex_meta(tree);
    if (tree_meta == anode_meta)
	return node_new(NODE(tree)->centre,
			atree_isokey_image_iv(NODE(tree)->left, i, f),
			atree_isokey_image_iv(NODE(tree)->right, i, f));
    else {
	cu_rank_t r = cuex_opr_r(tree_meta);
	cuex_t *arr = cu_salloc(r*sizeof(cuex_t));
	cu_debug_assert(cuex_meta_is_opr(tree_meta) && r > i);
	memcpy(arr, cuex_opn_begin(tree), r*sizeof(cuex_t));
	arr[i] = cu_call(f, arr[i]);
	return cuex_opn_by_arr(tree_meta, arr);
    }
}

cuex_t
cuex_atree_isokey_image_iv(cuex_t tree, cu_rank_t i,
			   cu_clop(f, cuex_t, cuex_t))
{
    if (cuex_atree_is_empty(tree))
	return tree;
    else
	return atree_isokey_image_iv(tree, i, f);
}

static cuex_t
atree_isokey_image_kv(cuex_t tree, cu_clop(f, cuex_t, cuex_t, cuex_t))
{
    cuex_meta_t tree_meta = cuex_meta(tree);
    if (tree_meta == anode_meta)
	return node_new(NODE(tree)->centre,
			atree_isokey_image_kv(NODE(tree)->left, f),
			atree_isokey_image_kv(NODE(tree)->right, f));
    else {
	cu_rank_t r = cuex_opr_r(tree_meta);
	cuex_t *arr = cu_salloc(r*sizeof(cuex_t));
	cu_debug_assert(cuex_meta_is_opr(tree_meta) && r >= 2);
	memcpy(arr, cuex_opn_begin(tree), r*sizeof(cuex_t));
	arr[1] = cu_call(f, arr[0], arr[1]);
	return cuex_opn_by_arr(tree_meta, arr);
    }
}

cuex_t
cuex_atree_isokey_image_kv(cuex_t tree,
			   cu_clop(f, cuex_t, cuex_t, cuex_t))
{
    if (cuex_atree_is_empty(tree))
	return tree;
    else
	return atree_isokey_image_kv(tree, f);
}

static int
atree_depth(cuex_t tree)
{
    int max_depth = 0, depth = 0;
    while (cuex_meta(tree) == anode_meta) {
	int left_depth;
	++depth;
	left_depth = depth + atree_depth(NODE(tree)->left);
	if (left_depth > max_depth)
	    max_depth = left_depth;
	tree = NODE(tree)->right;
    }
    return max_depth;
}

int
cuex_atree_depth(cuex_t tree)
{
    if (cuex_atree_is_empty(tree))
	return 0;
    else
	return atree_depth(tree);
}

static size_t
atree_card(cuex_t tree)
{
    size_t card = 1;
    while (cuex_meta(tree) == anode_meta) {
	card += atree_card(NODE(tree)->left);
	tree = NODE(tree)->right;
    }
    return card;
}

size_t
cuex_atree_card(cuex_t tree)
{
    if (cuex_atree_is_empty(tree))
	return 0;
    else
	return atree_card(tree);
}


/* == Iteration == */

struct atree_itr_s
{
    cuex_t *sp;
    cuex_t stack[1];
};

size_t
cuex_atree_itr_size(cuex_t tree)
{
    return sizeof(struct atree_itr_s)
	+ (cuex_atree_depth(tree) + 1)*sizeof(cuex_t);
}

void
cuex_atree_itr_init(void *_itr, cuex_t atree)
{
#define itr ((struct atree_itr_s *)_itr)
    itr->stack[0] = NULL;
    if (cuex_atree_is_empty(atree))
	itr->sp = &itr->stack[0];
    else {
	itr->stack[1] = atree;
	itr->sp = &itr->stack[1];
    }
#undef itr
}

cuex_t
cuex_atree_itr_get(void *_itr)
{
#define itr ((struct atree_itr_s *)_itr)
    cuex_t *sp = itr->sp;
    cuex_t tree = *sp;
    if (!tree)
	return NULL;
    sp--;
    while (cuex_meta(tree) == anode_meta) {
	*++sp = NODE(tree)->right;
	tree = NODE(tree)->left;
    }
    itr->sp = sp;
    return tree;
#undef itr
}

cuex_t
cuex_atree_itr_get_at_1(void *_itr)
{
#define itr ((struct atree_itr_s *)_itr)
    cuex_t *sp = itr->sp;
    cuex_t tree = *sp;
    if (!tree)
	return NULL;
    sp--;
    while (cuex_meta(tree) == anode_meta) {
	*++sp = NODE(tree)->right;
	tree = NODE(tree)->left;
    }
    itr->sp = sp;
    return cuex_opn_at(tree, 1);
#undef itr
}


/* == Printing == */

cu_clos_def(_atree_print_elt, cu_prot(void, cuex_t elt),
    (cufo_stream_t fos; int index;))
{
    cu_clos_self(_atree_print_elt);
    if (self->index++ != 0) {
	cufo_space(self->fos);
	cufo_tagputs(self->fos, cufoT_operator, "∧");
	cufo_space(self->fos);
    }
    cufo_print_ex(self->fos, elt);
}

static void
_atree_print(cufo_stream_t fos, cufo_prispec_t spec, cuex_t obj)
{
    if (cuex_atree_is_empty(obj))
	cufo_tagputs(fos, cufoT_literal, "⊤");
    else {
	_atree_print_elt_t cb;
	cb.fos = fos;
	cb.index = 0;
	cufo_tagputc(fos, cufoT_operator, '(');
	cuex_atree_iter(obj, _atree_print_elt_prep(&cb));
	cufo_tagputc(fos, cufoT_operator, ')');
    }
}


/* == Interfaces == */

static cu_box_t
_atree_impl(cu_word_t intf_number, ...)
{
    switch (intf_number) {
	case CUOO_INTF_FOPRINT_FN:
	    return CUOO_INTF_FOPRINT_FN_BOX(_atree_print);
	default:
	    return CUOO_IMPL_NONE;
    }
}


/* == Initialisation == */

void
cuexP_atree_init()
{
    cuexP_anode_type = cuoo_type_new_opaque_hcs(
	_atree_impl, sizeof(struct cuex_anode) - CUOO_HCOBJ_SHIFT);
}
