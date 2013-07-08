/*
 * This game is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This game is distributed in the hope that it will be fun,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this game; if not, write to the Free
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * goblinhack@gmail.com
 */
#include <libgen.h>
#include <unistd.h>
#include <SDL.h>
#include <time.h>

#include "main.h"
#include "ramdisk.h"

#define MAZE_NO_ROONEXT_TO_OTHER_ROOMS
#define MAZE_HOW_LONG_TO_SPEND_TRYING_TO_SOLVE_MAZE 1000
#define MAZE_HOW_LIKELY_PERCENT_ARE_FORKS           55
#undef MAZE_DEBUG_SHOW_AS_GENERATING
#undef MSZE_DEBUG_PRINT_EXITS

#define tcup(x,y)           printf("\033[%d;%dH", y + 1, x + 1);

/*
 * A single jigpiece used to build the level.
 *
 * +---------+
 * |xxxxxxxxx|
 * |x.......x|
 * |x.......x|
 * |x.......x|
 * |D.......x|
 * |x.......x|
 * |x.......x|
 * |x.......x|
 * |xxxxxxxxx|
 * +---------+
 */
#define JIGPIECE_MAX                    2000
#define JIGPIECE_WIDTH                  13
#define JIGPIECE_HEIGHT                 13

/*
 * A solvable maze, e.g.:
 * +---+---+---+
 * |   |       |
 * +   +   +---+
 * | E |       |
 * +   +   +---|
 * | * |       |
 * +---+---+---+
 */
#define MAZE_WIDTH                      9
#define MAZE_HEIGHT                     5

/*
 *
 * xxxxxxxxx         xxxxxxxxx
 * x.......x         x.......x
 * x.......x         x.......x
 * x.......x+++++++++x.......x
 * x.......D,,,,,,,,,D.......x
 * x.......x+++++++++x.......x
 * x.......x         x.......x
 * x.......x         x.......x
 * xxxxxxxxx         xxxxDxxxx
 *                      +,+
 *                      +,+
 *                      +,+
 *    +++++++++++++++++++,+++
 *    +,,,,,,,,,,,,,,,,,,,,,+
 *    +,++++++,++++++++++++,+
 *    +,+    +,+          +,+
 *    +,+    +,+          +,+
 *    +,+    +,+          +,+
 *    +,+    +,+       xxxxx+
 *    +,+    +,++++++++x...x
 *   xxDxx   +,,,,,,,,,D...x
 *   x...x   ++++++++++x...x
 *   x...x             xxxxx
 *   x...x
 *   xxxxx
 */
#define MAZE_CHAR_WIDTH             ((MAZE_WIDTH * JIGPIECE_WIDTH) + 1)
#define MAZE_CHAR_HEIGHT            ((MAZE_HEIGHT * JIGPIECE_HEIGHT) + 1)

#define MAP_TERM_BUFFER_WIDTH       (((MAZE_WIDTH) * JIGPIECE_WIDTH) + 1)
#define MAP_TERM_BUFFER_HEIGHT      (((MAZE_HEIGHT) * JIGPIECE_HEIGHT) + 1)

static char map_term_buffer[MAP_TERM_BUFFER_WIDTH][MAP_TERM_BUFFER_HEIGHT];
static uint8_t map_term_buffer_fg[MAP_TERM_BUFFER_WIDTH][MAP_TERM_BUFFER_HEIGHT];
static uint8_t map_term_buffer_bg[MAP_TERM_BUFFER_WIDTH][MAP_TERM_BUFFER_HEIGHT];
static int32_t map_term_buffer_at_x;
static int32_t map_term_buffer_at_y;

enum {
    MAP_EMPTY           = ' ',
    MAP_WATER           = 'W',
    MAP_SPACE           = 's',
    MAP_FLOOR           = '.',
    MAP_WALL            = 'x',
    MAP_CORRIDOR        = ',',
    MAP_CORRIDOR_WALL   = '+',
    MAP_CORRIDOR_POSS   = '?',
    MAP_CORRIDOR_OK     = 'o',
    MAP_CORRIDOR_DEAD   = '0',
    MAP_CORRIDOR_FORK   = '/',
    MAP_DOOR            = 'D',
    MAP_SECRET_DOOR     = 'S',
    MAP_MONST           = 'm',
    MAP_MAJ_MONST       = 'M',
    MAP_TRAP            = '!',
    MAP_TRAPDOOR        = 'T',
    MAP_TREASURE        = '$',
    MAP_FOOD            = 'f',
    MAP_EXIT_WEST       = '<',
    MAP_EXIT_EAST       = '>',
    MAP_EXIT_SOUTH      = 'v',
    MAP_EXIT_NORTH      = '^',
    MAP_PADDING         = 'Z',
    MAP_MAX             = 255,
};

enum {
    TERM_COLOR_BLACK,
    TERM_COLOR_RED,
    TERM_COLOR_GREEN,
    TERM_COLOR_YELLOW,
    TERM_COLOR_BLUE,
    TERM_COLOR_PINK,
    TERM_COLOR_CYAN,
    TERM_COLOR_WHITE,
};

uint8_t map_fg[] = {
    [MAP_EMPTY]          = TERM_COLOR_BLACK,
    [MAP_SPACE]          = TERM_COLOR_WHITE,
    [MAP_WATER]          = TERM_COLOR_BLACK,
    [MAP_FLOOR]          = TERM_COLOR_WHITE,
    [MAP_WALL]           = TERM_COLOR_WHITE,
    [MAP_CORRIDOR]       = TERM_COLOR_YELLOW,
    [MAP_CORRIDOR_WALL]  = TERM_COLOR_BLUE,
    [MAP_CORRIDOR_DEAD]  = TERM_COLOR_RED,
    [MAP_CORRIDOR_POSS]  = TERM_COLOR_CYAN,
    [MAP_CORRIDOR_OK]    = TERM_COLOR_GREEN,
    [MAP_CORRIDOR_FORK]  = TERM_COLOR_CYAN,
    [MAP_DOOR]           = TERM_COLOR_WHITE,
    [MAP_SECRET_DOOR]    = TERM_COLOR_WHITE,
    [MAP_MONST]          = TERM_COLOR_GREEN,
    [MAP_MAJ_MONST]      = TERM_COLOR_RED,
    [MAP_TRAP]           = TERM_COLOR_RED,
    [MAP_TRAPDOOR]       = TERM_COLOR_RED,
    [MAP_TREASURE]       = TERM_COLOR_YELLOW,
    [MAP_FOOD]           = TERM_COLOR_GREEN,
    [MAP_EXIT_WEST]      = TERM_COLOR_RED,
    [MAP_EXIT_EAST]      = TERM_COLOR_RED,
    [MAP_EXIT_SOUTH]     = TERM_COLOR_RED,
    [MAP_EXIT_NORTH]     = TERM_COLOR_RED,
    [MAP_PADDING]        = TERM_COLOR_WHITE,
};

uint8_t map_bg[] = {
    [MAP_EMPTY]          = TERM_COLOR_BLACK,
    [MAP_SPACE]          = TERM_COLOR_BLACK,
    [MAP_WATER]          = TERM_COLOR_CYAN,
    [MAP_FLOOR]          = TERM_COLOR_BLACK,
    [MAP_WALL]           = TERM_COLOR_BLUE,
    [MAP_CORRIDOR]       = TERM_COLOR_BLACK,
    [MAP_CORRIDOR_WALL]  = TERM_COLOR_BLACK,
    [MAP_CORRIDOR_DEAD]  = TERM_COLOR_BLACK,
    [MAP_CORRIDOR_POSS]  = TERM_COLOR_BLACK,
    [MAP_CORRIDOR_OK]    = TERM_COLOR_BLACK,
    [MAP_CORRIDOR_FORK]  = TERM_COLOR_BLACK,
    [MAP_DOOR]           = TERM_COLOR_BLACK,
    [MAP_SECRET_DOOR]    = TERM_COLOR_RED,
    [MAP_MONST]          = TERM_COLOR_BLACK,
    [MAP_MAJ_MONST]      = TERM_COLOR_BLACK,
    [MAP_TRAP]           = TERM_COLOR_BLACK,
    [MAP_TRAPDOOR]       = TERM_COLOR_BLACK,
    [MAP_TREASURE]       = TERM_COLOR_BLACK,
    [MAP_FOOD]           = TERM_COLOR_BLACK,
    [MAP_EXIT_WEST]      = TERM_COLOR_BLACK,
    [MAP_EXIT_EAST]      = TERM_COLOR_BLACK,
    [MAP_EXIT_SOUTH]     = TERM_COLOR_BLACK,
    [MAP_EXIT_NORTH]     = TERM_COLOR_BLACK,
    [MAP_PADDING]        = TERM_COLOR_BLACK,
};

enum {
    /*
     * Order is important so 3 - x == opposite dir
     */
    DIR_NORTH,
    DIR_EAST,
    DIR_WEST,
    DIR_SOUTH,
    DIR_MAX,
};

/*
 * A single jigpiece used to build the level.
 *
 * +---------+
 * |xxxxxxxxx|
 * |x.......x|
 * |x.......x|
 * |x.......x|
 * |D.......x|
 * |x.......x|
 * |x.......x|
 * |x.......x|
 * |xxxxxxxxx|
 * +---------+
 */
