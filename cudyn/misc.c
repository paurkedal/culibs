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
#include <cuoo/intf.h>
#include <cuoo/properties.h>
#include <cu/idr.h>
#include <cu/str.h>
#include <cufo/stream.h>
#include <cuex/oprinfo.h>	/* FIXME: remove dep */
#include <cuex/ex.h>


/* Elementary types. */

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

static cu_box_t
cudynP_char_impl(cu_word_t intf, ...)
{
    switch (intf) {
	case CUOO_INTF_PRINT_FN:
	    return CUOO_INTF_PRINT_FN_BOX(cudynP_char_print);
	case CUOO_INTF_FOPRINT_FN:
	    return CUOO_INTF_FOPRINT_FN_BOX(cudynP_char_foprint);
	default:
	    return CUOO_IMPL_NONE;
    }
}
static cu_box_t
cudynP_uint8_impl(cu_word_t intf, ...)
{
    switch (intf) {
	case CUOO_INTF_PRINT_FN:
	    return CUOO_INTF_PRINT_FN_BOX(cudynP_uint8_print);
	case CUOO_INTF_FOPRINT_FN:
	    return CUOO_INTF_FOPRINT_FN_BOX(cudynP_uint8_foprint);
	default:
	    return CUOO_IMPL_NONE;
    }
}
static cu_box_t
cudynP_int8_impl(cu_word_t intf, ...)
{
    switch (intf) {
	case CUOO_INTF_PRINT_FN:
	    return CUOO_INTF_PRINT_FN_BOX(cudynP_int8_print);
	case CUOO_INTF_FOPRINT_FN:
	    return CUOO_INTF_FOPRINT_FN_BOX(cudynP_int8_foprint);
	default:
	    return CUOO_IMPL_NONE;
    }
}
static cu_box_t
cudynP_uint16_impl(cu_word_t intf, ...)
{
    switch (intf) {
	case CUOO_INTF_PRINT_FN:
	    return CUOO_INTF_PRINT_FN_BOX(cudynP_uint16_print);
	case CUOO_INTF_FOPRINT_FN:
	    return CUOO_INTF_FOPRINT_FN_BOX(cudynP_uint16_foprint);
	default:
	    return CUOO_IMPL_NONE;
    }
}
static cu_box_t
cudynP_int16_impl(cu_word_t intf, ...)
{
    switch (intf) {
	case CUOO_INTF_PRINT_FN:
	    return CUOO_INTF_PRINT_FN_BOX(cudynP_int16_print);
	case CUOO_INTF_FOPRINT_FN:
	    return CUOO_INTF_FOPRINT_FN_BOX(cudynP_int16_foprint);
	default:
	    return CUOO_IMPL_NONE;
    }
}
static cu_box_t
cudynP_uint32_impl(cu_word_t intf, ...)
{
    switch (intf) {
	case CUOO_INTF_PRINT_FN:
	    return CUOO_INTF_PRINT_FN_BOX(cudynP_uint32_print);
	case CUOO_INTF_FOPRINT_FN:
	    return CUOO_INTF_FOPRINT_FN_BOX(cudynP_uint32_foprint);
	default:
	    return CUOO_IMPL_NONE;
    }
}
static cu_box_t
cudynP_int32_impl(cu_word_t intf, ...)
{
    switch (intf) {
	case CUOO_INTF_PRINT_FN:
	    return CUOO_INTF_PRINT_FN_BOX(cudynP_int32_print);
	case CUOO_INTF_FOPRINT_FN:
	    return CUOO_INTF_FOPRINT_FN_BOX(cudynP_int32_foprint);
	default:
	    return CUOO_IMPL_NONE;
    }
}
static cu_box_t
cudynP_uint64_impl(cu_word_t intf, ...)
{
    switch (intf) {
	case CUOO_INTF_PRINT_FN:
	    return CUOO_INTF_PRINT_FN_BOX(cudynP_uint64_print);
	case CUOO_INTF_FOPRINT_FN:
	    return CUOO_INTF_FOPRINT_FN_BOX(cudynP_uint64_foprint);
	default:
	    return CUOO_IMPL_NONE;
    }
}
static cu_box_t
cudynP_int64_impl(cu_word_t intf, ...)
{
    switch (intf) {
	case CUOO_INTF_PRINT_FN:
	    return CUOO_INTF_PRINT_FN_BOX(cudynP_int64_print);
	case CUOO_INTF_FOPRINT_FN:
	    return CUOO_INTF_FOPRINT_FN_BOX(cudynP_int64_foprint);
	default:
	    return CUOO_IMPL_NONE;
    }
}

