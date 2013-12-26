/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <string.h>

extern boolean term_init(void);
extern void term_fini(void);
extern int get_term_size(int fd, int *x, int *y);

extern int ZX_TERM_WIDTH;
extern int ZX_TERM_HEIGHT;

#define ZX_TERM_CORE_MAX_SIZE   512
#define ZX_TERM_MAX_SIZE        ZX_TERM_CORE_MAX_SIZE

typedef enum {
    ZX_TERM_COLOR_BLACK,
    ZX_TERM_COLOR_RED,
    ZX_TERM_COLOR_GREEN,
    ZX_TERM_COLOR_YELLOW,
    ZX_TERM_COLOR_BLUE,
    ZX_TERM_COLOR_PINK,
    ZX_TERM_COLOR_CYAN,
    ZX_TERM_COLOR_WHITE,
    ZX_TERM_COLOR_RESET,
    ZX_TERM_COLOR_MAX,
} zx_term_color;

extern int zx_term_core_screen_width;
extern int zx_term_core_screen_height;
extern char *zx_term_core_buffer;
extern int zx_term_core_buffer_pos;
extern int zx_term_core_buffer_size;

//
// Speeds up goto strings to the terminal
//
extern char zx_term_core_goto_data[ZX_TERM_CORE_MAX_SIZE]
[ZX_TERM_CORE_MAX_SIZE][20];

extern int zx_term_core_init(void);
extern void zx_term_core_exit(int sig);
extern void zx_term_core_sigwinch_handler(int sig);

//
// Formatted puts, with %%fg=yellow being the format style
//
extern void zx_term_putf(const char *s);

//
// Formatted and centered on the x axis
//
extern void zx_term_putfy(int y, const char *s);

//
// Current y, x middle
//
extern void zx_term_putfmx(const char *s);

extern void zx_term_refresh(void);

extern void zx_term_core_cursor_show(void);
extern void zx_term_core_cursor_hide(void);
extern void zx_term_core_goto(int x, int y);
extern void zx_term_core_cls(void);
extern void zx_term_core_bell(void);
extern void zx_term_cursor_right(void);
extern void zx_term_putc(const char c);
extern void zx_term_puts(const char* s);
extern void zx_term_fill_c(const char c);
extern void zx_term_fill_fg(const zx_term_color fg);
extern void zx_term_fill_bg(const zx_term_color bg);
extern void zx_term_fg(zx_term_color fg);
extern void zx_term_bg(zx_term_color bg);
extern void zx_term_fgbg(zx_term_color fg, zx_term_color bg);
extern void zx_term_goto(int x, int y);
extern void zx_term_cursor_show(void);
extern void zx_term_cursor_hide(void);
extern void zx_term_bell(void);
extern void zx_term_cls(void);
extern void zx_term_cls_now(void);
extern void zx_term_clear_buffer(void);
extern void zx_term_clear_bbuffer(void);
extern int term_test(int32_t argc, char *argv[]);
extern void zx_term_scroll(void);
extern void term_log(const char *buf);
