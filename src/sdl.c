/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <SDL.h>
#include <unistd.h>
#include "glapi.h"

#include "main.h"
#include "glapi.h"
#include "wid.h"
#include "wid_console.h"
#include "wid_notify.h"
#include "color.h"
#include "time_util.h"
#include "thing.h"
#include "sdl_util.h"
#include "init_fn.h"
#include "wid_splash.h"
#include "wid_intro_demo.h"
#include "config.h"
#include "tex.h"
#include "ttf.h"
#include "slre.h"
#include "map.h"
#include "command.h"
#include "server.h"
#include "client.h"
#include "term.h"
#include "timer.h"
#include "socket_util.h"

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

uint8_t sdl_main_loop_running;
int32_t sdl_init_video;

static double sdl_wheel_mouse_accel = 1.0;

#ifdef ENABLE_SDL_WINDOW /* { */
SDL_Window *window; /* Our window handle */
SDL_GLContext context; /* Our opengl context handle */
#endif /* } */

void sdl_fini (void)
{
    if (HEADLESS) {
        return;
    }

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

static inline uint8_t sdl_find_video_size (int32_t w, int32_t h)
{
    static uint8_t first = true;
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

    DBG("SDL video   : %dx%d not available", w, h);

    first = false;

    return (false);
}

uint8_t sdl_init (void)
{
    if (HEADLESS) {
        return (true);
    }

    int32_t VIDEO_WIDTH;
    int32_t VIDEO_HEIGHT;
    int32_t value;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        ERR("Couldn't initialize SDL: %s", SDL_GetError());
        return (false);
    }

    sdl_init_video = 1;

    INIT_LOG("SDL version : %u.%u", SDL_MAJOR_VERSION, SDL_MINOR_VERSION);

    uint8_t iphone_size = sdl_find_video_size(
                            IPHONE_VIDEO_WIDTH,
                            IPHONE_VIDEO_HEIGHT);

    uint8_t medium_size = sdl_find_video_size(
                            MEDIUM_VIDEO_WIDTH,
                            MEDIUM_VIDEO_HEIGHT);

    uint8_t ipad_size = sdl_find_video_size(
                            IPAD_VIDEO_WIDTH,
                            IPAD_VIDEO_HEIGHT);

    uint8_t default_size = sdl_find_video_size(
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
    } else if (default_size) {
        VIDEO_WIDTH = DEFAULT_VIDEO_WIDTH;
        VIDEO_HEIGHT = DEFAULT_VIDEO_HEIGHT;
    } else if (medium_size) {
        VIDEO_WIDTH = MEDIUM_VIDEO_WIDTH;
        VIDEO_HEIGHT = MEDIUM_VIDEO_HEIGHT;
    } else if (iphone_size) {
        VIDEO_WIDTH = IPHONE_VIDEO_WIDTH;
        VIDEO_HEIGHT = IPHONE_VIDEO_HEIGHT;
    } else if (ipad_size) {
        VIDEO_WIDTH = IPAD_VIDEO_WIDTH;
        VIDEO_HEIGHT = IPAD_VIDEO_HEIGHT;
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

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    /*
     * Don't use this. It seemed to mess up graphics on FireGL.
     *
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
     */

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

#if SDL_MAJOR_VERSION == 1 /* { */
    SDL_WM_SetCaption("gorynlich", "gorynlich");
#else
    SDL_SetWindowTitle(window, "gorynlich");
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

#if (SDL_MAJOR_VERSION == 2) || \
        (SDL_MAJOR_VERSION == 1 && SDL_MINOR_VERSION > 2) /* { */
    DBG("Vsync       : %d", SDL_GL_GetSwapInterval());
#endif /* } */

#if SDL_MAJOR_VERSION == 1 /* { */
    SDL_EnableKeyRepeat(150, 50);
#endif /* } */

    return (true);
}

#if (SDL_MAJOR_VERSION == 2) || \
        (SDL_MAJOR_VERSION == 1 && SDL_MINOR_VERSION > 2) /* { */
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

uint32_t mouse_down;
int32_t mouse_x;
int32_t mouse_y;

static void sdl_event (SDL_Event * event)
{
    SDL_KEYSYM *key;

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

#if (SDL_MAJOR_VERSION == 2) || \
        (SDL_MAJOR_VERSION == 1 && SDL_MINOR_VERSION > 2) /* { */
    case SDL_TEXTINPUT:
        DBG("Keyboard: text input \"%s\" in window %d",
            event->text.text, event->text.windowID);
        break;
#endif /* } */

#if (SDL_MAJOR_VERSION == 2) || \
        (SDL_MAJOR_VERSION == 1 && SDL_MINOR_VERSION > 2) /* { */
    case SDL_MOUSEWHEEL:
        DBG("Mouse: wheel scrolled %d in x and %d in y in window %d",
            event->wheel.x, event->wheel.y, event->wheel.windowID);

        SDL_GetMouseState(&mouse_x, &mouse_y);

        mouse_x *= global_config.xscale;
        mouse_y *= global_config.yscale;

        {
            static uint32_t ts;

            if (time_have_x_tenths_passed_since(5, ts)) {
                sdl_wheel_mouse_accel = 1.0;
            } else {
                sdl_wheel_mouse_accel *= ENABLE_WHEEL_SCROLL_SPEED_SCALE;

                if (sdl_wheel_mouse_accel > ENABLE_WHEEL_MAX_SCROLL_SPEED_SCALE) {
                    sdl_wheel_mouse_accel = ENABLE_WHEEL_MAX_SCROLL_SPEED_SCALE;
                }
            }

            ts = time_get_time_cached();
        }

        double wheel_x = event->wheel.x;
        double wheel_y = event->wheel.y;

        wheel_x *= sdl_wheel_mouse_accel;
        wheel_y *= sdl_wheel_mouse_accel;

        wid_mouse_motion(mouse_x, mouse_y, 0, 0, wheel_x, wheel_y);
        break;
#endif /* } */

    case SDL_MOUSEMOTION:
        mouse_down = SDL_GetMouseState(&mouse_x, &mouse_y);

        DBG("Mouse: moved to %d,%d (%d,%d) state %d",
            event->motion.x, event->motion.y,
            event->motion.xrel, event->motion.yrel, mouse_down);

        mouse_x *= global_config.xscale;
        mouse_y *= global_config.yscale;

        wid_mouse_motion(mouse_x, mouse_y,
                         event->motion.xrel, event->motion.yrel,
                         0, 0);
        break;

    case SDL_MOUSEBUTTONDOWN:
        mouse_down = SDL_GetMouseState(&mouse_x, &mouse_y);

        DBG("Mouse DOWN: button %d pressed at %d,%d state %x",
            event->button.button, event->button.x, event->button.y, 
            mouse_down);

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
        mouse_down = SDL_GetMouseState(&mouse_x, &mouse_y);

        DBG("Mouse UP: button %d released at %d,%d state %d",
            event->button.button, event->button.x, event->button.y, 
            mouse_down);

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

#if (SDL_MAJOR_VERSION == 2) || \
        (SDL_MAJOR_VERSION == 1 && SDL_MINOR_VERSION > 2) /* { */
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

uint8_t sdl_is_exiting (void)
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
 * User has entered a command, run it
 */
uint8_t fps_enable (tokens_t *tokens, void *context)
{
    char *s = tokens->args[2];

    if (!s || (*s == '\0')) {
        global_config.fps_counter = true;
    } else {
        global_config.fps_counter = strtol(s, 0, 10) ? 1 : 0;
    }

    DBG("FPS mode set to %u", global_config.fps_counter);

    return (true);
}

/*
 * User has entered a command, run it
 */
uint8_t sdl_user_exit (tokens_t *tokens, void *context)
{
    sdl_exit();

    return (true);
}

/*
 * Main loop
 */
void sdl_loop (void)
{
    SDL_Event events[10];
    uint8_t init_done = false;
    int32_t found;
    int32_t i;
    uint16_t frames = 0;

#if (SDL_MAJOR_VERSION == 2) || \
        (SDL_MAJOR_VERSION == 1 && SDL_MINOR_VERSION > 2) /* { */
    SDL_SetEventFilter(sdl_filter_events, 0);
#endif /* } */

    if (!HEADLESS) {
        glEnable(GL_TEXTURE_2D);
    }

    /*
     * Wait for events
     */
    int32_t timestamp_then = time_get_time_milli();
    int32_t timestamp_then2 = timestamp_then;

    sdl_main_loop_running = true;

    if (!HEADLESS) {
        glClearColor(BLACK.r, BLACK.g, BLACK.b, 1.0f);
    }

    /*
     * Turn on syncing for the intro screen.
     */
    if (!HEADLESS) {

#if SDL_MAJOR_VERSION == 1 && SDL_MINOR_VERSION == 2 /* { */

    SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);

#else /* } { */

    SDL_GL_SetSwapInterval(1);

#endif /* } */

    }

    while (!init_done) {
        /*
         * Clear the screen
         */
        if (!HEADLESS) {
            glClear(GL_COLOR_BUFFER_BIT);
        }

        time_update_time_milli();

        /*
         * Splash screen loading bar for init functions.
         */
        init_done = !action_init_fns_tick(&init_fns);

        sdl_splashscreen_update();

        /*
         * Let widgets move.
         */
        wid_tick_all();

        /*
         * Let widgets move.
         */
        wid_move_all();

        /*
         * Display windows.
         */
        wid_display_all();

        /*
         * Flip
         */
        if (!HEADLESS) {
#ifdef ENABLE_SDL_WINDOW /* { */
            SDL_GL_SwapWindow(window);
#else /* } { */
            SDL_GL_SwapBuffers();
#endif /* } */
        } else {
            usleep(MAIN_LOOP_DELAY);
        }
    }

    if (!HEADLESS) {
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    }

    /*
     * Don't use this. It seemed to mess up graphics on FireGL.
     *
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
     */

    for (;;) {
        /*
         * Clear the screen
         */
        if (!HEADLESS) {
            glClear(GL_COLOR_BUFFER_BIT);
        }

        time_update_time_milli();

        if (!sdl_intro_demo_update()) {
            break;
        }

        /*
         * Let widgets move.
         */
        wid_tick_all();

        /*
         * Let widgets move.
         */
        wid_move_all();

        /*
         * Display windows.
         */
        wid_display_all();

        /*
         * Flip
         */
        if (!HEADLESS) {
#ifdef ENABLE_SDL_WINDOW /* { */
            SDL_GL_SwapWindow(window);
#else /* } { */
            SDL_GL_SwapBuffers();
#endif /* } */

            SDL_Delay(MAIN_LOOP_DELAY);
        } else {
            usleep(MAIN_LOOP_DELAY);
        }
    }

    wid_console_hello();

    if (!HEADLESS) {
#if SDL_MAJOR_VERSION == 1 && SDL_MINOR_VERSION == 2 /* { */

        if (global_config.display_sync) {
            SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
        } else {
            SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 0);
        }

#else /* } { */

        if (global_config.display_sync) {
            SDL_GL_SetSwapInterval(1);
        } else {
            SDL_GL_SetSwapInterval(0);
        }

#endif /* } */
    }

    for (;;) {
        /*
         * Clear the screen
         */
        if (!HEADLESS) {
            glClear(GL_COLOR_BUFFER_BIT);
        }

        frames++;

        /*
         * Do processing of some things, like reading the keyboard or doing
         * stuff with widgets only occasionally if we do not need to.
         */
        int32_t timestamp_now = time_update_time_milli();

        if (timestamp_now - timestamp_then > 20) {

            /*
             * Wheel mouse acceleration.
             */
#if 0
            sdl_wheel_mouse_accel *= 0.9;
            if (sdl_wheel_mouse_accel < 1.0) {
                sdl_wheel_mouse_accel = 1.0;
            }
#endif

            /*
             * Give up some CPU to allow events to arrive and time for the GPU 
             * to process the above.
             */
            timestamp_then = timestamp_now;

            /*
             * Clean up dead widgets.
             */
            wid_gc_all();

            if (!HEADLESS) {
                /*
                 * Read events
                 */
                SDL_PumpEvents();

#if (SDL_MAJOR_VERSION == 2) || \
            (SDL_MAJOR_VERSION == 1 && SDL_MINOR_VERSION > 2) /* { */
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
            }

            if (HEADLESS || enable_console) {
                console_tick();
            }

            if (!sdl_main_loop_running) {
                break;
            }
        }

        /*
         * Fire global timers.
         */
        if (server_timers) {
            action_timers_tick(&server_timers);
        }

        if (client_timers) {
            action_timers_tick(&client_timers);
        }

        /*
         * Let widgets move.
         */
        wid_tick_all();

        /*
         * Network server i/o.
         */
        server_tick();

        /*
         * Network client i/o.
         */
        client_tick();

        /*
         * Flush out buffered messages.
         */
        socket_tick();

        /*
         * Let things move.
         */
        thing_tick_all();

        /*
         * Let widgets move.
         */
        wid_move_all();

        /*
         * Display windows.
         */
        wid_display_all();

        /*
         * FPS counter.
         */
        if (!HEADLESS) {
            static char fps_text[10] = {0};

            /*
             * Very occasional.
             */
            if (timestamp_now - timestamp_then2 >= 1000) {

                timestamp_then2 = timestamp_now;

                /*
                 * User notifications.
                 */
                wid_notify_tick();

                /*
                 * FPS
                 */
                if (global_config.fps_counter) {
                    /*
                     * Update FPS counter.
                     */
                    snprintf(fps_text, sizeof(fps_text), "%u", frames);

                    frames = 0;
                }
            }

            /*
             * FPS
             */
            if (global_config.fps_counter) {
                glcolor(RED);

                ttf_puts(small_font, fps_text, 0, 0, 1.0, 1.0, true);
            }
        }

        blit_flush();

        /*
         * Flip
         */
        if (!HEADLESS) {
#ifdef ENABLE_SDL_WINDOW /* { */
            SDL_GL_SwapWindow(window);
#else /* } { */
            SDL_GL_SwapBuffers();
#endif /* } */

            if (!global_config.display_sync) {
                SDL_Delay(MAIN_LOOP_DELAY);
            }
        } else {
            usleep(MAIN_LOOP_DELAY);
        }
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