static cu_box_t
cudynP_float_impl(cu_word_t intf, ...)
{
    switch (intf) {
	case CUOO_INTF_PRINT_FN:
	    return CUOO_INTF_PRINT_FN_BOX(cudynP_float_print);
	case CUOO_INTF_FOPRINT_FN:
	    return CUOO_INTF_FOPRINT_FN_BOX(cudynP_float_foprint);
	default:
	    return CUOO_IMPL_NONE;
    }
}
static cu_box_t
cudynP_double_impl(cu_word_t intf, ...)
{
    switch (intf) {
	case CUOO_INTF_PRINT_FN:
	    return CUOO_INTF_PRINT_FN_BOX(cudynP_double_print);
	case CUOO_INTF_FOPRINT_FN:
	    return CUOO_INTF_FOPRINT_FN_BOX(cudynP_double_foprint);
	default:
	    return CUOO_IMPL_NONE;
    }
}


/* == unit == */

cudyn_elmtype_t cudynP_unit_type;
cuex_t cudynP_unit_value;

static cu_str_t
cudynP_unit_to_str(void *val)
{
    return cu_str_new_cstr("()");
}

static cu_box_t
cudynP_unit_impl(cu_word_t intf_number, ...)
{
    switch (intf_number) {
	case CUOO_INTF_TO_STR_FN:
	    return CUOO_INTF_TO_STR_FN_BOX(cudynP_unit_to_str);
	default:
	    return CUOO_IMPL_NONE;
    }
}


/* == cu_bool_t == */

cudyn_elmtype_t cudynP_bool_type;

struct cudynP_bool_aligntest { char ch; cu_bool_t x; };

static void
cudynP_bool_print(cuex_t e, FILE *out)
{
    fputs(cudyn_to_bool(e)? "true" : "false", out);
}

static void
cudynP_bool_foprint(cufo_stream_t fos, cufo_prispec_t spec, void *e)
{
    cufo_puts(fos, cudyn_to_bool(e)? "true" : "false");
}

static cu_box_t
cudynP_bool_impl(cu_word_t intf_number, ...)
{
    switch (intf_number) {
	case CUOO_INTF_PRINT_FN:
	    return CUOO_INTF_PRINT_FN_BOX(cudynP_bool_print);
	case CUOO_INTF_FOPRINT_FN:
	    return CUOO_INTF_FOPRINT_FN_BOX(cudynP_bool_foprint);
	default:
	    return CUOO_IMPL_NONE;
    }
}

cuex_t cudynP_true;
cuex_t cudynP_false;


/* == cuex_t and cuex_meta_t == */

CUDYN_ARRTYPE_DEFN(cuex, cuex_t)

CUDYN_ETYPEARR_DEFN(metaint, cuex_meta_t)

static void
cudynP_metaint_print(cuex_t e, FILE *out)
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
	cuoo_type_t type = cuoo_type_from_meta(meta);
	cu_fprintf(out, "%!", type);
    }
    else
	fprintf(out, "__meta_0x%lx", (unsigned long)meta);
}

static void
cudynP_metaint_foprint(cufo_stream_t fos, cufo_prispec_t spec, void *e)
{
    cuex_meta_t meta = cuex_meta(e);
    cuex_oprinfo_t oi = cuex_oprinfo(meta);
    if (oi)
	cufo_puts(fos, cuex_oprinfo_name(oi));
    else if (cuex_meta_is_opr(meta))
	cufo_printf(fos, "__opr%d/%d",
		    (unsigned int)cuex_opr_index(meta),
		    (unsigned int)cuex_opr_r(meta));
    else if (cuex_meta_is_type(meta)) {
	cuoo_type_t type = cuoo_type_from_meta(meta);
	cufo_printsp_ex(fos, spec, type);
    }
    else
	cufo_printf(fos, "__meta_0x%lx", (unsigned long)meta);
}

static cu_box_t
cudynP_metaint_impl(cu_word_t intf_number, ...)
{
    switch (intf_number) {
	case CUOO_INTF_PRINT_FN:
	    return CUOO_INTF_PRINT_FN_BOX(cudynP_metaint_print);
	case CUOO_INTF_FOPRINT_FN:
	    return CUOO_INTF_FOPRINT_FN_BOX(cudynP_metaint_foprint);
	default:
	    return CUOO_IMPL_NONE;
    }
}

unsigned long
cudyn_castget_ulong(cuex_t e)
{
    cuex_meta_t meta = cuex_meta(e);
    if (cuex_meta_is_type(meta)) {
	switch (cuoo_type_shape(cuoo_type_from_meta(meta))) {
	    case CUOO_SHAPE_SCALAR_UINT64:
	    case CUOO_SHAPE_SCALAR_INT64:
		return cudyn_to_uint64(e);
	    case CUOO_SHAPE_SCALAR_UINT32:
	    case CUOO_SHAPE_SCALAR_INT32:
		return cudyn_to_uint32(e);
	    case CUOO_SHAPE_SCALAR_UINT16:
	    case CUOO_SHAPE_SCALAR_INT16:
		return cudyn_to_uint16(e);
	    case CUOO_SHAPE_SCALAR_UINT8:
	    case CUOO_SHAPE_SCALAR_INT8:
		return cudyn_to_uint8(e);
	    default:
		break;
	}
    }
    cu_bugf("Could not cast %! to unsigned long", e);
}

