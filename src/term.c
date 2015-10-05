/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license.
 */

#include <stdio.h>
#include <signal.h>
#ifndef _WIN32
#include <sys/termios.h>
#include <sys/ioctl.h>
#endif
#include <unistd.h>
#include <stdlib.h>

#include "main.h"
#include "marshal.h"
#include "config.h"
#include "string_util.h"
#include "term.h"

typedef struct term_cell_ {
    char                        c;
    //
    // Indicates that this cell has been modified since the last
    // frame was drawn.
    //
    uint8_t                     fg:3;
    uint8_t                     bg:3;
    uint8_t                     touched:1;
} term_cell;

#ifdef ENABLE_CONSOLE
int enable_console = 1;
#else
int enable_console = 0;
#endif

#ifndef _WIN32
static void term_core_refresh(void);

int TERM_WIDTH = TERM_MAX_SIZE;
int TERM_HEIGHT = TERM_MAX_SIZE;
int term_x;
int term_y;

term_color term_fg_current = TERM_COLOR_RESET;
term_color term_bg_current = TERM_COLOR_BLACK;

uint8_t term_cursor_move_only;

term_cell term_cells[TERM_MAX_SIZE][TERM_MAX_SIZE];
term_cell term_bcells[TERM_MAX_SIZE][TERM_MAX_SIZE];
char term_core_goto_data[TERM_CORE_MAX_SIZE][TERM_CORE_MAX_SIZE][20];

char *term_core_buffer;
int term_core_buffer_pos;
int term_core_buffer_size;

static uint8_t term_init_done;
static struct termios term_original_settings;
#endif

#ifndef _WIN32
static void term_core_exit (void)
{
    static uint8_t exitting;

    if (exitting) {
        return;
    }

    exitting = true;

    term_core_cursor_show();
    term_core_refresh();

    //
    // Keep this last, after any terminal flushing
    //

    free(term_core_buffer);

    //
    // Resore the terminal
    //
    tcsetattr(0, TCSANOW, &term_original_settings);
}

static void term_core_init_terminal (void)
{
    struct termios t;

    tcgetattr(0, &term_original_settings);
    memcpy(&t, &term_original_settings, sizeof(struct termios));

    term_core_buffer_size = TERM_WIDTH * TERM_HEIGHT * 32;

    if (!(term_core_buffer = (char*) malloc(term_core_buffer_size))) {
        ERR("no mem");
    }
    
    t.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
    t.c_oflag &= ~OPOST;
    t.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
    t.c_cflag &= ~(CSIZE|PARENB);
    t.c_cflag |= CS8;

    tcsetattr(0, TCSANOW, &t);

    term_core_cls();

    term_core_cursor_hide();
    term_core_refresh();
}

static void term_core_goto_init (void)
{
    int x, y;

    for (x = 0; x < TERM_CORE_MAX_SIZE; x++) {
        for (y = 0; y < TERM_CORE_MAX_SIZE; y++) {
            sprintf(term_core_goto_data[x][y], "\033[%d;%dH", y, x);
        }
    }
}
#endif

uint8_t term_init (void)
{
#ifndef _WIN32
    if (!HEADLESS && !enable_console) {
        return (true);
    }

    if (term_init_done) {
        return (true);
    }

    term_init_done = true;

    get_term_size(STDIN_FILENO, &TERM_WIDTH, &TERM_HEIGHT);

    if (TERM_WIDTH > TERM_MAX_SIZE) {
        TERM_WIDTH = TERM_MAX_SIZE;
    }

    if (TERM_HEIGHT > TERM_MAX_SIZE) {
        TERM_HEIGHT = TERM_MAX_SIZE;
    }

    term_core_init_terminal();

    term_core_goto_init();

    term_cls_now();

#endif
    return (true);
}

void term_fini (void)
{
#ifndef _WIN32
    FINI_LOG("%s", __FUNCTION__);

    if (term_init_done) {
        term_init_done = false;

        term_core_exit();

        /*
         * Extra cleaning if needed.
         *
        printf("\033[m");
        printf("\033c");
         */
    }
#endif
}

#ifndef _WIN32
static void term_core_putc (const char c)
{
    term_core_buffer[term_core_buffer_pos++] = c;

    if (term_core_buffer_pos >= term_core_buffer_size) {
        term_core_buffer_size <<= 1;
        term_core_buffer = (char*) realloc(term_core_buffer,
                                              term_core_buffer_size);

        if (!term_core_buffer) {
            ERR("no mem");
        }
    }
}

