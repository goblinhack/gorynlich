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

#if 0
static int GH_MAP_WIDTH                 = 40;
static int GH_MAP_HEIGHT                = 40;

static int GH_MAP_FILL_PROB             = 40;
static int GH_MAP_R1                    = 5;
static int GH_MAP_R2                    = 2;
static int GH_MAP_GENERATIONS           = 5;

static int randpick (void)
{
    if ((rand () % 100) < GH_MAP_FILL_PROB) {
        return (GH_MAP_ROCK);
    } else {
        return (GH_MAP_ROOM);
    }
}

//
// Grow our cells
//
void gh_maze::cave_generation (void)
{
    GH_FUNCTION()

    int x, y, i, j;

    for (y=1; y < maze_h-1; y++) {
        for (x=1; x < maze_w-1; x++) {

            int adjcount_r1 = 0,
            adjcount_r2 = 0;

            //
            // Count adjacent room tiles.
            //
            for (i=-1; i <= 1; i++) {
                for (j=-1; j <= 1; j++) {
                    if (map[y+i][x+j] != GH_MAP_ROOM) {
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

                    if (map[i][j] != GH_MAP_ROOM) {
                        adjcount_r2++;
                    }
                }
            }

            //
            // Adjust for the grow threshold for rock or flow.
            //
            if ((adjcount_r1 >= GH_MAP_R1) ||
                (adjcount_r2 <= GH_MAP_R2)) {
                map_save[y][x] = GH_MAP_ROCK;
            } else {
                map_save[y][x] = GH_MAP_ROOM;
            }
        }
    }

    for (y=1; y < maze_h-1; y++) {
        for (x=1; x < maze_w-1; x++) {
            map[y][x] = map_save[y][x];
        }
    }
}

//
// Generate a cave!
//
void gh_maze::make_cave (void)
{
    GH_FUNCTION()

    int x, y, i;

    for (y=1; y < maze_h-1; y++) {
        for (x=1; x < maze_w-1; x++) {
            map[y][x] = randpick();
        }
    }

    for (y=0; y < maze_h; y++) {
        for (x=0; x < maze_w; x++) {
            map_save[y][x] = GH_MAP_ROCK;
        }
    }

    for (y=0; y < maze_h; y++) {
        map[y][0] = map[y][maze_w-1] = GH_MAP_ROCK;
        map[y][0] = map[y][maze_w-2] = GH_MAP_ROCK;
        map[y][0] = map[y][maze_w-3] = GH_MAP_ROCK;
        map[y][0] = map[y][maze_w-4] = GH_MAP_ROCK;
        map[y][0] = map[y][maze_w-5] = GH_MAP_ROCK;
        map[y][0] = map[y][0] = GH_MAP_ROCK;
        map[y][0] = map[y][1] = GH_MAP_ROCK;
        map[y][0] = map[y][2] = GH_MAP_ROCK;
        map[y][0] = map[y][3] = GH_MAP_ROCK;
        map[y][0] = map[y][4] = GH_MAP_ROCK;
    }

    for (x=0; x < maze_w; x++) {
        map[0][x] = map[maze_h-1][x] = GH_MAP_ROCK;
        map[0][x] = map[maze_h-2][x] = GH_MAP_ROCK;
        map[0][x] = map[maze_h-3][x] = GH_MAP_ROCK;
        map[0][x] = map[maze_h-4][x] = GH_MAP_ROCK;
        map[0][x] = map[maze_h-5][x] = GH_MAP_ROCK;
        map[0][x] = map[0][x] = GH_MAP_ROCK;
        map[0][x] = map[1][x] = GH_MAP_ROCK;
        map[0][x] = map[2][x] = GH_MAP_ROCK;
        map[0][x] = map[3][x] = GH_MAP_ROCK;
        map[0][x] = map[4][x] = GH_MAP_ROCK;
    }

    GH_MAP_GENERATIONS = 3 + (rand() % 10);

    for (i=0; i < GH_MAP_GENERATIONS; i++) {
        cave_generation();
    }
}

//
// Generate a maze!
//
void gh_maze::cave_gen (void)
{
    GH_FUNCTION()

    GH_LOG("Generate new cave");

redo:
    maze_w = GH_MIN(GH_MAP_WIDTH + level*2, GH_MAX_MAZE_SIZE - 2);
    maze_h = GH_MIN(GH_MAP_HEIGHT + level*2, GH_MAX_MAZE_SIZE - 2);
    maze_w = GH_MAX(maze_w, GH_MIN_MAZE_SIZE);
    maze_h = GH_MAX(maze_h, GH_MIN_MAZE_SIZE);

    maze_reset();
    memset(map_save, ' ', sizeof(map_save));
    make_cave();

    //
    // Change the floor to dirt!
    //
    for (int y=1; y < maze_h-1; y++) {
        for (int x=1; x < maze_w-1; x++) {
            if (map[y][x] == GH_MAP_ROOM) {
                map[y][x] = GH_MAP_CAVE_DIRT;
            }
        }
    }

    //
    // More of everything in caves.
    //
    place_traps();
    place_traps();
    place_monst();
    place_monst();
    place_treasure();
    place_treasure();
    place_potion();
    place_potion();

    place_obstacle(GH_MAP_START);
    place_obstacle(GH_MAP_EXIT);

    if (!maze_test()) {
        /*
         * No solution for the maze!
         */
        goto redo;
    }

    maze_print();
}
#endif
