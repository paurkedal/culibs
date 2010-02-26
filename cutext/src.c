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

#include <cu/conf.h>
#include <cutext/src.h>
#include <cu/memory.h>
#include <cu/int.h>
#include <cu/diag.h>
#include <iconv.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

size_t cutextP_buffer_limit = 1000000;

void
cutext_src_init(cutext_src_t src, cutext_producer_t producer)
{
    cu_buffer_init(&src->buf, 4096);
    src->produce = producer;
}

cutext_src_t
cutext_src_new(cutext_producer_t producer)
{
    cutext_src_t src = cu_gnew(struct cutext_src);
    cutext_src_init(src, producer);
    return src;
}

void
cutext_src_init_grab(cutext_src_t src_init, cutext_src_t src_drop)
{
    cu_buffer_init_drop(&src_init->buf, &src_drop->buf);
    src_init->produce = src_drop->produce;
    src_drop->produce = cu_clop_null;
}

cutext_src_t
cutext_src_new_grab(cutext_src_t src_drop)
{
    cutext_src_t src = cu_gnew(struct cutext_src);
    cutext_src_init_grab(src, src_drop);
    return src;
}

cutext_status_t
cutextP_src_lookahead(cutext_src_t ibuf, size_t size)
{
    cutext_status_t r;
    size_t multiplier = 1;
rec:
    cu_buffer_extend_freecap(&ibuf->buf, size*multiplier);
    r = cu_call(ibuf->produce,
		&ibuf->buf.content_end,
		cu_ptr_diff(ibuf->buf.storage_end, ibuf->buf.content_end));
    if (r == cutext_status_buffer_too_small &&
	&ibuf->buf.content_end - &ibuf->buf.content_start < size) {
	if (multiplier > 1 && multiplier*size > cutextP_buffer_limit) {
	    cu_errf("Buffer limit exceeded.");
	    abort();
	}
	multiplier *= 2;
	goto rec;
    }
    return r;
}

/* Producers
 * ========= */

cu_clos_def(_charr_producer,
	    cu_prot(cutext_status_t, void **p, size_t size),
	    ( char const *arr;
	      size_t size; ))
{
    cu_clos_self(_charr_producer);
    if (self->size == 0)
	return cutext_status_eos;
    if (size > self->size)
	size = self->size;
    memcpy(*p, self->arr, size);
    *p = cu_ptr_add(*p, size);
    self->arr += size;
    self->size -= size;
    return cutext_status_ok;
}
cutext_producer_t
cutext_producer_new_charr(char const *arr, size_t size)
{
    _charr_producer_t *cb = cu_gnew(_charr_producer_t);
    cb->arr = arr;
    cb->size = size;
    return _charr_producer_prep(cb);
}

cu_clos_def(_read_producer,
	    cu_prot(cutext_status_t, void **p, size_t size),
	    (int fd;))
{
    ssize_t size_read;
    cu_clos_self(_read_producer);
    while (size > 0) {
	size_read = read(self->fd, *p, size);
	if (size_read == -1)
	    return cutext_status_error;
	else if (size_read == 0)
	    return cutext_status_eos;
	*p = cu_ptr_add(*p, size_read);
	size -= size_read;
    }
    return cutext_status_ok;
}
cutext_producer_t
cutext_producer_new_read(int fd)
{
    _read_producer_t *cb = cu_gnew(_read_producer_t);
    cb->fd = fd;
    return _read_producer_prep(cb);
}


