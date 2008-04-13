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

#ifndef CUFO_TAGDEFS_H
#define CUFO_TAGDEFS_H

#include <cufo/tag.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cufo_tagdefs_h cufo/tagdefs.h: Predefined Tags
 *@{\ingroup cufo_mod */

/* Generic Inline */
extern cufo_tag_t cufoT_emph;
extern cufo_tag_t cufoT_italic;
extern cufo_tag_t cufoT_bold;
extern cufo_tag_t cufoT_underline;
extern cufo_tag_t cufoT_superscript;
extern cufo_tag_t cufoT_subscript;

/* Generic Blocks */
extern cufo_tag_t cufoT_indent;
extern cufo_tag_t cufoT_section;
extern cufo_tag_t cufoT_title;
extern cufo_tag_t cufoT_para;

/* Diagnostic */
extern cufo_tag_t cufoT_location;
extern cufo_tag_t cufoT_message;
extern cufo_tag_t cufoT_runtime_info;
extern cufo_tag_t cufoT_runtime_warning;
extern cufo_tag_t cufoT_runtime_error;
extern cufo_tag_t cufoT_bug_info;
extern cufo_tag_t cufoT_bug_warning;
extern cufo_tag_t cufoT_bug_error;
extern cufo_tag_t cufoT_bug_trace;

/* Expressions */
extern cufo_tag_t cufoT_codepre;
extern cufo_tag_t cufoT_code;
extern cufo_tag_t cufoT_comment;
extern cufo_tag_t cufoT_keyword;
extern cufo_tag_t cufoT_operator;
extern cufo_tag_t cufoT_variable;
extern cufo_tag_t cufoT_type;
extern cufo_tag_t cufoT_literal;

/*!@}*/
CU_END_DECLARATIONS

#endif
