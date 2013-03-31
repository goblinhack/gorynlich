/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <SDL.h>
#include "glapi.h"

#include "main.h"
#include "gl.h"
#include "wid.h"
#include "wid_console.h"
#include "color.h"
#include "time.h"
#include "thing.h"
#include "sdl.h"
#include "init_fn.h"
#include "wid_splash.h"
#include "config.h"
#include "tex.h"
#include "ttf.h"
#include "slre.h"
#include "token.h"

#ifndef SDL_BUTTON_WHEELLEFT
#define SDL_BUTTON_WHEELLEFT 6
#endif

#ifndef SDL_BUTTON_WHEELRIGHT
#define SDL_BUTTON_WHEELRIGHT 7
#endif

#if SDL_MAJOR_VERSION == 1 && SDL_MINOR_VERSION == 2 /* { */
#undef ENABLE_SDL_WINDOW
#else /* } { */
#define ENABLE_SDL_WINDOW
#endif /* } */

#ifdef __IPHONE_OS_VERSION_MIN_REQUIRED /* { */
#define ENABLE_SDL_WINDOW
#endif /* } */

#ifdef DISABLE_SDL_WINDOW
#undef ENABLE_SDL_WINDOW
#endif

#ifdef ENABLE_SDL_WINDOW /* { */
#ifndef sdl_uikitkeyboard_h
#define sdl_uikitkeyboard_h
/*
 * For some reason, the header file is not installed.
 */
extern DECLSPEC int32_t SDLCALL SDL_iPhoneKeyboardShow(SDL_Window * window);
extern DECLSPEC int32_t SDLCALL SDL_iPhoneKeyboardHide(SDL_Window * window);
extern DECLSPEC SDL_bool SDLCALL SDL_iPhoneKeyboardIsShown(SDL_Window * window);
extern DECLSPEC int32_t SDLCALL SDL_iPhoneKeyboardToggle(SDL_Window * window);
#endif
#endif /* } */

static boolean sdl_main_loop_running;
int32_t sdl_init_video;

#ifdef ENABLE_SDL_WINDOW /* { */
SDL_Window *window; /* Our window handle */
SDL_GLContext context; /* Our opengl context handle */
#endif /* } */

void sdl_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (sdl_init_video) {
        sdl_init_video = 0;
        SDL_VideoQuit();
    }

#ifdef ENABLE_SDL_WINDOW /* { */
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
#endif /* } */

    SDL_Quit();
}

static inline boolean sdl_find_video_size (int32_t w, int32_t h)
{
    static boolean first = true;
    int32_t i;

    first = true;

#if SDL_MAJOR_VERSION == 1 && SDL_MINOR_VERSION == 2 /* { */
    SDL_Rect **modes;

    /* Get available fullscreen/hardware modes */
    modes = SDL_ListModes(0, SDL_FULLSCREEN|SDL_HWSURFACE);

    /* Check if there are any modes available */
    if (modes == (SDL_Rect**)0) {
        DIE("No modes available!");
    }

    /* Check if our resolution is restricted */
    if (modes == (SDL_Rect**)-1) {
        return (true);
    }

    for (i=0; modes[i]; ++i) {
        if (first) {
            DBG("SDL video   : %dx%d available (full screen)",
                    modes[i]->w, modes[i]->h);
        }

        if ((modes[i]->w == w) && (modes[i]->h == h)) {
            INIT_LOG("SDL video   : %dx%d available (full screen)", w, h);
            return (true);
        }
    }
#else /* } { */
    for (i = 0; i < SDL_GetNumDisplayModes(0); ++i) {

        SDL_DisplayMode mode;

        SDL_GetDisplayMode(0, i, &mode);

        if (first) {
            DBG("SDL video   : %dx%d available (full screen)", mode.w, mode.h);
        }

        if ((mode.w == w) && (mode.h == h)) {
            INIT_LOG("SDL video   : %dx%d available (full screen)", w, h);
            return (true);
        }
    }
#endif /* } */

#if SDL_MAJOR_VERSION == 1 && SDL_MINOR_VERSION == 2 /* { */
    /* Get available fullscreen/hardware modes */
    modes = SDL_ListModes(0, 0);

    /* Check if there are any modes available */
    if (modes == (SDL_Rect**)0) {
        DIE("No modes available!");
    }

    /* Check if our resolution is restricted */
    if (modes == (SDL_Rect**)-1) {
        return (true);
    }

    for (i=0; modes[i]; ++i) {
        if (first) {
            DBG("SDL video   : %dx%d available (any mode)",
                    modes[i]->w, modes[i]->h);
        }

        if ((modes[i]->w == w) && (modes[i]->h == h)) {
            INIT_LOG("SDL video   : %dx%d available (any mode)", w, h);
            return (true);
        }
    }
#else /* } { */
    for (i = 0; i < SDL_GetNumDisplayModes(0); ++i) {

        SDL_DisplayMode mode;

        SDL_GetDisplayMode(0, i, &mode);

        if (first) {
            DBG("SDL video   : %dx%d available (any mode)", mode.w, mode.h);
        }

        if ((mode.w == w) && (mode.h == h)) {
            INIT_LOG("SDL video   : %dx%d available (any mode)", w, h);
            return (true);
        }
    }
#endif /* } */

    INIT_LOG("SDL video   : %dx%d not available", w, h);

    first = false;

    return (false);
}

