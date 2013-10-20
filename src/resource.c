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

thing_templatep PLAYER_0;
uint16_t PLAYER_0_ID;

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
        0,
        0,
        0,
        0,
        0,
        0,
        // rock darker bricks 
        // ----------------------------------------------------
        "rock_1_0",
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
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        // lava -----------------------------------------------------------
        "lava_0_0",
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
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        // water -----------------------------------------------------------
        "water_0_0",
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
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        // ----------------------------------------------------------------
        "rock_bg_0",
        "rock_bg_1",
        "rock_bg_2",
        "rock_bg_3",
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
        0,
        0,
        0,
        0,
        0,
        0,
        // ----------------------------------------------------------------
        "man_walk_1",
        "man_walk_2",
        "man_walk_3",
        "man_walk_4",
        "man_walk_5",
        "man_walk_6",
        "man_walk_7",
        "man_walk_8",
        "man_jump_1",
        "man_idle_1",
        "man_idle_2",
        "man_idle_3",
        "man_idle_4",
        "man_idle_5",
        "man_idle_7",
        0, // 15
        0, // 16
        0, // 17
        0, // 18
        0, // 19
        0, // 20
        0, // 21
        0, // 22
        0, // 23
        0, // 24
        0, // 25
        0, // 26
        0, // 27
        0, // 28
        0, // 29
        0, // 30
        0, // 31
        0, // 32
        0, // 33
        0, // 34
        0, // 35
        0, // 36
        0, // 37
        0, // 38
        0, // 39
        0, // 40
        0, // 41
        0, // 42
        0, // 43
        0, // 44
        0, // 45
        0, // 46
        0, // 47
        0, // 48
        0, // 49
        0, // 50
        0, // 51
        0, // 52
        0, // 53
        0, // 54
        0, // 55
        0, // 56
        0, // 57
        0, // 58
        0, // 59
        0, // 60
        0, // 61
        0, // 62
        0, // 63
        // ----------------------------------------------------------------
        "man_dead_1",
        "man_dead_2",
        "man_dead_3",
        "man_dead_4",
        "man_dead_5",
        "man_dead_6",
        "man_dead_7",
        "man_dead_8",
        0, // 8
        0, // 9
        0, // 10
        0, // 11
        0, // 12
        0, // 13
        0, // 14
        0, // 15
        0, // 16
        0, // 17
        0, // 18
        0, // 19
        0, // 20
        0, // 21
        0, // 22
        0, // 23
        0, // 24
        0, // 25
        0, // 26
        0, // 27
        0, // 28
        0, // 29
        0, // 30
        0, // 31
        0, // 32
        0, // 33
        0, // 34
        0, // 35
        0, // 36
        0, // 37
        0, // 38
        0, // 39
        0, // 40
        0, // 41
        0, // 42
        0, // 43
        0, // 44
        0, // 45
        0, // 46
        0, // 47
        0, // 48
        0, // 49
        0, // 50
        0, // 51
        0, // 52
        0, // 53
        0, // 54
        0, // 55
        0, // 56
        0, // 57
        0, // 58
        0, // 59
        0, // 60
        0, // 61
        0, // 62
        0, // 63
        // ----------------------------------------------------------------
        "man_climb_1",
        "man_climb_2",
        "man_climb_3",
        "man_climb_4",
        "man_climb_5",
        "man_climb_6",
        "man_climb_7",
        "man_climb_8",
        "man_climb_9",
        "man_climb_10",
        "man_victory_1",
        "man_victory_2",
        0, // 12
        0, // 13
        0, // 14
        0, // 15
        0, // 16
        0, // 17
        0, // 18
        0, // 19
        0, // 20
        0, // 21
        0, // 22
        0, // 23
        0, // 24
        0, // 25
        0, // 26
        0, // 27
        0, // 28
        0, // 29
        0, // 30
        0, // 31
        0, // 32
        0, // 33
        0, // 34
        0, // 35
        0, // 36
        0, // 37
        0, // 38
        0, // 39
        0, // 40
        0, // 41
        0, // 42
        0, // 43
        0, // 44
        0, // 45
        0, // 46
        0, // 47
        0, // 48
        0, // 49
        0, // 50
        0, // 51
        0, // 52
        0, // 53
        0, // 54
        0, // 55
        0, // 56
        0, // 57
        0, // 58
        0, // 59
        0, // 60
        0, // 61
        0, // 62
        0, // 63
        // ----------------------------------------------------------------
        "mask_0",       // 0
        "gem_0",        // 1
        "boulder_0",    // 2
        "ladder_0",     // 3
        "ladder_1",     // 4
        "ladder_2",     // 5
        0, // 6
        0, // 7
        0, // 8
        0, // 9
        0, // 10
        0, // 11
        0, // 12
        0, // 13
        0, // 14
        0, // 15
        0, // 16
        0, // 17
        0, // 18
        0, // 19
        0, // 20
        0, // 21
        0, // 22
        0, // 23
        0, // 24
        0, // 25
        0, // 26
        0, // 27
        0, // 28
        0, // 29
        0, // 30
        0, // 31
        0, // 32
        0, // 33
        0, // 34
        0, // 35
        0, // 36
        0, // 37
        0, // 38
        0, // 39
        0, // 40
        0, // 41
        0, // 42
        0, // 43
        0, // 44
        0, // 45
        0, // 46
        0, // 47
        0, // 48
        0, // 49
        0, // 50
        0, // 51
        0, // 52
        0, // 53
        0, // 54
        0, // 55
        0, // 56
        0, // 57
        0, // 58
        0, // 59
        0, // 60
        0, // 61
        0, // 62
        0, // 63
        // ----------------------------------------------------------------
        "skeleton_0",   // 0
        "mask_1",       // 1
        "mask_2",       // 2
        "gem_1",        // 3
        "gem_2",        // 4
        "gem_3",        // 5
        "gem_4",        // 6
        0, // 7
        0, // 8
        0, // 9
        0, // 10
        0, // 11
        0, // 12
        0, // 13
        0, // 14
        0, // 15
        0, // 16
        0, // 17
        0, // 18
        0, // 19
        0, // 20
        0, // 21
        0, // 22
        0, // 23
        0, // 24
        0, // 25
        0, // 26
        0, // 27
        0, // 28
        0, // 29
        0, // 30
        0, // 31
        0, // 32
        0, // 33
        0, // 34
        0, // 35
        0, // 36
        0, // 37
        0, // 38
        0, // 39
        0, // 40
        0, // 41
        0, // 42
        0, // 43
        0, // 44
        0, // 45
        0, // 46
        0, // 47
        0, // 48
        0, // 49
        0, // 50
        0, // 51
        0, // 52
        0, // 53
        0, // 54
        0, // 55
        0, // 56
        0, // 57
        0, // 58
        0, // 59
        0, // 60
        0, // 61
        0, // 62
        0, // 63
        // ----------------------------------------------------------------
        0, // 0
        0, // 1
        0, // 2
        0, // 3
        "gem_5",        // 4
        "gem_6",        // 5
        "gem_7",        // 6
        0, // 7
        0, // 8
        0, // 9
        0, // 10
        0, // 11
        0, // 12
        0, // 13
        0, // 14
        0, // 15
        0, // 16
        0, // 17
        0, // 18
        0, // 19
        0, // 20
        0, // 21
        0, // 22
        0, // 23
        0, // 24
        0, // 25
        0, // 26
        0, // 27
        0, // 28
        0, // 29
        0, // 30
        0, // 31
        0, // 32
        0, // 33
        0, // 34
        0, // 35
        0, // 36
        0, // 37
        0, // 38
        0, // 39
        0, // 40
        0, // 41
        0, // 42
        0, // 43
        0, // 44
        0, // 45
        0, // 46
        0, // 47
        0, // 48
        0, // 49
        0, // 50
        0, // 51
        0, // 52
        0, // 53
        0, // 54
        0, // 55
        0, // 56
        0, // 57
        0, // 58
        0, // 59
        0, // 60
        0, // 61
        0, // 62
        0, // 63
        // ----------------------------------------------------------------
        0, // 0
        0, // 1
        0, // 2
        0, // 3
        0, // 4
        0, // 5
        0, // 6
        0, // 7
        0, // 8
        0, // 9
        0, // 10
        0, // 11
        0, // 12
        0, // 13
        0, // 14
        0, // 15
        0, // 16
        0, // 17
        0, // 18
        0, // 19
        0, // 20
        0, // 21
        0, // 22
        0, // 23
        0, // 24
        0, // 25
        0, // 26
        0, // 27
        0, // 28
        0, // 29
        0, // 30
        0, // 31
        0, // 32
        0, // 33
        0, // 34
        0, // 35
        0, // 36
        0, // 37
        0, // 38
        0, // 39
        0, // 40
        0, // 41
        0, // 42
        0, // 43
        0, // 44
        0, // 45
        0, // 46
        0, // 47
        0, // 48
        0, // 49
        0, // 50
        0, // 51
        0, // 52
        0, // 53
        0, // 54
        0, // 55
        0, // 56
        0, // 57
        0, // 58
        0, // 59
        0, // 60
        0, // 61
        0, // 62
        0, // 63
    };

    tile_load_arr("sprites_small", TILE_WIDTH, TILE_HEIGHT,
                  ARRAY_SIZE(small_tiles), small_tiles);

    PLAYER_0 = thing_template_load("data/things/player_0");
    PLAYER_0_ID = thing_template_to_id(PLAYER_0);

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
