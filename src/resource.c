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
#include "thing_template.h"
#include "init_fn.h"

thing_templatep BRICK_0;
uint16_t BRICK_0_ID;

thing_templatep BRICK_1;
uint16_t BRICK_1_ID;

thing_templatep BRICK_2;
uint16_t BRICK_2_ID;

thing_templatep BRICK_3;
uint16_t BRICK_3_ID;

thing_templatep BRICK_4;
uint16_t BRICK_4_ID;

thing_templatep BRICK_5;
uint16_t BRICK_5_ID;

thing_templatep BRICK_6;
uint16_t BRICK_6_ID;

thing_templatep BRICK_7;
uint16_t BRICK_7_ID;

thing_templatep BRICK_8;
uint16_t BRICK_8_ID;

thing_templatep ROCK_0;
uint16_t ROCK_0_ID;

thing_templatep ROCK_1;
uint16_t ROCK_1_ID;

thing_templatep LAVA_0;
uint16_t LAVA_0_ID;

thing_templatep WATER_0;
uint16_t WATER_0_ID;

static boolean resource_init_1 (void *context)
{
    return (tex_load("data/gfx/gorynlich.png",
                     "gorynlich") != 0);
}

static boolean resource_init_2 (void *context)
{
    return (true);
}

static boolean resource_init_3 (void *context)
{
    return (true);
}

static boolean resource_init_4 (void *context)
{
    return (true);
}

static boolean resource_init_5 (void *context)
{
    return (true);
}

static boolean resource_init_6 (void *context)
{
    return (true);
}

static boolean resource_init_7 (void *context)
{
    return (true);
}

static boolean resource_init_8 (void *context)
{
    return (true);
}

