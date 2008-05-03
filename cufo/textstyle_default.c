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

#include <cufo/textstream.h>
#include <cufo/tagdefs.h>
#include <cu/wstring.h>

typedef struct dtx_stream_s *dtx_stream_t;
struct dtx_stream_s
{
    cu_inherit (cufo_textstream_s);
};
#define STATE(tstate) cu_from(dtx_stream, cufo_textstream, tstate)

cu_clop_def(stream_init, void, cufo_textstream_t tos)
{
}

cu_wstring_t
indent_enter(cufo_textstream_t tos, cufo_tag_t tag, va_list attrs)
{
    tos->left_margin += 4;
    return NULL;
}

cu_wstring_t
indent_leave(cufo_textstream_t tos, cufo_tag_t tag)
{
    tos->left_margin -= 4;
    return NULL;
}

cu_wstring_t
codepre_enter(cufo_textstream_t tos, cufo_tag_t tag, va_list attrs)
{
    tos->cont_eol_insert = CU_WSTRING_C(" \\");
    tos->cont_indent = 8;
    return NULL;
}

cu_wstring_t
codepre_leave(cufo_textstream_t tos, cufo_tag_t tag)
{
    if (tos->is_cont) {
	cufo_newline(cu_to(cufo_stream, tos));
	cufo_flush(cu_to(cufo_stream, tos));
    }
    tos->cont_eol_insert = NULL;
    tos->cont_indent = 0;
    return NULL;
}

CUFO_TEXTSTYLER_STATIC(indent);
CUFO_TEXTSTYLER_STATIC(codepre);

struct cufo_textstyle_s cufoP_default_textstyle;

void
cufoP_textstyle_default_init(void)
{
    cufo_textstyle_init(&cufoP_default_textstyle, sizeof(struct dtx_stream_s),
			cu_clop_ref(stream_init));
#define B(tag) \
	cufo_textstyle_bind_static(&cufoP_default_textstyle, \
				   cufoT_##tag, &tag##_styler)
    B(indent);
    B(codepre);
#undef B
}
