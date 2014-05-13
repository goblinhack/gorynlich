/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <SDL.h>

#include "main.h"
#include "resource.h"
#include "tex.h"
#include "tile.h"
#include "init_fn.h"
#include "thing.h"

static boolean resource_init_1 (void *context)
{
    return (tex_load("data/gfx/gorynlich.png",
                     "gorynlich") != 0);
}

static boolean resource_init_2 (void *context)
{
    return (tex_load("data/gfx/title2.png",
                     "title2") != 0);
}

static boolean resource_init_3 (void *context)
{
    return (tex_load("data/gfx/title.png",
                     "title") != 0);
}

static boolean resource_init_4 (void *context)
{
    return (tex_load("data/gfx/help1.png",
                     "help1") != 0);
}

static boolean resource_init_5 (void *context)
{
    return (tex_load("data/gfx/help2.png",
                     "help2") != 0);
}

static boolean resource_init_6 (void *context)
{
    return (tex_load("data/gfx/help3.png",
                     "help3") != 0);
}

static boolean resource_init_7 (void *context)
{
    return (tex_load("data/gfx/dragon.png",
                     "dragon") != 0);
}

static boolean resource_init_8 (void *context)
{
    return (tex_load("data/gfx/title3.png",
                     "title3") != 0);
}

static boolean resource_init_9 (void *context)
{
    return (tex_load("data/gfx/title_small.png",
                     "title_small") != 0);
}

static boolean resource_init_10 (void *context)
{
    return (true);
}

static boolean resource_init_11 (void *context)
{
    return (true);
}

static boolean resource_init_12 (void *context)
{
    return (true);
}

static boolean resource_init_13 (void *context)
{
    return (true);
}

static boolean resource_init_14 (void *context)
{
    return (true);
}

static boolean resource_init_15 (void *context)
{
    return (true);
}

static boolean resource_init_16 (void *context)
{
    return (true);
}

static boolean resource_init_17 (void *context)
{
    return (true);
}

static boolean resource_init_18 (void *context)
{
    return (true);
}

static boolean resource_init_19 (void *context)
{
    return (true);
}

static boolean resource_init_20 (void *context)
{
    return (true);
}

static boolean resource_init_21 (void *context)
{
    return (tex_load_tiled("data/gfx/sprites_small.png",
                           "sprites_small",
                           TILE_WIDTH, TILE_HEIGHT) != 0);
}

static boolean resource_init_22 (void *context)
{
    return (true);
}

