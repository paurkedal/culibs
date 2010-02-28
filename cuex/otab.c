/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2006--2010  Petter Urkedal <paurkedal@eideticdew.org>
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

#include <cuex/otab.h>
#include <cu/idr.h>
#include <cu/int.h>
#include <cu/str.h>
#include <cu/cstr.h>
#include <cucon/bitarray.h>
#include <inttypes.h>
#include <ctype.h>

cu_dlog_def(_file, "dtag=cuex.otab");

#define SIMPLERANGE(rng) cu_to(cuex_otab_simplerange, rng)
#if 0
#  define OFMT "CUEX_META_C(0x%"CUEX_PRIxMETA")"
#else
#  define OFMT "0x%"CUEX_PRIxMETA"UL"
#endif

#ifndef cuex_opr
#  define cuex_opr(sel, r)						\
    (((cuex_meta_t)(sel) << CUEX_OPR_SELECT_SHIFT) |			\
     ((cuex_meta_t)(r) << CUEX_OPR_ARITY_SHIFT) | 1)
#endif

static cuex_otab_def_t otab_allocdef(cuex_otab_t tab, cu_idr_t idr,
				     cu_location_t loc,
				     cuex_otab_objkind_t kind, size_t size);

cu_clop_def(simplerange_cmp, int, void *lhs, void *rhs)
{
    cuex_meta_t lhs_min = ((cuex_otab_simplerange_t)lhs)->range_min;
    cuex_meta_t rhs_min = ((cuex_otab_simplerange_t)rhs)->range_min;
    if (lhs_min < rhs_min)
	return -1;
    else if (lhs_min > rhs_min)
	return 1;
    else
	return 0;
}

static cu_bool_t
simplerange_init(cuex_otab_t tab, cuex_otab_simplerange_t self,
		 cuex_otab_range_t super,
		 cuex_meta_t rel_min, cuex_meta_t rel_maxp1);


static cuex_otab_def_t
otab_allocdef(cuex_otab_t tab, cu_idr_t idr, cu_location_t loc,
	      cuex_otab_objkind_t kind, size_t size)
{
    cuex_otab_def_t def;
    if (!idr)
	def = cu_galloc(size);
    else if (!cucon_pmap_insert_mem(&tab->env, idr, size, &def)) {
	if (tab->error) {
	    (*tab->error)(tab, loc, "Multiple definions of %s.",
			  cu_idr_to_cstr(idr));
	    (*tab->error)(tab, def->loc, "First defined here.");
	}
	return NULL;
    }
    def->idr = idr;
    def->loc = loc;
    def->objkind = kind;
    def->is_extern = tab->is_extern;
    return def;
}

void
cuex_otab_init(cuex_otab_t tab, int width,
	       void (*error)(cuex_otab_t, cu_location_t, char const *msg, ...))
{
    cuex_otab_range_t all;
    cucon_pmap_init(&tab->env);
    all = cu_from2(cuex_otab_range, cuex_otab_simplerange, cuex_otab_def,
		   otab_allocdef(tab, cu_idr_by_cstr("__all"), NULL,
				 cuex_otab_range_kind,
				 sizeof(struct cuex_otab_range)));
    SIMPLERANGE(all)->super = NULL;
    SIMPLERANGE(all)->range_low_bit = 0;
    SIMPLERANGE(all)->range_min = 0;
    SIMPLERANGE(all)->range_maxp1 = 1 << width;
    cucon_rbset_init(&SIMPLERANGE(all)->subrange_set, simplerange_cmp);
    cucon_list_init(&all->prop_list);
    cucon_list_init(&all->opr_list);
    tab->name = NULL;
    tab->is_extern = cu_false;
    tab->all_width = width;
    tab->all = all;
    tab->error = error;
}

cuex_otab_t
cuex_otab_new(int width,
	      void (*error)(cuex_otab_t, cu_location_t, char const *msg, ...))
{
    cuex_otab_t tab = cu_gnew(struct cuex_otab);
    cuex_otab_init(tab, width, error);
    return tab;
}

