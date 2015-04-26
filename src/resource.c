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

static uint8_t resource_init_1 (void *context)
{
    return (tex_load("data/gfx/gorynlich.png",
                     "gorynlich") != 0);
}

static uint8_t resource_init_1a (void *context)
{
    return (tex_load("data/gfx/alpha.png",
                     "alpha") != 0);
}

static uint8_t resource_init_2 (void *context)
{
    return (tex_load("data/gfx/title2.png",
                     "title2") != 0);
}

static uint8_t resource_init_3 (void *context)
{
    return (tex_load("data/gfx/title.png",
                     "title") != 0);
}

static uint8_t resource_init_4 (void *context)
{
    return (tex_load("data/gfx/gothic_tall_axe.png",
                     "gothic_tall_axe") != 0);
}

static uint8_t resource_init_5 (void *context)
{
    return (tex_load("data/gfx/gothic_tall_chest.png",
                     "gothic_tall_chest") != 0);
}

static uint8_t resource_init_6 (void *context)
{
    return (tex_load("data/gfx/gothic_tall_skull.png",
                     "gothic_tall_skull") != 0);
}

static uint8_t resource_init_7 (void *context)
{
    return (tex_load("data/gfx/dragon.png",
                     "dragon") != 0);
}

static uint8_t resource_init_8 (void *context)
{
    return (tex_load("data/gfx/title3.png",
                     "title3") != 0);
}

static uint8_t resource_init_9 (void *context)
{
    return (tex_load("data/gfx/title_small.png",
                     "title_small") != 0);
}

static uint8_t resource_init_10 (void *context)
{
    return (tex_load("data/gfx/gravestone.png",
                     "gravestone") != 0);
}

static uint8_t resource_init_11 (void *context)
{
    return (tex_load("data/gfx/gravestone2.png",
                     "gravestone2") != 0);
}

static uint8_t resource_init_12 (void *context)
{
    return (tex_load("data/gfx/main_title.png",
                     "main_title") != 0);
}

static uint8_t resource_init_13 (void *context)
{
    return (tex_load_tiled("data/gfx/explosion.png",
                           "explosion",
                           1024, 1024) != 0) &&
           (tex_load_tiled_black_and_white(
                           "data/gfx/explosion.png",
                           "explosion_black_and_white",
                           1024, 1024) != 0);
}

static uint8_t resource_init_14 (void *context)
{
    return (tex_load_tiled("data/gfx/cloudkill.png",
                           "cloudkill",
                           1024, 1024) != 0) &&
           (tex_load_tiled_black_and_white(
                           "data/gfx/cloudkill.png",
                           "cloudkill_black_and_white",
                           1024, 1024) != 0);
}

static uint8_t resource_init_15 (void *context)
{
    return (tex_load_tiled("data/gfx/poison.png",
                           "poison",
                           1024, 1024) != 0) &&
           (tex_load_tiled_black_and_white(
                           "data/gfx/poison.png",
                           "poison_black_and_white",
                           1024, 1024) != 0);
}

static uint8_t resource_init_16 (void *context)
{
    return (tex_load_tiled("data/gfx/anim_man.png",
                           "anim_man",
                           1024, 1024) != 0);
}

static uint8_t resource_init_17 (void *context)
{
    return (tex_load_tiled("data/gfx/anim_treasure_chest.png",
                           "anim_treasure_chest",
                           1024, 1024) != 0);
}

static uint8_t resource_init_18 (void *context)
{
    return (tex_load_tiled("data/gfx/anim_eyes.png",
                           "anim_eyes",
                           512, 512) != 0);
}

static uint8_t resource_init_19 (void *context)
{
    return (tex_load("data/gfx/giant_rock.png",
                     "giant_rock") != 0);
}

static uint8_t resource_init_20 (void *context)
{
    return (tex_load("data/gfx/wall_floor.png",
                     "wall_floor") != 0);
}

static uint8_t resource_init_21 (void *context)
{
    return (tex_load_tiled("data/gfx/sprites.png",
                           "sprites",
                           TILE_WIDTH, TILE_HEIGHT) != 0) &&
           (tex_load_tiled_black_and_white(
                           "data/gfx/sprites.png",
                           "sprites_black_and_white",
                           TILE_WIDTH, TILE_HEIGHT) != 0);
}

static uint8_t resource_init_22 (void *context)
{
    return (tex_load("data/gfx/button_red.png",
                     "button_red") != 0);
}

static uint8_t resource_init_23 (void *context)
{
    return (tex_load("data/gfx/button_green.png",
                     "button_green") != 0);
}

static uint8_t resource_init_24 (void *context)
{
    return (tex_load("data/gfx/title4.png",
                     "title4") != 0);
}

static uint8_t resource_init_25 (void *context)
{
    return (tex_load("data/gfx/button_black.png",
                     "button_black") != 0);
}

static uint8_t resource_init_26 (void *context)
{
    return (tex_load("data/gfx/squiggles.png",
                     "squiggles") != 0);
}

static uint8_t resource_init_27 (void *context)
{
    return (tex_load("data/gfx/itembar.png",
                     "itembar") != 0);
}

static uint8_t resource_init_28 (void *context)
{
    return (tex_load("data/gfx/crystalball.png",
                     "crystalball") != 0);
}

static uint8_t resource_init_29 (void *context)
{
    return (tex_load("data/gfx/crystalball_purple.png",
                     "crystalball_purple") != 0);
}

static uint8_t resource_init_30 (void *context)
{
    return (tex_load("data/gfx/title5.png",
                     "title5") != 0);
}

static uint8_t resource_init_31 (void *context)
{
    return (tex_load_tiled("data/gfx/sawblade.png",
                           "sawblade",
                           512, 512) != 0) &&
           (tex_load_tiled_black_and_white(
                           "data/gfx/sawblade.png",
                           "sawblade_black_and_white",
                           512, 512) != 0);
}

static uint8_t resource_init_32 (void *context)
{
    return (tex_load_tiled("data/gfx/sawblade2.png",
                           "sawblade2",
                           512, 512) != 0) &&
           (tex_load_tiled_black_and_white(
                           "data/gfx/sawblade2.png",
                           "sawblade2_black_and_white",
                           512, 512) != 0);
}

static uint8_t resource_init_33 (void *context)
{
    return (tex_load_tiled("data/gfx/sawblade3.png",
                           "sawblade3",
                           512, 512) != 0) &&
           (tex_load_tiled_black_and_white(
                           "data/gfx/sawblade3.png",
                           "sawblade3_black_and_white",
                           512, 512) != 0);
}

static uint8_t resource_init_34 (void *context)
{
    return (tex_load_tiled("data/gfx/sawblade4.png",
                           "sawblade4",
                           512, 512) != 0) &&
           (tex_load_tiled_black_and_white(
                           "data/gfx/sawblade4.png",
                           "sawblade4_black_and_white",
                           512, 512) != 0);
}

static uint8_t resource_init_35 (void *context)
{
    return (tex_load_tiled("data/gfx/sawblade5.png",
                           "sawblade5",
                           512, 512) != 0) &&
           (tex_load_tiled_black_and_white(
                           "data/gfx/sawblade5.png",
                           "sawblade5_black_and_white",
                           512, 512) != 0);
}

static uint8_t resource_init_36 (void *context)
{
    return (tex_load_tiled("data/gfx/shield.png",
                           "shield",
                           128, 128) != 0) &&
           (tex_load_tiled_black_and_white(
                           "data/gfx/shield.png",
                           "shield_black_and_white",
                           128, 128) != 0);
}

