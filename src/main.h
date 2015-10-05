/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file.
 */

#pragma once

#pragma once

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "config.h"

/*
 * Enhanced random number generator.
 */
#include "pcg_basic.h"
#define myrand() pcg32_random()
#define mysrand(a) pcg32_srandom(a,a)
//#define myrand() rand()
//#define mysrand(a) srand(a)

#ifdef _WIN32
#include <windows.h>

static inline void uSleep (int waitTime) 
{
    __int64 time1 = 0, time2 = 0, freq = 0;

    QueryPerformanceCounter((LARGE_INTEGER *) &time1);
    QueryPerformanceFrequency((LARGE_INTEGER *)&freq);

    do {
        QueryPerformanceCounter((LARGE_INTEGER *) &time2);
    } while((time2-time1) < waitTime);
}
#else
#define uSleep usleep
#endif

/*
 * Misc
 */
#undef ONESEC
#define ONESEC                          1000

#ifndef true
#define true                            1
#endif

#ifndef false
#define false                           0
#endif

#ifndef max
#define max(a,b)                        (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)                        (((a) < (b)) ? (a) : (b))
#endif

#define swap(a,b) {                                 \
            typeof (a) c;                           \
            c = a;                                  \
            a = b;                                  \
            b = c;                                  \
        }

/*
 * Simple array routines
 */
#define ARRAY_SIZE(_array_)             (sizeof(_array_)/sizeof(_array_[0]))

#define FOR_ALL_IN_ARRAY(iterator, _array_)                                 \
    typeof(&_array_[0]) iterator;                                           \
    for ((iterator) = (_array_);                                            \
         (iterator) < ((_array_) + ARRAY_SIZE(_array_));                    \
         (iterator)++)

#define FOR_NEXT_IN_ARRAY(iterator, _array_)                                \
    for ((iterator)++;                                                      \
         (iterator) < ((_array_) + ARRAY_SIZE(_array_));                    \
         (iterator)++)

/*
 * GCC extension for offset
 */
#ifdef __GNUC__
#if defined(__GNUC__) && __GNUC__ > 3
#define STRUCT_OFFSET(STRUCT, MEMBER) __builtin_offsetof(STRUCT, MEMBER)
#else
#define STRUCT_OFFSET(STRUCT, MEMBER) \
                     ((size_t) ( (char *)&((st *)0)->m - (char *)0 ))
#endif
#else
#define STRUCT_OFFSET(STRUCT, MEMBER) \
                     ((size_t) ( (char *)&((st *)0)->m - (char *)0 ))
#endif

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

/*
 * Types
 */
#ifndef __int8_t_defined
#ifndef _INT8_T
#define _INT8_T
typedef signed char          int8_t;
#endif /*_INT8_T */

#ifndef _INT16_T
#define _INT16_T
typedef short                int16_t;
#endif /* _INT16_T */

#ifndef _INT32_T
#define _INT32_T
typedef int                  int32_t;
#endif /* _INT32_T */

#ifndef _INT64_T
#define _INT64_T
typedef long long            int64_t;
#endif /* _INT64_T */

#ifndef _UINT8_T
#define _UINT8_T
typedef unsigned char        uint8_t;
#endif /*_UINT8_T */

#ifndef _UINT16_T
#define _UINT16_T
typedef unsigned short       uint16_t;
#endif /* _UINT16_T */

#ifndef _UINT32_T
#define _UINT32_T
typedef unsigned int         uint32_t;
#endif /* _UINT32_T */

#ifndef _UINT64_T
#define _UINT64_T
typedef unsigned long long   uint64_t;
#endif /* _UINT64_T */
#endif

/*
 * msys functions seem to accept either / or \ so we don't need to worry.
 */
#define DSEP "/"
#define DCHAR '/'

#if defined(__MINGW32__) || defined(__linux__)
/*
 * SDL provides SDL_strlcat so linking will work. This just fixes the
 * warnings.
 */
uint32_t strlcpy(char *dst, const char *src, uint32_t maxlen);
uint32_t strlcat(char *dst, const char *src, uint32_t maxlen);
#endif

/*
 * Globals:
 */
