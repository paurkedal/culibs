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

#include <cuex/assoc.h>
#include <cuex/opn.h>
#include <cudyn/prop.h>
#include <cudyn/properties.h>
#include <cu/int.h>

typedef struct cuex_assoc_s *cuex_assoc_t;

struct cuex_assoc_s
{
    CU_HCOBJ
    cu_word_t centre;
    cuex_t left, right;
};

cuex_t cuexP_assoc_empty;
cudyn_stdtype_t cuexP_assoc_type;

/* Type-cast function parameters. */
#define ASSOC ((cuex_assoc_t)(assoc))
#define ASSOC0 ((cuex_assoc_t)(assoc0))
#define ASSOC1 ((cuex_assoc_t)(assoc1))

#define PAIR_CENTER(left, right) \
    pair_centre((cu_word_t)(left), (cu_word_t)(right))
#define ASSOC_META cudyn_stdtype_to_meta(cuexP_assoc_type)

/* Anything goes, since singletons are represented as-is. */
#define cuex_is_assoc(ex) cu_true

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

static cuex_assoc_t
node_new(cu_word_t centre, cuex_t left, cuex_t right)
{
    struct cuex_assoc_s tpl;
    tpl.centre = centre;
    tpl.left = left;
    tpl.right = right;
    return cudyn_halloc_by_value(cudyn_stdtype_to_type(cuexP_assoc_type),
				 sizeof(struct cuex_assoc_s), &tpl);
}

cuex_t
cuex_assoc_find(cu_clop(get_key, cu_word_t, cuex_t), cuex_t assoc, cu_word_t key)
{
    cuex_meta_t assoc_meta;
    if (!cuex_is_assoc(assoc))
	cu_bugf("First argument to cuex_assoc_find must be an association.");
tail_call:
    assoc_meta = cuex_meta(assoc);
    if (assoc_meta != ASSOC_META) {
	if (cu_call(get_key, assoc) == key)
	    return assoc;
	else
	    return NULL;
    } else {
	cu_word_t centre = ASSOC->centre;
	if ((cu_word_t)key < centre) {
	    if ((cu_word_t)key < centre_to_min(centre))
		return NULL;
	    else {
		assoc = ASSOC->left;
		goto tail_call;
	    }
	} else {
	    if (key > centre_to_max(centre))
		return NULL;
	    else {
		assoc = ASSOC->right;
		goto tail_call;
	    }
	}
    }
}

static cuex_t
assoc_insert(cu_clop(get_key, cu_word_t, cuex_t), cuex_t assoc, cuex_t value)
{
    cu_word_t value_key = cu_call(get_key, value);
    cuex_meta_t assoc_meta;
    assoc_meta = cuex_meta(assoc);
    if (assoc_meta != ASSOC_META) {
	cu_word_t assoc_key;
	assoc_key = cu_call(get_key, assoc);
	if (value_key == assoc_key)
	    return assoc;
	else if (value_key < assoc_key)
	    return node_new(PAIR_CENTER(value_key, assoc_key), value, assoc);
	else
	    return node_new(PAIR_CENTER(assoc_key, value_key), assoc, value);
    } else {
	cu_word_t centre = ASSOC->centre;
	if ((cu_word_t)value_key < centre) {
	    if ((cu_word_t)value_key < centre_to_min(centre))
		return node_new(PAIR_CENTER(value_key, centre), value, assoc);
	    else
		return node_new(centre,
				assoc_insert(get_key, ASSOC->left, value),
				ASSOC->right);
	} else {
	    if ((cu_word_t)value_key > centre_to_max(centre))
		return node_new(PAIR_CENTER(centre, value_key), assoc, value);
	    else
		return node_new(centre,
				ASSOC->left,
				assoc_insert(get_key, ASSOC->right, value));
	}
    }
}

cuex_t
cuex_assoc_insert(cu_clop(get_key, cu_word_t, cuex_t),
		  cuex_t assoc, cuex_t value)
{
    if (!cuex_is_assoc(assoc))
	cu_bugf("First argument to cuex_assoc_insert must be an association.");
    if (assoc == cuexP_assoc_empty)
	return value;
    return assoc_insert(get_key, assoc, value);
}

static cuex_t
assoc_erase(cu_clop(get_key, cu_word_t, cuex_t), cuex_t assoc, cu_word_t key)
{
    cuex_meta_t assoc_meta;
    assoc_meta = cuex_meta(assoc);
    if (assoc_meta != ASSOC_META) {
	cu_word_t assoc_key;
	assoc_key = cu_call(get_key, assoc);
	if (key == assoc_key)
	    return NULL;
	else
	    return assoc;
    } else {
	cu_word_t centre = ASSOC->centre;
	if ((cu_word_t)key < centre) {
	    if ((cu_word_t)key < centre_to_min(centre))
		return assoc;
	    else {
		cuex_t new_left = assoc_erase(get_key, ASSOC->left, key);
		if (new_left == NULL)
		    return ASSOC->right;
		else
		    return node_new(centre, new_left, ASSOC->right);
	    }
	} else {
	    if ((cu_word_t)key > centre_to_max(centre))
		return assoc;
	    else {
		cuex_t new_right = assoc_erase(get_key, ASSOC->right, key);
		if (new_right == NULL)
		    return ASSOC->left;
		else
		    return node_new(centre, ASSOC->left, new_right);
	    }
	}
    }
}

