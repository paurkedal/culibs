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

#include <cu/tstate.h>
#include <cutext/ucs4.h>
#include <cu/memory.h>
#include <cu/conf.h>
#include <string.h>
#include <errno.h>
#include <iconv.h>

#define REOPEN_ICONV 1
#ifdef CUCONF_WORDS_BIGENDIAN
#  define UCS4HOST "UCS-4"
#else
#  define UCS4HOST "UCS-4LE"
#endif

int
cutext_charr_to_ucs4arr(char const **src_arr, size_t *src_cnt,
		     cutext_ucs4char_t **dst_arr, size_t *dst_cap_scaled)
{
    size_t dst_cap = *dst_cap_scaled*4;
#ifdef REOPEN_ICONV
    iconv_t cd = iconv_open(UCS4HOST, "UTF-8");
    if (!cd) {
	cu_errf("Could not initiate UCS-4 to UTF-8 conversion: %s",
		 strerror(errno));
	abort();
    }
    if (iconv(cd, (void *)src_arr, src_cnt, (void *)dst_arr, &dst_cap)
	== (size_t)-1
	|| iconv(cd, NULL, NULL, (void *)dst_arr, &dst_cap)
	== (size_t)-1) {
	iconv_close(cd);
	return errno;
    }
    *dst_cap_scaled = dst_cap/4;
    return 0;
#else
    cutext_ucs4char_t *dst_beg;
    cuP_tstate_t tstate = cuP_tstate();
    if (!tstate->iconv_utf8_to_ucs4) {
	tstate->iconv_utf8_to_ucs4 = iconv_open(UCS4HOST, "UTF-8");
	if (!tstate->iconv_utf8_to_ucs4) {
	    cu_errf("Could not initiate UCS-4 to UTF-8 conversion: %s",
		     strerror(errno));
	    abort();
	}
    }
    if (iconv(tstate->iconv_utf8_to_ucs4,
	      (char **)src_arr, src_cnt, (char **)dst_arr, &dst_cap)
	== (size_t)-1)
	goto fail;
    if (iconv(tstate->iconv_utf8_to_ucs4,
	      NULL, NULL, (char **)dst_arr, &dst_cap)
	== (size_t)-1)
	goto fail;
    *dst_cap_scaled = dst_cap/4;
    return 0;
fail:
    iconv(tstate->iconv_utf8_to_ucs4, NULL, NULL, NULL, NULL);
    return errno;
#endif
}

int
cutext_ucs4arr_to_charr(cutext_ucs4char_t const **src_arr, size_t *src_cnt_sc,
		     char **dst_arr, size_t *dst_cnt)
{
    size_t src_cnt = *src_cnt_sc*sizeof(cutext_ucs4char_t);
#ifdef REOPEN_ICONV
    iconv_t cd = iconv_open("UTF-8", UCS4HOST);
    if (!cd) {
	cu_errf("Could not initiate UTF-8 to UCS-4 conversion: %s",
		 strerror(errno));
	abort();
    }
    if (iconv(cd, (void *)src_arr, &src_cnt, dst_arr, dst_cnt)
	== (size_t)-1
	|| iconv(cd, NULL, NULL, dst_arr, dst_cnt)
	== (size_t)-1) {
	iconv_close(cd);
	return errno;
    }
    iconv_close(cd);
    *src_cnt_sc = src_cnt/sizeof(cutext_ucs4char_t);
    return 0;
#else
    cuP_tstate_t tstate = cuP_tstate();
    if (!tstate->iconv_ucs4_to_utf8) {
	tstate->iconv_ucs4_to_utf8 = iconv_open("UTF-8", UCS4HOST);
	if (!tstate->iconv_ucs4_to_utf8) {
	    cu_errf("Could not initiate UTF-8 to UCS-4 conversion: %s",
		     strerror(errno));
	    abort();
	}
    }
    if (iconv(tstate->iconv_ucs4_to_utf8,
	      (char **)src_arr, &src_cnt, dst_arr, dst_cnt) == (size_t)-1)
	goto fail;
    if (iconv(tstate->iconv_ucs4_to_utf8,
	      NULL, NULL, dst_arr, dst_cnt) == (size_t)-1)
	goto fail;
    *src_cnt_sc = src_cnt/sizeof(cutext_ucs4char_t);
    return 0;
fail:
    iconv(tstate->iconv_ucs4_to_utf8, NULL, NULL, NULL, NULL);
    return errno;
#endif
}

int
cutext_ucs4char_to_charr(cutext_ucs4char_t ucs4ch, char **dst_arr, size_t *dst_cnt)
{
    size_t ucs4cnt = 1;
    cutext_ucs4char_t const *ucs4arr = &ucs4ch;
    return cutext_ucs4arr_to_charr(&ucs4arr, &ucs4cnt, dst_arr, dst_cnt);
}