static void
report_conflict(cuex_otab_t tab,
		cuex_otab_simplerange_t self,
		cuex_otab_simplerange_t conflict)
{
    if (tab->error) {
	cu_location_t self_loc = cu_to(cuex_otab_def, self)->loc;
	cu_location_t conflict_loc = cu_to(cuex_otab_def, conflict)->loc;
	if (cu_to(cuex_otab_def, self)->objkind == cuex_otab_reservation_kind)
	    (*tab->error)(tab, self_loc,
			  "Reservation in %s conflicts with ...",
			  cu_idr_to_cstr(cuex_otab_range_idr(self->super)));
	else
	    (*tab->error)(tab, self_loc, "Range %s conflicts with ...",
			  cu_idr_to_cstr(cu_to(cuex_otab_def, self)->idr));
	if (cu_to(cuex_otab_def, self)->objkind == cuex_otab_reservation_kind)
	    (*tab->error)(tab, conflict_loc, "... reservation in %s.",
			  cu_idr_to_cstr(cuex_otab_range_idr(self->super)));
	else
	    (*tab->error)(tab, conflict_loc, "... %s defined here.",
			  cu_idr_to_cstr(cu_to(cuex_otab_def, conflict)->idr));
    }
}

static cu_bool_t
simplerange_init(cuex_otab_t tab, cuex_otab_simplerange_t self,
		 cuex_otab_range_t super,
		 cuex_meta_t min, cuex_meta_t maxp1)
{
    cu_bool_t status = cu_true;
    cuex_otab_simplerange_t below, equal, above;
    cu_debug_assert(min < maxp1);
    self->super = super;
    self->range_low_bit = SIMPLERANGE(super)->range_low_bit;
    self->range_min = SIMPLERANGE(super)->range_min
		    + (min << self->range_low_bit);
    self->range_maxp1 = SIMPLERANGE(super)->range_min
		      + (maxp1 << self->range_low_bit);
    if ((maxp1 << self->range_low_bit) == 0 ||
	self->range_maxp1 > SIMPLERANGE(super)->range_maxp1) {
	if (tab->error) {
	    (*tab->error)(tab, cu_to(cuex_otab_def, self)->loc,
			  "Range does not fit in %s.",
			  cu_idr_to_cstr(
				cu_to2(cuex_otab_def, cuex_otab_simplerange,
				       super)->idr));
	    return cu_false;
	}
    }
    cucon_rbset_init(&self->subrange_set, simplerange_cmp);
    cucon_rbset_nearest(&SIMPLERANGE(super)->subrange_set, self,
			    &below, &equal, &above);
    if (equal) {
	report_conflict(tab, self, equal);
	status = cu_false;
    }
    if (below && below->range_maxp1 > self->range_min) {
	report_conflict(tab, self, below);
	status = cu_false;
    }
    if (above && self->range_maxp1 > above->range_min) {
	report_conflict(tab, self, above);
	return cu_false;
    }
    if (!status)
	return cu_false;
    cucon_rbset_insert(&SIMPLERANGE(super)->subrange_set, self);
    return cu_true;
}

cuex_otab_range_t
cuex_otab_defrange(cuex_otab_t tab, cu_idr_t idr, cu_location_t loc,
		   cuex_otab_range_t super,
		   cuex_meta_t rel_min, cuex_meta_t rel_maxp1)
{
    cuex_otab_def_t def;
    cuex_otab_range_t rng;
    def = otab_allocdef(tab, idr, loc, cuex_otab_range_kind,
			sizeof(struct cuex_otab_range));
    if (!def)
	return NULL;
    rng = cu_from2(cuex_otab_range, cuex_otab_simplerange, cuex_otab_def, def);
    if (!simplerange_init(tab, cu_to(cuex_otab_simplerange, rng),
			  super, rel_min, rel_maxp1))
	return NULL;
    cucon_list_init(&rng->prop_list);
    cucon_list_init(&rng->opr_list);
    return rng;
}