typedef struct {
    char c[JIGPIECE_WIDTH][JIGPIECE_HEIGHT];

    /*
     * A bitmap of exit directions.
     */
    int32_t exits[DIR_MAX];

    /*
     * What map types are in this piece?
     */
    int32_t has[MAP_MAX];
} jigpiece_t;

/*
 * A solvable maze, e.g.:
 * +---+---+---+---+---+---+---+---+---+---+---+---+
 * |   |                   |       |       |       |
 * +   +   +---+---+---+   +   +   +   +   +   +   +
 * | E |               |   |   |       |   |   |   |
 * +   +   +---+---+   +   +   +---+---+   +   +   +
 * | * |           |   |   |       |   |   |   |   |
 * +   +---+   +---+   +---+---+   +   +   +   +   +
 * | *   * |   |       |       |       |       |   |
 * +---+   +   +   +   +   +   +---+   +---+---+   +
 * |   | * |   |   |   |   |           |           |
 * +   +   +   +   +---+   +---+---+---+   +---+---+
 * | *   * |   |           |           |           |
 * +   +---+   +---+---+---+   +---+   +   +---+   +
 * | *   S |               |   |   |   |   |       |
 * +---+   +---+---+   +---+   +   +   +   +   +---+
 * |   |   |       |   |       |       |   |       |
 * +   +   +   +   +---+   +---+   +---+---+---+   +
 * |   |       |       |   |   |   |               |
 * +   +---+---+---+   +   +   +   +---+   +---+   +
 * |                       |               |       |
 * +---+---+---+---+---+---+---+---+---+---+---+---+
 */
typedef struct maze_cell_t_ {
    struct maze_cell_t_ *exit[4];
    int32_t possible_jigpieces[JIGPIECE_MAX];
    int32_t possible_jigpieces_size;
    int32_t jigpiece;
    int32_t x;
    int32_t y;
    uint8_t exits:4;
    uint8_t walked:1;
    uint8_t start:1;
    uint8_t end:1;
    uint8_t solved:1;
    uint8_t best:1;
} maze_cell_t;

#define MAZE_CELL(c, x, y)          (c + (MAZE_WIDTH * (y)) + (x))
#define MAZE_CAN_GO(c, dir)         (c->exit[dir] && !(c->exit[dir]->walked))
#define MAZE_HAS_EXIT(c, x, y, dir) (MAZE_CELL(c, (x), (y))->exits & (1<<(dir)))

#define MAZE_FRAG_DIRECTIONS 8

typedef struct {
    int32_t jigpieces_cnt;
    jigpiece_t jigpiece[JIGPIECE_MAX];

    int32_t fragments_cnt;
    jigpiece_t fragments[MAZE_FRAG_DIRECTIONS][JIGPIECE_MAX];
    int32_t fragment_to_alt_base[JIGPIECE_MAX];
    int32_t fragments_cnt_alts[JIGPIECE_MAX];

    int32_t frag_alt_cnt;
    jigpiece_t frag_alt[MAZE_FRAG_DIRECTIONS][JIGPIECE_MAX];

    maze_cell_t maze[MAZE_WIDTH * MAZE_HEIGHT];
} dungeon_t;

/*
 * Globals:
 */
static int32_t opt_seed;

static char *dieat (int32_t line, int32_t col, char *why)
{
    DIE("Died at line %u, col %i: %s", line, col, why);
}

/*
 * filetobuf
 *
 * Read a whole file into an allocated buffer.
 */
static char *filetobuf (const char *file)
{
    int32_t len;
    char *buf;

    buf = (typeof(buf)) ramdisk_load_copy(file, &len);

    return (buf);
}

/*
 * map_term_buffer_goto
 */
static void map_term_buffer_goto (int32_t x, int32_t y)
{
    map_term_buffer_at_x = x;
    map_term_buffer_at_y = y;
}

/*
 * map_term_buffer_putchar
 */
static void map_term_buffer_putchar (int32_t m)
{
    if (map_term_buffer_at_x < 0) {
        return;
    }

    if (map_term_buffer_at_y < 0) {
        return;
    }

    if (map_term_buffer_at_x >= MAP_TERM_BUFFER_WIDTH) {
        return;
    }

    if (map_term_buffer_at_y >= MAP_TERM_BUFFER_HEIGHT) {
        return;
    }

    map_term_buffer_fg[map_term_buffer_at_x][map_term_buffer_at_y] = map_fg[m];
    map_term_buffer_bg[map_term_buffer_at_x][map_term_buffer_at_y] = map_bg[m];
    map_term_buffer[map_term_buffer_at_x++][map_term_buffer_at_y] = m;
}

/*
 * map_term_buffer_getchar
 */
static uint8_t map_term_buffer_getchar (int32_t x, int32_t y)
{
    if (x < 0) {
        return (MAP_EMPTY);
    }

    if (y < 0) {
        return (MAP_EMPTY);
    }

    if (x >= MAP_TERM_BUFFER_WIDTH) {
        return (MAP_EMPTY);
    }

    if (y >= MAP_TERM_BUFFER_HEIGHT) {
        return (MAP_EMPTY);
    }

    return (map_term_buffer[x][y]);
}

/*
 * map_term_buffer_set_fgbg
 */
static void map_term_buffer_set_fgbg (uint8_t fg, uint8_t bg)
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

    printf("%s", data[(bg & 7) * 8 + (fg & 7)]);
}

/*
 * map_term_buffer_print
 */
static void map_term_buffer_print (void)
{
    int32_t need_nl;
    int32_t x;
    int32_t y;
    int32_t fg;
    int32_t bg;

    need_nl = 0;

    tcup(0,0);

    for (y = 0; y < MAP_TERM_BUFFER_HEIGHT; y++) {
        for (x = 0; x < MAP_TERM_BUFFER_WIDTH; x++) {
            char c;

            c = map_term_buffer[x][y];
            fg = map_term_buffer_fg[x][y];
            bg = map_term_buffer_bg[x][y];

            if (!c) {
                fg = TERM_COLOR_WHITE;
                bg = TERM_COLOR_BLACK;
                c = ' ';
            }

            if (x == 0) {
                if (need_nl) {
                    putchar('\n');
                }
                need_nl = 1;
            }

            map_term_buffer_set_fgbg(fg, bg);
            putchar(c);
        }
    }
}

/*
 * map_term_buffer_print_file
 */
static void map_term_buffer_print_file (void)
{
    char tmp[20];
    FILE *fp;
    int32_t need_nl;
    int32_t x;
    int32_t y;

    snprintf(tmp, sizeof(tmp) - 1, "maps.%u", opt_seed);

    fp = fopen(tmp, "w");
    if (!fp) {
        ERR("can't write map file");
        return;
    }

    need_nl = 0;

    for (y = 0; y < MAP_TERM_BUFFER_HEIGHT; y++) {
        for (x = 0; x < MAP_TERM_BUFFER_WIDTH; x++) {
            char c;

            c = map_term_buffer[x][y];

            if (!c) {
                c = ' ';
            }

            if (x == 0) {
                if (need_nl) {
                    fputc('\n', fp);
                }
                need_nl = 1;
            }

            fputc(c, fp);
        }
    }

    fputc('\n', fp);

    fclose(fp);
}

/*
 * Read jigpiece data from a buffer.
 */