static void term_core_puts (const char *str)
{
    const size_t l = strlen(str);

    if (term_core_buffer_pos + l >= term_core_buffer_size) {
        while (term_core_buffer_pos + l >= term_core_buffer_size) {
            term_core_buffer_size <<= 1;
        }

        term_core_buffer = (char*) realloc(term_core_buffer,
                                              term_core_buffer_size);

        if (!term_core_buffer) {
            ERR("no mem");
        }
    }

    memcpy(term_core_buffer + term_core_buffer_pos, str, l);

    term_core_buffer_pos += l;
}

static void term_core_refresh (void)
{
    if (write(1, term_core_buffer, term_core_buffer_pos) < 0) {
        /*
         * Really what can we do?
         */
    }

    term_core_buffer_pos = 0;
}

static inline void term_core_fg (unsigned char a)
{
    static const char *data[] = {
        "[30m", "[31m", "[32m", "[33m",
        "[34m", "[35m", "[36m", "[37m",
        "\033[m",
    };

    if (a >= ARRAY_SIZE(data)) {
        ERR("overflow");
    }

    term_core_puts(data[a]);
}

static void term_core_fgbg (unsigned char fg, unsigned char bg)
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

    if (bg == 0) {
        /*
         * Allow the default color to be 0, that of the terminal.
         */
        term_core_fg(fg);
        return;
    }

    term_core_puts((char*)data[(bg & 7) * 8 + (fg & 7)]);
}

static inline void term_puts_fg (unsigned char a)
{
    static const char *data[] = {
        "[30m", "[31m", "[32m", "[33m",
        "[34m", "[35m", "[36m", "[37m",
        "\033[m",
    };

    if (a >= ARRAY_SIZE(data)) {
        ERR("overflow");
    }

    fputs(data[a], stdout);
}

static inline void term_puts_fgbg (unsigned char fg, unsigned char bg)
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

    if (bg == 0) {
        /*
         * Allow the default color to be 0, that of the terminal.
         */
        term_puts_fg(fg);
        return;
    }

    fputs((char*)data[(bg & 7) * 8 + (fg & 7)], stdout);
}
#endif

void term_core_cursor_show (void)
{
#ifndef _WIN32
    term_core_puts("\033[?25h");
#endif
}

void term_core_cursor_hide (void)
{
#ifndef _WIN32
    term_core_puts("\033[?25l");
#endif
}

void term_core_goto (int x, int y)
{
#ifndef _WIN32
    term_core_puts(term_core_goto_data[x][y]);
#endif
}

void term_core_cls (void)
{
#ifndef _WIN32
    term_core_refresh();

    //
    // Clear the screen with black
    //
    term_core_goto(0, 0);
    term_core_puts("\033[2J");
    term_core_refresh();
#endif
}

void term_core_bell (void)
{
#ifndef _WIN32
    term_core_puts("\007");
#endif
}

#ifndef _WIN32
static void term_put (term_cell *e)
{
    if (term_cursor_move_only) {
        term_x++;
        return;
    }

    if (term_x >= TERM_MAX_SIZE) { 
        // not screen width so we can store long lines
        term_x++;
        return;
    }

    if (term_x < 0) {
        term_x++;
        return;
    }

    if (term_y >= TERM_HEIGHT) {
        term_x++;
        return;
    }

    if (term_y < 0) {
        term_x++;
        return;
    }

    term_cell *o = &term_cells[term_x][term_y];

    e->touched = true;

    *o = *e;
    term_x++;
}
#endif

void term_cursor_right (void)
{
#ifndef _WIN32
    term_x++;
#endif
}

void term_putc (const char c)
{
#ifndef _WIN32
    term_cell e = {0};

    e.c = c; 
    e.fg = term_fg_current;
    e.bg = term_bg_current;

    term_put(&e);
#endif
}

void term_puts (const char* s)
{
#ifndef _WIN32
    char c;

    while ((c = *s++) != '\0') {
        term_putc(c);
    }
#endif
}

void term_fill_c (const char c)
{
#ifndef _WIN32
    int x, y;

    for (x = 0; x < TERM_WIDTH; x++) {
        for (y = 0; y < TERM_HEIGHT; y++) {
            term_cells[x][y].c = c;
            term_cells[x][y].touched = true;
        }
    }
#endif
}

void term_scroll (void)
{
#ifndef _WIN32
    int x, y;

    for (x = 0; x < TERM_WIDTH; x++) {
        for (y = 0; y < TERM_HEIGHT; y++) {
            if (y >= TERM_HEIGHT - 2) {
                term_cells[x][y].c  = ' ';
                term_cells[x][y].fg = 0;
                term_cells[x][y].bg = 0;
                term_cells[x][y].touched = true;
            } else {
                term_cells[x][y].c  = term_cells[x][y + 1].c;
                term_cells[x][y].fg = term_cells[x][y + 1].fg;
                term_cells[x][y].bg = term_cells[x][y + 1].bg;
                term_cells[x][y].touched = true;
            }
        }
    }
#endif
}