cuex_otab_prop_t
cuex_otab_defprop(cuex_otab_t tab, cu_idr_t idr, cu_location_t loc,
		  cuex_otab_range_t super,
		  int width, char const *type_cstr, cu_bool_t is_implicit)
{
    cuex_otab_def_t def;
    cuex_otab_prop_t prop;
    int alignbit;
    if (!cucon_rbset_is_empty(&SIMPLERANGE(super)->subrange_set)) {
	if (tab->error)
	    (*tab->error)(tab, loc,
			  "Cannot add property to %s which already has "
			  "subranges.",
			  cu_idr_to_cstr(
				cu_to2(cuex_otab_def, cuex_otab_simplerange,
				       super)->idr));
	return NULL;
    }
    alignbit = cu_ulong_log2_lowbit(SIMPLERANGE(super)->range_min |
				    SIMPLERANGE(super)->range_maxp1);
    if (width + SIMPLERANGE(super)->range_low_bit > alignbit) {
	if (tab->error)
	    (*tab->error)(tab, loc,
			  "Insufficient number of usable bits for property "
			  "%s, only %d available.",
			  cu_idr_to_cstr(idr),
			  alignbit - SIMPLERANGE(super)->range_low_bit);
	return NULL;
    }
    def = otab_allocdef(tab, idr, loc, cuex_otab_prop_kind,
			sizeof(struct cuex_otab_prop));
    if (!def)
	return NULL;
    prop = cu_from(cuex_otab_prop, cuex_otab_def, def);
    prop->is_implicit = is_implicit;
    prop->width = width;
    prop->low_bit = SIMPLERANGE(super)->range_low_bit;
    prop->type_cstr = type_cstr;
    SIMPLERANGE(super)->range_low_bit += width;
    cucon_list_append_ptr(&super->prop_list, prop);
    return prop;
}

cu_bool_t
cuex_otab_reserve(cuex_otab_t tab, cu_location_t loc, cuex_otab_range_t super,
		  cuex_meta_t rel_min, cuex_meta_t rel_maxp1,
		  cu_bool_t is_full)
{
    cuex_otab_def_t def;
    cuex_otab_reservation_t rsv;
    def = otab_allocdef(tab, NULL, loc, cuex_otab_reservation_kind,
			sizeof(struct cuex_otab_reservation));
    rsv = cu_from2(cuex_otab_reservation, cuex_otab_simplerange,
		   cuex_otab_def, def);
    if (!simplerange_init(tab, cu_to(cuex_otab_simplerange, rsv),
			  super, rel_min, rel_maxp1))
	return cu_false;
    rsv->is_full = is_full;
    cucon_array_init(&rsv->freemask, cu_false, 0);
    cucon_bitarray_init_fill(&rsv->all_freemask, rel_maxp1 - rel_min, cu_true);
    cucon_bitarray_init_fill(&rsv->multi_freemask, rel_maxp1-rel_min, cu_true);
    return cu_true;
}

cu_clos_def(alloc_in_reservation,
	    cu_prot(cu_bool_t, void *key),
  ( cuex_meta_t index;
    cu_rank_t r; ))
{
    cu_clos_self(alloc_in_reservation);
    cuex_otab_reservation_t rsv;
    cucon_bitarray_t bv CU_NOINIT(NULL);
    cu_rank_t i;
    cuex_meta_t index;
    if (((cuex_otab_def_t)key)->objkind != cuex_otab_reservation_kind)
	return cu_true;
    rsv = cu_from2(cuex_otab_reservation, cuex_otab_simplerange,
		   cuex_otab_def, key);
    cu_dlogf(_file,
	     "RSV %p, length %d: Looking for operator in %s of arity %d.",
	     rsv, cuex_otab_reservation_length(rsv),
	     cu_idr_to_cstr(cuex_otab_range_idr(
		     cuex_otab_reservation_super(rsv))),
	     self->r);
    if (rsv->is_full) {
	cu_dlogf(_file, "RSV %p is full.", rsv);
	return cu_true;
    }
    if (self->r == -1) {
	index = cucon_bitarray_find(&rsv->all_freemask, 0, cu_true);
	if (index != (size_t)-1) {
	    cu_debug_assert(cucon_bitarray_at(&rsv->multi_freemask, index));
	    cucon_bitarray_set_at(&rsv->multi_freemask, index, cu_false);
	    i = cucon_array_size(&rsv->freemask)/sizeof(struct cucon_bitarray);
	    bv = cucon_array_ref_at(&rsv->freemask, 0);
	    while (i--) {
		cu_debug_assert(cucon_bitarray_at(bv, index));
		cucon_bitarray_set_at(bv++, index, cu_false);
	    }
	}
    }
    else {
	i = cucon_array_size(&rsv->freemask)/sizeof(struct cucon_bitarray);
	if (i <= self->r) {
	    bv = cucon_array_extend_gp(&rsv->freemask,
				       (self->r + 1 - i)
					    * sizeof(struct cucon_bitarray));
	    cu_dlogf(_file, "RSV %p: Expanding to arity %d", rsv, self->r);
	    do
		cucon_bitarray_init_copy(bv++, &rsv->multi_freemask);
	    while (++i <= self->r);
	    --bv;
	}
	else
	    bv = cucon_array_ref_at(&rsv->freemask,
				    self->r*sizeof(struct cucon_bitarray));
	index = cucon_bitarray_find(bv, 0, cu_true);
	if (index != (size_t)-1)
	    cucon_bitarray_set_at(bv, index, cu_false);
    }
    if (index != (size_t)-1) {
	cucon_bitarray_set_at(&rsv->all_freemask, index, cu_false);
	index <<= cuex_otab_reservation_low_bit(rsv);
	index += cuex_otab_reservation_min(rsv);
	cu_dlogf(_file, "RSV %p: Allocated slot %d.", rsv, index);
	self->index = index;
	return cu_false;
    }
    else {
	cu_dlogf(_file, "RSV %p: All slots taken. 0x%lx.", rsv, bv->arr[0]);
	return cu_true;
    }
}

