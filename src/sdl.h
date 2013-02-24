/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file.
 */

boolean sdl_init(void);
void sdl_fini(void);
void sdl_loop(void);
void sdl_show_keyboard(void);
void sdl_hide_keyboard(void);
void sdl_exit(void);
boolean sdl_is_exiting(void);

extern int32_t sdl_init_video;
