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


/* Adaptor: Junction from Source and Sink */

typedef struct junction_from_source_sink_s *junction_from_source_sink_t;
struct junction_from_source_sink_s
{
    cu_inherit (cu_ptr_junction_s);
    cu_ptr_source_t source;
    cu_ptr_sink_t sink;
};

void *
junction_from_source_sink_get(cu_ptr_source_t source)
{
    junction_from_source_sink_t self;
    self = cu_from2(junction_from_source_sink,
		    cu_ptr_junction, cu_ptr_source, source);
    return cu_ptr_source_get(self->source);
}

void
junction_from_source_sink_put(cu_ptr_sink_t sink, void *elt)
{
    junction_from_source_sink_t self;
    self = cu_from2(junction_from_source_sink,
		    cu_ptr_junction, cu_ptr_sink, sink);
    return cu_ptr_sink_put(self->sink, elt);
}

cu_ptr_junction_t
cu_ptr_junction_from_source_sink(cu_ptr_source_t source, cu_ptr_sink_t sink)
{
    junction_from_source_sink_t self;
    self = cu_gnew(struct junction_from_source_sink_s);
    cu_ptr_junction_init(cu_to(cu_ptr_junction, self),
			 junction_from_source_sink_get,
			 junction_from_source_sink_put);
    self->source = source;
    self->sink = sink;
    return cu_to(cu_ptr_junction, self);
}


/* Adaptor: Junctor from Source and Sinktor */

typedef struct junctor_from_source_sinktor_s *junctor_from_source_sinktor_t;
struct junctor_from_source_sinktor_s
{
    cu_inherit (cu_ptr_junctor_s);
    cu_ptr_source_t source;
    cu_ptr_sinktor_t sinktor;
};

void *
junctor_from_source_sinktor_get(cu_ptr_source_t source)
{
    junctor_from_source_sinktor_t self;
    self = cu_from3(junctor_from_source_sinktor, cu_ptr_junctor,
		    cu_ptr_junction, cu_ptr_source, source);
    return cu_ptr_source_get(self->source);
}

void
junctor_from_source_sinktor_put(cu_ptr_sink_t sink, void *elt)
{
    junctor_from_source_sinktor_t self;
    self = cu_from3(junctor_from_source_sinktor, cu_ptr_junctor,
		    cu_ptr_junction, cu_ptr_sink, sink);
    return cu_ptr_sinktor_put(self->sinktor, elt);
}

void *
junctor_from_source_sinktor_finish(cu_ptr_junctor_t junctor)
{
    junctor_from_source_sinktor_t self;
    self = cu_from(junctor_from_source_sinktor, cu_ptr_junctor, junctor);
    return cu_ptr_sinktor_finish(self->sinktor);
}

cu_ptr_junctor_t
cu_ptr_junctor_from_source_sinktor(cu_ptr_source_t source,
				   cu_ptr_sinktor_t sinktor)
{
    junctor_from_source_sinktor_t self;
    self = cu_gnew(struct junctor_from_source_sinktor_s);
    cu_ptr_junctor_init(cu_to(cu_ptr_junctor, self),
			junctor_from_source_sinktor_get,
			junctor_from_source_sinktor_put,
			junctor_from_source_sinktor_finish);
    self->source = source;
    self->sinktor = sinktor;
    return cu_to(cu_ptr_junctor, self);
}
