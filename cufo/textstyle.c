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

#include <cufo/textstyle.h>
#include <cucon/hzmap.h>

void
cufo_textstyle_init(cufo_textstyle_t style, size_t state_size,
		    cu_clop(state_init, void, cufo_textstate_t state))
{
    style->state_size = state_size;
    style->state_init = state_init;
    cucon_hzmap_init(&style->tag_to_styler, 1);
}

void
cufo_textstyle_bind_static(cufo_textstyle_t style,
			   cufo_tag_t tag, cufo_textstyler_t styler)
{
    styler->tag = (cu_word_t)tag;
    cucon_hzmap_insert_node(&style->tag_to_styler,
			    cu_to(cucon_hzmap_node, styler));
}

