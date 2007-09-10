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

#ifndef CUEX_FWD_H
#define CUEX_FWD_H

#include <cuoo/fwd.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuex_fwd_h cuex/fwd.h: Forward Declarations
 * @{\ingroup cuex_mod */

typedef struct cuex_fpvar_s	*cuex_fpvar_t;		/* fpvar.h */
typedef struct cuex_gvar_s	*cuex_gvar_t;		/* gvar.h */
typedef struct cuex_opn_s	*cuex_opn_t;		/* opn.h */
typedef struct cuex_oprinfo_s	*cuex_oprinfo_t;	/* oprinfo.h */
typedef struct cuex_pvar_s	*cuex_pvar_t;		/* pvar.h */
typedef struct cuex_subst_s	*cuex_subst_t;		/* subst.h */
typedef struct cuex_veqv_s	*cuex_veqv_t;		/* subst.h */
typedef struct cuex_tpvar_s	*cuex_tpvar_t;		/* tpvar.h */
typedef struct cuex_tvar_s	*cuex_tvar_t;		/* tvar.h */
typedef struct cuex_var_s	*cuex_var_t;		/* var.h */

#define cuex_meta_is_opr(meta) (cuex_meta_kind(meta) == cuex_meta_kind_opr)
#define cuex_var_from_ex(ex) ((cuex_var_t)(ex))
#define cuex_is_idr(ex) (cuex_meta(ex) == cuoo_type_to_meta(cu_idr_type()))
#define cuex_idr_from_ex(ex) ((cu_idr_t)(ex))

/*!This function must be called before you use the cuex library or start
 * any threads which may use it. */
void cuex_init(void);

/*!@}*/
CU_END_DECLARATIONS

#endif
