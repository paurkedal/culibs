/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2007--2009  Petter Urkedal <urkedal@nbi.dk>
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
#include <cu/clos.h>

CU_BEGIN_DECLARATIONS
/** \defgroup cu_ptr_seq_h cu/ptr_seq.h: Pointer Sequence Objects
 ** @{ \ingroup cu_seq_mod
 **
 ** This header defines generic ways of processing sequences of pointers.  The
 ** structs combine three concepts: \e sources which provide a sequence of
 ** pointers, \e sinks which consume a sequence of pointers, and construction.
 **
 ** A combination of a source and a sink is here called a \e junction.  It can
 ** be used to compute the image of a collection of objects in cases where it
 ** is desirable that the transforming function controls the iteration.
 **
 ** A sink can be useful for finishing the construction of a container.  A
 ** combination of a sink and a constructor is here called a \e sinktor.
 ** Similarly, we the combination of a junction and a constructor will be
 ** called a junctor.
 **/

/** \name API */

/** A pointer sequence source. */
struct cu_ptr_source
{
    void *(*get)(cu_ptr_source_t self);
};

/** Initialises the callback of \a source. */
CU_SINLINE void
cu_ptr_source_init(cu_ptr_source_t source,
		   void *(*get)(cu_ptr_source_t))
{
    source->get = get;
}

/** Returs the next element of the source sequence, or \c NULL if there are no
 ** more elements. */
CU_SINLINE void *
cu_ptr_source_get(cu_ptr_source_t source)
{ return source->get(source); }


/** A pointer sequence sink. */
struct cu_ptr_sink
{
    void (*put)(cu_ptr_sink_t self, void *elt);
};

/** Initialises the callback of \a sink. */
CU_SINLINE void
cu_ptr_sink_init(cu_ptr_sink_t sink,
		 void (*put)(cu_ptr_sink_t, void *elt))
{
    sink->put = put;
}

/** Puts \a elt into \a sink. */
CU_SINLINE void
cu_ptr_sink_put(cu_ptr_sink_t sink, void *elt)
{ sink->put(sink, elt); }


/** A pointer sequence junction (source and sink). */
struct cu_ptr_junction
{
    cu_inherit (cu_ptr_source);
    cu_inherit (cu_ptr_sink);
};

/** Initialises the callbacks of \a junction. */
CU_SINLINE void
cu_ptr_junction_init(cu_ptr_junction_t junction,
		     void *(*get)(cu_ptr_source_t),
		     void (*put)(cu_ptr_sink_t, void *))
{
    cu_ptr_source_init(cu_to(cu_ptr_source, junction), get);
    cu_ptr_sink_init(cu_to(cu_ptr_sink, junction), put);
}

/** Returs the next element of the source sequence of the junction. */
CU_SINLINE void *
cu_ptr_junction_get(cu_ptr_junction_t junction)
{ return cu_ptr_source_get(cu_to(cu_ptr_source, junction)); }

/** Puts \a elt into the sink of the junction. */
CU_SINLINE void
cu_ptr_junction_put(cu_ptr_junction_t junction, void *elt)
{ cu_ptr_sink_put(cu_to(cu_ptr_sink, junction), elt); }

/** Returns the source of \a junction. */
CU_SINLINE cu_ptr_source_t
cu_ptr_junction_source(cu_ptr_junction_t junction)
{ return cu_to(cu_ptr_source, junction); }

/** Returns the sink of \a junction. */
CU_SINLINE cu_ptr_sink_t
cu_ptr_junction_sink(cu_ptr_junction_t junction)
{ return cu_to(cu_ptr_sink, junction); }


/** A pointer sequence sink and constructor. */
struct cu_ptr_sinktor
{
    cu_inherit (cu_ptr_sink);
    void *(*finish)(cu_ptr_sinktor_t self);
};

/** Initialises the callbacks of \a sinktor. */
CU_SINLINE void
cu_ptr_sinktor_init(cu_ptr_sinktor_t sinktor,
		    void (*put)(cu_ptr_sink_t, void *),
		    void *(*finish)(cu_ptr_sinktor_t sinktor))
{
    cu_ptr_sink_init(cu_to(cu_ptr_sink, sinktor), put);
    sinktor->finish = finish;
}

/** Puts \a elt into the sink of the sinktor. */
CU_SINLINE void
cu_ptr_sinktor_put(cu_ptr_sinktor_t sinktor, void *elt)
{ cu_ptr_sink_put(cu_to(cu_ptr_sink, sinktor), elt); }