static void jigpieces_read (dungeon_t *dg, char *buf)
{
    char *c = buf;
    int32_t cnt_cells_per_line;
    int32_t x;
    int32_t n;
    int32_t y;
    int32_t col;
    int32_t line;
    char command[80];
    char *s;
    int32_t reading_jigpieces;
    int32_t reading_fragments;
    int32_t reading_frag_alt;

    reading_jigpieces = 0;
    reading_fragments = 0;
    reading_frag_alt = 0;

    col = 1;
    line = 1;

    while (*c) {
        /*
         * Skip comments
         */
        if (*c == '#') {
            while (*c != '\n') {
                c++;
            }
            c++;
            col = 0;
            line++;
            continue;
        }

        /*
         * : begins commands.
         */
        if (*c == ':') {
            c++;
            s = command;

            memset(command, 0, sizeof(command));

            while ((*c != '\n') && (s < command + sizeof(command))) {
                *s++ = *c++;
            }

            reading_jigpieces = 0;
            reading_fragments = 0;
            reading_frag_alt = 0;

            if (!strcmp(command, "jigpieces")) {
                reading_jigpieces = 1;
            } else if (!strcmp(command, "fragment")) {
                reading_fragments = 1;
            } else if (!strcmp(command, "alternative")) {
                reading_frag_alt = 1;
            } else {
                dieat(line, col, "unknown command");
            }

            c++;
            col = 0;
            line++;
            continue;
        }

        /*
         * Read jigpiece header
         *
         * +--------+--------+...
         */
        if (*c != '+') {
            dieat(line, col, "expecting start of line");
        }

        if (!reading_jigpieces &&
            !reading_fragments &&
            !reading_frag_alt) {
            dieat(line, col,
                  "data found when no reading jigpieces or fragments");
        }

        c++;
        col++;

        /*
         * Count the number of jigpiece across per line.
         */
        cnt_cells_per_line = 0;

        while (*c != '\n') {
            if (*c == '+') {
                cnt_cells_per_line++;
            }
            c++;
            col++;
        }

        if (*c != '\n') {
            dieat(line, col, "expecing end of line");
        }

        /*
         * Move onto the next line.
         */
        c++;
        col = 0;
        line++;

        if (*c == EOF) {
            return;
        }

        if (dg->jigpieces_cnt + cnt_cells_per_line >= JIGPIECE_MAX) {
            dieat(line, col, "too many jigpieces");
        }

        if (!*c) {
            return;
        }

        /*
         * Read one line of data
         * |........|........|...
         */
        col = 0;
        y = 0;

        for (y = 0; y < JIGPIECE_HEIGHT; y++) {
            for (n = 0; n < cnt_cells_per_line; n++) {

                if (*c != '|') {
                    dieat(line, col, "expecting start of jigpiece data");
                }

                c++;
                col++;

                for (x = 0; x < JIGPIECE_WIDTH; x++) {

                    if (*c == '\n') {
                        dieat(line, col, "premature end of jigpiece data");
                    }

                    if (reading_jigpieces) {
                        dg->jigpiece[dg->jigpieces_cnt + n].c[x][y] = *c;
                    } else if (reading_fragments) {
                        dg->fragments[0][dg->fragments_cnt + n].c[x][y] = *c;
                    } else if (reading_frag_alt) {
                        dg->frag_alt[0][dg->frag_alt_cnt + n].c[x][y] = *c;
                    } else {
                        DIE("bug");
                    }

                    c++;
                    col++;
                }
            }

            /*
             * Should be at the end of the line
             */
            if (*c != '|') {
                dieat(line, col, "premature end of file");
            }

            c++;
            col++;

            if (*c != '\n') {
                dieat(line, col, "expecting end of line");
            }

            /*
             * Move onto the next line.
             */
            c++;
            col = 0;
            line++;
        }

        if (reading_jigpieces) {
            dg->jigpieces_cnt += cnt_cells_per_line;
        } else if (reading_fragments) {

            if (!dg->fragment_to_alt_base[dg->fragments_cnt]) {
                dg->fragment_to_alt_base[dg->fragments_cnt] =
                                dg->frag_alt_cnt;
            }

            dg->fragments_cnt += cnt_cells_per_line;
        } else if (reading_frag_alt) {
            dg->frag_alt_cnt += cnt_cells_per_line;

            dg->fragments_cnt_alts[dg->fragments_cnt-1] +=
                            cnt_cells_per_line;
        }
    }
}

/*
 * jigpiece_char_is_occupiable
 */
static int32_t jigpiece_char_is_occupiable (char c)
{
    return (c == MAP_FLOOR) ||
           (c == MAP_CORRIDOR) ||
           (c == MAP_MONST) ||
           (c == MAP_MAJ_MONST) ||
           (c == MAP_TREASURE) ||
           (c == MAP_SECRET_DOOR) ||
           (c == MAP_DOOR);
}

/*
 * jigpiece_char_at
 */
static char jigpiece_char_at (dungeon_t *dg, int32_t which, int32_t x, int32_t y)
{
    if (x < 0) {
        return (MAP_PADDING);
    }

    if (y < 0) {
        return (MAP_PADDING);
    }

    if (x >= JIGPIECE_WIDTH) {
        return (MAP_PADDING);
    }

    if (y >= JIGPIECE_HEIGHT) {
        return (MAP_PADDING);
    }

    return (dg->jigpiece[which].c[x][y]);
}

/*
 * jigpiece_create_exits
 */
static void jigpiece_create_exits (dungeon_t *dg)
{
    int32_t which;
    int32_t x;
    int32_t y;

    for (which = 0; which < dg->jigpieces_cnt; which++) {
        /*
         * ?.......
         * ?.......
         * ?.......
         * ?.......
         * ?.......
         * ?.......
         * ?.......
         * ?.......
         */
        x = 0;
        for (y = 0; y < JIGPIECE_HEIGHT; y++) {
            if (jigpiece_char_is_occupiable(
                                        jigpiece_char_at(dg, which, x, y))) {
                dg->jigpiece[which].exits[DIR_WEST] |= 1<<y;
            }
        }

        /*
         * .......?
         * .......?
         * .......?
         * .......?
         * .......?
         * .......?
         * .......?
         * .......?
         */
        x = JIGPIECE_WIDTH-1;
        for (y = 0; y < JIGPIECE_HEIGHT; y++) {
            if (jigpiece_char_is_occupiable(
                                        jigpiece_char_at(dg, which, x, y))) {
                dg->jigpiece[which].exits[DIR_EAST] |= 1<<y;
            }
        }

        /*
         * ????????
         * ........
         * ........
         * ........
         * ........
         * ........
         * ........
         * ........
         */
        y = 0;
        for (x = 0; x < JIGPIECE_WIDTH; x++) {
            if (jigpiece_char_is_occupiable(
                                        jigpiece_char_at(dg, which, x, y))) {
                dg->jigpiece[which].exits[DIR_NORTH] |= 1<<x;
            }
        }

        /*
         * ........
         * ........
         * ........
         * ........
         * ........
         * ........
         * ........
         * ????????
         */
        y = JIGPIECE_HEIGHT-1;
        for (x = 0; x < JIGPIECE_WIDTH; x++) {
            if (jigpiece_char_is_occupiable(
                                        jigpiece_char_at(dg, which, x, y))) {
                dg->jigpiece[which].exits[DIR_SOUTH] |= 1<<x;
            }
        }
    }
}

/*
 * jigpiece_print
 */
static inline void jigpiece_print (dungeon_t *dg, int32_t which)
{
    int32_t x;
    int32_t y;

    for (y = 0; y < JIGPIECE_HEIGHT; y++) {
        for (x = 0; x < JIGPIECE_WIDTH; x++) {

#ifdef MSZE_DEBUG_PRINT_EXITS
            if (which) {
                if (x == 0) {
                    if (dg->jigpiece[which].exits[DIR_WEST] & (1 << y)) {
                        putchar(MAP_EXIT_WEST);
                        continue;
                    }
                } else if (x == JIGPIECE_WIDTH-1) {
                    if (dg->jigpiece[which].exits[DIR_EAST] & (1 << y)) {
                        putchar(MAP_EXIT_EAST);
                        continue;
                    }
                }

                if (y == 0) {
                    if (dg->jigpiece[which].exits[DIR_NORTH] & (1 << x)) {
                        putchar(MAP_EXIT_NORTH);
                        continue;
                    }
                } else if (y == JIGPIECE_HEIGHT-1) {
                    if (dg->jigpiece[which].exits[DIR_SOUTH] & (1 << x)) {
                        putchar(MAP_EXIT_SOUTH);
                        continue;
                    }
                }
            }
#endif
            putchar(dg->jigpiece[which].c[x][y]);
        }
        putchar('\n');
    }
    putchar('\n');
}

/*
 * fragment_print
 */
static inline void fragment_print (dungeon_t *dg, int32_t dir, int32_t which)
{
    int32_t x;
    int32_t y;

    for (y = 0; y < JIGPIECE_HEIGHT; y++) {
        for (x = 0; x < JIGPIECE_WIDTH; x++) {
            putchar(dg->fragments[dir][which].c[x][y]);
        }
        putchar('\n');
    }
    putchar('\n');
}

/*
 * jigpiece_printat
 */
static void jigpiece_printat (dungeon_t *dg, 
                              int32_t atx, int32_t aty, int32_t which)
{
    int32_t x;
    int32_t y;

    for (y = 0; y < JIGPIECE_HEIGHT; y++) {
        map_term_buffer_goto(atx, aty + y);

        for (x = 0; x < JIGPIECE_WIDTH; x++) {

#ifdef MSZE_DEBUG_PRINT_EXITS
            if (which) {
                if (x == 0) {
                    if (dg->jigpiece[which].exits[DIR_WEST] & (1 << y)) {
                        map_term_buffer_putchar(MAP_EXIT_WEST);
                        continue;
                    }
                } else if (x == JIGPIECE_WIDTH-1) {
                    if (dg->jigpiece[which].exits[DIR_EAST] & (1 << y)) {
                        map_term_buffer_putchar(MAP_EXIT_EAST);
                        continue;
                    }
                }

                if (y == 0) {
                    if (dg->jigpiece[which].exits[DIR_NORTH] & (1 << x)) {
                        map_term_buffer_putchar(MAP_EXIT_NORTH);
                        continue;
                    }
                } else if (y == JIGPIECE_HEIGHT-1) {
                    if (dg->jigpiece[which].exits[DIR_SOUTH] & (1 << x)) {
                        map_term_buffer_putchar(MAP_EXIT_SOUTH);
                        continue;
                    }
                }
            }
#endif

            map_term_buffer_putchar(dg->jigpiece[which].c[x][y]);
        }
    }
}

/*
 * jigpiece_printat_with_border
 */
