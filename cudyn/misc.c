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

#include <cudyn/misc.h>
#if CUCONF_HAVE_LIBFFI_FFI_H
#  include <libffi/ffi.h>
#else
#  include <ffi.h>
#endif
#include <inttypes.h>
#include <cudyn/properties.h>
#include <cu/idr.h>
#include <cu/str.h>
#include <cuex/oprinfo.h>

CUDYN_ETYPEARR_DEFN_PRINT(char, char, "(char)%c")
CUDYN_ETYPEARR_DEFN_PRINT(int8, int8_t, "(int8_t)%"PRId8)
CUDYN_ETYPEARR_DEFN_PRINT(uint8, uint8_t, "(uint8_t)%"PRIu8)
CUDYN_ETYPEARR_DEFN_PRINT(int16, int16_t, "(int16_t)%"PRId16)
CUDYN_ETYPEARR_DEFN_PRINT(uint16, uint16_t, "(uint16_t)%"PRIu16)
#if CUCONF_SIZEOF_INT == 4
CUDYN_ETYPEARR_DEFN_PRINT(int32, int32_t, "%"PRId32)
CUDYN_ETYPEARR_DEFN_PRINT(uint32, uint32_t, "%"PRIu32"U")
#else
CUDYN_ETYPEARR_DEFN_PRINT(int32, int32_t, "(int32_t)%"PRId32)
CUDYN_ETYPEARR_DEFN_PRINT(uint32, uint32_t, "(uint32_t)%"PRIu32)
#endif
#if CUCONF_SIZEOF_INT == 8
CUDYN_ETYPEARR_DEFN_PRINT(int64, int64_t, "%"PRId64)
CUDYN_ETYPEARR_DEFN_PRINT(uint64, uint64_t, "%"PRIu64"U")
#else
CUDYN_ETYPEARR_DEFN_PRINT(int64, int64_t, "(int64_t)%"PRId64)
CUDYN_ETYPEARR_DEFN_PRINT(uint64, uint64_t, "(uint64_t)%"PRIu64)
#endif
CUDYN_ETYPEARR_DEFN_PRINT(float, float, "%g")
CUDYN_ETYPEARR_DEFN_PRINT(double, double, "%lgL")

cudyn_elmtype_t cudynP_bool_type;

struct cudynP_bool_aligntest { char ch; cu_bool_t x; };

static void cudynP_bool_print(cuex_t e, FILE *out)
{
    fputs(cudyn_to_bool(e)? "true" : "false", out);
}

cuex_t cudynP_true;
cuex_t cudynP_false;

void
cudynP_str_print(cuex_t ex, FILE *out)
{
    char const *s = cu_str_to_cstr(cu_str_quote(ex));
    fprintf(out, "%s", s);
}

void
cudynP_idr_print(cuex_t ex, FILE *out)
{
    char const *s = cu_idr_to_cstr(ex);
    fputs(s, out);
}

CUDYN_ARRTYPE_DEFN(cuex, cuex_t)

CUDYN_ETYPEARR_DEFN(metaint, cuex_meta_t)
static void cudynP_metaint_print(cuex_t e, FILE *out)
{
    cuex_meta_t meta = cuex_meta(e);
    cuex_oprinfo_t oi = cuex_oprinfo(meta);
    if (oi)
	fputs(cuex_oprinfo_name(oi), out);
    else if (cuex_meta_is_opr(meta))
	fprintf(out, "__opr%d/%d",
		(unsigned int)cuex_opr_index(meta),
	       	(unsigned int)cuex_opr_r(meta));
    else if (cuex_meta_is_type(meta)) {
	cudyn_type_t type = cudyn_type_from_meta(meta);
	cu_fprintf(out, "%!", type);
    }
    else
	fprintf(out, "__meta_0x%lx", (unsigned long)meta);
}

unsigned long
cudyn_castget_ulong(cuex_t e)
{
    cuex_meta_t meta = cuex_meta(e);
    if (cuex_meta_is_type(meta)) {
	switch (cudyn_type_typekind(cudyn_type_from_meta(meta))) {
	    case cudyn_typekind_elmtype_uint64:
	    case cudyn_typekind_elmtype_int64:
		return cudyn_to_uint64(e);
	    case cudyn_typekind_elmtype_uint32:
	    case cudyn_typekind_elmtype_int32:
		return cudyn_to_uint32(e);
	    case cudyn_typekind_elmtype_uint16:
	    case cudyn_typekind_elmtype_int16:
		return cudyn_to_uint16(e);
	    case cudyn_typekind_elmtype_uint8:
	    case cudyn_typekind_elmtype_int8:
		return cudyn_to_uint8(e);
	    default:
		break;
	}
    }
    cu_bugf("Could not cast %! to unsigned long", e);
}

