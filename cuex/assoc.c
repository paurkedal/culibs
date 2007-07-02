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

cuex_t cuexP_assoc_empty;
cudyn_stdtype_t cuexP_assoc_type;

#define ASSOC ((cuex_assoc_t)(assoc))
#define PAIR_CENTER(left, right) \
    pair_center((cu_word_t)(left), (cu_word_t)(right))

CU_SINLINE cu_word_t
assoc_center(cuex_t assoc)
{
    cuex_meta_t meta = cuex_meta(assoc);
    if (cuex_meta_is_opr(meta))
	return cuex_opn_at(assoc, 0);
    else
	return ASSOC->center;
}

CU_SINLINE cu_word_t
pair_center(cu_word_t left, cu_word_t right)
{
    cu_word_t mask;
    mask = cu_word_dcover(left ^ right);
    return (left & ~mask) | ((mask + CU_WORD_C(1)) >> 1);
}

CU_SINLINE cu_word_t
center_to_min(cu_word_t center)
{
    cu_word_t mask = center ^ (center - 1);
    return center & ~mask;
}

CU_SINLINE cu_word_t
center_to_max(cu_word_t center)
{
    cu_word_t mask = center ^ (center - 1);
    return center | mask;
}

static cuex_assoc_t
node_new(cu_word_t center, cuex_t left, cuex_t right)
{
    struct cuex_assoc_s tpl;
    cu_debug_assert(assoc_center(left) < center);
    cu_debug_assert(center <= assoc_center(right));
    tpl.center = center;
    tpl.left = left;
    tpl.right = right;
    return cudyn_halloc_by_value(cudyn_stdtype_to_type(cuexP_assoc_type),
				 sizeof(struct cuex_assoc_s), &tpl);
}

cuex_t
cuex_assoc_find(cuex_t assoc, cuex_t key)
{
    cuex_meta_t assoc_meta;
    if (!cuex_is_assoc(assoc))
	cu_bugf("First argument to cuex_assoc_find must be an association.");
tail_call:
    assoc_meta = cuex_meta(assoc);
    if (cuex_meta_is_opr(assoc_meta)) {
	cu_debug_assert(cuex_opr_r(assoc_meta) >= 1);
	if (cuex_opn_at(assoc, 0) == key)
	    return assoc;
	else
	    return NULL;
    } else {
	cu_word_t center = ASSOC->center;
	cu_debug_assert(assoc_meta == cudyn_stdtype_to_meta(cuexP_assoc_type));
	if ((cu_word_t)key < center) {
	    if ((cu_word_t)key < center_to_min(center))
		return NULL;
	    else {
		assoc = ASSOC->left;
		goto tail_call;
	    }
	} else {
	    if ((cu_word_t)key > center_to_max(center))
		return NULL;
	    else {
		assoc = ASSOC->right;
		goto tail_call;
	    }
	}
    }
}

static cuex_t
assoc_insert(cuex_t assoc, cuex_t value)
{
    cuex_t value_key = cuex_opn_at(value, 0);
    cuex_meta_t assoc_meta;
    assoc_meta = cuex_meta(assoc);
    if (cuex_meta_is_opr(assoc_meta)) {
	cuex_t assoc_key;
	cu_debug_assert(cuex_opr_r(assoc_meta) >= 1);
	assoc_key = cuex_opn_at(assoc, 0);
	if (value_key == assoc_key)
	    return assoc;
	else if (value_key < assoc_key)
	    return node_new(PAIR_CENTER(value_key, assoc_key), value, assoc);
	else
	    return node_new(PAIR_CENTER(assoc_key, value_key), assoc, value);
    } else {
	cu_word_t center;
	cu_debug_assert(assoc_meta == cudyn_stdtype_to_meta(cuexP_assoc_type));
	center = ASSOC->center;
	if ((cu_word_t)value_key < center) {
	    if ((cu_word_t)value_key < center_to_min(center))
		return node_new(PAIR_CENTER(value_key, center), value, assoc);
	    else
		return node_new(center,
				assoc_insert(ASSOC->left, value),
				ASSOC->right);
	} else {
	    if ((cu_word_t)value_key > center_to_max(center))
		return node_new(PAIR_CENTER(center, value_key), assoc, value);
	    else
		return node_new(center,
				ASSOC->left,
				assoc_insert(ASSOC->right, value));
	}
    }
}

