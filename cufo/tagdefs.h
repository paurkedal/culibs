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
extern cufo_tag_t cufo_i_emph;
extern cufo_tag_t cufo_i_italic;
extern cufo_tag_t cufo_i_bold;
extern cufo_tag_t cufo_i_underline;
extern cufo_tag_t cufo_i_superscript;
extern cufo_tag_t cufo_i_subscript;

/* Generic Blocks */
extern cufo_tag_t cufo_i_indent;
extern cufo_tag_t cufo_i_section;
extern cufo_tag_t cufo_b_title;
extern cufo_tag_t cufo_b_para;

/* Diagnostic */
extern cufo_tag_t cufo_i_location;
extern cufo_tag_t cufo_i_message;
extern cufo_tag_t cufo_b_runtime_info;
extern cufo_tag_t cufo_b_runtime_warning;
extern cufo_tag_t cufo_b_runtime_error;
extern cufo_tag_t cufo_b_bug_info;
extern cufo_tag_t cufo_b_bug_warning;
extern cufo_tag_t cufo_b_bug_error;
extern cufo_tag_t cufo_b_bug_trace;

/* Expressions */
extern cufo_tag_t cufo_b_codepre;
extern cufo_tag_t cufo_i_code;
extern cufo_tag_t cufo_i_comment;
extern cufo_tag_t cufo_i_keyword;
extern cufo_tag_t cufo_i_operator;
extern cufo_tag_t cufo_i_variable;
extern cufo_tag_t cufo_i_type;
extern cufo_tag_t cufo_i_literal;

/*!@}*/
CU_END_DECLARATIONS

#endif
