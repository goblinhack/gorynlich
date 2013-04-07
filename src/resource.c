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
    return (tex_load("data/gfx/explosion.png",
                     "explosion") != 0);
}

static boolean resource_init_3 (void *context)
{
    return (true);
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
    return (tex_load("data/gfx/pipes.png",
                     "pipes") != 0);
}

static boolean resource_init_14 (void *context)
{
    return (tex_load("data/gfx/plant.png",
                     "plant") != 0);
}

static boolean resource_init_15 (void *context)
{
    return (tex_load("data/gfx/road.png",
                     "road") != 0);
}

static boolean resource_init_16 (void *context)
{
    return (tex_load("data/gfx/rocket.png",
                     "rocket") != 0);
}

static boolean resource_init_17 (void *context)
{
    return (true);
}

static boolean resource_init_18 (void *context)
{
    return (tex_load("data/gfx/sprites_large.png",
                     "sprites_large") != 0);
}

static boolean resource_init_19 (void *context)
{
    return (tex_load("data/gfx/sprites_med.png",
                     "sprites_med") != 0);
}

static boolean resource_init_20 (void *context)
{
    return (tex_load("data/gfx/sprites_med2.png",
                     "sprites_med2") != 0);
}

static boolean resource_init_21 (void *context)
{
    return (tex_load_tiled("data/gfx/sprites_small.png",
                           "sprites_small",
                           8, 8) != 0);
}

static boolean resource_init_22 (void *context)
{
    return (tex_load("data/gfx/stars.png",
                     "stars") != 0);
}

static boolean resource_init_23 (void *context)
{
    tile_load("sprites_small", 8, 8,
              13,
              "m",
              "a",
              "i",
              "l",
              "s",
              "n",
              "unused1",
              "food1",
              "noentry",
              "floor1",
              "floor2",
              "floor3",
              "floor4"
              );

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