static boolean resource_init_23 (void *context)
{
    static const char *small_tiles[] = {
        // row 0 floor tiles
        "space",
        "player-collision-map",
        "monst-collision-map",
        "weapon-collision-map",
        "noentry",
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        // row 1 items
        "keys1",
        "keys2",
        "keys3",
        "coins1",
        "chest1",
        "amulet1",
        "water1",
        "water2",
        "spam",
        "potion1",
        "potion2",
        "potion3",
        "potion4",
        "potion5",
        "mask1",
        "mask2",
        "mask3",
        "gem1",
        "gem2",
        "gem3",
        "gem4",
        "gem5",
        "gem6",
        "gem7",
        "generator1",
        "generator4",
        "generator3",
        "generator2",
        "generator5",
        "generator6",
        "generator7",
        0,
        // row 2
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        "sphere1",
        "sphere2",
        "sphere3",
        "sphere4",
        "arrow-up",
        "arrow-tr",
        "arrow-right",
        "arrow-br",
        "arrow-down",
        "arrow-bl",
        "arrow-left",
        "arrow-tl",
        "food1",
        0,
        0,
        0,
        "exit1",
        "exit2",
        "exit3",
        "exit4",
        "exit5",
        "exit6",
        "exit7",
        0,
        // row 3
        "bomb1",
        "bomb2",
        "bomb3",
        "bomb4",
        "bomb5",
        "bomb6",
        "bomb7",
        "bomb8",
        "bomb9",
        "pipe-node",
        "pipe-bot",
        "pipe-left",
        "pipe-bl",
        "pipe-top",
        "pipe-vert",
        "pipe-tl",
        "pipe-t270",
        "pipe-right",
        "pipe-br",
        "pipe-horiz",
        "pipe-t180",
        "pipe-tr",
        "pipe-t90",
        "pipe-t",
        "pipe-x",
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        // row 4
        "explosion1",
        "explosion2",
        "explosion3",
        "explosion4",
        "explosion5",
        "explosion6",
        "explosion7",
        "explosion8",
        "explosion9",
        "explosion10",
        "explosion11",
        "explosion12",
        "explosion13",
        "explosion14",
        "explosion15",
        "explosion16",
        "explosion17",
        "explosion18",
        "explosion19",
        "explosion20",
        "explosion21",
        "explosion22",
        "explosion23",
        "explosion24",
        "explosion25",
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        // row 5
        "plant1",
        "plant2",
        "plant3",
        "plant4",
        "plant5",
        "plant6",
        "plant7",
        "plant8",
        "plant9",
        "plant10",
        "plant11",
        "plant12",
        "seedpod1",
        "seedpod2",
        "seedpod3",
        "seedpod4",
        "seedpod5",
        "seedpod6",
        "seedpod7",
        "seedpod8",
        "seedpod9",
        "seedpod10",
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        // row 6
        "door_0_0",
        "door_0_is_join_node",
        "door_0_is_join_left",
        "door_0_is_join_bot",
        "door_0_is_join_right",
        "door_0_is_join_top",
        "door_0_is_join_horiz",
        "door_0_is_join_vert",
        "door_0_is_join_tl2",
        "door_0_is_join_bl2",
        "door_0_is_join_br2",
        "door_0_is_join_tr2",
        "door_0_is_join_tl",
        "door_0_is_join_bl",
        "door_0_is_join_br",
        "door_0_is_join_tr",
        "door_0_is_join_t270_3",
        "door_0_is_join_t180_3",
        "door_0_is_join_t90_3",
        "door_0_is_join_t_3",
        "door_0_is_join_t270",
        "door_0_is_join_t180",
        "door_0_is_join_t90",
        "door_0_is_join_t",
        "door_0_is_join_t270_2",
        "door_0_is_join_t180_2",
        "door_0_is_join_t90_2",
        "door_0_is_join_t_2",
        "door_0_is_join_t270_1",
        "door_0_is_join_t180_1",
        "door_0_is_join_t90_1",
        "door_0_is_join_t_1",
        // row 7
        "door_0_is_join_x",
        "door_0_is_join_x1",
        "door_0_is_join_x1_270",
        "door_0_is_join_x1_180",
        "door_0_is_join_x1_90",
        "door_0_is_join_x2",
        "door_0_is_join_x2_270",
        "door_0_is_join_x2_180",
        "door_0_is_join_x2_90",
        "door_0_is_join_x3",
        "door_0_is_join_x3_180",
        "door_0_is_join_x4",
        "door_0_is_join_x4_270",
        "door_0_is_join_x4_180",
        "door_0_is_join_x4_90",
        "door_0_is_join_vert2",
        "door_0_is_join_horiz2",
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        // row 8
        "wall_0_0",
        "wall_0_is_join_node",
        "wall_0_is_join_left",
        "wall_0_is_join_bot",
        "wall_0_is_join_right",
        "wall_0_is_join_top",
        "wall_0_is_join_horiz",
        "wall_0_is_join_vert",
        "wall_0_is_join_tl2",
        "wall_0_is_join_bl2",
        "wall_0_is_join_br2",
        "wall_0_is_join_tr2",
        "wall_0_is_join_tl",
        "wall_0_is_join_bl",
        "wall_0_is_join_br",
        "wall_0_is_join_tr",
        "wall_0_is_join_t270_3",
        "wall_0_is_join_t180_3",
        "wall_0_is_join_t90_3",
        "wall_0_is_join_t_3",
        "wall_0_is_join_t270",
        "wall_0_is_join_t180",
        "wall_0_is_join_t90",
        "wall_0_is_join_t",
        "wall_0_is_join_t270_2",
        "wall_0_is_join_t180_2",
        "wall_0_is_join_t90_2",
        "wall_0_is_join_t_2",
        "wall_0_is_join_t270_1",
        "wall_0_is_join_t180_1",
        "wall_0_is_join_t90_1",
        "wall_0_is_join_t_1",
        // row 9
        "wall_0_is_join_x",
        "wall_0_is_join_x1",
        "wall_0_is_join_x1_270",
        "wall_0_is_join_x1_180",
        "wall_0_is_join_x1_90",
        "wall_0_is_join_x2",
        "wall_0_is_join_x2_270",
        "wall_0_is_join_x2_180",
        "wall_0_is_join_x2_90",
        "wall_0_is_join_x3",
        "wall_0_is_join_x3_180",
        "wall_0_is_join_x4",
        "wall_0_is_join_x4_270",
        "wall_0_is_join_x4_180",
        "wall_0_is_join_x4_90",
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        // row 10
        "floor1a",
        "floor1b",
        "floor1c",
        "floor1d",
        "floor1e",
        "floor1f",
        "floor2a",
        "floor2b",
        "floor2c",
        "floor2d",
        "floor2e",
        "floor2f",
        "floor3a",
        "floor3b",
        "floor3c",
        "floor3d",
        "floor3e",
        "floor3f",
        "floor4a",
        "floor4b",
        "floor4c",
        "floor4d",
        "floor4e",
        "floor5a",
        "floor5b",
        "floor5c",
        "floor6a",
        "floor6b",
        "floor6c",
        "floor6d",
        "floor6e",
        "floor6f",
        // row 11
        "wall2_0_0",
        "wall2_0_is_join_node",
        "wall2_0_is_join_left",
        "wall2_0_is_join_bot",
        "wall2_0_is_join_right",
        "wall2_0_is_join_top",
        "wall2_0_is_join_horiz",
        "wall2_0_is_join_vert",
        "wall2_0_is_join_tl2",
        "wall2_0_is_join_bl2",
        "wall2_0_is_join_br2",
        "wall2_0_is_join_tr2",
        "wall2_0_is_join_tl",
        "wall2_0_is_join_bl",
        "wall2_0_is_join_br",
        "wall2_0_is_join_tr",
        "wall2_0_is_join_t270_3",
        "wall2_0_is_join_t180_3",
        "wall2_0_is_join_t90_3",
        "wall2_0_is_join_t_3",
        "wall2_0_is_join_t270",
        "wall2_0_is_join_t180",
        "wall2_0_is_join_t90",
        "wall2_0_is_join_t",
        "wall2_0_is_join_t270_2",
        "wall2_0_is_join_t180_2",
        "wall2_0_is_join_t90_2",
        "wall2_0_is_join_t_2",
        "wall2_0_is_join_t270_1",
        "wall2_0_is_join_t180_1",
        "wall2_0_is_join_t90_1",
        "wall2_0_is_join_t_1",
        // row 12
        "wall2_0_is_join_x",
        "wall2_0_is_join_x1",
        "wall2_0_is_join_x1_270",
        "wall2_0_is_join_x1_180",
        "wall2_0_is_join_x1_90",
        "wall2_0_is_join_x2",
        "wall2_0_is_join_x2_270",
        "wall2_0_is_join_x2_180",
        "wall2_0_is_join_x2_90",
        "wall2_0_is_join_x3",
        "wall2_0_is_join_x3_180",
        "wall2_0_is_join_x4",
        "wall2_0_is_join_x4_270",
        "wall2_0_is_join_x4_180",
        "wall2_0_is_join_x4_90",
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        // row 13
        "wall3_0_0",
        "wall3_0_is_join_node",
        "wall3_0_is_join_left",
        "wall3_0_is_join_bot",
        "wall3_0_is_join_right",
        "wall3_0_is_join_top",
        "wall3_0_is_join_horiz",
        "wall3_0_is_join_vert",
        "wall3_0_is_join_tl2",
        "wall3_0_is_join_bl2",
        "wall3_0_is_join_br2",
        "wall3_0_is_join_tr2",
        "wall3_0_is_join_tl",
        "wall3_0_is_join_bl",
        "wall3_0_is_join_br",
        "wall3_0_is_join_tr",
        "wall3_0_is_join_t270_3",
        "wall3_0_is_join_t180_3",
        "wall3_0_is_join_t90_3",
        "wall3_0_is_join_t_3",
        "wall3_0_is_join_t270",
        "wall3_0_is_join_t180",
        "wall3_0_is_join_t90",
        "wall3_0_is_join_t",
        "wall3_0_is_join_t270_2",
        "wall3_0_is_join_t180_2",
        "wall3_0_is_join_t90_2",
        "wall3_0_is_join_t_2",
        "wall3_0_is_join_t270_1",
        "wall3_0_is_join_t180_1",
        "wall3_0_is_join_t90_1",
        "wall3_0_is_join_t_1",
        // row 14
        "wall3_0_is_join_x",
        "wall3_0_is_join_x1",
        "wall3_0_is_join_x1_270",
        "wall3_0_is_join_x1_180",
        "wall3_0_is_join_x1_90",
        "wall3_0_is_join_x2",
        "wall3_0_is_join_x2_270",
        "wall3_0_is_join_x2_180",
        "wall3_0_is_join_x2_90",
        "wall3_0_is_join_x3",
        "wall3_0_is_join_x3_180",
        "wall3_0_is_join_x4",
        "wall3_0_is_join_x4_270",
        "wall3_0_is_join_x4_180",
        "wall3_0_is_join_x4_90",
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        // row 15
        "wall4_0_0",
        "wall4_0_is_join_node",
        "wall4_0_is_join_left",
        "wall4_0_is_join_bot",
        "wall4_0_is_join_right",
        "wall4_0_is_join_top",
        "wall4_0_is_join_horiz",
        "wall4_0_is_join_vert",
        "wall4_0_is_join_tl2",
        "wall4_0_is_join_bl2",
        "wall4_0_is_join_br2",
        "wall4_0_is_join_tr2",
        "wall4_0_is_join_tl",
        "wall4_0_is_join_bl",
        "wall4_0_is_join_br",
        "wall4_0_is_join_tr",
        "wall4_0_is_join_t270_3",
        "wall4_0_is_join_t180_3",
        "wall4_0_is_join_t90_3",
        "wall4_0_is_join_t_3",
        "wall4_0_is_join_t270",
        "wall4_0_is_join_t180",
        "wall4_0_is_join_t90",
        "wall4_0_is_join_t",
        "wall4_0_is_join_t270_2",
        "wall4_0_is_join_t180_2",
        "wall4_0_is_join_t90_2",
        "wall4_0_is_join_t_2",
        "wall4_0_is_join_t270_1",
        "wall4_0_is_join_t180_1",
        "wall4_0_is_join_t90_1",
        "wall4_0_is_join_t_1",
        // row 16
        "wall4_0_is_join_x",
        "wall4_0_is_join_x1",
        "wall4_0_is_join_x1_270",
        "wall4_0_is_join_x1_180",
        "wall4_0_is_join_x1_90",
        "wall4_0_is_join_x2",
        "wall4_0_is_join_x2_270",
        "wall4_0_is_join_x2_180",
        "wall4_0_is_join_x2_90",
        "wall4_0_is_join_x3",
        "wall4_0_is_join_x3_180",
        "wall4_0_is_join_x4",
        "wall4_0_is_join_x4_270",
        "wall4_0_is_join_x4_180",
        "wall4_0_is_join_x4_90",
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        // row 17
        "wall5_0_0",
        "wall5_0_is_join_node",
        "wall5_0_is_join_left",
        "wall5_0_is_join_bot",
        "wall5_0_is_join_right",
        "wall5_0_is_join_top",
        "wall5_0_is_join_horiz",
        "wall5_0_is_join_vert",
        "wall5_0_is_join_tl2",
        "wall5_0_is_join_bl2",
        "wall5_0_is_join_br2",
        "wall5_0_is_join_tr2",
        "wall5_0_is_join_tl",
        "wall5_0_is_join_bl",
        "wall5_0_is_join_br",
        "wall5_0_is_join_tr",
        "wall5_0_is_join_t270_3",
        "wall5_0_is_join_t180_3",
        "wall5_0_is_join_t90_3",
        "wall5_0_is_join_t_3",
        "wall5_0_is_join_t270",
        "wall5_0_is_join_t180",
        "wall5_0_is_join_t90",
        "wall5_0_is_join_t",
        "wall5_0_is_join_t270_2",
        "wall5_0_is_join_t180_2",
        "wall5_0_is_join_t90_2",
        "wall5_0_is_join_t_2",
        "wall5_0_is_join_t270_1",
        "wall5_0_is_join_t180_1",
        "wall5_0_is_join_t90_1",
        "wall5_0_is_join_t_1",
        // row 18
        "wall5_0_is_join_x",
        "wall5_0_is_join_x1",
        "wall5_0_is_join_x1_270",
        "wall5_0_is_join_x1_180",
        "wall5_0_is_join_x1_90",
        "wall5_0_is_join_x2",
        "wall5_0_is_join_x2_270",
        "wall5_0_is_join_x2_180",
        "wall5_0_is_join_x2_90",
        "wall5_0_is_join_x3",
        "wall5_0_is_join_x3_180",
        "wall5_0_is_join_x4",
        "wall5_0_is_join_x4_270",
        "wall5_0_is_join_x4_180",
        "wall5_0_is_join_x4_90",
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        // row 19
        "fireball1a-tr",
        "fireball1a-right",
        "fireball1a-br",
        "fireball1a-down",
        "fireball1a-bl",
        "fireball1a-left",
        "fireball1a-tl",
        "fireball1a-up",
        "fireball1b-tr",
        "fireball1b-right",
        "fireball1b-br",
        "fireball1b-down",
        "fireball1b-bl",
        "fireball1b-left",
        "fireball1b-tl",
        "fireball1b-up",
        "fireball1c-tr",
        "fireball1c-right",
        "fireball1c-br",
        "fireball1c-down",
        "fireball1c-bl",
        "fireball1c-left",
        "fireball1c-tl",
        "fireball1c-up",
        "fireball1d-tr",
        "fireball1d-right",
        "fireball1d-br",
        "fireball1d-down",
        "fireball1d-bl",
        "fireball1d-left",
        "fireball1d-tl",
        "fireball1d-up",

        // row 20
        
        "ghost1a-right",
        "ghost1a-br",
        "ghost1a-down",
        "ghost1a-bl",
        "ghost1a-left",
        "ghost1a-tl",
        "ghost1a-up",
        "ghost1a-tr",

        "ghost1b-right",
        "ghost1b-br",
        "ghost1b-down",
        "ghost1b-bl",
        "ghost1b-left",
        "ghost1b-tl",
        "ghost1b-up",
        "ghost1b-tr",

        "ghost1c-right",
        "ghost1c-br",
        "ghost1c-down",
        "ghost1c-bl",
        "ghost1c-left",
        "ghost1c-tl",
        "ghost1c-up",
        "ghost1c-tr",

        "ghost1d-right",
        "ghost1d-br",
        "ghost1d-down",
        "ghost1d-bl",
        "ghost1d-left",
        "ghost1d-tl",
        "ghost1d-up",
        "ghost1d-tr",

        // row 21
        
        "ghost2a-right",
        "ghost2a-br",
        "ghost2a-down",
        "ghost2a-bl",
        "ghost2a-left",
        "ghost2a-tl",
        "ghost2a-up",
        "ghost2a-tr",

        "ghost2b-right",
        "ghost2b-br",
        "ghost2b-down",
        "ghost2b-bl",
        "ghost2b-left",
        "ghost2b-tl",
        "ghost2b-up",
        "ghost2b-tr",

        "ghost2c-right",
        "ghost2c-br",
        "ghost2c-down",
        "ghost2c-bl",
        "ghost2c-left",
        "ghost2c-tl",
        "ghost2c-up",
        "ghost2c-tr",

        "ghost2d-right",
        "ghost2d-br",
        "ghost2d-down",
        "ghost2d-bl",
        "ghost2d-left",
        "ghost2d-tl",
        "ghost2d-up",
        "ghost2d-tr",

        // row 22
        "ghost3a-right",
        "ghost3a-br",
        "ghost3a-down",
        "ghost3a-bl",
        "ghost3a-left",
        "ghost3a-tl",
        "ghost3a-up",
        "ghost3a-tr",

        "ghost3b-right",
        "ghost3b-br",
        "ghost3b-down",
        "ghost3b-bl",
        "ghost3b-left",
        "ghost3b-tl",
        "ghost3b-up",
        "ghost3b-tr",

        "ghost3c-right",
        "ghost3c-br",
        "ghost3c-down",
        "ghost3c-bl",
        "ghost3c-left",
        "ghost3c-tl",
        "ghost3c-up",
        "ghost3c-tr",

        "ghost3d-right",
        "ghost3d-br",
        "ghost3d-down",
        "ghost3d-bl",
        "ghost3d-left",
        "ghost3d-tl",
        "ghost3d-up",
        "ghost3d-tr",

        // row 23
        "monk1-right",
        "monk1-br",
        "monk1-down",
        "monk1-bl",
        "monk1-left",
        "monk1-tl",
        "monk1-up",
        "monk1-tr",
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        // row 24
        "demon1-right",
        "demon1-br",
        "demon1-down",
        "demon1-bl",
        "demon1-left",
        "demon1-tl",
        "demon1-up",
        "demon1-tr",
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        // row 25
        "lizard1-right",
        "lizard1-br",
        "lizard1-down",
        "lizard1-bl",
        "lizard1-left",
        "lizard1-tl",
        "lizard1-up",
        "lizard1-tr",
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        // row 26
        "warrior-right",
        "warrior-br",
        "warrior-down",
        "warrior-bl",
        "warrior-left",
        "warrior-tl",
        "warrior-up",
        "warrior-tr",
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        // row 27
        "valkyrie-right",
        "valkyrie-br",
        "valkyrie-down",
        "valkyrie-bl",
        "valkyrie-left",
        "valkyrie-tl",
        "valkyrie-up",
        "valkyrie-tr",
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        // row 28
        "death1-right",
        "death1-br",
        "death1-down",
        "death1-bl",
        "death1-left",
        "death1-tl",
        "death1-up",
        "death1-tr",
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        // row 29
        "wizard-right",
        "wizard-br",
        "wizard-down",
        "wizard-bl",
        "wizard-left",
        "wizard-tl",
        "wizard-up",
        "wizard-tr",
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        // row 30
        "elf-right",
        "elf-br",
        "elf-down",
        "elf-bl",
        "elf-left",
        "elf-tl",
        "elf-up",
        "elf-tr",
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        // row 31
        "dwarf-right",
        "dwarf-br",
        "dwarf-down",
        "dwarf-bl",
        "dwarf-left",
        "dwarf-tl",
        "dwarf-up",
        "dwarf-tr",
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
    };

    tile_load_arr("sprites_small", TILE_WIDTH, TILE_HEIGHT,
                  ARRAY_SIZE(small_tiles), small_tiles);

    thing_template_load(&THING_WALL, "data/things/wall");
    thing_template_load(&THING_WALL2, "data/things/wall2");
    thing_template_load(&THING_WALL3, "data/things/wall3");
    thing_template_load(&THING_WALL4, "data/things/wall4");
    thing_template_load(&THING_WALL5, "data/things/wall5");
    thing_template_load(&THING_DOOR, "data/things/door");
    thing_template_load(&THING_NOENTRY, "data/things/noentry");
    thing_template_load(&THING_FOOD, "data/things/food");
    thing_template_load(&THING_FLOOR, "data/things/floor");
    thing_template_load(&THING_FLOOR2, "data/things/floor2");
    thing_template_load(&THING_FLOOR3, "data/things/floor3");
    thing_template_load(&THING_FLOOR4, "data/things/floor4");
    thing_template_load(&THING_FLOOR5, "data/things/floor5");
    thing_template_load(&THING_FLOOR6, "data/things/floor6");
    thing_template_load(&THING_WARRIOR, "data/things/warrior");
    thing_template_load(&THING_VALKYRIE, "data/things/valkyrie");
    thing_template_load(&THING_WIZARD, "data/things/wizard");
    thing_template_load(&THING_ELF, "data/things/elf");
    thing_template_load(&THING_DWARF, "data/things/dwarf");
    thing_template_load(&THING_GHOST1, "data/things/ghost1");
    thing_template_load(&THING_GHOST2, "data/things/ghost2");
    thing_template_load(&THING_GHOST3, "data/things/ghost3");
    thing_template_load(&THING_DEMON, "data/things/demon");
    thing_template_load(&THING_MONK, "data/things/monk");
    thing_template_load(&THING_LIZARD, "data/things/lizard");
    thing_template_load(&THING_DEATH, "data/things/death");
    thing_template_load(&THING_PLANT, "data/things/plant");
    thing_template_load(&THING_SEEDPOD, "data/things/seedpod");
    thing_template_load(&THING_PIPE, "data/things/pipe");
    thing_template_load(&THING_EXPLOSION, "data/things/explosion");
    thing_template_load(&THING_BOMB, "data/things/bomb");
    thing_template_load(&THING_SPAM, "data/things/spam");
    thing_template_load(&THING_POTION1, "data/things/potion1");
    thing_template_load(&THING_POTION2, "data/things/potion2");
    thing_template_load(&THING_POTION3, "data/things/potion3");
    thing_template_load(&THING_POTION4, "data/things/potion4");
    thing_template_load(&THING_POTION5, "data/things/potion5");
    thing_template_load(&THING_WATER1, "data/things/water1");
    thing_template_load(&THING_WATER2, "data/things/water2");
    thing_template_load(&THING_MASK1, "data/things/mask1");
    thing_template_load(&THING_MASK2, "data/things/mask2");
    thing_template_load(&THING_MASK3, "data/things/mask3");
    thing_template_load(&THING_GEM1, "data/things/gem1");
    thing_template_load(&THING_GEM2, "data/things/gem2");
    thing_template_load(&THING_GEM3, "data/things/gem3");
    thing_template_load(&THING_GEM4, "data/things/gem4");
    thing_template_load(&THING_GEM5, "data/things/gem5");
    thing_template_load(&THING_GEM6, "data/things/gem6");
    thing_template_load(&THING_GEM7, "data/things/gem7");
    thing_template_load(&THING_EXIT1, "data/things/exit1");
    thing_template_load(&THING_EXIT2, "data/things/exit2");
    thing_template_load(&THING_EXIT3, "data/things/exit3");
    thing_template_load(&THING_EXIT4, "data/things/exit4");
    thing_template_load(&THING_EXIT5, "data/things/exit5");
    thing_template_load(&THING_EXIT6, "data/things/exit6");
    thing_template_load(&THING_EXIT7, "data/things/exit7");
    thing_template_load(&THING_GENERATOR1, "data/things/generator1");
    thing_template_load(&THING_GENERATOR2, "data/things/generator2");
    thing_template_load(&THING_GENERATOR3, "data/things/generator3");
    thing_template_load(&THING_GENERATOR4, "data/things/generator4");
    thing_template_load(&THING_GENERATOR5, "data/things/generator5");
    thing_template_load(&THING_GENERATOR6, "data/things/generator6");
    thing_template_load(&THING_GENERATOR7, "data/things/generator7");
    thing_template_load(&THING_KEYS1, "data/things/keys1");
    thing_template_load(&THING_KEYS2, "data/things/keys2");
    thing_template_load(&THING_KEYS3, "data/things/keys3");
    thing_template_load(&THING_COINS1, "data/things/coins1");
    thing_template_load(&THING_AMULET1, "data/things/amulet1");
    thing_template_load(&THING_CHEST1, "data/things/chest1");
    thing_template_load(&THING_ARROW, "data/things/arrow");
    thing_template_load(&THING_FIREBALL, "data/things/fireball");

    return (true);
}

