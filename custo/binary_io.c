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

/* BINARY IO
 *
 * Size-independent encoding of integers.
 *
 * Encoding 0.  Not in use.
 *
 *     unsigned_integer ::= (1xxxxxxx)* 0xxxxxxx
 *
 *     signed_integer ::= 0sxxxxxx
 *			| 1sxxxxxx (1xxxxxxx)* 0xxxxxxx
 *
 * Where 'x's are the bits forming the value, starting with the most
 * significant bit, and padding with zeroes on the left if necessary,
 * and 's = 0' for non-negative or 's = 1' for negative.
 *
 * Encoding 1.  This encoding moves all length-bits to the first
 * byte(s), and thus makes it possible to read the rest of bytes in
 * one operation.  That is, most reads can be done with two calls to
 * fread, since most numbers are less than 2^32.
 *
 *     unsigned_integer ::= (llllllll)* l...lx...x (xxxxxxxx)*
 *     signed_integer	::= (slllllll)* l...lx...x (xxxxxxxx)*
 *			  | sl...lx...x (xxxxxxxx)*
 *
 * where 'l = 0, 10, 110, 1110, ...' encodes the number of bytes in
 * the last '(xxxxxxxx)*'-sequence, 'x' are the bits from the value
 * padded on the left with zeroes if necessary, and 's = 0' for
 * non-negative and 's = 1' for negative numbers.
 */

#include <custo/binary_io.h>
#include <cu/str.h>
#include <cu/memory.h>
#include <assert.h>


/* Logsize Integer Encoding
 * ------------------------ */

#if 0 /* Encoding 0.  Not in use. */

cu_bool_fast_t
custo_fwrite_uintmax(uintmax_t n, FILE *out)
{
#define MAX_CODED_SIZE (sizeof(uintmax_t)*8/7 + 1)
    unsigned char buf[MAX_CODED_SIZE];
    size_t k = MAX_CODED_SIZE - 1;
    buf[MAX_CODED_SIZE - 1] = n & 0x7f;
    n >>= 7;
    while (n) {
	buf[--k] = 0x80 + (n & 0x7f);
	n >>= 7;
    }
    return fwrite(buf + k, MAX_CODED_SIZE - k, 1, out) == 1;
#undef MAX_CODED_SIZE
}

cu_bool_fast_t
custo_fread_uintmax(uintmax_t *pn, FILE *in)
{
    uintmax_t n = 0;
    unsigned char ch;
    do {
	if (fread(&ch, 1, 1, in) != 1)
	    return cu_false;
	n <<= 7;
	n += ch & 0x7f;
    } while (ch & 0x80);
    *pn = n;
    return cu_true;
}

cu_bool_fast_t
custo_fwrite_intmax(intmax_t sn, FILE *out)
{
#define MAX_CODED_SIZE (sizeof(uintmax_t)*8/7 + 2)
    unsigned char buf[MAX_CODED_SIZE];
    size_t k = MAX_CODED_SIZE - 1;
    uintmax_t n;
    if (sn < 0)
	n = -sn;
    else
	n = sn;
    buf[MAX_CODED_SIZE - 1] = n & 0x7f;
    n >>= 7;
    while (n) {
	buf[--k] = 0x80 + (n & 0x7f);
	n >>= 7;
    }
    if (buf[k] & 0x40)
	buf[--k] = 0x80;
    if (sn < 0)
	buf[k] |= 0x40;
    return fwrite(buf + k, MAX_CODED_SIZE - k, 1, out) == 1;
#undef MAX_CODED_SIZE
}

cu_bool_fast_t
custo_fread_intmax(intmax_t *pn, FILE *in)
{
    uintmax_t n;
    unsigned char ch0, ch;
    if (fread(&ch0, 1, 1, in) != 1)
	return cu_false;
    n = ch0 & 0x3f;
    if (ch0 & 0x80)
	do {
	    if (fread(&ch, 1, 1, in) != 1)
		return cu_false;
	    n <<= 7;
	    n |= ch & 0x7f;
	} while (ch & 0x80);
    if (ch0 & 0x40)
	*pn = -n;
    else
	*pn = n;
    return cu_true;
}

#else /* Encoding 1. */

cu_bool_fast_t
custo_fwrite_uintmax(uintmax_t n, FILE *out)
{
#define BUF_SIZE (sizeof(uintmax_t) + sizeof(uintmax_t)/8 + 1)
    unsigned char buf[BUF_SIZE];
    size_t k = BUF_SIZE;
    size_t l;
    unsigned char m;
    do {
	buf[--k] = n & 0xff;
	n >>= 8;
    } while (n);
    l = (BUF_SIZE - 1 - k) % 8;
    m = ~(0xff >> (l + 1));
    if (buf[k] & m) {
	--k;
	l = (BUF_SIZE - 1 - k) % 8;
	buf[k] = ~(0xff >> l);
    }
    else
	buf[k] |= m << 1;
    l = (BUF_SIZE - 1 - k)/8;
    while (l) {
	buf[--k] = 0xff;
	--l;
    }
    return fwrite(buf + k, BUF_SIZE - k, 1, out) == 1;
#undef BUF_SIZE
}