boolean sdl_init (void)
{
    int32_t VIDEO_WIDTH;
    int32_t VIDEO_HEIGHT;
    int32_t value;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        ERR("Couldn't initialize SDL: %s", SDL_GetError());
        return (false);
    }

    sdl_init_video = 1;

    INIT_LOG("SDL version : %u.%u", SDL_MAJOR_VERSION, SDL_MINOR_VERSION);

    boolean iphone_size = sdl_find_video_size(
                            IPHONE_VIDEO_WIDTH,
                            IPHONE_VIDEO_HEIGHT);

    boolean medium_size = sdl_find_video_size(
                            MEDIUM_VIDEO_WIDTH,
                            MEDIUM_VIDEO_HEIGHT);

    boolean ipad_size = sdl_find_video_size(
                            IPAD_VIDEO_WIDTH,
                            IPAD_VIDEO_HEIGHT);

    boolean default_size = sdl_find_video_size(
                            DEFAULT_VIDEO_WIDTH,
                            DEFAULT_VIDEO_HEIGHT);
    /*
     * If we have a saved setting, use that.
     */
    if (global_config.video_pix_width && global_config.video_pix_height) {
        VIDEO_WIDTH = global_config.video_pix_width;
        VIDEO_HEIGHT = global_config.video_pix_height;
    /*
     * Else guess.
     */
    } else if (medium_size) {
        VIDEO_WIDTH = MEDIUM_VIDEO_WIDTH;
        VIDEO_HEIGHT = MEDIUM_VIDEO_HEIGHT;
    } else if (iphone_size) {
        VIDEO_WIDTH = IPHONE_VIDEO_WIDTH;
        VIDEO_HEIGHT = IPHONE_VIDEO_HEIGHT;
    } else if (ipad_size) {
        VIDEO_WIDTH = IPAD_VIDEO_WIDTH;
        VIDEO_HEIGHT = IPAD_VIDEO_HEIGHT;
    } else if (default_size) {
        VIDEO_WIDTH = DEFAULT_VIDEO_WIDTH;
        VIDEO_HEIGHT = DEFAULT_VIDEO_HEIGHT;
    } else {
        VIDEO_WIDTH = DEFAULT_VIDEO_WIDTH;
        VIDEO_HEIGHT = DEFAULT_VIDEO_HEIGHT;
    }

    /*
     * Ortho fixup.
     */
    gl_ortho_set(VIDEO_WIDTH, VIDEO_HEIGHT);

    global_config.video_pix_width = VIDEO_WIDTH;
    global_config.video_pix_height = VIDEO_HEIGHT;

    INIT_LOG("SDL video   : %dx%d",
             global_config.video_pix_width, global_config.video_pix_height);

    INIT_LOG("Ortho video : %dx%d",
             global_config.video_gl_width, global_config.video_gl_height);

    INIT_LOG("X/Y scale   : %f, %f",
             global_config.xscale, global_config.yscale);

    SDL_WM_SetCaption("gorynlich", "gorynlich");

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    /*
     * Don't use this. It seemed to mess up graphics on FireGL.
     *
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
     */

#if SDL_MAJOR_VERSION == 1 && SDL_MINOR_VERSION == 2 /* { */

#    ifdef ENABLE_VIDEO_SYNC /* { */
        SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
#    else
        SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 0);
#    endif /* } */

#else /* } { */

