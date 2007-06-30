/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2005--2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CU_HCONS_WP_H
#define CU_HCONS_WP_H

#include <cu/fwd.h>
#include <cu/thread.h>
#include <cu/memory.h>

CU_BEGIN_DECLARATIONS

typedef struct cu_hcset_s *cu_hcset_t;
typedef struct cu_hcnode_s *cu_hcnode_t;
struct cu_hcnode_s
{
    cu_hidden_ptr_t ptr;
    void *next;
};

struct cu_hcset_s
{
    cu_mutex_t mutex;
    size_t mask;
    size_t insert_cnt;
    void **arr;
};
extern struct cu_hcset_s cuP_hcset;

CU_SINLINE void **
cu_hcset_link(cu_hcset_t hcset, cu_hash_t hash)
{
    return &hcset->arr[hcset->mask & hash];
}

CU_SINLINE void
cu_hcset_lock(cu_hcset_t hcset)
{
    cu_mutex_lock(&hcset->mutex);
}

CU_SINLINE void
cu_hcset_unlock(cu_hcset_t hcset)
{
    cu_mutex_unlock(&hcset->mutex);
}

void cu_hcset_adjust(cu_hcset_t hcset);

#define CU_HC_ENTER(obj_t, obj, meta, size, hash, EQ)			\
do {									\
    void **cuL_link;							\
    void **cuL_free_link = NULL;					\
    cu_hcnode_t cuL_node;						\
    cu_hcset_lock(&cuP_hcset);						\
    cuL_link = cu_hcset_link(&cuP_hcset, hash);				\
    while (((uintptr_t)(cuL_node = *cuL_link) & 1) == 0 && cuL_node) {	\
	obj = cu_weakptr_get(&cuL_node->ptr);				\
	if (obj) {							\
	    if (cuex_meta(obj) == meta && (EQ)) {			\
		cu_hcset_unlock(&cuP_hcset);				\
		return obj;						\
	    }								\
	}								\
	else								\
	    cuL_free_link = (void **)&cuL_node->ptr;			\
	cuL_link = &cuL_node->next;					\
    }									\
    if (cuL_node) {							\
	obj = cu_weakptr_get((cu_hidden_ptr_t *)cuL_link);		\
	if (obj) {							\
	    if (cuex_meta(obj) == meta && (EQ)) {			\
		cu_hcset_unlock(&cuP_hcset);				\
		return obj;						\
	    }								\
	    else if (cuL_free_link)					\
		cuL_link = cuL_free_link;				\
	    else {							\
		cu_weakptr_clear_even((cu_hidden_ptr_t *)cuL_link);	\
		cuL_node = cu_galloc(sizeof(struct cu_hcnode_s));	\
		cu_weakptr_cct(&cuL_node->ptr, obj);			\
		*cuL_link = cuL_node;					\
		cuL_link = &cuL_node->next;				\
	    }								\
	}								\
    }									\
    obj = cuex_oalloc(meta, size);					\
    cu_weakptr_cct((cu_hidden_ptr_t*)cuL_link, obj);			\
    ++cuP_hcset.insert_cnt;						\
} while (0)

#define CU_HC_RETURN(obj)						\
do {									\
    cu_hcset_unlock(&cuP_hcset);					\
    if (cuP_hcset.insert_cnt > cuP_hcset.mask)				\
	cu_hcset_adjust(&cuP_hcset);					\
    return obj;								\
} while (0)

#define CU_HC(obj_t, obj, meta, size, rep_hash, EQ, CCT)		\
{									\
    obj_t obj;								\
    CU_HC_ENTER(obj_t, obj, meta, size, rep_hash, EQ);			\
    CCT;								\
    CU_HC_RETURN(obj);							\
}

CU_END_DECLARATIONS

#endif
