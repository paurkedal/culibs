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

#ifndef CUFO_TEXTSTREAM_H
#define CUFO_TEXTSTREAM_H

#include <cufo/stream.h>
#include <cufo/sink.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cufo_textsink_h cufo/textsink.h: Text Target Styler and Stream
 *@{\ingroup cufo_mod
 *
 * \note Don't use this directly unless writing a text style definition,
 * instead call the appropriate open-function from \ref cufo_stream_h
 * "cufo/stream.h".
 */

/*!The text-sink struct for use by text-stylers. */
struct cufo_textsink_s
{
    cu_inherit (cutext_sink_s);
    cutext_sink_t subsink;

    struct cu_buffer_s buf;
    struct cu_buffer_s buf_markup;
    int buffered_width;

    cufo_textstyle_t style;

    int tabstop;
    int left_margin;
    int right_margin;
    int cont_indent;
    cu_wstring_t cont_eol_insert;
    cu_wstring_t cont_bol_insert;

    cu_bool_t is_cont : 1;
    size_t input_pos;
};

CU_SINLINE int
cufo_textsink_width(cufo_textsink_t sink)
{ return sink->right_margin - sink->left_margin; }

void cufo_textsink_block_boundary(cufo_textsink_t sink);

/*!A \ref cufo_textstyle_s callback for a single tag. */
struct cufo_textstyler_s
{
    /* Node and key */
    cu_inherit (cucon_hzmap_node_s);
    cu_word_t tag;

    /* Value */
    cu_wstring_t (*enter)(cufo_textsink_t sink, cufo_tag_t tag,
			  cufo_attrbind_t attrbinds);
    cu_wstring_t (*leave)(cufo_textsink_t sink, cufo_tag_t tag);
};

/*!A style definition for a text sink. */
struct cufo_textstyle_s
{
    size_t sink_size;
    cu_clop(sink_init, void, cufo_textsink_t sink);
    struct cucon_hzmap_s tag_to_styler;
    cu_wstring_t (*default_enter)(cufo_textsink_t sink, cufo_tag_t tag,
				  cufo_attrbind_t attrbinds);
    cu_wstring_t (*default_leave)(cufo_textsink_t sink, cufo_tag_t tag);
};

/*!Initialises \a style with the given sink size and sink initialiser.  \a
 * sink_size is the full size of the sink struct of some type derived from
 * \ref cufo_textsink_s.  At sink creation, the \ref cufo_textsink_s base
 * struct is first initialised, then \a sink_init is called to finish the
 * initialisation. */
void cufo_textstyle_init(cufo_textstyle_t style, size_t sink_size,
			 cu_clop(sink_init, void, cufo_textsink_t));

/*!Make a static declaration and partial initialisation of a \ref
 * cufo_textstyler_s struct using \a name as a prefix for the identifiers
 * involved.  This will define <i>name</i><tt>_styler</tt> from
 * <i>name</i><tt>_enter</tt> and <i>name</i><tt>_leave</tt>.  Association with
 * a tag is postponed to \a cufo_textstyle_bind_static, since the tags need to
 * be dynamically allocated. */
#define CUFO_TEXTSTYLER_STATIC(name) \
    static struct cufo_textstyler_s name##_styler \
	= {CUCON_HZMAP_NODE_INIT, 0, name##_enter, name##_leave}

/*!Bind \a styler, which declared and partly initialised with \ref
 * CUFO_TEXTSTYLER_STATIC, as callbacks for \a tag.  \a styler may only occur
 * in one such call. */
void cufo_textstyle_bind_static(cufo_textstyle_t style,
				cufo_tag_t tag, cufo_textstyler_t styler);

struct cufo_textstyle_s cufoP_default_textstyle;

CU_SINLINE cufo_textstyle_t
cufo_default_textstyle(void)
{ return &cufoP_default_textstyle; }

/*!@}*/
CU_END_DECLARATIONS

#endif