#    ifdef ENABLE_VIDEO_SYNC /* { */
        SDL_GL_SetSwapInterval(1);
#    else
        SDL_GL_SetSwapInterval(0);
#    endif /* } */

#endif /* } */

    uint32_t video_flags;

#ifndef ENABLE_SDL_WINDOW /* { */
        INIT_LOG("SDL mode    : video");

        video_flags = SDL_OPENGL;

#       ifdef __IPHONE_OS_VERSION_MIN_REQUIRED /* { */
            video_flags |= SDL_WINDOW_FULLSCREEN | SDL_WINDOW_BORDERLESS;
#       endif /* } */

        if (SDL_SetVideoMode(VIDEO_WIDTH, VIDEO_HEIGHT,
                            0 /* bpp */, video_flags) == 0) {

            global_config.video_pix_width = 0;
            global_config.video_pix_height = 0;

            config_save();

            DIE("Couldn't set display %ux%u: %s",
                VIDEO_WIDTH, VIDEO_HEIGHT,
                SDL_GetError());
        }

#   else /* } { */
        INIT_LOG("SDL mode    : window");

        video_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;

#       ifdef __IPHONE_OS_VERSION_MIN_REQUIRED /* { */
            video_flags |= SDL_WINDOW_FULLSCREEN | SDL_WINDOW_BORDERLESS;
#       endif /* } */

        window = SDL_CreateWindow("gorynlich",
                                SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED,
                                VIDEO_WIDTH, VIDEO_HEIGHT,
                                video_flags);
        if (!window) {
            global_config.video_pix_width = 0;
            global_config.video_pix_height = 0;

            config_save();

            DIE("Couldn't set windowed display %ux%u: %s",
                VIDEO_WIDTH, VIDEO_HEIGHT,
                SDL_GetError());
        }

        context = SDL_GL_CreateContext(window);

#endif /* } */

    INIT_LOG("GL Vendor   : %s", glGetString(GL_VENDOR));
    INIT_LOG("GL Renderer : %s", glGetString(GL_RENDERER));
    INIT_LOG("GL Version  : %s", glGetString(GL_VERSION));
    INIT_LOG("GL Exts     : %s", glGetString(GL_EXTENSIONS));

    SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &value);
    DBG("Red         : %d", value);

    SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &value);
    DBG("Green       : %d", value);

    SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &value);
    DBG("Blue        : %d", value);

    SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &value);
    DBG("Depth       : %d", value);

    SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &value);
    DBG("Doub Buffer : %d", value);

    SDL_GL_GetAttribute(SDL_GL_ACCELERATED_VISUAL, &value);
    DBG("Hw Accel    : %d", value);

#if SDL_MAJOR_VERSION == 1 && SDL_MINOR_VERSION > 2 /* { */
    DBG("Vsync       : %d", SDL_GL_GetSwapInterval());
#endif /* } */

    SDL_EnableKeyRepeat(150, 50);

    return (true);
}

#if SDL_MAJOR_VERSION == 1 && SDL_MINOR_VERSION > 2 /* { */
static int32_t sdl_filter_events (void *userdata, SDL_Event *event)
{
    switch (event->type) {
        /* This is important!  Queue it if we want to quit. */
        case SDL_QUIT:
            return (1);

        /* Mouse and keyboard events go to threads */
        case SDL_MOUSEMOTION:
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEWHEEL:
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            return (1);

        /* Drop all other events */
        default:
            return (0);
    }
}
#endif /* } */

