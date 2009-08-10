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
#include <cufo/sink.h>
#include <cutext/sink.h>
#include <cu/memory.h>
#include <cu/buffer.h>

typedef struct _xmlsink_s *_xmlsink_t;
struct _xmlsink_s
{
    cu_inherit (cutext_sink_s);
    cutext_sink_t subsink;
};

static size_t
_xmlsink_write(cutext_sink_t sink, void const *data, size_t size)
{
    _xmlsink_t xmlsink = cu_from(_xmlsink, cutext_sink, sink);
    size_t wz;
    char const *s_frag = data;
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
	    size_t frag_size = s_cur - s_frag;
	    wz = cutext_sink_write(xmlsink->subsink, s_frag, frag_size);
	    if (wz != frag_size) {
		if (wz == (size_t)-1)
		    return (size_t)-1;
		else
		    cu_bugf("Sink should but did not consume all data.");
	    }
	    s_frag = ++s_cur;

	    wz = cutext_sink_write(xmlsink->subsink, repl, repl_len);
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
    if (s_frag < s_end) {
	size_t frag_size = s_end - s_frag;
	wz = cutext_sink_write(xmlsink->subsink, s_frag, frag_size);
	if (wz == (size_t)-1)
	    return (size_t)-1;
	return size + wz - frag_size;
    }
    else
	return size;
}

static cu_bool_t
_xmlsink_raw_write(_xmlsink_t xmlsink, char const *buf, size_t len)
{
    size_t wz;
    wz = cutext_sink_write(xmlsink->subsink, buf, len);
    if (wz == (size_t)-1)
	return cu_false; /* FIXME: Find a way to report error. */
    else if (wz != len)
	cu_bugf("Sink should but did not consume all raw data.");
    else
	return cu_true;
}

static cu_bool_t
_xmlsink_enter(cutext_sink_t sink, cufo_tag_t tag, cufo_attrbind_t attrbinds)
{
    _xmlsink_t xmlsink = cu_from(_xmlsink, cutext_sink, sink);
    char const *name;
    size_t name_len;
    cufo_attr_t attr;
    struct cu_buffer_s buf;

    name = cufo_tag_name(tag);
    name_len = strlen(name);
    cu_buffer_init(&buf, name_len + 2);
    cu_buffer_write(&buf, "<", 1);
    cu_buffer_write(&buf, name, name_len);

    while ((attr = attrbinds->attr)) {
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
		cs = cu_unbox_ptr(char const *, attrbinds->value);
		cu_buffer_write(&buf, "\"", 1);
		cu_buffer_write(&buf, cs, strlen(cs));
		cu_buffer_write(&buf, "\"", 1);
		break;
	    case cufo_attrtype_int:
		i = cu_unbox_int(attrbinds->value);
		cu_buffer_extend_freecap(&buf, sizeof(int)*3 + 3);
		s = cu_buffer_content_end(&buf);
		sprintf(s, "\"%d\"%n", i, &n);
		cu_buffer_incr_content_end(&buf, n);
		break;
	    case cufo_attrtype_enum:
		i = cu_unbox_int(attrbinds->value);
		cs = (*attr->extra.enum_name)(i);
		cu_buffer_write(&buf, "\"", 1);
		cu_buffer_write(&buf, cs, strlen(cs));
		cu_buffer_write(&buf, "\"", 1);
		break;
	    default:
		cu_bug_unfinished();
	}
	++attrbinds;
    }
    cu_buffer_write(&buf, ">", 1);
    _xmlsink_raw_write(xmlsink, cu_buffer_content_start(&buf),
		       cu_buffer_content_size(&buf));
    return cu_true;
}

static void
_xmlsink_leave(cutext_sink_t sink, cufo_tag_t tag)
{
    _xmlsink_t xmlsink = cu_from(_xmlsink, cutext_sink, sink);
    char const *name;
    size_t name_len;
    char *buf;

    name = cufo_tag_name(tag);
    name_len = strlen(name);
    buf = cu_salloc(name_len + 3);
    buf[0] = '<';
    buf[1] = '/';
    memcpy(buf + 2, name, name_len);
    buf[name_len + 2] = '>';
    _xmlsink_raw_write(xmlsink, buf, name_len + 3);
}

static cu_bool_t
_xmlsink_flush(cutext_sink_t sink)
{
    _xmlsink_t xmlsink = cu_from(_xmlsink, cutext_sink, sink);
    return cutext_sink_flush(xmlsink->subsink);
}

static cu_box_t
_xmlsink_close(cutext_sink_t sink)
{
    _xmlsink_t xmlsink = cu_from(_xmlsink, cutext_sink, sink);
    _xmlsink_raw_write(xmlsink, "</document>\n", 12);
    return cutext_sink_finish(xmlsink->subsink);
}

static void
_xmlsink_discard(cutext_sink_t sink)
{
    _xmlsink_t xmlsink = cu_from(_xmlsink, cutext_sink, sink);
    cutext_sink_discard(xmlsink->subsink);
}

static cu_bool_t
_xmlsink_iterA_subsinks(cutext_sink_t sink, cu_clop(f, cu_bool_t, cutext_sink_t))
{
    _xmlsink_t xmlsink = cu_from(_xmlsink, cutext_sink, sink);
    return cu_call(f, xmlsink->subsink);
}

static struct cutext_sink_descriptor_s _xmlsink_descriptor = {
    CU_DSINK_DESCRIPTOR_DEFAULTS,
    .flags = CU_DSINK_FLAG_CUFO_EXT | CU_DSINK_FLAG_CLOGFREE,
    .write = _xmlsink_write,
    .flush = _xmlsink_flush,
    .finish = _xmlsink_close,
    .discard = _xmlsink_discard,
    .iterA_subsinks = _xmlsink_iterA_subsinks,
    .enter = _xmlsink_enter,
    .leave = _xmlsink_leave,
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

cutext_sink_t
cufo_sink_new_xml(cutext_sink_t subsink)
{
    _xmlsink_t xmlsink = cu_gnew(struct _xmlsink_s);
    char const *sub_encoding = cutext_sink_encoding(subsink);
    if (!sub_encoding)
	sub_encoding = "UTF-8";
    else if (!ascii_compat_encoding(sub_encoding))
	subsink = cutext_sink_stack_iconv("UTF-8", subsink);
    if (!cutext_sink_is_clogfree(subsink))
	subsink = cutext_sink_stack_buffer(subsink);
    cutext_sink_init(cu_to(cutext_sink, xmlsink), &_xmlsink_descriptor);
    xmlsink->subsink = subsink;
    _xmlsink_raw_write(xmlsink, "<?xml version=\"1.0\" encoding=\"", 30);
    _xmlsink_raw_write(xmlsink, sub_encoding, strlen(sub_encoding));
    _xmlsink_raw_write(xmlsink, "\"?>\n<document>\n", 15);
    return cu_to(cutext_sink, xmlsink);
}

cufo_stream_t
cufo_open_xml(cutext_sink_t subsink)
{
    cutext_sink_t sink = cufo_sink_new_xml(subsink);
    return cufo_open_sink(sink);
}