static inline void jigpiece_printat_with_border (dungeon_t *dg, int32_t atx, int32_t aty, int32_t which)
{
    int32_t x;
    int32_t y;

    map_term_buffer_goto(atx, aty);

    for (x = 0; x < JIGPIECE_WIDTH + 2; x++) {
        map_term_buffer_putchar(MAP_PADDING);
    }

    map_term_buffer_goto(atx, aty+1);

    for (y = 0; y < JIGPIECE_HEIGHT; y++) {
        map_term_buffer_goto(atx, aty + y + 1);
        map_term_buffer_putchar(MAP_PADDING);

        for (x = 0; x < JIGPIECE_WIDTH; x++) {

            if (which == 0) {
                map_term_buffer_putchar(MAP_PADDING);
                continue;
            }

#ifdef MSZE_DEBUG_PRINT_EXITS
            if (x == 0) {
                if (dg->jigpiece[which].exits[DIR_WEST] & (1 << y)) {
                    map_term_buffer_putchar(MAP_EXIT_WEST);
                    continue;
                }
            } else if (x == JIGPIECE_WIDTH-1) {
                if (dg->jigpiece[which].exits[DIR_EAST] & (1 << y)) {
                    map_term_buffer_putchar(MAP_EXIT_EAST);
                    continue;
                }
            }

            if (y == 0) {
                if (dg->jigpiece[which].exits[DIR_NORTH] & (1 << x)) {
                    map_term_buffer_putchar(MAP_EXIT_NORTH);
                    continue;
                }
            } else if (y == JIGPIECE_HEIGHT-1) {
                if (dg->jigpiece[which].exits[DIR_SOUTH] & (1 << x)) {
                    map_term_buffer_putchar(MAP_EXIT_SOUTH);
                    continue;
                }
            }
#endif
            map_term_buffer_putchar(dg->jigpiece[which].c[x][y]);
        }

        map_term_buffer_putchar(MAP_PADDING);
    }

    map_term_buffer_goto(atx, aty + y + 1);

    for (x = 0; x < JIGPIECE_WIDTH + 2; x++) {
        map_term_buffer_putchar(MAP_PADDING);
    }
}

/*
 * jigpiece_count_char_types
 *
 * How many of whatever type of map char are in each which
 */
static void jigpiece_count_char_types (dungeon_t *dg, int32_t which)
{
    int32_t x;
    int32_t y;

    for (y = 0; y < JIGPIECE_HEIGHT; y++) {

        for (x = 0; x < JIGPIECE_WIDTH; x++) {

            if (which == 0) {
                continue;
            }

            if (x == 0) {
                if (dg->jigpiece[which].exits[DIR_WEST] & (1 << y)) {
                    dg->jigpiece[which].has[MAP_EXIT_WEST]++;
                    continue;
                }
            } else if (x == JIGPIECE_WIDTH-1) {
                if (dg->jigpiece[which].exits[DIR_EAST] & (1 << y)) {
                    dg->jigpiece[which].has[MAP_EXIT_EAST]++;
                    continue;
                }
            }

            if (y == 0) {
                if (dg->jigpiece[which].exits[DIR_NORTH] & (1 << x)) {
                    dg->jigpiece[which].has[MAP_EXIT_NORTH]++;
                    continue;
                }
            } else if (y == JIGPIECE_HEIGHT-1) {
                if (dg->jigpiece[which].exits[DIR_SOUTH] & (1 << x)) {
                    dg->jigpiece[which].has[MAP_EXIT_SOUTH]++;
                    continue;
                }
            }

            dg->jigpiece[which].has[(int32_t)dg->jigpiece[which].c[x][y]]++;
        }
    }
}

/*
 * maze_print_cells
 */
static void maze_print_cells (dungeon_t *dg)
{
    int32_t w = MAZE_WIDTH;
    int32_t h = MAZE_HEIGHT;
    char which;
    int32_t y;
    int32_t x;

    for (x = 0; x < w; x++) {
        printf("+---");
    }

    printf("+\n");

    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            maze_cell_t *c = MAZE_CELL(dg->maze, x, y);

            which = ' ';

            if (c->start) {
                which = 'S';
            } else if (c->end) {
                which = 'E';
            } else if (c->best) {
                which = '*';
            } else if (c->solved) {
                which = ' ';
            }

            if ((x > 0) && MAZE_HAS_EXIT(dg->maze, x, y, DIR_WEST)) {
                printf("  %c ", which);
            } else {
                printf("| %c ", which);
            }
        }

        printf("|\n");

        for (x = 0; x < w; x++) {
            if ((y < h) && MAZE_HAS_EXIT(dg->maze, x, y, DIR_SOUTH)) {
                printf("+   ");
            } else {
                printf("+---");
            }
        }

        printf("+\n");
    }
}

/*
 * jigpiece_add_fragments
 *
 * Replace fragments of the maze to make it more interesting.
 */
static void jigpiece_add_fragments (dungeon_t *dg)
{
    int32_t f;
    int32_t i;
    int32_t c;
    int32_t x;
    int32_t y;
    int32_t ax;
    int32_t ay;
    int32_t cx;
    int32_t cy;
    int32_t tries;
    int32_t dir;
    int32_t frag;
    int32_t alt;
    int32_t map;

    /*
     * For each fragment.
     */
    for (f = 0; f < dg->fragments_cnt; f++) {
        /*
         * For each orientation of a fragment.
         */
        for (dir = 0; dir < MAZE_FRAG_DIRECTIONS; dir++) {
            /*
             * For each orientation of a fragment.
             */
            for (c = 0; c < dg->fragments_cnt_alts[f]; c++) {
#ifdef MAZE_DEBUG_SHOW_AS_GENERATING
                maze_print_cells(dg);
                map_term_buffer_print();
#endif
                /*
                 * Try and place the fragment.
                 */
                tries = 0;
                while (tries++ < MAZE_CHAR_WIDTH * MAZE_CHAR_HEIGHT) {

                    ax = rand() % (MAZE_CHAR_WIDTH + JIGPIECE_WIDTH);
                    ay = rand() % (MAZE_CHAR_HEIGHT + JIGPIECE_HEIGHT);
                    ax -= JIGPIECE_WIDTH;
                    ay -= JIGPIECE_HEIGHT;

                    for (x = 0; x < JIGPIECE_WIDTH; x++) {
                        for (y = 0; y < JIGPIECE_HEIGHT; y++) {
                            /*
                             * Skip empty spaces.
                             */
                            frag = dg->fragments[dir][f].c[x][y];
                            if (frag == MAP_EMPTY) {
                                continue;
                            }

                            cx = ax + x;
                            cy = ay + y;

                            /*
                             * It's ok to be off map but only if a space.
                             */
                            if ((cx < 0) || (cx >= MAZE_CHAR_WIDTH) ||
                                (cy < 0) || (cy >= MAZE_CHAR_HEIGHT)) {
                                goto next;
                            }

                            /*
                             * Check the fragment is an exact match.
                             */
                            map = map_term_buffer_getchar(cx, cy);
                            if (map != frag) {
                                /*
                                 * Allow the fragment to force that spaces
                                 * must exist.
                                 */
                                if (frag == MAP_SPACE) {
                                    if (map == MAP_EMPTY) {
                                        continue;
                                    }
                                }
                                goto next;
                            }
                        }
                    }

                    /*
                     * Choose something to replace the fragment.
                     */
                    i = rand() % dg->fragments_cnt_alts[f];
                    i += dg->fragment_to_alt_base[f];

                    /*
                     * Place the fragment.
                     */
                    for (x = 0; x < JIGPIECE_WIDTH; x++) {
                        for (y = 0; y < JIGPIECE_HEIGHT; y++) {
                            alt = dg->frag_alt[dir][i].c[x][y];
                            frag = dg->fragments[dir][f].c[x][y];

                            cx = ax + x;
                            cy = ay + y;

                            /*
                             * Skip off map.
                             */
                            if ((cx < 0) || (cx >= MAZE_CHAR_WIDTH) ||
                                (cy < 0) || (cy >= MAZE_CHAR_HEIGHT)) {
                                continue;
                            }

                            /*
                             * If the alternative has space and the fragment
                             * does not, overwrite it.
                             */
                            if (alt == MAP_EMPTY) {
                                if (frag == MAP_EMPTY) {
                                    continue;
                                }
                            }

                            map_term_buffer_goto(cx, cy);
                            map_term_buffer_putchar(alt);
                        }
                    }
                    break;
next:
                    continue;
                }
            }
        }
    }
}

/*
 * jigpiece_create_room_door_varieties
 *
 * Make random copies of room jigpieces and replace the doors with floors
 */
static void jigpiece_create_room_door_varieties (dungeon_t *dg)
{
    int32_t c;
    int32_t x;
    int32_t y;
    int32_t r;
    int32_t which;
    int32_t origs_cnt;

    origs_cnt = dg->jigpieces_cnt;

    for (c = 0; c < origs_cnt; c++) {

        if (!dg->jigpiece[c].has[MAP_DOOR]) {
            continue;
        }

        for (r = 0; r < 40; r++) {
            for (x = 0; x < JIGPIECE_WIDTH; x++) {
                for (y = 0; y < JIGPIECE_HEIGHT; y++) {

                    dg->jigpiece[dg->jigpieces_cnt].c[x][y] =
                                    dg->jigpiece[c].c[x][y];

                    if (dg->jigpiece[dg->jigpieces_cnt].c[x][y] == MAP_DOOR) {

                        which = (rand() % 100);
                        if (which < 10) {
                            dg->jigpiece[dg->jigpieces_cnt].c[x][y] = MAP_DOOR;
                        } else {
                            dg->jigpiece[dg->jigpieces_cnt].c[x][y] = MAP_WALL;
                        }
                    }
                }
            }

            jigpiece_count_char_types(dg, dg->jigpieces_cnt);
            dg->jigpieces_cnt++;

            if (dg->jigpieces_cnt >= JIGPIECE_MAX) {
                DIE("Too many jigpiece to mirror for doors");
            }
        }
    }
}