#define ICONV_BLOCK_SIZE 1024
cu_clos_def(_producer_iconv,
	    cu_prot(cutext_status_t, void **p, size_t size),
	    ( cutext_src_t src;
	      iconv_t cd; ))
{
    cu_clos_self(_producer_iconv);
    cutext_status_t status = cutext_status_ok;
    while (size > 0) {
	size_t avail;
	char *iconv_inbuf, *iconv_outbuf;
	cutext_src_lookahead(self->src, ICONV_BLOCK_SIZE);
	avail = cutext_src_data_size(self->src);
	if (avail == 0)
	    return cutext_status_eos;
	iconv_inbuf = self->src->buf.content_start;
	iconv_outbuf = *p;
	if (iconv(self->cd, &iconv_inbuf, &avail, &iconv_outbuf, &size)
	    == (size_t)-1) {
	    switch (errno) {
		case EILSEQ:
		    cu_errf("Invalid multibyte sequence."); /* XX */
		    return cutext_status_error;
		case EINVAL:
		    break;
		case E2BIG:
		    cu_debug_assert(size == 0);
		    break;
		default:
		    cu_debug_unreachable();
	    }
	}
	self->src->buf.content_start = iconv_inbuf;
	*p = iconv_outbuf;
    }
    return status;
}

static void
_producer_iconv_cleanup(void *obj, void *cd)
{
    iconv_close(((_producer_iconv_t *)obj)->cd);
}

cutext_producer_t
cutext_producer_new_iconv(cutext_src_t src,
			  cutext_encoding_t src_chenc,
			  cutext_encoding_t self_chenc)
{
    _producer_iconv_t *cb = cu_gnew(_producer_iconv_t);
    cb->src = src;
    cb->cd = iconv_open(cutext_encoding_name(self_chenc),
			cutext_encoding_name(src_chenc));
    if (cb->cd == NULL) {
	cu_errf("Could not initiate UTF-8 to UCS-4 conversion: %s",
		 strerror(errno));
	abort();
    }
    cu_gc_register_finaliser(cb, _producer_iconv_cleanup, NULL, NULL, NULL);
    return _producer_iconv_prep(cb);
}

char const *
cutext_encoding_name(cutext_encoding_t chenc)
{
    char const *name = cutextP_encoding_name(chenc);
    cu_debug_assert(name);
    return name;
}

cutext_encoding_t
cutext_src_detect_chenc(cutext_src_t src)
{
    size_t n;
    unsigned char *s;
    cutext_src_lookahead(src, 4);
    n = cutext_src_data_size(src);
    s = cutext_src_data_start(src);
    if (n == 0)
	return CUTEXT_ENCODING_NONE;
    else if (n == 1 || n == 3)
	return CUTEXT_ENCODING_UTF8;
    else if (n == 2) {
	if ((s[0] == 0xfe && s[1] == 0xff) || s[0] == 0)
	    return CUTEXT_ENCODING_UTF16;
	if ((s[0] == 0xff && s[1] == 0xfe) || s[1] == 0)
	    return CUTEXT_ENCODING_UTF16LE;
	return CUTEXT_ENCODING_UTF8;
    }

    /* Detect by byte order mark 0x0000feff. */

    /* UTF-8 */
    if (s[0] == 0xef && s[1] == 0xbb && s[2] == 0xbf)
	return CUTEXT_ENCODING_UTF8;
    /* Network byte order */
    if (s[0] == 0xfe && s[1] == 0xff)
	return CUTEXT_ENCODING_UTF16;
    if (s[0] == 0x00 && s[1] == 0x00) {
	if (s[2] == 0xfe && s[3] == 0xff)
	    return CUTEXT_ENCODING_UCS4;
    }
    /* Little endian */
    if (s[0] == 0xff && s[1] == 0xfe) {
	if (s[2] == 0x00 && s[3] == 0x00)
	    return CUTEXT_ENCODING_UCS4LE;
	return CUTEXT_ENCODING_UTF16LE;
    }

    /* Detect by assuming the first character is ASCII */

    /* Network byte order */
    if (s[0] == 0) { /* UTF-16, UCS-2, UCS-4 */
	if (s[1] == 0)
	    return CUTEXT_ENCODING_UCS4;
	return CUTEXT_ENCODING_UTF16;
    }
    /* Little endian */
    if (s[2] == 0 && s[3] == 0)
	return CUTEXT_ENCODING_UCS4LE;
    if (s[1] == 0)
	return CUTEXT_ENCODING_UTF16LE;
    if (s[2] != 0)
	return CUTEXT_ENCODING_UTF8;
    return CUTEXT_ENCODING_UNKNOWN;
}
