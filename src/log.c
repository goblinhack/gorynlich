/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include "slre.h"
#include <SDL.h>

#include "main.h"
#include "backtrace.h"
#include "wid_console.h"
#include "time.h"
#include "command.h"
#include "thing.h"
#include "level.h"
#include "item.h"
#include "timer.h"
#include "init_fn.h"
#include "wid.h"
#include "net.h"
#include "term.h"

static char buf[200];
boolean debug_enabled;
boolean croaked;

/*
 * putfgbg
 */
static inline void putfgbg (uint8_t fg, uint8_t bg)
{
    static const char *data[] = {
            "\033[40;30m", "\033[40;31m", "\033[40;32m", "\033[40;33m",
            "\033[40;34m", "\033[40;35m", "\033[40;36m", "\033[40;37m",
            "\033[41;30m", "\033[41;31m", "\033[41;32m", "\033[41;33m",
            "\033[41;34m", "\033[41;35m", "\033[41;36m", "\033[41;37m",
            "\033[42;30m", "\033[42;31m", "\033[42;32m", "\033[42;33m",
            "\033[42;34m", "\033[42;35m", "\033[42;36m", "\033[42;37m",
            "\033[43;30m", "\033[43;31m", "\033[43;32m", "\033[43;33m",
            "\033[43;34m", "\033[43;35m", "\033[43;36m", "\033[43;37m",
            "\033[44;30m", "\033[44;31m", "\033[44;32m", "\033[44;33m",
            "\033[44;34m", "\033[44;35m", "\033[44;36m", "\033[44;37m",
            "\033[45;30m", "\033[45;31m", "\033[45;32m", "\033[45;33m",
            "\033[45;34m", "\033[45;35m", "\033[45;36m", "\033[45;37m",
            "\033[46;30m", "\033[46;31m", "\033[46;32m", "\033[46;33m",
            "\033[46;34m", "\033[46;35m", "\033[46;36m", "\033[46;37m",
            "\033[47;30m", "\033[47;31m", "\033[47;32m", "\033[47;33m",
            "\033[47;34m", "\033[47;35m", "\033[47;36m", "\033[47;37m",
    };

    printf("%s", data[(bg & 7) * 8 + (fg & 7)]);
}

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
    boolean looking_for_start = false;

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

static void log_ (const char *fmt, va_list args)
{
    uint32_t len;

    buf[0] = '\0';
    timestamp(buf, sizeof(buf));
    len = (uint32_t)strlen(buf);
    vsnprintf(buf + len, sizeof(buf) - len, fmt, args);

    putf(MY_STDOUT, buf);
    fflush(MY_STDOUT);

    term_log(buf);
}

void LOG (const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    log_(fmt, args);
    va_end(args);
}

static void warn_ (const char *fmt, va_list args)
{
    uint32_t len;

    buf[0] = '\0';
    timestamp(buf, sizeof(buf));
    len = (uint32_t)strlen(buf);
    vsnprintf(buf + len, sizeof(buf) - len, fmt, args);

    putf(MY_STDOUT, buf);
    fflush(MY_STDOUT);

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
    uint32_t len;

    buf[0] = '\0';
    timestamp(buf, sizeof(buf));
    len = (uint32_t)strlen(buf);

    snprintf(buf + len, sizeof(buf) - len, "Init ");

    len = (uint32_t)strlen(buf);
    vsnprintf(buf + len, sizeof(buf) - len, fmt, args);

    putf(MY_STDOUT, buf);
    fflush(MY_STDOUT);

    wid_console_log(buf + len);
    term_log(buf + len);
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
    uint32_t len;

    buf[0] = '\0';
    timestamp(buf, sizeof(buf));
    len = (uint32_t)strlen(buf);

    len = (uint32_t)strlen(buf);
    vsnprintf(buf + len, sizeof(buf) - len, fmt, args);

    if (HEADLESS) {
        putf(MY_STDOUT, buf);
        fflush(MY_STDOUT);
    }

    wid_console_log(buf + len);
    term_log(buf + len);
}