/*
 * jigpiece_create_room_door_corridors
 *
 * Make random copies of room jigpieces and replace the doors with floors
 */
static void jigpiece_create_room_door_corridors (dungeon_t *dg)
{
    int32_t c;
    int32_t x;
    int32_t y;
    int32_t dx;
    int32_t dy;

    for (c = 0; c < dg->jigpieces_cnt; c++) {

        if (!dg->jigpiece[c].has[MAP_DOOR]) {
            continue;
        }

        for (x = 0; x < JIGPIECE_WIDTH; x++) {
            for (y = 0; y < JIGPIECE_HEIGHT; y++) {

                if (dg->jigpiece[c].c[x][y] != MAP_DOOR) {
                    continue;
                }

                dy = y;
                for (dx = x+1; dx < JIGPIECE_WIDTH; dx++) {
                    if (dg->jigpiece[c].c[dx][dy] != MAP_EMPTY) {
                        break;
                    }
                    dg->jigpiece[c].c[dx][dy] = MAP_CORRIDOR;
                }

                for (dx = x-1; dx >= 0; dx--) {
                    if (dg->jigpiece[c].c[dx][dy] != MAP_EMPTY) {
                        break;
                    }
                    dg->jigpiece[c].c[dx][dy] = MAP_CORRIDOR;
                }

                dx = x;
                for (dy = y+1; dy < JIGPIECE_HEIGHT; dy++) {
                    if (dg->jigpiece[c].c[dx][dy] != MAP_EMPTY) {
                        break;
                    }
                    dg->jigpiece[c].c[dx][dy] = MAP_CORRIDOR;
                }

                for (dy = y-1; dy >= 0; dy--) {
                    if (dg->jigpiece[c].c[dx][dy] != MAP_EMPTY) {
                        break;
                    }
                    dg->jigpiece[c].c[dx][dy] = MAP_CORRIDOR;
                }
            }
        }
    }
}

/*
 * jigpiece_create_mirrored_pieces
 */
static void jigpiece_create_mirrored_pieces (dungeon_t *dg)
{
    int32_t c;
    int32_t x;
    int32_t y;
    int32_t r;
    int32_t origs_cnt;
    int32_t prev;

    origs_cnt = dg->jigpieces_cnt;

    for (c = 0; c < origs_cnt; c++) {

        /*
         * How many doors etc... are in each jigpiece? We use this to avoid
         * flipping some jigpiece types.
         */
        jigpiece_count_char_types(dg, c);

        /*
         * Generate 3 rotations
         */
        prev = c;
        for (r=0; r<3; r++) {
            /*
             * Rotate 90 degrees
             */
            if (dg->jigpieces_cnt >= JIGPIECE_MAX) {
                DIE("Too many jigpiece to mirror");
            }

            for (x = 0; x < JIGPIECE_WIDTH; x++) {
                for (y = 0; y < JIGPIECE_HEIGHT; y++) {

                    dg->jigpiece[dg->jigpieces_cnt].c[JIGPIECE_WIDTH - 1 - y][x] =
                            dg->jigpiece[prev].c[x][y];
                }
            }

            jigpiece_count_char_types(dg, dg->jigpieces_cnt);
            prev = dg->jigpieces_cnt++;
        }

        /*
         * Mirror horizontally
         */
        if (dg->jigpieces_cnt >= JIGPIECE_MAX) {
            DIE("Too many jigpiece to mirror");
        }

        for (x = 0; x < JIGPIECE_WIDTH; x++) {
            for (y = 0; y < JIGPIECE_HEIGHT; y++) {

                dg->jigpiece[dg->jigpieces_cnt].c[JIGPIECE_WIDTH - 1 - x][y] =
                        dg->jigpiece[c].c[x][y];
            }
        }

        jigpiece_count_char_types(dg, dg->jigpieces_cnt);
        dg->jigpieces_cnt++;

        /*
         * Mirror vertically
         */
        if (dg->jigpieces_cnt >= JIGPIECE_MAX) {
            DIE("Too many jigpiece to mirror");
        }

        for (x = 0; x < JIGPIECE_WIDTH; x++) {
            for (y = 0; y < JIGPIECE_HEIGHT; y++) {

                dg->jigpiece[dg->jigpieces_cnt].c[x][JIGPIECE_HEIGHT - 1 - y] =
                        dg->jigpiece[c].c[x][y];
            }
        }

        jigpiece_count_char_types(dg, dg->jigpieces_cnt);
        dg->jigpieces_cnt++;

        /*
         * Mirror horizontally and vertically
         */
        if (dg->jigpieces_cnt >= JIGPIECE_MAX - 1) {
            DIE("Too many jigpiece to mirror");
        }

        for (x = 0; x < JIGPIECE_WIDTH; x++) {
            for (y = 0; y < JIGPIECE_HEIGHT; y++) {

                /*
                 * Bit of a hack, use the next jigpiece as scratch space else
                 * we end up overwriting what we are mirroring vertically
                 * below.
                 */
                dg->jigpiece[dg->jigpieces_cnt+1].c[JIGPIECE_WIDTH - 1 - x][y] =
                        dg->jigpiece[c].c[x][y];
            }
        }

        for (x = 0; x < JIGPIECE_WIDTH; x++) {
            for (y = 0; y < JIGPIECE_HEIGHT; y++) {

                dg->jigpiece[dg->jigpieces_cnt].c[x][JIGPIECE_HEIGHT - 1 - y] =
                        dg->jigpiece[dg->jigpieces_cnt+1].c[x][y];
            }
        }

        jigpiece_count_char_types(dg, dg->jigpieces_cnt);
        dg->jigpieces_cnt++;
    }
}

/*
 * jigpiece_create_mirrored_fragments
 */
static void jigpiece_create_mirrored_fragments (dungeon_t *dg)
{
    int32_t c;
    int32_t x;
    int32_t y;
    int32_t dir;
    int32_t prev;

    for (c = 0; c < dg->fragments_cnt; c++) {
        /*
         * Generate 3 rotations
         */
        prev = c;
        for (dir=1; dir<4; dir++) {
            /*
             * Rotate 90 degrees
             */
            for (x = 0; x < JIGPIECE_WIDTH; x++) {
                for (y = 0; y < JIGPIECE_HEIGHT; y++) {

                    dg->fragments[dir][c].c[JIGPIECE_WIDTH - 1 - y][x] =
                                    dg->fragments[dir-1][c].c[x][y];
                }
            }
        }

        /*
         * Mirror horizontally
         */
        for (x = 0; x < JIGPIECE_WIDTH; x++) {
            for (y = 0; y < JIGPIECE_HEIGHT; y++) {

                dg->fragments[dir][c].c[JIGPIECE_WIDTH - 1 - x][y] =
                                dg->fragments[0][c].c[x][y];
            }
        }

        /*
         * Mirror vertically
         */
        dir++;

        for (x = 0; x < JIGPIECE_WIDTH; x++) {
            for (y = 0; y < JIGPIECE_HEIGHT; y++) {

                dg->fragments[dir][c].c[x][JIGPIECE_HEIGHT - 1 - y] =
                            dg->fragments[0][c].c[x][y];
            }
        }

        /*
         * Mirror horizontally and vertically
         */
        dir++;

        for (x = 0; x < JIGPIECE_WIDTH; x++) {
            for (y = 0; y < JIGPIECE_HEIGHT; y++) {

                /*
                 * Bit of a hack, use the next fragment as scratch space else
                 * we end up overwriting what we are mirroring vertically
                 * below.
                 */
                dg->fragments[dir][c+1].c[JIGPIECE_WIDTH - 1 - x][y] =
                                    dg->fragments[0][c].c[x][y];
            }
        }

        for (x = 0; x < JIGPIECE_WIDTH; x++) {
            for (y = 0; y < JIGPIECE_HEIGHT; y++) {

                dg->fragments[dir][c].c[x][JIGPIECE_HEIGHT - 1 - y] =
                                    dg->fragments[dir][c+1].c[x][y];
            }
        }
    }
}

/*
 * jigpiece_create_mirrored_frag_alt
 */
