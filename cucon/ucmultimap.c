/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2010  Petter Urkedal <paurkedal@eideticdew.org>
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

#include <cucon/ucmultimap.h>
#include <cucon/ucset.h>
#include <cucon/ucmap_priv.h>
#include <cu/memory.h>
#include <inttypes.h>

#define UCMAP(M) ((cucon_ucmap_t)(M))

int
cucon_ucmultimap_element_cmp(void const *e0_, void const *e1_)
{
    struct cucon_ucmultimap_element *e0, *e1;
    e0 = (struct cucon_ucmultimap_element *)e0_;
    e1 = (struct cucon_ucmultimap_element *)e1_;
    if (e0->key < e1->key)     return -1;
    if (e0->key > e1->key)     return  1;
    if (e0->value < e1->value) return -1;
    if (e0->value > e1->value) return  1;
    return 0;
}

CU_SINLINE cucon_ucmultimap_t _ucmap_node_left(cucon_ucmultimap_t M)
{ return (cucon_ucmultimap_t)cucon_ucmap_node_left((cucon_ucmap_t)M); }

CU_SINLINE cucon_ucmultimap_t _ucmap_node_right(cucon_ucmultimap_t M)
{ return (cucon_ucmultimap_t)cucon_ucmap_node_left((cucon_ucmap_t)M); }

CU_SINLINE uintptr_t _ucmap_node_key(cucon_ucmultimap_t M)
{ return cucon_ucmap_node_key((cucon_ucmap_t)M); }

CU_SINLINE cu_bool_t _ucmap_node_has_value(cucon_ucmultimap_t M)
{ return cucon_ucmap_node_has_value((cucon_ucmap_t)M); }

CU_SINLINE cucon_ucset_t _ucmap_node_value(cucon_ucmultimap_t M)
{ return (cucon_ucset_t)cucon_ucmap_node_value((cucon_ucmap_t)M); }

cucon_ucmultimap_t
cucon_ucmultimap_isokey_merge(cucon_ucmultimap_t M,
			      uintptr_t k, cucon_ucset_t V)
{
    cucon_ucset_t Vi = cucon_ucmultimap_isokey_find(M, k);
    cucon_ucset_t Vn = cucon_ucset_union(Vi, V);
    if (Vn == Vi)
	return M;
    else
	return (cucon_ucmultimap_t)cucon_ucmap_insert_ptr(UCMAP(M), k, Vn);
}

cucon_ucmultimap_t
cucon_ucmultimap_isokey_unmerge(cucon_ucmultimap_t M,
				uintptr_t k, cucon_ucset_t V)
{
    cucon_ucset_t Vi = cucon_ucmultimap_isokey_find(M, k);
    cucon_ucset_t Vn = cucon_ucset_compl(Vi, V);
    if (Vn == Vi)
	return M;
    else if (Vn == NULL)
	return cucon_ucmultimap_isokey_erase(M, k);
    else
	return cucon_ucmultimap_isokey_insert(M, k, Vn);
}

cucon_ucmultimap_t
cucon_ucmultimap_singleton(uintptr_t k, uintptr_t v)
{
    return (cucon_ucmultimap_t)
	cucon_ucmap_singleton_ptr(k, cucon_ucset_singleton(v));
}

cu_bool_t
cucon_ucmultimap_is_singleton(cucon_ucmultimap_t M)
{
    if (!cucon_ucmap_is_singleton(UCMAP(M)))
	return cu_false;
    return cucon_ucset_is_singleton((cucon_ucset_t)(UCMAP(M)->value));
}

cucon_ucmultimap_t
cucon_ucmultimap_from_sorted_array(struct cucon_ucmultimap_element *arr,
				   size_t len)
{
    struct cucon_ucmap_element *isokey_arr;
    uintptr_t *sub_arr;
    uintptr_t key;
    size_t isokey_len, sub_len;

    isokey_arr = cu_snewarr(struct cucon_ucmap_element, len);
    sub_arr = cu_snewarr(uintptr_t, len);
    isokey_len = 0;
    while (len) {
	sub_len = 0;
	key = arr->key;
	do {
	    sub_arr[sub_len++] = arr->value;
	    ++arr; --len;
	} while (len && arr->key == key);
	isokey_arr[isokey_len].key = key;
	isokey_arr[isokey_len].value
	    = (uintptr_t)cucon_ucset_from_sorted_array(sub_arr, sub_len);
	++isokey_len;
    }
    return (cucon_ucmultimap_t)cucon_ucmap_from_sorted_array(isokey_arr,
							     isokey_len);
}

cucon_ucmultimap_t
cucon_ucmultimap_insert(cucon_ucmultimap_t M, uintptr_t k, uintptr_t v)
{
    cucon_ucset_t Vo = cucon_ucmap_find_ptr(UCMAP(M), k);
    cucon_ucset_t Vn = cucon_ucset_insert(Vo, v);
    if (Vn == Vo)
	return M;
    else
	return (cucon_ucmultimap_t)cucon_ucmap_insert_ptr(UCMAP(M), k, Vn);
}

