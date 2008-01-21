/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2004--2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUCON_FWD_H
#define CUCON_FWD_H

#include <cu/fwd.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cucon_fwd_h cucon/fwd.h: Forward Declarations
 * @{\ingroup cucon_mod */

typedef struct cucon_arr_s		*cucon_arr_t;		/* arr.h */
typedef struct cucon_digraph_s		*cucon_digraph_t;	/* digraph.h */
typedef struct cucon_digraph_vertex_s	*cucon_digraph_vertex_t;/* digraph.h */
typedef struct cucon_digraph_edge_s	*cucon_digraph_edge_t;	/* digraph.h */
typedef struct cucon_hmap_s		*cucon_hmap_t;		/* hmap.h */
typedef struct cucon_hzmap_s		*cucon_hzmap_t;		/* hzmap.h */
typedef struct cucon_hzmap_itr_s	*cucon_hzmap_itr_t;	/* hzmap.h */
typedef struct cucon_hzset_s		*cucon_hzset_t;		/* hzset.h */
typedef struct cucon_hzset_itr_s	*cucon_hzset_itr_t;	/* hzset.h */
typedef struct cucon_hset_s		*cucon_hset_t;		/* hset.h */
typedef struct cucon_list_s		*cucon_list_t;		/* list.h */
typedef struct cucon_listnode_s		*cucon_listnode_t;	/* list.h */
typedef struct cucon_pcmap_s		*cucon_pcmap_t;		/* pcmap.h */
typedef struct cucon_pmap_s		*cucon_pmap_t;		/* pmap.h */
typedef struct cucon_parr_s		*cucon_parr_t;		/* parr.h */
typedef struct cucon_poelt_s		*cucon_poelt_t;		/* po.h */
typedef struct cucon_po_s		*cucon_po_t;		/* po.h */
typedef struct cucon_pritree_s		*cucon_pritree_t;	/* pritree.h */
typedef struct cucon_prinode_s		*cucon_prinode_t;	/* pritree.h */
typedef struct cucon_pset_s		*cucon_pset_t;		/* pset.h */
typedef struct cucon_rbmap_s		*cucon_rbmap_t;		/* rbmap.h */
typedef struct cucon_rbset_s		*cucon_rbset_t;		/* rbset.h */
typedef struct cucon_rbnode_s		*cucon_rbnode_t;	/* rbtree.h */
typedef struct cucon_rbtree_s		*cucon_rbtree_t;	/* rbtree.h */
typedef struct cucon_rpmap_s		*cucon_rpmap_t;		/* rpmap.h */
typedef struct cucon_rumap_s		*cucon_rumap_t;		/* rumap.h */
typedef struct cucon_slink_s		*cucon_slink_t;		/* slink.h */
typedef struct cucon_stack_s		*cucon_stack_t;		/* stack.h */
typedef struct cucon_umap_s		*cucon_umap_t;		/* umap.h */
typedef struct cucon_uset_s		*cucon_uset_t;		/* uset.h */
typedef struct cucon_ucmap_s		*cucon_ucmap_t;		/* ucmap.h */
typedef struct cucon_ucset_s		*cucon_ucset_t;		/* ucset.h */
typedef struct cucon_wmap_s		*cucon_wmap_t;		/* wmap.h */

void cucon_init(void);

/*!\deprecated Use cucon_listnode_t. */
#define cucon_list_it_t cucon_listnode_t

/*!@}*/
CU_END_DECLARATIONS

#endif