static void jigpiece_create_mirrored_frag_alt (dungeon_t *dg)
{
    int32_t c;
    int32_t x;
    int32_t y;
    int32_t dir;
    int32_t prev;

    for (c = 0; c < dg->frag_alt_cnt; c++) {
        /*
         * Generate 3 rotations
         */
        prev = c;
        for (dir=1; dir<4; dir++) {
            /*
             * Rotate 90 degrees
             */
            for (x = 0; x < JIGPIECE_WIDTH; x++) {
                for (y = 0; y < JIGPIECE_HEIGHT; y++) {

                    dg->frag_alt[dir][c].c[JIGPIECE_WIDTH - 1 - y][x] =
                                    dg->frag_alt[dir-1][c].c[x][y];
                }
            }
        }

        /*
         * Mirror horizontally
         */
        for (x = 0; x < JIGPIECE_WIDTH; x++) {
            for (y = 0; y < JIGPIECE_HEIGHT; y++) {

                dg->frag_alt[dir][c].c[JIGPIECE_WIDTH - 1 - x][y] =
                                dg->frag_alt[0][c].c[x][y];
            }
        }

        /*
         * Mirror vertically
         */
        dir++;

        for (x = 0; x < JIGPIECE_WIDTH; x++) {
            for (y = 0; y < JIGPIECE_HEIGHT; y++) {

                dg->frag_alt[dir][c].c[x][JIGPIECE_HEIGHT - 1 - y] =
                            dg->frag_alt[0][c].c[x][y];
            }
        }

        /*
         * Mirror horizontally and vertically
         */
        dir++;

        for (x = 0; x < JIGPIECE_WIDTH; x++) {
            for (y = 0; y < JIGPIECE_HEIGHT; y++) {

                /*
                 * Bit of a hack, use the next fragment as scratch space else
                 * we end up overwriting what we are mirroring vertically
                 * below.
                 */
                dg->frag_alt[dir][c+1].c[JIGPIECE_WIDTH - 1 - x][y] =
                                    dg->frag_alt[0][c].c[x][y];
            }
        }

        for (x = 0; x < JIGPIECE_WIDTH; x++) {
            for (y = 0; y < JIGPIECE_HEIGHT; y++) {

                dg->frag_alt[dir][c].c[x][JIGPIECE_HEIGHT - 1 - y] =
                                    dg->frag_alt[dir][c+1].c[x][y];
            }
        }
    }
}

/*
 * jigpiece_intersect_score
 */
static int32_t jigpiece_intersect_score (dungeon_t *dg, int32_t a, int32_t dir, int32_t b)
{
    return (
        (dg->jigpiece[a].exits[dir] == dg->jigpiece[b].exits[3 - dir]) ?
            1 : 0);
}

/*
 * maze_generate_all_random_directions
 */
static void maze_generate_all_random_directions (dungeon_t *dg, maze_cell_t * c)
{
    int32_t dir;
    int32_t any_dir;
    int32_t new_dir;

    c->walked = 1;

    while (1) {
        /*
         * Check we have somewhere we can go to.
         */
        for (dir = 0, any_dir = 0; dir < 4; dir++) {
            any_dir += MAZE_CAN_GO(c, dir);
        }

        /*
         * At the end of the maze?
         */
        if (!any_dir) {
            return;
        }

        /*
         * Choose a random dir to walk.
         */
        do {
            new_dir = rand() % 4;
        } while (!MAZE_CAN_GO(c, new_dir));

        c->exits |= (1 << new_dir);
        c->exit[new_dir]->exits |= (1 << (3 - new_dir));

        if ((rand() % 100) < MAZE_HOW_LIKELY_PERCENT_ARE_FORKS) {
            do {
                new_dir = rand() % 4;
            } while (!MAZE_CAN_GO(c, new_dir));

            c->exits |= (1 << new_dir);
            c->exit[new_dir]->exits |= (1 << (3 - new_dir));
        }

        maze_generate_all_random_directions(dg, c->exit[new_dir]);
    }
}

static void maze_corridor_change_poss_to_dead (void)
{
    int32_t x;
    int32_t y;

    for (x = 1; x < MAZE_CHAR_WIDTH - 1; x++) {
        for (y = 1; y < MAZE_CHAR_HEIGHT - 1; y++) {
            if (map_term_buffer_getchar(x, y) == MAP_CORRIDOR_POSS) {
                map_term_buffer_goto(x, y);
                map_term_buffer_putchar(MAP_CORRIDOR_DEAD);
            }
        }
    }
}

static void maze_corridor_change_poss_to_ok (void)
{
    int32_t x;
    int32_t y;

    for (x = 1; x < MAZE_CHAR_WIDTH - 1; x++) {
        for (y = 1; y < MAZE_CHAR_HEIGHT - 1; y++) {
            if (map_term_buffer_getchar(x, y) == MAP_CORRIDOR_POSS) {
                map_term_buffer_goto(x, y);
                map_term_buffer_putchar(MAP_CORRIDOR_OK);
            }
        }
    }
}

static void maze_corridor_change_ok_to_corridor (void)
{
    int32_t x;
    int32_t y;

    for (x = 1; x < MAZE_CHAR_WIDTH - 1; x++) {
        for (y = 1; y < MAZE_CHAR_HEIGHT - 1; y++) {
            if (map_term_buffer_getchar(x, y) == MAP_CORRIDOR) {
                map_term_buffer_goto(x, y);
                map_term_buffer_putchar(MAP_EMPTY);
            }
        }
    }

    for (x = 1; x < MAZE_CHAR_WIDTH - 1; x++) {
        for (y = 1; y < MAZE_CHAR_HEIGHT - 1; y++) {
            if (map_term_buffer_getchar(x, y) == MAP_CORRIDOR_OK) {
                map_term_buffer_goto(x, y);
                map_term_buffer_putchar(MAP_CORRIDOR);
            }
            if (map_term_buffer_getchar(x, y) == MAP_CORRIDOR_FORK) {
                map_term_buffer_goto(x, y);
                map_term_buffer_putchar(MAP_CORRIDOR);
            }
            if (map_term_buffer_getchar(x, y) == MAP_CORRIDOR_DEAD) {
                map_term_buffer_goto(x, y);
                map_term_buffer_putchar(MAP_EMPTY);
            }
        }
    }
}

/*
 * Flood fill from a corridor junction
 */
static void maze_corridor_flood_fill (int32_t x, int32_t y,
                                      int32_t sx, int32_t sy,
                                      int32_t depth)
{
    int32_t dx;
    int32_t dy;
    uint8_t end;
    uint8_t forks;

    if (depth++) {
        if ((map_term_buffer_getchar(x, y) == MAP_DOOR) ||
            (map_term_buffer_getchar(x, y) == MAP_CORRIDOR_OK) ||
            (map_term_buffer_getchar(x, y) == MAP_CORRIDOR_FORK)) {
            maze_corridor_change_poss_to_ok();
            return;
        }

        if (map_term_buffer_getchar(x, y) == MAP_CORRIDOR_POSS) {
        } else if (map_term_buffer_getchar(x, y) != MAP_CORRIDOR) {
            maze_corridor_change_poss_to_dead();
            return;
        }
    }

    end = 0;
    forks = 0;
    for (dx = -1; dx <=1; dx++) {
        for (dy = -1; dy <=1; dy++) {

            /*
             * Only treat adjoining if at right angles.
             */
            if ((dx != 0) && (dy != 0)) {
                continue;
            }

            if (!dx && !dy) {
                continue;
            }

            if ((x + dx == sx) && (y + dy == sy)) {
                continue;
            }

            if (map_term_buffer_getchar(x+dx, y+dy) == MAP_CORRIDOR) {
                forks++;
            }

            if ((map_term_buffer_getchar(x+dx, y+dy) == MAP_DOOR) ||
                (map_term_buffer_getchar(x+dx, y+dy) == MAP_CORRIDOR_OK) ||
                (map_term_buffer_getchar(x+dx, y+dy) == MAP_CORRIDOR_FORK)) {
                end++;
            }
        }
    }

    if (!forks) {
        if (end) {
            maze_corridor_change_poss_to_ok();
            return;
        }
        maze_corridor_change_poss_to_dead();
        return;
    }

    if (forks > 1) {
        map_term_buffer_goto(x, y);
        map_term_buffer_putchar(MAP_CORRIDOR_FORK);
        maze_corridor_change_poss_to_ok();
    }

    for (dx = -1; dx <=1; dx++) {
        for (dy = -1; dy <=1; dy++) {

            /*
             * Only treat adjoining if at right angles.
             */
            if ((dx != 0) && (dy != 0)) {
                continue;
            }

            if (!dx && !dy) {
                continue;
            }

            if (map_term_buffer_getchar(x+dx, y+dy) == MAP_CORRIDOR) {
                map_term_buffer_goto(x+dx, y+dy);
                map_term_buffer_putchar(MAP_CORRIDOR_POSS);

                if (forks > 1) {
                    maze_corridor_flood_fill(x+dx, y+dy, x+dx, y+dy, 0);
                } else {
                    maze_corridor_flood_fill(x+dx, y+dy, sx, sy, depth);
                }
            }
        }
    }
}

/*
 * Make sure all corridors lead somewhere useful. This catches the harder
 * deadend cases.
 */
static void maze_verify_corridor_endpoints (void)
{
    int32_t x;
    int32_t y;

    for (x = 1; x < MAZE_CHAR_WIDTH - 1; x++) {
        for (y = 1; y < MAZE_CHAR_HEIGHT - 1; y++) {

            if (map_term_buffer_getchar(x, y) != MAP_DOOR) {
                continue;
            }

            maze_corridor_flood_fill(x, y, x, y, 0);
        }
    }

    maze_corridor_change_ok_to_corridor();
}

/*
 * Trim all doors that go nowhere
 */
