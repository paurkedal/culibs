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

CU_BEGIN_DECLARATIONS
/*!\defgroup cufo_textstream_h cufo/textstream.h: Text Target Styler and Stream
 *@{\ingroup cufo_mod
 *
 * \note Don't use this directly unless writing a text style definition,
 * instead call the appropriate open-function from \ref cufo_stream_h
 * "cufo/stream.h".
 */

#define CUFO_ENTER_CONTROL_SEQ 0x91
#define CUFO_LEAVE_CONTROL_SEQ 0x92

/*!The text-stream struct for use by text-stylers. */
struct cufo_textstream_s
{
    cu_inherit (cufo_stream_s);
    struct cu_buffer_s buf;
    cu_dsink_t sink;
    int buffered_width;

    cufo_textstyle_t style;

    int tabstop;
    int left_margin;
    int right_margin;
    int cont_indent;
    cu_wstring_t cont_eol_insert;
    cu_wstring_t cont_bol_insert;
    cu_bool_t is_cont : 1;
};

CU_SINLINE int
cufo_textstream_width(cufo_textstream_t tos)
{ return tos->right_margin - tos->left_margin; }

/*!A \ref cufo_textstyle_s callback for a single tag. */
struct cufo_textstyler_s
{
    /* Node and key */
    cu_inherit (cucon_hzmap_node_s);
    cu_word_t tag;

    /* Value */
    cu_wstring_t (*enter)(cufo_textstream_t tos, cufo_tag_t tag, va_list attrs);
    cu_wstring_t (*leave)(cufo_textstream_t tos, cufo_tag_t tag);
};

/*!A style definition for a text stream. */
struct cufo_textstyle_s
{
    size_t stream_size;
    cu_clop(stream_init, void, cufo_textstream_t tos);
    struct cucon_hzmap_s tag_to_styler;
    cu_wstring_t (*default_enter)(cufo_textstream_t tos, cufo_tag_t tag,
				  va_list attrs);
    cu_wstring_t (*default_leave)(cufo_textstream_t tos, cufo_tag_t tag);
};

/*!Initialises \a style with the given stream size and stream initialiser.  \a
 * stream_size is the full size of the stream struct of some type derived from
 * \ref cufo_textstream_s.  At stream creation, the \ref cufo_textstream_s base
 * struct is first initialised, then \a stream_init is called to finish the
 * initialisation. */
void cufo_textstyle_init(cufo_textstyle_t style, size_t stream_size,
			 cu_clop(stream_init, void, cufo_textstream_t));

/*!Make a static declaration and partial initialisation of a \ref
 * cufo_textstyler_s struct using \a name as a prefix for the identifiers
 * involved.  This will define <i>name</i><tt>_styler</tt> from
 * <i>name</i><tt>_enter</tt> and <i>name</i><tt>_leave</tt>.  Association with
 * a tag is postponed to \a cufo_textstyle_bind_static, since the tags need to
 * be dynamically allocated. */
#define CUFO_TEXTSTYLER_STATIC(name) \
    static struct cufo_textstyler_s name##_styler \
	= {{}, 0, name##_enter, name##_leave}

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
