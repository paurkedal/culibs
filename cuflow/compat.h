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

#if CU_COMPAT < 20091122 && !defined(cuflow_tstate_s)
#  define cuflow_tstate_s	cuflow_tstate
#  define cuflow_continuation_s	cuflow_continuation
#  define cuflow_cacheconf_s	cuflow_cacheconf
#  define cuflow_exeq_s		cuflow_exeq
#  define cuflow_exeq_entry_s	cuflow_exeq_entry
#  define cuflow_gflexq_s	cuflow_gflexq
#  define cuflow_gflexq_entry_s	cuflow_gflexq_entry
#  define cuflow_promise_s	cuflow_promise
#  define cuflow_workq_s	cuflow_workq
#  define cuflow_cache_s	cuflow_cache
#  define cuflow_cacheobj_s	cuflow_cacheobj
#endif
#if CU_COMPAT < 20091201 && !defined(cuflow_cacheconf_cct)
#  define cuflow_cacheconf_cct	cuflow_cacheconf_init
#  define cuflow_cache_cct	cuflow_cache_init
#  define cuflow_cache_dct	cuflow_cache_deinit
#endif