cucon_ucmultimap_t
cucon_ucmultimap_erase(cucon_ucmultimap_t M, uintptr_t k, uintptr_t v)
{
    cucon_ucset_t Vo = cucon_ucmap_find_ptr(UCMAP(M), k);
    cucon_ucset_t Vn = cucon_ucset_erase(Vo, v);
    if (Vn == Vo)
	return M;
    else if (Vn == NULL)
	return (cucon_ucmultimap_t)cucon_ucmap_erase(UCMAP(M), k);
    else
	return (cucon_ucmultimap_t)cucon_ucmap_insert_ptr(UCMAP(M), k, Vn);
}

cu_bool_t
cucon_ucmultimap_contains(cucon_ucmultimap_t M, uintptr_t k, uintptr_t v)
{
    return cucon_ucset_find(cucon_ucmultimap_isokey_find(M, k), v);
}

cu_clos_def(_card_counter, cu_prot(cu_bool_t, uintptr_t k, cucon_ucset_t V),
	    ( size_t count; ))
{
    cu_clos_self(_card_counter);
    self->count += cucon_ucset_card(V);
    return cu_true;
}
size_t
cucon_ucmultimap_card(cucon_ucmultimap_t M)
{
    _card_counter_t counter;
    counter.count = 0;
    cucon_ucmultimap_isokey_iterA(_card_counter_prep(&counter), M);
    return counter.count;
}

cu_clos_def(_iterA_cb, cu_prot(cu_bool_t, uintptr_t v),
    ( cu_clop(f, cu_bool_t, uintptr_t, uintptr_t); uintptr_t k; ))
{
    cu_clos_self(_iterA_cb);
    return cu_call(self->f, self->k, v);
}

cu_bool_t
cucon_ucmultimap_iterA(cu_clop(f, cu_bool_t, uintptr_t, uintptr_t),
		       cucon_ucmultimap_t node)
{
    _iterA_cb_t cb;
    _iterA_cb_init(&cb);
    cb.f = f;
    while (node) {
	if (!cucon_ucmultimap_iterA(f, _ucmap_node_left(node)))
	    return cu_false;
	if (_ucmap_node_has_value(node)) {
	    cb.k = _ucmap_node_key(node);
	    if (!cucon_ucset_conj(_ucmap_node_value(node), _iterA_cb_ref(&cb)))
		return cu_false;
	}
	node = _ucmap_node_right(node);
    }
    return cu_true;
}

cu_clop_def(_ucset_eq, cu_bool_t, uintptr_t V0, uintptr_t V1)
{
    return cucon_ucset_eq((cucon_ucset_t)V0, (cucon_ucset_t)V1);
}

cu_bool_t
cucon_ucmultimap_eq(cucon_ucmultimap_t M0, cucon_ucmultimap_t M1)
{
#if CUCONP_UCMAP_ENABLE_HCONS && CUCON_UCSET_ENABLE_HCONS
    return M0 == M1;
#elif CUCON_UCSET_ENABLE_HCONS
    return cucon_ucmap_eq((cucon_ucmap_t)M0, (cucon_ucmap_t)M1);
#else
    return cucon_ucmap_eq_ptr(cu_clop_ref(_ucset_eq),
			      (cucon_ucmap_t)M0, (cucon_ucmap_t)M1);
#endif
}

cu_clop_def(_union_merge, uintptr_t, uintptr_t S0, uintptr_t S1)
{
    return (uintptr_t)cucon_ucset_union((cucon_ucset_t)S0, (cucon_ucset_t)S1);
}

cucon_ucmultimap_t
cucon_ucmultimap_union(cucon_ucmultimap_t M0, cucon_ucmultimap_t M1)
{
    return (cucon_ucmultimap_t)
	cucon_ucmap_combined_union(cu_clop_ref(_union_merge),
				   (cucon_ucmap_t)M0, (cucon_ucmap_t)M1);
}

cu_clos_def(_fprint_uintptr, cu_prot(cu_bool_t, uintptr_t k, cucon_ucset_t vs),
    ( int i; FILE *fh; ))
{
    cu_clos_self(_fprint_uintptr);
    if (self->i++) fputs(", ", self->fh);
    fprintf(self->fh, "%"PRIxPTR" ↦ ", k);
    cucon_ucset_fprint_uintptr(vs, self->fh);
    return cu_true;
}

void
cucon_ucmultimap_fprint_uintptr(cucon_ucmultimap_t M, FILE *fh)
{
    _fprint_uintptr_t cb;
    cb.i = 0;
    cb.fh = fh;
    fputc('{', fh);
    cucon_ucmultimap_isokey_iterA(_fprint_uintptr_prep(&cb), M);
    fputc('}', fh);
}
