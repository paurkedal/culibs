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

#include <cufo/textsink.h>
#include <cufo/tagdefs.h>
#include <cu/wstring.h>

typedef struct _dtx_sink_s *_dtx_sink_t;
struct _dtx_sink_s
{
    cu_inherit (cufo_textsink_s);
};
#define STATE(tstate) cu_from(_dtx_sink, cufo_textsink, tstate)

cu_clop_def(_stream_init, void, cufo_textsink_t sink)
{
}

static cu_wstring_t
_indent_enter(cufo_textsink_t sink, cufo_tag_t tag, cufo_attrbind_t attrbinds)
{
    sink->left_margin += 4;
    return NULL;
}

static cu_wstring_t
_indent_leave(cufo_textsink_t sink, cufo_tag_t tag)
{
    sink->left_margin -= 4;
    return NULL;
}

static cu_wstring_t
_codepre_enter(cufo_textsink_t sink, cufo_tag_t tag, cufo_attrbind_t attrbinds)
{
    sink->cont_eol_insert = CU_WSTRING_C(" \\");
    sink->cont_indent = 8;
    cufo_textsink_block_boundary(sink);
    return NULL;
}

static cu_wstring_t
_codepre_leave(cufo_textsink_t sink, cufo_tag_t tag)
{
    cufo_textsink_block_boundary(sink);
    sink->cont_eol_insert = NULL;
    sink->cont_indent = 0;
    return NULL;
}

CUFO_TEXTSTYLER_STATIC(_indent);
CUFO_TEXTSTYLER_STATIC(_codepre);

struct cufo_textstyle_s cufoP_default_textstyle;

void
cufoP_textstyle_default_init(void)
{
    cufo_textstyle_init(&cufoP_default_textstyle, sizeof(struct _dtx_sink_s),
			cu_clop_ref(_stream_init));
#define B(tag) \
	cufo_textstyle_bind_static(&cufoP_default_textstyle, \
				   cufoT_##tag, &_##tag##_styler)
    B(indent);
    B(codepre);
#undef B
}
