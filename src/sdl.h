/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file.
 */

#pragma once

boolean sdl_init(void);
void sdl_fini(void);
void sdl_loop(void);
void sdl_show_keyboard(void);
void sdl_hide_keyboard(void);
void sdl_exit(void);
boolean sdl_is_exiting(void);

extern int32_t sdl_init_video;
extern boolean sdl_exiting;

#if SDL_MAJOR_VERSION == 1 /* { */
#define SDL_KEYSYM SDL_keysym
#else
#define SDL_KEYSYM SDL_Keysym
#endif /* } */

struct tokens_t_;
boolean sdl_user_exit(struct tokens_t_ *tokens, void *context);