cu_bool_fast_t
custo_fread_uintmax(uintmax_t *pn, FILE *in)
{
#define BUF_SIZE sizeof(uintmax_t)
    unsigned char buf[BUF_SIZE];
    size_t i, l = 0, m;
    uintmax_t n;
    for (;;) {
	if (fread(buf, 1, 1, in) != 1)
	    return cu_false;
	if (buf[0] == 0xff)
	    l += 8;
	else
	    break;
    }
    m = buf[0];
    i = 0;
    while (m & 0x80) {
	++i;
	m <<= 1;
    }
    l += i;

    n = buf[0] & (0xff >> i);
    if (l) {
	if (l > BUF_SIZE || (l == BUF_SIZE && n != 0)) /* overflow */
	    return cu_false;
	if (fread(buf, l, 1, in) != 1)
	    return cu_false;
	for (i = 0; i < l; ++i) {
	    n <<= 8;
	    n += buf[i];
	}
    }
    *pn = n;
    return cu_true;
#undef BUF_SIZE
}

cu_bool_fast_t
custo_fwrite_intmax(intmax_t ns, FILE *out)
{
#define BUF_SIZE (sizeof(uintmax_t) + sizeof(uintmax_t)/8 + 1)
    unsigned char buf[BUF_SIZE];
    size_t k = BUF_SIZE;
    size_t l;
    unsigned char m;
    uintmax_t n = ns < 0? -ns : ns;
    do {
	buf[--k] = n & 0xff;
	n >>= 8;
    } while (n);
    l = (BUF_SIZE - k) % 8;
    m = ~(0xff >> (l + 1));
    if (buf[k] & m) {
	--k;
	l = (BUF_SIZE - k) % 8;
	buf[k] = ~(0xff >> l);
    }
    else
	buf[k] |= m << 1;
    l = (BUF_SIZE - k)/8;
    while (l) {
	buf[--k] = 0xff;
	--l;
    }
    if (ns >= 0)
	buf[k] &= 0x7f;
    return fwrite(buf + k, BUF_SIZE - k, 1, out) == 1;
#undef BUF_SIZE
}

cu_bool_fast_t
custo_fread_intmax(intmax_t *pn, FILE *in)
{
#define BUF_SIZE sizeof(uintmax_t)
    unsigned char ch0;
    unsigned char buf[BUF_SIZE];
    size_t i, l = 0, m;
    uintmax_t n;
    if (fread(&ch0, 1, 1, in) != 1)
	return cu_false;
    if ((ch0 & 0x7f) == 0x7f) {
	l += 7;
	for (;;) {
	    if (fread(buf, 1, 1, in) != 1)
		return cu_false;
	    if (buf[0] == 0xff)
		l += 8;
	    else
		break;
	}
	m = buf[0];
	i = 0;
	while (m & 0x80) {
	    ++i;
	    m <<= 1;
	}
	l += i;
	n = buf[0] & (0xff >> i);
    }
    else {
	m = ch0;
	i = 0;
	while (m & 0x40) {
	    ++i;
	    m <<= 1;
	}
	l += i;
	n = ch0 & (0x7f >> i);
    }
    if (l) {
	if (l > BUF_SIZE || (l == BUF_SIZE && n != 0))
	    return cu_false; /* overflow */
	if (fread(buf, l, 1, in) != 1)
	    return cu_false;
	for (i = 0; i < l; ++i) {
	    n <<= 8;
	    n += buf[i];
	}
    }
    if (n > INTMAX_MAX)
	return cu_false; /* overflow */
    *pn = (ch0 & 0x80)? -n : n;
    return cu_true;
#undef BUF_SIZE
}

#endif


/* Strings
 * ------- */

cu_bool_fast_t
custo_fread_str_construct(cu_str_t str, FILE *file)
{
    uintmax_t n;
    if (!custo_fread_uintmax(&n, file))
	return cu_false;
    cu_str_cct_uninit(str, n);
    if (fread(str->arr, n, 1, file) != n)
	return cu_false;
    return cu_true;
}

cu_str_t
custo_fread_str_new(FILE *file)
{
    cu_str_t str = cu_gnew(struct cu_str_s);
    custo_fread_str_construct(str, file);
    return str;
}

cu_bool_fast_t
custo_fwrite_str(cu_str_t str, FILE *file)
{
    custo_fwrite_uintmax(str->len, file);
    if (fwrite(str->arr, 1, str->len, file) != str->len)
	return cu_false;
    return cu_true;
}
