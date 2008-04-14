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

#include <cufo/stream.h>
#include <cuos/dsink.h>
#include <cu/dsink.h>

/* Strip Target */

cufo_stream_t
cufo_open_strip_fd(char const *encoding, int fd)
{
    cu_dsink_t subsink = cuos_dsink_fdopen(fd);
    if (!subsink)
	return NULL;
    return cufo_open_strip_sink(encoding, subsink);
}

cufo_stream_t
cufo_open_strip_file(char const *encoding, char const *path)
{
    cu_dsink_t subsink = cuos_dsink_open(path);
    if (!subsink)
	return NULL;
    return cufo_open_strip_sink(encoding, subsink);
}

cufo_stream_t
cufo_open_strip_str(void)
{
    cu_dsink_t subsink = cu_dsink_new_str();
    return cufo_open_strip_sink("UTF-8", subsink);
}

cufo_stream_t
cufo_open_strip_wstring(void)
{
    cu_dsink_t subsink = cu_dsink_new_wstring();
    return cufo_open_strip_sink(cu_wchar_encoding, subsink);
}

/* Text Target */

cufo_stream_t
cufo_open_text_fd(char const *encoding, int fd)
{
    cu_dsink_t subsink = cuos_dsink_fdopen(fd);
    if (!subsink)
	return NULL;
    return cufo_open_text_sink(encoding, subsink);
}

cufo_stream_t
cufo_open_text_file(char const *encoding, char const *path)
{
    cu_dsink_t subsink = cuos_dsink_open(path);
    if (!subsink)
	return NULL;
    return cufo_open_text_sink(encoding, subsink);
}

cufo_stream_t
cufo_open_text_str(void)
{
    cu_dsink_t subsink = cu_dsink_new_str();
    return cufo_open_text_sink("UTF-8", subsink);
}

cufo_stream_t
cufo_open_text_wstring(void)
{
    cu_dsink_t subsink = cu_dsink_new_wstring();
    return cufo_open_text_sink(cu_wchar_encoding, subsink);
}
