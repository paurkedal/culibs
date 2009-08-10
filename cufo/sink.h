/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2009  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUFO_DSINK_H
#define CUFO_DSINK_H

#include <cufo/fwd.h>
#include <cutext/sink.h>

CU_BEGIN_DECLARATIONS
/** \defgroup cufo_dsink_h cufo/dsink.h: Data Sinks with Markup Handlers
 ** @{ \ingroup cufo_mod */

CU_SINLINE cu_bool_t
cufo_sink_enter(cutext_sink_t sink, cufo_tag_t tag, cufo_attrbind_t attrbinds)
{ return (*sink->descriptor->enter)(sink, tag, attrbinds); }

CU_SINLINE void
cufo_sink_leave(cutext_sink_t sink, cufo_tag_t tag)
{ (*sink->descriptor->leave)(sink, tag); }

cutext_sink_t cufo_sink_new_strip(cutext_sink_t subsink);
cutext_sink_t cufo_sink_new_xml(cutext_sink_t subsink);
cutext_sink_t cufo_sink_stack_buffer(cutext_sink_t subsink);
cutext_sink_t cufo_sink_stack_iconv(char const *new_enc, cutext_sink_t subsink);

/** @} */
CU_END_DECLARATIONS

#endif
