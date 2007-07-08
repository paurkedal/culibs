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
#include <cudyn/prop.h>
#include <cudyn/properties.h>
#include <cu/int.h>
#include <cu/algo.h>

typedef struct cuex_anode_s *cuex_atree_t;

struct cuex_anode_s
{
    CU_HCOBJ
    cu_word_t centre;
    cuex_t left, right;
};

cudyn_stdtype_t cuexP_atree_type;

#define NODE(tree) ((cuex_atree_t)(tree))

#define PAIR_CENTER(left, right) \
    pair_centre((cu_word_t)(left), (cu_word_t)(right))
#define anode_meta cudyn_stdtype_to_meta(cuexP_atree_type)

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
    struct cuex_anode_s tpl;
    tpl.centre = centre;
    tpl.left = left;
    tpl.right = right;
    return cudyn_halloc_by_value(cudyn_stdtype_to_type(cuexP_atree_type),
				 sizeof(struct cuex_anode_s), &tpl);
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

static cuex_t
atree_insert(cu_clop(get_key, cu_word_t, cuex_t), cuex_t tree, cuex_t value)
{
    cu_word_t value_key = cu_call(get_key, value);
    cuex_meta_t tree_meta;
    tree_meta = cuex_meta(tree);
    if (tree_meta != anode_meta) {
	cu_word_t atree_key;
	atree_key = cu_call(get_key, tree);
	if (value_key == atree_key)
	    return tree;
	else if (value_key < atree_key)
	    return node_new(PAIR_CENTER(value_key, atree_key), value, tree);
	else
	    return node_new(PAIR_CENTER(atree_key, value_key), tree, value);
    } else {
	cu_word_t centre = NODE(tree)->centre;
	if ((cu_word_t)value_key < centre) {
	    if ((cu_word_t)value_key < centre_to_min(centre))
		return node_new(PAIR_CENTER(value_key, centre), value, tree);
	    else
		return node_new(centre,
				atree_insert(get_key, NODE(tree)->left, value),
				NODE(tree)->right);
	} else {
	    if ((cu_word_t)value_key > centre_to_max(centre))
		return node_new(PAIR_CENTER(centre, value_key), tree, value);
	    else
		return node_new(centre,
				NODE(tree)->left,
				atree_insert(get_key,
					     NODE(tree)->right, value));
	}
    }
}

cuex_t
cuex_atree_insert(cu_clop(get_key, cu_word_t, cuex_t),
		  cuex_t tree, cuex_t value)
{
    if (cuex_atree_is_empty(tree))
	return value;
    return atree_insert(get_key, tree, value);
}

