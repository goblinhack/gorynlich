/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license.
 */

#include "slre.h"
#include <SDL.h>

#if SDL_MAJOR_VERSION >= 2
#include "SDL_messagebox.h"
#endif

#include "main.h"
#include "backtrace.h"
#include "wid_console.h"
#include "time_util.h"
#include "command.h"
#include "thing.h"
#include "level.h"
#include "timer.h"
#include "init_fn.h"
#include "wid.h"
#include "term.h"
#include "wid_popup.h"
#include "wid_tooltip.h"
#include "socket_util.h"
#include "wid_notify.h"
#include "sound.h"
#include "wid_game_map_client.h"

uint8_t debug_enabled = 0;
uint8_t croaked;

/*
 * putfg
 */
static void putfg (uint8_t fg, FILE *fp)
{
    static const char *data[] = {
            "\033[1;30m", "\033[1;31m", "\033[1;32m", "\033[1;33m",
            "\033[1;34m", "\033[1;35m", "\033[1;36m", "\033[1;37m",
            "\033[m",
    };

    fputs(data[fg], fp);
}

/*
 * putbg
 */
static void putbg (uint8_t bg, FILE *fp)
{
    static const char *data[] = {
            "\033[1;40m", "\033[1;41m", "\033[1;42m", "\033[1;43m",
            "\033[1;44m", "\033[1;45m", "\033[1;46m", "\033[1;47m",
            "\033[m",
    };

    fputs(data[bg], fp);
}

static int color_to_index (const char **s)
{
    if (!strncmp(*s, "black$", sizeof("black$")-1)) {
	*s += sizeof("black$")-1;
	return (TERM_COLOR_BLACK);
    }
    if (!strncmp(*s, "red$", sizeof("red$")-1)) {
	*s += sizeof("red$")-1;
	return (TERM_COLOR_RED);
    }
    if (!strncmp(*s, "green$", sizeof("green$")-1)) {
	*s += sizeof("green$")-1;
	return (TERM_COLOR_GREEN);
    }
    if (!strncmp(*s, "yellow$", sizeof("yellow$")-1)) {
	*s += sizeof("yellow$")-1;
	return (TERM_COLOR_YELLOW);
    }
    if (!strncmp(*s, "blue$", sizeof("blue$")-1)) {
	*s += sizeof("blue$")-1;
	return (TERM_COLOR_BLUE);
    }
    if (!strncmp(*s, "pink$", sizeof("pink$")-1)) {
	*s += sizeof("pink$")-1;
	return (TERM_COLOR_PINK);
    }
    if (!strncmp(*s, "cyan$", sizeof("cyan$")-1)) {
	*s += sizeof("cyan$")-1;
	return (TERM_COLOR_CYAN);
    }
    if (!strncmp(*s, "white$", sizeof("white$")-1)) {
	*s += sizeof("white$")-1;
	return (TERM_COLOR_WHITE);
    }
    if (!strncmp(*s, "reset$", sizeof("reset$")-1)) {
	*s += sizeof("reset$")-1;
	return (TERM_COLOR_RESET);
    }

    return (TERM_COLOR_WHITE);
}

static void putf (FILE *fp, const char *s)
{
    char c;
    uint8_t looking_for_start = false;

    while ((c = *s++) != '\0') {
	if (!looking_for_start) {
	    if (c == '%') {
		looking_for_start = true;
		continue;
	    }
	} else if (looking_for_start) {
	    if (c == '%') {
		if (!strncmp(s, "fg=", 3)) {
		    s += 3;
		    putfg(color_to_index(&s), fp);
		    looking_for_start = false;
		    continue;
		}

		if (!strncmp(s, "bg=", 3)) {
		    s += 3;
		    putbg(color_to_index(&s), fp);
		    looking_for_start = false;
		    continue;
		}
	    }

	    putc(c, fp);
	}

	looking_for_start = false;

	putc(c, fp);
    }

    putc('\n', fp);
}