cuex_t
cuex_assoc_insert(cuex_t assoc, cuex_t value)
{
    cuex_meta_t value_meta = cuex_meta(value);
    if (!cuex_is_assoc(assoc))
	cu_bugf("First argument to cuex_assoc_insert must be an association.");
    if (!cuex_meta_is_opr(value_meta))
	cu_bugf("Second argument to cuex_assoc_insert must be an operation.");
    if (cuex_opr_r(value_meta) < 1)
	cu_bugf("Second argument to cuex_assoc_insert must have at "
		"least one operand.");
    if (assoc == cuexP_assoc_empty)
	return value;
    return assoc_insert(assoc, value);
}

static cuex_t
assoc_erase(cuex_t assoc, cuex_t key)
{
    cuex_meta_t assoc_meta;
    assoc_meta = cuex_meta(assoc);
    if (cuex_meta_is_opr(assoc_meta)) {
	cuex_t assoc_key;
	cu_debug_assert(cuex_opr_r(assoc_meta) >= 1);
	assoc_key = cuex_opn_at(assoc, 0);
	if (key == assoc_key)
	    return NULL;
	else
	    return assoc;
    } else {
	cu_word_t center;
	cu_debug_assert(assoc_meta == cudyn_stdtype_to_meta(cuexP_assoc_type));
	center = ASSOC->center;
	if ((cu_word_t)key < center) {
	    if ((cu_word_t)key < center_to_min(center))
		return assoc;
	    else {
		cuex_t new_left = assoc_erase(ASSOC->left, key);
		if (new_left == NULL)
		    return ASSOC->right;
		else
		    return node_new(center, new_left, ASSOC->right);
	    }
	} else {
	    if ((cu_word_t)key > center_to_max(center))
		return assoc;
	    else {
		cuex_t new_right = assoc_erase(ASSOC->right, key);
		if (new_right == NULL)
		    return ASSOC->left;
		else
		    return node_new(center, ASSOC->left, new_right);
	    }
	}
    }
}

cuex_t
cuex_assoc_erase(cuex_t assoc, cuex_t key)
{
    cuex_t res;
    if (!cuex_is_assoc(assoc))
	cu_bugf("First argument of cuex_assoc_erase must be an association.");
    if (assoc == cuexP_assoc_empty)
	return assoc;
    res = assoc_erase(assoc, key);
    if (res == NULL)
	return cuexP_assoc_empty;
    else
	return res;
}

static void
assoc_iter(cuex_t assoc, cu_clop(fn, void, cuex_t))
{
    cuex_meta_t assoc_meta;
tail_call:
    assoc_meta = cuex_meta(assoc);
    if (cuex_meta_is_opr(assoc_meta))
	cu_call(fn, assoc);
    else {
	cu_debug_assert(assoc_meta == cudyn_stdtype_to_meta(cuexP_assoc_type));
	assoc_iter(ASSOC->left, fn);
	assoc = ASSOC->right;
	goto tail_call;
    }
}

void
cuex_assoc_iter(cuex_t assoc, cu_clop(fn, void, cuex_t))
{
    if (!cuex_is_assoc(assoc))
	cu_bugf("First argument to cuex_assoc_iter must be an association.");
    if (assoc == cuexP_assoc_empty)
	return;
    return assoc_iter(assoc, fn);
}

cu_clos_def(assoc_print_elt, cu_prot(void, cuex_t elt),
    (FILE *out; int index;))
{
    cu_clos_self(assoc_print_elt);
    if (self->index++ != 0)
	fputs(", ", self->out);
    cu_fprintf(self->out, "%! ↦ %!", cuex_opn_at(elt, 0), elt);
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

    empty_tpl.center = 1;
    empty_tpl.left = NULL;
    empty_tpl.right = NULL;
    cuexP_assoc_empty =
	cudyn_halloc_by_value(cudyn_stdtype_to_type(cuexP_assoc_type),
			      sizeof(struct cuex_assoc_s), &empty_tpl);

    cudyn_prop_define_ptr(cudyn_raw_print_fn_prop(),
			  cuexP_assoc_type, assoc_print);
}