cuex_otab_opr_t
cuex_otab_defopr(cuex_otab_t tab, cu_idr_t idr, cu_location_t loc,
		 cuex_otab_range_t super, cu_rank_t r)
{
    alloc_in_reservation_t alloc_cb;
    cuex_otab_def_t def;
    cuex_otab_opr_t op;
    def = otab_allocdef(tab, idr, loc, cuex_otab_opr_kind,
			sizeof(struct cuex_otab_opr));
    if (!def)
	return NULL;
    op = cu_from(cuex_otab_opr, cuex_otab_def, def);
    alloc_cb.r = r;
    if (cucon_rbset_conj(&SIMPLERANGE(super)->subrange_set,
			 alloc_in_reservation_prep(&alloc_cb))) {
	if (tab->error) {
	    (*tab->error)(tab, loc,
			  "Reservations for %s are full or absent.",
			  cu_idr_to_cstr(cuex_otab_range_idr(super)));
	    return NULL;
	}
    }
    op->has_ctor = cu_false;
    op->r = r;
    op->index = alloc_cb.index;
    cucon_list_append_ptr(&super->opr_list, op);
    return op;
}


/* Output
 * ====== */

#if CUEX_OTAB_ENABLE_ARGCTOR
static int
_print_prop_argl(cuex_otab_range_t range, cu_bool_t is_var_arity, FILE *out)
{
    cucon_listnode_t node;
    cuex_otab_range_t super = cuex_otab_range_super(range);
    int arg_index;
    if (super)
	arg_index = _print_prop_argl(super, is_var_arity, out);
    else
	arg_index = 0;
    for (node = cucon_list_begin(&range->prop_list);
	 node != cucon_list_end(&range->prop_list);
	 node = cucon_listnode_next(node)) {
	cuex_otab_prop_t prop = cucon_listnode_ptr(node);
	if (prop->is_implicit)
	    continue;
	if (!arg_index && !is_var_arity)
	    fputc('(', out);
	else
	    fputs(", ", out);
	fprintf(out, "a%d", arg_index);
	++arg_index;
    }
    return arg_index;
}

static int
_print_prop_bits(cuex_otab_range_t range, FILE *out)
{
    cucon_listnode_t node;
    cuex_otab_range_t super = cuex_otab_range_super(range);
    int arg_index;
    if (super)
	arg_index = _print_prop_bits(super, out);
    else
	arg_index = 0;
    for (node = cucon_list_begin(&range->prop_list);
	 node != cucon_list_end(&range->prop_list);
	 node = cucon_listnode_next(node)) {
	cuex_otab_prop_t prop = cucon_listnode_ptr(node);
	if (prop->is_implicit)
	    continue;
	fprintf(out, " | ((a%d) << %d)",
		arg_index, cuex_otab_prop_low_bit(prop));
    }
    return arg_index;
}
#endif