static void msg_ (uint32_t level, 
                  uint32_t thing_id,
                  const char *fmt, va_list args)
{
    char buf[MAXSTR];
    uint32_t len;

    if (level == SOUND) {
        vsnprintf(buf, sizeof(buf), fmt, args);
        sound_play(buf);
        return;
    }

    buf[0] = '\0';
    timestamp(buf, sizeof(buf));
    len = (uint32_t)strlen(buf);
    vsnprintf(buf + len, sizeof(buf) - len, fmt, args);

    putf(MY_STDOUT, buf);
    fflush(MY_STDOUT);

    if (level == POPUP) {
        widp w;

        w = wid_tooltip_transient(buf + len, 3 * ONESEC);
        wid_move_to_pct_centered(w, 0.5, -0.1);
        wid_move_to_pct_centered_in(w, 0.5, 0.1, ONESEC / 2);

        return;
    } 
    
    if (wid_notify(level, buf + len)) {
        wid_console_log(buf + len);

#if 0
        if (level == CHAT) {
            wid_chat_log(buf + len);
        }
#endif
        term_log(buf + len);
    }
}

static void log_ (const char *fmt, va_list args)
{
    char buf[MAXSTR];
    uint32_t len;

    buf[0] = '\0';
    timestamp(buf, sizeof(buf));
    len = (uint32_t)strlen(buf);
    vsnprintf(buf + len, sizeof(buf) - len, fmt, args);

    putf(MY_STDOUT, buf);
    fflush(MY_STDOUT);
}

void LOG (const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    log_(fmt, args);
    va_end(args);
}

static void logs_ (const char *fmt, va_list args)
{
    char buf[MAXSTR];

    buf[0] = '\0';
    vsnprintf(buf, sizeof(buf), fmt, args);

    fwrite(buf, strlen(buf), 1, MY_STDOUT);
    fflush(MY_STDOUT);
}

void LOGS (const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    logs_(fmt, args);
    va_end(args);
}

static void warn_ (const char *fmt, va_list args)
{
    char buf[MAXSTR];
    uint32_t len;

    buf[0] = '\0';
    timestamp(buf, sizeof(buf));
    len = (uint32_t)strlen(buf);
    vsnprintf(buf + len, sizeof(buf) - len, fmt, args);

    putf(MY_STDOUT, buf);
    fflush(MY_STDOUT);

    wid_console_log(buf);
    term_log(buf);
}

void WARN (const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    warn_(fmt, args);
    va_end(args);
}

static void init_log_ (const char *fmt, va_list args)
{
    char buf[MAXSTR];
    uint32_t len;

    buf[0] = '\0';
    timestamp(buf, sizeof(buf));
    len = (uint32_t)strlen(buf);

    snprintf(buf + len, sizeof(buf) - len, "Init ");

    len = (uint32_t)strlen(buf);
    vsnprintf(buf + len, sizeof(buf) - len, fmt, args);

    putf(MY_STDOUT, buf);
    fflush(MY_STDOUT);
}

void INIT_LOG (const char *fmt, ...)
{
    va_list args;

    if (!debug_enabled) {
        return;
    }

    va_start(args, fmt);
    init_log_(fmt, args);
    va_end(args);
}

static void fini_log_ (const char *fmt, va_list args)
{
    char buf[MAXSTR];
    uint32_t len;

    buf[0] = '\0';
    timestamp(buf, sizeof(buf));
    len = (uint32_t)strlen(buf);

    snprintf(buf + len, sizeof(buf) - len, "Fini  ");

    len = (uint32_t)strlen(buf);
    vsnprintf(buf + len, sizeof(buf) - len, fmt, args);

    putf(MY_STDOUT, buf);
    fflush(MY_STDOUT);
}

void FINI_LOG (const char *fmt, ...)
{
    va_list args;

    if (!debug_enabled) {
        return;
    }

    va_start(args, fmt);
    fini_log_(fmt, args);
    va_end(args);
}

static void con_ (const char *fmt, va_list args)
{
    char buf[MAXSTR];
    uint32_t len;

    buf[0] = '\0';
    timestamp(buf, sizeof(buf));
    len = (uint32_t)strlen(buf);
    vsnprintf(buf + len, sizeof(buf) - len, fmt, args);

    putf(MY_STDOUT, buf);
    fflush(MY_STDOUT);

    wid_console_log(buf + len);
    term_log(buf + len);
}

