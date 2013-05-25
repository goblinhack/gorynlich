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

thing_templatep WALL_1;
thing_templatep ROCK;

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
                           8, 8) != 0);
}

static boolean resource_init_22 (void *context)
{
    return (true);
}

static boolean resource_init_23 (void *context)
{
    static const char *small_tiles[] = {
        "space",
        "rock1",
        "rock2",
        "rock3",
        "rock4",
        "rock5",
        "rock6",
        "rock7",
        "rock1_is_join_node",
        "rock1_is_join_left",
        "rock1_is_join_bot",
        "rock1_is_join_right",
        "rock1_is_join_top",
        "rock1_is_join_horiz",
        "rock1_is_join_vert",
        "rock1_is_join_tl2",
        "rock1_is_join_bl2",
        "rock1_is_join_br2",
        "rock1_is_join_tr2",
        "rock1_is_join_tl",
        "rock1_is_join_bl",
        "rock1_is_join_br",
        "rock1_is_join_tr",
        "rock1_is_join_t270_3",
        "rock1_is_join_t180_3",
        "rock1_is_join_t90_3",
        "rock1_is_join_t_3",
        "rock1_is_join_t270",
        "rock1_is_join_t180",
        "rock1_is_join_t90",
        "rock1_is_join_t",
        "rock1_is_join_t270_2",
        "rock1_is_join_t180_2",
        "rock1_is_join_t90_2",
        "rock1_is_join_t_2",
        "rock1_is_join_t270_1",
        "rock1_is_join_t180_1",
        "rock1_is_join_t90_1",
        "rock1_is_join_t_1",
        "rock1_is_join_x",
        "rock1_is_join_x1",
        "rock1_is_join_x1_270",
        "rock1_is_join_x1_180",
        "rock1_is_join_x1_90",
        "rock1_is_join_x2",
        "rock1_is_join_x2_270",
        "rock1_is_join_x2_180",
        "rock1_is_join_x2_90",
        "rock1_is_join_x3",
        "rock1_is_join_x3_180",
        "rock1_is_join_x4",
        "rock1_is_join_x4_270",
        "rock1_is_join_x4_180",
        "rock1_is_join_x4_90",
        0,
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
        "wall_0_0",
        "wall_0_1",
        "wall_0_2",
        "wall_0_3",
        "wall_0_4",
        "wall_0_5",
        "wall_0_6",
        "wall_0_7",
        "wall_0_1_is_join_node",
        "wall_0_1_is_join_left",
        "wall_0_1_is_join_bot",
        "wall_0_1_is_join_right",
        "wall_0_1_is_join_top",
        "wall_0_1_is_join_horiz",
        "wall_0_1_is_join_vert",
        "wall_0_1_is_join_tl2",
        "wall_0_1_is_join_bl2",
        "wall_0_1_is_join_br2",
        "wall_0_1_is_join_tr2",
        "wall_0_1_is_join_tl",
        "wall_0_1_is_join_bl",
        "wall_0_1_is_join_br",
        "wall_0_1_is_join_tr",
        "wall_0_1_is_join_t270_3",
        "wall_0_1_is_join_t180_3",
        "wall_0_1_is_join_t90_3",
        "wall_0_1_is_join_t_3",
        "wall_0_1_is_join_t270",
        "wall_0_1_is_join_t180",
        "wall_0_1_is_join_t90",
        "wall_0_1_is_join_t",
        "wall_0_1_is_join_t270_2",
        "wall_0_1_is_join_t180_2",
        "wall_0_1_is_join_t90_2",
        "wall_0_1_is_join_t_2",
        "wall_0_1_is_join_t270_1",
        "wall_0_1_is_join_t180_1",
        "wall_0_1_is_join_t90_1",
        "wall_0_1_is_join_t_1",
        "wall_0_1_is_join_x",
        "wall_0_1_is_join_x1",
        "wall_0_1_is_join_x1_270",
        "wall_0_1_is_join_x1_180",
        "wall_0_1_is_join_x1_90",
        "wall_0_1_is_join_x2",
        "wall_0_1_is_join_x2_270",
        "wall_0_1_is_join_x2_180",
        "wall_0_1_is_join_x2_90",
        "wall_0_1_is_join_x3",
        "wall_0_1_is_join_x3_180",
        "wall_0_1_is_join_x4",
        "wall_0_1_is_join_x4_270",
        "wall_0_1_is_join_x4_180",
        "wall_0_1_is_join_x4_90",
        0,
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

    tile_load_arr("sprites_small", 8, 16,
                  ARRAY_SIZE(small_tiles), small_tiles);

    WALL_1 = thing_template_load("data/things/wall");
    ROCK = thing_template_load("data/things/rock");

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