static void maze_remove_deadend_doors (void)
{
    int32_t got;
    int32_t x;
    int32_t y;

    do {
        got = 0;

        for (x = 1; x < MAZE_CHAR_WIDTH - 1; x++) {
            for (y = 1; y < MAZE_CHAR_HEIGHT - 1; y++) {
                /* ...
                 * ..#
                 * ...
                 */
                if ((map_term_buffer_getchar(x, y) == MAP_DOOR) &&
                    (map_term_buffer_getchar(x-1, y) == MAP_EMPTY)) {
                    map_term_buffer_goto(x, y);
                    map_term_buffer_putchar(MAP_WALL);
                    got++;
                }

                /* ...
                 * #..
                 * ...
                 */
                if ((map_term_buffer_getchar(x, y) == MAP_DOOR) &&
                    (map_term_buffer_getchar(x+1, y) == MAP_EMPTY)) {
                    map_term_buffer_goto(x, y);
                    map_term_buffer_putchar(MAP_WALL);
                    got++;
                }

                /* .#.
                 * ...
                 * ...
                 */
                if ((map_term_buffer_getchar(x, y) == MAP_DOOR) &&
                    (map_term_buffer_getchar(x, y-1) == MAP_EMPTY)) {
                    map_term_buffer_goto(x, y);
                    map_term_buffer_putchar(MAP_WALL);
                    got++;
                }

                /* ...
                 * ...
                 * .#.
                 */
                if ((map_term_buffer_getchar(x, y) == MAP_DOOR) &&
                    (map_term_buffer_getchar(x, y+1) == MAP_EMPTY)) {
                    map_term_buffer_goto(x, y);
                    map_term_buffer_putchar(MAP_WALL);
                    got++;
                }
            }
        }
    } while (got);
}

/*
 * Add doors where corridors adjoin walls
 */
static void maze_add_secret_doors (void)
{
    int32_t got;
    int32_t x;
    int32_t y;

    do {
        got = 0;

        for (x = 1; x < MAZE_CHAR_WIDTH - 1; x++) {
            for (y = 1; y < MAZE_CHAR_HEIGHT - 1; y++) {
                /* ...
                 * ..#
                 * ...
                 */
                if ((map_term_buffer_getchar(x, y) == MAP_WALL) &&
                    (map_term_buffer_getchar(x, y+1) == MAP_WALL) &&
                    (map_term_buffer_getchar(x, y-1) == MAP_WALL) &&
                    (map_term_buffer_getchar(x-1, y) == MAP_CORRIDOR) &&
                    (map_term_buffer_getchar(x+1, y) == MAP_FLOOR)) {
                    map_term_buffer_goto(x, y);
                    map_term_buffer_putchar(MAP_SECRET_DOOR);
                    got++;
                }

                /* ...
                 * #..
                 * ...
                 */
                if ((map_term_buffer_getchar(x, y) == MAP_WALL) &&
                    (map_term_buffer_getchar(x, y+1) == MAP_WALL) &&
                    (map_term_buffer_getchar(x, y-1) == MAP_WALL) &&
                    (map_term_buffer_getchar(x+1, y) == MAP_CORRIDOR) &&
                    (map_term_buffer_getchar(x-1, y) == MAP_FLOOR)) {
                    map_term_buffer_goto(x, y);
                    map_term_buffer_putchar(MAP_SECRET_DOOR);
                    got++;
                }

                /* .#.
                 * ...
                 * ...
                 */
                if ((map_term_buffer_getchar(x, y) == MAP_WALL) &&
                    (map_term_buffer_getchar(x-1, y) == MAP_WALL) &&
                    (map_term_buffer_getchar(x+1, y) == MAP_WALL) &&
                    (map_term_buffer_getchar(x, y-1) == MAP_CORRIDOR) &&
                    (map_term_buffer_getchar(x, y+1) == MAP_FLOOR)) {
                    map_term_buffer_goto(x, y);
                    map_term_buffer_putchar(MAP_SECRET_DOOR);
                    got++;
                }

                /* ...
                 * ...
                 * .#.
                 */
                if ((map_term_buffer_getchar(x, y) == MAP_WALL) &&
                    (map_term_buffer_getchar(x-1, y) == MAP_WALL) &&
                    (map_term_buffer_getchar(x+1, y) == MAP_WALL) &&
                    (map_term_buffer_getchar(x, y+1) == MAP_CORRIDOR) &&
                    (map_term_buffer_getchar(x, y-1) == MAP_FLOOR)) {
                    map_term_buffer_goto(x, y);
                    map_term_buffer_putchar(MAP_SECRET_DOOR);
                    got++;
                }
            }
        }
    } while (got);
}

/*
 * Wrap all corridors in walls
 */
static void maze_add_corridor_walls (void)
{
    int32_t x;
    int32_t y;
    int32_t dx;
    int32_t dy;

    for (x = 1; x < MAZE_CHAR_WIDTH - 1; x++) {
        for (y = 1; y < MAZE_CHAR_HEIGHT - 1; y++) {

            if ((map_term_buffer_getchar(x, y) != MAP_CORRIDOR)) {
                continue;
            }

            for (dx = -1; dx <=1; dx++) {
                for (dy = -1; dy <=1; dy++) {
                    if (map_term_buffer_getchar(x+dx, y+dy) == MAP_EMPTY) {
                        map_term_buffer_goto(x+dx, y+dy);
                        map_term_buffer_putchar(MAP_CORRIDOR_WALL);
                    }
                }
            }
        }
    }
}

/*
 * maze_print
 */
static void maze_print (dungeon_t *dg, uint8_t last)
{
    int32_t w = MAZE_WIDTH;
    int32_t h = MAZE_HEIGHT;
    int32_t y;
    int32_t x;

    for (y = 0; y < MAZE_CHAR_HEIGHT; y++) {
        for (x = 0; x < MAZE_CHAR_WIDTH; x++) {
            map_term_buffer_goto(x, y);
            map_term_buffer_putchar(MAP_EMPTY);
        }
    }

    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {

            maze_cell_t *c = MAZE_CELL(dg->maze, x, y);

            jigpiece_printat(dg,
                             (JIGPIECE_WIDTH) * x+1,
                             (JIGPIECE_HEIGHT) * y+1, c->jigpiece);
        }
    }

    if (last) {
        maze_verify_corridor_endpoints();
        maze_verify_corridor_endpoints();
        maze_verify_corridor_endpoints();
        maze_remove_deadend_doors();
        maze_add_secret_doors();
        jigpiece_create_mirrored_fragments(dg);
        jigpiece_create_mirrored_frag_alt(dg);
        jigpiece_add_fragments(dg);
        maze_add_corridor_walls();
    }

    if (last) {
        map_term_buffer_print();
        map_term_buffer_print_file();

        memset(map_term_buffer, 0, sizeof(map_term_buffer));
        putchar('\n');
        map_term_buffer_set_fgbg(TERM_COLOR_WHITE, TERM_COLOR_BLACK);

        putchar('\n');
        maze_print_cells(dg);
        putchar('\n');
    }
}

/*
 * maze_jigsaw_generate_all_possible_pieces
 *
 * Make a list of all the jigpiece that satisfy the connection needs of this
 * maze cell.
 */
static boolean maze_jigsaw_generate_all_possible_pieces (dungeon_t *dg)
{
    int32_t w = MAZE_WIDTH;
    int32_t h = MAZE_HEIGHT;
    maze_cell_t *mcell;
    int32_t dir;
    int32_t x;
    int32_t y;
    int32_t c;
    int32_t a;
    int32_t b;

    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {

            mcell = MAZE_CELL(dg->maze, x, y);
            mcell->possible_jigpieces_size = 0;
            mcell->x = x;
            mcell->y = y;

            for (c = 1; c < dg->jigpieces_cnt; c++) {
                for (dir = 0; dir < DIR_MAX; dir++) {
                    /*
                     * Filter to pieces that have at least one exit in each of
                     * all the directions needed.
                     */
                    a = dg->jigpiece[c].exits[dir] ? 1 : 0;
                    b = MAZE_HAS_EXIT(dg->maze, x, y, dir) ? 1 : 0;

                    if (a != b) {
                        break;
                    }
                }

                if (dir == DIR_MAX) {
                    mcell->possible_jigpieces[mcell->possible_jigpieces_size++] = c;
                }
            }

            if (!mcell->possible_jigpieces_size) {
                return (false);
            }
        }
    }

    return (true);
}

/*
 * maze_generate_jigpiece_find
 *
 * Returns 0 on failure. 1 on being able to fill the maze with jigsaw pieces.
 */