static void dying_ (const char *fmt, va_list args)
{
    char buf[MAXSTR];
    uint32_t len;

    buf[0] = '\0';
    timestamp(buf, sizeof(buf));
    len = (uint32_t)strlen(buf);

    snprintf(buf + len, sizeof(buf) - len, "DYING: ");

    len = (uint32_t)strlen(buf);
    vsnprintf(buf + len, sizeof(buf) - len, fmt, args);

    putf(MY_STDOUT, buf);
    fflush(MY_STDOUT);
}

static void err_ (const char *fmt, va_list args)
{
    char buf[MAXSTR];
    uint32_t len;

    buf[0] = '\0';
    timestamp(buf, sizeof(buf));
    len = (uint32_t)strlen(buf);

    snprintf(buf + len, sizeof(buf) - len, "ERROR: %%%%fg=red$");

    len = (uint32_t)strlen(buf);
    vsnprintf(buf + len, sizeof(buf) - len, fmt, args);

    len = (uint32_t)strlen(buf);
    snprintf(buf + len, sizeof(buf) - len, "%%%%fg=reset$");

    putf(MY_STDERR, buf);
    fflush(MY_STDERR);

    putf(MY_STDOUT, buf);
    fflush(MY_STDOUT);

    wid_console_log(buf);
    term_log(buf);

    backtrace_print();
    fflush(MY_STDOUT);
}

static void croak_ (const char *fmt, va_list args)
{
    char buf[MAXSTR];
    uint32_t len;
    uint32_t tslen;

    buf[0] = '\0';
    timestamp(buf, sizeof(buf));
    tslen = len = (uint32_t)strlen(buf);

    snprintf(buf + len, sizeof(buf) - len, "FATAL ERROR: ");

    len = (uint32_t)strlen(buf);
    vsnprintf(buf + len, sizeof(buf) - len, fmt, args);

    fprintf(stderr, "%s\n", buf);
    fflush(stderr);

    ERR("%s", buf + tslen);

    if (croaked) {
        return;
    }

    croaked = true;

    die();
}

void CON (const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    con_(fmt, args);
    va_end(args);
}

void DYING (const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    dying_(fmt, args);
    va_end(args);
}

void DBG (const char *fmt, ...)
{
    va_list args;

    if (!debug_enabled) {
        return;
    }

    va_start(args, fmt);
    log_(fmt, args);
    va_end(args);
}

void ERR (const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    err_(fmt, args);
    va_end(args);
}

void CROAK (const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    croak_(fmt, args);
    va_end(args);

    quit();
}

static void msg_server_shout_at_all_players_ (uint32_t level,
                                              const char *fmt, va_list args)
{
    char buf[MAXSTR];
    uint32_t len;

    buf[0] = '\0';
    timestamp(buf, sizeof(buf));
    len = (uint32_t)strlen(buf);
    vsnprintf(buf + len, sizeof(buf) - len, fmt, args);

    putf(MY_STDOUT, buf);
    fflush(MY_STDOUT);

    socket_tx_server_shout_at_all_players(level, buf + len);
}

void MSG_SERVER_SHOUT_AT_ALL_PLAYERS (uint32_t level,
                                      const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    msg_server_shout_at_all_players_(level, fmt, args);
    va_end(args);
}

static void msg_server_shout_at_player_ (uint32_t level,
                                         thingp t,
                                         const char *fmt, va_list args)
{
    char buf[MAXSTR];
    uint32_t len;

    buf[0] = '\0';
    timestamp(buf, sizeof(buf));
    len = (uint32_t)strlen(buf);
    if (t->on_server) {
        snprintf(buf + len, sizeof(buf) - len, "Server: Thing %s: Shout: ", 
                 thing_logname(t));
    } else {
        snprintf(buf + len, sizeof(buf) - len, "Client: Thing %s: Shout: ", 
                 thing_logname(t));
    }

    len = (uint32_t)strlen(buf);
    vsnprintf(buf + len, sizeof(buf) - len, fmt, args);

    putf(MY_STDOUT, buf);
    fflush(MY_STDOUT);

    if (!t->player) {
        return;
    }

    gsocketp s = t->player->socket;
    if (!s) {
        return;
    }

    socket_tx_server_shout_only_to(s, level, buf + len);
}

