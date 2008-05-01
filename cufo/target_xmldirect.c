/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2008  Petter Urkedal <urkedal@nbi.dk>
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

#include <cufo/stream.h>
#include <cufo/tag.h>
#include <cufo/attr.h>
#include <cu/dsink.h>
#include <cu/memory.h>
#include <cu/buffer.h>

typedef struct xd_stream_s *xd_stream_t;
struct xd_stream_s
{
    cu_inherit (cufo_stream_s);
    cu_dsink_t sink;
};

#define SX_STREAM(os) cu_from(xd_stream, cufo_stream, os)

static size_t
xd_write(cufo_stream_t os, void const *data, size_t size)
{
    size_t wz;
    char const *s_frag_start = data;
    char const *s_cur = data;
    char const *s_end = (char const *)data + size;
    while (s_cur < s_end) {
	char const *repl;
	size_t repl_len;
	switch (*s_cur) {
	    case '<': repl = "&lt;";  repl_len = 4; break;
	    case '>': repl = "&gt;";  repl_len = 4; break;
	    case '&': repl = "&amp;"; repl_len = 5; break;
	    default: repl = NULL; break;
	}
	if (repl) {
	    size_t frag_size = s_cur - s_frag_start;
	    wz = cu_dsink_write(SX_STREAM(os)->sink, s_frag_start, frag_size);
	    if (wz != frag_size) {
		if (wz == (size_t)-1)
		    return (size_t)-1;
		else
		    cu_bugf("Sink should but did not consume all data.");
	    }
	    s_frag_start = ++s_cur;

	    wz = cu_dsink_write(SX_STREAM(os)->sink, repl, repl_len);
	    if (wz != repl_len) {
		if (wz == (size_t)-1)
		    return (size_t)-1;
		else
		    cu_bugf("Sink should but did not consume all data.");
	    }
	}
	else
	    ++s_cur;
    }
    if (s_frag_start < s_end) {
	size_t frag_size = s_end - s_frag_start;
	wz = cu_dsink_write(SX_STREAM(os)->sink, s_frag_start, frag_size);
	if (wz == (size_t)-1)
	    return (size_t)-1;
	return size + wz - frag_size;
    }
    else
	return size;
}

static void
xd_raw_write(cufo_stream_t os, char const *buf, size_t len)
{
    size_t wz;
    wz = cu_dsink_write(SX_STREAM(os)->sink, buf, len);
    if (wz == (size_t)-1)
	cufo_flag_error(os);
    else if (wz != len)
	cu_bugf("Sink should but did not consume all raw data.");
}

static void
xd_enter(cufo_stream_t os, cufo_tag_t tag, va_list va)
{
    char const *name;
    size_t name_len;
    cufo_attr_t attr;
    struct cu_buffer_s buf;

    cufoP_flush(os, cu_true);

    name = cufo_tag_name(tag);
    name_len = strlen(name);
    cu_buffer_init(&buf, name_len + 2);
    cu_buffer_write(&buf, "<", 1);
    cu_buffer_write(&buf, name, name_len);

    while ((attr = va_arg(va, cufo_attr_t))) {
	name = cufo_attr_name(attr);
	name_len = strlen(name);
	cu_buffer_write(&buf, " ", 1);
	cu_buffer_write(&buf, name, name_len);
	cu_buffer_write(&buf, "=", 1);
	switch (cufo_attr_type(attr)) {
		char const *cs;
		char *s;
		int i, n;
	    case cufo_attrtype_fixed:
		cs = attr->extra.fixed_value;
		cu_buffer_write(&buf, "\"", 1);
		cu_buffer_write(&buf, cs, strlen(cs));
		cu_buffer_write(&buf, "\"", 1);
		break;
	    case cufo_attrtype_cstr:
		/* TODO: Escape string. */
		cs = va_arg(va, char const *);
		cu_buffer_write(&buf, "\"", 1);
		cu_buffer_write(&buf, cs, strlen(cs));
		cu_buffer_write(&buf, "\"", 1);
		break;
	    case cufo_attrtype_int:
		i = va_arg(va, int);
		cu_buffer_extend_freecap(&buf, sizeof(int)*3 + 3);
		s = cu_buffer_content_end(&buf);
		sprintf(s, "\"%d\"%n", i, &n);
		cu_buffer_incr_content_end(&buf, n);
		break;
	    case cufo_attrtype_enum:
		i = va_arg(va, int);
		cs = (*attr->extra.enum_name)(i);
		cu_buffer_write(&buf, "\"", 1);
		cu_buffer_write(&buf, cs, strlen(cs));
		cu_buffer_write(&buf, "\"", 1);
		break;
	    default:
		cu_bug_unfinished();
	}
    }
    cu_buffer_write(&buf, ">", 1);
    xd_raw_write(os, cu_buffer_content_start(&buf),
		 cu_buffer_content_size(&buf));
}

static void
xd_leave(cufo_stream_t os, cufo_tag_t tag)
{
    char const *name;
    size_t name_len;
    char *buf;

    cufoP_flush(os, cu_true);

    name = cufo_tag_name(tag);
    name_len = strlen(name);
    buf = cu_salloc(name_len + 3);
    buf[0] = '<';
    buf[1] = '/';
    memcpy(buf + 2, name, name_len);
    buf[name_len + 2] = '>';
    xd_raw_write(os, buf, name_len + 3);
}

static void *
xd_close(cufo_stream_t os)
{
    if (!cufo_have_error(os)) {
	xd_raw_write(os, "</document>\n", 12);
	cufo_flush(os);
    }
    return cu_dsink_finish(SX_STREAM(os)->sink);
}

static void
xd_flush(cufo_stream_t os)
{
    cu_dsink_flush(SX_STREAM(os)->sink);
}

static struct cufo_target_s xd_target = {
    .write = xd_write,
    .enter = xd_enter,
    .leave = xd_leave,
    .close = xd_close,
    .flush = xd_flush,
};

static cu_bool_t
ascii_compat_encoding(char const *enc)
{
    switch (*enc) {
	case 'A': case 'a':
	    return strcasecmp(enc, "ASCII") == 0;
	case 'U':
	    return strcasecmp(enc, "UTF-8") == 0
		|| strcasecmp(enc, "UTF8") == 0;
	case 'I':
	    return strncasecmp(enc, "ISO-8859-", 9) == 0
		|| strncasecmp(enc, "ISO8859-", 8) == 0;
	default:
	    return cu_false;
    }
}

cufo_stream_t
cufo_open_xmldirect(char const *encoding, cu_dsink_t sink)
{
    xd_stream_t os = cu_gnew(struct xd_stream_s);
    if (!encoding)
	encoding = "UTF-8";
    else if (!ascii_compat_encoding(encoding)) {
	cu_bugf("Encoding %s not yet supported by the simple XML target.",
		encoding);
	/* sink = cu_dsink_new_iconv(sink, "UTF-8", encoding); */
	encoding = "UTF-8";
    }
    if (cufo_stream_init(cu_to(cufo_stream, os), encoding, &xd_target)) {
	os->sink = sink;
	xd_raw_write(cu_to(cufo_stream, os),
		     "<?xml version=\"1.0\" encoding=\"", 30);
	xd_raw_write(cu_to(cufo_stream, os), encoding, strlen(encoding));
	xd_raw_write(cu_to(cufo_stream, os), "\"?>\n<document>\n", 15);
	return cu_to(cufo_stream, os);
    } else
	return NULL;
}