cuex_t
cudyn_load(cuoo_type_t t, void *p)
{
    cuoo_shape_t shape = cuoo_type_shape(t);
    switch (shape) {
	case CUOO_SHAPE_OPAQUE:
	case CUOO_SHAPE_METATYPE:
	case CUOO_SHAPE_TVAR:
	    cu_debug_assert(cuex_meta(p) == cuoo_type_to_meta(t));
	    return p;
	case CUOO_SHAPE_PTRTYPE:
	    return cudyn_ptr(cudyn_ptrtype_from_type(t), p);

	case CUOO_SHAPE_UNIT:
	    return cudyn_unit_value();
	case CUOO_SHAPE_SCALAR_BOOL:
	    return cudyn_bool(*(cu_bool_t *)p);
	case CUOO_SHAPE_SCALAR_UINT8:
	    return cudyn_uint8(*(uint8_t *)p);
	case CUOO_SHAPE_SCALAR_INT8:
	    return cudyn_int8(*(int8_t *)p);
	case CUOO_SHAPE_SCALAR_UINT16:
	    return cudyn_uint16(*(uint16_t *)p);
	case CUOO_SHAPE_SCALAR_INT16:
	    return cudyn_int16(*(int16_t *)p);
	case CUOO_SHAPE_SCALAR_UINT32:
	    return cudyn_uint32(*(uint32_t *)p);
	case CUOO_SHAPE_SCALAR_INT32:
	    return cudyn_int32(*(int32_t *)p);
	case CUOO_SHAPE_SCALAR_UINT64:
	    return cudyn_uint64(*(uint64_t *)p);
	case CUOO_SHAPE_SCALAR_INT64:
	    return cudyn_int64(*(int64_t *)p);
	case CUOO_SHAPE_SCALAR_METAINT:
	    return cudyn_metaint(*(cuex_meta_t *)p);
	case CUOO_SHAPE_SCALAR_FLOAT:
	    return cudyn_float(*(float *)p);
	case CUOO_SHAPE_SCALAR_DOUBLE:
	    return cudyn_double(*(double *)p);
	case CUOO_SHAPE_SCALAR_CHAR:
	    return cudyn_char(*(char *)p);

	default:
	    /* none, proto, ctortype, arrtype, tuptype, sigtype, duntype,
	     * sngtype */
	    return NULL;
    }
}

void
cudynP_misc_init()
{
    /* Integers */
    CUDYN_ETYPEARR_INIT(int8,   INT8,	uint8_t,  SINT, &ffi_type_schar)
    CUDYN_ETYPEARR_INIT(uint8,  UINT8,	uint8_t,  UINT, &ffi_type_uchar)
    CUDYN_ETYPEARR_INIT(int16,  INT16,	int16_t,  SINT, &ffi_type_sshort)
    CUDYN_ETYPEARR_INIT(uint16, UINT16,	uint16_t, UINT, &ffi_type_ushort)
    CUDYN_ETYPEARR_INIT(int32,  INT32,	int32_t,  SINT, &ffi_type_sint)
    CUDYN_ETYPEARR_INIT(uint32, UINT32,	uint32_t, UINT, &ffi_type_uint)
    CUDYN_ETYPEARR_INIT(int64,  INT64,	int64_t,  SINT, &ffi_type_slong)
    CUDYN_ETYPEARR_INIT(uint64, UINT64,	uint64_t, UINT, &ffi_type_ulong)

    /* Floats */
    CUDYN_ETYPEARR_INIT(float,  FLOAT,	float,   FLOAT, &ffi_type_float)
    CUDYN_ETYPEARR_INIT(double, DOUBLE,	double,  FLOAT, &ffi_type_double)

    /* Unit */
    cudynP_unit_type = cudyn_elmtype_new(CUOO_SHAPE_UNIT, cudynP_unit_impl,
					 0, 0 CUDYN_LIBFFI_ARG(&ffi_type_void));
    cuoo_prop_define_ptr(cuoo_raw_c_name_prop(), cudynP_unit_type,
			 cu_idr_by_cstr("void"));
    cudynP_unit_value = cuoo_oalloc(cudyn_unit_type(), 0);
	/* OBS, keys size 0 indicates no hashcons support in type. */

    /* Bool */
    CUDYN_ETYPE_INIT(bool, BOOL, int, BOOL, &ffi_type_sint)
    cudynP_true = cudyn_bool(1);
    cudynP_false = cudyn_bool(0);

    /* Misc */
    CUDYN_ETYPEARR_INIT(char, CHAR, char, SINT, &ffi_type_schar)
    CUDYN_ETYPEARR_INIT(metaint, METAINT, cuex_meta_t, UINT, &ffi_type_ulong);
}