void MSG_SERVER_SHOUT_AT_PLAYER (uint32_t level,
                                 thingp t,
                                 const char *fmt, ...)
{
    va_list args;

    verify(t);

    va_start(args, fmt);
    msg_server_shout_at_player_(level, t, fmt, args);
    va_end(args);
}

static void msg_over_thing_ (uint32_t level,
                             uint32_t thing_id,
                             const char *fmt, va_list args)
{
    char buf[MAXSTR];

    thingp t = thing_client_find(thing_id);
    if (!t) {
        return;
    }

    if (level == SOUND) {
        vsnprintf(buf, sizeof(buf), fmt, args);
        sound_play(buf);
        return;
    }

    widp wid_thing = thing_wid(t);
    if (!wid_thing) {
        return;
    }

    vsnprintf(buf, sizeof(buf), fmt, args);

    /*
     * Center a widget over the thing.
     */
    fpoint tl, br;

    wid_get_tl_br(wid_thing, &tl, &br);
    widp w = wid_new_container(wid_game_map_client_grid_container, "wid_tooltip");

    wid_set_tl_br_no_relative_offset(w, tl, br);
    wid_set_text(w, buf);
    wid_set_text_outline(w, true);
    wid_set_font(w, med_font);
    wid_move_delta_pct_in(w, 0.0, -0.05, 0);
    wid_move_end(w);
    wid_move_delta_pct_in(w, 0.0, -0.1, 1500);

    if (level == POPUP) {
        wid_fade_out(w, 1500);
        wid_destroy_in(w, 1500);
    } else {
        wid_fade_out(w, 3000);
        wid_destroy_in(w, 3000);
    }

    wid_set_no_shape(w);
    wid_set_z_depth(w, MAP_DEPTH_ACTIONS);
    wid_raise(w);

    if (level == POPUP) {
        return;
    }

    if (wid_notify(level, buf)) {
        wid_console_log(buf);

        term_log(buf);
    }
}

void MSG_CLIENT_SHOUT_OVER_PLAYER (uint32_t level, 
                                   uint32_t thing_id,
                                   const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    msg_over_thing_(level, thing_id, fmt, args);
    va_end(args);
}

static void msg_server_shout_over_thing_ (uint32_t level,
                                          thingp t,
                                          const char *fmt, va_list args)
{
    char buf[MAXSTR];

    vsnprintf(buf, sizeof(buf), fmt, args);

    socket_tx_server_shout_over(level, t->thing_id, buf);
}

void MSG_SERVER_SHOUT_OVER_THING (uint32_t level,
                                  thingp t,
                                  const char *fmt, ...)
{
    va_list args;

    verify(t);

    va_start(args, fmt);
    msg_server_shout_over_thing_(level, t, fmt, args);
    va_end(args);
}

static void thing_log_ (thingp t, const char *fmt, va_list args)
{
    char buf[MAXSTR];
    uint32_t len;

    buf[0] = '\0';
    timestamp(buf, sizeof(buf));
    len = (uint32_t)strlen(buf);
    if (t->on_server) {
        snprintf(buf + len, sizeof(buf) - len, "Server: Thing %s: ", 
                 thing_logname(t));
    } else {
        snprintf(buf + len, sizeof(buf) - len, "Client: Thing %s: ", 
                 thing_logname(t));
    }
    len = (uint32_t)strlen(buf);
    vsnprintf(buf + len, sizeof(buf) - len, fmt, args);

    putf(MY_STDOUT, buf);
    fflush(MY_STDOUT);
}

void THING_LOG (thingp t, const char *fmt, ...)
{
    va_list args;

    verify(t);

    va_start(args, fmt);
    thing_log_(t, fmt, args);
    va_end(args);
}

static void thing_con_ (thingp t, const char *fmt, va_list args)
{
    char buf[MAXSTR];
    uint32_t len;

    buf[0] = '\0';
    timestamp(buf, sizeof(buf));
    len = (uint32_t)strlen(buf);
    if (t->on_server) {
        snprintf(buf + len, sizeof(buf) - len, "Server: Thing %s: ", 
                 thing_logname(t));
    } else {
        snprintf(buf + len, sizeof(buf) - len, "Client: Thing %s: ", 
                 thing_logname(t));
    }
    len = (uint32_t)strlen(buf);
    vsnprintf(buf + len, sizeof(buf) - len, fmt, args);

    putf(MY_STDOUT, buf);
    fflush(MY_STDOUT);

    wid_console_log(buf);
    term_log(buf);
}