static void
_print_std_range(cuex_otab_t otab, cuex_otab_range_t range,
		 FILE *out_h, FILE *out_c)
{
    cucon_listnode_t node;
    char const *range_name = cu_idr_to_cstr(cuex_otab_range_idr(range));
    cuex_meta_t min = cuex_otab_range_min(range);
    cuex_meta_t maxp1 = cuex_otab_range_maxp1(range);
    cuex_meta_t prop_mask;
    cuex_otab_range_t super = cuex_otab_range_super(range);
    int lbit = cuex_otab_range_low_bit(range);
    int oa_lbit = super? cuex_otab_range_low_bit(super) : 0;
    char const *range_ucname;
    range_ucname = cu_cstr_toupper(cu_cstr_new_copy(range_name));
    fprintf(out_h,
	    "\n/*-- range %s "
	    "= %s[0x%"CUEX_PRIxMETA" .! 0x%"CUEX_PRIxMETA"] --*/\n\n",
	    range_name,
	    super? cu_idr_to_cstr(cuex_otab_range_idr(super)) : "__all",
	    min, maxp1);

    /* The Group */
    if (!cuex_otab_range_to_def(range)->is_extern) {
	fprintf(out_h,
		"#define %s_REL_MIN "OFMT"\n"
		"#define %s_REL_MAXP "OFMT"\n"
		"#define %s_SHIFT %d\n"
		"#define %s_ATTRS_SHIFT %d\n"
		"#define %s_ATTRS_WIDTH %d\n\n",
		range_ucname, min,
		range_ucname, maxp1,
		range_ucname, lbit + CUEX_OPR_SELECT_SHIFT,
		range_ucname, oa_lbit + CUEX_OPR_SELECT_SHIFT,
		range_ucname, lbit - oa_lbit);
	fprintf(out_h,
		"CU_SINLINE cu_bool_t %s_contains(cuex_meta_t opr)\n",
		range_name);
	switch ((min != 0) + 2*(maxp1 != cuex_otab_all_maxp(otab))) {
	    case 0:
		fputs("{ return cu_true; }\n\n", out_h);
		break;
	    case 1:
		fprintf(out_h, "{ return "OFMT" <= opr; }\n\n",
			min << CUEX_OPR_SELECT_SHIFT);
		break;
	    case 2:
		fprintf(out_h, "{ return opr < "OFMT"; }\n\n",
			maxp1 << CUEX_OPR_SELECT_SHIFT);
		break;
	    case 3:
		fprintf(out_h, "{ return "OFMT" <= opr && opr < "OFMT"; }\n\n",
			min << CUEX_OPR_SELECT_SHIFT,
			maxp1 << CUEX_OPR_SELECT_SHIFT);
		break;
	}
	fprintf(out_h,
		"CU_SINLINE cuex_meta_t %s_strip(cuex_meta_t opr)\n"
		"{ return opr & "OFMT"; }\n\n",
		range_name,
		~(((CUEX_META_C(1) << lbit) - 1) << CUEX_OPR_SELECT_SHIFT));
    }

    /* Attributes */
    for (node = cucon_list_begin(&range->prop_list);
	 node != cucon_list_end(&range->prop_list);
	 node = cucon_listnode_next(node)) {
	cuex_otab_prop_t prop = cucon_listnode_ptr(node);
	char const *prop_name = cu_idr_to_cstr(cuex_otab_prop_idr(prop));
	char const *prop_ucname = cu_cstr_toupper(cu_cstr_new_copy(prop_name));
	char const *type_cstr;
	cuex_meta_t attr_mask = ((CUEX_META_C(1) << prop->width) - 1)
				<< (prop->low_bit + CUEX_OPR_SELECT_SHIFT);
	int attr_shift = prop->low_bit + CUEX_OPR_SELECT_SHIFT;
	if (cu_to(cuex_otab_def, prop)->is_extern)
	    continue;
	type_cstr = prop->type_cstr;
	cu_verbf(20, "    Properity %s.",
		 cu_idr_to_cstr(cuex_otab_prop_idr(prop)));
	if (!type_cstr) type_cstr = "unsigned int";
	fprintf(out_h, "#define %s(a) ((cuex_meta_t)(a) << %d)\n",
		prop_ucname, prop->low_bit + CUEX_OPR_SELECT_SHIFT);
	fprintf(out_h, "#define %s_MASK "OFMT"\n",
		prop_ucname, attr_mask);
	fprintf(out_h, "#define %s_WIDTH %d\n\n",
		prop_ucname, prop->width);
	fprintf(out_h,
		"CU_SINLINE cuex_meta_t %s_set(cuex_meta_t opr, %s val)\n"
		"{ return (opr & "OFMT") | ((cuex_meta_t)val << %d); }\n\n",
		prop_name, type_cstr, ~attr_mask, attr_shift);
	fprintf(out_h,
		"CU_SINLINE %s %s(cuex_meta_t opr)\n"
		"{ return (%s)((opr >> %d) & 0x%x); }\n\n",
		type_cstr, prop_name, type_cstr, attr_shift,
		(1 << prop->width) - 1);
    }
    prop_mask = (CUEX_META_C(1) << (cuex_otab_range_low_bit(range)
				    + CUEX_OPR_SELECT_SHIFT))
	      - (CUEX_META_C(1) << CUEX_OPR_SELECT_SHIFT);

    /* Operators */
    for (node = cucon_list_begin(&range->opr_list);
	 node != cucon_list_end(&range->opr_list);
	 node = cucon_listnode_next(node)) {
	cuex_otab_opr_t opr = cucon_listnode_ptr(node);
	char const *opr_name, *opr_ucname;
	char const *arity0_arg;
	cuex_meta_t index, meta;
	int flags = 0;
	int r, i;
	if (cuex_otab_opr_to_def(opr)->is_extern)
	    continue;
	opr_name = cu_idr_to_cstr(cuex_otab_opr_idr(opr));
	opr_ucname = cu_cstr_toupper(cu_cstr_new_copy(opr_name));
	index = opr->index;
	r = opr->r;
	if (r >= 0) {
	    meta = cuex_opr(index, r);
	    if (opr->has_ctor)
		meta |= CUEX_OPRFLAG_CTOR;
	    arity0_arg = "";
	    fprintf(out_h, "#define %s "OFMT" /* 0x%"CUEX_PRIxMETA" */\n",
		    opr_ucname, meta, index);
	    fprintf(out_h, "#define %s(", opr_name);
	    if (r > 0) {
		fputs("e0", out_h);
		for (i = 1; i < r; ++i)
		    fprintf(out_h, ", e%d", i);
		fprintf(out_h, ") cuex_opn("OFMT, meta);
		for (i = 0; i < r; ++i)
		    fprintf(out_h, ", e%d", i);
		fprintf(out_h, ")\n");
	    } else {
		/* TODO. Nullary operators can be pre-constructed. */
		fprintf(out_h, ") cuex_opn("OFMT")\n", meta);
	    }
	}
	else {
	    meta = cuex_opr(index, 0);
	    if (opr->has_ctor)
		meta |= CUEX_OPRFLAG_CTOR;
	    arity0_arg = "(0)";
	    fprintf(out_h, "#define %s(r) ("OFMT" | (cuex_meta_t)(r) << %d)\n",
		    opr_ucname, meta, CUEX_OPR_ARITY_SHIFT);
	}
	if (!opr->has_ctor)
	    fprintf(out_c,
		    "  {CUCON_UMAP_NODE_INIT(%s%s), \"%s\", 0, NULL, %d},\n",
		    opr_ucname, arity0_arg, opr_name, flags);
	else
	    fprintf(out_c,
		    "  {CUCON_UMAP_NODE_INIT(%s%s), \"%s\", "
		    "%s_cache_size, %s_cache_init, %d},\n",
		    opr_ucname, arity0_arg, opr_name,
		    opr_name, opr_name, flags);

#if CUEX_OTAB_ENABLE_ARGCTOR
	/* Operation */
	fprintf(out_h, "#define %s_A", opr_ucname);
	if (r == -1) {
	    fputs("(r", out_h);
	    _print_prop_argl(range, cu_true, out_h);
	}
	else
	    _print_prop_argl(range, cu_false, out_h);
	fputs(")\n{\n", out_h);
	fprintf(out_h, " cuex_opr(0x%x", index);
	_print_prop_bits(range, out_h);
	if (r == -1)
	    fprintf(out_h, ", r)\n");
	else
	    fprintf(out_h, ", %d)\n", r);
#endif

#if CUEX_OTAB_ENABLE_COMPARISON
	/* Operator comparison */
	fprintf(out_h, "CU_SINLINE cu_bool_t %s_eq(cuex_meta_t o)\n{\n",
		opr_name);
	if (r == -1)
	    fprintf(out_h,
		    "    return (o & "OFMT") == "OFMT";\n",
		    ~(cuex_meta_t)(prop_mask | CUEX_OPR_ARITY_MASK),
		    cuex_opr(index, 0));
	else
	    fprintf(out_h,
		    "    return (o & "OFMT") == "OFMT";\n",
		    ~prop_mask, cuex_opr(index, r));
	fputs("}\n", out_h);
#endif
    }
    fputc('\n', out_h);
}

