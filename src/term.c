/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <stdio.h>
#include <signal.h>
#include <sys/termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>

#include "main.h"
#include "marshal.h"
#include "config.h"
#include "string.h"
#include "term.h"

typedef struct zx_term_cell_ {
    char                        c;
    //
    // Indicates that this cell has been modified since the last
    // frame was drawn.
    //
    uint8_t                     fg;
    uint8_t                     bg;
    uint8_t                     touched:1;
} zx_term_cell;

static void zx_term_core_refresh(void);

int ZX_TERM_WIDTH;
int ZX_TERM_HEIGHT;
int zx_term_x;
int zx_term_y;

zx_term_color zx_term_fg_current = ZX_TERM_COLOR_WHITE;
zx_term_color zx_term_bg_current = ZX_TERM_COLOR_BLACK;

boolean zx_term_cursor_move_only;

zx_term_cell zx_term_cells[ZX_TERM_MAX_SIZE][ZX_TERM_MAX_SIZE];
zx_term_cell zx_term_bcells[ZX_TERM_MAX_SIZE][ZX_TERM_MAX_SIZE];
char zx_term_core_goto_data[ZX_TERM_CORE_MAX_SIZE][ZX_TERM_CORE_MAX_SIZE][20];

char *zx_term_core_buffer;
int zx_term_core_buffer_pos;
int zx_term_core_buffer_size;

static boolean term_init_done;
static struct termios zx_term_original_settings;

static void zx_term_core_exit (void)
{
    static boolean exitting;

    if (exitting) {
        return;
    }

    exitting = true;

    //
    // Resore the terminal
    //
    tcsetattr(0, TCSANOW, &zx_term_original_settings);

    zx_term_core_cls();

    zx_term_core_cursor_show();
    zx_term_core_refresh();

    //
    // Keep this last, after any terminal flushing
    //
    free(zx_term_core_buffer);
}

static void zx_term_core_init_zx_terminal (void)
{
    struct termios t;

    zx_term_core_buffer_size = ZX_TERM_WIDTH *
                               ZX_TERM_HEIGHT * 32;

    if (!(zx_term_core_buffer = (char*) malloc(zx_term_core_buffer_size))) {
        DIE("no mem");
    }
    
    tcgetattr(0, &zx_term_original_settings);
    memcpy(&t, &zx_term_original_settings, sizeof(struct termios));

    t.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
    t.c_oflag &= ~OPOST;
    t.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
    t.c_cflag &= ~(CSIZE|PARENB);
    t.c_cflag |= CS8;

    tcsetattr(0, TCSANOW, &t);

    zx_term_core_cls();

    zx_term_core_cursor_hide();
    zx_term_core_refresh();
}

static void zx_term_core_goto_init (void)
{
    int x, y;

    for (x = 0; x < ZX_TERM_CORE_MAX_SIZE; x++) {
        for (y = 0; y < ZX_TERM_CORE_MAX_SIZE; y++) {
            sprintf(zx_term_core_goto_data[x][y], "\033[%d;%dH", y, x);
        }
    }
}

boolean term_init (void)
{
    term_init_done = true;

    get_term_size(STDIN_FILENO, &ZX_TERM_WIDTH, &ZX_TERM_HEIGHT);

    zx_term_core_init_zx_terminal();

    zx_term_core_goto_init();

    zx_term_cls_now();

    return (true);
}

void term_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (term_init_done) {
        zx_term_core_exit();

        term_init_done = false;
    }
}

static void zx_term_core_putc (const char c)
{
    zx_term_core_buffer[zx_term_core_buffer_pos++] = c;

    if (zx_term_core_buffer_pos >= zx_term_core_buffer_size) {
        zx_term_core_buffer_size <<= 1;
        zx_term_core_buffer = (char*) realloc(zx_term_core_buffer,
                                              zx_term_core_buffer_size);

        if (!zx_term_core_buffer) {
            DIE("no mem");
        }
    }
}

static void zx_term_core_puts (const char *str)
{
    const int l = strlen(str);

    if (zx_term_core_buffer_pos + l >= zx_term_core_buffer_size) {
        while (zx_term_core_buffer_pos + l >= zx_term_core_buffer_size) {
            zx_term_core_buffer_size <<= 1;
        }

        zx_term_core_buffer = (char*) realloc(zx_term_core_buffer,
                                              zx_term_core_buffer_size);

        if (!zx_term_core_buffer) {
            DIE("no mem");
        }
    }

    memcpy(zx_term_core_buffer + zx_term_core_buffer_pos, str, l);

    zx_term_core_buffer_pos += l;
}

static void zx_term_core_refresh (void)
{
    write(1, zx_term_core_buffer, zx_term_core_buffer_pos);

    zx_term_core_buffer_pos = 0;
}

