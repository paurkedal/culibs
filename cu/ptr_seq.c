/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2007  Petter Urkedal <urkedal@nbi.dk>
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

#include <cu/ptr_seq.h>
#include <cu/memory.h>
#include <cu/diag.h>

size_t
cu_ptr_source_count(cu_ptr_source_t source)
{
    size_t cnt = 0;
    while (cu_ptr_source_get(source) != NULL)
	++cnt;
    return cnt;
}

cu_bool_t
cu_ptr_source_forall(cu_clop(f, cu_bool_t, void *), cu_ptr_source_t source)
{
    void *elt;
    while ((elt = cu_ptr_source_get(source)))
	if (!cu_call(f, elt))
	    return cu_false;
    return cu_true;
}

cu_bool_t
cu_ptr_source_forsome(cu_clop(f, cu_bool_t, void *), cu_ptr_source_t source)
{
    void *elt;
    while ((elt = cu_ptr_source_get(source)))
	if (cu_call(f, elt))
	    return cu_true;
    return cu_false;
}

int
cu_ptr_source_compare(cu_clop(f, int, void *, void *),
		      cu_ptr_source_t source0, cu_ptr_source_t source1)
{
    void *elt0, *elt1;
    for (;;) {
	int cmp;
	elt0 = cu_ptr_source_get(source0);
	elt1 = cu_ptr_source_get(source1);
	if (!elt0)
	    return elt1? -1 : 0;
	else if (!elt1)
	    return 1;
	cmp = cu_call(f, elt0, elt1);
	if (cmp != 0)
	    return cmp;
    }
}

void
cu_ptr_source_sink_short(cu_ptr_source_t source, cu_ptr_sink_t sink)
{
    void *elt;
    while ((elt = cu_ptr_source_get(source)))
	cu_ptr_sink_put(sink, elt);
}

void **
cu_ptr_source_store(cu_ptr_source_t source, size_t array_size,
		    void **array_out)
{
    void *elt;
    while (array_size--) {
	elt = cu_ptr_source_get(source);
	if (!elt)
	    break;
	(*array_out++) = elt;
    }
    return array_out;
}

void
cu_ptr_junction_short(cu_ptr_junction_t junction)
{
    void *elt;
    while ((elt = cu_ptr_junction_get(junction)))
	cu_ptr_junction_put(junction, elt);
}

void
cu_ptr_source_sink_image(cu_clop(f, void *, void *),
			 cu_ptr_source_t source, cu_ptr_sink_t sink)
{
    void *elt;
    while ((elt = cu_ptr_source_get(source)))
	cu_ptr_sink_put(sink, cu_call(f, elt));
}

void
cu_ptr_source_sink_image_cfn(void *(*f)(void *),
			     cu_ptr_source_t source, cu_ptr_sink_t sink)
{
    void *elt;
    while ((elt = cu_ptr_source_get(source)))
	cu_ptr_sink_put(sink, (*f)(elt));
}

void
cu_ptr_junction_image(cu_clop(f, void *, void *), cu_ptr_junction_t junction)
{
    void *elt;
    while ((elt = cu_ptr_junction_get(junction)))
	cu_ptr_junction_put(junction, cu_call(f, elt));
}

void
cu_ptr_junction_image_cfn(void *(*f)(void *), cu_ptr_junction_t junction)
{
    void *elt;
    while ((elt = cu_ptr_junction_get(junction)))
	cu_ptr_junction_put(junction, (*f)(elt));
}

void *
cu_ptr_junctor_image(cu_clop(f, void *, void *), cu_ptr_junctor_t junctor)
{
    void *elt;
    while ((elt = cu_ptr_junctor_get(junctor)))
	cu_ptr_junctor_put(junctor, cu_call(f, elt));
    return cu_ptr_junctor_finish(junctor);
}

void *
cu_ptr_junctor_image_cfn(void *(*f)(void *), cu_ptr_junctor_t junctor)
{
    void *elt;
    while ((elt = cu_ptr_junctor_get(junctor)))
	cu_ptr_junctor_put(junctor, (*f)(elt));
    return cu_ptr_junctor_finish(junctor);
}

void
cu_ptr_source_sink_filter(cu_clop(f, cu_bool_t, void *),
			  cu_ptr_source_t source, cu_ptr_sink_t sink)
{
    void *elt;
    while ((elt = cu_ptr_source_get(source)))
	if (cu_call(f, elt))
	    cu_ptr_sink_put(sink, elt);
}

void
cu_ptr_junction_filter(cu_clop(f, cu_bool_t, void *),
		       cu_ptr_junction_t junction)
{
    void *elt;
    while ((elt = cu_ptr_junction_get(junction)))
	if (cu_call(f, elt))
	    cu_ptr_junction_put(junction, elt);
}

void *
cu_ptr_junctor_filter(cu_clop(f, cu_bool_t, void *), cu_ptr_junctor_t junctor)
{
    void *elt;
    while ((elt = cu_ptr_junctor_get(junctor)))
	if (cu_call(f, elt))
	    cu_ptr_junctor_put(junctor, elt);
    return cu_ptr_junctor_finish(junctor);
}


/* Empty Sequences */

static void *
_empty_source_get(cu_ptr_source_t source)
{
    return NULL;
}

static void
_empty_sink_put(cu_ptr_sink_t sink, void *ptr)
{
    cu_bugf("Tried to return value to empty junctor.");
}