cu_clos_def(_print_std_sources_cb,
	    cu_prot(cu_bool_t, void *def),
  ( cuex_otab_t tab;
    FILE *out_h, *out_c; ))
{
    cu_clos_self(_print_std_sources_cb);
    if (cuex_otab_def_idr(def))
	cu_verbf(20, "Processing %s.",
		cu_idr_to_cstr(cuex_otab_def_idr(def)));
    else
	cu_verbf(20, "Reservation.");
    switch (cuex_otab_def_kind(def)) {
	case cuex_otab_range_kind: {
	    cuex_otab_range_t range = cuex_otab_range_from_def(def);
	    _print_std_range(self->tab, range, self->out_h, self->out_c);
	    cucon_rbset_conj(&SIMPLERANGE(range)->subrange_set,
			     _print_std_sources_cb_ref(self));
	    break;
	}
	case cuex_otab_reservation_kind: {
	    cuex_otab_reservation_t rsv = cuex_otab_reservation_from_def(def);
	    cucon_rbset_conj(&SIMPLERANGE(rsv)->subrange_set,
			     _print_std_sources_cb_ref(self));
	    break;
	}
	case cuex_otab_opr_kind:
	case cuex_otab_prop_kind:
	    cu_debug_unreachable();
    }
    return cu_true;
}