typedef struct aplayer_ *aplayerp;
typedef struct socket_ *gsocketp;
typedef struct net_ *netp;
typedef struct font *fontp;
typedef struct wid_console *wid_consolep;
typedef struct wid_chat *wid_chatp;
typedef struct config *configp;
typedef struct wid_ *widp;
typedef struct tree_root_ *tree_rootp;
typedef struct tex_ *texp;
typedef struct music_ *musicp;
typedef struct sound_ *soundp;
typedef struct tile_ *tilep;
typedef struct thing_ *thingp;
typedef struct thing_stats_ *thing_statsp;
typedef struct item_t_ *itemp;
typedef struct host_ *hostp;
typedef struct action_timer_ *timerp;
typedef struct action_init_fn_ *init_fnp;
typedef struct tp_ *tpp;
typedef struct tpp_data_ *tpp_data;
typedef struct thing_tile_ *thing_tilep;
typedef struct tokens_t_ *tokenp;
typedef struct tree_demarshal_ *demarshal_p;
typedef struct tree_marshal_ *marshal_p;
typedef struct tokens_t_ *tokensp;
typedef struct level_t_ *levelp;
typedef struct map_t_ *mapp;
typedef struct msg_player_state_ *msg_player_statep;

extern thingp player;
extern fontp vsmall_font;
extern fontp fixed_font;
extern fontp small_font;
extern fontp med_font;
extern fontp large_font;
extern fontp vlarge_font;
extern fontp vvlarge_font;
extern wid_consolep wid_console;
extern wid_chatp wid_chat;
extern configp config;

typedef uint8_t (*tp_is_callback)(tpp);

/*
 * log.c
 */
#define DIE(args...)                                                          \
    DYING("Died at %s:%s():%u", __FILE__, __FUNCTION__, __LINE__);            \
    CROAK(args);                                                              \
    exit(1);

extern uint8_t croaked;

#ifdef ENABLE_ASSERT
#define ASSERT(x)                                                             \
    if (!(x)) {                                                               \
        DIE("Failed assert");                                                 \
    }
#else
#define ASSERT(x)
#endif