static void raw_ (const char *fmt, va_list args)
{
    uint32_t len;

    buf[0] = '\0';
    len = (uint32_t)strlen(buf);

    len = (uint32_t)strlen(buf);
    vsnprintf(buf + len, sizeof(buf) - len, fmt, args);

    if (HEADLESS) {
        putf(MY_STDOUT, buf);
        fflush(MY_STDOUT);
    }

    wid_console_log(buf + len);
    term_log(buf + len);
}

static void dying_ (const char *fmt, va_list args)
{
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
    uint32_t len;

    buf[0] = '\0';
    timestamp(buf, sizeof(buf));
    len = (uint32_t)strlen(buf);

    snprintf(buf + len, sizeof(buf) - len, "FATAL ERROR: ");

    len = (uint32_t)strlen(buf);
    vsnprintf(buf + len, sizeof(buf) - len, fmt, args);

    putf(MY_STDERR, buf);
    fflush(MY_STDERR);

    putf(MY_STDOUT, buf);
    fflush(MY_STDOUT);

    backtrace_print();
    fflush(MY_STDOUT);

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

void RAW (const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    raw_(fmt, args);
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

static void thing_log_ (thingp t, const char *fmt, va_list args)
{
    static char buf[200];
    uint32_t len;

    buf[0] = '\0';
    timestamp(buf, sizeof(buf));
    len = (uint32_t)strlen(buf);
    snprintf(buf + len, sizeof(buf) - len, "Thing %s: ", thing_logname(t));
    len = (uint32_t)strlen(buf);
    vsnprintf(buf + len, sizeof(buf) - len, fmt, args);

    putf(MY_STDOUT, buf);
    fflush(MY_STDOUT);
}

void THING_LOG (thingp t, const char *fmt, ...)
{
    va_list args;

    if (!debug_enabled) {
        return;
    }

    verify(t);

    va_start(args, fmt);
    thing_log_(t, fmt, args);
    va_end(args);
}

static void host_log_ (hostp t, const char *fmt, va_list args)
{
    static char buf[200];
    uint32_t len;

    buf[0] = '\0';
    timestamp(buf, sizeof(buf));
    len = (uint32_t)strlen(buf);
    snprintf(buf + len, sizeof(buf) - len, "%s: ", host_logname(t));
    len = (uint32_t)strlen(buf);
    vsnprintf(buf + len, sizeof(buf) - len, fmt, args);

    putf(MY_STDOUT, buf);
    fflush(MY_STDOUT);
}

void HOST_LOG (hostp t, const char *fmt, ...)
{
    va_list args;

    if (!debug_enabled) {
        return;
    }

    verify(t);

    va_start(args, fmt);
    host_log_(t, fmt, args);
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
    static char buf[200];
    uint32_t len;

    buf[0] = '\0';
    timestamp(buf, sizeof(buf));
    len = (uint32_t)strlen(buf);
    snprintf(buf + len, sizeof(buf) - len, "Level %s: ", level_get_logname(l));
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
boolean debug_enable (tokens_t *tokens, void *context)
{
    debug_enabled = true;

    return (true);
}

/*
 * User has entered a command, run it
 */
boolean debug_disable (tokens_t *tokens, void *context)
{
    debug_enabled = false;

    return (true);
}

static void item_log_ (itemp t, const char *fmt, va_list args)
{
    static char buf[200];
    uint32_t len;

    buf[0] = '\0';
    timestamp(buf, sizeof(buf));
    len = (uint32_t)strlen(buf);
    snprintf(buf + len, sizeof(buf) - len, "Items %s: ", item_logname(t));
    len = (uint32_t)strlen(buf);
    vsnprintf(buf + len, sizeof(buf) - len, fmt, args);

    putf(MY_STDOUT, buf);
    fflush(MY_STDOUT);
}

void ITEM_LOG (itemp t, const char *fmt, ...)
{
    va_list args;

    if (!debug_enabled) {
        return;
    }

    verify(t);

    va_start(args, fmt);
    item_log_(t, fmt, args);
    va_end(args);
}

static void action_timer_log_ (timerp t, const char *fmt, va_list args)
{
    static char buf[200];
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
    static char buf[200];
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
    static char buf[200];
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
