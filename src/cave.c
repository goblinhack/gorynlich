/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <libgen.h>
#include <unistd.h>
#include <SDL.h>

#include "main.h"
#include "map.h"

/*
 *
 *
  Cellular Automata Method for Generating Random Cave-Like Levels


      From RogueBasin

Jump to: navigation < #column-one>, search < #searchInput>

By Jim Babcock < /index.php?title=Jim_Babcock&action=edit>

It is an old and fairly well documented trick to use cellular automata
 < /index.php?title=Cellular_automata > to generate cave-like structures.
The basic idea is to fill the map randomly, then repeatedly apply the
4-5 rule: a tile is a wall if it is a wall and has 4 neighbors that are
walls, or if it is not a wall and has 5 neighbors that are. This rule
can be stated more simply: a tile becomes or remains a wall if the 3x3
region centered on it contains at least 5 walls. (/Note: It is important
to do this for each tile simultaneously. If you update one, then use its
value when you update the next, your results won't look as good, and the
tricks described later won't work./)

If the map initially contains 45% walls, and the process above is
repeated 5 times, the output looks like (for example)

############################################################
###....####################################.....############
##......######################..#########.........##########
##......#####################....#######...........####.####
##......###################.........................##...###
##......##################..........................###...##
#........##############.............................###...##
#........#############...............................#....##
##.......##############..................................###
##.......###..############..............................####
##.......##....############.............................####
#..............############...###........................###
#...............###########..#####...............##.......##
#................#################...............##.......##
##.....#####..........###########....#..........###.......##
##....#######...........########....###.........####......##
##....#######............######....####........#####......##
##....#######.............####....#####.......#####......###
#......######..............###....####........####......####
#.......######.............###...####.........###.......####
#........#####.............###..####.....................###
##........####..............#...####.....................###
#####......##...................####.....................###
######...........................##.....................####
######..................................................####
######.........###.....................####.............####
######......#########.................######............####
#######....#############.......##############.....###..#####
##############################################..############
############################################################

The problem is, the results of the algorithm are very inconsistent. Not
only is it prone to generating disjoint (not connected) maps, like in
this example, with the same parameters:

############################################################
#####################################################..#####
#####.....##################...###############............##
#####......###########.####....########.#####..............#
#####.......#########..###.....###############.............#
####.............#####.###....###################.........##
###...............########...####################........###
##.................#######...####################........###
##.......##.........#####.....##################.........###
##......####.........###.......################...........##
##.....########.................#######..######...........##
##...###########................######...#######..........##
#########..######..............######....########........###
########....######..#####......#####.....##########......###
#######......#############.....#####.....###########.....###
#######.......############......###.......###########.....##
#######.......###########.......###.......###########.....##
######.......####..######.......####.......#########......##
#####.......####....#####.......####..........######.....###
####........####......####......####...........#####.....###
####.........###.......###......####...##......######.....##
####...##.....###.......#......###########.....#######.....#
#####.####.....#####...........############....########....#
##########.....######..........############....#########..##
#########.......#####...........##########.....#############
#########.......####...............#####........############
##########......####................###...........##########
###########....#####.....######.....####...........#########
################################...##########.....##########
############################################################

it also sometimes generates maps which consist of basically one huge
open space, like this one:

############################################################
############################################################
#################.####.######..######........###############
########...#####...##...####....####..........######...#####
###.####...####....###..####....####..........#####.....####
##...###....##.....###...##.....###............###......####
###.####...........###..........###.....###.........########
########...........###...........#.......#.........#########
#######.............#..............................#########
######.....##..........................................#####
###.......###...........................................####
##.......####...........................................####
##......................................................####
##.....................................................#####
##.....................................................#####
#.....................................##...............#####
#.....................................##................####
#...........................###......###................####
##...........###............###.....#####...............####
###...........#..............##.....######............######
######.......................##......######........#########
##########..................##.........#####......##########
###########................###..........####......##########
############................#...........####......##########
###.....#####.........##...............#####.......####..###
###.....#####.......#####.............#####..............###
###.....####.......#####..............#####.....##.......###
####...#####.......#####.....#####...######....####.....####
#############.##########....################..##############
############################################################

We can fix the disjoint segments problem in one of three ways. Either
throw away maps that have disjoint segments in them, connect up the
segments after the fact, or fill in all but the biggest segment. We
can't just retry when we get a disjoint map, because if the map is big
then, statistically, that will be almost 100% of the time. Filling in
all but the biggest segment will tend to produce a small area in a map
that was supposed to be big. Connecting up the regions works, but it
tends to look unnatural, as in the example from above, now connected:

############################################################
#####################################################..#####
#####.....##################...###############............##
#####......###########.####....########....................#
#####.......#########..###.....###############.............#
####.............####..###....###################.........##
###...............###.####...####################........###
##.................##.####...####################........###
##.......##.........#.###.....##################.........###
##......####..........##.......################...........##
##.....########.................#######..######...........##
##...###########................######...#######..........##
#########..######..............######....########........###
########............#####......#####.....##########......###
#######......#############...............###########.....###
#######.......############......###.......###########.....##
#######.......###########.......###.......###########.....##
######.......####..######.......####.......#########......##
#####.......####....#####.......####.....................###
####........####......####......####...........#####.....###
####.........###.......###......####...##......######.....##
####...##.....###.......#......###########.....#######.....#
#####.####.....#####...........############....########....#
##########.....######..........############....#########..##
#########.......#####...........##########.....#############
#########.......####...............#####........############
##########......####................###...........##########
###########....#####.....######.....####...........#########
################################...##########.....##########
############################################################

The solution to both problems, as it turns out, is to revisit the
original cellular automata rules. Recall that the original rule was

    * There is a wall initially at P with 45% probability
    * In the next generation, there is a wall at spot P if the number of
      tiles around P which are walls is at least 5

Or, in more compact notation:

    * Winit (p) = rand[0,100) <  45
    * R (p) = the number of tiles within 1 step of p which are walls
    * W? (p) = R (p) ? 5

Now, one of the problems was that we tend to get big, open areas. So why
not try filling those areas in? Change the rule to

    * W? (p) = R (p) ? 5 or p is in the middle of an open space

Or more formally,

    * Rn (p) = the number of tiles within n steps of p which are walls
    * W? (p) = R1 (p)?5 || R2 (p)=0

So how does it look?

Winit (p) = rand[0,100) <  45
Repeat 5: W? (p) = R1 (p) ? 5 || R2 (p) ? 1

############################################################
##....######################################################
#.......###..#####..####....###########################..###
#...#.........###.............##########..############....##
##...###...#..###..............########....######..###....##
###...######...#..##...........######.....######....###..###
####..######......##..##........####...#..######....########
####..###.#.......#...##...#.....##...###..######...########
#####.........##...........##........####.....##.....#######
#####.........##...........#.......##.....#.............####
####...###...##................#...##......###...........###
####...###..##...###....##.....##..##..##..###....##.....###
#########..###..####...###.............###..##..####.....###
########...##..#####...##...............#...#...####....####
#######........######......###...##....................#####
#######..##.....######....##########...................#####
######..####.......####...#########...................######
####....####..##....##.....#######...##..............#######
###.....###..#####......#...####....##................######
##..##..###..###........##.........#....#......##......#####
#..####..##...##.................###...##.....####......####
#.....#..###..#..##..........#..###..###.....#####......####
##.......####.....#...##........##..###....#######......####
######....###.......................###...#######....#######
#######......................##.....###...#######...########
########.................#######....####...#####....########
########..............###########..######...........########
#########....####....######################........#########
###################.########################################
############################################################

This is more interesting - it doesn't have any big open areas, it has a
decent layout. It's almost fully connected. Still, it has some new
problems: there are isolated single-tile walls in places, and in general
it's not very smooth. But with a little tweaking:

Winit (p) = rand[0,100) <  40
Repeat 4: W? (p) = R1 (p) ? 5 || R2 (p) ? 2
Repeat 3: W? (p) = R1 (p) ? 5

############################################################
###...###########..#############################.....#######
##..........####....################..#########.........####
##...........##.....####..#########.......####..######...###
##.......#..........###....###.................########..###
##......###........###........................#########..###
##.......##.........#........................##########...##
##.......###...........##.............###....#########.....#
##.......######.......####...........#####....#####........#
###.....#########....#####...........######...#####........#
###########################...........#####...#######.....##
#############...###########.............##....########....##
############.........#######...................#######....##
###########...........########......###............##....###
###..#####.............#########...##########............###
##....###...............######################..........####
###..........................######..#########..........####
####..........................###.....#######...........####
####.................##................##................###
####...###..........####...............#..................##
###...#####.........####..............##......##...........#
##....########......#####............####....####..........#
##....#########.....#####............####....####..........#
##.....######.......#####.............##.....####...##.....#
##......##..........#####....................####..####....#
###.................####.........###........############..##
###............##..######.###...############################
####..........##############################################
######..####################################################
############################################################

Notice that the initial fill percentage is a little lower, the cutoffs
are different, and we switch rules after a few generations. This is more
like the desired result. So, with these parameters, I give you some more
samples, at various sizes.

##############################
##..###....###.....###########
#...........................##
#.................##.........#
##..###..........#######.....#
########.........#######.....#
############......####......##
#############...............##
###..########...............##
##....#######...###..#......##
##.....######...#######.....##
##......#####...######......##
##...#...###....#####.......##
##..##.........######.......##
##..........##########......##
###........#####..####.....###
###........####....###.....###
###....##...##.....###.....###
###...####........####......##
###...#####......#####......##
##....#####.....#####.......##
##.....###.....####.........##
##..............##..........##
###.........................##
####........................##
####.......................###
####..........##...........###
#####...###..####.........####
####################.....#####
##############################

############################################################
#########......###...##........####...####.....#############
####............................##.....##............#######
###................###.....##..........................#####
###...............#####...####..........................####
###...............#####....####............#............####
####...###.........####.......##.......................#####
##########..........###........##.....................######
##########...........#..........##...................#######
#########.......................#####..............#########
######.........................#######...#......############
#####..........................############....#############
####...........................###########......######..####
###..........##.................#########................###
##.......#######........#..........######...###.........####
##......########.......##............###...######.....######
###.....#######...............#####........########..#######
###......#####...##...........######........################
###......#####..####...........#####.........###############
#######..#####..####............###...........#######....###
########..###...#####......###.................#####......##
########.......######......####.................###.......##
########.......######.......##....##..................##..##
#######.......######....##.......####................####..#
######.......#######....###......####..........###..#####..#
#####........######.....######....##..........##########...#
######........###........######...............########.....#
##########...............#######.............########.....##
#############...#######..###########...#####################
############################################################

################################################################
#######..#####............####..####...###....###########...####
###......####..................................#########.....###
##.......###.......................................####......###
##......####.............##.........................##......####
##......#####......##...####................##..............####
##......##############..#####...............###.............####
##.....###############...#####...............##........##...####
##.....################..######.........#.............####...###
##.....################...#####........###............####....##
###.....#####..##...##....####.........###.............###....##
####....####..............###..........###...................###
####....####.............####..........##....................###
###.....####...........#####.........................##.....####
##.......##............#####....................##..####...#####
##.....................######................###########...#####
###..##........##......###############.....############....#####
########......####....#################..###############..######
#########...####################################################
################################################################

############################################################
#########......###...##........####...####.....#############
####............................##.....##............#######
###................###.....##..........................#####
###...............#####...####..........................####
###...............#####....####............#............####
####...###.........####.......##.......................#####
##########..........###........##.....................######
##########...........#..........##...................#######
#########.......................#####..............#########
######.........................#######...#......############
#####..........................############....#############
####...........................###########......######..####
###..........##.................#########................###
##.......#######........#..........######...###.........####
##......########.......##............###...######.....######
###.....#######...............#####........########..#######
###......#####...##...........######........################
###......#####..####...........#####.........###############
#######..#####..####............###...........#######....###
########..###...#####......###.................#####......##
########.......######......####.................###.......##
########.......######.......##....##..................##..##
#######.......######....##.......####.........##.....####..#
#####........#######....###......####........#####..#####..#
####........#######.....######...#####.......############..#
####.......######..........####..#########..#############..#
###........#####............###..########################.##
##.........####.............###..################.######..##
#....###...####......####....#...######..#######...#####..##
#.....#.....##......######.......#####....#####....#####..##
#...................######........####....###.....#####...##
#....................#####........#####..###......##......##
#....................######........########................#
##......#............########.......#######................#
##......##...........#########......#######...............##
###.....#............####...##.....#######..##...........###
###..........#......####...........######..####..........###
##..........####....####...........#####..######.........###
##...........####..#####............##....######.........###
##............###..######......#...........####..........###
##............###..#######.....##........................###
##.......###...#....#######....#..........................##
###......###.........######.........................##.....#
###.......#..........######........#####...........####....#
###............###...######......########.........#####....#
###...........#####..######.....#########.........####.....#
###...........#####.#######.....########...........###.....#
###...........####..########...#########......##...###....##
###...........####...##################......####..###....##
###...........####......##############.......####..###....##
###...........####..........##########........##...###....##
###............####..........#########.............####..###
###...........#####...........#######..............#########
###.....##########............######.......##......#########
##.....###########.....###.....####.......####......########
##.....############....###......##.......#####........######
###...##############..#####.............#######.......######
################################...##..#####################
############################################################

There's still no guarantee of connectedness, though. However, it's now
consistently almost-connected, so that you can reasonably just drop the
isolated chunks.

Finally, here is the C program I used to try out different parameters.
Before putting this into an actual game, handling of disconnected
regions is needed.

*/