static void sdl_event (SDL_Event * event)
{
    static int32_t mouse_x;
    static int32_t mouse_y;

    SDL_keysym *key;

    switch (event->type) {
#if SDL_MAJOR_VERSION == 1 && SDL_MINOR_VERSION > 2 /* { */
    case SDL_WINDOWEVENT:
        switch (event->window.event) {
        case SDL_WINDOWEVENT_SHOWN:
            DBG("Window %d shown", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_HIDDEN:
            DBG("Window %d hidden", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_EXPOSED:
            DBG("Window %d exposed", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_MOVED:
            DBG("Window %d moved to %d,%d",
                event->window.windowID, event->window.data1,
                event->window.data2);
            break;
        case SDL_WINDOWEVENT_RESIZED:
            DBG("Window %d resized to %dx%d",
                event->window.windowID, event->window.data1,
                event->window.data2);
            break;
        case SDL_WINDOWEVENT_MINIMIZED:
            DBG("Window %d minimized", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_MAXIMIZED:
            DBG("Window %d maximized", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_RESTORED:
            DBG("Window %d restored", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_ENTER:
            DBG("Mouse entered window %d", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_LEAVE:
            DBG("Mouse left window %d", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_FOCUS_GAINED:
            DBG("Window %d gained keyboard focus", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_FOCUS_LOST:
            DBG("Window %d lost keyboard focus", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_CLOSE:
            DBG("Window %d closed", event->window.windowID);
            break;
        default:
            DBG("Window %d got unknown event %d",
                event->window.windowID, event->window.event);
            break;
        }
        break;
#endif /* } */

    case SDL_KEYDOWN:
        DBG("Keyboard: key pressed keycode 0x%08X = %s",
            event->key.keysym.sym,
            SDL_GetKeyName(event->key.keysym.sym));

            key = &event->key.keysym;

            wid_key_down(key, mouse_x, mouse_y);
        break;

    case SDL_KEYUP:
        DBG("Keyboard: key released keycode 0x%08X = %s",
            event->key.keysym.sym,
            SDL_GetKeyName(event->key.keysym.sym));

            key = &event->key.keysym;

            wid_key_up(key, mouse_x, mouse_y);
        break;

#if SDL_MAJOR_VERSION == 1 && SDL_MINOR_VERSION > 2 /* { */
    case SDL_TEXTINPUT:
        DBG("Keyboard: text input \"%s\" in window %d",
            event->text.text, event->text.windowID);
        break;
#endif /* } */

#if SDL_MAJOR_VERSION == 1 && SDL_MINOR_VERSION > 2 /* { */
    case SDL_MOUSEWHEEL:
        DBG("Mouse: wheel scrolled %d in x and %d in y in window %d",
            event->wheel.x, event->wheel.y, event->wheel.windowID);

        SDL_GetMouseState(&mouse_x, &mouse_y);

        mouse_x *= global_config.xscale;
        mouse_y *= global_config.yscale;

        wid_mouse_motion(mouse_x, mouse_y,
                         0, 0,
                         event->wheel.x, event->wheel.y);
        break;
#endif /* } */

    case SDL_MOUSEMOTION:
        DBG("Mouse: moved to %d,%d (%d,%d)",
            event->motion.x, event->motion.y,
            event->motion.xrel, event->motion.yrel);

        SDL_GetMouseState(&mouse_x, &mouse_y);

        mouse_x *= global_config.xscale;
        mouse_y *= global_config.yscale;

        wid_mouse_motion(mouse_x, mouse_y,
                         event->motion.xrel, event->motion.yrel,
                         0, 0);
        break;

    case SDL_MOUSEBUTTONDOWN:
        DBG("Mouse DOWN: button %d pressed at %d,%d",
            event->button.button, event->button.x, event->button.y);

        SDL_GetMouseState(&mouse_x, &mouse_y);

#if SDL_MAJOR_VERSION == 1 && SDL_MINOR_VERSION == 2 /* { */
        if (event->button.button == SDL_BUTTON_WHEELUP) {
            DBG("  wheel up");
            wid_mouse_motion(mouse_x, mouse_y, 0, 0, 0, 10);
            break;
        } else if (event->button.button == SDL_BUTTON_WHEELDOWN) {
            DBG("  wheel down");
            wid_mouse_motion(mouse_x, mouse_y, 0, 0, 0, -10);
            break;
        } else if (event->button.button == SDL_BUTTON_WHEELLEFT) {
            DBG("  wheel left");
            wid_mouse_motion(mouse_x, mouse_y, 0, 0, -10, 0);
            break;
        } else if (event->button.button == SDL_BUTTON_WHEELRIGHT) {
            DBG("  wheel right");
            wid_mouse_motion(mouse_x, mouse_y, 0, 0, 10, 0);
            break;
        }
#endif /* } */

        mouse_x *= global_config.xscale;
        mouse_y *= global_config.yscale;

        wid_mouse_down(event->button.button, mouse_x, mouse_y);
        break;

    case SDL_MOUSEBUTTONUP:
        DBG("Mouse UP: button %d released at %d,%d",
            event->button.button, event->button.x, event->button.y);

        SDL_GetMouseState(&mouse_x, &mouse_y);

        mouse_x *= global_config.xscale;
        mouse_y *= global_config.yscale;

        wid_mouse_up(event->button.button, mouse_x, mouse_y);
        break;

    case SDL_JOYBALLMOTION:
        DBG("Joystick %d: ball %d moved by %d,%d",
            event->jball.which, event->jball.ball, event->jball.xrel,
            event->jball.yrel);
        break;

    case SDL_JOYHATMOTION:
        DBG("Joystick %d: hat %d moved to ", event->jhat.which,
            event->jhat.hat);

        switch (event->jhat.value) {
        case SDL_HAT_CENTERED:
            DBG("CENTER");
            break;
        case SDL_HAT_UP:
            DBG("UP");
            break;
        case SDL_HAT_RIGHTUP:
            DBG("RIGHTUP");
            break;
        case SDL_HAT_RIGHT:
            DBG("RIGHT");
            break;
        case SDL_HAT_RIGHTDOWN:
            DBG("RIGHTDOWN");
            break;
        case SDL_HAT_DOWN:
            DBG("DOWN");
            break;
        case SDL_HAT_LEFTDOWN:
            DBG("LEFTDOWN");
            break;
        case SDL_HAT_LEFT:
            DBG("LEFT");
            break;
        case SDL_HAT_LEFTUP:
            DBG("LEFTUP");
            break;
        default:
            DBG("UNKNOWN");
            break;
        }
        break;

    case SDL_JOYBUTTONDOWN:
        DBG("Joystick %d: button %d pressed",
            event->jbutton.which, event->jbutton.button);
        break;

    case SDL_JOYBUTTONUP:
        DBG("Joystick %d: button %d released",
            event->jbutton.which, event->jbutton.button);
        break;

#if SDL_MAJOR_VERSION == 1 && SDL_MINOR_VERSION > 2 /* { */
    case SDL_CLIPBOARDUPDATE:
        DBG("Clipboard updated");
        break;
#endif /* } */

    case SDL_QUIT:
        DIE("Quit requested");
        break;

    case SDL_USEREVENT:
        DBG("User event %d", event->user.code);
        break;

    default:
        DBG("Unknown event %d", event->type);
        break;
    }
}

boolean sdl_is_exiting (void)
{
    return (!sdl_main_loop_running);
}

void sdl_exit (void)
{
    if (!sdl_main_loop_running) {
        return;
    }

    FINI_LOG("Main loop is exiting...");

    sdl_main_loop_running = false;
}

/*
 * QUAD per array element.
 */
#define NUMBER_COORDS_PER_VERTEX 4

/*
 * x and y per element.
 */
#define NUMBER_DIMENSIONS_PER_COORD 2

static const uint32_t NUMBER_BYTES_PER_VERTICE =
                                            sizeof(GLfloat) *
                                            NUMBER_DIMENSIONS_PER_COORD;

static const uint32_t NUMBER_BYTES_PER_ARRAY_ELEM =
                                            sizeof(GLfloat) *
                                            NUMBER_COORDS_PER_VERTEX *
                                            NUMBER_DIMENSIONS_PER_COORD;
/*
 * Two arrays, xy and uv.
 */
static const uint32_t NUMBER_ARRAY_ELEM_ARRAYS = 2;

typedef struct {
    uint16_t tile;
} map_tile_t;

#define MAP_CHUNK_WIDTH 1024
#define MAP_CHUNK_HEIGHT 4096
#define MAP_CHUNKS_WIDTH 3

/*
 * All the rendering info for one parallax frame of tiles.
 */
typedef struct {
    /*
     * All the tiles in this frame.
     */
    map_tile_t tiles[MAP_CHUNK_WIDTH * MAP_CHUNKS_WIDTH][MAP_CHUNK_HEIGHT];

    /*
     * This is the huge buffer that contains all arrays.
     */
    float *gl_array_buf;
    uint32_t gl_array_size;

    /*
     * Texture for this frame. One texture for all tiles here.
     */
    texp tex;
    int bind;

    /*
     * Size of the texture in pixels.
     */
    uint32_t tex_width;
    uint32_t tex_height;

    /*
     * Single tile size.
     */
    uint32_t tex_tile_width;
    uint32_t tex_tile_height;

    /*
     * How many tiles across and down.
     */
    uint32_t tex_tiles_width;
    uint32_t tex_tiles_height;

    /*
     * And the float size of that tile in the parent tex.
     */
    float tex_float_width;
    float tex_float_height;

} map_frame_ctx_t;

map_frame_ctx_t map;

/*
 * map_tile_to_tex_coords
 *
 * Given a tile in a tile array, return the tex co-ords.
 */
static inline void
map_tile_to_tex_coords (map_frame_ctx_t *map,
                        const uint16_t tile,
                        GLfloat *tex_left,
                        GLfloat *tex_right,
                        GLfloat *tex_top,
                        GLfloat *tex_bottom)
{
    const uint16_t tx = tile % map->tex_tiles_width; 
    const uint16_t ty = tile / map->tex_tiles_height; 

    *tex_left   = map->tex_float_width * tx;
    *tex_right  = *tex_left + map->tex_float_width;
    *tex_top    = map->tex_float_height * ty;
    *tex_bottom = *tex_top + map->tex_float_height;
}

/*
 * map_chunk_init
 */
static void map_chunk_init (map_frame_ctx_t *map, uint32_t chunk)
{
    const uint32_t sx = chunk * MAP_CHUNK_WIDTH;
    const uint32_t ex = (chunk+1) * MAP_CHUNK_WIDTH;
    const uint32_t sy = 0;
    const uint32_t ey = MAP_CHUNK_HEIGHT;
    uint32_t x;
    uint32_t y;
    uint32_t cnt = 0;

    for (x = sx; x < ex; x++) {
        for (y = sy; y < ey; y++) {
            map->tiles[x][y].tile = cnt++;
            if (cnt > 64*28) {
                cnt = 0;
            }
        }
    }
}

/*
 * map_chunks_init
 */
static void map_chunks_init (map_frame_ctx_t *map)
{
    uint32_t chunk;

    for (chunk = 0; chunk < MAP_CHUNKS_WIDTH; chunk++) {
        map_chunk_init(map, chunk);
    }
}

/*
 * map_gl_init
 */
static void map_gl_init (map_frame_ctx_t *map)
{
    /*
     * Our array size requirements.
     */
    uint32_t gl_array_size_required;

    /*
     * Screen size.
     */
    uint16_t width = global_config.video_pix_width;
    uint16_t height = global_config.video_pix_height;

    /*
     * If the screen size has changed or this is the first run, allocate our
     * buffer if our size requirements have changed.
     */
    gl_array_size_required =
                    width *
                    height *
                    NUMBER_BYTES_PER_ARRAY_ELEM *
                    NUMBER_ARRAY_ELEM_ARRAYS * 2; // for degenerate triangles

    /*
     * Requirements have changed for buffer space?
     */
    if (map->gl_array_size != gl_array_size_required) {
        map->gl_array_size = gl_array_size_required;

        if (map->gl_array_buf) {
            myfree(map->gl_array_buf);
        }

        map->gl_array_buf = myzalloc(gl_array_size_required, "GL xy buffer");
    }

    if (!map->tex) {
        map->tex = tex_find("sprites_small");
        if (!map->tex) {
            return;
        }

        map->bind = tex_get_gl_binding(map->tex);

        map->tex_width = tex_get_width(map->tex);
        map->tex_height = tex_get_height(map->tex);

        map->tex_tile_width = tex_get_tile_width(map->tex);
        map->tex_tile_height = tex_get_tile_height(map->tex);

        map->tex_tiles_width = tex_get_tiles_width(map->tex);
        map->tex_tiles_height = tex_get_tiles_height(map->tex);

        map->tex_float_width  =
                        (1.0 / (float)map->tex_width) * map->tex_tile_width;
        map->tex_float_height =
                        (1.0 / (float)map->tex_height) * map->tex_tile_height;
    }
}

/*
 * map_init
 */
static void map_init (map_frame_ctx_t *map)
{
    map_gl_init(map);

    map_chunks_init(map);
}

/*
 * map_display
 *
 * Our main rendering loop.
 */
static void map_display (map_frame_ctx_t *map,
                         const uint32_t mx, const uint32_t my)
{
    glBindTexture(GL_TEXTURE_2D, map->bind);
    glcolor(WHITE);

    /*
     * Where we are currently up to in writing to these buffers.
     */
    GLfloat *bufp;

    /*
     * Our array size requirements.
     */
    uint32_t nvertices;

    /*
     * Individual co-ordinates for each tile.
     */
    GLfloat left;
    GLfloat right;
    GLfloat top;
    GLfloat bottom;

    GLfloat tex_left;
    GLfloat tex_right;
    GLfloat tex_top;
    GLfloat tex_bottom;

    /*
     * Screen size.
     */
    uint16_t width = global_config.video_pix_width;
    uint16_t height = global_config.video_pix_height;

    /*
     * Temps
     */
    uint16_t x;
    uint16_t y;
    map_tile_t *map_tile;

    tex_left   = 0;
    tex_right  = map->tex_float_width;
    tex_top    = 0;
    tex_bottom = map->tex_float_height;

    bufp = map->gl_array_buf;

    uint32_t cx = mx;
    uint32_t cy = my;

    left = 0;

    for (x = 0; x <= width - TILE_WIDTH; x += TILE_WIDTH, cx++) {

        right = left + TILE_WIDTH;
        top = 0;
        cy = my;
        y = 0;

        map_tile = &map->tiles[cx][cy];

        uint32_t tile = map_tile->tile;

        map_tile_to_tex_coords(map, tile,
                               &tex_left,
                               &tex_right,
                               &tex_top,
                               &tex_bottom);

        /*
         * Repeat the first vertex so we create a degenerate triangle.
         */
        if (cx != mx) {
            gl_push_texcoord(&bufp, tex_left,  tex_top);
            gl_push_vertex(&bufp, left,  top);
        }

        for (y = 0;
             y <= height - TILE_HEIGHT;
             y += TILE_HEIGHT, cy++, map_tile++) {

            bottom = top + TILE_HEIGHT;

            tile = map_tile->tile;

            map_tile_to_tex_coords(map, tile,
                                   &tex_left,
                                   &tex_right,
                                   &tex_top,
                                   &tex_bottom);

            /*
             * Repeat the first vertex so we create a degenerate triangle.
             */
            gl_push_texcoord(&bufp, tex_left,  tex_top);
            gl_push_vertex(&bufp, left,  top);

            gl_push_texcoord(&bufp, tex_right, tex_top);
            gl_push_vertex(&bufp, right, top);

            gl_push_texcoord(&bufp, tex_left,  tex_bottom);
            gl_push_vertex(&bufp, left,  bottom);

            gl_push_texcoord(&bufp, tex_right, tex_bottom);
            gl_push_vertex(&bufp, right, bottom);

            top += TILE_HEIGHT;
        }

        /*
         * Repeat the last vertex so we create a degenerate triangle.
         */
        gl_push_texcoord(&bufp, tex_right, tex_bottom);
        gl_push_vertex(&bufp, right, bottom);

        left += TILE_WIDTH;
    }

    /*
     * Display all the tiles selected above in one blast.
     */
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    nvertices = (bufp - map->gl_array_buf) /
                    (NUMBER_DIMENSIONS_PER_COORD * NUMBER_ARRAY_ELEM_ARRAYS);

    glTexCoordPointer(
        NUMBER_DIMENSIONS_PER_COORD, // (x,y)
        GL_FLOAT,
        NUMBER_BYTES_PER_VERTICE * 2,
        map->gl_array_buf);

    glVertexPointer(
        NUMBER_DIMENSIONS_PER_COORD, // (x,y)
        GL_FLOAT,
        NUMBER_BYTES_PER_VERTICE * 2,
        ((char*)map->gl_array_buf) + NUMBER_BYTES_PER_VERTICE);

    glBindTexture(GL_TEXTURE_2D, tex_get_gl_binding(map->tex));

    glDrawArrays(GL_TRIANGLE_STRIP, 0, nvertices);

    glBindTexture(GL_TEXTURE_2D, 0);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}

boolean fps_enabled = 1;

/*
 * User has entered a command, run it
 */
boolean fps_enable (tokens_t *tokens, void *context)
{
    fps_enabled = true;

    return (true);
}

/*
 * User has entered a command, run it
 */
boolean fps_disable (tokens_t *tokens, void *context)
{
    fps_enabled = false;

    return (true);
}

/*
 * Main loop
 */
void sdl_loop (void)
{
    SDL_Event events[10];
    boolean init_done = false;
    int32_t found;
    int32_t i;
    char fps_text[10] = {0};
    uint16_t frames = 0;

#if SDL_MAJOR_VERSION == 1 && SDL_MINOR_VERSION > 2 /* { */
    SDL_SetEventFilter(sdl_filter_events, 0);
#endif /* } */

    glEnable(GL_TEXTURE_2D);

    wid_console_hello();

    /*
     * Wait for events
     */
    int32_t timestamp_then = time_get_time_milli();
    int32_t timestamp_then2 = timestamp_then;

    sdl_main_loop_running = true;

#ifdef ENABLE_INVERTED_DISPLAY
    glClearColor(WHITE.r, WHITE.g, WHITE.b, 1.0f);
#else
    glClearColor(BLACK.r, BLACK.g, BLACK.b, 1.0f);
#endif

    while (!init_done) {
        /*
         * Clear the screen
         */
        glClear(GL_COLOR_BUFFER_BIT);

        /*
         * Splash screen loading bar for init functions.
         */
        init_done = !action_init_fns_tick(&init_fns);

        sdl_splashscreen_update();

        /*
         * Display windows.
         */
        wid_display_all();

        /*
         * Flip
         */
#ifdef ENABLE_SDL_WINDOW /* { */
        SDL_GL_SwapWindow(window);
#else /* } { */
        SDL_GL_SwapBuffers();
#endif /* } */
    }

    memset(&map, 0, sizeof(map));

    map_init(&map);

    for (;;) {
        /*
         * Clear the screen
         */
        glClear(GL_COLOR_BUFFER_BIT);

        /*
         * Do the main GL work early on to fill up the pipeline and then do
         * the game processing below to allow it to be drained before we swap 
         * the buffers.
         */
        map_display(&map, 100, 0);

        frames++;

        /*
         * Do processing of some things, like reading the keyboard or doing
         * stuff with widgets only occasionally if we do not need to.
         */
        int32_t timestamp_now = time_get_time_milli();

        if (timestamp_now - timestamp_then2 >= 1000) {

            timestamp_then2 = timestamp_now;

            /*
             * Update FSP counter.
             */
            snprintf(fps_text, sizeof(fps_text), "%u", frames);

            frames = 0;
        }

        if (timestamp_now - timestamp_then > 50) {
            /*
             * Give up some CPU to allow events to arrive and time for the GPU 
             * to process the above.
             */
            SDL_Delay(5);

            timestamp_then = timestamp_now;

            /*
             * Let widgets move.
             */
            wid_tick_all();

            /*
             * Read events
             */
            SDL_PumpEvents();

#if SDL_MAJOR_VERSION == 1 && SDL_MINOR_VERSION > 2 /* { */
            found = SDL_PeepEvents(events,
                                   ARRAY_SIZE(events),
                                   SDL_GETEVENT,
                                   SDL_QUIT,
                                   SDL_LASTEVENT);
#else /* } { */
            found = SDL_PeepEvents(events,
                                   ARRAY_SIZE(events),
                                   SDL_GETEVENT,
                                   SDL_QUITMASK|
                                   SDL_MOUSEEVENTMASK|
                                   /*
                                    * Seems not to be in SDL 1.2.14
                                   SDL_MOUSEWHEELMASK|
                                    */
                                   SDL_MOUSEMOTIONMASK|
                                   SDL_MOUSEBUTTONDOWNMASK|
                                   SDL_MOUSEBUTTONUPMASK|
                                   SDL_KEYDOWNMASK|
                                   SDL_KEYUPMASK);
#endif /* } */

            for (i = 0; i < found; ++i) {
                sdl_event(&events[i]);
            }

            if (!sdl_main_loop_running) {
                break;
            }
        }

#if 0
        /*
         * Let things move.
         */
        thing_tick_all();
#endif

        /*
         * Display windows.
         */
        wid_display_all();

        /*
         * FPS counter.
         */
        if (fps_enabled) {
            glcolor(RED);
            ttf_puts(small_font, fps_text, 0, 0, 1.0, 1.0, true);
        }

        /*
         * Flip
         */
#ifdef ENABLE_SDL_WINDOW /* { */
        SDL_GL_SwapWindow(window);
#else /* } { */
        SDL_GL_SwapBuffers();
#endif /* } */
    }
}

void sdl_show_keyboard (void)
{
#ifdef __IPHONE_OS_VERSION_MIN_REQUIRED /* { */
            SDL_iPhoneKeyboardShow(window);
#endif /* } */
}

void sdl_hide_keyboard (void)
{
#ifdef __IPHONE_OS_VERSION_MIN_REQUIRED /* { */
            SDL_iPhoneKeyboardHide(window);
#endif /* } */
}
