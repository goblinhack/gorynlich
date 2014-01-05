/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file.
 */

#include <stdio.h>
#include <stdint.h>
#include "config.h"

/*
 * Misc
 */
#undef ONESEC
#define ONESEC                          1000

#define MAXSTR                          256

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
#define boolean              char

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
typedef struct socket_ *socketp;
typedef struct net_ *netp;
typedef struct font *fontp;
typedef struct wid_console *wid_consolep;
typedef struct config *configp;
typedef struct wid_ *widp;
typedef struct tree_root_ *tree_rootp;
typedef struct tex_ *texp;
typedef struct music_ *musicp;
typedef struct sound_ *soundp;
typedef struct tile_ *tilep;
typedef struct thing_ *thingp;
typedef struct host_ *hostp;
typedef struct action_timer_ *timerp;
typedef struct action_init_fn_ *init_fnp;
typedef struct item_ *itemp;
typedef struct thing_template_ *thing_templatep;
typedef struct thing_tile_ *thing_tilep;
typedef struct tokens_t_ *tokenp;
typedef struct tree_demarshal_ *demarshal_p;
typedef struct tree_marshal_ *marshal_p;
typedef struct tokens_t_ *tokensp;
typedef struct level_t_ *levelp;
typedef struct map_t_ *mapp;

extern thingp player;
extern fontp vsmall_font;
extern fontp small_font;
extern fontp med_font;
extern fontp large_font;
extern fontp vlarge_font;
extern wid_consolep wid_console;
extern configp config;

typedef boolean (*thing_template_is_callback)(thing_templatep);

/*
 * log.c
 */
#define DIE(args...)                                                          \
    DYING("Died at %s:%s():%u", __FILE__, __FUNCTION__, __LINE__);            \
    CROAK(args);                                                              \
    exit(1);

extern boolean croaked;

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
void WARN(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
void INIT_LOG(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
void FINI_LOG(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
void CON(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
void ERR(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
void DBG(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
void HOST_LOG(hostp, const char *fmt, ...) \
                     __attribute__ ((format (printf, 2, 3)));
void THING_LOG(thingp, const char *fmt, ...) \
                     __attribute__ ((format (printf, 2, 3)));
void THING_DBG(thingp, const char *fmt, ...) \
                     __attribute__ ((format (printf, 2, 3)));
void ITEM_LOG(itemp, const char *fmt, ...) \
                     __attribute__ ((format (printf, 2, 3)));
void TIMER_LOG(timerp, const char *fmt, ...) \
                     __attribute__ ((format (printf, 2, 3)));
void INIT_FN_LOG(init_fnp, const char *fmt, ...) \
                     __attribute__ ((format (printf, 2, 3)));
void LEVEL_LOG(levelp, const char *fmt, ...) \
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

#define ERR_POPUP(...)                                                      \
{                                                                           \
    char *tmp = dynprintf(__VA_ARGS__);                                     \
    (void) wid_popup_error(tmp);                                            \
    myfree(tmp);                                                            \
}                                                                           \

extern boolean debug_enabled;
extern boolean debug_socket_ping_enabled;
extern boolean debug_socket_connect_enabled;
extern boolean debug_socket_players_enabled;

boolean debug_enable(tokensp, void *context);
boolean debug_socket_ping_enable(tokensp, void *context);
boolean debug_socket_connect_enable(tokensp, void *context);
boolean debug_socket_players_enable(tokensp, void *context);

extern boolean fps_enabled;
boolean fps_enable(tokensp, void *context);

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
extern boolean quitting;

void quit(void);
void restart(void);
void die(void);
void tree_debug_test(void);
boolean miniz_test(int32_t argc, char *argv[]);
boolean mzip_file_test(int32_t argc, char *argv[]);
boolean dir_test(int32_t argc, char *argv[]);
boolean string_test(int32_t argc, char *argv[]);
boolean marshal_test(int32_t argc, char *argv[]);
boolean enum_test(int32_t argc, char *argv[]);
boolean ptrcheck_test(int32_t argc, char *argv[]);

/*
 * config.c
 */
void config_fini(void);
boolean config_init(void);
boolean config_save(void);
boolean config_load(void);

struct config {
    int32_t video_pix_width;
    int32_t video_pix_height;
    int32_t video_gl_width;
    int32_t video_gl_height;
    float xscale;
    float yscale;
    int32_t sound_volume;
    int32_t music_volume;
};

extern struct config global_config;

#define SOUND_MIN 0
#define SOUND_MAX 3

/*
 * color.c
 */
typedef struct {
    uint8_t r, g, b, a;
} color;

/*
 * resource.c
 */
extern thing_templatep PLAYER_0;
extern uint16_t PLAYER_0_ID;

extern thing_templatep BRICK_0;
extern uint16_t BRICK_0_ID;

extern thing_templatep BRICK_1;
extern uint16_t BRICK_1_ID;

extern thing_templatep BRICK_2;
extern uint16_t BRICK_2_ID;

extern thing_templatep BRICK_3;
extern uint16_t BRICK_3_ID;

extern thing_templatep BRICK_4;
extern uint16_t BRICK_4_ID;

extern thing_templatep BRICK_5;
extern uint16_t BRICK_5_ID;

extern thing_templatep BRICK_6;
extern uint16_t BRICK_6_ID;

extern thing_templatep BRICK_7;
extern uint16_t BRICK_7_ID;

extern thing_templatep BRICK_8;
extern uint16_t BRICK_8_ID;

extern thing_templatep ROCK_0;
extern uint16_t ROCK_0_ID;

extern thing_templatep ROCK_1;
extern uint16_t ROCK_1_ID;

extern thing_templatep LAVA_0;
extern uint16_t LAVA_0_ID;

extern thing_templatep WATER_0;
extern uint16_t WATER_0_ID;

/*
 * mazegen.c
 */
int32_t map_jigsaw_test(int32_t argc, char **argv);

/*
 * net.h
 */
extern boolean is_server;
extern boolean is_client;
extern boolean is_headless;

/*
 * Server with no displa.
 */
#define HEADLESS ((is_server && !is_client) || is_headless)