/** Finishes the construction based on the sink state. */
CU_SINLINE void *
cu_ptr_sinktor_finish(cu_ptr_sinktor_t sinktor)
{ return sinktor->finish(sinktor); }

/** Returns the sink of \a sinktor. */
CU_SINLINE cu_ptr_sink_t
cu_ptr_sinktor_sink(cu_ptr_sinktor_t sinktor)
{ return cu_to(cu_ptr_sink, sinktor); }


/** A pointer sequence junction and constructor. */
struct cu_ptr_junctor
{
    cu_inherit (cu_ptr_junction);
    void *(*finish)(cu_ptr_junctor_t self);
};

/** Initialises the callbacks of \a junctor. */
CU_SINLINE void
cu_ptr_junctor_init(cu_ptr_junctor_t junctor,
		    void *(*get)(cu_ptr_source_t),
		    void (*put)(cu_ptr_sink_t, void *),
		    void *(*finish)(cu_ptr_junctor_t))
{
    cu_ptr_junction_init(cu_to(cu_ptr_junction, junctor), get, put);
    junctor->finish = finish;
}

/** Returns the next element of the source of the junctor. */
CU_SINLINE void *
cu_ptr_junctor_get(cu_ptr_junctor_t junctor)
{ return cu_ptr_junction_get(cu_to(cu_ptr_junction, junctor)); }

/** Puts \a elt into the sink of the junctor. */
CU_SINLINE void
cu_ptr_junctor_put(cu_ptr_junctor_t junctor, void *elt)
{ cu_ptr_junction_put(cu_to(cu_ptr_junction, junctor), elt); }

/** Finishes the construction based to the sink state. */
CU_SINLINE void *
cu_ptr_junctor_finish(cu_ptr_junctor_t junctor)
{ return junctor->finish(junctor); }

/** Returns the junction of \a junctor. */
CU_SINLINE cu_ptr_junction_t
cu_ptr_junctor_junction(cu_ptr_junctor_t junctor)
{ return cu_to(cu_ptr_junction, junctor); }

/** Returns the source of \a junctor. */
CU_SINLINE cu_ptr_source_t
cu_ptr_junctor_source(cu_ptr_junctor_t junctor)
{ return cu_to2(cu_ptr_source, cu_ptr_junction, junctor); }

/** Returns the sink of \a junctor. */
CU_SINLINE cu_ptr_sink_t
cu_ptr_junctor_sink(cu_ptr_junctor_t junctor)
{ return cu_to2(cu_ptr_sink, cu_ptr_junction, junctor); }

/** @} */

/** \name Basic Adaptors and Implementations
 ** @{ */

cu_ptr_junction_t cu_ptr_junction_from_source_sink(cu_ptr_source_t source,
						   cu_ptr_sink_t sink);

cu_ptr_junctor_t cu_ptr_junctor_from_source_sinktor(cu_ptr_source_t source,
						    cu_ptr_sinktor_t sinktor);

struct cu_ptr_array_source
{
    cu_inherit (cu_ptr_source);
    void **cur, **end;
};

/** A source which fetches pointers from [\a begin, \a end). */
void cu_ptr_array_source_init(cu_ptr_array_source_t src,
			      void **begin, void **end);

/** A source which fetches pointers from [\a begin, \a end). */
cu_ptr_source_t cu_ptr_source_from_array(void **begin, void **end);

extern struct cu_ptr_source cuP_empty_ptr_source;
extern struct cu_ptr_sink cuP_empty_ptr_sink;
extern struct cu_ptr_junction cuP_empty_ptr_junction;

/** A source which always returns \c NULL. */
CU_SINLINE cu_ptr_source_t cu_empty_ptr_source()
{ return &cuP_empty_ptr_source; }

/** A sink which does not accept any items. An attempt to put an item into it
 ** is considered a bug and will cause the program to abort. */
CU_SINLINE cu_ptr_sink_t cu_empty_ptr_sink()
{ return &cuP_empty_ptr_sink; }

/** A junction with an empty source and an empty sink.
 ** \see cu_empty_ptr_source
 ** \see cu_empty_ptr_sink */
CU_SINLINE cu_ptr_junction_t cu_empty_ptr_junction()
{ return &cuP_empty_ptr_junction; }

/** A junctor with an empty source and an empty sink, which returns
 ** \a result. */
cu_ptr_junctor_t cu_empty_ptr_junctor(void *result);

/** @} */

/** \name Basic Algorithms
 ** @{ */

/** Counts the number of remaining elements on \a source, draining it in the
 ** process. Takes time proportional to the count. */
size_t cu_ptr_source_count(cu_ptr_source_t source);