static boolean resource_init_9 (void *context)
{
    return (true);
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
        "space",
        "rock_0_0",
        "rock_0_1",
        "rock_0_2",
        "rock_0_3",
        "rock_0_4",
        "rock_0_5",
        "rock_0_6",
        "rock_0_is_join_node",
        "rock_0_is_join_left",
        "rock_0_is_join_bot",
        "rock_0_is_join_right",
        "rock_0_is_join_top",
        "rock_0_is_join_horiz",
        "rock_0_is_join_vert",
        "rock_0_is_join_tl2",
        "rock_0_is_join_bl2",
        "rock_0_is_join_br2",
        "rock_0_is_join_tr2",
        "rock_0_is_join_tl",
        "rock_0_is_join_bl",
        "rock_0_is_join_br",
        "rock_0_is_join_tr",
        "rock_0_is_join_t270_3",
        "rock_0_is_join_t180_3",
        "rock_0_is_join_t90_3",
        "rock_0_is_join_t_3",
        "rock_0_is_join_t270",
        "rock_0_is_join_t180",
        "rock_0_is_join_t90",
        "rock_0_is_join_t",
        "rock_0_is_join_t270_2",
        "rock_0_is_join_t180_2",
        "rock_0_is_join_t90_2",
        "rock_0_is_join_t_2",
        "rock_0_is_join_t270_1",
        "rock_0_is_join_t180_1",
        "rock_0_is_join_t90_1",
        "rock_0_is_join_t_1",
        "rock_0_is_join_x",
        "rock_0_is_join_x1",
        "rock_0_is_join_x1_270",
        "rock_0_is_join_x1_180",
        "rock_0_is_join_x1_90",
        "rock_0_is_join_x2",
        "rock_0_is_join_x2_270",
        "rock_0_is_join_x2_180",
        "rock_0_is_join_x2_90",
        "rock_0_is_join_x3",
        "rock_0_is_join_x3_180",
        "rock_0_is_join_x4",
        "rock_0_is_join_x4_270",
        "rock_0_is_join_x4_180",
        "rock_0_is_join_x4_90",
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
        // rock darker bricks 
        // ----------------------------------------------------
        "rock_1_0",
        "rock_1_1",
        "rock_1_2",
        "rock_1_3",
        "rock_1_4",
        "rock_1_5",
        "rock_1_6",
        "rock_1_7",
        "rock_1_is_join_node",
        "rock_1_is_join_left",
        "rock_1_is_join_bot",
        "rock_1_is_join_right",
        "rock_1_is_join_top",
        "rock_1_is_join_horiz",
        "rock_1_is_join_vert",
        "rock_1_is_join_tl2",
        "rock_1_is_join_bl2",
        "rock_1_is_join_br2",
        "rock_1_is_join_tr2",
        "rock_1_is_join_tl",
        "rock_1_is_join_bl",
        "rock_1_is_join_br",
        "rock_1_is_join_tr",
        "rock_1_is_join_t270_3",
        "rock_1_is_join_t180_3",
        "rock_1_is_join_t90_3",
        "rock_1_is_join_t_3",
        "rock_1_is_join_t270",
        "rock_1_is_join_t180",
        "rock_1_is_join_t90",
        "rock_1_is_join_t",
        "rock_1_is_join_t270_2",
        "rock_1_is_join_t180_2",
        "rock_1_is_join_t90_2",
        "rock_1_is_join_t_2",
        "rock_1_is_join_t270_1",
        "rock_1_is_join_t180_1",
        "rock_1_is_join_t90_1",
        "rock_1_is_join_t_1",
        "rock_1_is_join_x",
        "rock_1_is_join_x1",
        "rock_1_is_join_x1_270",
        "rock_1_is_join_x1_180",
        "rock_1_is_join_x1_90",
        "rock_1_is_join_x2",
        "rock_1_is_join_x2_270",
        "rock_1_is_join_x2_180",
        "rock_1_is_join_x2_90",
        "rock_1_is_join_x3",
        "rock_1_is_join_x3_180",
        "rock_1_is_join_x4",
        "rock_1_is_join_x4_270",
        "rock_1_is_join_x4_180",
        "rock_1_is_join_x4_90",
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
        // gray bricks ----------------------------------------------------
        "brick_0_0",
        "brick_0_1",
        "brick_0_2",
        "brick_0_3",
        "brick_0_4",
        "brick_0_5",
        "brick_0_6",
        "brick_0_7",
        "brick_0_is_join_node",
        "brick_0_is_join_left",
        "brick_0_is_join_bot",
        "brick_0_is_join_right",
        "brick_0_is_join_top",
        "brick_0_is_join_horiz",
        "brick_0_is_join_vert",
        "brick_0_is_join_tl2",
        "brick_0_is_join_bl2",
        "brick_0_is_join_br2",
        "brick_0_is_join_tr2",
        "brick_0_is_join_tl",
        "brick_0_is_join_bl",
        "brick_0_is_join_br",
        "brick_0_is_join_tr",
        "brick_0_is_join_t270_3",
        "brick_0_is_join_t180_3",
        "brick_0_is_join_t90_3",
        "brick_0_is_join_t_3",
        "brick_0_is_join_t270",
        "brick_0_is_join_t180",
        "brick_0_is_join_t90",
        "brick_0_is_join_t",
        "brick_0_is_join_t270_2",
        "brick_0_is_join_t180_2",
        "brick_0_is_join_t90_2",
        "brick_0_is_join_t_2",
        "brick_0_is_join_t270_1",
        "brick_0_is_join_t180_1",
        "brick_0_is_join_t90_1",
        "brick_0_is_join_t_1",
        "brick_0_is_join_x",
        "brick_0_is_join_x1",
        "brick_0_is_join_x1_270",
        "brick_0_is_join_x1_180",
        "brick_0_is_join_x1_90",
        "brick_0_is_join_x2",
        "brick_0_is_join_x2_270",
        "brick_0_is_join_x2_180",
        "brick_0_is_join_x2_90",
        "brick_0_is_join_x3",
        "brick_0_is_join_x3_180",
        "brick_0_is_join_x4",
        "brick_0_is_join_x4_270",
        "brick_0_is_join_x4_180",
        "brick_0_is_join_x4_90",
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
        // cyan bricks ----------------------------------------------------
        "brick_1_0",
        "brick_1_1",
        "brick_1_2",
        "brick_1_3",
        "brick_1_4",
        "brick_1_5",
        "brick_1_6",
        "brick_1_7",
        "brick_1_is_join_node",
        "brick_1_is_join_left",
        "brick_1_is_join_bot",
        "brick_1_is_join_right",
        "brick_1_is_join_top",
        "brick_1_is_join_horiz",
        "brick_1_is_join_vert",
        "brick_1_is_join_tl2",
        "brick_1_is_join_bl2",
        "brick_1_is_join_br2",
        "brick_1_is_join_tr2",
        "brick_1_is_join_tl",
        "brick_1_is_join_bl",
        "brick_1_is_join_br",
        "brick_1_is_join_tr",
        "brick_1_is_join_t270_3",
        "brick_1_is_join_t180_3",
        "brick_1_is_join_t90_3",
        "brick_1_is_join_t_3",
        "brick_1_is_join_t270",
        "brick_1_is_join_t180",
        "brick_1_is_join_t90",
        "brick_1_is_join_t",
        "brick_1_is_join_t270_2",
        "brick_1_is_join_t180_2",
        "brick_1_is_join_t90_2",
        "brick_1_is_join_t_2",
        "brick_1_is_join_t270_1",
        "brick_1_is_join_t180_1",
        "brick_1_is_join_t90_1",
        "brick_1_is_join_t_1",
        "brick_1_is_join_x",
        "brick_1_is_join_x1",
        "brick_1_is_join_x1_270",
        "brick_1_is_join_x1_180",
        "brick_1_is_join_x1_90",
        "brick_1_is_join_x2",
        "brick_1_is_join_x2_270",
        "brick_1_is_join_x2_180",
        "brick_1_is_join_x2_90",
        "brick_1_is_join_x3",
        "brick_1_is_join_x3_180",
        "brick_1_is_join_x4",
        "brick_1_is_join_x4_270",
        "brick_1_is_join_x4_180",
        "brick_1_is_join_x4_90",
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
        // blue bricks ----------------------------------------------------
        "brick_2_0",
        "brick_2_1",
        "brick_2_2",
        "brick_2_3",
        "brick_2_4",
        "brick_2_5",
        "brick_2_6",
        "brick_2_7",
        "brick_2_is_join_node",
        "brick_2_is_join_left",
        "brick_2_is_join_bot",
        "brick_2_is_join_right",
        "brick_2_is_join_top",
        "brick_2_is_join_horiz",
        "brick_2_is_join_vert",
        "brick_2_is_join_tl2",
        "brick_2_is_join_bl2",
        "brick_2_is_join_br2",
        "brick_2_is_join_tr2",
        "brick_2_is_join_tl",
        "brick_2_is_join_bl",
        "brick_2_is_join_br",
        "brick_2_is_join_tr",
        "brick_2_is_join_t270_3",
        "brick_2_is_join_t180_3",
        "brick_2_is_join_t90_3",
        "brick_2_is_join_t_3",
        "brick_2_is_join_t270",
        "brick_2_is_join_t180",
        "brick_2_is_join_t90",
        "brick_2_is_join_t",
        "brick_2_is_join_t270_2",
        "brick_2_is_join_t180_2",
        "brick_2_is_join_t90_2",
        "brick_2_is_join_t_2",
        "brick_2_is_join_t270_1",
        "brick_2_is_join_t180_1",
        "brick_2_is_join_t90_1",
        "brick_2_is_join_t_1",
        "brick_2_is_join_x",
        "brick_2_is_join_x1",
        "brick_2_is_join_x1_270",
        "brick_2_is_join_x1_180",
        "brick_2_is_join_x1_90",
        "brick_2_is_join_x2",
        "brick_2_is_join_x2_270",
        "brick_2_is_join_x2_180",
        "brick_2_is_join_x2_90",
        "brick_2_is_join_x3",
        "brick_2_is_join_x3_180",
        "brick_2_is_join_x4",
        "brick_2_is_join_x4_270",
        "brick_2_is_join_x4_180",
        "brick_2_is_join_x4_90",
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
        // green bricks ----------------------------------------------------
        "brick_3_0",
        "brick_3_1",
        "brick_3_2",
        "brick_3_3",
        "brick_3_4",
        "brick_3_5",
        "brick_3_6",
        "brick_3_7",
        "brick_3_is_join_node",
        "brick_3_is_join_left",
        "brick_3_is_join_bot",
        "brick_3_is_join_right",
        "brick_3_is_join_top",
        "brick_3_is_join_horiz",
        "brick_3_is_join_vert",
        "brick_3_is_join_tl2",
        "brick_3_is_join_bl2",
        "brick_3_is_join_br2",
        "brick_3_is_join_tr2",
        "brick_3_is_join_tl",
        "brick_3_is_join_bl",
        "brick_3_is_join_br",
        "brick_3_is_join_tr",
        "brick_3_is_join_t270_3",
        "brick_3_is_join_t180_3",
        "brick_3_is_join_t90_3",
        "brick_3_is_join_t_3",
        "brick_3_is_join_t270",
        "brick_3_is_join_t180",
        "brick_3_is_join_t90",
        "brick_3_is_join_t",
        "brick_3_is_join_t270_2",
        "brick_3_is_join_t180_2",
        "brick_3_is_join_t90_2",
        "brick_3_is_join_t_2",
        "brick_3_is_join_t270_1",
        "brick_3_is_join_t180_1",
        "brick_3_is_join_t90_1",
        "brick_3_is_join_t_1",
        "brick_3_is_join_x",
        "brick_3_is_join_x1",
        "brick_3_is_join_x1_270",
        "brick_3_is_join_x1_180",
        "brick_3_is_join_x1_90",
        "brick_3_is_join_x2",
        "brick_3_is_join_x2_270",
        "brick_3_is_join_x2_180",
        "brick_3_is_join_x2_90",
        "brick_3_is_join_x3",
        "brick_3_is_join_x3_180",
        "brick_3_is_join_x4",
        "brick_3_is_join_x4_270",
        "brick_3_is_join_x4_180",
        "brick_3_is_join_x4_90",
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
        // orange bricks ----------------------------------------------------
        "brick_4_0",
        "brick_4_1",
        "brick_4_2",
        "brick_4_3",
        "brick_4_4",
        "brick_4_5",
        "brick_4_6",
        "brick_4_7",
        "brick_4_is_join_node",
        "brick_4_is_join_left",
        "brick_4_is_join_bot",
        "brick_4_is_join_right",
        "brick_4_is_join_top",
        "brick_4_is_join_horiz",
        "brick_4_is_join_vert",
        "brick_4_is_join_tl2",
        "brick_4_is_join_bl2",
        "brick_4_is_join_br2",
        "brick_4_is_join_tr2",
        "brick_4_is_join_tl",
        "brick_4_is_join_bl",
        "brick_4_is_join_br",
        "brick_4_is_join_tr",
        "brick_4_is_join_t270_3",
        "brick_4_is_join_t180_3",
        "brick_4_is_join_t90_3",
        "brick_4_is_join_t_3",
        "brick_4_is_join_t270",
        "brick_4_is_join_t180",
        "brick_4_is_join_t90",
        "brick_4_is_join_t",
        "brick_4_is_join_t270_2",
        "brick_4_is_join_t180_2",
        "brick_4_is_join_t90_2",
        "brick_4_is_join_t_2",
        "brick_4_is_join_t270_1",
        "brick_4_is_join_t180_1",
        "brick_4_is_join_t90_1",
        "brick_4_is_join_t_1",
        "brick_4_is_join_x",
        "brick_4_is_join_x1",
        "brick_4_is_join_x1_270",
        "brick_4_is_join_x1_180",
        "brick_4_is_join_x1_90",
        "brick_4_is_join_x2",
        "brick_4_is_join_x2_270",
        "brick_4_is_join_x2_180",
        "brick_4_is_join_x2_90",
        "brick_4_is_join_x3",
        "brick_4_is_join_x3_180",
        "brick_4_is_join_x4",
        "brick_4_is_join_x4_270",
        "brick_4_is_join_x4_180",
        "brick_4_is_join_x4_90",
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
        // purple bricks ----------------------------------------------------
        "brick_5_0",
        "brick_5_1",
        "brick_5_2",
        "brick_5_3",
        "brick_5_4",
        "brick_5_5",
        "brick_5_6",
        "brick_5_7",
        "brick_5_is_join_node",
        "brick_5_is_join_left",
        "brick_5_is_join_bot",
        "brick_5_is_join_right",
        "brick_5_is_join_top",
        "brick_5_is_join_horiz",
        "brick_5_is_join_vert",
        "brick_5_is_join_tl2",
        "brick_5_is_join_bl2",
        "brick_5_is_join_br2",
        "brick_5_is_join_tr2",
        "brick_5_is_join_tl",
        "brick_5_is_join_bl",
        "brick_5_is_join_br",
        "brick_5_is_join_tr",
        "brick_5_is_join_t270_3",
        "brick_5_is_join_t180_3",
        "brick_5_is_join_t90_3",
        "brick_5_is_join_t_3",
        "brick_5_is_join_t270",
        "brick_5_is_join_t180",
        "brick_5_is_join_t90",
        "brick_5_is_join_t",
        "brick_5_is_join_t270_2",
        "brick_5_is_join_t180_2",
        "brick_5_is_join_t90_2",
        "brick_5_is_join_t_2",
        "brick_5_is_join_t270_1",
        "brick_5_is_join_t180_1",
        "brick_5_is_join_t90_1",
        "brick_5_is_join_t_1",
        "brick_5_is_join_x",
        "brick_5_is_join_x1",
        "brick_5_is_join_x1_270",
        "brick_5_is_join_x1_180",
        "brick_5_is_join_x1_90",
        "brick_5_is_join_x2",
        "brick_5_is_join_x2_270",
        "brick_5_is_join_x2_180",
        "brick_5_is_join_x2_90",
        "brick_5_is_join_x3",
        "brick_5_is_join_x3_180",
        "brick_5_is_join_x4",
        "brick_5_is_join_x4_270",
        "brick_5_is_join_x4_180",
        "brick_5_is_join_x4_90",
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
        // yellow bricks ----------------------------------------------------
        "brick_6_0",
        "brick_6_1",
        "brick_6_2",
        "brick_6_3",
        "brick_6_4",
        "brick_6_5",
        "brick_6_6",
        "brick_6_7",
        "brick_6_is_join_node",
        "brick_6_is_join_left",
        "brick_6_is_join_bot",
        "brick_6_is_join_right",
        "brick_6_is_join_top",
        "brick_6_is_join_horiz",
        "brick_6_is_join_vert",
        "brick_6_is_join_tl2",
        "brick_6_is_join_bl2",
        "brick_6_is_join_br2",
        "brick_6_is_join_tr2",
        "brick_6_is_join_tl",
        "brick_6_is_join_bl",
        "brick_6_is_join_br",
        "brick_6_is_join_tr",
        "brick_6_is_join_t270_3",
        "brick_6_is_join_t180_3",
        "brick_6_is_join_t90_3",
        "brick_6_is_join_t_3",
        "brick_6_is_join_t270",
        "brick_6_is_join_t180",
        "brick_6_is_join_t90",
        "brick_6_is_join_t",
        "brick_6_is_join_t270_2",
        "brick_6_is_join_t180_2",
        "brick_6_is_join_t90_2",
        "brick_6_is_join_t_2",
        "brick_6_is_join_t270_1",
        "brick_6_is_join_t180_1",
        "brick_6_is_join_t90_1",
        "brick_6_is_join_t_1",
        "brick_6_is_join_x",
        "brick_6_is_join_x1",
        "brick_6_is_join_x1_270",
        "brick_6_is_join_x1_180",
        "brick_6_is_join_x1_90",
        "brick_6_is_join_x2",
        "brick_6_is_join_x2_270",
        "brick_6_is_join_x2_180",
        "brick_6_is_join_x2_90",
        "brick_6_is_join_x3",
        "brick_6_is_join_x3_180",
        "brick_6_is_join_x4",
        "brick_6_is_join_x4_270",
        "brick_6_is_join_x4_180",
        "brick_6_is_join_x4_90",
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
        // pink bricks ----------------------------------------------------
        "brick_7_0",
        "brick_7_1",
        "brick_7_2",
        "brick_7_3",
        "brick_7_4",
        "brick_7_5",
        "brick_7_6",
        "brick_7_7",
        "brick_7_is_join_node",
        "brick_7_is_join_left",
        "brick_7_is_join_bot",
        "brick_7_is_join_right",
        "brick_7_is_join_top",
        "brick_7_is_join_horiz",
        "brick_7_is_join_vert",
        "brick_7_is_join_tl2",
        "brick_7_is_join_bl2",
        "brick_7_is_join_br2",
        "brick_7_is_join_tr2",
        "brick_7_is_join_tl",
        "brick_7_is_join_bl",
        "brick_7_is_join_br",
        "brick_7_is_join_tr",
        "brick_7_is_join_t270_3",
        "brick_7_is_join_t180_3",
        "brick_7_is_join_t90_3",
        "brick_7_is_join_t_3",
        "brick_7_is_join_t270",
        "brick_7_is_join_t180",
        "brick_7_is_join_t90",
        "brick_7_is_join_t",
        "brick_7_is_join_t270_2",
        "brick_7_is_join_t180_2",
        "brick_7_is_join_t90_2",
        "brick_7_is_join_t_2",
        "brick_7_is_join_t270_1",
        "brick_7_is_join_t180_1",
        "brick_7_is_join_t90_1",
        "brick_7_is_join_t_1",
        "brick_7_is_join_x",
        "brick_7_is_join_x1",
        "brick_7_is_join_x1_270",
        "brick_7_is_join_x1_180",
        "brick_7_is_join_x1_90",
        "brick_7_is_join_x2",
        "brick_7_is_join_x2_270",
        "brick_7_is_join_x2_180",
        "brick_7_is_join_x2_90",
        "brick_7_is_join_x3",
        "brick_7_is_join_x3_180",
        "brick_7_is_join_x4",
        "brick_7_is_join_x4_270",
        "brick_7_is_join_x4_180",
        "brick_7_is_join_x4_90",
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
        // orange bricks ----------------------------------------------------
        "brick_8_0",
        "brick_8_1",
        "brick_8_2",
        "brick_8_3",
        "brick_8_4",
        "brick_8_5",
        "brick_8_6",
        "brick_8_7",
        "brick_8_is_join_node",
        "brick_8_is_join_left",
        "brick_8_is_join_bot",
        "brick_8_is_join_right",
        "brick_8_is_join_top",
        "brick_8_is_join_horiz",
        "brick_8_is_join_vert",
        "brick_8_is_join_tl2",
        "brick_8_is_join_bl2",
        "brick_8_is_join_br2",
        "brick_8_is_join_tr2",
        "brick_8_is_join_tl",
        "brick_8_is_join_bl",
        "brick_8_is_join_br",
        "brick_8_is_join_tr",
        "brick_8_is_join_t270_3",
        "brick_8_is_join_t180_3",
        "brick_8_is_join_t90_3",
        "brick_8_is_join_t_3",
        "brick_8_is_join_t270",
        "brick_8_is_join_t180",
        "brick_8_is_join_t90",
        "brick_8_is_join_t",
        "brick_8_is_join_t270_2",
        "brick_8_is_join_t180_2",
        "brick_8_is_join_t90_2",
        "brick_8_is_join_t_2",
        "brick_8_is_join_t270_1",
        "brick_8_is_join_t180_1",
        "brick_8_is_join_t90_1",
        "brick_8_is_join_t_1",
        "brick_8_is_join_x",
        "brick_8_is_join_x1",
        "brick_8_is_join_x1_270",
        "brick_8_is_join_x1_180",
        "brick_8_is_join_x1_90",
        "brick_8_is_join_x2",
        "brick_8_is_join_x2_270",
        "brick_8_is_join_x2_180",
        "brick_8_is_join_x2_90",
        "brick_8_is_join_x3",
        "brick_8_is_join_x3_180",
        "brick_8_is_join_x4",
        "brick_8_is_join_x4_270",
        "brick_8_is_join_x4_180",
        "brick_8_is_join_x4_90",
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
        // lava -----------------------------------------------------------
        "lava_0_0",
        "lava_0_1",
        "lava_0_2",
        "lava_0_3",
        "lava_0_4",
        "lava_0_5",
        "lava_0_6",
        "lava_0_7",
        "lava_0_is_join_node",
        "lava_0_is_join_left",
        "lava_0_is_join_bot",
        "lava_0_is_join_right",
        "lava_0_is_join_top",
        "lava_0_is_join_horiz",
        "lava_0_is_join_vert",
        "lava_0_is_join_tl2",
        "lava_0_is_join_bl2",
        "lava_0_is_join_br2",
        "lava_0_is_join_tr2",
        "lava_0_is_join_tl",
        "lava_0_is_join_bl",
        "lava_0_is_join_br",
        "lava_0_is_join_tr",
        "lava_0_is_join_t270_3",
        "lava_0_is_join_t180_3",
        "lava_0_is_join_t90_3",
        "lava_0_is_join_t_3",
        "lava_0_is_join_t270",
        "lava_0_is_join_t180",
        "lava_0_is_join_t90",
        "lava_0_is_join_t",
        "lava_0_is_join_t270_2",
        "lava_0_is_join_t180_2",
        "lava_0_is_join_t90_2",
        "lava_0_is_join_t_2",
        "lava_0_is_join_t270_1",
        "lava_0_is_join_t180_1",
        "lava_0_is_join_t90_1",
        "lava_0_is_join_t_1",
        "lava_0_is_join_x",
        "lava_0_is_join_x1",
        "lava_0_is_join_x1_270",
        "lava_0_is_join_x1_180",
        "lava_0_is_join_x1_90",
        "lava_0_is_join_x2",
        "lava_0_is_join_x2_270",
        "lava_0_is_join_x2_180",
        "lava_0_is_join_x2_90",
        "lava_0_is_join_x3",
        "lava_0_is_join_x3_180",
        "lava_0_is_join_x4",
        "lava_0_is_join_x4_270",
        "lava_0_is_join_x4_180",
        "lava_0_is_join_x4_90",
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
        // water -----------------------------------------------------------
        "water_0_0",
        "water_0_1",
        "water_0_2",
        "water_0_3",
        "water_0_4",
        "water_0_5",
        "water_0_6",
        "water_0_7",
        "water_0_is_join_node",
        "water_0_is_join_left",
        "water_0_is_join_bot",
        "water_0_is_join_right",
        "water_0_is_join_top",
        "water_0_is_join_horiz",
        "water_0_is_join_vert",
        "water_0_is_join_tl2",
        "water_0_is_join_bl2",
        "water_0_is_join_br2",
        "water_0_is_join_tr2",
        "water_0_is_join_tl",
        "water_0_is_join_bl",
        "water_0_is_join_br",
        "water_0_is_join_tr",
        "water_0_is_join_t270_3",
        "water_0_is_join_t180_3",
        "water_0_is_join_t90_3",
        "water_0_is_join_t_3",
        "water_0_is_join_t270",
        "water_0_is_join_t180",
        "water_0_is_join_t90",
        "water_0_is_join_t",
        "water_0_is_join_t270_2",
        "water_0_is_join_t180_2",
        "water_0_is_join_t90_2",
        "water_0_is_join_t_2",
        "water_0_is_join_t270_1",
        "water_0_is_join_t180_1",
        "water_0_is_join_t90_1",
        "water_0_is_join_t_1",
        "water_0_is_join_x",
        "water_0_is_join_x1",
        "water_0_is_join_x1_270",
        "water_0_is_join_x1_180",
        "water_0_is_join_x1_90",
        "water_0_is_join_x2",
        "water_0_is_join_x2_270",
        "water_0_is_join_x2_180",
        "water_0_is_join_x2_90",
        "water_0_is_join_x3",
        "water_0_is_join_x3_180",
        "water_0_is_join_x4",
        "water_0_is_join_x4_270",
        "water_0_is_join_x4_180",
        "water_0_is_join_x4_90",
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
        // ----------------------------------------------------------------
    };

    tile_load_arr("sprites_small", TILE_WIDTH, TILE_HEIGHT,
                  ARRAY_SIZE(small_tiles), small_tiles);

    BRICK_0 = thing_template_load("data/things/brick_0");
    BRICK_0_ID = thing_template_to_id(BRICK_0);

    BRICK_1 = thing_template_load("data/things/brick_1");
    BRICK_1_ID = thing_template_to_id(BRICK_1);

    BRICK_2 = thing_template_load("data/things/brick_2");
    BRICK_2_ID = thing_template_to_id(BRICK_2);

    BRICK_3 = thing_template_load("data/things/brick_3");
    BRICK_3_ID = thing_template_to_id(BRICK_3);

    BRICK_4 = thing_template_load("data/things/brick_4");
    BRICK_4_ID = thing_template_to_id(BRICK_4);

    BRICK_5 = thing_template_load("data/things/brick_5");
    BRICK_5_ID = thing_template_to_id(BRICK_5);

    BRICK_6 = thing_template_load("data/things/brick_6");
    BRICK_6_ID = thing_template_to_id(BRICK_6);

    BRICK_7 = thing_template_load("data/things/brick_7");
    BRICK_7_ID = thing_template_to_id(BRICK_7);

    BRICK_8 = thing_template_load("data/things/brick_8");
    BRICK_8_ID = thing_template_to_id(BRICK_8);

    ROCK_0 = thing_template_load("data/things/rock_0");
    ROCK_0_ID = thing_template_to_id(ROCK_0);

    ROCK_1 = thing_template_load("data/things/rock_1");
    ROCK_1_ID = thing_template_to_id(ROCK_1);

    LAVA_0 = thing_template_load("data/things/lava_0");
    LAVA_0_ID = thing_template_to_id(LAVA_0);

    WATER_0 = thing_template_load("data/things/water_0");
    WATER_0_ID = thing_template_to_id(WATER_0);

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
