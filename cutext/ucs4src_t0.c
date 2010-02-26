/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2007--2010  Petter Urkedal <urkedal@nbi.dk>
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

#include <cutext/ucs4src.h>
#include <cutext/wctype.h>
#include <cucon/pmap.h>

cu_clop_def(print_prop, void, void const *key, void *value)
{
    cu_verbf(0, "%s = %S", (char const *)key, (cu_str_t)value);
}

int
main()
{
    cutext_producer_t producer;
    cutext_ucs4src_t ucs4src;
    cucon_pmap_t props;

    cutext_init();

    producer = cutext_producer_new_read(0);
    ucs4src = cutext_ucs4src_new_detect(producer,
					cu_str_new_cstr("(stdin)"), 1, 0);
    props = cutext_ucs4src_properties(ucs4src);
    if (props)
	cucon_pmap_iter_ptr(props, print_prop);
    while (!cutext_ucs4src_is_eof(ucs4src)) {
	cu_wchar_t ch = cutext_ucs4src_peek(ucs4src);
	if (cutext_iswprint(ch))
	    cu_errf_at(cutext_ucs4src_sref(ucs4src), "%jc 0x%x", ch, ch);
	else
	    cu_errf_at(cutext_ucs4src_sref(ucs4src), "? 0x%x", ch, ch);
	cutext_ucs4src_advance(ucs4src, 1);
    }
    return 0;
}