static cuex_t
atree_erase(cu_clop(get_key, cu_word_t, cuex_t), cuex_t tree, cu_word_t key)
{
    cuex_meta_t tree_meta;
    tree_meta = cuex_meta(tree);
    if (tree_meta != anode_meta) {
	cu_word_t atree_key;
	atree_key = cu_call(get_key, tree);
	if (key == atree_key)
	    return NULL;
	else
	    return tree;
    } else {
	cu_word_t centre = NODE(tree)->centre;
	if ((cu_word_t)key < centre) {
	    if ((cu_word_t)key < centre_to_min(centre))
		return tree;
	    else {
		cuex_t new_left;
		new_left = atree_erase(get_key, NODE(tree)->left, key);
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
		new_right = atree_erase(get_key, NODE(tree)->right, key);
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
    cuex_t res;
    if (cuex_atree_is_empty(tree))
	return tree;
    res = atree_erase(get_key, tree, key);
    if (res == NULL)
	return cuex_atree_empty();
    else
	return res;
}

static cuex_t
atree_union(cu_clop(get_key, cu_word_t, cuex_t), cuex_t tree0, cuex_t tree1)
{
    cuex_meta_t meta0 = cuex_meta(tree0);
    cuex_meta_t meta1 = cuex_meta(tree1);
    if (meta0 != anode_meta)
	return atree_insert(get_key, tree1, tree0);
    else if (meta1 != anode_meta)
	return atree_insert(get_key, tree0, tree1);
    else {
	cu_word_t centre0 = NODE(tree0)->centre;
	cu_word_t centre1 = NODE(tree1)->centre;
	if (centre0 == centre1)
	    return node_new(centre0,
		atree_union(get_key, NODE(tree0)->left, NODE(tree1)->left),
		atree_union(get_key, NODE(tree0)->right, NODE(tree1)->right));
	else if (centre0 < centre1) {
	    cu_word_t max0 = centre_to_max(centre0);
	    if (max0 < centre1) {
		cu_word_t min1 = centre_to_min(centre1);
		if (max0 < min1)		/* [---] [---] */
		    return node_new(pair_centre(centre0, centre1),
				    tree0, tree1);
		else				/* [[---]---] */
		    return node_new(centre1,
				atree_union(get_key, tree0, NODE(tree1)->left),
				NODE(tree1)->right);
	    }
	    else				/* [---[---]] */
		return node_new(centre0,
			    NODE(tree0)->left,
			    atree_union(get_key, NODE(tree0)->right, tree1));
	}
	else {  /* As above but with tree0 and tree1 swapped. */
	    cu_word_t max1 = centre_to_max(centre1);
	    if (max1 < centre0) {
		cu_word_t min0 = centre_to_min(centre0);
		if (max1 < min0)
		    return node_new(pair_centre(centre1, centre0),
				    tree1, tree0);
		else
		    return node_new(centre0,
				atree_union(get_key, tree1, NODE(tree0)->left),
				NODE(tree0)->right);
	    }
	    else
		return node_new(centre1,
			    NODE(tree1)->left,
			    atree_union(get_key, NODE(tree1)->right, tree0));
	}
    }
}

cuex_t
cuex_atree_union(cu_clop(get_key, cu_word_t, cuex_t),
		 cuex_t tree0, cuex_t tree1)
{
    if (cuex_atree_is_empty(tree0))
	return tree1;
    if (cuex_atree_is_empty(tree1))
	return tree0;
    return atree_union(get_key, tree0, tree1);
}

static cuex_t
atree_isecn(cu_clop(get_key, cu_word_t, cuex_t), cuex_t tree0, cuex_t tree1)
{
    cuex_meta_t meta0 = cuex_meta(tree0);
    cuex_meta_t meta1 = cuex_meta(tree1);
    if (meta0 != anode_meta)
	return cuex_atree_find(get_key, tree1, cu_call(get_key, tree0));
    else if (meta1 != anode_meta)
	return cuex_atree_find(get_key, tree0, cu_call(get_key, tree1));
    else {
	cu_word_t centre0 = NODE(tree0)->centre;
	cu_word_t centre1 = NODE(tree1)->centre;
	if (centre0 == centre1) {
	    cuex_t new_left;
	    cuex_t new_right;
	    new_left = atree_isecn(get_key,
				   NODE(tree0)->left, NODE(tree1)->left);
	    new_right = atree_isecn(get_key,
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
		    return atree_isecn(get_key, tree0, NODE(tree1)->left);
	    }
	    else				/* [---[---]] */
		return atree_isecn(get_key, NODE(tree0)->right, tree1);
	}
	else {  /* As above but with tree0 and tree1 swapped. */
	    cu_word_t max1 = centre_to_max(centre1);
	    if (max1 < centre0) {
		cu_word_t min0 = centre_to_min(centre0);
		if (max1 < min0)
		    return NULL;
		else
		    return atree_isecn(get_key, tree1, NODE(tree0)->left);
	    }
	    else
		return atree_isecn(get_key, NODE(tree1)->right, tree0);
	}
    }
}

cuex_t
cuex_atree_isecn(cu_clop(get_key, cu_word_t, cuex_t),
		 cuex_t tree0, cuex_t tree1)
{
    cuex_t e;
    if (cuex_atree_is_empty(tree0))
	return tree0;
    if (cuex_atree_is_empty(tree1))
	return tree1;
    e = atree_isecn(get_key, tree0, tree1);
    if (!e)
	e = cuex_atree_empty();
    return e;
}

static cu_bool_t
atree_subseteq(cu_clop(get_key, cu_word_t, cuex_t), cuex_t tree0, cuex_t tree1)
{
    cuex_meta_t meta0, meta1;
    if (tree0 == tree1)
	return cu_true;
    meta0 = cuex_meta(tree0);
    meta1 = cuex_meta(tree1);
    if (meta0 != anode_meta)
	return cuex_atree_find(get_key, tree1, cu_call(get_key, tree0))
	    != NULL;
    else if (meta1 != anode_meta)
	return cu_false;
    else {
	cu_word_t centre0 = NODE(tree0)->centre;
	cu_word_t centre1 = NODE(tree1)->centre;
	if (centre0 == centre1) {
	    if (!atree_subseteq(get_key, NODE(tree0)->left, NODE(tree1)->left))
		return cu_false;
	    else
		return atree_subseteq(get_key,
				      NODE(tree0)->right, NODE(tree1)->right);
	} else if (centre0 < centre1) {
	    cu_word_t min1 = centre_to_min(centre1);
	    if (min1 < centre0)		/* [[0]1---] */
		return atree_subseteq(get_key, tree0, NODE(tree1)->left);
	    else
		return cu_false;
	} else { /* centre1 < centre0 */
	    cu_word_t max1 = centre_to_max(centre1);
	    if (max1 > centre0)		/* [---1[0]] */
		return atree_subseteq(get_key, tree0, NODE(tree1)->right);
	    else
		return cu_false;
	}
    }
}

static cu_order_t
atree_order(cu_clop(get_key, cu_word_t, cuex_t),
	    cuex_t tree0, cuex_t tree1)
{
    cuex_meta_t meta0, meta1;
    if (tree0 == tree1)
	return cu_order_eq;
    meta0 = cuex_meta(tree0);
    meta1 = cuex_meta(tree1);
    if (meta0 != anode_meta) {
	if (cuex_atree_find(get_key, tree1, cu_call(get_key, tree0)))
	    return cu_order_lt;
	else
	    return cu_order_none;
    } else if (meta1 != anode_meta) {
	if (cuex_atree_find(get_key, tree0, cu_call(get_key, tree1)))
	    return cu_order_gt;
	else
	    return cu_order_none;
    } else {
	cu_word_t centre0 = NODE(tree0)->centre;
	cu_word_t centre1 = NODE(tree1)->centre;
	if (centre0 == centre1) {
	    cu_order_t left_order, right_order;
	    left_order = atree_order(get_key,
				     NODE(tree0)->left, NODE(tree1)->left);
	    if (left_order == cu_order_none)
		return cu_order_none;
	    right_order = atree_order(get_key,
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
		    if (atree_subseteq(get_key, tree0, NODE(tree1)->left))
			return cu_order_lt;
		    else
			return cu_order_none;
		}
	    } else {			/* [---[---]] */
		if (atree_subseteq(get_key, tree1, NODE(tree0)->right))
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
		    if (atree_subseteq(get_key, tree1, NODE(tree0)->left))
			return cu_order_gt;
		    else
			return cu_order_none;
		}
	    } else {
		if (atree_subseteq(get_key, tree0, NODE(tree1)->right))
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
	return atree_subseteq(get_key, tree0, tree1);
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
	return atree_order(get_key, tree0, tree1);
}

static void
atree_iter(cuex_t tree, cu_clop(fn, void, cuex_t))
{
    cuex_meta_t tree_meta;
tail_call:
    tree_meta = cuex_meta(tree);
    if (tree_meta == anode_meta) {
	atree_iter(NODE(tree)->left, fn);
	tree = NODE(tree)->right;
	goto tail_call;
    } else
	cu_call(fn, tree);
}

void
cuex_atree_iter(cuex_t tree, cu_clop(fn, void, cuex_t))
{
    if (cuex_atree_is_empty(tree))
	return;
    atree_iter(tree, fn);
}

static cu_bool_t
atree_conj(cuex_t tree, cu_clop(fn, cu_bool_t, cuex_t))
{
    cuex_meta_t tree_meta;
tail_call:
    tree_meta = cuex_meta(tree);
    if (tree_meta == anode_meta) {
	if (!atree_conj(NODE(tree)->left, fn))
	    return cu_false;
	tree = NODE(tree)->right;
	goto tail_call;
    } else
	return cu_call(fn, tree);
}

cu_bool_t
cuex_atree_conj(cuex_t tree, cu_clop(fn, cu_bool_t, cuex_t))
{
    if (cuex_atree_is_empty(tree))
	return cu_true;
    return atree_conj(tree, fn);
}

static void
atree_image(cuex_t tree, cu_clop(fn, cuex_t, cuex_t),
	    cu_clop(get_key, cu_word_t, cuex_t), cuex_t *accu)
{
    cuex_meta_t tree_meta;
tail_call:
    tree_meta = cuex_meta(tree);
    if (tree_meta == anode_meta) {
	atree_image(NODE(tree)->left, fn, get_key, accu);
	tree = NODE(tree)->right;
	goto tail_call;
    } else {
	cuex_t e = cu_call(fn, tree);
	if (!*accu)
	    *accu = e;
	else
	    *accu = atree_insert(get_key, *accu, e);
    }
}

cuex_t
cuex_atree_image(cuex_t tree, cu_clop(fn, cuex_t, cuex_t),
		 cu_clop(get_key, cu_word_t, cuex_t))
{
    cuex_t accu;
    if (cuex_atree_is_empty(tree))
	return tree;
    accu = NULL;
    atree_image(tree, fn, get_key, &accu);
    return accu;
}

static cuex_t
atree_isokey_image(cuex_t tree, cu_clop(fn, cuex_t, cuex_t))
{
    cuex_meta_t tree_meta = cuex_meta(tree);
    if (tree_meta == anode_meta)
	return node_new(NODE(tree)->centre,
			atree_isokey_image(NODE(tree)->left, fn),
			atree_isokey_image(NODE(tree)->right, fn));
    else
	return cu_call(fn, tree);
}

cuex_t
cuex_atree_isokey_image(cuex_t tree, cu_clop(fn, cuex_t, cuex_t))
{
    if (cuex_atree_is_empty(tree))
	return tree;
    else
	return atree_isokey_image(tree, fn);
}

cu_clos_def(atree_print_elt, cu_prot(void, cuex_t elt),
    (FILE *out; int index;))
{
    cu_clos_self(atree_print_elt);
    if (self->index++ != 0)
	fputs(" ∧ ", self->out);
    cu_fprintf(self->out, "%!", elt);
}

static void
atree_print(void *obj, FILE *out)
{
    atree_print_elt_t cb;
    cb.out = out;
    cb.index = 0;
    if (cuex_atree_is_empty(obj))
	fputs("⊤", out);
    else {
	fputc('(', out);
	cuex_atree_iter(obj, atree_print_elt_prep(&cb));
	fputc(')', out);
    }
}

void
cuexP_atree_init()
{
    cuexP_atree_type = cudyn_stdtype_new_hcs(sizeof(struct cuex_anode_s) -
					     CU_HCOBJ_SHIFT);

    cudyn_prop_define_ptr(cudyn_raw_print_fn_prop(),
			  cuexP_atree_type, atree_print);
}