static inline void zx_term_core_fg (unsigned char a)
{
    static char *data[] = {
        "[30m", "[31m", "[32m", "[33m",
        "[34m", "[35m", "[36m", "[37m",
    };

    zx_term_core_puts(data[a & 7]);
}

static inline void zx_term_core_bg (unsigned char a)
{
    static char *data[] = {
        "[40m", "[41m", "[42m", "[43m",
        "[44m", "[45m", "[46m", "[47m",
    };

    zx_term_core_puts(data[a & 7]);
}

static void zx_term_core_fgbg (unsigned char fg, unsigned char bg)
{
    static const char *data[] = {
            "[40;30m", "[40;31m", "[40;32m", "[40;33m",
            "[40;34m", "[40;35m", "[40;36m", "[40;37m",
            "[41;30m", "[41;31m", "[41;32m", "[41;33m",
            "[41;34m", "[41;35m", "[41;36m", "[41;37m",
            "[42;30m", "[42;31m", "[42;32m", "[42;33m",
            "[42;34m", "[42;35m", "[42;36m", "[42;37m",
            "[43;30m", "[43;31m", "[43;32m", "[43;33m",
            "[43;34m", "[43;35m", "[43;36m", "[43;37m",
            "[44;30m", "[44;31m", "[44;32m", "[44;33m",
            "[44;34m", "[44;35m", "[44;36m", "[44;37m",
            "[45;30m", "[45;31m", "[45;32m", "[45;33m",
            "[45;34m", "[45;35m", "[45;36m", "[45;37m",
            "[46;30m", "[46;31m", "[46;32m", "[46;33m",
            "[46;34m", "[46;35m", "[46;36m", "[46;37m",
            "[47;30m", "[47;31m", "[47;32m", "[47;33m",
            "[47;34m", "[47;35m", "[47;36m", "[47;37m",
    };

    zx_term_core_puts((char*)data[(bg & 7) * 8 + (fg & 7)]);
}

void zx_term_core_cursor_show (void)
{
    zx_term_core_puts("\033[?25h");
}

void zx_term_core_cursor_hide (void)
{
    zx_term_core_puts("\033[?25l");
}

void zx_term_core_goto (int x, int y)
{
    zx_term_core_puts(zx_term_core_goto_data[x][y]);
}

void zx_term_core_cls (void)
{
    zx_term_core_refresh();

    //
    // Clear the screen with black
    //
    zx_term_core_goto(0, 0);
    zx_term_core_bg(0);
    zx_term_core_puts("\033[2J");
    zx_term_core_refresh();
}

void zx_term_core_bell (void)
{
    zx_term_core_puts("\007");
}

static void zx_term_put (zx_term_cell *e)
{
    if (zx_term_cursor_move_only) {
        zx_term_x++;
        return;
    }

    if (zx_term_x >= ZX_TERM_WIDTH) {
        zx_term_x++;
        return;
    }

    if (zx_term_x < 0) {
        zx_term_x++;
        return;
    }

    if (zx_term_y >= ZX_TERM_HEIGHT) {
        zx_term_x++;
        return;
    }

    if (zx_term_y < 0) {
        zx_term_x++;
        return;
    }

    zx_term_cell *o = &zx_term_cells[zx_term_x][zx_term_y];

    e->touched = true;

    *o = *e;
    zx_term_x++;
}

void zx_term_cursor_right (void)
{
    zx_term_x++;
}

void zx_term_putc (const char c)
{
    zx_term_cell e = {0};

    e.c = c; 
    e.fg = zx_term_fg_current;
    e.bg = zx_term_bg_current;

    zx_term_put(&e);
}

void zx_term_puts (const char* s)
{
    char c;

    while ((c = *s++) != '\0') {
        zx_term_putc(c);
    }
}

void zx_term_fill_c (const char c)
{
    int x, y;

    for (x = 0; x < ZX_TERM_WIDTH; x++) {
        for (y = 0; y < ZX_TERM_HEIGHT; y++) {
            zx_term_cells[x][y].c = c;
            zx_term_cells[x][y].touched = true;
        }
    }
}

void zx_term_scroll (void)
{
    int x, y;

    for (x = 0; x < ZX_TERM_WIDTH; x++) {
        for (y = 0; y < ZX_TERM_HEIGHT; y++) {
            if (y >= ZX_TERM_HEIGHT - 2) {
                zx_term_cells[x][y].c  = ' ';
                zx_term_cells[x][y].fg = 0;
                zx_term_cells[x][y].bg = 0;
                zx_term_cells[x][y].touched = true;
            } else {
                zx_term_cells[x][y].c  = zx_term_cells[x][y + 1].c;
                zx_term_cells[x][y].fg = zx_term_cells[x][y + 1].fg;
                zx_term_cells[x][y].bg = zx_term_cells[x][y + 1].bg;
                zx_term_cells[x][y].touched = true;
            }
        }
    }
}