/** True iff \a f maps each remaining element of \a source to true. Stops after
 ** the first failure. */
cu_bool_t cu_ptr_source_iterA(cu_clop(f, cu_bool_t, void *),
			      cu_ptr_source_t source);

/** True iff \a f maps at least one of the remaining elements to true. Stops
 ** after the first match. */
cu_bool_t cu_ptr_source_iterE(cu_clop(f, cu_bool_t, void *),
			      cu_ptr_source_t source);

cu_bool_t cu_ptr_source_bareA(cu_bool_t (*f)(void *), cu_ptr_source_t source);
cu_bool_t cu_ptr_source_bareE(cu_bool_t (*f)(void *), cu_ptr_source_t source);

/** Calls \a f on each element of \a source. */
void cu_ptr_source_iter(cu_clop(f, void, void *), cu_ptr_source_t source);

/** Compares the two sources by applying \a f to each parallel pair of elements
 ** drawn from \a source0 and \a source1.  At the first non-zero return by \a
 ** f, that value is returned.  If both sources are emptied, 0 is returned, if
 ** only \a source0 emptied, -1 is returned, and if only \a source1 is emptied,
 ** 1 is returned. */
int cu_ptr_source_compare(cu_clop(f, int, void *, void *),
			  cu_ptr_source_t source0, cu_ptr_source_t source1);

/** Transfers the remaining elements in \a source to \a sink. */
void cu_ptr_source_sink_short(cu_ptr_source_t source, cu_ptr_sink_t sink);

/** Stores up to \a array_size pointers from \a source into an array of
 ** pointers starting at \a array_out, and returns a pointer past the last
 ** assigned element. */
void **cu_ptr_source_store(cu_ptr_source_t source, size_t array_size,
			   void **array_out);

/** Transfers the remaining elements of the source of \a junction to its \a
 ** sink. This is equivalent to \ref cu_ptr_source_sink_short applied to the
 ** source and sink of \a junction. */
void cu_ptr_junction_short(cu_ptr_junction_t junction);

/** Transforms the remaining elements of \a source with \a f and puts them in
 ** order into \a sink. */
void cu_ptr_source_sink_image(cu_clop(f, void *, void *),
			      cu_ptr_source_t source, cu_ptr_sink_t sink);

/** A variant \ref cu_ptr_source_sink_image accepting a contextfree C function
 ** as callback. */
void cu_ptr_source_sink_image_cfn(void *(*f)(void *),
				  cu_ptr_source_t source, cu_ptr_sink_t sink);

/** Transforms the remaining elements of the source of \a junction with \a f
 ** and puts them in order into the sink of \a junction. This is equivalent to
 ** \ref cu_ptr_source_sink_image applied to the source and sink of \a
 ** junction. */
void cu_ptr_junction_image(cu_clop(f, void *, void *),
			   cu_ptr_junction_t junction);

/** A variant \ref cu_ptr_junction_image accepting a contextfree C function as
 ** callback. */
void cu_ptr_junction_image_cfn(void *(*f)(void *), cu_ptr_junction_t junction);

/** Same as \ref cu_ptr_junction_image, but also returns the result of calling
 ** \ref cu_ptr_junctor_finish at the end. */
void *cu_ptr_junctor_image(cu_clop(f, void *, void *),
			   cu_ptr_junctor_t junctor);

/** A variant \ref cu_ptr_junctor_image accepting a contextfree C function as
 ** callback. */
void *cu_ptr_junctor_image_cfn(void *(*f)(void *), cu_ptr_junctor_t junctor);

/** Filters the elements of \a source, putting those which \a f maps to true
 ** into \a sink. */
void cu_ptr_source_sink_filter(cu_clop(f, cu_bool_t, void *),
			       cu_ptr_source_t source, cu_ptr_sink_t sink);

/** Filters from the source to the sink of \a junction with \a f by putting
 ** only those which \a f maps to true into the sink. This is equivalent to
 ** \ref cu_ptr_source_sink_filter applied to the source and sink parts of \a
 ** junction. */
void cu_ptr_junction_filter(cu_clop(f, cu_bool_t, void *),
			    cu_ptr_junction_t junction);

/** Performs \ref cu_ptr_junction_filter, then returns the result of \ref
 ** cu_ptr_junctor_finish. */
void *cu_ptr_junctor_filter(cu_clop(f, cu_bool_t, void *),
			    cu_ptr_junctor_t junctor);

/** @} */
/** @} */
CU_END_DECLARATIONS

#define cu_ptr_source_forall cu_ptr_source_iterA
#define cu_ptr_source_forsome cu_ptr_source_iterE

#endif
