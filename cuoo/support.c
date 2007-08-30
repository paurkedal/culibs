/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2007  Petter Urkedal <urkedal@nbi.dk>
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

#include <cuoo/support.h>
#include <cuoo/meta.h>
#include <cuoo/hcobj.h>
#include <cu/hash.h>
#include <cu/diag.h>


/* Predefined Hash Functions
 * ------------------------- */

cu_hash_t
cuoo_obj_hash_1w(void *obj)
{
    cuex_meta_t meta = cuex_meta(obj);
    cu_word_t *data = cu_ptr_add(obj, CUOO_HCOBJ_SHIFT);
    return cu_1word_hash_bj(data[0], meta);
}

cu_hash_t
cuoo_obj_hash_2w(void *obj)
{
    cuex_meta_t meta = cuex_meta(obj);
    cu_word_t *data = cu_ptr_add(obj, CUOO_HCOBJ_SHIFT);
    return cu_2word_hash_bj(data[0], data[1], meta);
}

#define DEFHF(COUNT) \
    cu_hash_t \
    cuoo_obj_hash_##COUNT##w(void *obj) \
    { \
	cuex_meta_t meta = cuex_meta(obj); \
	cu_word_t *data = cu_ptr_add(obj, CUOO_HCOBJ_SHIFT); \
	return cu_wordarr_hash_bj(COUNT, data, meta); \
    }
DEFHF(3) DEFHF(4) DEFHF(5) DEFHF(6) DEFHF(7) DEFHF(8)
cu_hash_t (*cuoo_obj_hash_arr[])(void *obj) = {
    cuoo_obj_hash_1w, cuoo_obj_hash_2w,
    cuoo_obj_hash_3w, cuoo_obj_hash_4w,
    cuoo_obj_hash_5w, cuoo_obj_hash_6w,
    cuoo_obj_hash_7w, cuoo_obj_hash_8w,
};

cu_hash_t (*cuoo_obj_hash_fn(size_t count))(void *)
{
    if (count == 0)
	cu_bugf("cuoo_obj_hash_fn called with count == 0.");
    if (count <= 8)
	cu_bugf("cuoo_obj_hash_fn currently only supports count <= 8.");
    return cuoo_obj_hash_arr[count - 1];
}