void term_fill_fg (const term_color fg)
{
#ifndef _WIN32
    int x, y;

    for (x = 0; x < TERM_WIDTH; x++) {
        for (y = 0; y < TERM_HEIGHT; y++) {
            term_cells[x][y].fg = fg;
            term_cells[x][y].touched = true;
        }
    }
#endif
}

void term_fill_bg (const term_color bg)
{
#ifndef _WIN32
    int x, y;

    for (x = 0; x < TERM_WIDTH; x++) {
        for (y = 0; y < TERM_HEIGHT; y++) {
            term_cells[x][y].bg = bg;
            term_cells[x][y].touched = true;
        }
    }
#endif
}

void term_fg (term_color fg)
{
#ifndef _WIN32
    term_fg_current = fg;
#endif
}

void term_bg (term_color bg)
{
#ifndef _WIN32
    term_bg_current = bg;
#endif
}

void term_fgbg (term_color fg, term_color bg)
{
#ifndef _WIN32
    term_fg_current = fg;
    term_bg_current = bg;
#endif
}

void term_goto (int x, int y)
{
#ifndef _WIN32
    term_x = x;
    term_y = y;
#endif
}

void term_cursor_show (void)
{
#ifndef _WIN32
    term_core_cursor_show();
#endif
}

void term_cursor_hide (void)
{
#ifndef _WIN32
    term_core_cursor_hide();
#endif
}

void term_bell (void)
{
#ifndef _WIN32
    term_core_bell();
#endif
}

void term_cls (void)
{
#ifndef _WIN32
    term_fill_c(' ');
#endif
}

void term_cls_now (void)
{
#ifndef _WIN32
    term_cls();

    term_refresh();

    term_core_cls();
#endif
}

void term_clear_buffer (void)
{
#ifndef _WIN32
    memset(term_cells, 0, sizeof(term_cells));
#endif
}

void term_clear_bbuffer (void)
{
#ifndef _WIN32
    memset(term_bcells, 0, sizeof(term_bcells));
#endif
}

#ifndef _WIN32
static term_color term_color_string_to_index (const char **s)
{
    if (!strncmp(*s, "black$", sizeof("black$")-1)) {
        *s += sizeof("black$")-1;
        return (TERM_COLOR_BLACK);
    }
    if (!strncmp(*s, "red$", sizeof("red$")-1)) {
        *s += sizeof("red$")-1;
        return (TERM_COLOR_RED);
    }
    if (!strncmp(*s, "green$", sizeof("green$")-1)) {
        *s += sizeof("green$")-1;
        return (TERM_COLOR_GREEN);
    }
    if (!strncmp(*s, "yellow$", sizeof("yellow$")-1)) {
        *s += sizeof("yellow$")-1;
        return (TERM_COLOR_YELLOW);
    }
    if (!strncmp(*s, "blue$", sizeof("blue$")-1)) {
        *s += sizeof("blue$")-1;
        return (TERM_COLOR_BLUE);
    }
    if (!strncmp(*s, "pink$", sizeof("pink$")-1)) {
        *s += sizeof("pink$")-1;
        return (TERM_COLOR_PINK);
    }
    if (!strncmp(*s, "cyan$", sizeof("cyan$")-1)) {
        *s += sizeof("cyan$")-1;
        return (TERM_COLOR_CYAN);
    }
    if (!strncmp(*s, "white$", sizeof("white$")-1)) {
        *s += sizeof("white$")-1;
        return (TERM_COLOR_WHITE);
    }
    if (!strncmp(*s, "reset$", sizeof("reset$")-1)) {
        *s += sizeof("reset$")-1;
        return (TERM_COLOR_RESET);
    }

    return (TERM_COLOR_WHITE);
}
#endif

void term_putf (const char *s)
{
#ifndef _WIN32
    char c;
    uint8_t looking_for_start = false;

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
                    term_fg(term_color_string_to_index(&s));
                    looking_for_start = false;
                    continue;
                }

                if (!strncmp(s, "bg=", 3)) {
                    s += 3;
                    term_bg(term_color_string_to_index(&s));
                    looking_for_start = false;
                    continue;
                }
            }
            term_putc(c);
        }

        looking_for_start = false;

        term_putc(c);
    }
#endif
}

void term_putfy (int y, const char *s)
{
#ifndef _WIN32
    term_goto(0, y);

    term_cursor_move_only = true;
    term_putf(s);
    term_cursor_move_only = false;

    term_goto(TERM_WIDTH/2 - term_x/2, y);
    term_putf(s);
#endif
}

void term_putfmx (const char *s)
{
#ifndef _WIN32
    term_putfy(term_y, s);
#endif
}

