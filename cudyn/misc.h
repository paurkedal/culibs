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

#ifndef CUDYN_MISC_H
#define CUDYN_MISC_H

#include <cudyn/fwd.h>
#include <cudyn/properties.h>
#include <cudyn/typetpl.h>
#include <cudyn/type.h>
#include <cu/idr.h>
#include <stdlib.h>
#include <stdint.h>

CU_BEGIN_DECLARATIONS
/*!\addtogroup cudyn_misc_h
 * @{ */

/* Integer Types
 * -------------
 *
 * There are two naming schemes for the types depending on your view.  The
 * size-based scheme comprises the distinct types
 *
 *      int8,  int16,  int32,  int64  -- integers
 *     uint8, uint16, uint32, uint64  -- unsigned integers
 *
 * and the aliases int, uint for the default integer on the system
 * which is typically int32, uint32 or int64, uint64.
 *
 * The C style scheme is defined below. */

CUDYN_ETYPEARR_DCLN(  int8,   int8_t)
CUDYN_ETYPEARR_DCLN( uint8,  uint8_t)
CUDYN_ETYPEARR_DCLN( int16,  int16_t)
CUDYN_ETYPEARR_DCLN(uint16, uint16_t)
CUDYN_ETYPEARR_DCLN( int32,  int32_t)
CUDYN_ETYPEARR_DCLN(uint32, uint32_t)
CUDYN_ETYPEARR_DCLN( int64,  int64_t)
CUDYN_ETYPEARR_DCLN(uint64, uint64_t)


/* Other Elementary Types
 * ---------------------- */

/*!\addtogroup cudyn_bool_mod
 * @{ */

extern cudyn_elmtype_t cudynP_bool_type;

CU_SINLINE cudyn_type_t cudyn_bool_type()
{ return cudyn_elmtype_to_type(cudynP_bool_type); }

CU_SINLINE cuex_t cudyn_bool(cu_bool_t x)
{
    x = !!x;
    return cudyn_halloc(cudyn_bool_type(), sizeof(cu_word_t), &x);
}

CU_SINLINE cu_bool_t cudyn_is_bool(cuex_t ex)
{ return cuex_meta(ex) == cudyn_type_to_meta(cudyn_bool_type()); }

CU_SINLINE cu_bool_t cudyn_to_bool(cuex_t ex)
{ return *(cu_word_t *)((char *)ex + CU_HCOBJ_SHIFT); }

extern cuex_t cudynP_true;
extern cuex_t cudynP_false;

/*!The dynamic boolean true value. */
#define cudyn_true() cudynP_true
/*!The dynamic boolean false value. */
#define cudyn_false() cudynP_false

/*!@}*/

CUDYN_ETYPEARR_DCLN(float, float)
CUDYN_ETYPEARR_DCLN(double, double)

/* This is meant for charaters. */
CUDYN_ETYPEARR_DCLN(char, char)

CUDYN_ARRTYPE_DCLN(cuex, cuex_t)
CUDYN_ETYPEARR_DCLN(metaint, cuex_meta_t)


/* C Style Integer Types
 * ---------------------
 *
 * The C-style scheme comprises the types
 *
 *     int8 (signed char), uint8 (unsigned char),
 *     short, ushort, int, uint, long, ulong
 *
 * which are not necessarily distinct. */

CUDYN_ETYPEARR_ALIAS(uchar, unsigned char, uint8)

#if CUCONF_SIZEOF_SHORT == 2
CUDYN_ETYPEARR_ALIAS(short, short, int16)
CUDYN_ETYPEARR_ALIAS(ushort, unsigned short, uint16)
#elif CUCONF_SIZEOF_SHORT == 4
CUDYN_ETYPEARR_ALIAS(short, short, int16)
CUDYN_ETYPEARR_ALIAS(ushort, unsigned short, uint16)
#else
#  error "Missing configuration macros or unexpected sizeof(short)"
#endif

#if CUCONF_SIZEOF_INT == 2
CUDYN_ETYPEARR_ALIAS(int, int, int16)
CUDYN_ETYPEARR_ALIAS(uint, unsigned int, uint16)
#elif CUCONF_SIZEOF_INT == 4
CUDYN_ETYPEARR_ALIAS(int, int, int32)
CUDYN_ETYPEARR_ALIAS(uint, unsigned int, uint32)
#elif CUCONF_SIZEOF_INT == 8
CUDYN_ETYPEARR_ALIAS(int, int, int64)
CUDYN_ETYPEARR_ALIAS(uint, unsigned int, uint64)
#else
#  error "Missing configuration macros or unexpected sizeof(int)"
#endif

#if CUCONF_SIZEOF_LONG == 2
CUDYN_ETYPEARR_ALIAS(long, long, int16)
CUDYN_ETYPEARR_ALIAS(ulong, unsigned long, uint16)
#elif CUCONF_SIZEOF_LONG == 4
CUDYN_ETYPEARR_ALIAS(long, long, int32)
CUDYN_ETYPEARR_ALIAS(ulong, unsigned long, uint32)
#elif CUCONF_SIZEOF_LONG == 8
CUDYN_ETYPEARR_ALIAS(long, long, int64)
CUDYN_ETYPEARR_ALIAS(ulong, unsigned long, uint64)
#else
#  error "Missing configuration macros or unexpected sizeof(long)"
#endif

CU_SINLINE cuex_t
cudyn_ptr(cudyn_ptrtype_t ptrtype, void *ptr)
{
    return cudyn_halloc(cudyn_ptrtype_to_type(ptrtype), sizeof(void *), &ptr);
}

CU_SINLINE void *
cudyn_to_ptr(cuex_t e)
{ return *(void **)((char *)e + CU_HCOBJ_SHIFT); }


extern cuex_t cudynP_singular_obj;

CU_SINLINE cuex_t cudyn_singular_obj(void)
{ return cudynP_singular_obj; }

CU_SINLINE cu_bool_t cudyn_is_singular_obj(cuex_t obj)
{ return obj == cudynP_singular_obj; }


/* Functions which applies to many types
 * ===================================== */

unsigned long cudyn_castget_ulong(cuex_t e);

cuex_t cudyn_load(cudyn_type_t t, void *ptr);

/*!@}*/
CU_END_DECLARATIONS

#endif