void THING_CON (thingp t, const char *fmt, ...)
{
    va_list args;

    verify(t);

    va_start(args, fmt);
    thing_con_(t, fmt, args);
    va_end(args);
}

static void thing_err_ (thingp t, const char *fmt, va_list args)
{
    char buf[MAXSTR];
    uint32_t len;

    buf[0] = '\0';
    timestamp(buf, sizeof(buf));
    len = (uint32_t)strlen(buf);
    if (t->on_server) {
        snprintf(buf + len, sizeof(buf) - len, "ERROR: Server: Thing %s: ", 
                 thing_logname(t));
    } else {
        snprintf(buf + len, sizeof(buf) - len, "ERROR: Client: Thing %s: ", 
                 thing_logname(t));
    }
    len = (uint32_t)strlen(buf);
    vsnprintf(buf + len, sizeof(buf) - len, fmt, args);

    putf(MY_STDOUT, buf);
    fflush(MY_STDOUT);

    putf(MY_STDERR, buf);
    fflush(MY_STDERR);

    backtrace_print();
    fflush(MY_STDOUT);

    wid_console_log(buf);
    term_log(buf);
}

void THING_ERR (thingp t, const char *fmt, ...)
{
    va_list args;

    verify(t);

    va_start(args, fmt);
    thing_err_(t, fmt, args);
    va_end(args);
}

void THING_DBG (thingp t, const char *fmt, ...)
{
    va_list args;

    verify(t);

    if (!debug_enabled) {
        return;
    }

    va_start(args, fmt);
    thing_log_(t, fmt, args);
    va_end(args);
}

static void level_log_ (levelp l, const char *fmt, va_list args)
{
    char buf[MAXSTR];
    uint32_t len;

    buf[0] = '\0';
    timestamp(buf, sizeof(buf));
    len = (uint32_t)strlen(buf);

    if (l->on_server) {
        snprintf(buf + len, sizeof(buf) - len, "Server: Level %s: ", 
                 level_get_logname(l));
    } else {
        snprintf(buf + len, sizeof(buf) - len, "Client: Level %s: ", 
                 level_get_logname(l));
    }
    len = (uint32_t)strlen(buf);
    vsnprintf(buf + len, sizeof(buf) - len, fmt, args);

    putf(MY_STDOUT, buf);
    fflush(MY_STDOUT);
}

void LEVEL_LOG (levelp t, const char *fmt, ...)
{
    va_list args;

    verify(t);

    va_start(args, fmt);
    level_log_(t, fmt, args);
    va_end(args);
}

/*
 * User has entered a command, run it
 */
uint8_t debug_enable (tokens_t *tokens, void *context)
{
    char *s = tokens->args[2];

    if (!s || (*s == '\0')) {
        debug_enabled = 1;
    } else {
        debug_enabled = strtol(s, 0, 10) ? 1 : 0;
    }

    if (!debug_enabled) {
        debug_enabled = 0;
        debug_socket_ping_enabled = 0;
    }

    CON("Debug mode set to %u", debug_enabled);

    return (true);
}

static void action_timer_log_ (timerp t, const char *fmt, va_list args)
{
    char buf[MAXSTR];
    uint32_t len;

    buf[0] = '\0';
    timestamp(buf, sizeof(buf));
    len = (uint32_t)strlen(buf);
    snprintf(buf + len, sizeof(buf) - len, "Timer %s: ", action_timer_logname(t));
    len = (uint32_t)strlen(buf);
    vsnprintf(buf + len, sizeof(buf) - len, fmt, args);

    putf(MY_STDOUT, buf);
    fflush(MY_STDOUT);
}

void TIMER_LOG (timerp t, const char *fmt, ...)
{
    va_list args;

    if (!debug_enabled) {
        return;
    }

    verify(t);

    va_start(args, fmt);
    action_timer_log_(t, fmt, args);
    va_end(args);
}

