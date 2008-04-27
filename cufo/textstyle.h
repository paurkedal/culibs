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

#ifndef CUFO_TEXTSTYLE_H
#define CUFO_TEXTSTYLE_H

#include <cufo/fwd.h>
#include <cucon/hzmap.h>
#include <cu/inherit.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cufo_textstyle_h cufo/textstyle.h: Text Target Styler Support
 *@{\ingroup cufo_mod */

/*!Text stream state which can be modified by style callbacks. */
struct cufo_textstate_s
{
    int tabstop;
    int left_margin;
    int right_margin;
    int cont_indent;
    cu_wstring_t cont_eol_insert;
    cu_wstring_t cont_bol_insert;
};

CU_SINLINE int cufo_textstate_width(cufo_textstate_t ts)
{ return ts->right_margin - ts->left_margin; }

/*!A \ref cufo_textstyle_s callback for a single tag. */
struct cufo_textstyler_s
{
    /* Node and key */
    cu_inherit (cucon_hzmap_node_s);
    cu_word_t tag;

    /* Value */
    cu_wstring_t (*enter)(cufo_textstate_t state, cufo_tag_t tag,
			  va_list attrs);
    cu_wstring_t (*leave)(cufo_textstate_t state, cufo_tag_t tag);
};

/*!A style definition for a text stream. */
struct cufo_textstyle_s
{
    size_t state_size; /* state struct inherited from cufo_textstate_s */
    cu_clop(state_init, void, cufo_textstate_t state);
    struct cucon_hzmap_s tag_to_styler;
};

/*!Initialises \a style with the given state size and state initialisation
 * function.  \a state_size is the full size of the state information of a type
 * derived from or the same as \ref cufo_textstate_s.  At stream creation, the
 * base struct will be filled with default values, and then \a state_init is
 * called to do the full \a state_size bytes initialisation. */
void cufo_textstyle_init(cufo_textstyle_t style, size_t state_size,
			 cu_clop(state_init, void, cufo_textstate_t state));

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

CU_SINLINE cufo_textstyle_t cufo_default_textstyle(void)
{ return &cufoP_default_textstyle; }

/*!@}*/
CU_END_DECLARATIONS

#endif