void CROAK(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
void DYING(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
void LOG(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
void LOGS(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
void WARN(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
void INIT_LOG(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
void FINI_LOG(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
void CON(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));

enum {
    CHAT,
    INFO,
    WARNING,
    CRITICAL,
    POPUP,
    OVER_THING,
    SOUND,
    SOUND_GLOBAL,
};

/*
 * Bloody windows uses MSG hence MESG
 */
void MESG(uint32_t level, const char *fmt, ...) 
    __attribute__ ((format (printf, 2, 3)));

void SDL_MSG_BOX(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
void MSG_BOX(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
void ERR(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
void DBG(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
void HOST_LOG(hostp, const char *fmt, ...)
                     __attribute__ ((format (printf, 2, 3)));

void MSG_SERVER_SHOUT_AT(uint32_t level,
                         thingp,
                         double x,
                         double y,
                         const char *fmt, ...)
                         __attribute__ ((format (printf, 5, 6)));

void MSG_CLIENT_SHOUT_AT(uint32_t level,
                         uint32_t thing_id,
                         double x,
                         double y,
                         const char *fmt, ...)
                         __attribute__ ((format (printf, 5, 6)));

void MSG_SERVER_SHOUT_AT_ALL_PLAYERS(uint32_t level,
                                     double x,
                                     double y,
                                     const char *fmt, ...)
                                     __attribute__ ((format (printf, 4, 5)));

void THING_LOG(thingp, const char *fmt, ...)
                     __attribute__ ((format (printf, 2, 3)));
void THING_CON(thingp, const char *fmt, ...)
                     __attribute__ ((format (printf, 2, 3)));
void THING_ERR(thingp, const char *fmt, ...)
                     __attribute__ ((format (printf, 2, 3)));
void THING_DBG(thingp, const char *fmt, ...)
                     __attribute__ ((format (printf, 2, 3)));
void ITEM_LOG(itemp, const char *fmt, ...)
                     __attribute__ ((format (printf, 2, 3)));
void TIMER_LOG(timerp, const char *fmt, ...)
                     __attribute__ ((format (printf, 2, 3)));
void INIT_FN_LOG(init_fnp, const char *fmt, ...)
                     __attribute__ ((format (printf, 2, 3)));
void LEVEL_LOG(levelp, const char *fmt, ...)
                     __attribute__ ((format (printf, 2, 3)));

#ifdef ENABLE_WID_DEBUG
void WID_LOG(widp, const char *fmt, ...) \
                     __attribute__ ((format (printf, 2, 3)));

void WID_DBG(widp, const char *fmt, ...) \
                     __attribute__ ((format (printf, 2, 3)));
#else
#define WID_DBG(...)
#define WID_LOG(...)
#endif

extern uint8_t debug_enabled;
extern uint8_t debug_socket_ping_enabled;
extern uint8_t debug_socket_connect_enabled;
extern uint8_t debug_socket_players_enabled;

uint8_t debug_enable(tokensp, void *context);
uint8_t debug_socket_ping_enable(tokensp, void *context);
uint8_t debug_socket_connect_enable(tokensp, void *context);
uint8_t debug_socket_players_enable(tokensp, void *context);

uint8_t fps_enable(tokensp, void *context);

/*
 * util.c
 */
void *myzalloc_(uint32_t size, const char *what, const char *func,
                const char *file, const uint32_t line);

void *mymalloc_(uint32_t size, const char *what, const char *func,
                const char *file, const uint32_t line);

void *myrealloc_(void *ptr, uint32_t size, const char *what, const char *func,
                 const char *file, const uint32_t line);


void myfree_(void *ptr, const char *func, const char *file,
             const uint32_t line);

char *dupstr_(const char *in, const char *what, const char *func,
              const char *file, const uint32_t line);

#include "ptrcheck.h"

/*
 * point.c
 */
typedef struct {
    double x;
    double y;
} fpoint;

typedef struct {
    int32_t x;
    int32_t y;
} point;

typedef struct {
    double x;
    double y;
    double z;
} fpoint3d;

typedef struct {
    int32_t x;
    int32_t y;
    int32_t z;
} point3d;

typedef struct {
    int32_t width;
    int32_t height;
} size;

typedef struct {
    double width;
    double height;
} fsize;

/*
 * main.c
 */
extern FILE *LOG_STDOUT;
extern FILE *LOG_STDERR;

#define MY_STDOUT (LOG_STDOUT ? LOG_STDOUT : stdout)
#define MY_STDERR (LOG_STDERR ? LOG_STDERR : stderr)

extern char *EXEC_FULL_PATH_AND_NAME;
extern char *EXEC_DIR;
extern char *DATA_PATH;
extern char *LEVELS_PATH;
extern char *GFX_PATH;
extern char *TTF_PATH;
extern uint8_t quitting;

void quit(void);
void restart(void);
void die(void);
void tree_debug_test(void);
uint8_t miniz_test(int32_t argc, char *argv[]);
uint8_t mzip_file_test(int32_t argc, char *argv[]);
uint8_t dir_test(int32_t argc, char *argv[]);
uint8_t string_test(int32_t argc, char *argv[]);
uint8_t marshal_test(int32_t argc, char *argv[]);
uint8_t enum_test(int32_t argc, char *argv[]);
uint8_t ptrcheck_test(int32_t argc, char *argv[]);

/*
 * config.c
 */
void config_fini(void);
uint8_t config_init(void);
uint8_t config_save(void);
uint8_t config_load(void);

#include "thing_stats.h"

struct config {
    int32_t video_pix_width;
    int32_t video_pix_height;
    int32_t video_gl_width;
    int32_t video_gl_height;
    float xscale;
    float yscale;
    int32_t sound_volume;
    int32_t music_volume;
    int32_t display_sync;
    int32_t full_screen;
    int32_t intro_screen;
    int32_t fps_counter;
    int32_t deathmatch;
    int32_t deathmatch_monsters;
    char server_name[SMALL_STRING_LEN_MAX];
    char user_server_name[SMALL_STRING_LEN_MAX];
    uint16_t server_port;
    uint16_t user_server_port;
    uint8_t server_max_players;
    uint8_t server_current_players;

    /*
     * These are the stats we start out with.
     */
    thing_stats stats;

    /*
     * Where we hold onto the stats once the player is dead and has no thing 
     * so we can keep on showing their score.
     */
    thing_stats dead_stats;

    uint8_t starting_stats_inited;

    uint8_t game_over;

    /*
     * Current level. We need to keep seperate copies as they will go out of 
     * sync as a player moves from one level to another and we do not want msg
     * updates from the server that are old overwriting the global variable 
     * the server had just set (for combined server/client game).
     */
    level_pos_t server_level_pos;
    level_pos_t client_level_pos;
};

extern struct config global_config;

#define SOUND_MAX 20

/*
 * color.c
 */
typedef struct {
    uint8_t r, g, b, a;
} color;

/*
 * mazegen.c
 */
int32_t map_jigsaw_test(int32_t argc, char **argv);

/*
 * net.h
 */
extern uint8_t on_server;
extern uint8_t is_client;
extern uint8_t is_headless;
extern uint8_t single_player_mode;

/*
 * main.c
 */
extern uint8_t opt_quickstart;

/*
 * Server with no display.
 */
#define HEADLESS is_headless

extern int debug;