static int32_t
maze_generate_jigpiece_find (dungeon_t *dg, maze_cell_t *mcell,
                             uint32_t *count)
{
    int32_t intersect_list[mcell->possible_jigpieces_size];
    int32_t intersect_list_size;
    maze_cell_t *ocell;
    int32_t dir;
    int32_t exits;
    int32_t p;
    int32_t c;
    int32_t i;
    int32_t ok;

    (*count)++;

    /*
     * Tried to long to solve this maze? Try another.
     */
    if (*count > MAZE_HOW_LONG_TO_SPEND_TRYING_TO_SOLVE_MAZE) {
        return (0);
    }

    /*
     * Already solved this cell?
     */
    if (mcell->jigpiece) {
        return (1);
    }

    c = 0;
    intersect_list_size = 0;

    /*
     * +---+---+
     * | M |   |
     * +   +   +
     * | O |   |
     * +   +   +
     * |       |
     * +---+---+
     *
     * mcell == M
     * ocell == O
     */
    for (p = 0; p < mcell->possible_jigpieces_size; p++) {

        c = mcell->possible_jigpieces[p];

        /*
         * Filter to only cells that satisfy all direction exits.
         */
        for (dir = 0; dir < DIR_MAX; dir++) {

            /*
             * Try to satisfy M and create a cell that joins onto all its
             * neighbors.
             */
            if (!(mcell->exits & (1 << dir))) {
                continue;
            }

            ocell = mcell->exit[dir];
            if (!ocell) {
                continue;
            }

            /*
             * If no neighbor in a given direction, we can't determine what to
             * put.
             */
            if (!ocell->jigpiece) {
                continue;
            }

#ifdef MAZE_NO_ROONEXT_TO_OTHER_ROOMS
            /*
             * No rooms next to other rooms just to cut the number of rooms in
             * half.
             */
            if (dg->jigpiece[c].has[MAP_FLOOR]) {
                if (dg->jigpiece[ocell->jigpiece].has[MAP_FLOOR]) {
                    break;
                }
            }
#endif

            /*
             * If the cells join exactly then this is a possibility.
             */
            if (!jigpiece_intersect_score(dg, c, dir, ocell->jigpiece)) {
                break;
            }
        }

        if (dir == DIR_MAX) {
            intersect_list[intersect_list_size++] = c;
        }
    }

    if (!intersect_list_size) {
        mcell->jigpiece = 0;
        return (0);
    }

    /*
     * Choose a random jigsaw piece. 
     */
    for (i=0; i < 1 + (rand() % 3); i++) {
        mcell->jigpiece = intersect_list[rand() % intersect_list_size];

        exits = 0;
        ok = 1;

        /*
         * And now make sure it fits all adjoining exits.
         */
        for (dir = 0; dir < DIR_MAX; dir++) {
            if (!(mcell->exits & (1 << dir))) {
                continue;
            }

            ocell = mcell->exit[dir];
            if (!ocell) {
                continue;
            }

            if (ocell->jigpiece) {
                continue;
            }

            exits = 1;

            /*
             * If it does not fit any one direction, abort.
             */
            if (!maze_generate_jigpiece_find(dg, ocell, count)) {
                ok = 0;
                break;
            }
        }

        /*
         * If there were no adjoining pieces, then we pass automatically.
         */
        if (!exits) {
            return (1);
        }

        if (!ok) {
            /*
             * This piece does not fit. Try another.
             */
            continue;
        }

        /*
         * This piece fits
         */
        return (1);
    }

    mcell->jigpiece = 0;

    return (ok);
}

/*
 * maze_jigsaw_solve
 */
static int32_t maze_jigsaw_solve (dungeon_t *dg)
{
    int32_t w = MAZE_WIDTH;
    int32_t h = MAZE_HEIGHT;
    maze_cell_t *mcell;
    maze_cell_t *ocell;
    int32_t x, y;
    int32_t dir;
    int32_t c;

    x = 0;
    y = 0;
    mcell = MAZE_CELL(dg->maze, x, y);

    for (c = 1; c < mcell->possible_jigpieces_size; c++) {
        /*
         * Reset the maze.
         */
        for (y = 0; y < h; y++) {
            for (x = 0; x < w; x++) {
                ocell = MAZE_CELL(dg->maze, x, y);
                ocell->jigpiece = 0;
            }
        }

        x = 0;
        y = 0;
        mcell->jigpiece =
            mcell->possible_jigpieces[rand() % mcell->possible_jigpieces_size];

        for (dir = 0; dir < DIR_MAX; dir++) {
            if (!(mcell->exits & (1 << dir))) {
                continue;
            }

            ocell = mcell->exit[dir];
            if (!ocell) {
                continue;
            }

            /*
             * Recursively try to solve the maze jigsaw.
             */
            uint32_t count = 0;

            if (!maze_generate_jigpiece_find(dg, ocell, &count)) {
                break;
            }
        }

        /*
         * If we satisfy all directions, we have a complete solution.
         */
        if (dir == DIR_MAX) {
            return (1);
        }
    }

    return (0);
}

/*
 * maze_solve_search
 */
static int32_t maze_solve_search (dungeon_t *dg, maze_cell_t *c)
{
    int32_t dir;

    if (c->solved) {
        return (0);
    }

    c->solved = 1;
    c->best = 1;

    if (c->end) {
        return (1);
    }

    for (dir = 0; dir < 4; dir++) {
        if (c->exits & (1 << dir)) {
            if (maze_solve_search(dg, c->exit[dir])) {
                return (1);
            }
        }
    }

    c->best = 0;
    return (0);
}

/*
 * maze_solve
 */
static int32_t maze_solve (dungeon_t *dg, int32_t w, int32_t h)
{
    int32_t x, y;
    maze_cell_t *s = 0;
    maze_cell_t *e = 0;
    maze_cell_t *c;

    /*
     * Assign a random start and exit.
     */
    while (1) {
        x = rand() % w;
        y = rand() % h;
        c = MAZE_CELL(dg->maze, x, y);

        if (!c->exits) {
            continue;
        }

        if (c == s) {
            continue;
        }

        if (c == e) {
            continue;
        }

        if (!s) {
            s = c;
            continue;
        }

        if (!e) {
            e = c;

            s->start = 1;
            e->end = 1;

            if (maze_solve_search(dg, s)) {
                return (1);
            }

            s->start = 0;
            e->end = 0;

            return (0);
        }
    }
}

/*
 * maze_generate_and_solve
 */
static boolean maze_generate_and_solve (dungeon_t *dg)
{
    int32_t y;
    int32_t x;
    int32_t w = MAZE_WIDTH;
    int32_t h = MAZE_HEIGHT;

    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            maze_cell_t * c = MAZE_CELL(dg->maze, x, y);

            c->exits = 0;
            c->exit[DIR_NORTH] = y > 0 ?
                            MAZE_CELL(dg->maze,   x  , y - 1) : 0;
            c->exit[DIR_SOUTH] = y < h-1 ?
                            MAZE_CELL(dg->maze,   x  , y + 1) : 0;
            c->exit[DIR_WEST]  = x > 0 ?
                            MAZE_CELL(dg->maze, x - 1,   y  ) : 0;
            c->exit[DIR_EAST]  = x < w-1 ?
                            MAZE_CELL(dg->maze, x + 1,   y  ) : 0;
        }
    }

    maze_generate_all_random_directions(dg, MAZE_CELL(dg->maze, 0, 0));

    if (!maze_solve(dg, w, h)) {
#ifdef MAZE_DEBUG_SHOW_AS_GENERATING
        maze_print(dg, 0);
#endif
        return (false);
    }

#ifdef MAZE_DEBUG_SHOW_AS_GENERATING
    maze_print(dg, 0);
#endif
    return (true);
}

/*
 * generate_level
 */
static int32_t generate_level (const char *jigpiece_file,
                               const char *frag_file,
                               int32_t opt_seed)
{
    dungeon_t *dg;
    int32_t c;

    char *buf;

    dg = mymalloc(sizeof(*dg), __FUNCTION__);
    if (!dg) {
        DIE("no memory");
    }

    memset(dg, 0, sizeof(*dg));

    int seed;

    if (opt_seed) {
        seed = opt_seed;
    } else {
        seed = time(0);
    }

    srand(seed);

    buf = filetobuf(jigpiece_file);
    if (!buf) {
        DIE("no buf");
    }

    jigpieces_read(dg, buf);
    myfree(buf);

    for (c = 0; c < dg->jigpieces_cnt; c++) {
        jigpiece_count_char_types(dg, c);
    }

    jigpiece_create_room_door_varieties(dg);
    jigpiece_create_room_door_corridors(dg);
    jigpiece_create_mirrored_pieces(dg);
    jigpiece_create_exits(dg);

    for (;;) {
        if (!maze_generate_and_solve(dg)) {
            printf("seed %u, maze create failed", seed);
            goto reseed;
        }

        if (!maze_jigsaw_generate_all_possible_pieces(dg)) {
            printf("seed %u, maze connections failed", seed);
            goto reseed;
        }

        if (!maze_jigsaw_solve(dg)) {
            printf("seed %u, maze generate failed", seed);
            goto reseed;
        }

        break;
reseed:
        fflush(stdout);
        seed = time(0);
        srand(seed);
        printf("try seed %u", seed);
        memset(dg->maze, 0, sizeof(dg->maze));
    }

    maze_print(dg, 1);
    myfree(dg);

    return (1);
}

/*
 * main
 */
int32_t map_test (int32_t argc, char **argv)
{
    char *jigpiece_file;
    char *frag_file;
    int32_t rc;
    char c;

    jigpiece_file = "data/map/jigsaw.map";
    frag_file = "fragments.map";
    opt_seed = 0;

    while ((c = getopt(argc, argv, "f:s:")) != -1) {
        switch (c) {
        case 's':
            opt_seed = atoi(optarg);;
            break;

        default:
            DIE("-f <input file file>, -s <seed>");
            break;
        }
    }

    rc = generate_level(jigpiece_file, frag_file, opt_seed);
    if (!rc) {
        DIE("failed to generate a maze!");
    }

    return (rc);
}

