/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2003--2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUEX_PVAR_H
#define CUEX_PVAR_H

#include <cuex/fwd.h>
#include <cuex/ex.h>
#include <cuex/var.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuex_pvar cuex/pvar.h: Variables with Internal Properties
 * @{\ingroup cuex_mod
 * \warning This is work in progress. The interface will change.
 */

/*!The meta of a variable with internal properties quantified as \a qcode. */
#define cuex_pvarmeta(qcode) cuex_varmeta_kqi(cuex_varkind_pvar, qcode, 0)

/*!True iff \a meta is of a \c cu_pvar_t with any quantisation. */
#define cuex_meta_is_pvarmeta(meta)					\
    cuex_is_varmeta_k(meta, cuex_varkind_pvar)

/*!True iff \a meta is of a \c cu_pvar_t quantified as \a qcode. */
#define cuex_meta_is_pvarmeta_q(meta, qcode)				\
    cuex_is_varmeta_kq(meta, cuex_varkind_pvar, qcode)

/*!True iff \a meta is of a universal \c cu_pvar_t. */
#define cuex_meta_is_pvarmeta_u(meta)					\
    cuex_meta_is_pvarmeta_q(meta, cuex_qcode_u)
/*!True iff \a meta is of a existential \c cu_pvar_t. */
#define cuex_meta_is_pvarmeta_e(meta)					\
    cuex_meta_is_pvarmeta_q(meta, cuex_qcode_e)
/*!True iff \a meta is of a weak universal \c cu_pvar_t. */
#define cuex_meta_is_pvarmeta_w(meta)					\
    cuex_meta_is_pvarmeta_q(meta, cuex_qcode_w)
/*!True iff \a meta is of a unquantified \c cu_pvar_t. */
#define cuex_meta_is_pvarmeta_n(meta)					\
    cuex_meta_is_pvarmeta_q(meta, cuex_qcode_n)

#define cuex_qcode_unknown cuex_qcode_n
#define cuex_qcode_na cuex_qcode_n
#define cuex_qcode_arg cuex_qcode_n
#define cuex_qcode_uniq cuex_qcode_n
#define cuex_qcode_active_s cuex_qcode_u
#define cuex_qcode_active_w cuex_qcode_w
#define cuex_qcode_passive cuex_qcode_e

char const *cuex_qcode_name(cuex_qcode_t quant);

struct cuex_pvar_s
{
    CUOO_OBJ
    cu_offset_t offset;	/* In bits if 'type' is a bitfield */
    void *module; /* XXX */
    cu_idr_t idr;
    cuex_t type;
    cuex_pvar_t var_chain;
};

/*!Return a unique variable with the given quantization. */
cuex_pvar_t cuex_pvar_new(cuex_qcode_t qcode);

/*!Return a unique variable with the given quantization and set its
 * print name with 'chi_ex_set_print_str'. */
cuex_pvar_t cuex_pvar_new_named(cuex_qcode_t qcode, char const *print_str);

#define cuex_pvar_from_ex(ex) ((cuex_pvar_t)(ex))
#define cuex_pvar_to_ex(var) ((cuex_t)(var))
#define cuex_pvar_from_var(v) ((cuex_pvar_t)(v))
#define cuex_pvar_to_var(v) ((cuex_var_t)(v))

/*!The quantifier code of \a var. */
CU_SINLINE cuex_qcode_t
cuex_pvar_qcode(cuex_pvar_t var)
{ return cuex_varmeta_qcode(cuex_meta(var)); }

CU_SINLINE void
cuex_pvar_set_static_type(cuex_pvar_t var, cuex_t type)
{ var->type = type; }

CU_SINLINE cuex_t
cuex_pvar_static_type(cuex_pvar_t var)
{ return var->type; }

CU_SINLINE cu_offset_t
cuex_pvar_offset(cuex_pvar_t var)
{
    assert(var->offset != (cu_offset_t)-1);
    return var->offset;
}

cu_bool_t cuex_pvar_has_offset(cuex_pvar_t);
#define cuex_pvar_has_offset(var) \
	(CU_MARG(cuex_pvar_t, var)->offset != (cu_offset_t)-1)

/* XXX */
CU_SINLINE void *cuex_pvar_module(cuex_pvar_t var) { return var->module; }

CU_SINLINE cu_idr_t
cuex_pvar_idr(cuex_pvar_t var) { return var->idr; }

/* XXX */
void cuex_pvar_set_module(cuex_pvar_t var, void *M, cu_idr_t idr);

cu_str_t cuex_pvar_name(cuex_pvar_t var);

cuex_t cuex_pvar_qname(cuex_pvar_t var);

cu_idr_t cuex_pvar_qidr(cuex_pvar_t var);

/*!@}*/
CU_END_DECLARATIONS
#endif
