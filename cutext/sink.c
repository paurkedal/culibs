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

#include <cutext/sink.h>
#include <cu/diag.h>
#include <cu/str.h>

size_t
cutext_sink_noop_write(cutext_sink_t sink, void const *data, size_t size)
{
    return size;
}

cu_bool_t
cutext_sink_noop_flush(cutext_sink_t sink)
{
    return cu_true;
}

cu_box_t
cutext_sink_noop_finish(cutext_sink_t sink)
{
    return cu_box_void();
}

void
cutext_sink_noop_discard(cutext_sink_t sink)
{
}

cu_clos_def(_subsink_write, cu_prot(cu_bool_t, cutext_sink_t sink),
    ( void const *data; size_t size; size_t size_check; ))
{
    cu_clos_self(_subsink_write);
    size_t size_out = cutext_sink_write(sink, self->data, self->size);
    if (size_out == CU_DSINK_WRITE_ERROR)
	return cu_false;
    else if (self->size_check == (size_t)-1) {
	self->size_check = size_out;
	return cu_true;
    }
    else {
	if (self->size_check != size_out)
	    cu_bugf("Subsinks report different written sizes. "
		    "Use clogfree sinks or implement a custom write method "
		    "for the sink-combining sink.");
	return cu_true;
    }
}

size_t
cutext_sink_subsinks_write(cutext_sink_t sink, void const *data, size_t size)
{
    _subsink_write_t f;
    f.data = data;
    f.size = size;
    f.size_check = (size_t)-1;
    if (cutext_sink_iterA_subsinks(sink, _subsink_write_prep(&f)))
	return f.size_check == (size_t)-1? size : f.size_check;
    else
	return 0;
}

cu_clos_def(_subsink_flush, cu_prot(cu_bool_t, cutext_sink_t sink),
    ( cu_bool_t status; ))
{
    cu_clos_self(_subsink_flush);
    if (!cutext_sink_flush(sink))
	self->status = cu_false;
    return cu_true;
}

cu_bool_t
cutext_sink_subsinks_flush(cutext_sink_t sink)
{
    _subsink_flush_t f;
    f.status = cu_true;
    cutext_sink_iterA_subsinks(sink, _subsink_flush_prep(&f));
    return f.status;
}

cu_bool_t
cutext_sink_empty_iterA_subsinks(cutext_sink_t sink,
				 cu_clop(f, cu_bool_t, cutext_sink_t))
{
    return cu_true;
}

static cu_str_t
_default_debug_state_info(cutext_sink_t sink)
{
    return cu_str_new_fmt("cutext_sink_t @ %p with descriptor @ %p",
			  (void *)sink, (void *)sink->descriptor);
}

cu_box_t
cutext_sink_info_inherit(cutext_sink_t sink, cutext_sink_info_key_t key,
			 cutext_sink_t subsink)
{
    if (cutext_sink_info_key_inherits(key))
	return cutext_sink_info(subsink, key);
    else
	return cutext_sink_default_info(sink, key);
}

cu_box_t
cutext_sink_default_info(cutext_sink_t sink, cutext_sink_info_key_t key)
{
    switch (key) {
	case CUTEXT_SINK_INFO_ENCODING:
	    return cu_box_ptr(cutext_sink_info_encoding_t, NULL);
	case CUTEXT_SINK_INFO_NCOLUMNS:
	    return cu_box_int(80);
	case CUTEXT_SINK_INFO_DEBUG_STATE:
	    return cu_box_ptr(cutext_sink_info_debug_state_t,
			      _default_debug_state_info(sink));
	default:
	    cu_bug_unreachable();
    }
}

cu_bool_t
cutext_sink_noop_enter(cutext_sink_t sink, struct cufo_tag_s *tag,
		       struct cufo_attrbind_s *attrbinds)
{
    return cu_false;
}

void
cutext_sink_noop_leave(cutext_sink_t sink, struct cufo_tag_s *tag)
{
}

cu_clos_def(_sink_debug_dump, cu_prot(cu_bool_t, cutext_sink_t sink),
    ( int indent; ))
{
    cu_clos_self(_sink_debug_dump);
    cu_str_t info;
    int i;
    info = cu_unbox_ptr(cutext_sink_info_debug_state_t,
			cutext_sink_info(sink, CUTEXT_SINK_INFO_DEBUG_STATE));
    for (i = 0; i < self->indent; ++i)
	fputc(' ', stderr);
    fputs(cu_str_to_cstr(info), stderr);
    fputc('\n', stderr);
    self->indent += 4;
    return cutext_sink_iterA_subsinks(sink, _sink_debug_dump_ref(self));
}

void
cutext_sink_debug_dump(cutext_sink_t sink)
{
    _sink_debug_dump_t f;
    f.indent = 0;
    cu_call(_sink_debug_dump_prep(&f), sink);
}

void
cutext_sink_assert_clogfree(cutext_sink_t sink)
{
    if (!sink->descriptor->flags & CUTEXT_SINK_FLAG_CLOGFREE)
	cu_bugf("Caller expected a clog-free sink, but sink does not "
		"guarante to be clog-free.");
}

#define DCOUNTSINK(sink) cu_from(cutext_countsink, cutext_sink, sink)

static size_t
_dcountsink_write(cutext_sink_t sink, void const *arr, size_t len)
{
    DCOUNTSINK(sink)->count += len;
    return len;
}

static struct cutext_sink_descriptor_s _dcountsink_descriptor = {
    CUTEXT_SINK_DESCRIPTOR_DEFAULTS,
    .flags = CUTEXT_SINK_FLAG_CLOGFREE,
    .write = _dcountsink_write
};

void
cutext_countsink_init(cutext_countsink_t sink)
{
    cutext_sink_init(cu_to(cutext_sink, sink), &_dcountsink_descriptor);
    sink->count = 0;
}
