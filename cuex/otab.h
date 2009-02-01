/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2006--2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUEX_OTAB_H
#define CUEX_OTAB_H

#include <cuex/fwd.h>
#include <cucon/pmap.h>
#include <cucon/rbset.h>
#include <cucon/list.h>
#include <cucon/arr.h>
#include <cucon/bitvect.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuex_otab_h cuex/otab.h: Operator Definition Support
 *@{\ingroup cuex_internal_mod */

typedef enum {
    cuex_otab_range_kind,
    cuex_otab_reservation_kind,
    cuex_otab_opr_kind,
    cuex_otab_prop_kind
} cuex_otab_objkind_t;

typedef struct cuex_otab_s *cuex_otab_t;
typedef struct cuex_otab_def_s *cuex_otab_def_t;
typedef struct cuex_otab_opr_s *cuex_otab_opr_t;
typedef struct cuex_otab_prop_s *cuex_otab_prop_t;
typedef struct cuex_otab_range_s *cuex_otab_range_t;
typedef struct cuex_otab_reservation_s *cuex_otab_reservation_t;
typedef struct cuex_otab_simplerange_s *cuex_otab_simplerange_t;

struct cuex_otab_s
{
    cu_str_t name;
    cu_bool_t is_extern;
    cuex_otab_range_t all;
    int all_width;
    struct cucon_pmap_s env;
    void (*error)(cuex_otab_t tab, cu_sref_t sref, char const *msg, ...);
    cu_str_t h_prologue, h_epilogue, c_prologue, c_epilogue;
};

cuex_otab_t cuex_otab_new(int width,
			  void (*error)(cuex_otab_t tab, cu_sref_t sref,
					char const *msg, ...));

void cuex_otab_init(cuex_otab_t tab, int width,
		    void (*error)(cuex_otab_t tab, cu_sref_t sref,
				  char const *msg, ...));

CU_SINLINE void cuex_otab_set_extern(cuex_otab_t tab, cu_bool_t is_extern)
{ tab->is_extern = is_extern; }

CU_SINLINE cu_bool_t cuex_otab_is_extern(cuex_otab_t tab)
{ return tab->is_extern; }

CU_SINLINE cuex_otab_def_t cuex_otab_lookup(cuex_otab_t tab, cu_idr_t idr)
{ return cucon_pmap_find_mem(&tab->env, idr); }

CU_SINLINE int cuex_otab_all_width(cuex_otab_t tab)
{ return tab->all_width; }

CU_SINLINE cuex_meta_t cuex_otab_all_mask(cuex_otab_t tab)
{ return (CUEX_META_C(1) << tab->all_width) - 1; }

CU_SINLINE cuex_meta_t cuex_otab_all_maxp(cuex_otab_t tab)
{ return CUEX_META_C(1) << tab->all_width; }

CU_SINLINE void cuex_otab_set_h_prologue(cuex_otab_t tab, cu_str_t str)
{ tab->h_prologue = str; }

CU_SINLINE void cuex_otab_set_h_epilogue(cuex_otab_t tab, cu_str_t str)
{ tab->h_epilogue = str; }

CU_SINLINE void cuex_otab_set_c_prologue(cuex_otab_t tab, cu_str_t str)
{ tab->c_prologue = str; }

CU_SINLINE void cuex_otab_set_c_epilogue(cuex_otab_t tab, cu_str_t str)
{ tab->c_epilogue = str; }

struct cuex_otab_def_s
{
    cu_bool_t is_extern;
    cu_idr_t idr; /* NULL for reservation */
    cu_sref_t sref;
    cuex_otab_objkind_t objkind;
};

CU_SINLINE cuex_otab_objkind_t cuex_otab_def_kind(cuex_otab_def_t def)
{ return def->objkind; }

CU_SINLINE cu_idr_t cuex_otab_def_idr(cuex_otab_def_t def)
{ return def->idr; }

CU_SINLINE cu_sref_t cuex_otab_def_sref(cuex_otab_def_t def)
{ return def->sref; }

struct cuex_otab_simplerange_s
{
    cu_inherit (cuex_otab_def_s);
    cuex_otab_range_t super;
    int range_low_bit;		/* of real range bits */
    cuex_meta_t range_min;	/* absolute */
    cuex_meta_t range_maxp1;	/* absolute */
    struct cucon_rbset_s subrange_set; /* of cuex_otab_simplerange_t */
};

struct cuex_otab_range_s
{
    cu_inherit (cuex_otab_simplerange_s);
    struct cucon_list_s prop_list;
    struct cucon_list_s opr_list;
};

CU_SINLINE cuex_otab_range_t cuex_otab_range_from_def(cuex_otab_def_t def)
{ return cu_from2(cuex_otab_range, cuex_otab_simplerange, cuex_otab_def,
		  def); }

CU_SINLINE cuex_otab_def_t cuex_otab_range_to_def(cuex_otab_range_t range)
{ return cu_to2(cuex_otab_def, cuex_otab_simplerange, range); }

CU_SINLINE cuex_otab_range_t cuex_otab_range_super(cuex_otab_range_t range)
{ return cu_to(cuex_otab_simplerange, range)->super; }

CU_SINLINE cu_idr_t cuex_otab_range_idr(cuex_otab_range_t range)
{ return cuex_otab_def_idr(cuex_otab_range_to_def(range)); }

CU_SINLINE cu_sref_t cuex_otab_range_sref(cuex_otab_range_t range)
{ return cuex_otab_def_sref(cuex_otab_range_to_def(range)); }

CU_SINLINE cuex_meta_t cuex_otab_range_low_bit(cuex_otab_range_t range)
{ return cu_to(cuex_otab_simplerange, range)->range_low_bit; }

CU_SINLINE cuex_meta_t cuex_otab_range_min(cuex_otab_range_t range)
{ return cu_to(cuex_otab_simplerange, range)->range_min; }

CU_SINLINE cuex_meta_t cuex_otab_range_maxp1(cuex_otab_range_t range)
{ return cu_to(cuex_otab_simplerange, range)->range_maxp1; }

CU_SINLINE cuex_meta_t cuex_otab_range_length(cuex_otab_range_t range)
{ return cuex_otab_range_maxp1(range) - cuex_otab_range_min(range); }

CU_SINLINE cuex_meta_t cuex_otab_range_inner_length(cuex_otab_range_t range)
{ return cuex_otab_range_length(range) >> cuex_otab_range_low_bit(range); }

struct cuex_otab_prop_s
{
    cu_inherit (cuex_otab_def_s);
    cu_bool_t is_implicit;
    int width;
    int low_bit;
    char const *type_cstr;
};

CU_SINLINE cu_idr_t cuex_otab_prop_idr(cuex_otab_prop_t prop)
{ return cu_to(cuex_otab_def, prop)->idr; }

CU_SINLINE cu_sref_t cuex_otab_prop_sref(cuex_otab_prop_t prop)
{ return cu_to(cuex_otab_def, prop)->sref; }

CU_SINLINE int cuex_otab_prop_width(cuex_otab_prop_t prop)
{ return prop->width; }

CU_SINLINE int cuex_otab_prop_low_bit(cuex_otab_prop_t prop)
{ return prop->low_bit; }


/*!Return a subrange from \a rel_min to \a rel_maxp1 - 1 of the inner range
 * of \a super. */
cuex_otab_range_t
cuex_otab_defrange(cuex_otab_t tab, cu_idr_t idr, cu_sref_t sref,
		   cuex_otab_range_t super,
		   cuex_meta_t rel_min, cuex_meta_t rel_maxp1);

cuex_otab_prop_t
cuex_otab_defprop(cuex_otab_t tab, cu_idr_t idr, cu_sref_t sref,
		  cuex_otab_range_t super, int width, char const *type_cstr,
		  cu_bool_t is_implicit);

struct cuex_otab_reservation_s
{
    cu_inherit (cuex_otab_simplerange_s);
    cu_bool_t is_full; /* always true if we don't own it */
    struct cucon_bitvect_s all_freemask;
    struct cucon_bitvect_s multi_freemask;
    struct cucon_arr_s freemask; /* of cucon_bitvect_s */
};

CU_SINLINE cuex_otab_reservation_t
cuex_otab_reservation_from_def(cuex_otab_def_t def)
{ return cu_from2(cuex_otab_reservation, cuex_otab_simplerange, cuex_otab_def,
		  def); }

CU_SINLINE cuex_otab_range_t
cuex_otab_reservation_super(cuex_otab_reservation_t rsv)
{ return cu_to(cuex_otab_simplerange, rsv)->super; }

CU_SINLINE int
cuex_otab_reservation_low_bit(cuex_otab_reservation_t rsv)
{ return cu_to(cuex_otab_simplerange, rsv)->range_low_bit; }

CU_SINLINE cuex_meta_t
cuex_otab_reservation_min(cuex_otab_reservation_t rsv)
{ return cu_to(cuex_otab_simplerange, rsv)->range_min; }

CU_SINLINE cuex_meta_t
cuex_otab_reservation_maxp1(cuex_otab_reservation_t rsv)
{ return cu_to(cuex_otab_simplerange, rsv)->range_maxp1; }

CU_SINLINE cuex_meta_t
cuex_otab_reservation_length(cuex_otab_reservation_t rsv)
{ return cuex_otab_reservation_maxp1(rsv) - cuex_otab_reservation_min(rsv); }

struct cuex_otab_opr_s
{
    cu_inherit (cuex_otab_def_s);
    cu_bool_t has_ctor;
    cuex_meta_t index;
    int r;
};

CU_SINLINE cuex_otab_opr_t
cuex_otab_opr_from_def(cuex_otab_def_t def)
{ return cu_from(cuex_otab_opr, cuex_otab_def, def); }

CU_SINLINE cuex_otab_def_t
cuex_otab_opr_to_def(cuex_otab_opr_t opr)
{ return cu_to(cuex_otab_def, opr); }

CU_SINLINE cu_idr_t
cuex_otab_opr_idr(cuex_otab_opr_t opr)
{ return cuex_otab_def_idr(cu_to(cuex_otab_def, opr)); }

CU_SINLINE cu_sref_t
cuex_otab_opr_sref(cuex_otab_opr_t opr)
{ return cuex_otab_def_sref(cu_to(cuex_otab_def, opr)); }

CU_SINLINE void
cuex_otab_opr_give_ctor(cuex_otab_opr_t opr)
{ opr->has_ctor = cu_true; }

/*!Reserve \a rel_min to \a rel_maxp1 - 1 of \a super, where \a rel_min and \a
 * rel_maxp1 are measured relative to the inner range of \a super. */
cu_bool_t
cuex_otab_reserve(cuex_otab_t tab, cu_sref_t sref, cuex_otab_range_t super,
		  cuex_meta_t rel_min, cuex_meta_t rel_maxp1,
		  cu_bool_t is_full);

cuex_otab_opr_t
cuex_otab_defopr(cuex_otab_t tab, cu_idr_t idr, cu_sref_t sref,
		 cuex_otab_range_t super, cu_rank_t r);

cu_bool_t
cuex_otab_print_std_sources(cuex_otab_t otab, cu_str_t out_h, cu_str_t out_c);

/*!@}*/
CU_END_DECLARATIONS

#endif