void term_refresh (void)
{
#ifndef _WIN32
    if (!term_init_done) {
        return;
    }

    int x, y;
    uint8_t term_is_cursor_valid = false;

    for (y = 0; y < TERM_HEIGHT; y++) {
        term_is_cursor_valid = false;

        //
        // Hitting the max col seems to mess the terminal up
        //
        for (x = 0; x < TERM_WIDTH; x++) {

            term_cell *c = &term_cells[x][y];

            if (!c->touched) {
                //
                // We're skipping some spaces. Need a goto when we restart.
                //
                term_is_cursor_valid = false;
                continue;
            }

            term_cell *b = &term_bcells[x][y];

            uint8_t cell_c_back_buffer_changed = (b->c != c->c);
            uint8_t cell_fg_back_buffer_changed = (b->fg != c->fg);
            uint8_t cell_bg_back_buffer_changed = (b->bg != c->bg);

            /*
             * Any changes in this cell with the back buffer cell?
             */
            if (!cell_c_back_buffer_changed &&
                !cell_fg_back_buffer_changed &&
                !cell_bg_back_buffer_changed) {
                //
                // We're skipping some spaces. Need a goto when we restart.
                //
                term_is_cursor_valid = false;
                continue;
            }

            /*
             * Has the cursor moved more than 1 char since current_pen change?
             * If so we need a goto.
             */
            if (!term_is_cursor_valid) {
                term_is_cursor_valid = true;
                /*
                 * Terminal lines start at 1
                 */
                term_core_goto(x + 1, y + 1);
            }

            term_core_fgbg(c->fg, c->bg);

            if (!c->c) {
                term_core_putc(' ');
            } else {
                term_core_putc(c->c);
            }
        }
    }

    memcpy(term_bcells, term_cells, sizeof(term_cells));

    term_core_refresh();
#endif
}

int term_test (int32_t argc, char *argv[])
{
#ifndef _WIN32
    int x, y;

    int r = 0;
    for (;;) {
        int i;

        for (i = 0; i < 40; i++) {
            r++;

            for (x = 0; x < TERM_CORE_MAX_SIZE; x++) {
                for (y = 0; y < TERM_CORE_MAX_SIZE; y++) {
                    term_goto(x, y);

                    int d = (x + y + r);

                    term_putc('a' + (d % 26));
                    term_fg((term_color)(d % 8));
                    term_bg((term_color)((d+1) % 8));
                }
            }

            usleep(50000);

            term_refresh();
        }

        for (i = 0; i < 40; i++) {
            term_scroll();
            term_refresh();
            usleep(50000);
        }

        term_refresh();
    }

#endif
    return (0);
}

#ifndef _WIN32
static void term_print_last_line (void)
{
    int x, y;

    y = term_y;

    //
    // Hitting the max col seems to mess the terminal up
    //
    for (x = 0; x < term_x; x++) {

        term_cell *c = &term_cells[x][y];

        term_puts_fgbg(c->fg, c->bg);

        if (!c->c) {
            putchar(' ');
        } else {
            putchar(c->c);
        }
    }
}
#endif

void term_log (const char *buf)
{
#ifndef _WIN32
    if (HEADLESS || enable_console) {
        term_scroll();
        term_goto(0, TERM_HEIGHT - 2);
    }

    term_putf(buf);

    if (HEADLESS || enable_console) {
        term_refresh();
    } else {
        term_print_last_line();
        putchar('\n');
        term_x = 0;
    }
#endif
}

/*
 * inquire actual terminal size (this it what the 
 * kernel thinks - not was the user on the over end
 * of the phone line has really).
 */
int
get_term_size (int fd, int *x, int *y)
{
#ifndef _WIN32
#ifdef TIOCGSIZE
    struct ttysize win;

#elif defined(TIOCGWINSZ)
    struct winsize win;

#endif

#ifdef TIOCGSIZE
    if (!ioctl(fd, TIOCGSIZE, &win)) {
        if (y) {
            *y = win.ts_lines;
        }
        if (x) {
            *x = win.ts_cols;
        }
    }
#elif defined TIOCGWINSZ
    if (ioctl(fd, TIOCGWINSZ, &win)) {
        return (0);
    }

    if (y) {
        *y = win.ws_row;
    }

    if (x) {
        *x = win.ws_col;
    }
#else
    {
        const char *s;
        s=getenv("LINES");
        if (s) {
            if (y) {
                *y = strtol(s,NULL,10);
            }
        }
        s=getenv("COLUMNS");
        if (s) {
            if (x) {
                *x = strtol(s,NULL,10);
            }
        }
    }
#endif
#endif

    if (x) {
        if (!*x) {
            *x = 80;
        }
    }

    if (y) {
        if (!*y) {
            *y = 25;
        }
    }

    return (1);
}