cuex_t
cuex_assoc_erase(cu_clop(get_key, cu_word_t, cuex_t), cuex_t assoc, cu_word_t key)
{
    cuex_t res;
    if (!cuex_is_assoc(assoc))
	cu_bugf("First argument of cuex_assoc_erase must be an association.");
    if (assoc == cuexP_assoc_empty)
	return assoc;
    res = assoc_erase(get_key, assoc, key);
    if (res == NULL)
	return cuexP_assoc_empty;
    else
	return res;
}

static cuex_t
assoc_union(cu_clop(get_key, cu_word_t, cuex_t), cuex_t assoc0, cuex_t assoc1)
{
    cuex_meta_t meta0 = cuex_meta(assoc0);
    cuex_meta_t meta1 = cuex_meta(assoc1);
    if (meta0 != ASSOC_META)
	return assoc_insert(get_key, assoc1, assoc0);
    else if (meta1 != ASSOC_META)
	return assoc_insert(get_key, assoc0, assoc1);
    else {
	cu_word_t centre0 = ASSOC0->centre;
	cu_word_t centre1 = ASSOC1->centre;
	if (centre0 == centre1)
	    return node_new(centre0,
			    assoc_union(get_key, ASSOC0->left, ASSOC1->left),
			    assoc_union(get_key, ASSOC0->right, ASSOC1->right));
	else if (centre0 < centre1) {
	    cu_word_t max0 = centre_to_max(centre0);
	    if (max0 < centre1) {
		cu_word_t min1 = centre_to_min(centre1);
		if (max0 < min1)		/* [---] [---] */
		    return node_new(pair_centre(centre0, centre1),
				    assoc0, assoc1);
		else			/* [[---]---] */
		    return node_new(centre1,
				    assoc_union(get_key, assoc0, ASSOC1->left),
				    ASSOC1->right);
	    }
	    else				/* [---[---]] */
		return node_new(centre0,
				ASSOC0->left,
				assoc_union(get_key, ASSOC0->right, assoc1));
	}
	else {  /* As above but with assoc0 and assoc1 swapped. */
	    cu_word_t max1 = centre_to_max(centre1);
	    if (max1 < centre0) {
		cu_word_t min0 = centre_to_min(centre0);
		if (max1 < min0)
		    return node_new(pair_centre(centre1, centre0),
				    assoc1, assoc0);
		else
		    return node_new(centre0,
				    assoc_union(get_key, assoc1, ASSOC0->left),
				    ASSOC0->right);
	    }
	    else
		return node_new(centre1,
				ASSOC1->left,
				assoc_union(get_key, ASSOC1->right, assoc0));
	}
    }
}

cuex_t
cuex_assoc_union(cu_clop(get_key, cu_word_t, cuex_t),
		 cuex_t assoc0, cuex_t assoc1)
{
    if (cuex_assoc_is_empty(assoc0))
	return assoc1;
    if (cuex_assoc_is_empty(assoc1))
	return assoc0;
    return assoc_union(get_key, assoc0, assoc1);
}

static cuex_t
assoc_isecn(cu_clop(get_key, cu_word_t, cuex_t), cuex_t assoc0, cuex_t assoc1)
{
    cuex_meta_t meta0 = cuex_meta(assoc0);
    cuex_meta_t meta1 = cuex_meta(assoc1);
    if (meta0 != ASSOC_META)
	return cuex_assoc_find(get_key, assoc1, cu_call(get_key, assoc0));
    else if (meta1 != ASSOC_META)
	return cuex_assoc_find(get_key, assoc0, cu_call(get_key, assoc1));
    else {
	cu_word_t centre0 = ASSOC0->centre;
	cu_word_t centre1 = ASSOC1->centre;
	if (centre0 == centre1) {
	    cuex_t new_left;
	    cuex_t new_right;
	    new_left = assoc_isecn(get_key, ASSOC0->left, ASSOC1->left);
	    new_right = assoc_isecn(get_key, ASSOC0->right, ASSOC1->right);
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
		else			/* [[---]---] */
		    return assoc_isecn(get_key, assoc0, ASSOC1->left);
	    }
	    else				/* [---[---]] */
		return assoc_isecn(get_key, ASSOC0->right, assoc1);
	}
	else {  /* As above but with assoc0 and assoc1 swapped. */
	    cu_word_t max1 = centre_to_max(centre1);
	    if (max1 < centre0) {
		cu_word_t min0 = centre_to_min(centre0);
		if (max1 < min0)
		    return NULL;
		else
		    return assoc_isecn(get_key, assoc1, ASSOC0->left);
	    }
	    else
		return assoc_isecn(get_key, ASSOC1->right, assoc0);
	}
    }
}