static int32_t MAP_FILL_PROB             = 40;
static int32_t MAP_R1                    = 5;
static int32_t MAP_R2                    = 2;
static int32_t MAP_GENERATIONS           = 5;

/*
 * Used temporarily during level generation.
 */
static thing_templatep map_save[MAP_WIDTH][MAP_HEIGHT];

//
// Grow our cells
//
static void cave_generation (map_frame_ctx_t *map,
                             thing_templatep rock,
                             int32_t z)
{
    const int32_t maze_w = map->map_width;
    const int32_t maze_h = map->map_height;
    int32_t x, y, i, j;

    for (y=1; y < maze_h-1; y++) {
        for (x=1; x < maze_w-1; x++) {

            int32_t adjcount_r1 = 0,
            adjcount_r2 = 0;

            //
            // Count adjacent room tiles.
            //
            for (i=-1; i <= 1; i++) {
                for (j=-1; j <= 1; j++) {
                    if (map_get(map, x+j, y+i, z) != 0) {
                        adjcount_r1++;
                    }
                }
            }

            for (i=y-2; i <= y+2; i++) {
                for (j=x-2; j <= x+2; j++) {

                    if ((abs(i-y) == 2) && (abs(j-x)==2)) {
                        //
                        // Too close to the edge.
                        //
                        continue;
                    }

                    if (i < 0 || j < 0 || i>=maze_h || j>=maze_w) {
                        //
                        // Out of bounds.
                        //
                        continue;
                    }

                    if (map_get(map, j, i, z) != 0) {
                        adjcount_r2++;
                    }
                }
            }

            //
            // Adjust for the grow threshold for rock or flow.
            //
            if ((adjcount_r1 >= MAP_R1) ||
                (adjcount_r2 <= MAP_R2)) {
                map_save[x][y] = rock;
            } else {
                map_save[x][y] = 0;
            }
        }
    }

    for (y=1; y < maze_h-1; y++) {
        for (x=1; x < maze_w-1; x++) {
            map_set(map, x, y, z, map_save[x][y]);
        }
    }
}

