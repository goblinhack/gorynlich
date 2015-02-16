/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file.
 */

#pragma once

#include <SDL.h>
#include <SDL_keyboard.h>

#include "main.h"

uint8_t sdl_init(void);
void sdl_fini(void);
void sdl_loop(void);
void sdl_show_keyboard(void);
void sdl_hide_keyboard(void);
void sdl_exit(void);
uint8_t sdl_is_exiting(void);
void sdl_joy_rumble(float strength, uint32_t ms);

extern int32_t sdl_init_video;
extern uint8_t sdl_exiting;

#if SDL_MAJOR_VERSION == 1 /* { */
#define SDL_KEYSYM SDL_keysym
#else
#define SDL_KEYSYM SDL_Keysym
#endif /* } */

struct tokens_t_;
uint8_t sdl_user_exit(struct tokens_t_ *tokens, void *context);

extern uint32_t mouse_down;
extern int32_t mouse_x;
extern int32_t mouse_y;
extern uint8_t sdl_shift_held;
extern int sdl_joy_index;
extern int sdl_joy_axes;
extern int sdl_joy_buttons;
extern int sdl_joy_balls;