cuex_t
cuex_assoc_isecn(cu_clop(get_key, cu_word_t, cuex_t),
		 cuex_t assoc0, cuex_t assoc1)
{
    cuex_t e;
    if (cuex_assoc_is_empty(assoc0))
	return assoc0;
    if (cuex_assoc_is_empty(assoc1))
	return assoc1;
    e = assoc_isecn(get_key, assoc0, assoc1);
    if (!e)
	e = cuex_assoc_empty();
    return e;
}

static void
assoc_iter(cuex_t assoc, cu_clop(fn, void, cuex_t))
{
    cuex_meta_t assoc_meta;
tail_call:
    assoc_meta = cuex_meta(assoc);
    if (assoc_meta == ASSOC_META) {
	assoc_iter(ASSOC->left, fn);
	assoc = ASSOC->right;
	goto tail_call;
    } else
	cu_call(fn, assoc);
}

void
cuex_assoc_iter(cuex_t assoc, cu_clop(fn, void, cuex_t))
{
    if (!cuex_is_assoc(assoc))
	cu_bugf("First argument to cuex_assoc_iter must be an association.");
    if (assoc == cuexP_assoc_empty)
	return;
    assoc_iter(assoc, fn);
}

static cu_bool_t
assoc_conj(cuex_t assoc, cu_clop(fn, cu_bool_t, cuex_t))
{
    cuex_meta_t assoc_meta;
tail_call:
    assoc_meta = cuex_meta(assoc);
    if (assoc_meta == ASSOC_META) {
	if (!assoc_conj(ASSOC->left, fn))
	    return cu_false;
	assoc = ASSOC->right;
	goto tail_call;
    } else
	return cu_call(fn, assoc);
}

cu_bool_t
cuex_assoc_conj(cuex_t assoc, cu_clop(fn, cu_bool_t, cuex_t))
{
    if (!cuex_is_assoc(assoc))
	cu_bugf("First argument to cuex_assoc_conj must be an association.");
    if (assoc == cuexP_assoc_empty)
	return cu_true;
    return assoc_conj(assoc, fn);
}

static void
assoc_image(cuex_t assoc, cu_clop(fn, cuex_t, cuex_t),
	    cu_clop(get_key, cu_word_t, cuex_t), cuex_t *accu)
{
    cuex_meta_t assoc_meta;
tail_call:
    assoc_meta = cuex_meta(assoc);
    if (assoc_meta == ASSOC_META) {
	assoc_image(ASSOC->left, fn, get_key, accu);
	assoc = ASSOC->right;
	goto tail_call;
    } else {
	cuex_t e = cu_call(fn, assoc);
	if (!*accu)
	    *accu = e;
	else
	    *accu = assoc_insert(get_key, *accu, e);
    }
}

cuex_t
cuex_assoc_image(cuex_t assoc, cu_clop(fn, cuex_t, cuex_t),
		 cu_clop(get_key, cu_word_t, cuex_t))
{
    cuex_t accu;
    if (cuex_assoc_is_empty(assoc))
	return assoc;
    accu = NULL;
    assoc_image(assoc, fn, get_key, &accu);
    return accu;
}

static cuex_t
assoc_isokey_image(cuex_t assoc, cu_clop(fn, cuex_t, cuex_t))
{
    cuex_meta_t assoc_meta = cuex_meta(assoc);
    if (assoc_meta == ASSOC_META)
	return node_new(ASSOC->centre,
			assoc_isokey_image(ASSOC->left, fn),
			assoc_isokey_image(ASSOC->right, fn));
    else
	return cu_call(fn, assoc);
}

cuex_t
cuex_assoc_isokey_image(cuex_t assoc, cu_clop(fn, cuex_t, cuex_t))
{
    if (assoc == cuexP_assoc_empty)
	return assoc;
    else
	return assoc_isokey_image(assoc, fn);
}

cu_clos_def(assoc_print_elt, cu_prot(void, cuex_t elt),
    (FILE *out; int index;))
{
    cu_clos_self(assoc_print_elt);
    if (self->index++ != 0)
	fputs(", ", self->out);
    cu_fprintf(self->out, "%!", elt);
}

void
assoc_print(void *obj, FILE *out)
{
    assoc_print_elt_t cb;
    cb.out = out;
    cb.index = 0;
    fputc('{', out);
    cuex_assoc_iter(obj, assoc_print_elt_prep(&cb));
    fputc('}', out);
}

void
cuexP_assoc_init()
{
    struct cuex_assoc_s empty_tpl;

    cuexP_assoc_type = cudyn_stdtype_new_hcs(sizeof(struct cuex_assoc_s) -
					     CU_HCOBJ_SHIFT);

    empty_tpl.centre = 1;
    empty_tpl.left = NULL;
    empty_tpl.right = NULL;
    cuexP_assoc_empty =
	cudyn_halloc_by_value(cudyn_stdtype_to_type(cuexP_assoc_type),
			      sizeof(struct cuex_assoc_s), &empty_tpl);

    cudyn_prop_define_ptr(cudyn_raw_print_fn_prop(),
			  cuexP_assoc_type, assoc_print);
}
