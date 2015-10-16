/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */

#pragma once

#include <string.h>

extern uint8_t term_init(void);
extern void term_fini(void);
extern int get_term_size(int fd, int *x, int *y);

extern int TERM_WIDTH;
extern int TERM_HEIGHT;

#define TERM_CORE_MAX_SIZE   128
#define TERM_MAX_SIZE        TERM_CORE_MAX_SIZE

typedef enum {
    TERM_COLOR_BLACK,
    TERM_COLOR_RED,
    TERM_COLOR_GREEN,
    TERM_COLOR_YELLOW,
    TERM_COLOR_BLUE,
    TERM_COLOR_PINK,
    TERM_COLOR_CYAN,
    TERM_COLOR_WHITE,
    TERM_COLOR_RESET,
    TERM_COLOR_MAX,
} term_color;

extern int term_core_screen_width;
extern int term_core_screen_height;
extern char *term_core_buffer;
extern int term_core_buffer_pos;
extern int term_core_buffer_size;

//
// Speeds up goto strings to the terminal
//
extern char term_core_goto_data[TERM_CORE_MAX_SIZE]
                                  [TERM_CORE_MAX_SIZE][20];

//
// Formatted puts, with %%fg=yellow being the format style
//
extern void term_putf(const char *s);

//
// Formatted and centered on the x axis
//
extern void term_putfy(int y, const char *s);

//
// Current y, x middle
//
extern void term_putfmx(const char *s);

extern void term_refresh(void);

extern void term_core_cursor_show(void);
extern void term_core_cursor_hide(void);
extern void term_core_goto(int x, int y);
extern void term_core_cls(void);
extern void term_core_bell(void);
extern void term_cursor_right(void);
extern void term_putc(const char c);
extern void term_puts(const char* s);
extern void term_fill_c(const char c);
extern void term_fill_fg(const term_color fg);
extern void term_fill_bg(const term_color bg);
extern void term_fg(term_color fg);
extern void term_bg(term_color bg);
extern void term_fgbg(term_color fg, term_color bg);
extern void term_goto(int x, int y);
extern void term_cursor_show(void);
extern void term_cursor_hide(void);
extern void term_bell(void);
extern void term_cls(void);
extern void term_cls_now(void);
extern void term_clear_buffer(void);
extern void term_clear_bbuffer(void);
extern int term_test(int32_t argc, char *argv[]);
extern void term_scroll(void);
extern void term_log(const char *buf);
extern int enable_console;