cuex_t
cudyn_load(cudyn_type_t t, void *p)
{
    cudyn_typekind_t typekind = cudyn_type_typekind(t);
    switch (typekind) {
	case cudyn_typekind_stdtype:
	case cudyn_typekind_stdtypeoftypes:
	case cudyn_typekind_tvar:
	    cu_debug_assert(cuex_meta(p) == cudyn_type_to_meta(t));
	    return p;
	case cudyn_typekind_ptrtype:
	    return cudyn_ptr(cudyn_ptrtype_from_type(t), p);
	case cudyn_typekind_sngtype:
	    return cudyn_singular_obj();

	case cudyn_typekind_elmtype_bool:
	    return cudyn_bool(*(cu_bool_t *)p);
	case cudyn_typekind_elmtype_uint8:
	    return cudyn_uint8(*(uint8_t *)p);
	case cudyn_typekind_elmtype_int8:
	    return cudyn_int8(*(int8_t *)p);
	case cudyn_typekind_elmtype_uint16:
	    return cudyn_uint16(*(uint16_t *)p);
	case cudyn_typekind_elmtype_int16:
	    return cudyn_int16(*(int16_t *)p);
	case cudyn_typekind_elmtype_uint32:
	    return cudyn_uint32(*(uint32_t *)p);
	case cudyn_typekind_elmtype_int32:
	    return cudyn_int32(*(int32_t *)p);
	case cudyn_typekind_elmtype_uint64:
	    return cudyn_uint64(*(uint64_t *)p);
	case cudyn_typekind_elmtype_int64:
	    return cudyn_int64(*(int64_t *)p);
	case cudyn_typekind_elmtype_metaint:
	    return cudyn_metaint(*(cuex_meta_t *)p);
	case cudyn_typekind_elmtype_float:
	    return cudyn_float(*(float *)p);
	case cudyn_typekind_elmtype_double:
	    return cudyn_double(*(double *)p);
	case cudyn_typekind_elmtype_char:
	    return cudyn_char(*(char *)p);

	default:
	    /* none, proto, ctortype, arrtype, tuptype, sigtype, duntype,
	     * sngtype */
	    cu_bugf("Unimplemented");
	    return NULL;
    }
}

cuex_t cudynP_singular_obj;

void
cudynP_misc_init()
{
    CUDYN_ETYPEARR_INIT_PRINT(int8,   uint8_t,  SINT, &ffi_type_schar)
    CUDYN_ETYPEARR_INIT_PRINT(uint8,  uint8_t,  UINT, &ffi_type_uchar)
    CUDYN_ETYPEARR_INIT_PRINT(int16,  int16_t,  SINT, &ffi_type_sshort)
    CUDYN_ETYPEARR_INIT_PRINT(uint16, uint16_t, UINT, &ffi_type_ushort)
    CUDYN_ETYPEARR_INIT_PRINT(int32,  int32_t,  SINT, &ffi_type_sint)
    CUDYN_ETYPEARR_INIT_PRINT(uint32, uint32_t, UINT, &ffi_type_uint)
    CUDYN_ETYPEARR_INIT_PRINT(int64,  int64_t,  SINT, &ffi_type_slong)
    CUDYN_ETYPEARR_INIT_PRINT(uint64, uint64_t, UINT, &ffi_type_ulong)
    CUDYN_ETYPEARR_INIT_PRINT(float,  float,   FLOAT, &ffi_type_float)
    CUDYN_ETYPEARR_INIT_PRINT(double, double,  FLOAT, &ffi_type_double)

    CUDYN_ETYPE_INIT_PRINT(bool, int, BOOL, &ffi_type_sint)
    cudynP_true = cudyn_bool(1);
    cudynP_false = cudyn_bool(0);

    cudynP_singular_obj = cudyn_oalloc(cudyn_singular_type(), 0);

    CUDYN_ETYPEARR_INIT_PRINT(char, char, SINT, &ffi_type_schar)

    cudyn_prop_condset_ptr(cudyn_raw_print_fn_prop(), cu_str_type(),
			   cudynP_str_print);
    cudyn_prop_condset_ptr(cudyn_raw_print_fn_prop(), cu_idr_type(),
			   cudynP_idr_print);

    CUDYN_ETYPEARR_INIT_PRINT(metaint, cuex_meta_t, UINT, &ffi_type_ulong);
}
