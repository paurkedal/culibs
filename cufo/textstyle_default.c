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
#include <cufo/tagdefs.h>
#include <cu/wstring.h>

typedef struct state_s *state_t;
struct state_s
{
    cu_inherit (cufo_textstate_s);
};
#define STATE(tstate) cu_from(state, cufo_textstate, tstate)

cu_clop_def(state_init, void, cufo_textstate_t state)
{
}

cu_wstring_t
indent_enter(cufo_textstate_t state, cufo_tag_t tag, va_list attrs)
{
    state->left_margin += 4;
    return NULL;
}

cu_wstring_t
indent_leave(cufo_textstate_t state, cufo_tag_t tag)
{
    state->left_margin -= 4;
    return NULL;
}

cu_wstring_t
codepre_enter(cufo_textstate_t state, cufo_tag_t tag, va_list attrs)
{
    state->cont_eol_insert = CU_WSTRING_C(" \\");
    state->cont_indent = 8;
    return NULL;
}

cu_wstring_t
codepre_leave(cufo_textstate_t state, cufo_tag_t tag)
{
    state->cont_eol_insert = NULL;
    state->cont_indent = 0;
    return NULL;
}

CUFO_TEXTSTYLER_STATIC(indent);
CUFO_TEXTSTYLER_STATIC(codepre);

struct cufo_textstyle_s cufoP_default_textstyle;

void
cufoP_textstyle_default_init(void)
{
    cufo_textstyle_init(&cufoP_default_textstyle, sizeof(struct state_s),
			cu_clop_ref(state_init));
#define B(tag) \
	cufo_textstyle_bind_static(&cufoP_default_textstyle, \
				   cufoT_##tag, &tag##_styler)
    B(indent);
    B(codepre);
#undef B
}