void zx_term_fill_fg (const zx_term_color fg)
{
    int x, y;

    for (x = 0; x < ZX_TERM_WIDTH; x++) {
        for (y = 0; y < ZX_TERM_HEIGHT; y++) {
            zx_term_cells[x][y].fg = fg;
            zx_term_cells[x][y].touched = true;
        }
    }
}

void zx_term_fill_bg (const zx_term_color bg)
{
    int x, y;

    for (x = 0; x < ZX_TERM_WIDTH; x++) {
        for (y = 0; y < ZX_TERM_HEIGHT; y++) {
            zx_term_cells[x][y].bg = bg;
            zx_term_cells[x][y].touched = true;
        }
    }
}

void zx_term_fg (zx_term_color fg)
{
    zx_term_fg_current = fg;
}

void zx_term_bg (zx_term_color bg)
{
    zx_term_bg_current = bg;
}

void zx_term_fgbg (zx_term_color fg, zx_term_color bg)
{
    zx_term_fg_current = fg;
    zx_term_bg_current = bg;
}

void zx_term_goto (int x, int y)
{
    zx_term_x = x;
    zx_term_y = y;
}

void zx_term_cursor_show (void)
{
    zx_term_core_cursor_show();
}

void zx_term_cursor_hide (void)
{
    zx_term_core_cursor_hide();
}

void zx_term_bell (void)
{
    zx_term_core_bell();
}

void zx_term_cls (void)
{
    zx_term_fill_c(' ');
    zx_term_fill_bg(ZX_TERM_COLOR_BLACK);
    zx_term_fill_fg(ZX_TERM_COLOR_BLACK);
}

void zx_term_cls_now (void)
{
    zx_term_cls();

    zx_term_refresh();

    zx_term_core_cls();
}

void zx_term_clear_buffer (void)
{
    memset(zx_term_cells, 0, sizeof(zx_term_cells));
}

void zx_term_clear_bbuffer (void)
{
    memset(zx_term_bcells, 0, sizeof(zx_term_bcells));
}

static zx_term_color zx_term_color_string_to_index (const char **s)
{
    if (!strncmp(*s, "black$", sizeof("black$")-1)) {
        *s += sizeof("black$")-1;
        return (ZX_TERM_COLOR_BLACK);
    }
    if (!strncmp(*s, "red$", sizeof("red$")-1)) {
        *s += sizeof("red$")-1;
        return (ZX_TERM_COLOR_RED);
    }
    if (!strncmp(*s, "green$", sizeof("green$")-1)) {
        *s += sizeof("green$")-1;
        return (ZX_TERM_COLOR_GREEN);
    }
    if (!strncmp(*s, "yellow$", sizeof("yellow$")-1)) {
        *s += sizeof("yellow$")-1;
        return (ZX_TERM_COLOR_YELLOW);
    }
    if (!strncmp(*s, "blue$", sizeof("blue$")-1)) {
        *s += sizeof("blue$")-1;
        return (ZX_TERM_COLOR_BLUE);
    }
    if (!strncmp(*s, "pink$", sizeof("pink$")-1)) {
        *s += sizeof("pink$")-1;
        return (ZX_TERM_COLOR_PINK);
    }
    if (!strncmp(*s, "cyan$", sizeof("cyan$")-1)) {
        *s += sizeof("cyan$")-1;
        return (ZX_TERM_COLOR_CYAN);
    }
    if (!strncmp(*s, "white$", sizeof("white$")-1)) {
        *s += sizeof("white$")-1;
        return (ZX_TERM_COLOR_WHITE);
    }
    if (!strncmp(*s, "reset$", sizeof("reset$")-1)) {
        *s += sizeof("reset$")-1;
        return (ZX_TERM_COLOR_RESET);
    }

    return (ZX_TERM_COLOR_WHITE);
}

void zx_term_putf (const char *s)
{
    char c;
    boolean looking_for_start = false;

    while ((c = *s++) != '\0') {

        if (!looking_for_start) {
            if (c == '%') {
                    looking_for_start = true;
                    continue;
            }
        } else if (looking_for_start) {
            if (c == '%') {
                if (!strncmp(s, "fg=", 3)) {
                    s += 3;
                    zx_term_fg(zx_term_color_string_to_index(&s));
                    looking_for_start = false;
                    continue;
                }

                if (!strncmp(s, "bg=", 3)) {
                    s += 3;
                    zx_term_bg(zx_term_color_string_to_index(&s));
                    looking_for_start = false;
                    continue;
                }
            }
            zx_term_putc(c);
        }

        looking_for_start = false;

        zx_term_putc(c);
    }
}

