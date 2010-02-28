/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2009--2010  Petter Urkedal <paurkedal@eideticdew.org>
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

#ifndef CU_COMPAT
#  define CU_COMPAT 20090701
#endif

#if CU_COMPAT < 20091122 && !defined(cu_upcast)
#  define cu_upcast	cu_to
#  define cu_upcast2	cu_to2
#  define cu_upcast3	cu_to3
#  define cu_upcast4	cu_to4
#  define cu_upcast5	cu_to5
#  define cu_upcast_virtual cu_to_virtual
#  define cu_downcast	cu_from
#  define cu_downcast2	cu_from2
#  define cu_downcast3	cu_from3
#  define cu_downcast4	cu_from4
#  define cu_downcast5	cu_from5
#  define cu_buffer_s		cu_buffer
#  define cu_dlink_s		cu_dlink
#  define cu_dbufsink_s		cu_dbufsink
#  define cu_dcountsink_s	cu_dcountsink
#  define cu_dsink_s		cu_dsink
#  define cu_dsource_s		cu_dsource
#  define cu_idr_s		cu_idr
#  define cu_log_facility_s	cu_log_facility
#  define cu_ptr_array_source_s	cu_ptr_array_source
#  define cu_ptr_source_s	cu_ptr_source
#  define cu_ptr_sink_s		cu_ptr_sink
#  define cu_ptr_junction_s	cu_ptr_junction
#  define cu_ptr_sinktor_s	cu_ptr_sinktor
#  define cu_ptr_junctor_s	cu_ptr_junctor
#  define cu_sref_s		cu_sref
#  define cu_str_s		cu_str
#  define cu_wstring_s		cu_wstring
#  define cu_hook_node_s	cu_hook_node
#  define cu_installdirs_s	cu_installdirs
#  define cu_hidden_ptr_s	cu_hidden_ptr
#  define cu_scratch_s		cu_scratch
#endif

#if CU_COMPAT < 20100228 && !defined(cufo_print_sref)
#  define cufo_print_sref	cufo_print_location
#endif
