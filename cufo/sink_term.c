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

#include <cufo/stream.h>
#include <cufo/termstyle.h>
#include <cufo/sink.h>
#include <cutext/sink.h>
#include <cu/thread.h>
#include <cu/str.h>
#ifdef HAVE_NCURSES_H
#  include <ncurses.h>
#else
#  include <curses.h>
#endif
#include <term.h>
#include <atomic_ops.h>

/* tparm seems to use global storage, so at least protect ourselves */
static pthread_mutex_t _curses_mutex = CU_MUTEX_INITIALISER;

static struct {
    char const *sitm;	/* start italics */
    char const *ritm;	/* end italics */
    char const *smul;	/* start underline */
    char const *rmul;	/* end underline */
    char const *bold;	/* start bold, end with sgr0 */
    char const *rev;	/* start reverse, end with sgr0 */
    char const *setf;	/* set forground colour */
    char const *setb;	/* set background colour */
    char const *sgr0;	/* reset bold, reverse, and colours */
} _tistr;

static char const *
_safe_tigetstr(char const *name)
{
    char const *s = tigetstr(name);
    if (s == (char const *)-1) {
	cu_warnf("tigetstr did not accept capability name \"%s\".", name);
	s = NULL;
    }
    return s;
}

#define FACE_SGR0_MASK (CUFO_TERMFACE_BOLD | CUFO_TERMFACE_REVERSE)

typedef struct _state_s *_state_t;
struct _state_s
{
    _state_t prev;
    unsigned int booleans;
    short fgcolour;
    short bgcolour;
};

static struct _state_s _initial_state = {NULL, 0, -1, -1};

typedef struct _termsink_s *_termsink_t;
struct _termsink_s
{
    cu_inherit (cutext_sink_s);
    cutext_sink_t subsink;

    cufo_termstyle_t style;
    _state_t current_state;
};

static void
_apply_tistr(_termsink_t sink, char const *tistr)
{
    if (tistr) {
	size_t tistr_len = strlen(tistr), wr_len;
	wr_len = cutext_sink_write(sink->subsink, tistr, tistr_len);
	cu_debug_assert(wr_len == tistr_len); /* clogfree */
    }
}

static void
_change_style(_termsink_t sink,
	      _state_t old_state, _state_t new_state)
{
    unsigned int old_bools = old_state->booleans;
    unsigned int new_bools = new_state->booleans;
    unsigned int attr_diff = old_bools ^ new_bools;
    cu_bool_t change_bg = new_state->bgcolour != old_state->bgcolour;
    cu_bool_t change_fg = new_state->fgcolour != old_state->fgcolour;
    cu_bool_t unset_bg = change_bg && new_state->bgcolour == -1;
    cu_bool_t unset_fg = change_fg && new_state->fgcolour == -1;
    if (unset_fg || unset_bg || (old_bools & ~new_bools & FACE_SGR0_MASK)) {
	_apply_tistr(sink, _tistr.sgr0);
	old_bools = 0;
	attr_diff = old_bools ^ new_bools;
	change_fg = new_state->fgcolour != -1;
	change_bg = new_state->bgcolour != -1;
    }
    if (change_bg || change_fg) {
	cu_mutex_lock(&_curses_mutex);
	if (change_bg && _tistr.setb)
	    _apply_tistr(sink, tparm(_tistr.setb, new_state->bgcolour));
	if (change_fg && _tistr.setf)
	    _apply_tistr(sink, tparm(_tistr.setf, new_state->fgcolour));
	cu_mutex_unlock(&_curses_mutex);
    }
    if (attr_diff & CUFO_TERMFACE_BOLD)
	_apply_tistr(sink, _tistr.bold);
    if (attr_diff & CUFO_TERMFACE_REVERSE)
	_apply_tistr(sink, _tistr.rev);
    if (attr_diff & CUFO_TERMFACE_ITALIC)
	_apply_tistr(sink, new_bools & CUFO_TERMFACE_ITALIC
		     ? _tistr.sitm : _tistr.ritm);
    if (attr_diff & CUFO_TERMFACE_UNDERLINE)
	_apply_tistr(sink, new_bools & CUFO_TERMFACE_UNDERLINE
		     ? _tistr.smul : _tistr.rmul);
}

static void
_push_face(_termsink_t sink, cufo_termface_t face)
{
    unsigned int enables = face->enables;
    _state_t old_state = sink->current_state;
    _state_t new_state = cu_gnew(struct _state_s);
    new_state->prev = old_state;
    new_state->booleans = (old_state->booleans | (enables & face->booleans))
		       & (~enables | face->booleans);
    new_state->fgcolour
	= enables & CUFO_TERMFACE_FGCOLOUR? face->fgcolour : old_state->fgcolour;
    new_state->bgcolour
	= enables & CUFO_TERMFACE_BGCOLOUR? face->bgcolour : old_state->bgcolour;
    _change_style(sink, old_state, new_state);
    sink->current_state = new_state;
}

static void
_pop_face(_termsink_t sink)
{
    _state_t old_state = sink->current_state;
    _state_t new_state = old_state->prev;
    _change_style(sink, old_state, new_state);
    CU_GWIPE(old_state->prev);
    sink->current_state = new_state;
}

