/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2002--2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CU_SREF_H
#define CU_SREF_H

#include <stdio.h>
#include <cu/fwd.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cu_sref cu/sref.h: Source Code References
 * @{ \ingroup cu_util_mod
 * This defines a datatype which can hold a reference to a point or a range of
 * a file, with either line-precision or column-precision.
 */

struct cu_sref_s
{
    cu_sref_t chain_tail;
    cu_str_t path;
    int line, last_line;
    short column, last_column;
    int tabstop;
};

/*!Create a source file point reference.  <tt>\a path = NULL</tt> means stdin.
 * \a column may be -1, in which case the reference has line-precision. */
void cu_sref_init(cu_sref_t, cu_str_t path, int line, int column);

/*!\copydoc cu_sref_init */
cu_sref_t cu_sref_new(cu_str_t path, int line, int column);

/*!\copydoc cu_sref_init */
cu_sref_t cu_sref_new_cstr(char const *path, int line, int column);

/*!Create a source file range reference.
 * \pre \a first_column and \a last_column must either be both -1 (for
 * line-precision) or both positive (for column precision). */
void cu_sref_init_range(cu_sref_t, cu_str_t path,
			int first_line, int first_column,
			int last_line, int last_column);

/*!\copydoc cu_sref_init_range */
cu_sref_t cu_sref_new_range(cu_str_t path,
			    int first_line, int first_column,
			    int last_line, int last_column);

/*!Create a source file range reference from two source file point references.
 * \pre \a first and \a last must be point references within the same file,
 * and have the same precision and tabstop. */
void cu_sref_init_span(cu_sref_t, cu_sref_t first, cu_sref_t last);

/*!\copydoc cu_sref_init_span */
cu_sref_t cu_sref_new_span(cu_sref_t first, cu_sref_t last);

/*!Construct \a srf as a point reference to the start of \a src. */
void cu_sref_init_first(cu_sref_t srf, cu_sref_t src);

/*!Construct \a srf as a point reference to the end of \a src. */
void cu_sref_init_last(cu_sref_t srf, cu_sref_t src);

/*!Set the start of \a srf to \a first. */
void cu_sref_set_sref_first(cu_sref_t srf, cu_sref_t first);

/*!Set the end of \a srf to \a last. */
void cu_sref_set_sref_last(cu_sref_t srf, cu_sref_t last);

cu_bool_t cu_sref_compatible(cu_sref_t srf0, cu_sref_t srf1);

/*!True iff \a srf is a point reference. */
CU_SINLINE cu_bool_t cu_sref_is_point(cu_sref_t srf)
{ return srf->last_line < 0; }

/*!True iff \a srf is a range reference. */
CU_SINLINE cu_bool_t cu_sref_is_range(cu_sref_t srf)
{ return srf->last_line >= 0; }

/*!Modify \a new_head to be the head of a chain of source references
 * which continues with \a old_chain. */
CU_SINLINE void
cu_sref_set_chain_tail(cu_sref_t head, cu_sref_t chain_tail)
{ head->chain_tail = chain_tail; }

/*!The next in a chain of references. */
CU_SINLINE cu_sref_t
cu_sref_chain_tail(cu_sref_t srf) { return srf->chain_tail; }

/*!Change the width of tabulator stops. The default is 8. */
void cu_sref_set_tabstop(cu_sref_t, int w);

/*!Return an unknown source reference. */
CU_SINLINE cu_sref_t
cu_sref_unknown(void) { return NULL; }

/*!Return non-zero iff \a srf is known. */
CU_SINLINE int
cu_sref_is_known(cu_sref_t srf) { return srf != NULL; }

/*!Construct \a srf as a copy af \a srf0.  Chained references will
 * be shared, so make sure you only modify \a srf itself. */
void cu_sref_init_copy(cu_sref_t srf, cu_sref_t srf0);

/*!Return a copy of \a srf0. */
cu_sref_t cu_sref_new_copy(cu_sref_t);

/* Return non-zero iff \a srf is defined. */
/* int cu_sref_is_def(cu_sref_t srf); */
/* #define cu_sref_is_def(srf) ((srf)->line >= 0) */

/*!Return the path of \a srf. */
CU_SINLINE cu_str_t
cu_sref_path(cu_sref_t srf) { return srf->path; }

/*!Return non-zero iff \a srf refers to standard input. */
CU_SINLINE int
cu_sref_is_stdin(cu_sref_t srf) { return srf->path == NULL; }

/*!Return the line number of \a srf. */
CU_SINLINE int
cu_sref_line(cu_sref_t srf) { return srf->line; }

/*!Return the column number of \a srf. */
CU_SINLINE int
cu_sref_column(cu_sref_t srf) { return srf->column; }

/*!Advance the column number by 1. */
CU_SINLINE void
cu_sref_next(cu_sref_t srf) { ++srf->column; }

/*!Advance the column number to that after a tab at the current
 * column. */
void cu_sref_tab(cu_sref_t);

/*!Advance the line number by 1 and reset the column number to 0. */
void cu_sref_newline(cu_sref_t);

/*!Advance \a srf according to \a ch. */
CU_SINLINE void
cu_sref_advance_char(cu_sref_t srf, char ch)
{
    switch (ch) {
	case '\n': cu_sref_newline(srf); break;
	case '\t': cu_sref_tab(srf); break;
	default: if ((unsigned char)ch <= 127) cu_sref_next(srf); break;
    }
}

/*!Advance \a sref with \a cnt columns. */
CU_SINLINE void
cu_sref_advance_columns(cu_sref_t sref, int cnt) { sref->column += cnt; }

/*!Print the source reference \a file in the format file_name:line
 * or, if column is defined, file_name:line:column. */
void cu_sref_fprint(cu_sref_t, FILE* file);

/*!\deprecated Use \ref cu_sref_init. */
#define cu_sref_cct		cu_sref_init
/*!\deprecated Use \ref cu_sref_init_range. */
#define cu_sref_cct_range	cu_sref_init_range
/*!\deprecated Use \ref cu_sref_init_span. */
#define cu_sref_cct_sref_range	cu_sref_init_span
/*!\deprecated Use \ref cu_sref_init_first. */
#define cu_sref_cct_sref_first	cu_sref_init_first
/*!\deprecated Use \ref cu_sref_init_last. */
#define cu_sref_cct_sref_last	cu_sref_init_last
/*!\deprecated Use \ref cu_sref_init_copy. */
#define cu_sref_cct_copy	cu_sref_init_copy
/*!\deprecated Use \ref cu_sref_new_span. */
#define cu_serf_new_sref_range	cu_sref_new_span

/*!@}*/
CU_END_DECLARATIONS
#endif
