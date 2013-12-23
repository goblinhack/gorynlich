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

static char buf[200];
boolean debug_enabled;
boolean croaked;

static void log_ (const char *fmt, va_list args)
{
    uint32_t len;

    buf[0] = '\0';
    timestamp(buf, sizeof(buf));
    len = (uint32_t)strlen(buf);
    vsnprintf(buf + len, sizeof(buf) - len, fmt, args);

    puts(buf);
    fflush(stdout);
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

    puts(buf);
    fflush(stdout);
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

    snprintf(buf + len, sizeof(buf) - len, "Init  ");

    len = (uint32_t)strlen(buf);
    vsnprintf(buf + len, sizeof(buf) - len, fmt, args);

    puts(buf);
    fflush(stdout);

    wid_console_log(buf + len);
}

void INIT_LOG (const char *fmt, ...)
{
    va_list args;

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

    puts(buf);
    fflush(stdout);
}

void FINI_LOG (const char *fmt, ...)
{
    va_list args;

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

    snprintf(buf + len, sizeof(buf) - len, "Con   ");

    len = (uint32_t)strlen(buf);
    vsnprintf(buf + len, sizeof(buf) - len, fmt, args);

    puts(buf);
    fflush(stdout);

    wid_console_log(buf + len);
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

    puts(buf);
    fflush(stdout);
}

static void err_ (const char *fmt, va_list args)
{
    uint32_t len;

    buf[0] = '\0';
    timestamp(buf, sizeof(buf));
    len = (uint32_t)strlen(buf);

    snprintf(buf + len, sizeof(buf) - len, "ERROR: ");

    len = (uint32_t)strlen(buf);
    vsnprintf(buf + len, sizeof(buf) - len, fmt, args);

    fprintf(stderr, "%s\n", buf);
    fflush(stderr);

    backtrace_print();
    fflush(stdout);

    wid_console_log(buf);
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

    puts(buf);
    fflush(stdout);

    backtrace_print();
    fflush(stdout);

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

    puts(buf);
    fflush(stdout);
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

    puts(buf);
    fflush(stdout);
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

    puts(buf);
    fflush(stdout);
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

    puts(buf);
    fflush(stdout);
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

    puts(buf);
    fflush(stdout);
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

    puts(buf);
    fflush(stdout);
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

    puts(buf);
    fflush(stdout);
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

#ifndef ENABLE_WID_DEBUG
    if (!debug_enabled) {
        return;
    }
#endif

    va_start(args, fmt);
    wid_log_(t, fmt, args);
    va_end(args);
}