static cufo_termstyle_t
_default_termstyle(void)
{
    static pthread_mutex_t mutex = CU_MUTEX_INITIALISER;
    static AO_t done_init = 0;
    static struct cufo_termstyle_s style;
    if (!AO_load_acquire_read(&done_init)) {
	char const *style_name;
	cu_mutex_lock(&mutex);
	cufo_termstyle_init(&style);
	style_name = getenv("CUFO_TERMSTYLE");
	if (style_name == NULL)
	    style_name = "default";
	if (!cufo_termstyle_loadinto(&style, cu_str_new_cstr(style_name)))
	    cu_warnf("Could not find style %s.", style_name);
	AO_store_release_write(&done_init, 1);
	cu_mutex_unlock(&mutex);
    }
    return &style;
}

static cu_bool_t
_termsink_enter(cutext_sink_t dsink, cufo_tag_t tag, cufo_attrbind_t attrbinds)
{
    _termsink_t tsink = cu_from(_termsink, cutext_sink, dsink);
    cufo_termface_t face = cufo_termstyle_get(tsink->style, tag);
    if (face) {
	_push_face(tsink, face);
	return cu_true;
    }
    else
	return cufo_sink_enter(tsink->subsink, tag, attrbinds);
}

static void
_termsink_leave(cutext_sink_t dsink, cufo_tag_t tag)
{
    _termsink_t tsink = cu_from(_termsink, cutext_sink, dsink);
    cufo_termface_t face = cufo_termstyle_get(tsink->style, tag);
    if (face)
	_pop_face(tsink);
    else
	cufo_sink_leave(tsink->subsink, tag);
}

static size_t
_termsink_write(cutext_sink_t dsink, void const *data, size_t len)
{
    _termsink_t tsink = cu_from(_termsink, cutext_sink, dsink);
    return cutext_sink_write(tsink->subsink, data, len);
}

static cu_bool_t
_termsink_iterA_subsinks(cutext_sink_t dsink, cu_clop(f, cu_bool_t, cutext_sink_t))
{
    _termsink_t tsink = cu_from(_termsink, cutext_sink, dsink);
    return cu_call(f, tsink->subsink);
}

static cu_box_t
_termsink_info(cutext_sink_t dsink, cutext_sink_info_key_t key)
{
    _termsink_t tsink = cu_from(_termsink, cutext_sink, dsink);
    switch (key) {
	case CU_DSINK_INFO_ENCODING:
	    return cutext_sink_info(tsink->subsink, key);
	default:
	    return cutext_sink_default_info(dsink, key);
    }
}

static struct cutext_sink_descriptor_s _termsink_descriptor = {
    CU_DSINK_DESCRIPTOR_DEFAULTS,
    .write = _termsink_write,
    .flags = CU_DSINK_FLAG_CLOGFREE | CU_DSINK_FLAG_CUFO_EXT,
    .iterA_subsinks = _termsink_iterA_subsinks,
    .info = _termsink_info,
    .enter = _termsink_enter,
    .leave = _termsink_leave
};

cutext_sink_t
cufo_termsink_new(cufo_termstyle_t termstyle, char const *term,
		  char const *encoding, int fd, cu_bool_t close_fd)
{
    static AO_t done_init = 0;
    _termsink_t sink;
    cutext_sink_t subsink;
    int err;

    if (close_fd)
	subsink = cutext_sink_fdopen_close(encoding, fd);
    else
	subsink = cutext_sink_fdopen(encoding, fd);
    cutext_sink_assert_clogfree(subsink);

    if (setupterm(term, fd, &err) != OK) {
	char const *err_msg;
	switch (err) {
	    case -1: err_msg = "terminfo database could not be found.";
	    case 0:  err_msg = "terminal type not found or too generic.";
	    case 1:  err_msg = "your terminal is a hardcopy.";
	    default: err_msg = "setupterm returned an unknown error code.";
	}
	cu_errf("Could not open terminal stream because %s", err_msg);
	return NULL;
    }
    if (!AO_load_acquire_read(&done_init)) {
	cu_mutex_lock(&_curses_mutex);
	_tistr.setf = _safe_tigetstr("setf");
	_tistr.setb = _safe_tigetstr("setb");
	_tistr.sitm = _safe_tigetstr("sitm");
	_tistr.ritm = _safe_tigetstr("ritm");
	_tistr.smul = _safe_tigetstr("smul");
	_tistr.rmul = _safe_tigetstr("rmul");
	_tistr.bold = _safe_tigetstr("bold");
	_tistr.rev  = _safe_tigetstr("rev");
	_tistr.sgr0 = _safe_tigetstr("sgr0");
	AO_store_release_write(&done_init, 1);
	cu_mutex_unlock(&_curses_mutex);
    }

    sink = cu_gnew(struct _termsink_s);
    cutext_sink_init(cu_to(cutext_sink, sink), &_termsink_descriptor);
    sink->subsink = subsink;
    sink->style = termstyle;
    sink->current_state = &_initial_state;

    return cu_to(cutext_sink, sink);
}

cufo_stream_t
cufo_open_term_fd(char const *encoding, char const *term, int fd)
{
    cufo_termstyle_t termstyle = _default_termstyle();
    cutext_sink_t termsink = cufo_termsink_new(termstyle, term, encoding, fd, cu_false);
    if (termsink)
	return cufo_open_text_sink(NULL, termsink);
    else
	return cufo_open_text_fd(encoding, NULL, fd);
}
