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

typedef struct cucon_arr		*cucon_arr_t;		/* arr.h */
typedef struct cucon_bitvect		*cucon_bitvect_t;	/* bitvect.h */
typedef struct cucon_digraph		*cucon_digraph_t;	/* digraph.h */
typedef struct cucon_digraph_vertex	*cucon_digraph_vertex_t;/* digraph.h */
typedef struct cucon_digraph_edge	*cucon_digraph_edge_t;	/* digraph.h */
typedef struct cucon_fibheap		*cucon_fibheap_t;	/* fibheap.h */
typedef struct cucon_fibnode		*cucon_fibnode_t;	/* fibheap.h */
typedef struct cucon_fibq		*cucon_fibq_t;		/* fibq.h */
typedef struct cucon_fibqnode		*cucon_fibqnode_t;	/* fibq.h */
typedef struct cucon_hmap		*cucon_hmap_t;		/* hmap.h */
typedef struct cucon_hzmap		*cucon_hzmap_t;		/* hzmap.h */
typedef struct cucon_hzmap_itr		*cucon_hzmap_itr_t;	/* hzmap.h */
typedef struct cucon_hzmap_node		*cucon_hzmap_node_t;	/* hzmap.h */
typedef struct cucon_hzset		*cucon_hzset_t;		/* hzset.h */
typedef struct cucon_hzset_itr		*cucon_hzset_itr_t;	/* hzset.h */
typedef struct cucon_hzset_node		*cucon_hzset_node_t;	/* hzset.h */
typedef struct cucon_hset		*cucon_hset_t;		/* hset.h */
typedef struct cucon_list		*cucon_list_t;		/* list.h */
typedef struct cucon_listnode		*cucon_listnode_t;	/* list.h */
typedef struct cucon_pcmap		*cucon_pcmap_t;		/* pcmap.h */
typedef struct cucon_pmap		*cucon_pmap_t;		/* pmap.h */
typedef struct cucon_parr		*cucon_parr_t;		/* parr.h */
typedef struct cucon_poelt		*cucon_poelt_t;		/* po.h */
typedef struct cucon_po			*cucon_po_t;		/* po.h */
typedef struct cucon_priq		*cucon_priq_t;		/* priq.h */
typedef struct cucon_pritree		*cucon_pritree_t;	/* pritree.h */
typedef struct cucon_prinode		*cucon_prinode_t;	/* pritree.h */
typedef struct cucon_pset		*cucon_pset_t;		/* pset.h */
typedef struct cucon_rbmap		*cucon_rbmap_t;		/* rbmap.h */
typedef struct cucon_rbset		*cucon_rbset_t;		/* rbset.h */
typedef struct cucon_rbnode		*cucon_rbnode_t;	/* rbtree.h */
typedef struct cucon_rbtree		*cucon_rbtree_t;	/* rbtree.h */
typedef struct cucon_rpmap		*cucon_rpmap_t;		/* rpmap.h */
typedef struct cucon_rumap		*cucon_rumap_t;		/* rumap.h */
typedef struct cucon_slink		*cucon_slink_t;		/* slink.h */
typedef struct cucon_stack		*cucon_stack_t;		/* stack.h */
typedef struct cucon_stack_itr		*cucon_stack_itr_t;	/* stack.h */
typedef struct cucon_umap		*cucon_umap_t;		/* umap.h */
typedef struct cucon_uset		*cucon_uset_t;		/* uset.h */
typedef struct cucon_ucmap		*cucon_ucmap_t;		/* ucmap.h */
typedef struct cucon_ucset		*cucon_ucset_t;		/* ucset.h */
typedef struct cucon_ucset_itr		*cucon_ucset_itr_t;	/* ucset.h */
typedef struct cucon_wmap		*cucon_wmap_t;		/* wmap.h */

void cucon_init(void);

/*!\deprecated Use cucon_listnode_t. */
#define cucon_list_it_t cucon_listnode_t

/*!@}*/
CU_END_DECLARATIONS

#ifndef CU_NCOMPAT
#  include <cucon/compat.h>
#endif
#endif
