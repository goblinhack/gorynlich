/*
    goblinhack.
    Copyright (C) 1999-2010 Neil McGill

    This game is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This game is distributed in the hope that it will be fun,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this game; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Neil McGill

    $Id: maze.cpp 479 2007-03-12 02:13:38Z goblinhack $
*/

#include <libgen.h>
#include <unistd.h>
#include <SDL.h>

#include "main.h"
#include "map.h"
#include "math.h"

typedef struct map_box_ {
    int32_t x1, y1, x3, y3;
} map_box;

static map_box boxes[MAP_WIDTH_LOG+1][MAP_WIDTH*MAP_HEIGHT];
static uint32_t boxes_index[MAP_WIDTH_LOG+1];
static float fmap[MAP_WIDTH][MAP_HEIGHT];

//
// Break a box into four more smaller boxes and store them at the next
// recurse level.
//
// (x1,y1)-----(x2,y1)-----(x3,y1)
//    |           |           |
//    |           |           |
// (x1,y2)-----(x2,y2)-----(x3,y2)
//    |           |           |
//    |           |           |
// (x1,y3)-----(x2,y3)-----(x3,y3)
//
//
static void 
make_boxes (int32_t x1, int32_t y1, int32_t x3, int32_t y3, uint32_t level)
{
    uint32_t index;

    if (level >= ARRAY_SIZE(boxes_index)) {
        DIE("ran out of box levels at index %u", level);
    }

    index = boxes_index[level];
    if (index >= ARRAY_SIZE(boxes[0])) {
        DIE("ran out of boxes at level %d, index %u", level, index);
    }

    map_box *box = &boxes[level][index];
    box->x1 = x1;
    box->y1 = y1;
    box->x3 = x3;
    box->y3 = y3;
    boxes_index[level]++;

    if (x3 - x1 == 1) {
        return;
    }

    if (y3 - y1 == 1) {
        return;
    }

    level++;

    int32_t x2 = ((x3 - x1) / 2) + x1;
    int32_t y2 = ((y3 - y1) / 2) + y1;

    make_boxes(x1, y1, x2, y2, level);
    make_boxes(x2, y1, x3, y2, level);
    make_boxes(x1, y2, x2, y3, level);
    make_boxes(x2, y2, x3, y3, level);
}

//
// Walk the current box (and then the next level below) and assign heights
// to the corners. We avoid recursion here and do each level one at a time
// to avoid any boxing of the landscape.
//
// (x1,y1)-----(x2,y1)-----(x3,y1)
//    |           |           |
//    |           |           |
// (x1,y2)-----(x2,y2)-----(x3,y2)
//    |           |           |
//    |           |           |
// (x1,y3)-----(x2,y3)-----(x3,y3)
//
//
static void make_map (float stdev, float stdev_shrink_factor)
{
    map_box *box;
    int32_t l;
    int32_t s;

    for (l = 0; l < ARRAY_SIZE(boxes_index); l++) {
        for (s = 0; s < boxes_index[l]; s++ ) {

            box = &boxes[l][s];
            int32_t x1 = box->x1;
            int32_t y1 = box->y1;
            int32_t x3 = box->x3;
            int32_t y3 = box->y3;

            int32_t x2 = ((x3 - x1) / 2) + x1;
            int32_t y2 = ((y3 - y1) / 2) + y1;

            fmap[x2][y1] =
                gauss(((fmap[x1][y1] + fmap[x3][y1]) / 2.0), stdev);
            fmap[x1][y2] =
                gauss(((fmap[x1][y1] + fmap[x1][y3]) / 2.0), stdev);
            fmap[x2][y3] =
                gauss(((fmap[x1][y3] + fmap[x3][y3]) / 2.0), stdev);
            fmap[x3][y2] =
                gauss(((fmap[x3][y1] + fmap[x3][y3]) / 2.0), stdev);
            fmap[x2][y2] =
                gauss(((fmap[x1][y1] + fmap[x3][y1] +
                        fmap[x1][y3] + fmap[x3][y3]) / 4.0), stdev);
        }

        stdev *= stdev_shrink_factor;

        if (!(int32_t)stdev) {
            stdev = 1.0;
        }
    }
}

void fractal_gen (map_frame_ctx_t *map,
                  float stdev,
                  float stdev_shrink_factor,
                  uint16_t rock,
                  uint16_t rock2)
{
    const int32_t maze_w = map->map_width;
    const int32_t maze_h = map->map_height;
    int32_t max_vertical_height = 100;
    int32_t x, y, z, height;

    memset(fmap, 0, sizeof(fmap));
    memset(boxes, 0, sizeof(boxes));
    memset(boxes_index, 0, sizeof(boxes_index));

    make_boxes(0, 0, /* top left */
               maze_w - 1, maze_h - 1, /* bottom right */
               0 /* level */);

    fmap[0][0]                        = rand() % max_vertical_height;
    fmap[maze_w - 1][0]               = rand() % max_vertical_height;
    fmap[0][maze_h - 1]               = rand() % max_vertical_height;
    fmap[maze_w - 1][maze_h - 1]      = rand() % max_vertical_height;

    fmap[0][0]                        -= max_vertical_height / 2.0;
    fmap[maze_w - 1][0]               -= max_vertical_height / 2.0;
    fmap[0][maze_h - 1]               -= max_vertical_height / 2.0;
    fmap[maze_w - 1][maze_h - 1]      -= max_vertical_height / 2.0;

    make_map(stdev, stdev_shrink_factor);

    for (x = 0; x < maze_w; x++) {
        for (y = 0; y < maze_h; y++) {
            height = (int32_t)
                            (
                             ((fmap[x][y] / max_vertical_height)) * 
                             ((float)(MAP_DEPTH))
                            );
                            
            if (height < 0) {
                height = 0;
            }

            if (height > MAP_DEPTH - 1) {
                height = MAP_DEPTH - 1;
            }

            char debug[] = " .,_:;!x*oO%&X#@%";

            printf("%c", debug[height]);

//            height = height % MAP_DEPTH;
            for (z = 0; z < height; z++) {
                if (height > MAP_DEPTH / 2) {
                    map_set(map, x, y, z, rock2);
                } else {
                    map_set(map, x, y, z, rock);
                }
            }
        }
        printf("\n");
    }

#define nDEBUG
#ifdef DEBUG
    for (y = 0; y < maze_h; y++) {
        for (x = 0; x < maze_w; x++) {
            if (fmap[x][y] > 120) {
                printf("^");
            } else if (fmap[x][y] > 100) {
                printf("&");
            } else if (fmap[x][y] > 90) {
                printf("=");
            } else if (fmap[x][y] > 80) {
                printf(":");
            } else if (fmap[x][y] > 50) {
                printf(".");
            } else if (fmap[x][y] < 0) {
                printf("w");
            } else if (fmap[x][y] < -10) {
                printf("W");
            } else {
                printf(" ");
            }
        }
        printf("\n");
    }
#endif
}