void zx_term_putfy (int y, const char *s)
{
    zx_term_goto(0, y);

    zx_term_cursor_move_only = true;
    zx_term_putf(s);
    zx_term_cursor_move_only = false;

    zx_term_goto(ZX_TERM_WIDTH/2 - zx_term_x/2, y);
    zx_term_putf(s);
}

void zx_term_putfmx (const char *s)
{
    zx_term_putfy(zx_term_y, s);
}

void zx_term_refresh (void)
{
    int x, y;
    boolean zx_term_is_cursor_valid = false;

    for (y = 0; y < ZX_TERM_HEIGHT; y++) {
        zx_term_is_cursor_valid = false;

        //
        // Hitting the max col seems to mess the terminal up
        //
        for (x = 0; x < ZX_TERM_WIDTH; x++) {

            zx_term_cell *c = &zx_term_cells[x][y];

            if (!c->touched) {
                //
                // We're skipping some spaces. Need a goto when we restart.
                //
                zx_term_is_cursor_valid = false;
                continue;
            }

            zx_term_cell *b = &zx_term_bcells[x][y];

            boolean cell_c_back_buffer_changed = (b->c != c->c);
            boolean cell_fg_back_buffer_changed = (b->fg != c->fg);
            boolean cell_bg_back_buffer_changed = (b->bg != c->bg);

            /*
             * Any changes in this cell with the back buffer cell?
             */
            if (!cell_c_back_buffer_changed &&
                !cell_fg_back_buffer_changed &&
                !cell_bg_back_buffer_changed) {
                //
                // We're skipping some spaces. Need a goto when we restart.
                //
                zx_term_is_cursor_valid = false;
                continue;
            }

            /*
             * Has the cursor moved more than 1 char since current_pen change?
             * If so we need a goto.
             */
            if (!zx_term_is_cursor_valid) {
                zx_term_is_cursor_valid = true;
                /*
                 * Terminal lines start at 1
                 */
                zx_term_core_goto(x + 1, y + 1);
            }

            zx_term_core_fgbg(c->fg, c->bg);

            if (!c->c) {
                zx_term_core_putc(' ');
            } else {
                zx_term_core_putc(c->c);
            }
        }
    }

    memcpy(zx_term_bcells, zx_term_cells, sizeof(zx_term_cells));

    zx_term_core_refresh();
}

/*
    Copyright (C) 1996 1997 Uwe Ohse

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    Contact: uwe@tirka.gun.de, Uwe Ohse @ DU3 (mausnet)
*/

int term_test (int32_t argc, char *argv[])
{
    int x, y;

    zx_term_core_cls();

    int r = 0;
    for (;;) {
        int i;

        for (i = 0; i < 40; i++) {
            r++;

            for (x = 0; x < ZX_TERM_CORE_MAX_SIZE; x++) {
                for (y = 0; y < ZX_TERM_CORE_MAX_SIZE; y++) {
                    zx_term_goto(x, y);

                    int d = (x + y + r);

                    zx_term_putc('a' + (d % 26));
                    zx_term_fgbg(d % 8, d % 8);
                }
            }

            term_log("------------abcdef-----------------");
            term_log("-                                 -");
            term_log("------------abcdef-----------------");

            zx_term_refresh();
        }

        for (i = 0; i < 40; i++) {
            zx_term_scroll();
            zx_term_refresh();
            usleep(50000);
        }

        zx_term_refresh();
    }
}

void term_log (const char *buf)
{
    zx_term_scroll();
    zx_term_goto(0, ZX_TERM_HEIGHT - 2);
    zx_term_putf(buf);
    zx_term_refresh();
}

/*
 * inquire actual terminal size (this it what the 
 * kernel thinks - not was the user on the over end
 * of the phone line has really).
 */
int
get_term_size (int fd, int *x, int *y)
{
#ifdef TIOCGSIZE
    struct ttysize win;

#elif defined(TIOCGWINSZ)
    struct winsize win;

#endif

#ifdef TIOCGSIZE
    if (ioctl (fd, TIOCGSIZE, &win))
        return 0;
    if (y)
        *y=win.ts_lines;
    if (x)
        *x=win.ts_cols;
#elif defined TIOCGWINSZ
    if (ioctl (fd, TIOCGWINSZ, &win))
        return 0;
    if (y)
        *y=win.ws_row;
    if (x)
        *x=win.ws_col;
#else
    {
        const char *s;
        s=getenv("LINES");
        if (s)
            *y=strtol(s,NULL,10);
        else
            *y=25;
        s=getenv("COLUMNS");
        if (s)
            *x=strtol(s,NULL,10);
        else
            *x=80;
    }
#endif
    return 1;
}