//
// Generate a cave!
//
void cave_gen (map_frame_ctx_t *map, thing_templatep rock, int32_t z)
{
    const int32_t maze_w = map->map_width;
    const int32_t maze_h = map->map_height;

    int32_t x, y, i;

    for (y=1; y < maze_h-1; y++) {
        for (x=1; x < maze_w-1; x++) {
            thing_templatep t;

            if ((rand() % 100) < MAP_FILL_PROB) {
                t = rock;
            } else {
                t = 0;
            }

            map_set(map, x, y, z, t);
        }
    }

    for (y=0; y < maze_h; y++) {
        for (x=0; x < maze_w; x++) {
            map_save[x][y] = rock;
        }
    }

    for (y=0; y < maze_h; y++) {
        map_set(map, 0, y, z, rock);
        map_set(map, maze_w-1, y, z, rock);
        map_set(map, 0, y, z, rock);
        map_set(map, maze_w-2, y, z, rock);
        map_set(map, 0, y, z, rock);
        map_set(map, maze_w-3, y, z, rock);
        map_set(map, 0, y, z, rock);
        map_set(map, maze_w-4, y, z, rock);
        map_set(map, 0, y, z, rock);
        map_set(map, maze_w-5, y, z, rock);
        map_set(map, 0, y, z, rock);
        map_set(map, 0, y, z, rock);
        map_set(map, 0, y, z, rock);
        map_set(map, 1, y, z, rock);
        map_set(map, 0, y, z, rock);
        map_set(map, 2, y, z, rock);
        map_set(map, 0, y, z, rock);
        map_set(map, 3, y, z, rock);
        map_set(map, 0, y, z, rock);
        map_set(map, 4, y, z, rock);
    }

    for (x=0; x < maze_w; x++) {
        map_set(map, x, 0, z, rock);
        map_set(map, x, maze_h-1, z, rock);
        map_set(map, x, 0, z, rock);
        map_set(map, x, maze_h-2, z, rock);
        map_set(map, x, 0, z, rock);
        map_set(map, x, maze_h-3, z, rock);
        map_set(map, x, 0, z, rock);
        map_set(map, x, maze_h-4, z, rock);
        map_set(map, x, 0, z, rock);
        map_set(map, x, maze_h-5, z, rock);
        map_set(map, x, 0, z, rock);
        map_set(map, x, 0, z, rock);
        map_set(map, x, 0, z, rock);
        map_set(map, x, 1, z, rock);
        map_set(map, x, 0, z, rock);
        map_set(map, x, 2, z, rock);
        map_set(map, x, 0, z, rock);
        map_set(map, x, 3, z, rock);
        map_set(map, x, 0, z, rock);
        map_set(map, x, 4, z, rock);
    }

    MAP_GENERATIONS = 3 + (rand() % 10);

    for (i=0; i < MAP_GENERATIONS; i++) {
        cave_generation(map, rock, 0);
    }
}
