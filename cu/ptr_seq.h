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

#ifndef CU_PTR_SEQ_H
#define CU_PTR_SEQ_H

#include <cu/fwd.h>
#include <cu/inherit.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cu_ptr_seq_h cu/ptr_seq.h: Pointer Sequence Objects
 *@{\ingroup cu_mod
 *
 * This header defines generic ways of processing sequences of pointers.  The
 * structs combine three concepts: \e sources which provide a sequence of
 * pointers, \e sinks which consume a sequence of pointers, and construction.
 *
 * A combination of a source and a sink is here called a \e junction.  It can
 * be used to compute the image of a collection of objects in cases where it is
 * desirable that the transforming function controls the iteration.
 *
 * A sink can be useful for finishing the construction of a container.  A
 * combination of a sink and a constructor is here called a \e sinktor.
 * Similarly, we the combination of a junction and a constructor will be called
 * a junctor.
 */

/*!A pointer sequence source. */
struct cu_ptr_source_s
{
    void *(*get)(cu_ptr_source_t self);
};

/*!Initialises the callback of \a source. */
CU_SINLINE void
cu_ptr_source_init(cu_ptr_source_t source,
		   void *(*get)(cu_ptr_source_t))
{
    source->get = get;
}

/*!Returs the next element of the source sequence, or \c NULL if there are no
 * more elements. */
CU_SINLINE void *
cu_ptr_source_get(cu_ptr_source_t source)
{ return source->get(source); }


/*!A pointer sequence sink. */
struct cu_ptr_sink_s
{
    void (*put)(cu_ptr_sink_t self, void *elt);
};

/*!Initialises the callback of \a sink. */
CU_SINLINE void
cu_ptr_sink_init(cu_ptr_sink_t sink,
		 void (*put)(cu_ptr_sink_t, void *elt))
{
    sink->put = put;
}

/*!Puts \a elt into \a sink. */
CU_SINLINE void
cu_ptr_sink_put(cu_ptr_sink_t sink, void *elt)
{ sink->put(sink, elt); }


/*!A pointer sequence junction (source and sink). */
struct cu_ptr_junction_s
{
    cu_inherit (cu_ptr_source_s);
    cu_inherit (cu_ptr_sink_s);
};

/*!Initialises the callbacks of \a junction. */
CU_SINLINE void
cu_ptr_junction_init(cu_ptr_junction_t junction,
		     void *(*get)(cu_ptr_source_t),
		     void (*put)(cu_ptr_sink_t, void *))
{
    cu_ptr_source_init(cu_to(cu_ptr_source, junction), get);
    cu_ptr_sink_init(cu_to(cu_ptr_sink, junction), put);
}

/*!Returs the next element of the source sequence of the junction. */
CU_SINLINE void *
cu_ptr_junction_get(cu_ptr_junction_t junction)
{ return cu_ptr_source_get(cu_to(cu_ptr_source, junction)); }

/*!Puts \a elt into the sink of the junction. */
CU_SINLINE void
cu_ptr_junction_put(cu_ptr_junction_t junction, void *elt)
{ cu_ptr_sink_put(cu_to(cu_ptr_sink, junction), elt); }

/*!Returns the source of \a junction. */
CU_SINLINE cu_ptr_source_t
cu_ptr_junction_source(cu_ptr_junction_t junction)
{ return cu_to(cu_ptr_source, junction); }

/*!Returns the sink of \a junction. */
CU_SINLINE cu_ptr_sink_t
cu_ptr_junction_sink(cu_ptr_junction_t junction)
{ return cu_to(cu_ptr_sink, junction); }


/*!A pointer sequence sink and constructor. */
struct cu_ptr_sinktor_s
{
    cu_inherit (cu_ptr_sink_s);
    void *(*finish)(cu_ptr_sinktor_t self);
};

/*!Initialises the callbacks of \a sinktor. */
CU_SINLINE void
cu_ptr_sinktor_init(cu_ptr_sinktor_t sinktor,
		    void (*put)(cu_ptr_sink_t, void *),
		    void *(*finish)(cu_ptr_sinktor_t sinktor))
{
    cu_ptr_sink_init(cu_to(cu_ptr_sink, sinktor), put);
    sinktor->finish = finish;
}

/*!Puts \a elt into the sink of the sinktor. */
CU_SINLINE void
cu_ptr_sinktor_put(cu_ptr_sinktor_t sinktor, void *elt)
{ cu_ptr_sink_put(cu_to(cu_ptr_sink, sinktor), elt); }

/*!Finishes the construction based on the sink state. */
CU_SINLINE void *
cu_ptr_sinktor_finish(cu_ptr_sinktor_t sinktor)
{ return sinktor->finish(sinktor); }

/*!Returns the sink of \a sinktor. */
CU_SINLINE cu_ptr_sink_t
cu_ptr_sinktor_sink(cu_ptr_sinktor_t sinktor)
{ return cu_to(cu_ptr_sink, sinktor); }


/*!A pointer sequence junction and constructor. */
struct cu_ptr_junctor_s
{
    cu_inherit (cu_ptr_junction_s);
    void *(*finish)(cu_ptr_junctor_t self);
};

/*!Initialises the callbacks of \a junctor. */
CU_SINLINE void
cu_ptr_junctor_init(cu_ptr_junctor_t junctor,
		    void *(*get)(cu_ptr_source_t),
		    void (*put)(cu_ptr_sink_t, void *),
		    void *(*finish)(cu_ptr_junctor_t))
{
    cu_ptr_junction_init(cu_to(cu_ptr_junction, junctor), get, put);
    junctor->finish = finish;
}

/*!Returns the next element of the source of the junctor. */
CU_SINLINE void *
cu_ptr_junctor_get(cu_ptr_junctor_t junctor)
{ return cu_ptr_junction_get(cu_to(cu_ptr_junction, junctor)); }

/*!Puts \a elt into the sink of the junctor. */
CU_SINLINE void
cu_ptr_junctor_put(cu_ptr_junctor_t junctor, void *elt)
{ cu_ptr_junction_put(cu_to(cu_ptr_junction, junctor), elt); }

/*!Finishes the construction based to the sink state. */
CU_SINLINE void *
cu_ptr_junctor_finish(cu_ptr_junctor_t junctor)
{ return junctor->finish(junctor); }

/*!Returns the junction of \a junctor. */
CU_SINLINE cu_ptr_junction_t
cu_ptr_junctor_junction(cu_ptr_junctor_t junctor)
{ return cu_to(cu_ptr_junction, junctor); }

/*!Returns the source of \a junctor. */
CU_SINLINE cu_ptr_source_t
cu_ptr_junctor_source(cu_ptr_junctor_t junctor)
{ return cu_to2(cu_ptr_source, cu_ptr_junction, junctor); }

/*!Returns the sink of \a junctor. */
CU_SINLINE cu_ptr_sink_t
cu_ptr_junctor_sink(cu_ptr_junctor_t junctor)
{ return cu_to2(cu_ptr_sink, cu_ptr_junction, junctor); }

/*!@}*/
CU_END_DECLARATIONS

#endif
