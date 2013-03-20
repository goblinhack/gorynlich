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

    tile_load("sprites_med", 64, 76,
              96,
              /* row 1 */
              "space",
              "flower1",
              "flower2",
              "flower3",
              "flower4",
              (char*)0,
              (char*)0,
              (char*)0,
              /* row 2 */
              "flower5",
              "flower6",
              "flower7",
              (char*)0,
              "star_green" ,
              "star_purple" ,
              "star_pink" ,
              "star_red" ,
              /* row 3 */
              (char*)0,
              (char*)0,
              "letter1",
              "letter2",
              "star_yellow" ,
              "star_blue" ,
              "star_cyan" ,
              "star_black" ,
              /* row 4 */
              (char*)0,
              (char*)0,
              "grass-block",
              "grass-horiz",
              "grass-vert",
              (char*)0,
              (char*)0,
              (char*)0,
              /* row 5 */
              "grass-node",
              "grass-left",
              "grass-right",
              "grass-top",
              "grass-bot",
              (char*)0,
              (char*)0,
              (char*)0,
              /* row 6 */
              "grass-tl",
              "grass-tr",
              "grass-bl",
              "grass-br",
              "grass-t270",
              (char*)0,
              (char*)0,
              (char*)0,
              /* row 7 */
              "grass-t90",
              "grass-t",
              "grass-t180",
              "grass-x",
              (char*)0,
              (char*)0,
              (char*)0,
              (char*)0,
              /* row 8 */
              "grass-tl2",
              "grass-tr2",
              "grass-bl2",
              "grass-br2",
              (char*)0,
              (char*)0,
              (char*)0,
              (char*)0,
              /* row 9 */
              "grass-t270-1",
              "grass-t270-2",
              "grass-t270-3",
              (char*)0,
              (char*)0,
              (char*)0,
              (char*)0,
              (char*)0,
              /* row 10 */
              "grass-t90-1",
              "grass-t90-2",
              "grass-t90-3",
              (char*)0,
              (char*)0,
              (char*)0,
              (char*)0,
              (char*)0,
              /* row 11 */
              "grass-t-1",
              "grass-t-2",
              "grass-t-3",
              (char*)0,
              (char*)0,
              (char*)0,
              (char*)0,
              (char*)0,
              /* row 12 */
              "grass-t180-1",
              "grass-t180-2",
              "grass-t180-3",
              (char*)0,
              (char*)0,
              (char*)0,
              (char*)0,
              (char*)0
            );

    tile_load("sprites_med2", 64, 76,
              24,
              /* row 1 */
              "seedpod1",
              "seedpod2",
              "seedpod3",
              "seedpod4",
              "seedpod5",
              (char*)0,
              (char*)0,
              (char*)0,
              /* row 2 */
              "seedpod6",
              "seedpod7",
              "seedpod8",
              "seedpod9",
              "seedpod10",
              "spam",
              (char*)0,
              (char*)0,
              /* row 3 */
              "bomb1",
              "bomb2",
              "bomb3",
              "bomb4",
              "bomb5",
              "bomb6",
              "bomb7",
              "bomb8"
            );

    tile_load("plant", 64, 76,
              16,
              /* row 1 */
              "plant1",
              "plant2",
              "plant3",
              "plant4",
              "plant5",
              "plant6",
              "plant7",
              "plant8",
              /* row 2 */
              "plant9",
              "plant10",
              "plant11",
              "plant12",
              (char*)0,
              (char*)0,
              (char*)0,
              (char*)0
            );

    tile_load("sprites_large", 128, 128,
              8,
              "house1",
              "house2",
              "house3",
              "house4",
              "house5",
              "house6",
              "house7",
              "house8"
              );

    tile_load("rocket", 64, 76,
              8,
              /* row 1 */
              "rocket1",
              "rocket2",
              "rocket3",
              "rocket4",
              "rocket5",
              "rocket6",
              "rocket7",
              (char*)0
            );

    tile_load("road", 64, 64,
              16,
              /* row 1 */
              "car",
              "road-bot",
              "road-left",
              "road-bl",
              /* row 2 */
              "road-top",
              "road-vert",
              "road-tl",
              "road-t270",
              /* row 3 */
              "road-right",
              "road-br",
              "road-horiz",
              "road-t180",
              /* row 4 */
              "road-tr",
              "road-t90",
              "road-t",
              "road-x");

    tile_load("pipes", 64, 64,
              16,
              /* row 1 */
              "pipe-node",
              "pipe-bot",
              "pipe-left",
              "pipe-bl",
              /* row 2 */
              "pipe-top",
              "pipe-vert",
              "pipe-tl",
              "pipe-t270",
              /* row 3 */
              "pipe-right",
              "pipe-br",
              "pipe-horiz",
              "pipe-t180",
              /* row 4 */
              "pipe-tr",
              "pipe-t90",
              "pipe-t",
              "pipe-x");

    tile_load("explosion", 102, 102,
              25,
              /* row 1 */
              "explosion1",
              "explosion2",
              "explosion3",
              "explosion4",
              "explosion5",
              /* row 2 */
              "explosion6",
              "explosion7",
              "explosion8",
              "explosion9",
              "explosion10",
              /* row 3 */
              "explosion11",
              "explosion12",
              "explosion13",
              "explosion14",
              "explosion15",
              /* row 4 */
              "explosion16",
              "explosion17",
              "explosion18",
              "explosion19",
              "explosion20",
              /* row 5 */
              "explosion21",
              "explosion22",
              "explosion23",
              "explosion24",
              "explosion25");

    thing_template_load("data/things/wall");
    thing_template_load("data/things/floor");
    thing_template_load("data/things/esnail");
    thing_template_load("data/things/player");
    thing_template_load("data/things/letter");
    thing_template_load("data/things/plant");
    thing_template_load("data/things/seedpod");
    thing_template_load("data/things/food");
    thing_template_load("data/things/pipe");
    thing_template_load("data/things/road");
    thing_template_load("data/things/car");

    thing_template_load("data/things/explosion");
    thing_template_load("data/things/house");

    thing_template_load("data/things/m");
    thing_template_load("data/things/a");
    thing_template_load("data/things/i");
    thing_template_load("data/things/l");
    thing_template_load("data/things/s");
    thing_template_load("data/things/n");

    thing_template_load("data/things/star_yellow");

#if 0
    /*
     * Future.
     */
    thing_template_load("data/things/star_black");
    thing_template_load("data/things/star_green");
    thing_template_load("data/things/star_blue");
    thing_template_load("data/things/star_cyan");
    thing_template_load("data/things/star_pink");
    thing_template_load("data/things/star_purple");
    thing_template_load("data/things/star_red");
#endif

    thing_template_load("data/things/powerup_rocket");
    thing_template_load("data/things/powerup_spam");
    thing_template_load("data/things/noentry");

    thing_template_load("data/things/bomb");
    thing_template_load("data/things/spam");

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
