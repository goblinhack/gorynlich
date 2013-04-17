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
        "rock-tl1",
        "rock-tl2",
        "rock-tl3",
        "rock-tr1",
        "rock-tr2",
        "rock-tr3",
        "rock-bl1",
        "rock-bl2",
        "rock-bl3",
        "rock-br1",
        "rock-br2",
        "rock-br3",
        "rock-left1",
        "rock-left2",
        "rock-left3",
        "rock-right1",
        "rock-right2",
        "rock-right3",
        "rock-top1",
        "rock-top2",
        "rock-top3",
        "rock-bot1",
        "rock-bot2",
        "rock-bot3",
        "rock-vert1",
        "rock-vert2",
        "rock-vert3",
        "rock-horiz1",
        "rock-horiz2",
        "rock-horiz3",
        "rock-node1",
        "rock-node2",
        "rock-node3",
        "rock-node4",
        "rock-node5",
        "rock-node6",
        "rock-x",
        "rock-x-tl",
        "rock-x-tr",
        "rock-x-br",
        "rock-x-bl",
        "rock-t-x-180",
        "rock-t-x-270",
        "rock-t-x",
        "rock-t-x-90",
        "rock-dot-tl",
        "rock-dot-tr",
        "rock-dot-br",
        "rock-dot-bl",
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        // ----------------------------------------------------------------
        "rock8",
        "rock9",
        "rock10",
        "rock11",
        "rock12",
        "rock13",
        "rock14",
        "rock15",
        "rock-tl2-1",
        "rock-tl2-2",
        "rock-tl2-3",
        "rock-tr2-1",
        "rock-tr2-2",
        "rock-tr2-3",
        "rock-bl2-1",
        "rock-bl2-2",
        "rock-bl2-3",
        "rock-br2-1",
        "rock-br2-2",
        "rock-br2-3",
        "rock-left2-1",
        "rock-left2-2",
        "rock-left2-3",
        "rock-right2-1",
        "rock-right2-2",
        "rock-right2-3",
        "rock-top2-1",
        "rock-top2-2",
        "rock-top2-3",
        "rock-bot2-1",
        "rock-bot2-2",
        "rock-bot2-3",
        "rock-left3-1",
        "rock-left3-2",
        "rock-left3-3",
        "rock-right3-1",
        "rock-right3-2",
        "rock-right3-3",
        "rock-top3-1",
        "rock-top3-2",
        "rock-top3-3",
        "rock-bot3-1",
        "rock-bot3-2",
        "rock-bot3-3",
        "rock-left4-1",
        "rock-left4-2",
        "rock-left4-3",
        "rock-right4-1",
        "rock-right4-2",
        "rock-right4-3",
        "rock-top4-1",
        "rock-top4-2",
        "rock-top4-3",
        "rock-bot4-1",
        "rock-bot4-2",
        "rock-bot4-3",
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

    tile_load_arr("sprites_small", 8, 8,
                  ARRAY_SIZE(small_tiles), small_tiles);

    thing_template_load("data/things/wall");

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