struct cu_ptr_source_s cuP_empty_ptr_source = {_empty_source_get};
struct cu_ptr_sink_s cuP_empty_ptr_sink = {_empty_sink_put};
struct cu_ptr_junction_s cuP_empty_ptr_junction = {
    {_empty_source_get}, {_empty_sink_put}
};

struct cu_empty_ptr_junctor_s
{
    cu_inherit (cu_ptr_junctor_s);
    void *result;
};

static void *
_empty_junctor_finish(cu_ptr_junctor_t jct)
{
    return cu_from(cu_empty_ptr_junctor, cu_ptr_junctor, jct)->result;
}

cu_ptr_junctor_t
cu_empty_ptr_junctor(void *result)
{
    struct cu_empty_ptr_junctor_s *jct;
    jct = cu_gnew(struct cu_empty_ptr_junctor_s);
    jct->result = result;
    cu_ptr_junctor_init(cu_to(cu_ptr_junctor, jct),
			_empty_source_get, _empty_sink_put,
			_empty_junctor_finish);
    return cu_to(cu_ptr_junctor, jct);
}


/* Adaptor: Junction from Source and Sink */

typedef struct _junction_from_source_sink_s *_junction_from_source_sink_t;
struct _junction_from_source_sink_s
{
    cu_inherit (cu_ptr_junction_s);
    cu_ptr_source_t source;
    cu_ptr_sink_t sink;
};

static void *
_junction_from_source_sink_get(cu_ptr_source_t source)
{
    _junction_from_source_sink_t self;
    self = cu_from2(_junction_from_source_sink,
		    cu_ptr_junction, cu_ptr_source, source);
    return cu_ptr_source_get(self->source);
}

static void
_junction_from_source_sink_put(cu_ptr_sink_t sink, void *elt)
{
    _junction_from_source_sink_t self;
    self = cu_from2(_junction_from_source_sink,
		    cu_ptr_junction, cu_ptr_sink, sink);
    return cu_ptr_sink_put(self->sink, elt);
}

cu_ptr_junction_t
cu_ptr_junction_from_source_sink(cu_ptr_source_t source, cu_ptr_sink_t sink)
{
    _junction_from_source_sink_t self;
    self = cu_gnew(struct _junction_from_source_sink_s);
    cu_ptr_junction_init(cu_to(cu_ptr_junction, self),
			 _junction_from_source_sink_get,
			 _junction_from_source_sink_put);
    self->source = source;
    self->sink = sink;
    return cu_to(cu_ptr_junction, self);
}


/* Adaptor: Junctor from Source and Sinktor */

typedef struct _junctor_from_source_sinktor_s *_junctor_from_source_sinktor_t;
struct _junctor_from_source_sinktor_s
{
    cu_inherit (cu_ptr_junctor_s);
    cu_ptr_source_t source;
    cu_ptr_sinktor_t sinktor;
};

static void *
_junctor_from_source_sinktor_get(cu_ptr_source_t source)
{
    _junctor_from_source_sinktor_t self;
    self = cu_from3(_junctor_from_source_sinktor, cu_ptr_junctor,
		    cu_ptr_junction, cu_ptr_source, source);
    return cu_ptr_source_get(self->source);
}

static void
_junctor_from_source_sinktor_put(cu_ptr_sink_t sink, void *elt)
{
    _junctor_from_source_sinktor_t self;
    self = cu_from3(_junctor_from_source_sinktor, cu_ptr_junctor,
		    cu_ptr_junction, cu_ptr_sink, sink);
    return cu_ptr_sinktor_put(self->sinktor, elt);
}

static void *
_junctor_from_source_sinktor_finish(cu_ptr_junctor_t junctor)
{
    _junctor_from_source_sinktor_t self;
    self = cu_from(_junctor_from_source_sinktor, cu_ptr_junctor, junctor);
    return cu_ptr_sinktor_finish(self->sinktor);
}

cu_ptr_junctor_t
cu_ptr_junctor_from_source_sinktor(cu_ptr_source_t source,
				   cu_ptr_sinktor_t sinktor)
{
    _junctor_from_source_sinktor_t self;
    self = cu_gnew(struct _junctor_from_source_sinktor_s);
    cu_ptr_junctor_init(cu_to(cu_ptr_junctor, self),
			_junctor_from_source_sinktor_get,
			_junctor_from_source_sinktor_put,
			_junctor_from_source_sinktor_finish);
    self->source = source;
    self->sinktor = sinktor;
    return cu_to(cu_ptr_junctor, self);
}

/* Array source */

static void *
_ptr_array_source_get(cu_ptr_source_t src_)
{
    cu_ptr_array_source_t src;
    src = cu_from(cu_ptr_array_source, cu_ptr_source, src_);
    if (src->cur == src->end)
	return NULL;
    else
	return *src->cur++;
}

void
cu_ptr_array_source_init(cu_ptr_array_source_t src, void **begin, void **end)
{
    cu_ptr_source_init(cu_to(cu_ptr_source, src), _ptr_array_source_get);
    src->cur = begin;
    src->end = end;
}

cu_ptr_source_t
cu_ptr_source_from_array(void **begin, void **end)
{
    cu_ptr_array_source_t src = cu_gnew(struct cu_ptr_array_source_s);
    cu_ptr_array_source_init(src, begin, end);
    return cu_to(cu_ptr_source, src);
}