cu_clos_def(_print_std_sources_cb1,
	    cu_prot(cu_bool_t, void *def),
  ( int oa_cnt;
    int arg_cnt_max;
    cuex_meta_t arg_mask;
    FILE *out_c; ))
{
    cu_clos_self(_print_std_sources_cb1);
    switch (cuex_otab_def_kind(def)) {
	case cuex_otab_range_kind: {
	    cuex_otab_range_t range = cuex_otab_range_from_def(def);
	    if (cucon_list_is_empty(&range->prop_list))
		cucon_rbset_conj(&SIMPLERANGE(range)->subrange_set,
				 _print_std_sources_cb1_ref(self));
	    else {
		_print_std_sources_cb1_t cb1;
		cuex_meta_t min = cuex_otab_range_min(range);
		cuex_meta_t maxp = cuex_otab_range_maxp1(range);
		cucon_listnode_t prop_node;
		int oa_cnt;
		cuex_meta_t arg_mask;
		fprintf(self->out_c,
			"if ("OFMT" <= opr && opr < "OFMT") {\n",
			min << CUEX_OPR_SELECT_SHIFT,
			maxp << CUEX_OPR_SELECT_SHIFT);
		oa_cnt = self->oa_cnt;
		arg_mask = self->arg_mask;
		for (prop_node = cucon_list_begin(&range->prop_list);
		     prop_node != cucon_list_end(&range->prop_list);
		     prop_node = cucon_listnode_next(prop_node)) {
		    cuex_otab_prop_t prop = cucon_listnode_ptr(prop_node);
		    arg_mask |= ((CUEX_META_C(1) << prop->width) - 1)
				<< prop->low_bit;
		    fprintf(self->out_c,
			    "*oa_arr++ = (opr >> %d) & 0x%x;\n",
			    prop->low_bit + CUEX_OPR_SELECT_SHIFT,
			    (1 << prop->width) - 1);
		    ++oa_cnt;
		}
		cb1.oa_cnt = oa_cnt;
		cb1.arg_mask = arg_mask;
		cb1.arg_cnt_max =
		    oa_cnt > self->arg_cnt_max ? oa_cnt : self->arg_cnt_max;
		cb1.out_c = self->out_c;
		cucon_rbset_conj(&SIMPLERANGE(range)->subrange_set,
				 _print_std_sources_cb1_prep(&cb1));
		self->arg_cnt_max = cb1.arg_cnt_max;
		fprintf(self->out_c,
			"{\n*oa_cnt = %d;\n"
			"opr &= "OFMT";\n}\n",
			oa_cnt, ~(arg_mask << CUEX_OPR_SELECT_SHIFT));
		fputs("} else ", self->out_c);
	    }
	    break;
	}
	case cuex_otab_reservation_kind: {
	    cuex_otab_reservation_t rsv = cuex_otab_reservation_from_def(def);
	    cucon_rbset_conj(&SIMPLERANGE(rsv)->subrange_set,
			     _print_std_sources_cb1_ref(self));
	    break;
	}
	default:
	    cu_debug_unreachable();
    }
    return cu_true;
}

