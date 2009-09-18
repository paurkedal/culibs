/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2002--2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUCON_PO_H
#define CUCON_PO_H

#include <cu/clos.h>
#include <cucon/list.h>
#include <cucon/fwd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cucon_po_h cucon/po.h: Strict Partial Order
 * @{\ingroup cucon_misc_mod */

#define CUCON_PO_ELT_LINKS_PO 1

struct cucon_po_s
{
    struct cucon_poelt_s *bot;
    struct cucon_poelt_s *top;
};

struct cucon_poelt_s
{
    struct cucon_list_s isuccs;
    struct cucon_list_s ipreds;

    unsigned int level;	/* for topological traversal */

#ifdef CUCON_PO_ELT_LINKS_PO
    struct cucon_po_s *po;
#endif
};

/*!The result of comparing to partially ordered elements. */
typedef enum {
    cucon_pocmp_eq	/*!The elements are equal. */
  , cucon_pocmp_prec	/*!The LHS element precedes the RHS. */
  , cucon_pocmp_succ	/*!The LHS element succeeds the RHS. */
  , cucon_pocmp_unord	/*!The elements are unordered. */
} cucon_pocmp_t;

typedef cucon_listnode_t cucon_po_ipred_it_t;
typedef cucon_listnode_t cucon_po_isucc_it_t;

/*!Construct \a po as a partial order with only bottom and top elements. */
void		cucon_po_cct(cucon_po_t po);
#define		cucon_po_cct(po) cucon_po_cct_mem(po, 0, 0)

/*!Return a partial order with only bottom and top elements. */
cucon_po_t	cucon_po_new(void);
#define		cucon_po_new() cucon_po_new_mem(0, 0)

/*!Construct \a po as a partial order with only a bottom element with slot
 * size \a size_bot and a top element with slot size \a size_top. */
void		cucon_po_cct_mem(cucon_po_t po,
				 size_t size_bot, size_t size_top);
/*!Return a partial order with only a bottom element with slot size
 * \a size_bot and a top element with slot size \a size_top. */
cucon_po_t	cucon_po_new_mem(size_t size_bot, size_t size_top);

/*!Construct \a po as a partial order with only a bottom element with its
 * slot equal to \a bot and a top element with its slot equal to \a top. */
void		cucon_po_cct_ptr(cucon_po_t po, void *bot, void *top);

/*!Return a partial order with only a bottom element with its slot equal
 * to \a bot and a top element with its slot equal to \a top. */
cucon_po_t	cucon_po_new_ptr(void *bot, void *top);

/* Construct 'po' with pre-constructed detached (bottom, top)
 * elements ('bot', 'top').  XXX OBSCURE! */
void		cucon_po_cct_2elt(cucon_po_t po,
				  cucon_poelt_t bot, cucon_poelt_t top);

/*!Construct \a po, \a bot and \a top such that \a po is a partial order
 * with bottom element \a bot and top element \a top. */
void		cucon_po_cct_2elt_cct(cucon_po_t po,
				      cucon_poelt_t bot, cucon_poelt_t top);

/*!Return the bottom element of \a po. */
cucon_poelt_t	cucon_po_bot(cucon_po_t po);
#define		cucon_po_bot(po) ((po)->bot)

/*!Return the top element of \a po. */
cucon_poelt_t	cucon_po_top(cucon_po_t po);
#define		cucon_po_top(po) ((po)->top)

/*!Return the number of elements in the longest chain from
 * \c cucon_po_bot(po) to \c cucon_po_top(po). */
unsigned int	cucon_po_depth(cucon_po_t po);
#define		cucon_po_depth(po) (CU_MARG(cucon_po_t, po)->top->level + 1)

/*!Return an element with \a size bytes slot, which is newly constructed
 * and inserted into \a po, and constrained with respect to the bottom and
 * top elements only. */
cucon_poelt_t	cucon_po_insert_mem(cucon_po_t po, size_t size);

/*!Return an element which is newly constructed with slot equal to \a ptr,
 * inserted into \a po such that it is only constrained with respect to the
 * bottom and top elements. */
cucon_poelt_t	cucon_po_insert_ptr(cucon_po_t po, void *ptr);

/*!Construct \a elt, insert it into \a po such that it is only constrained
 * with respect to the bottom and top elements. */
void		cucon_po_insert_cct(cucon_po_t po, cucon_poelt_t elt);

/* Insert 'elt' (create it with 'cucon_poelt_new_alloc') into 'po' and
 * constrain it according to 'prec'.  XXXX Currently broken. */
void cucon_po_insert_constrain(cucon_po_t po, cucon_poelt_t elt,
			       cu_clop(prec, cu_bool_t, void *v0, void *v1));
/* Create a detacted element which can later be inserted into a partial
 * order with \ref cucon_po_insert_constrain. */
void		cucon_poelt_cct(cucon_poelt_t elt);
cucon_poelt_t	cucon_poelt_new_alloc(size_t size);

/*!Return a pointer to the slot of \a elt. */
CU_SINLINE void*cucon_poelt_get_mem(cucon_poelt_t elt)
{ return CU_ALIGNED_MARG_END(cucon_poelt_t, elt); }

/*!Assuming \a elt has a pointer slot, return the pointer. */
CU_SINLINE void*cucon_poelt_get_ptr(cucon_poelt_t elt)
{ return *(void **)CU_ALIGNED_MARG_END(cucon_poelt_t, elt); }

/*!Return the topological level of \a elt. */
CU_SINLINE
unsigned int	cucon_poelt_level(cucon_poelt_t elt) { return elt->level; }

/*!Return the element with slot at \a slot. */
cucon_poelt_t	cucon_poelt_of_data(void *slot);
#define		cucon_poelt_of_data(data) \
		CU_ALIGNED_PTR_FROM_END(cucon_poelt_t, data)

/* void		cucon_po_erase(cucon_poelt_t e); */

/*!If \a e1 ≼ \a e0, return false, else force the constraint \a e0 ≺ \a e1
 * and return true. */
cu_bool_t	cucon_po_constrain_prec(cucon_po_t po,
					cucon_poelt_t e0, cucon_poelt_t e1);

/*!True iff \a e0 ≺ \a e1. */
cu_bool_t	cucon_po_prec(cucon_poelt_t e0, cucon_poelt_t e1);

/*!True iff \a e0 ≼ \a e1. */
CU_SINLINE cu_bool_t
cucon_po_preceq(cucon_poelt_t e0, cucon_poelt_t e1)
{ return e0 == e1 || cucon_po_prec(e0, e1); }

/*!Call \a f\c (e) for each \c e such that \a below ≺ \c e ≺ \a above. */
void cucon_po_iter_open_range(cucon_poelt_t below, cucon_poelt_t above,
			      cu_clop(f, void, cucon_poelt_t));

/*!Call \a f\c (e) for each \c e such that \a min ≼ \c e ≺ \a above. */
void cucon_po_iter_left_range(cucon_poelt_t min, cucon_poelt_t above,
			      cu_clop(f, void, cucon_poelt_t));

/*!Call \a f\c (e) for each \c e such that \a below ≺ \c e ≼ \a max'. */
void cucon_po_iter_right_range(cucon_poelt_t below, cucon_poelt_t max,
			       cu_clop(f, void, cucon_poelt_t));

/*!Call \a f\c (e) for each \c e such that \a min ≼ \c e ≼ \a max. */
void cucon_po_iter_closed_range(cucon_poelt_t min, cucon_poelt_t max,
				cu_clop(f, void, cucon_poelt_t));

/*!Call \a f\c (x) for each immediate predecessor \c x of \a e. */
void cucon_po_iter_ipreds(cucon_poelt_t e, cu_clop(f, void, cucon_poelt_t));

/*!Call \a f\c (x) for each immediate successor \c x of \a e. */
void cucon_po_iter_isuccs(cucon_poelt_t e, cu_clop(f, void, cucon_poelt_t));

/*!Call \a f\c (x) for \c x = \a e and each successor of \a e such
 * when \a f\c (x) is called before \a f\c (y) iff \c x ≺ \c y. */
void cucon_po_topological_succs(cucon_poelt_t e,
				cu_clop(f, void, cucon_poelt_t));
void cucon_po_topological_preds(cucon_poelt_t e,
				cu_clop(f, void, cucon_poelt_t));
/* void cucon_po_breath_first_succs(cucon_poelt_t e, */
/* 			       cu_clop(f, void, cucon_poelt_t)); */

/*!True iff \a e has a single predecessor. */
CU_SINLINE cu_bool_t
cucon_poelt_has_single_ipred(cucon_poelt_t e)
{ return cucon_list_is_singleton(&e->ipreds); }

/*!True iff \a e has a single successor. */
CU_SINLINE cu_bool_t
cucon_poelt_has_single_isucc(cucon_poelt_t e)
{ return cucon_list_is_singleton(&e->isuccs); }

/*!Given search limits \a bot and \a top, and the predicate \a cmp which
 * classifies elements by \c cucon_pocmp_eq, \c cucon_pocmp_prec, \c
 * cucon_pocmp_succ, and \c cucon_pocmp_unord, in a way consistent with the
 * ordering as if describing a virtual subrange of [\a bot, \a top], this
 * function inserts the range into \a range, the immediate predecessors
 * into \a ipreds and the immediate successors into \a isuccs.
 * By consistent with the ordering it is meant that if
 * \c e is classified as \c cucon_pocmp_eq, then all its successors must
 * be classified as either \c cucon_pocmp_eq or \c cucon_pocmp_succ, etc.
 * Note that need not return \c cucon_pocmp_eq on any existing element,
 * but it must return \c cucon_pocmp_succ or \c cucon_pocmp_eq for \a
 * bot and \c cocon_pocmp_prec or \c cucon_pocmp_eq for \a top. */
void
cucon_po_range_and_bounds_of_fn(cucon_poelt_t bot, cucon_poelt_t top,
				cu_clop(cmp, cucon_pocmp_t, cucon_poelt_t),
				cucon_pmap_t range,
				cucon_pmap_t ipreds, cucon_pmap_t isuccs);

/*!Return an iterator to the first element of the range of predecessors
 * of \a e. */
CU_SINLINE cucon_po_ipred_it_t
cucon_po_ipred_begin(cucon_poelt_t e)
{ return cucon_list_begin(&e->ipreds); }

/*!Return an iterator past the last element of the range of predecessors
 * if \a e. */
CU_SINLINE cucon_po_ipred_it_t
cucon_po_ipred_end(cucon_poelt_t e)
{ return cucon_list_end(&e->ipreds); }

/*!Return the iterator next after \a it in a range of predecessors. */
CU_SINLINE cucon_po_ipred_it_t
cucon_po_ipred_it_next(cucon_po_ipred_it_t it)
{ return cucon_listnode_next(it); }

/*!Return the element referred by \a it in a range of predecessors. */
CU_SINLINE cucon_poelt_t
cucon_po_ipred_it_get(cucon_po_ipred_it_t it)
{ return (cucon_poelt_t)cucon_listnode_ptr(it); }

/*!Return an iterator to the first element of the range of successors
 * of \a e. */
CU_SINLINE cucon_po_isucc_it_t
cucon_po_isucc_begin(cucon_poelt_t e)
{ return cucon_list_begin(&e->isuccs); }

/*!Return an iterator past the last element of the range of successors
 * of \a e. */
CU_SINLINE cucon_po_isucc_it_t
cucon_po_isucc_end(cucon_poelt_t e)
{ return cucon_list_end(&e->isuccs); }

/*!Return the iterator next after \a it in a range of successors. */
CU_SINLINE cucon_po_isucc_it_t
cucon_po_isucc_it_next(cucon_po_isucc_it_t it)
{ return cucon_listnode_next(it); }

/*!Return the element referred by \a it in a range of successors. */
CU_SINLINE cucon_poelt_t
cucon_po_isucc_it_get(cucon_po_isucc_it_t it)
{ return (cucon_poelt_t)cucon_listnode_ptr(it); }

/* Return the smallest set which preserves the lower span of 'S'. */
cucon_pmap_t cucon_po_pruned_lspanning(cucon_po_t po, cucon_pmap_t S);

/*!Sequentially conjunct \a cb over the lower span of \a S given the
 * ordering \a po. */
cu_bool_t cucon_po_conj_lspan(cucon_po_t po, cucon_pmap_t S,
			      cu_clop(cb, cu_bool_t, cucon_poelt_t elt));

/*!Accumulate the lower span of \a max into \a accum.
 * \pre \a accum is downwards closed (fulfilled by the empty set).
 * \post \a accum is downwards closed. */
void cucon_po_lspan_accum(cucon_poelt_t max, cucon_pmap_t accum);

/*!Accumulate the upper span of \a min into \a accum.
 * \pre \a accum is upwards closed (fulfilled by the empty set).
 * \post \a accum is upwards closed. */
void cucon_po_uspan_accum(cucon_poelt_t min, cucon_pmap_t accum);

/*!Return the set of predecessors of \a max. */
cucon_pmap_t cucon_po_lspan(cucon_poelt_t max);

/*!Return the set of successors of \a min. */
cucon_pmap_t cucon_po_uspan(cucon_poelt_t min);

/*!Return the set of \c x ∈ \a S such that \a x ≼ \a max. */
cucon_pmap_t cucon_po_lspan_isecn(cucon_poelt_t max, cucon_pmap_t S);

/*!Return the set of \c x ∈ \a S such that \a min ≼ \a x. */
cucon_pmap_t cucon_po_uspan_isecn(cucon_poelt_t min, cucon_pmap_t S);

/*!Accumulate range (\a below, \a above) into \a S and return true iff
 * \a below ≺ \a above. */
cu_bool_t cucon_po_open_range_accum(cucon_poelt_t below, cucon_poelt_t above,
				    cucon_pmap_t S);

/*!Return the range (\a below, \a above). */
cucon_pmap_t cucon_po_open_range(cucon_poelt_t min, cucon_poelt_t max);

cu_bool_t
cucon_po_closed_range_and_succs(cucon_poelt_t min, cucon_poelt_t max,
				cucon_pmap_t range, cucon_pmap_t succs);

/*!Remove from \a S the successors of \a start which are not maxima
 * of \a S. */
void cucon_po_connected_prune_to_max(cucon_pmap_t S, cucon_poelt_t start);

/*!Remove from \a S the predecessors of \a start which are not minima
 * of \a S. */
void cucon_po_connected_prune_to_min(cucon_pmap_t S, cucon_poelt_t start);

/*!Return the set of infimums of the elements in \a L.
 * \arg L is a non-empty list of elements of the same partial order. */
cucon_pmap_t cucon_po_inf_of_list(cucon_list_t L);

/*!Return the set of supremums of the elements in \a L.
 * \arg L is a non-empty list of elements of the same partial order. */
cucon_pmap_t cucon_po_sup_of_list(cucon_list_t L);

cucon_pmap_t cucon_po_ucollect_reachable_ljunctions(cucon_poelt_t start);

/*!Print \a po to \a out in graphviz format, with labels according to
 * \a label. */
void cucon_po_print_gviz(cucon_po_t po,
			 cu_clop(label, cu_str_t, cucon_poelt_t),
			 FILE *out);

/* Debugging
 * --------- */

size_t cucon_po_debug_count_connections(cucon_po_t po);
void cucon_po_debug_check_nonredundant(cucon_poelt_t elt);
void cucon_po_debug_dump_gviz(cucon_po_t po, FILE *out);

/*!@}*/
CU_END_DECLARATIONS

#endif

/*
 * Local Variables:
 * coding: utf-8
 * End:
 */