static uint8_t resource_init_last (void *context)
{
    static const char *small_tiles[] = {
        // --------------------------------------------------------------------
        "space",
        "large-collision-map",
        "medium-collision-map",
        "small-collision-map",
        "tiny-collision-map",
        "noentry",
        "apple1",
        "apple2",
        "bread",
        "banana",
        "cheese",
        0,
        0,
        "button_a",
        "button_b",
        "button_y",
        "button_x",
        "button_s",
        "button_e",
        "button_q",
        "button_n",
        0,
        0,
        0,
        0,
        0,
        "brazier1",
        "brazier2",
        "brazier3",
        "brazier4",
        "brazier5",
        "brazier6",
        // --------------------------------------------------------------------
        "key",
        "keys2",
        "keys3",
        "coins1",
        "chest1",
        "amulet1",
        "water",
        "water_poison",
        "spam",
        "potion_monsticide",
        "potion_fire",
        "potion_cloudkill",
        "potion_life",
        "potion_shield",
        "ring1",
        "ring2",
        "ring3",
        "gem1",
        "gem2",
        "gem3",
        "gem4",
        "gem5",
        "gem6",
        "gem7",
        "generator1",
        "bonepile3",
        "bonepile2",
        "bonepile1",
        "generator5",
        "generator6",
        "generator7",
        0,
        // --------------------------------------------------------------------
        "torch1",
        "torch2",
        "torch3",
        "torch4",
        "bow2",
        "bow3",
        "bow1",
        "bow4",
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
        "tunnel1.1",
        "tunnel1.4",
        0,
        "exit2.1",
        "exit3.1",
        "exit4.1",
        "exit5.1",
        "exit6.1",
        "exit7.1",
        "exit1.1",
        0,
        0,
        // --------------------------------------------------------------------
        "sword1",
        "sword2",
        "sword3",
        "sword4",
        "sword5",
        "sword6",
        "axe1",
        "axe2",
        "axe3",
        "axe4",
        "axe5",
        "scythe1",
        "wand-fire1",
        "wand-fire2",
        "wand-fire3",
        "wand-fire4",
        0,
        0,
        0,
        0,
        "tunnel1.2",
        "tunnel1.5",
        0,
        "exit2.2",
        "exit3.2",
        "exit4.2",
        "exit5.2",
        "exit6.2",
        "exit7.2",
        "exit1.2",
        0,
        0,
        // --------------------------------------------------------------------
        "floor1a",
        "floor1b",
        "floor1c",
        "floor1d",
        "floor1e",
        "floor1f",
        "floor1g",
        "floor1h",
        "floor2a",
        "floor2b",
        "floor2c",
        "floor2d",
        "floor2e",
        "floor2f",
        "floor2g",
        "floor2h",
        0,
        0,
        0,
        0,
        "tunnel1.3",
        "tunnel1.6",
        0,
        "exit2.3",
        "exit3.3",
        "exit4.3",
        "exit5.3",
        "exit6.3",
        "exit7.3",
        "exit1.3",
        0,
        0,
        // --------------------------------------------------------------------
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
        "exit2.4",
        "exit3.4",
        "exit4.4",
        "exit5.4",
        "exit6.4",
        "exit7.4",
        "exit1.4",
        0,
        0,
        // --------------------------------------------------------------------
        "floor3a",
        "floor3b",
        "floor3c",
        "floor3d",
        "floor3e",
        "floor3f",
        "floor3g",
        "floor3h",

        "floor4a",
        "floor4b",
        "floor4c",
        "floor4d",
        "floor4e",
        "floor4f",
        "floor4g",
        "floor4h",

        "floor5a",
        "floor5b",
        "floor5c",
        "floor5d",
        "floor5e",
        "floor5f",
        "floor5g",
        "floor5h",

        "floor6a",
        "floor6b",
        "floor6c",
        "floor6d",
        "floor6e",
        "floor6f",
        "floor6g",
        "floor6h",
        // --------------------------------------------------------------------
        "door_1_0",
        "door_1_is_join_node",
        "door_1_is_join_left",
        "door_1_is_join_bot",
        "door_1_is_join_right",
        "door_1_is_join_top",
        "door_1_is_join_horiz",
        "door_1_is_join_vert",
        "door_1_is_join_tl2",
        "door_1_is_join_bl2",
        "door_1_is_join_br2",
        "door_1_is_join_tr2",
        "door_1_is_join_tl",
        "door_1_is_join_bl",
        "door_1_is_join_br",
        "door_1_is_join_tr",
        "door_1_is_join_t270_3",
        "door_1_is_join_t180_3",
        "door_1_is_join_t90_3",
        "door_1_is_join_t_3",
        "door_1_is_join_t270",
        "door_1_is_join_t180",
        "door_1_is_join_t90",
        "door_1_is_join_t",
        "door_1_is_join_t270_2",
        "door_1_is_join_t180_2",
        "door_1_is_join_t90_2",
        "door_1_is_join_t_2",
        "door_1_is_join_t270_1",
        "door_1_is_join_t180_1",
        "door_1_is_join_t90_1",
        "door_1_is_join_t_1",
        // --------------------------------------------------------------------
        "door_1_is_join_x",
        "door_1_is_join_x1",
        "door_1_is_join_x1_270",
        "door_1_is_join_x1_180",
        "door_1_is_join_x1_90",
        "door_1_is_join_x2",
        "door_1_is_join_x2_270",
        "door_1_is_join_x2_180",
        "door_1_is_join_x2_90",
        "door_1_is_join_x3",
        "door_1_is_join_x3_180",
        "door_1_is_join_x4",
        "door_1_is_join_x4_270",
        "door_1_is_join_x4_180",
        "door_1_is_join_x4_90",
        "door_1_is_join_vert2",
        "door_1_is_join_horiz2",
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
        // --------------------------------------------------------------------
        "door_2_0",
        "door_2_is_join_node",
        "door_2_is_join_left",
        "door_2_is_join_bot",
        "door_2_is_join_right",
        "door_2_is_join_top",
        "door_2_is_join_horiz",
        "door_2_is_join_vert",
        "door_2_is_join_tl2",
        "door_2_is_join_bl2",
        "door_2_is_join_br2",
        "door_2_is_join_tr2",
        "door_2_is_join_tl",
        "door_2_is_join_bl",
        "door_2_is_join_br",
        "door_2_is_join_tr",
        "door_2_is_join_t270_3",
        "door_2_is_join_t180_3",
        "door_2_is_join_t90_3",
        "door_2_is_join_t_3",
        "door_2_is_join_t270",
        "door_2_is_join_t180",
        "door_2_is_join_t90",
        "door_2_is_join_t",
        "door_2_is_join_t270_2",
        "door_2_is_join_t180_2",
        "door_2_is_join_t90_2",
        "door_2_is_join_t_2",
        "door_2_is_join_t270_1",
        "door_2_is_join_t180_1",
        "door_2_is_join_t90_1",
        "door_2_is_join_t_1",
        // --------------------------------------------------------------------
        "door_2_is_join_x",
        "door_2_is_join_x1",
        "door_2_is_join_x1_270",
        "door_2_is_join_x1_180",
        "door_2_is_join_x1_90",
        "door_2_is_join_x2",
        "door_2_is_join_x2_270",
        "door_2_is_join_x2_180",
        "door_2_is_join_x2_90",
        "door_2_is_join_x3",
        "door_2_is_join_x3_180",
        "door_2_is_join_x4",
        "door_2_is_join_x4_270",
        "door_2_is_join_x4_180",
        "door_2_is_join_x4_90",
        "door_2_is_join_vert2",
        "door_2_is_join_horiz2",
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        // --------------------------------------------------------------------
        "wall1_0_0",
        "wall1_0_is_join_node",
        "wall1_0_is_join_left",
        "wall1_0_is_join_bot",
        "wall1_0_is_join_right",
        "wall1_0_is_join_top",
        "wall1_0_is_join_horiz",
        "wall1_0_is_join_vert",
        "wall1_0_is_join_tl2",
        "wall1_0_is_join_bl2",
        "wall1_0_is_join_br2",
        "wall1_0_is_join_tr2",
        "wall1_0_is_join_tl",
        "wall1_0_is_join_bl",
        "wall1_0_is_join_br",
        "wall1_0_is_join_tr",
        "wall1_0_is_join_t270_3",
        "wall1_0_is_join_t180_3",
        "wall1_0_is_join_t90_3",
        "wall1_0_is_join_t_3",
        "wall1_0_is_join_t270",
        "wall1_0_is_join_t180",
        "wall1_0_is_join_t90",
        "wall1_0_is_join_t",
        "wall1_0_is_join_t270_2",
        "wall1_0_is_join_t180_2",
        "wall1_0_is_join_t90_2",
        "wall1_0_is_join_t_2",
        "wall1_0_is_join_t270_1",
        "wall1_0_is_join_t180_1",
        "wall1_0_is_join_t90_1",
        "wall1_0_is_join_t_1",
        // --------------------------------------------------------------------
        "wall1_0_is_join_x",
        "wall1_0_is_join_x1",
        "wall1_0_is_join_x1_270",
        "wall1_0_is_join_x1_180",
        "wall1_0_is_join_x1_90",
        "wall1_0_is_join_x2",
        "wall1_0_is_join_x2_270",
        "wall1_0_is_join_x2_180",
        "wall1_0_is_join_x2_90",
        "wall1_0_is_join_x3",
        "wall1_0_is_join_x3_180",
        "wall1_0_is_join_x4",
        "wall1_0_is_join_x4_270",
        "wall1_0_is_join_x4_180",
        "wall1_0_is_join_x4_90",
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        // --------------------------------------------------------------------
        "rock1_0_0",
        "rock1_0_is_join_node",
        "rock1_0_is_join_left",
        "rock1_0_is_join_bot",
        "rock1_0_is_join_right",
        "rock1_0_is_join_top",
        "rock1_0_is_join_horiz",
        "rock1_0_is_join_vert",
        "rock1_0_is_join_tl2",
        "rock1_0_is_join_bl2",
        "rock1_0_is_join_br2",
        "rock1_0_is_join_tr2",
        "rock1_0_is_join_tl",
        "rock1_0_is_join_bl",
        "rock1_0_is_join_br",
        "rock1_0_is_join_tr",
        "rock1_0_is_join_t270_3",
        "rock1_0_is_join_t180_3",
        "rock1_0_is_join_t90_3",
        "rock1_0_is_join_t_3",
        "rock1_0_is_join_t270",
        "rock1_0_is_join_t180",
        "rock1_0_is_join_t90",
        "rock1_0_is_join_t",
        "rock1_0_is_join_t270_2",
        "rock1_0_is_join_t180_2",
        "rock1_0_is_join_t90_2",
        "rock1_0_is_join_t_2",
        "rock1_0_is_join_t270_1",
        "rock1_0_is_join_t180_1",
        "rock1_0_is_join_t90_1",
        "rock1_0_is_join_t_1",
        // --------------------------------------------------------------------
        "rock1_0_is_join_x",
        "rock1_0_is_join_x1",
        "rock1_0_is_join_x1_270",
        "rock1_0_is_join_x1_180",
        "rock1_0_is_join_x1_90",
        "rock1_0_is_join_x2",
        "rock1_0_is_join_x2_270",
        "rock1_0_is_join_x2_180",
        "rock1_0_is_join_x2_90",
        "rock1_0_is_join_x3",
        "rock1_0_is_join_x3_180",
        "rock1_0_is_join_x4",
        "rock1_0_is_join_x4_270",
        "rock1_0_is_join_x4_180",
        "rock1_0_is_join_x4_90",
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        // --------------------------------------------------------------------
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
        // --------------------------------------------------------------------
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
        // --------------------------------------------------------------------
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
        // --------------------------------------------------------------------
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
        // --------------------------------------------------------------------
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
        // --------------------------------------------------------------------
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
        // --------------------------------------------------------------------
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
        // --------------------------------------------------------------------
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
        // --------------------------------------------------------------------
        "wall6_0_0",
        "wall6_0_is_join_node",
        "wall6_0_is_join_left",
        "wall6_0_is_join_bot",
        "wall6_0_is_join_right",
        "wall6_0_is_join_top",
        "wall6_0_is_join_horiz",
        "wall6_0_is_join_vert",
        "wall6_0_is_join_tl2",
        "wall6_0_is_join_bl2",
        "wall6_0_is_join_br2",
        "wall6_0_is_join_tr2",
        "wall6_0_is_join_tl",
        "wall6_0_is_join_bl",
        "wall6_0_is_join_br",
        "wall6_0_is_join_tr",
        "wall6_0_is_join_t270_3",
        "wall6_0_is_join_t180_3",
        "wall6_0_is_join_t90_3",
        "wall6_0_is_join_t_3",
        "wall6_0_is_join_t270",
        "wall6_0_is_join_t180",
        "wall6_0_is_join_t90",
        "wall6_0_is_join_t",
        "wall6_0_is_join_t270_2",
        "wall6_0_is_join_t180_2",
        "wall6_0_is_join_t90_2",
        "wall6_0_is_join_t_2",
        "wall6_0_is_join_t270_1",
        "wall6_0_is_join_t180_1",
        "wall6_0_is_join_t90_1",
        "wall6_0_is_join_t_1",
        // --------------------------------------------------------------------
        "wall6_0_is_join_x",
        "wall6_0_is_join_x1",
        "wall6_0_is_join_x1_270",
        "wall6_0_is_join_x1_180",
        "wall6_0_is_join_x1_90",
        "wall6_0_is_join_x2",
        "wall6_0_is_join_x2_270",
        "wall6_0_is_join_x2_180",
        "wall6_0_is_join_x2_90",
        "wall6_0_is_join_x3",
        "wall6_0_is_join_x3_180",
        "wall6_0_is_join_x4",
        "wall6_0_is_join_x4_270",
        "wall6_0_is_join_x4_180",
        "wall6_0_is_join_x4_90",
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        // --------------------------------------------------------------------
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

        // --------------------------------------------------------------------
        
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

        // --------------------------------------------------------------------
        
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

        // --------------------------------------------------------------------
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

        // --------------------------------------------------------------------
        
        "ghost1a-right-eyes",
        "ghost1a-br-eyes",
        "ghost1a-down-eyes",
        "ghost1a-bl-eyes",
        "ghost1a-left-eyes",
        "ghost1a-tl-eyes",
        "ghost1a-up-eyes",
        "ghost1a-tr-eyes",

        "ghost1b-right-eyes",
        "ghost1b-br-eyes",
        "ghost1b-down-eyes",
        "ghost1b-bl-eyes",
        "ghost1b-left-eyes",
        "ghost1b-tl-eyes",
        "ghost1b-up-eyes",
        "ghost1b-tr-eyes",

        "ghost1c-right-eyes",
        "ghost1c-br-eyes",
        "ghost1c-down-eyes",
        "ghost1c-bl-eyes",
        "ghost1c-left-eyes",
        "ghost1c-tl-eyes",
        "ghost1c-up-eyes",
        "ghost1c-tr-eyes",

        "ghost1d-right-eyes",
        "ghost1d-br-eyes",
        "ghost1d-down-eyes",
        "ghost1d-bl-eyes",
        "ghost1d-left-eyes",
        "ghost1d-tl-eyes",
        "ghost1d-up-eyes",
        "ghost1d-tr-eyes",

        // --------------------------------------------------------------------
        
        "ghost2a-right-eyes",
        "ghost2a-br-eyes",
        "ghost2a-down-eyes",
        "ghost2a-bl-eyes",
        "ghost2a-left-eyes",
        "ghost2a-tl-eyes",
        "ghost2a-up-eyes",
        "ghost2a-tr-eyes",

        "ghost2b-right-eyes",
        "ghost2b-br-eyes",
        "ghost2b-down-eyes",
        "ghost2b-bl-eyes",
        "ghost2b-left-eyes",
        "ghost2b-tl-eyes",
        "ghost2b-up-eyes",
        "ghost2b-tr-eyes",

        "ghost2c-right-eyes",
        "ghost2c-br-eyes",
        "ghost2c-down-eyes",
        "ghost2c-bl-eyes",
        "ghost2c-left-eyes",
        "ghost2c-tl-eyes",
        "ghost2c-up-eyes",
        "ghost2c-tr-eyes",

        "ghost2d-right-eyes",
        "ghost2d-br-eyes",
        "ghost2d-down-eyes",
        "ghost2d-bl-eyes",
        "ghost2d-left-eyes",
        "ghost2d-tl-eyes",
        "ghost2d-up-eyes",
        "ghost2d-tr-eyes",

        // --------------------------------------------------------------------
        "ghost3a-right-eyes",
        "ghost3a-br-eyes",
        "ghost3a-down-eyes",
        "ghost3a-bl-eyes",
        "ghost3a-left-eyes",
        "ghost3a-tl-eyes",
        "ghost3a-up-eyes",
        "ghost3a-tr-eyes",

        "ghost3b-right-eyes",
        "ghost3b-br-eyes",
        "ghost3b-down-eyes",
        "ghost3b-bl-eyes",
        "ghost3b-left-eyes",
        "ghost3b-tl-eyes",
        "ghost3b-up-eyes",
        "ghost3b-tr-eyes",

        "ghost3c-right-eyes",
        "ghost3c-br-eyes",
        "ghost3c-down-eyes",
        "ghost3c-bl-eyes",
        "ghost3c-left-eyes",
        "ghost3c-tl-eyes",
        "ghost3c-up-eyes",
        "ghost3c-tr-eyes",

        "ghost3d-right-eyes",
        "ghost3d-br-eyes",
        "ghost3d-down-eyes",
        "ghost3d-bl-eyes",
        "ghost3d-left-eyes",
        "ghost3d-tl-eyes",
        "ghost3d-up-eyes",
        "ghost3d-tr-eyes",

        // --------------------------------------------------------------------
	"eyebawla-right",
        "eyebawla-br",
        "eyebawla-down",
        "eyebawla-bl",
        "eyebawla-left",
        "eyebawla-tl",
        "eyebawla-up",
        "eyebawla-tr",

        "eyebawlb-right",
        "eyebawlb-br",
        "eyebawlb-down",
        "eyebawlb-bl",
        "eyebawlb-left",
        "eyebawlb-tl",
        "eyebawlb-up",
        "eyebawlb-tr",

        "eyebawlc-right",
        "eyebawlc-br",
        "eyebawlc-down",
        "eyebawlc-bl",
        "eyebawlc-left",
        "eyebawlc-tl",
        "eyebawlc-up",
        "eyebawlc-tr",

        "eyebawld-right",
        "eyebawld-br",
        "eyebawld-down",
        "eyebawld-bl",
        "eyebawld-left",
        "eyebawld-tl",
        "eyebawld-up",
        "eyebawld-tr",

        // --------------------------------------------------------------------
	"eyebawla-right-eyes",
        "eyebawla-br-eyes",
        "eyebawla-down-eyes",
        "eyebawla-bl-eyes",
        "eyebawla-left-eyes",
        "eyebawla-tl-eyes",
        "eyebawla-up-eyes",
        "eyebawla-tr-eyes",

        "eyebawlb-right-eyes",
        "eyebawlb-br-eyes",
        "eyebawlb-down-eyes",
        "eyebawlb-bl-eyes",
        "eyebawlb-left-eyes",
        "eyebawlb-tl-eyes",
        "eyebawlb-up-eyes",
        "eyebawlb-tr-eyes",

        "eyebawlc-right-eyes",
        "eyebawlc-br-eyes",
        "eyebawlc-down-eyes",
        "eyebawlc-bl-eyes",
        "eyebawlc-left-eyes",
        "eyebawlc-tl-eyes",
        "eyebawlc-up-eyes",
        "eyebawlc-tr-eyes",

        "eyebawld-right-eyes",
        "eyebawld-br-eyes",
        "eyebawld-down-eyes",
        "eyebawld-bl-eyes",
        "eyebawld-left-eyes",
        "eyebawld-tl-eyes",
        "eyebawld-up-eyes",
        "eyebawld-tr-eyes",

        // --------------------------------------------------------------------
        "lizard1-demo-right",
        "lizard1-demo-br",
        "lizard1-demo-down",
        "lizard1-demo-bl",
        "lizard1-demo-left",
        "lizard1-demo-tl",
        "lizard1-demo-up",
        "lizard1-demo-tr",
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
        // --------------------------------------------------------------------
        "death1-demo-right",
        "death1-demo-br",
        "death1-demo-down",
        "death1-demo-bl",
        "death1-demo-left",
        "death1-demo-tl",
        "death1-demo-up",
        "death1-demo-tr",
        "death1-right",
        "death1-br",
        "death1-down",
        "death1-bl",
        "death1-left",
        "death1-tl",
        "death1-up",
        "death1-tr",
        "death1-right2",
        "death1-br2",
        "death1-down2",
        "death1-bl2",
        "death1-left2",
        "death1-tl2",
        "death1-up2",
        "death1-tr2",
        "death1-right3",
        "death1-br3",
        "death1-down3",
        "death1-bl3",
        "death1-left3",
        "death1-tl3",
        "death1-up3",
        "death1-tr3",
        // --------------------------------------------------------------------
        "wizard-demo-right",
        "wizard-demo-br",
        "wizard-demo-down",
        "wizard-demo-bl",
        "wizard-demo-left",
        "wizard-demo-tl",
        "wizard-demo-up",
        "wizard-demo-tr",
        "wizard-right",
        "wizard-br",
        "wizard-down",
        "wizard-bl",
        "wizard-left",
        "wizard-tl",
        "wizard-up",
        "wizard-tr",
        "wizard-right2",
        "wizard-br2",
        "wizard-down2",
        "wizard-bl2",
        "wizard-left2",
        "wizard-tl2",
        "wizard-up2",
        "wizard-tr2",
        "wizard-right3",
        "wizard-br3",
        "wizard-down3",
        "wizard-bl3",
        "wizard-left3",
        "wizard-tl3",
        "wizard-up3",
        "wizard-tr3",
        // --------------------------------------------------------------------
        "elf-demo-right",
        "elf-demo-br",
        "elf-demo-down",
        "elf-demo-bl",
        "elf-demo-left",
        "elf-demo-tl",
        "elf-demo-up",
        "elf-demo-tr",
        "elf-right",
        "elf-br",
        "elf-down",
        "elf-bl",
        "elf-left",
        "elf-tl",
        "elf-up",
        "elf-tr",
        "elf-right2",
        "elf-br2",
        "elf-down2",
        "elf-bl2",
        "elf-left2",
        "elf-tl2",
        "elf-up2",
        "elf-tr2",
        "elf-right3",
        "elf-br3",
        "elf-down3",
        "elf-bl3",
        "elf-left3",
        "elf-tl3",
        "elf-up3",
        "elf-tr3",
        // --------------------------------------------------------------------
        "dwarf-demo-right",
        "dwarf-demo-br",
        "dwarf-demo-down",
        "dwarf-demo-bl",
        "dwarf-demo-left",
        "dwarf-demo-tl",
        "dwarf-demo-up",
        "dwarf-demo-tr",
        "dwarf-right",
        "dwarf-br",
        "dwarf-down",
        "dwarf-bl",
        "dwarf-left",
        "dwarf-tl",
        "dwarf-up",
        "dwarf-tr",
        "dwarf-right2",
        "dwarf-br2",
        "dwarf-down2",
        "dwarf-bl2",
        "dwarf-left2",
        "dwarf-tl2",
        "dwarf-up2",
        "dwarf-tr2",
        "dwarf-right3",
        "dwarf-br3",
        "dwarf-down3",
        "dwarf-bl3",
        "dwarf-left3",
        "dwarf-tl3",
        "dwarf-up3",
        "dwarf-tr3",
        // --------------------------------------------------------------------
        "warrior-demo-right",
        "warrior-demo-br",
        "warrior-demo-down",
        "warrior-demo-bl",
        "warrior-demo-left",
        "warrior-demo-tl",
        "warrior-demo-up",
        "warrior-demo-tr",
        "warrior-right",
        "warrior-br",
        "warrior-down",
        "warrior-bl",
        "warrior-left",
        "warrior-tl",
        "warrior-up",
        "warrior-tr",
        "warrior-right2",
        "warrior-br2",
        "warrior-down2",
        "warrior-bl2",
        "warrior-left2",
        "warrior-tl2",
        "warrior-up2",
        "warrior-tr2",
        "warrior-right3",
        "warrior-br3",
        "warrior-down3",
        "warrior-bl3",
        "warrior-left3",
        "warrior-tl3",
        "warrior-up3",
        "warrior-tr3",
        // --------------------------------------------------------------------
        "valkyrie-demo-right",
        "valkyrie-demo-br",
        "valkyrie-demo-down",
        "valkyrie-demo-bl",
        "valkyrie-demo-left",
        "valkyrie-demo-tl",
        "valkyrie-demo-up",
        "valkyrie-demo-tr",
        "valkyrie-right",
        "valkyrie-br",
        "valkyrie-down",
        "valkyrie-bl",
        "valkyrie-left",
        "valkyrie-tl",
        "valkyrie-up",
        "valkyrie-tr",
        "valkyrie-right2",
        "valkyrie-br2",
        "valkyrie-down2",
        "valkyrie-bl2",
        "valkyrie-left2",
        "valkyrie-tl2",
        "valkyrie-up2",
        "valkyrie-tr2",
        "valkyrie-right3",
        "valkyrie-br3",
        "valkyrie-down3",
        "valkyrie-bl3",
        "valkyrie-left3",
        "valkyrie-tl3",
        "valkyrie-up3",
        "valkyrie-tr3",
        // --------------------------------------------------------------------
        "wizard-demo-right-eyes",
        "wizard-demo-br-eyes",
        "wizard-demo-down-eyes",
        "wizard-demo-bl-eyes",
        "wizard-demo-left-eyes",
        "wizard-demo-tl-eyes",
        "wizard-demo-up-eyes",
        "wizard-demo-tr-eyes",
        "wizard-right-eyes",
        "wizard-br-eyes",
        "wizard-down-eyes",
        "wizard-bl-eyes",
        "wizard-left-eyes",
        "wizard-tl-eyes",
        "wizard-up-eyes",
        "wizard-tr-eyes",
        "wizard-right4",
        "wizard-br4",
        "wizard-down4",
        "wizard-bl4",
        "wizard-left4",
        "wizard-tl4",
        "wizard-up4",
        "wizard-tr4",
        "wizard-right5",
        "wizard-br5",
        "wizard-down5",
        "wizard-bl5",
        "wizard-left5",
        "wizard-tl5",
        "wizard-up5",
        "wizard-tr5",
        // --------------------------------------------------------------------
        "elf-demo-right-eyes",
        "elf-demo-br-eyes",
        "elf-demo-down-eyes",
        "elf-demo-bl-eyes",
        "elf-demo-left-eyes",
        "elf-demo-tl-eyes",
        "elf-demo-up-eyes",
        "elf-demo-tr-eyes",
        "elf-right-eyes",
        "elf-br-eyes",
        "elf-down-eyes",
        "elf-bl-eyes",
        "elf-left-eyes",
        "elf-tl-eyes",
        "elf-up-eyes",
        "elf-tr-eyes",
        "elf-right4",
        "elf-br4",
        "elf-down4",
        "elf-bl4",
        "elf-left4",
        "elf-tl4",
        "elf-up4",
        "elf-tr4",
        "elf-right5",
        "elf-br5",
        "elf-down5",
        "elf-bl5",
        "elf-left5",
        "elf-tl5",
        "elf-up5",
        "elf-tr5",
        // --------------------------------------------------------------------
        "dwarf-demo-right-eyes",
        "dwarf-demo-br-eyes",
        "dwarf-demo-down-eyes",
        "dwarf-demo-bl-eyes",
        "dwarf-demo-left-eyes",
        "dwarf-demo-tl-eyes",
        "dwarf-demo-up-eyes",
        "dwarf-demo-tr-eyes",
        "dwarf-right-eyes",
        "dwarf-br-eyes",
        "dwarf-down-eyes",
        "dwarf-bl-eyes",
        "dwarf-left-eyes",
        "dwarf-tl-eyes",
        "dwarf-up-eyes",
        "dwarf-tr-eyes",
        "dwarf-right4",
        "dwarf-br4",
        "dwarf-down4",
        "dwarf-bl4",
        "dwarf-left4",
        "dwarf-tl4",
        "dwarf-up4",
        "dwarf-tr4",
        "dwarf-right5",
        "dwarf-br5",
        "dwarf-down5",
        "dwarf-bl5",
        "dwarf-left5",
        "dwarf-tl5",
        "dwarf-up5",
        "dwarf-tr5",
        // --------------------------------------------------------------------
        "warrior-demo-right-eyes",
        "warrior-demo-br-eyes",
        "warrior-demo-down-eyes",
        "warrior-demo-bl-eyes",
        "warrior-demo-left-eyes",
        "warrior-demo-tl-eyes",
        "warrior-demo-up-eyes",
        "warrior-demo-tr-eyes",
        "warrior-right-eyes",
        "warrior-br-eyes",
        "warrior-down-eyes",
        "warrior-bl-eyes",
        "warrior-left-eyes",
        "warrior-tl-eyes",
        "warrior-up-eyes",
        "warrior-tr-eyes",
        "warrior-right4",
        "warrior-br4",
        "warrior-down4",
        "warrior-bl4",
        "warrior-left4",
        "warrior-tl4",
        "warrior-up4",
        "warrior-tr4",
        "warrior-right5",
        "warrior-br5",
        "warrior-down5",
        "warrior-bl5",
        "warrior-left5",
        "warrior-tl5",
        "warrior-up5",
        "warrior-tr5",
        // --------------------------------------------------------------------
        "valkyrie-demo-right-eyes",
        "valkyrie-demo-br-eyes",
        "valkyrie-demo-down-eyes",
        "valkyrie-demo-bl-eyes",
        "valkyrie-demo-left-eyes",
        "valkyrie-demo-tl-eyes",
        "valkyrie-demo-up-eyes",
        "valkyrie-demo-tr-eyes",
        "valkyrie-right-eyes",
        "valkyrie-br-eyes",
        "valkyrie-down-eyes",
        "valkyrie-bl-eyes",
        "valkyrie-left-eyes",
        "valkyrie-tl-eyes",
        "valkyrie-up-eyes",
        "valkyrie-tr-eyes",
        "valkyrie-right4",
        "valkyrie-br4",
        "valkyrie-down4",
        "valkyrie-bl4",
        "valkyrie-left4",
        "valkyrie-tl4",
        "valkyrie-up4",
        "valkyrie-tr4",
        "valkyrie-right5",
        "valkyrie-br5",
        "valkyrie-down5",
        "valkyrie-bl5",
        "valkyrie-left5",
        "valkyrie-tl5",
        "valkyrie-up5",
        "valkyrie-tr5",
        // --------------------------------------------------------------------
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        "wizard-dying1",
        "wizard-dying2",
        "wizard-dying3",
        "wizard-dying4",
        "wizard-dying5",
        "wizard-dying6",
        "wizard-dying7",
        "wizard-dying8",
        "wizard-dying9",
        "wizard-dying10",
        0,
        0,
        0,
        0,
        0,
        0,
        // --------------------------------------------------------------------
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        "elf-dying1",
        "elf-dying2",
        "elf-dying3",
        "elf-dying4",
        "elf-dying5",
        "elf-dying6",
        "elf-dying7",
        "elf-dying8",
        "elf-dying9",
        "elf-dying10",
        0,
        0,
        0,
        0,
        0,
        0,
        // --------------------------------------------------------------------
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        "dwarf-dying1",
        "dwarf-dying2",
        "dwarf-dying3",
        "dwarf-dying4",
        "dwarf-dying5",
        "dwarf-dying6",
        "dwarf-dying7",
        "dwarf-dying8",
        "dwarf-dying9",
        "dwarf-dying10",
        0,
        0,
        0,
        0,
        0,
        0,
        // --------------------------------------------------------------------
        "sword3.1",
        "sword3.2",
        "sword3.3",
        "sword3.4",
        "sword3.5",
        "sword3.6",
        "sword3.7",
        "sword3.8",
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        "warrior-dying1",
        "warrior-dying2",
        "warrior-dying3",
        "warrior-dying4",
        "warrior-dying5",
        "warrior-dying6",
        "warrior-dying7",
        "warrior-dying8",
        "warrior-dying9",
        "warrior-dying10",
        0,
        0,
        0,
        0,
        0,
        0,
        // --------------------------------------------------------------------
        "sword2.1",
        "sword2.2",
        "sword2.3",
        "sword2.4",
        "sword2.5",
        "sword2.6",
        "sword2.7",
        "sword2.8",
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        "valkyrie-dying1",
        "valkyrie-dying2",
        "valkyrie-dying3",
        "valkyrie-dying4",
        "valkyrie-dying5",
        "valkyrie-dying6",
        "valkyrie-dying7",
        "valkyrie-dying8",
        "valkyrie-dying9",
        "valkyrie-dying10",
        0,
        0,
        0,
        0,
        0,
        0,
        // --------------------------------------------------------------------
        "sword1.1",
        "sword1.2",
        "sword1.3",
        "sword1.4",
        "sword1.5",
        "sword1.6",
        "sword1.7",
        "sword1.8",
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        // --------------------------------------------------------------------
        "scythe1.1",
        "scythe1.2",
        "scythe1.3",
        "scythe1.4",
        "scythe1.5",
        "scythe1.6",
        "scythe1.7",
        "scythe1.8",
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        // --------------------------------------------------------------------
        "wand1.1",
        "wand1.2",
        "wand1.3",
        "wand1.4",
        "wand1.5",
        "wand1.6",
        "wand1.7",
        "wand1.8",
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        // --------------------------------------------------------------------
        "bow1.1",
        "bow1.2",
        "bow1.3",
        "bow1.4",
        "bow1.5",
        "bow1.6",
        "bow1.7",
        "bow1.8",
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        // --------------------------------------------------------------------
        "axe2.1",
        "axe2.2",
        "axe2.3",
        "axe2.4",
        "axe2.5",
        "axe2.6",
        "axe2.7",
        "axe2.8",
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        // --------------------------------------------------------------------
        "axe1.1",
        "axe1.2",
        "axe1.3",
        "axe1.4",
        "axe1.5",
        "axe1.6",
        "axe1.7",
        "axe1.8",
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        // --------------------------------------------------------------------
        "weapon_swing1.12",
        "weapon_swing1.11",
        "weapon_swing1.10",
        "weapon_swing1.9",
        "weapon_swing1.8",
        "weapon_swing1.7",
        "weapon_swing1.6",
        "weapon_swing1.5",
        "weapon_swing1.4",
        "weapon_swing1.3",
        "weapon_swing1.2",
        "weapon_swing1.1",
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        // --------------------------------------------------------------------
        "weapon_swing2.1",
        "weapon_swing2.2",
        "weapon_swing2.3",
        "weapon_swing2.4",
        "weapon_swing2.5",
        "weapon_swing2.6",
        "weapon_swing2.7",
        "weapon_swing2.8",
        "weapon_swing2.9",
        "weapon_swing2.10",
        "weapon_swing2.11",
        "weapon_swing2.12",
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        // --------------------------------------------------------------------
        "weapon_swing3.1",
        "weapon_swing3.2",
        "weapon_swing3.3",
        "weapon_swing3.4",
        "weapon_swing3.5",
        "weapon_swing3.6",
        "weapon_swing3.7",
        "weapon_swing3.8",
        "weapon_swing3.9",
        "weapon_swing3.10",
        "weapon_swing3.11",
        "weapon_swing3.12",
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        // --------------------------------------------------------------------
        "weapon_swing4.12",
        "weapon_swing4.11",
        "weapon_swing4.10",
        "weapon_swing4.9",
        "weapon_swing4.8",
        "weapon_swing4.7",
        "weapon_swing4.6",
        "weapon_swing4.5",
        "weapon_swing4.4",
        "weapon_swing4.3",
        "weapon_swing4.2",
        "weapon_swing4.1",
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        // --------------------------------------------------------------------
        "hit_miss.1",
        "hit_miss.2",
        "hit_miss.3",
        "hit_miss.4",
        "hit_miss.5",
        "heart",
        "sparkle",
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        // --------------------------------------------------------------------
        "hit_success.1",
        "hit_success.2",
        "hit_success.3",
        "hit_success.4",
        "hit_success.5",
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        "bomb1a",
        "bomb2a",
        "bomb3a",
        "bomb4a",
        "bomb5a",
        "bomb6a",
        "bomb7a",
        "bomb8a",
        "bomb9a",
        "bomb10a",
        "bomb1b",
        "bomb2b",
        "bomb3b",
        "bomb4b",
        "bomb5b",
        "bomb6b",
        "bomb7b",
        "bomb8b",
        "bomb9b",
        "bomb10b",
        // --------------------------------------------------------------------
        "blood1.1",
        "blood1.2",
        "blood1.3",
        "blood1.4",
        "blood1.5",
        "blood2.1",
        "blood2.2",
        "blood2.3",
        "blood2.4",
        "blood2.5",
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        "action_text",
        "action_zap",
        "action_sleep",
        "action_trigger_on_monst",
        "action_trigger_on_wall",
        "action_trigger_on_hero",
        "action_down",
        "action_right",
        "action_up",
        "action_left",
    };

    tile_load_arr("sprites", 
                  "sprites_black_and_white",
                  TILE_WIDTH, TILE_HEIGHT,
                  ARRAY_SIZE(small_tiles), small_tiles);

    static const char *explosion_tiles[] = {
        // --------------------------------------------------------------------
        "explosion1.1",
        "explosion1.2",
        "explosion1.3",
        "explosion1.4",
        "explosion2.1",
        "explosion2.2",
        "explosion2.3",
        "explosion2.4",
        // --------------------------------------------------------------------
        "explosion1.5",
        "explosion1.6",
        "explosion1.7",
        "explosion1.8",
        "explosion2.5",
        "explosion2.6",
        "explosion2.7",
        "explosion2.8",
        // --------------------------------------------------------------------
        "explosion1.9",
        "explosion1.10",
        "explosion1.11",
        "explosion1.12",
        "explosion2.9",
        "explosion2.10",
        "explosion2.11",
        "explosion2.12",
        // --------------------------------------------------------------------
        "explosion1.13",
        "explosion1.14",
        "explosion1.15",
        "explosion1.16",
        "explosion2.13",
        "explosion2.14",
        "explosion2.15",
        "explosion2.16",
        // --------------------------------------------------------------------
        "explosion3.1",
        "explosion3.2",
        "explosion3.3",
        "explosion3.4",
        "explosion4.1",
        "explosion4.2",
        "explosion4.3",
        "explosion4.4",
        // --------------------------------------------------------------------
        "explosion3.5",
        "explosion3.6",
        "explosion3.7",
        "explosion3.8",
        "explosion4.5",
        "explosion4.6",
        "explosion4.7",
        "explosion4.8",
        // --------------------------------------------------------------------
        "explosion3.9",
        "explosion3.10",
        "explosion3.11",
        "explosion3.12",
        "explosion4.9",
        "explosion4.10",
        "explosion4.11",
        "explosion4.12",
        // --------------------------------------------------------------------
        "explosion3.13",
        "explosion3.14",
        "explosion3.15",
        "explosion3.16",
        "explosion4.13",
        "explosion4.14",
        "explosion4.15",
        "explosion4.16",
    };

    tile_load_arr("explosion", 
                  "explosion_black_and_white",
                  TILE_WIDTH, TILE_HEIGHT,
                  ARRAY_SIZE(explosion_tiles), explosion_tiles);

    static const char *cloudkill_tiles[] = {
        // --------------------------------------------------------------------
        "cloudkill1.1",
        "cloudkill1.2",
        "cloudkill1.3",
        "cloudkill1.4",
        "cloudkill2.1",
        "cloudkill2.2",
        "cloudkill2.3",
        "cloudkill2.4",
        // --------------------------------------------------------------------
        "cloudkill1.5",
        "cloudkill1.6",
        "cloudkill1.7",
        "cloudkill1.8",
        "cloudkill2.5",
        "cloudkill2.6",
        "cloudkill2.7",
        "cloudkill2.8",
        // --------------------------------------------------------------------
        "cloudkill1.9",
        "cloudkill1.10",
        "cloudkill1.11",
        "cloudkill1.12",
        "cloudkill2.9",
        "cloudkill2.10",
        "cloudkill2.11",
        "cloudkill2.12",
        // --------------------------------------------------------------------
        "cloudkill1.13",
        "cloudkill1.14",
        "cloudkill1.15",
        "cloudkill1.16",
        "cloudkill2.13",
        "cloudkill2.14",
        "cloudkill2.15",
        "cloudkill2.16",
        // --------------------------------------------------------------------
        "cloudkill3.1",
        "cloudkill3.2",
        "cloudkill3.3",
        "cloudkill3.4",
        "cloudkill4.1",
        "cloudkill4.2",
        "cloudkill4.3",
        "cloudkill4.4",
        // --------------------------------------------------------------------
        "cloudkill3.5",
        "cloudkill3.6",
        "cloudkill3.7",
        "cloudkill3.8",
        "cloudkill4.5",
        "cloudkill4.6",
        "cloudkill4.7",
        "cloudkill4.8",
        // --------------------------------------------------------------------
        "cloudkill3.9",
        "cloudkill3.10",
        "cloudkill3.11",
        "cloudkill3.12",
        "cloudkill4.9",
        "cloudkill4.10",
        "cloudkill4.11",
        "cloudkill4.12",
        // --------------------------------------------------------------------
        "cloudkill3.13",
        "cloudkill3.14",
        "cloudkill3.15",
        "cloudkill3.16",
        "cloudkill4.13",
        "cloudkill4.14",
        "cloudkill4.15",
        "cloudkill4.16",
    };

    tile_load_arr("cloudkill", 
                  "cloudkill_black_and_white",
                  TILE_WIDTH, TILE_HEIGHT,
                  ARRAY_SIZE(cloudkill_tiles), cloudkill_tiles);

    static const char *poison_tiles[] = {
        // --------------------------------------------------------------------
        "poison1.1",
        "poison1.2",
        "poison1.3",
        "poison1.4",
        "poison2.1",
        "poison2.2",
        "poison2.3",
        "poison2.4",
        // --------------------------------------------------------------------
        "poison1.5",
        "poison1.6",
        "poison1.7",
        "poison1.8",
        "poison2.5",
        "poison2.6",
        "poison2.7",
        "poison2.8",
        // --------------------------------------------------------------------
        "poison1.9",
        "poison1.10",
        "poison1.11",
        "poison1.12",
        "poison2.9",
        "poison2.10",
        "poison2.11",
        "poison2.12",
        // --------------------------------------------------------------------
        "poison1.13",
        "poison1.14",
        "poison1.15",
        "poison1.16",
        "poison2.13",
        "poison2.14",
        "poison2.15",
        "poison2.16",
        // --------------------------------------------------------------------
        "poison3.1",
        "poison3.2",
        "poison3.3",
        "poison3.4",
        "poison4.1",
        "poison4.2",
        "poison4.3",
        "poison4.4",
        // --------------------------------------------------------------------
        "poison3.5",
        "poison3.6",
        "poison3.7",
        "poison3.8",
        "poison4.5",
        "poison4.6",
        "poison4.7",
        "poison4.8",
        // --------------------------------------------------------------------
        "poison3.9",
        "poison3.10",
        "poison3.11",
        "poison3.12",
        "poison4.9",
        "poison4.10",
        "poison4.11",
        "poison4.12",
        // --------------------------------------------------------------------
        "poison3.13",
        "poison3.14",
        "poison3.15",
        "poison3.16",
        "poison4.13",
        "poison4.14",
        "poison4.15",
        "poison4.16",
    };

    tile_load_arr("poison", 
                  "poison_black_and_white",
                  TILE_WIDTH, TILE_HEIGHT,
                  ARRAY_SIZE(poison_tiles), poison_tiles);

    static const char *sawblade_tiles[] = {
        // --------------------------------------------------------------------
        "sawblade1.1",
        "sawblade1.2",
        "sawblade1.3",
        "sawblade1.4",
        "sawblade1.5",
        "sawblade1.6",
        "sawblade1.7",
        "sawblade1.8",
        "sawblade1.9",
        "sawblade1.10",
        "sawblade1.11",
        "sawblade1.12",
        "sawblade1.13",
        "sawblade1.14",
        "sawblade1.15",
        "sawblade1.16",
    };

    tile_load_arr("sawblade", 
                  "sawblade_black_and_white",
                  128, 128,
                  ARRAY_SIZE(sawblade_tiles), sawblade_tiles);

    static const char *sawblade2_tiles[] = {
        // --------------------------------------------------------------------
        "sawblade2.1",
        "sawblade2.2",
        "sawblade2.3",
        "sawblade2.4",
        "sawblade2.5",
        "sawblade2.6",
        "sawblade2.7",
        "sawblade2.8",
        "sawblade2.9",
        "sawblade2.10",
        "sawblade2.11",
        "sawblade2.12",
        "sawblade2.13",
        "sawblade2.14",
        "sawblade2.15",
        "sawblade2.16",
    };

    tile_load_arr("sawblade2", 
                  "sawblade2_black_and_white",
                  128, 128,
                  ARRAY_SIZE(sawblade2_tiles), sawblade2_tiles);

    static const char *sawblade3_tiles[] = {
        // --------------------------------------------------------------------
        "sawblade3.1",
        "sawblade3.2",
        "sawblade3.3",
        "sawblade3.4",
        "sawblade3.5",
        "sawblade3.6",
        "sawblade3.7",
        "sawblade3.8",
        "sawblade3.9",
        "sawblade3.10",
        "sawblade3.11",
        "sawblade3.12",
        "sawblade3.13",
        "sawblade3.14",
        "sawblade3.15",
        "sawblade3.16",
    };

    tile_load_arr("sawblade3", 
                  "sawblade3_black_and_white",
                  128, 128,
                  ARRAY_SIZE(sawblade3_tiles), sawblade3_tiles);

    static const char *sawblade4_tiles[] = {
        // --------------------------------------------------------------------
        "sawblade4.1",
        "sawblade4.2",
        "sawblade4.3",
        "sawblade4.4",
        "sawblade4.5",
        "sawblade4.6",
        "sawblade4.7",
        "sawblade4.8",
        "sawblade4.9",
        "sawblade4.10",
        "sawblade4.11",
        "sawblade4.12",
        "sawblade4.13",
        "sawblade4.14",
        "sawblade4.15",
        "sawblade4.16",
    };

    tile_load_arr("sawblade4", 
                  "sawblade4_black_and_white",
                  128, 128,
                  ARRAY_SIZE(sawblade4_tiles), sawblade4_tiles);

    static const char *sawblade5_tiles[] = {
        // --------------------------------------------------------------------
        "sawblade5.1",
        "sawblade5.2",
        "sawblade5.3",
        "sawblade5.4",
        "sawblade5.5",
        "sawblade5.6",
        "sawblade5.7",
        "sawblade5.8",
        "sawblade5.9",
        "sawblade5.10",
        "sawblade5.11",
        "sawblade5.12",
        "sawblade5.13",
        "sawblade5.14",
        "sawblade5.15",
        "sawblade5.16",
    };

    tile_load_arr("sawblade5", 
                  "sawblade5_black_and_white",
                  128, 128,
                  ARRAY_SIZE(sawblade5_tiles), sawblade5_tiles);

    static const char *anim_man_tiles[] = {
        // --------------------------------------------------------------------
        "anim_man1.1",
        "anim_man1.2",
        "anim_man1.3",
        "anim_man1.4",
        "anim_man1.5",
        "anim_man1.6",
        "anim_man1.7",
        "anim_man1.8",
        "anim_man1.9",
        "anim_man1.10",
        "anim_man1.11",
        "anim_man1.12",
        "anim_man1.13",
        "anim_man1.14",
        "anim_man1.15",
        "anim_man1.16",
    };

    tile_load_arr("anim_man", 
                  0, /* black and white */
                  256, 256,
                  ARRAY_SIZE(anim_man_tiles), anim_man_tiles);

    static const char *anim_treasure_chest_tiles[] = {
        // --------------------------------------------------------------------
        "anim_treasure_chest1.1",
        "anim_treasure_chest1.2",
        "anim_treasure_chest1.3",
        "anim_treasure_chest1.4",
        "anim_treasure_chest1.5",
        "anim_treasure_chest1.6",
        "anim_treasure_chest1.7",
        "anim_treasure_chest1.8",
        "anim_treasure_chest1.9",
        "anim_treasure_chest1.10",
        "anim_treasure_chest1.11",
        "anim_treasure_chest1.12",
        "anim_treasure_chest1.13",
        "anim_treasure_chest1.14",
        "anim_treasure_chest1.15",
        "anim_treasure_chest1.16",
    };

    tile_load_arr("anim_treasure_chest", 
                  0, /* black and white */
                  256, 256,
                  ARRAY_SIZE(anim_treasure_chest_tiles), anim_treasure_chest_tiles);

    static const char *anim_eyes_tiles[] = {
        // --------------------------------------------------------------------
        "anim_eyes1.1",
        "anim_eyes1.2",
        "anim_eyes1.3",
        "anim_eyes1.4",
        "anim_eyes1.5",
        "anim_eyes1.6",
        "anim_eyes1.7",
        "anim_eyes1.8",
        "anim_eyes1.9",
        "anim_eyes1.10",
        "anim_eyes1.11",
        "anim_eyes1.12",
        "anim_eyes1.13",
        "anim_eyes1.14",
        "anim_eyes1.15",
        "anim_eyes1.16",
    };

    tile_load_arr("anim_eyes", 
                  0, /* black and white */
                  128, 128,
                  ARRAY_SIZE(anim_eyes_tiles), anim_eyes_tiles);

    static const char *crystalball_tiles[] = {
        // --------------------------------------------------------------------
        "crystalball.16",
        "crystalball.15",
        "crystalball.14",
        "crystalball.13",
        "crystalball.12",
        "crystalball.11",
        "crystalball.10",
        "crystalball.9",
        "crystalball.8",
        "crystalball.7",
        "crystalball.6",
        "crystalball.5",
        "crystalball.4",
        "crystalball.3",
        "crystalball.2",
        "crystalball.1",
    };

    tile_load_arr("crystalball", 
                  0, /* black and white */
                  256, 256,
                  ARRAY_SIZE(crystalball_tiles), 
                  crystalball_tiles);

    static const char *shield1_tiles[] = {
        // --------------------------------------------------------------------
        "shield1.16",
        "shield1.15",
        "shield1.14",
        "shield1.13",
        "shield1.12",
        "shield1.11",
        "shield1.10",
        "shield1.9",
        "shield1.8",
        "shield1.7",
        "shield1.6",
        "shield1.5",
        "shield1.4",
        "shield1.3",
        "shield1.2",
        "shield1.1",
    };

    tile_load_arr("shield", 
                  0, /* black and white */
                  32, 32,
                  ARRAY_SIZE(shield1_tiles), 
                  shield1_tiles);

    static const char *crystalball_purple_tiles[] = {
        // --------------------------------------------------------------------
        "crystalball_purple.16",
        "crystalball_purple.15",
        "crystalball_purple.14",
        "crystalball_purple.13",
        "crystalball_purple.12",
        "crystalball_purple.11",
        "crystalball_purple.10",
        "crystalball_purple.9",
        "crystalball_purple.8",
        "crystalball_purple.7",
        "crystalball_purple.6",
        "crystalball_purple.5",
        "crystalball_purple.4",
        "crystalball_purple.3",
        "crystalball_purple.2",
        "crystalball_purple.1",
    };

    tile_load_arr("crystalball_purple", 
                  0, /* black and white */
                  256, 256,
                  ARRAY_SIZE(crystalball_purple_tiles), 
                  crystalball_purple_tiles);

    /*
     * Walls
     */
    tp_load(&THING_ROCK1, "data/things/rock1");
    tp_load(&THING_WALL1, "data/things/wall1");
    tp_load(&THING_WALL_LIT1, "data/things/wall_lit1");
    tp_load(&THING_WALL2, "data/things/wall2");
    tp_load(&THING_WALL3, "data/things/wall3");
    tp_load(&THING_WALL4, "data/things/wall4");
    tp_load(&THING_WALL5, "data/things/wall5");
    tp_load(&THING_WALL6, "data/things/wall6");
    tp_load(&THING_FLOOR1, "data/things/floor1");
    tp_load(&THING_FLOOR2, "data/things/floor2");
    tp_load(&THING_FLOOR3, "data/things/floor3");
    tp_load(&THING_FLOOR4, "data/things/floor4");
    tp_load(&THING_FLOOR5, "data/things/floor5");
    tp_load(&THING_FLOOR6, "data/things/floor6");
    tp_load(&THING_DOOR1, "data/things/door1");
    tp_load(&THING_DOOR2, "data/things/door2");
    tp_load(&THING_NOENTRY, "data/things/noentry");
    tp_load(&THING_PIPE, "data/things/pipe");
    tp_load(&THING_BRAZIER, "data/things/brazier");
    tp_load(&THING_EXIT1, "data/things/exit1");
    tp_load(&THING_SAWBLADE1, "data/things/sawblade1");
    tp_load(&THING_SAWBLADE2, "data/things/sawblade2");
    tp_load(&THING_SAWBLADE3, "data/things/sawblade3");
    tp_load(&THING_SAWBLADE4, "data/things/sawblade4");
    tp_load(&THING_SAWBLADE5, "data/things/sawblade5");
#if 0
    tp_load(&THING_EXIT2, "data/things/exit2");
    tp_load(&THING_EXIT3, "data/things/exit3");
    tp_load(&THING_EXIT4, "data/things/exit4");
    tp_load(&THING_EXIT5, "data/things/exit5");
    tp_load(&THING_EXIT6, "data/things/exit6");
    tp_load(&THING_EXIT7, "data/things/exit7");
    tp_load(&THING_GENERATOR1, "data/things/generator1");
#endif
    tp_load(&THING_BONEPILE1, "data/things/bonepile1");
    tp_load(&THING_BONEPILE2, "data/things/bonepile2");
    tp_load(&THING_BONEPILE3, "data/things/bonepile3");
#if 0
    tp_load(&THING_GENERATOR5, "data/things/generator5");
    tp_load(&THING_GENERATOR6, "data/things/generator6");
    tp_load(&THING_GENERATOR7, "data/things/generator7");
#endif
    tp_load(&THING_BOMB, "data/things/bomb");

    /*
     * Projectiles
     */
    tp_load(&THING_ARROW, "data/things/arrow");
    tp_load(&THING_FIREBALL, "data/things/fireball");

    /*
     * Explosions
     */
    tp_load(&THING_WEAPON_HIT1, "data/things/weapon_swing1");

    tp_load(&THING_EXPLOSION1, "data/things/explosion1");
    tp_load(&THING_EXPLOSION2, "data/things/explosion2");
    tp_load(&THING_EXPLOSION3, "data/things/explosion3");
    tp_load(&THING_EXPLOSION4, "data/things/explosion4");
    tp_load(&THING_SHIELD1, "data/things/shield1");

    tp_load(&THING_POISON1, "data/things/poison1");
    tp_load(&THING_POISON2, "data/things/poison2");
    tp_load(&THING_POISON3, "data/things/poison3");
    tp_load(&THING_POISON4, "data/things/poison4");

    tp_load(&THING_CLOUDKILL1, "data/things/cloudkill1");
    tp_load(&THING_CLOUDKILL2, "data/things/cloudkill2");
    tp_load(&THING_CLOUDKILL3, "data/things/cloudkill3");
    tp_load(&THING_CLOUDKILL4, "data/things/cloudkill4");

    tp_load(&THING_HEART, "data/things/heart");
    tp_load(&THING_HIT_SUCCESS, "data/things/hit_success");
    tp_load(&THING_HIT_MISS, "data/things/hit_miss");
    tp_load(&THING_BLOOD1, "data/things/blood1");
    tp_load(&THING_BLOOD2, "data/things/blood2");

    /*
     * Wearable items that are used as weapons. Shown first on list of player 
     * items.
     */
    tp_load(&THING_WAND_FIRE, "data/things/wand_fire");
    tp_load(&THING_SWORD1, "data/things/sword1");
    tp_load(&THING_SWORD2, "data/things/sword2");
    tp_load(&THING_SWORD3, "data/things/sword3");
    tp_load(&THING_SWORD4, "data/things/sword4");
    tp_load(&THING_SWORD5, "data/things/sword5");
    tp_load(&THING_SWORD6, "data/things/sword6");
    tp_load(&THING_AXE1, "data/things/axe1");
    tp_load(&THING_AXE2, "data/things/axe2");
    tp_load(&THING_AXE3, "data/things/axe3");
    tp_load(&THING_AXE4, "data/things/axe4");
    tp_load(&THING_AXE5, "data/things/axe5");
    tp_load(&THING_SCYTHE1, "data/things/scythe1");
    tp_load(&THING_BOW1, "data/things/bow1");
    tp_load(&THING_BOW2, "data/things/bow2");
    tp_load(&THING_BOW3, "data/things/bow3");
    tp_load(&THING_BOW4, "data/things/bow4");

    /*
     * Lesser items like food.
     */
    tp_load(&THING_APPLE1, 
                        "data/things/apple1");
    tp_load(&THING_APPLE2, 
                        "data/things/apple2");
    tp_load(&THING_BREAD, 
                        "data/things/bread");
    tp_load(&THING_CHEESE, 
                        "data/things/cheese");
    tp_load(&THING_BANANA, 
                        "data/things/banana");

    tp_load(&THING_POTION_MONSTICIDE, 
                        "data/things/potion_monsticide");
    tp_load(&THING_POTION_FIRE, 
                        "data/things/potion_fire");
    tp_load(&THING_POTION_CLOUDKILL, 
                        "data/things/potion_cloudkill");
    tp_load(&THING_POTION_LIFE, 
                        "data/things/potion_life");
    tp_load(&THING_POTION_SHIELD, 
                        "data/things/potion_shield");
    tp_load(&THING_WATER, 
                        "data/things/water");
    tp_load(&THING_WATER_POISON, 
                        "data/things/water_poison");

    /*
     * Lower priority junk that cannot be used by clicking on.
     */
    tp_load(&THING_KEY, "data/things/key");
    tp_load(&THING_KEYS2, "data/things/keys2");
    tp_load(&THING_KEYS3, "data/things/keys3");
    tp_load(&THING_TORCH, "data/things/torch");
    tp_load(&THING_TORCHES, "data/things/torches");

    /*
     * Lowest priority junk
     */
    tp_load(&THING_MASK1, "data/things/ring1");
    tp_load(&THING_RING2, "data/things/ring2");
    tp_load(&THING_RING3, "data/things/ring3");
    tp_load(&THING_GEM1, "data/things/gem1");
    tp_load(&THING_GEM2, "data/things/gem2");
    tp_load(&THING_GEM3, "data/things/gem3");
    tp_load(&THING_GEM4, "data/things/gem4");
    tp_load(&THING_GEM5, "data/things/gem5");
    tp_load(&THING_GEM6, "data/things/gem6");
    tp_load(&THING_GEM7, "data/things/gem7");
    tp_load(&THING_COINS1, "data/things/coins1");
    tp_load(&THING_CHEST1, "data/things/chest1");
    tp_load(&THING_SPAM, "data/things/spam");

    /*
     * Heroes. Must come after items they can carry.
     */
    tp_load(&THING_WARRIOR, "data/things/warrior");
    tp_load(&THING_VALKYRIE, "data/things/valkyrie");
    tp_load(&THING_WIZARD, "data/things/wizard");
    tp_load(&THING_ELF, "data/things/elf");
    tp_load(&THING_DWARF, "data/things/dwarf");

    /*
     * Monsters
     */
    tp_load(&THING_GHOST1, "data/things/ghost1");
    tp_load(&THING_GHOST2, "data/things/ghost2");
    tp_load(&THING_GHOST3, "data/things/ghost3");
    tp_load(&THING_EYEBAWL, "data/things/eyebawl");
#if 0
    tp_load(&THING_DEMON, "data/things/demon");
    tp_load(&THING_MONK, "data/things/monk");
    tp_load(&THING_LIZARD, "data/things/lizard");
#endif
    tp_load(&THING_DEATH, "data/things/death");

    /*
     * Weapon animations.
     */
    tp_load(&THING_WAND1_ANIM, "data/things/wand1_anim");
    tp_load(&THING_SWORD1_ANIM, "data/things/sword1_anim");
    tp_load(&THING_SWORD2_ANIM, "data/things/sword2_anim");
    tp_load(&THING_SWORD3_ANIM, "data/things/sword3_anim");
    tp_load(&THING_AXE1_ANIM, "data/things/axe1_anim");
    tp_load(&THING_AXE2_ANIM, "data/things/axe2_anim");
    tp_load(&THING_BOW1_ANIM, "data/things/bow1_anim");
    tp_load(&THING_SCYTHE1_ANIM, "data/things/scythe1_anim");

    /*
     * Intro animations.
     */
    tp_load(&THING_ANIM_MAN, "data/things/anim_man");
    tp_load(&THING_ANIM_TREASURE_CHEST, "data/things/anim_treasure_chest");
    tp_load(&THING_ANIM_EYES, "data/things/anim_eyes");

    tp_load(&THING_ACTION_LEFT, "data/things/action_left");
    tp_load(&THING_ACTION_RIGHT, "data/things/action_right");
    tp_load(&THING_ACTION_UP, "data/things/action_up");
    tp_load(&THING_ACTION_DOWN, "data/things/action_down");
    tp_load(&THING_ACTION_SLEEP, "data/things/action_sleep");
    tp_load(&THING_ACTION_TEXT, "data/things/action_text");
    tp_load(&THING_ACTION_ZAP, "data/things/action_zap");
    tp_load(&THING_ACTION_TRIGGER_MONST, "data/things/action_trigger_on_monst");
    tp_load(&THING_ACTION_TRIGGER_HERO, "data/things/action_trigger_on_hero");
    tp_load(&THING_ACTION_TRIGGER_WALL, "data/things/action_trigger_on_wall");

    return (true);
}

uint8_t resource_init (void)
{
    if (tex_load("data/gfx/window_gothic_wide.png", "gothic_wide") == 0) {
        DIE("tex init");
    }

    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)resource_init_1,
                          0, "resource_init_1");
    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)resource_init_1a,
                          0, "resource_init_1a");
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
    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)resource_init_24,
                          0, "resource_init_24");
    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)resource_init_25,
                          0, "resource_init_25");
    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)resource_init_26,
                          0, "resource_init_26");
    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)resource_init_27,
                          0, "resource_init_27");
    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)resource_init_28,
                          0, "resource_init_28");
    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)resource_init_29,
                          0, "resource_init_29");
    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)resource_init_30,
                          0, "resource_init_30");
    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)resource_init_31,
                          0, "resource_init_31");
    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)resource_init_32,
                          0, "resource_init_32");
    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)resource_init_33,
                          0, "resource_init_33");
    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)resource_init_34,
                          0, "resource_init_34");
    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)resource_init_35,
                          0, "resource_init_35");
    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)resource_init_36,
                          0, "resource_init_36");
    action_init_fn_create(&init_fns,
                          (action_init_fn_callback)resource_init_last,
                          0, "resource_init_last");

    return (true);
}
