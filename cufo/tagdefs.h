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
/** \defgroup cufo_tagdefs_h cufo/tagdefs.h: Various Tag Definitions
 ** @{ \ingroup cufo_mod */

/** \name Generic Inline Tags
 ** @{ */

/** Delimits text to be emphasised. */
extern cufo_tag_t cufoT_emph;

/** Requests rendering in an italic font if available. */
extern cufo_tag_t cufoT_italic;

/** Requests rendering in a boldface font if available. */
extern cufo_tag_t cufoT_bold;

/** Requests underlining if available. */
extern cufo_tag_t cufoT_underline;

/** Delimits a superscript, commonly used for exponents, indices or labels. */
extern cufo_tag_t cufoT_superscript;

/** Delimits a subscript, commonly used for indices or labels. */
extern cufo_tag_t cufoT_subscript;

/** @}
 ** \name Generic Block Tags
 ** @{ */

/** Delimits a block to be set out from the surrounding text. */
extern cufo_tag_t cufoT_indent;

/** Recursive sectioning.  The depth may typically affect the rendering of \ref
 ** cufoT_title. */
extern cufo_tag_t cufoT_section;

/** Should occur as the first element of a section to indicate its title. */
extern cufo_tag_t cufoT_title;

/** Delimits a paragraph. */
extern cufo_tag_t cufoT_para;

/** @}
 ** \name Diagnostic Tags
 ** @{ */

/** Delimits a single log entry, as output with \ref cufo_logf and friends.
 ** This also applies to \ref cu_errf and friends after \ref cufo_init have
 ** installed its handlers. */
extern cufo_tag_t cufoT_logentry;

/** Delimits the source location part of a \ref cufoT_logentry. */
extern cufo_tag_t cufoT_location;

/** Delimits the message part of a \ref cufoT_logentry. */
extern cufo_tag_t cufoT_message;

/** @}
 ** \name Programmig Language Tags
 ** @{ */

/** Delimits block-level code quotation. */
extern cufo_tag_t cufoT_codepre;

/** Delimits inline code quotation. */
extern cufo_tag_t cufoT_code;

/** May be used to delimit a single declaration, statement, or similar logical
 ** unit of a program, depending on the language. */
extern cufo_tag_t cufoT_clause;

/** Delimits a comment. */
extern cufo_tag_t cufoT_comment;

/** Delimits a keyword. */
extern cufo_tag_t cufoT_keyword;

/** Delimits an operator. */
extern cufo_tag_t cufoT_operator;

/** Delimits a program variable. */
extern cufo_tag_t cufoT_variable;

/** Delimits a program datatype. */
extern cufo_tag_t cufoT_type;

/** Delimits a literal, like a number or a string. */
extern cufo_tag_t cufoT_literal;

/** Used inside \ref cufoT_literal to delimit escape sequences, string
 ** delimiters, or other non-literal parts of literals. */
extern cufo_tag_t cufoT_special;

/** @}
 ** @} */
CU_END_DECLARATIONS

#endif