static void action_init_fn_log_ (init_fnp t, const char *fmt, va_list args)
{
    char buf[MAXSTR];
    uint32_t len;

    buf[0] = '\0';
    timestamp(buf, sizeof(buf));
    len = (uint32_t)strlen(buf);
    snprintf(buf + len, sizeof(buf) - len, "InitFn %s: ",
             action_init_fn_logname(t));
    len = (uint32_t)strlen(buf);
    vsnprintf(buf + len, sizeof(buf) - len, fmt, args);

    putf(MY_STDOUT, buf);
    fflush(MY_STDOUT);
}

void INIT_FN_LOG (init_fnp t, const char *fmt, ...)
{
    va_list args;

    if (!debug_enabled) {
        return;
    }

    verify(t);

    va_start(args, fmt);
    action_init_fn_log_(t, fmt, args);
    va_end(args);
}

#ifdef ENABLE_WID_DEBUG
static void wid_log_ (widp t, const char *fmt, va_list args)
{
    char buf[MAXSTR];
    uint32_t len;

    buf[0] = '\0';
    timestamp(buf, sizeof(buf));
    len = (uint32_t)strlen(buf);
    snprintf(buf + len, sizeof(buf) - len, "Wid   %s: ", wid_logname(t));
    len = (uint32_t)strlen(buf);
    vsnprintf(buf + len, sizeof(buf) - len, fmt, args);

    putf(MY_STDOUT, buf);
    fflush(MY_STDOUT);
}

void WID_LOG (widp t, const char *fmt, ...)
{
    va_list args;

    verify(t);

    va_start(args, fmt);
    wid_log_(t, fmt, args);
    va_end(args);
}

void WID_DBG (widp t, const char *fmt, ...)
{
    va_list args;

    verify(t);

    if (!debug_enabled) {
        return;
    }

    va_start(args, fmt);
    wid_log_(t, fmt, args);
    va_end(args);
}
#endif

void MSG (uint32_t level, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    msg_(level, 0, fmt, args);
    va_end(args);
}

static void msgerr_ (const char *fmt, va_list args)
{
    char buf[MAXSTR];
    uint32_t ts_len;
    uint32_t len;

    buf[0] = '\0';
    timestamp(buf, sizeof(buf));
    len = (uint32_t)strlen(buf);

    snprintf(buf + len, sizeof(buf) - len, "ERROR: %%%%fg=red$");

    len = (uint32_t)strlen(buf);
    ts_len = len;

    vsnprintf(buf + len, sizeof(buf) - len, fmt, args);

    len = (uint32_t)strlen(buf);
    snprintf(buf + len, sizeof(buf) - len, "%%%%fg=reset$");

    putf(MY_STDERR, buf);
    fflush(MY_STDERR);

    putf(MY_STDOUT, buf);
    fflush(MY_STDOUT);

    wid_console_log(buf);
    term_log(buf);

    wid_popup_error(buf + ts_len);

    backtrace_print();
    fflush(MY_STDOUT);
}

void MSG_BOX (const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    msgerr_(fmt, args);
    va_end(args);
}

static void sdl_msgerr_ (const char *fmt, va_list args)
{
    char buf[MAXSTR];
#if SDL_MAJOR_VERSION >= 2
    uint32_t ts_len;
#endif
    uint32_t len;

    buf[0] = '\0';
    timestamp(buf, sizeof(buf));
    len = (uint32_t)strlen(buf);

    snprintf(buf + len, sizeof(buf) - len, "ERROR: %%%%fg=red$");

    len = (uint32_t)strlen(buf);
#if SDL_MAJOR_VERSION >= 2
    ts_len = len;
#endif

    vsnprintf(buf + len, sizeof(buf) - len, fmt, args);

#if SDL_MAJOR_VERSION >= 2
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, 
	"Gorynlich", buf + ts_len, 0);
#endif

    len = (uint32_t)strlen(buf);
    snprintf(buf + len, sizeof(buf) - len, "%%%%fg=reset$");

    putf(MY_STDERR, buf);
    fflush(MY_STDERR);

    putf(MY_STDOUT, buf);
    fflush(MY_STDOUT);

    backtrace_print();
}

void SDL_MSG_BOX (const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    sdl_msgerr_(fmt, args);
    va_end(args);
}
