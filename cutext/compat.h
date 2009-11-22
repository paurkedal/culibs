/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2009  Petter Urkedal <urkedal@nbi.dk>
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

#if CU_COMPAT < 20091122 && !defined(cutext_sink_s)
#  define cutext_sink_s		cutext_sink
#  define cutext_countsink_s	cutext_countsink
#  define cutext_buffersink_s	cutext_buffersink
#  define cutext_src_s		cutext_src
#  define cutext_ucs4src_s	cutext_ucs4src
#  define cutext_sink_descriptor_s cutext_sink_descriptor
#endif