cu_bool_t
cuex_otab_print_std_sources(cuex_otab_t tab, cu_str_t path_h, cu_str_t path_c)
{
    _print_std_sources_cb_t cb;
    _print_std_sources_cb1_t cb1;
    FILE *out_h, *out_c;
    cu_str_t tab_cpath;
    char *tab_name, *tab_ucname, *s;
    size_t i;

    cu_debug_assert(tab->name);
    i = cu_str_rchr(tab->name, '.');
    if (i != cu_str_index_none)
	tab_cpath = cu_str_substr(tab->name, 0, i);
    else
	tab_cpath = tab->name;
    tab_name = cu_cstr_new_copy(cu_str_to_cstr(tab_cpath));
    for (s = tab_name; *s; ++s) {
	if (!isalnum(*s))
	    *s = '_';
    }
    tab_ucname = cu_cstr_toupper(cu_cstr_new_copy(tab_name));

    out_h = fopen(cu_str_to_cstr(path_h), "w");
    if (!out_h) {
	(tab->error)(tab, NULL, "Cannot create output file %s.",
		     cu_str_to_cstr(path_h));
	return cu_false;
    }
    out_c = fopen(cu_str_to_cstr(path_c), "w");
    if (!out_c) {
	fclose(out_h);
	(tab->error)(tab, NULL, "Cannot create output file %s.",
		     cu_str_to_cstr(path_c));
	return cu_false;
    }

    /* Prologues. */
    fprintf(out_h,
	    "/* !! This is a generated file !! */\n\n"
	    "#ifndef %s_H\n"
	    "#define %s_H\n\n"
	    "#include <cuex/ex.h>\n%s\n"
	    "CU_BEGIN_DECLARATIONS\n\n",
	    tab_ucname, tab_ucname,
	    tab->h_prologue? cu_str_to_cstr(tab->h_prologue) : "");
    fprintf(out_c,
	    "/* !! This is a generated file !! */\n\n"
	    "#include <cuex/oprinfo.h>\n"
	    "#include <%s.h>\n"
	    "%s\n"
	    "static struct cuex_oprinfo oprinfo_arr[] = {\n",
	    cu_str_to_cstr(tab_cpath),
	    tab->c_prologue? cu_str_to_cstr(tab->c_prologue) : "");

    /* Main definitions. */
    cb.tab = tab;
    cb.out_h = out_h;
    cb.out_c = out_c;
    cucon_rbset_conj(&SIMPLERANGE(tab->all)->subrange_set,
		     _print_std_sources_cb_prep(&cb));
    fprintf(out_c,
	    "  {CUCON_UMAP_NODE_INIT(0), NULL, 0, NULL, 0}\n"
	    "};\n"
	    "\n");

    /* Argument decoding function. */
    cb1.oa_cnt = 0;
    cb1.arg_cnt_max = 0;
    cb1.arg_mask = 0;
    cb1.out_c = out_c;
    fprintf(out_c,
	    "cuex_meta_t\n"
	    "%s_decode_opr(cuex_meta_t opr, int *oa_cnt, int *oa_arr)\n{\n",
	    tab_name);
    cucon_rbset_conj(&SIMPLERANGE(tab->all)->subrange_set,
		     _print_std_sources_cb1_prep(&cb1));
    fprintf(out_h,
	    "/*!The maximum number of operator arguments used. */\n"
	    "#define %s_OA_COUNT_MAX %d\n\n"
	    "/*!Decode operator arguments into \\a oa_arr, set \\a oa_cnt\n"
	    " * to the number of arguments and return the operator after\n"
	    " * stripping its arguments.\n"
	    " * \\pre \\a oa_arr must have size at least "
	    "\\ref %s_OA_COUNT_MAX. */\n"
	    "cuex_meta_t\n"
	    "%s_decode_opr(cuex_meta_t opr, int *oa_cnt, int *oa_arr);\n\n",
	    tab_ucname, cb1.arg_cnt_max, tab_ucname, tab_name);
    fputs("*oa_cnt = 0;\n"
	  "return opr;\n"
	  "}\n\n",
	  out_c);

    /* Epilogues. */
    fprintf(out_h,
	    "extern void %s_init(void);\n\n"
	    "CU_END_DECLARATIONS\n"
	    "%s\n"
	    "#endif\n", tab_name,
	    tab->h_epilogue? cu_str_to_cstr(tab->h_epilogue) : "");
    fprintf(out_c,
	    "void\n"
	    "%s_init(void)\n"
	    "{\n"
	    "    cuex_oprinfo_register(oprinfo_arr);\n"
	    "}\n%s\n",
	    tab_name,
	    tab->c_epilogue? cu_str_to_cstr(tab->c_epilogue) : "");
    fclose(out_h);
    fclose(out_c);
    return cu_true;
}
