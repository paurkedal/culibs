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
#include <cutext/sink.h>
#include <cu/memory.h>
#include <cu/conf.h>
#include <unistd.h>
#ifdef CUCONF_HAVE_LANGINFO_H
#  include <langinfo.h>
#endif

cufo_stream_t
cufo_open_sink(cutext_sink_t sink)
{
    cufo_stream_t fos = cu_gnew(struct cufo_stream);
    char const *encoding = cutext_sink_encoding(sink);
    cufo_stream_init(fos, encoding, sink);
    return fos;
}

/* Strip Target */

cufo_stream_t
cufo_open_strip_fd(char const *encoding, int fd, cu_bool_t close_fd)
{
    cutext_sink_t subsink = cutext_sink_fdopen(encoding, fd, close_fd);
    if (!subsink)
	return NULL;
    return cufo_open_sink(subsink);
}

cufo_stream_t
cufo_open_strip_file(char const *encoding, char const *path)
{
    cutext_sink_t subsink = cutext_sink_fopen(encoding, path);
    if (!subsink)
	return NULL;
    return cufo_open_sink(subsink);
}

cufo_stream_t
cufo_open_strip_str(void)
{
    cutext_sink_t subsink = cutext_sink_new_str();
    return cufo_open_sink(subsink);
}

cufo_stream_t
cufo_open_strip_wstring(void)
{
    cutext_sink_t subsink = cutext_sink_new_wstring();
    return cufo_open_sink(subsink);
}

/* Text Target */

cufo_stream_t
cufo_open_text_fd(char const *encoding, cufo_textstyle_t style,
		  int fd, cu_bool_t close_fd)
{
    cutext_sink_t subsink = cutext_sink_fdopen(encoding, fd, close_fd);
    if (!subsink)
	return NULL;
    return cufo_open_text_sink(style, subsink);
}

cufo_stream_t
cufo_open_text_file(char const *encoding, cufo_textstyle_t style,
		    char const *path)
{
    cutext_sink_t subsink = cutext_sink_fopen(encoding, path);
    if (!subsink)
	return NULL;
    return cufo_open_text_sink(style, subsink);
}

cufo_stream_t
cufo_open_text_str(cufo_textstyle_t style)
{
    cutext_sink_t subsink = cutext_sink_new_str();
    return cufo_open_text_sink(style, subsink);
}

cufo_stream_t
cufo_open_text_wstring(cufo_textstyle_t style)
{
    cutext_sink_t subsink = cutext_sink_new_wstring();
    return cufo_open_text_sink(style, subsink);
}

/* Dummy TTY Target if not Enabled */

#ifndef CUCONF_ENABLE_TERM
cufo_stream_t
cufo_open_term_fd(char const *encoding, char const *term,
		  int fd, cu_bool_t close_fd)
{
    return cufo_open_text_fd(encoding, NULL, fd, close_fd);
}
#endif

cufo_stream_t
cufo_open_auto_fd(int fd, cu_bool_t close_fd)
{
    char const *encoding;
#if defined(CUCONF_HAVE_LANGINFO_H) && (CUCONF_HAVE_NL_LANGINFO)
    encoding = nl_langinfo(CODESET);
#else
    encoding = "UTF-8";
#endif

    /* Make a conservative guess whether to enable terminal capabilities. */
#if defined(CUCONF_ENABLE_TERM) && defined(CUCONF_HAVE_ISATTY)
    if (isatty(fd))
	return cufo_open_term_fd(encoding, NULL, fd, close_fd);
#endif

    return cufo_open_text_fd(encoding, NULL, fd, close_fd);
}