boolean resource_init (void)
{
    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)resource_init_1,
                          0, "resource_init_1");
    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)resource_init_2,
                          0, "resource_init_2");
    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)resource_init_3,
                          0, "resource_init_3");
    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)resource_init_4,
                          0, "resource_init_4");
    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)resource_init_5,
                          0, "resource_init_5");
    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)resource_init_6,
                          0, "resource_init_6");
    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)resource_init_7,
                          0, "resource_init_7");
    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)resource_init_8,
                          0, "resource_init_8");
    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)resource_init_9,
                          0, "resource_init_9");
    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)resource_init_10,
                          0, "resource_init_10");
    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)resource_init_11,
                          0, "resource_init_11");
    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)resource_init_12,
                          0, "resource_init_12");
    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)resource_init_13,
                          0, "resource_init_13");
    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)resource_init_14,
                          0, "resource_init_14");
    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)resource_init_15,
                          0, "resource_init_15");
    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)resource_init_16,
                          0, "resource_init_16");
    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)resource_init_17,
                          0, "resource_init_17");
    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)resource_init_18,
                          0, "resource_init_18");
    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)resource_init_19,
                          0, "resource_init_19");
    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)resource_init_20,
                          0, "resource_init_20");
    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)resource_init_21,
                          0, "resource_init_21");
    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)resource_init_22,
                          0, "resource_init_22");
    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)resource_init_23,
                          0, "resource_init_23");

    return (true);
}
