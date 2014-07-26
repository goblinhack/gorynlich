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
#define JIGPIECE_MAX                    20000

/*
 * A solvable maze, e.g.:
 * +---+---+---+
 * |   |       |
 * +   +   +---+
 * | E |       |
 * +   +   +---|
 * | * |       |
 * +---+---+---+
 *
 * which expands to something like:
 * 
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

enum {
    MAP_EMPTY           = ' ',
    MAP_WATER           = 'W',
    MAP_SPACE           = 's',
    MAP_FLOOR           = '.',
    MAP_ROCK            = 'r',
    MAP_WALL            = 'x',
    MAP_CORRIDOR        = ',',
    MAP_CORRIDOR_WALL   = '+',
    MAP_CORRIDOR_POSS   = '?',
    MAP_CORRIDOR_OK     = 'o',
    MAP_CORRIDOR_DEAD   = '0',
    MAP_CORRIDOR_FORK   = '/',
    MAP_LADDER          = 'H',
    MAP_MONST           = 'm',
    MAP_MAJ_MONST       = 'M',
    MAP_TRAP            = '!',
    MAP_TRAPDOOR        = 'T',
    MAP_TREASURE        = '$',
    MAP_FOOD            = 'f',
    MAP_LAVA            = 'L',
    MAP_EXIT_WEST       = '<',
    MAP_EXIT_EAST       = '>',
    MAP_EXIT_SOUTH      = 'v',
    MAP_EXIT_NORTH      = '^',
    MAP_END             = 'E',
    MAP_START           = 'S',
    MAP_PADDING         = 'Z',
    MAP_GRASS           = 'g',
    MAP_MAX             = 255,
};

uint8_t map_jigsaw_buffer_getchar(int32_t x, int32_t y);
uint8_t map_jigsaw_buffer2_getchar(int32_t x, int32_t y);