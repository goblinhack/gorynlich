/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <SDL.h>

#include "main.h"
#include "map.h"
#include "wid.h"
#include "thing.h"
#include "thing_template.h"
#include "thing_tile.h"
#include "level.h"
#include "level_private.h"
#include "tile.h"
#include "bits.h"

typedef boolean (*map_is_at_callback)(thing_templatep);

map_t *MAP;

static void map_init_bounds(map_t *map,
                            uint32_t map_width,
                            uint32_t map_height);
static void map_fixup(map_t *map);

/*
 * map_init
 */
boolean map_init (void)
{
    MAP = myzalloc(sizeof(map_t), "map frame");

    map_init_bounds(MAP, MAP_WIDTH, MAP_HEIGHT);

    map_display_init(MAP);

    map_init_tiles(MAP);

    map_fixup(MAP);

    MAP->lx = (MAP_WIDTH / 2) * TILE_SCREEN_WIDTH;
    MAP->ly = (MAP_HEIGHT / 2) * TILE_SCREEN_HEIGHT;

    map_move_delta_pixels(0, 0);

    map_display_wid_init();

    return (true);
}

/*
 * map_fini
 */
void map_fini (void)
{
    if (MAP) {
        myfree(MAP);
    }

    map_display_wid_fini();
}

/*
 * map_init_bounds
 */
static void map_init_bounds (map_t *map,
                             uint32_t map_width,
                             uint32_t map_height)
{
    uint32_t width = global_config.video_pix_width;
    uint32_t height = global_config.video_pix_height;

    /*
     * Allocate space for the tiles.
     */
    map->map_width = map_width;
    map->map_height = map_height;

    map->tiles_per_screen_x = width / TILE_SCREEN_WIDTH;
    map->tiles_per_screen_y = height / (TILE_SCREEN_HEIGHT / 2);

    /*
     * Absolute map bounds.
     */
    map->min_px = 0;
    map->max_px = map->map_width - map->tiles_per_screen_x;
    map->min_py = 0;
    map->max_py = map->map_height - map->tiles_per_screen_y;

    map->min_px *= TILE_SCREEN_WIDTH;
    map->max_px *= TILE_SCREEN_WIDTH;
    map->min_py *= TILE_SCREEN_HEIGHT;
    map->max_py *= TILE_SCREEN_HEIGHT;

    /*
     * Where we start off on the map.
     */
    map->px = map->map_width / 2;
    map->px -= map->tiles_per_screen_x / 2;
    map->px *= TILE_SCREEN_WIDTH;

    map->py = map->map_height / 2;
    map->py -= map->tiles_per_screen_y / 2;
    map->py *= TILE_SCREEN_HEIGHT;
}

/*
 * map_move_delta_pixels
 *
 * Shift the map by some pixels.
 */
void map_move_delta_pixels (int32_t dx, int32_t dy)
{
    uint32_t lx;
    uint32_t ly;

    if (MAP->px + dx < MAP->min_px) {
        dx = 0;
    }

    if (MAP->px + dx >= MAP->max_px - 1) {
        dx = 0;
    }

    if (MAP->py + dy < MAP->min_py) {
        dy = 0;
    }

    if (MAP->py + dy >= MAP->max_py - 1) {
        dy = 0;
    }

    MAP->px += dx;
    MAP->py += dy;

    lx = MAP->lx / TILE_SCREEN_WIDTH;
    ly = MAP->ly / TILE_SCREEN_HEIGHT;

map_set_thing_template(MAP, lx, ly, 0);

    MAP->lx += dx;
    MAP->ly += dy;

    lx = MAP->lx / TILE_SCREEN_WIDTH;
    ly = MAP->ly / TILE_SCREEN_HEIGHT;

    map_lightmap(MAP, lx, ly,
                 DEFAULT_LIGHT_RAY_LENGTH,
                 true);

    map_light_radiant(MAP, lx, ly);
map_set_thing_template(MAP, lx, ly, PLAYER_0_ID);
}

static boolean map_is_x_at (levelp level,
                            int32_t x, int32_t y, map_is_at_callback callback)
{
    thing_templatep thing_template;
    mapp map;

    map = level->map;

    thing_template = map_get_thing_template(map, x, y);
    if (thing_template) {
        if ((*callback)(thing_template)) {
            return (true);
        }
    }
    
    thingp thing_it = thing_get_map_first(map, x, y);
    while (thing_it) {
        if (thing_item_owner(thing_it)) {
            thing_it = thing_get_map_next(map, thing_it);
            continue;
        }

        if (thing_is_dead(thing_it)) {
            thing_it = thing_get_map_next(map, thing_it);
            continue;
        }

        thing_template = thing_get_template(thing_it);
        if (thing_template) {
            if ((*callback)(thing_template)) {
                return (true);
            }
        }

        thing_it = thing_get_map_next(map, thing_it);
    }

    return (false);
}

static boolean map_count_x_at (levelp level,
                               int32_t x, int32_t y,
                               map_is_at_callback callback)
{
    thing_templatep thing_template;
    uint32_t count;
    mapp map;

    count = 0;

    map = level->map;

    thing_template = map_get_thing_template(map, x, y);
    if (thing_template) {
        if ((*callback)(thing_template)) {
            count++;
        }
    }
    
    thingp thing_it = thing_get_map_first(map, x, y);
    while (thing_it) {
        if (thing_item_owner(thing_it)) {
            thing_it = thing_get_map_next(map, thing_it);
            continue;
        }

        if (thing_is_dead(thing_it)) {
            thing_it = thing_get_map_next(map, thing_it);
            continue;
        }

        thing_template = thing_get_template(thing_it);
        if (thing_template) {
            if ((*callback)(thing_template)) {
                count++;
            }
        }

        thing_it = thing_get_map_next(map, thing_it);
    }

    return (count);
}

boolean map_is_player_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_player));
}

boolean map_is_monst_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_monst));
}

boolean map_is_snail_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_snail));
}

boolean map_is_wall_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_wall));
}

boolean map_is_star_yellow_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_star_yellow));
}

boolean map_is_rock_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_rock));
}

boolean map_is_water_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_water));
}

boolean map_is_lava_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_lava));
}

boolean map_is_xxx4_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_xxx4));
}

boolean map_is_xxx5_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_xxx5));
}

boolean map_is_xxx6_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_xxx6));
}

boolean map_is_xxx7_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_xxx7));
}

boolean map_is_xxx8_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_xxx8));
}

boolean map_is_star_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_star));
}

boolean map_is_powerup_spam_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_powerup_spam));
}

boolean map_is_rock_0_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_rock_0));
}

boolean map_is_car_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_car));
}

boolean map_is_transparent_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_transparent));
}

boolean map_is_radiant_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_radiant));
}

boolean map_is_star_black_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_star_black));
}

boolean map_is_star_purple_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_star_purple));
}

boolean map_is_explosion_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_explosion));
}

boolean map_is_spikes_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_spikes));
}

boolean map_is_star_red_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_star_red));
}

boolean map_is_star_blue_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_star_blue));
}

boolean map_is_seedpod_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_seedpod));
}

boolean map_is_bomb_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_bomb));
}

boolean map_is_spam_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_spam));
}

boolean map_is_road_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_road));
}

boolean map_is_pipe_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_pipe));
}

boolean map_is_item_removed_at_level_end_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_item_removed_at_level_end));
}

boolean map_is_scarable_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_scarable));
}

boolean map_is_shrunk_when_carried_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_shrunk_when_carried));
}

boolean map_is_hidden_from_editor_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_hidden_from_editor));
}

boolean map_is_animated_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_animated));
}

boolean map_is_follows_owner_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_follows_owner));
}

boolean map_is_powerup_rocket_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_powerup_rocket));
}

boolean map_is_left_as_corpse_on_death_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_left_as_corpse_on_death));
}

boolean map_is_item_perma_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_item_perma));
}

boolean map_is_esnail_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_esnail));
}

boolean map_is_item_hidden_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_item_hidden));
}

boolean map_is_bonus_letter_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_bonus_letter));
}

boolean map_is_thing_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_thing));
}

boolean map_is_plant_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_plant));
}

boolean map_is_food_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_food));
}

boolean map_is_floor_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_floor));
}

boolean map_is_exit_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_exit));
}

boolean map_is_letter_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_letter));
}

static thingp map_thing_is_x_at (levelp level,
                                 int32_t x, int32_t y,
                                 map_is_at_callback callback)
{
    thing_templatep thing_template;
    mapp map;

    map = level->map;

    thing_template = map_get_thing_template(map, x, y);
    if (thing_template) {
        if ((*callback)(thing_template)) {
            return (0);
        }
    }
    
    thingp thing_it = thing_get_map_first(map, x, y);
    while (thing_it) {
        if (thing_item_owner(thing_it)) {
            thing_it = thing_get_map_next(map, thing_it);
            continue;
        }

        if (thing_is_dead(thing_it)) {
            thing_it = thing_get_map_next(map, thing_it);
            continue;
        }

        thing_template = thing_get_template(thing_it);
        if (thing_template) {
            if ((*callback)(thing_template)) {
                return (thing_it);
            }
        }

        thing_it = thing_get_map_next(map, thing_it);
    }

    return (0);
}

thingp map_thing_is_player_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_player));
}

thingp map_thing_is_monst_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_monst));
}

thingp map_thing_is_snail_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_snail));
}

thingp map_thing_is_wall_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_wall));
}

thingp map_thing_is_star_yellow_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_star_yellow));
}

thingp map_thing_is_rock_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_rock));
}

thingp map_thing_is_water_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_water));
}

thingp map_thing_is_lava_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_lava));
}

thingp map_thing_is_xxx4_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_xxx4));
}

thingp map_thing_is_xxx5_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_xxx5));
}

thingp map_thing_is_xxx6_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_xxx6));
}

thingp map_thing_is_xxx7_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_xxx7));
}

thingp map_thing_is_xxx8_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_xxx8));
}

thingp map_thing_is_star_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_star));
}

thingp map_thing_is_powerup_spam_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_powerup_spam));
}

thingp map_thing_is_rock_0_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_rock_0));
}

thingp map_thing_is_car_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_car));
}

thingp map_thing_is_transparent_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_transparent));
}

thingp map_thing_is_radiant_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_radiant));
}

thingp map_thing_is_star_black_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_star_black));
}

thingp map_thing_is_star_purple_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_star_purple));
}

thingp map_thing_is_explosion_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_explosion));
}

thingp map_thing_is_spikes_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_spikes));
}

thingp map_thing_is_star_red_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_star_red));
}

thingp map_thing_is_star_blue_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_star_blue));
}

thingp map_thing_is_seedpod_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_seedpod));
}

thingp map_thing_is_bomb_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_bomb));
}

thingp map_thing_is_spam_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_spam));
}

thingp map_thing_is_road_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_road));
}

thingp map_thing_is_pipe_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_pipe));
}

thingp map_thing_is_item_removed_at_level_end_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_item_removed_at_level_end));
}

thingp map_thing_is_scarable_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_scarable));
}

thingp map_thing_is_shrunk_when_carried_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_shrunk_when_carried));
}

thingp map_thing_is_hidden_from_editor_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_hidden_from_editor));
}

thingp map_thing_is_animated_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_animated));
}

thingp map_thing_is_follows_owner_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_follows_owner));
}

thingp map_thing_is_powerup_rocket_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_powerup_rocket));
}

thingp map_thing_is_left_as_corpse_on_death_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_left_as_corpse_on_death));
}

thingp map_thing_is_item_perma_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_item_perma));
}

thingp map_thing_is_esnail_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_esnail));
}

thingp map_thing_is_item_hidden_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_item_hidden));
}

thingp map_thing_is_bonus_letter_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_bonus_letter));
}

thingp map_thing_is_thing_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_thing));
}

thingp map_thing_is_plant_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_plant));
}

thingp map_thing_is_food_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_food));
}

thingp map_thing_is_floor_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_floor));
}

thingp map_thing_is_exit_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_exit));
}

thingp map_thing_is_letter_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_letter));
}

/*
 * Join up adjactent blocks.
 */
static void 
map_fixup (map_t *map)
{
    int32_t index;
    tilep tile;
    int32_t x;
    int32_t y;
    int32_t dx;
    int32_t dy;
    uint16_t nbrs[3][3];

    for (x = 1; x < MAP_WIDTH-1; x++) {
        for (y = 1; y < MAP_HEIGHT-1; y++) {

            uint16_t template_id = map->tiles[x][y].template_id;

            if (!template_id) {
                continue;
            }

            if ((template_id != ROCK_0_ID) &&
                (template_id != ROCK_1_ID) &&
                (template_id != LAVA_0_ID) &&
                (template_id != WATER_0_ID) &&
                (template_id != WATER_0_ID) &&
                (template_id != BRICK_0_ID) &&
                (template_id != BRICK_1_ID) &&
                (template_id != BRICK_2_ID) &&
                (template_id != BRICK_3_ID) &&
                (template_id != BRICK_4_ID) &&
                (template_id != BRICK_5_ID) &&
                (template_id != BRICK_6_ID) &&
                (template_id != BRICK_7_ID) &&
                (template_id != BRICK_8_ID)) {
                continue;
            }

            for (dx = -1; dx <= 1; dx++) {
                for (dy = -1; dy <= 1; dy++) {

                    uint16_t a_thing_template_id = 
                        map->tiles[x + dx][y + dy].template_id;

                    if ((a_thing_template_id != ROCK_0_ID) &&
                        (a_thing_template_id != ROCK_1_ID) &&
                        (a_thing_template_id != LAVA_0_ID) &&
                        (a_thing_template_id != WATER_0_ID) &&
                        (a_thing_template_id != BRICK_0_ID) &&
                        (a_thing_template_id != BRICK_1_ID) &&
                        (a_thing_template_id != BRICK_2_ID) &&
                        (a_thing_template_id != BRICK_3_ID) &&
                        (a_thing_template_id != BRICK_4_ID) &&
                        (a_thing_template_id != BRICK_5_ID) &&
                        (a_thing_template_id != BRICK_6_ID) &&
                        (a_thing_template_id != BRICK_7_ID) &&
                        (a_thing_template_id != BRICK_8_ID)) {
                        nbrs[dx + 1][dy + 1] = 0;
                        continue;
                    }

                    nbrs[dx + 1][dy + 1] = a_thing_template_id;
                }
            }

            uint16_t a = nbrs[0][0];
            uint16_t b = nbrs[1][0];
            uint16_t c = nbrs[2][0];
            uint16_t d = nbrs[0][1];
            uint16_t e = nbrs[1][1];
            uint16_t f = nbrs[2][1];
            uint16_t g = nbrs[0][2];
            uint16_t h = nbrs[1][2];
            uint16_t i = nbrs[2][2];

            uint8_t A = (a != 0) ? 1 : 0;
            uint8_t B = (b != 0) ? 1 : 0;
            uint8_t C = (c != 0) ? 1 : 0;
            uint8_t D = (d != 0) ? 1 : 0;
            uint8_t E = (e != 0) ? 1 : 0;
            uint8_t F = (f != 0) ? 1 : 0;
            uint8_t G = (g != 0) ? 1 : 0;
            uint8_t H = (h != 0) ? 1 : 0;
            uint8_t I = (i != 0) ? 1 : 0;

            const uint16_t omask =
                (I << 8) | (H << 7) | (G << 6) | (F << 5) |
                (E << 4) | (D << 3) | (C << 2) | (B << 1) |
                (A << 0);

            uint8_t score;
            uint8_t best = 0;

            index = -1;

            uint16_t mask;

#define BLOCK(a,b,c,d,e,f,g,h,i, _index_)                                   \
            mask =                                                      \
                (i << 8) | (h << 7) | (g << 6) | (f << 5) |             \
                (e << 4) | (d << 3) | (c << 2) | (b << 1) |             \
                (a << 0);                                               \
                                                                        \
            if ((mask & omask) == mask) {                               \
                uint32_t difference = mask ^ omask;                     \
                BITCOUNT(difference);                                   \
                score = 32 - difference;                                \
                if (score > best) {                                     \
                    best = score;                                       \
                    index = _index_;                                    \
                }                                                       \
            }                                                           \

            BLOCK(1, 1, 1,
                    1, 1, 1,
                    1, 1, 1, IS_JOIN_BLOCK)

            BLOCK(0, 0, 0,
                    0, 1, 0,
                    0, 0, 0, IS_JOIN_NODE)

            BLOCK(0, 0, 0,
                    0, 1, 1,
                    0, 0, 0, IS_JOIN_LEFT)

            BLOCK(0, 0, 0,
                    0, 1, 0,
                    0, 1, 0, IS_JOIN_TOP)

            BLOCK(0, 0, 0,
                    1, 1, 0,
                    0, 0, 0, IS_JOIN_RIGHT)

            BLOCK(0, 1, 0,
                    0, 1, 0,
                    0, 0, 0, IS_JOIN_BOT)

            BLOCK(0, 0, 0,
                    1, 1, 1,
                    0, 0, 0, IS_JOIN_HORIZ)

            BLOCK(0, 1, 0,
                    0, 1, 0,
                    0, 1, 0, IS_JOIN_VERT)

            BLOCK(0, 0, 0,
                    0, 1, 1,
                    0, 1, 1, IS_JOIN_TL2)

            BLOCK(0, 1, 1,
                    0, 1, 1,
                    0, 0, 0, IS_JOIN_BL2)

            BLOCK(1, 1, 0,
                    1, 1, 0,
                    0, 0, 0, IS_JOIN_BR2)

            BLOCK(0, 0, 0,
                    1, 1, 0,
                    1, 1, 0, IS_JOIN_TR2)

            BLOCK(0, 0, 0,
                    0, 1, 1,
                    0, 1, 0, IS_JOIN_TL)

            BLOCK(0, 1, 0,
                    0, 1, 1,
                    0, 0, 0, IS_JOIN_BL)

            BLOCK(0, 1, 0,
                    1, 1, 0,
                    0, 0, 0, IS_JOIN_BR)

            BLOCK(0, 0, 0,
                    1, 1, 0,
                    0, 1, 0, IS_JOIN_TR)

            BLOCK(1, 1, 0,
                    1, 1, 0,
                    1, 1, 0, IS_JOIN_T90_3)

            BLOCK(1, 1, 1,
                    1, 1, 1,
                    0, 0, 0, IS_JOIN_T180_3)

            BLOCK(0, 1, 1,
                    0, 1, 1,
                    0, 1, 1, IS_JOIN_T270_3)

            BLOCK(0, 0, 0,
                    1, 1, 1,
                    1, 1, 1, IS_JOIN_T_3)

            BLOCK(0, 1, 0,
                    0, 1, 1,
                    0, 1, 0, IS_JOIN_T270)

            BLOCK(0, 1, 0,
                    1, 1, 1,
                    0, 0, 0, IS_JOIN_T180)

            BLOCK(0, 1, 0,
                    1, 1, 0,
                    0, 1, 0, IS_JOIN_T90)

            BLOCK(0, 0, 0,
                    1, 1, 1,
                    0, 1, 0, IS_JOIN_T)

            BLOCK(0, 1, 1,
                    0, 1, 1,
                    0, 1, 0, IS_JOIN_T270_2)

            BLOCK(1, 1, 0,
                    1, 1, 1,
                    0, 0, 0, IS_JOIN_T180_2)

            BLOCK(0, 1, 0,
                    1, 1, 0,
                    1, 1, 0, IS_JOIN_T90_2)

            BLOCK(0, 0, 0,
                    1, 1, 1,
                    0, 1, 1, IS_JOIN_T_2)

            BLOCK(0, 1, 0,
                    0, 1, 1,
                    0, 1, 1, IS_JOIN_T270_1)

            BLOCK(0, 1, 1,
                    1, 1, 1,
                    0, 0, 0, IS_JOIN_T180_1)

            BLOCK(1, 1, 0,
                    1, 1, 0,
                    0, 1, 0, IS_JOIN_T90_1)

            BLOCK(0, 0, 0,
                    1, 1, 1,
                    1, 1, 0, IS_JOIN_T_1)

            BLOCK(0, 1, 0,
                    1, 1, 1,
                    0, 1, 0, IS_JOIN_X)

            BLOCK(0, 1, 0,
                    1, 1, 1,
                    0, 1, 1, IS_JOIN_X1)

            BLOCK(0, 1, 1,
                    1, 1, 1,
                    0, 1, 0, IS_JOIN_X1_270)

            BLOCK(1, 1, 0,
                    1, 1, 1,
                    0, 1, 0, IS_JOIN_X1_180)

            BLOCK(0, 1, 0,
                    1, 1, 1,
                    1, 1, 0, IS_JOIN_X1_90)

            BLOCK(0, 1, 0,
                    1, 1, 1,
                    1, 1, 1, IS_JOIN_X2)

            BLOCK(0, 1, 1,
                    1, 1, 1,
                    0, 1, 1, IS_JOIN_X2_270)

            BLOCK(1, 1, 1,
                    1, 1, 1,
                    0, 1, 0, IS_JOIN_X2_180)

            BLOCK(1, 1, 0,
                    1, 1, 1,
                    1, 1, 0, IS_JOIN_X2_90)

            BLOCK(0, 1, 1,
                    1, 1, 1,
                    1, 1, 0, IS_JOIN_X3)

            BLOCK(1, 1, 0,
                    1, 1, 1,
                    0, 1, 1, IS_JOIN_X3_180)

            BLOCK(0, 1, 1,
                    1, 1, 1,
                    1, 1, 1, IS_JOIN_X4)

            BLOCK(1, 1, 1,
                    1, 1, 1,
                    0, 1, 1, IS_JOIN_X4_270)

            BLOCK(1, 1, 1,
                    1, 1, 1,
                    1, 1, 0, IS_JOIN_X4_180)

            BLOCK(1, 1, 0,
                    1, 1, 1,
                    1, 1, 1, IS_JOIN_X4_90)

            if (index == -1) {
                DIE("%u%u%u %u%u%u %u%u%u not handled",
                    a ? 1 : 0,
                    b ? 1 : 0,
                    c ? 1 : 0,
                    d ? 1 : 0,
                    e ? 1 : 0,
                    f ? 1 : 0,
                    g ? 1 : 0,
                    h ? 1 : 0,
                    i ? 1 : 0);
            }

            thing_templatep t = id_to_thing_template(e);

            thing_tilep thing_tile = thing_tile_find(t, index, &tile);
            if (!thing_tile) {
                index = IS_JOIN_BLOCK;

                thing_tile_find(t, index, &tile);
                if (!tile) {
                    DIE("no joinable tile for %s", thing_template_name(t));
                }
            }

            if (!tile) {
                DIE("no tile for %s", thing_template_name(t));
            }

            uint32_t tile_index = tile_get_index(tile);

            map->tiles[x][y].tile = tile_index;
            map->tiles[x][y].template_id = e;
        }
    }
}

static uint32_t level_count_is_x (levelp level, map_is_at_callback callback)
{
    uint32_t count;
    int32_t x;
    int32_t y;

    count = 0;

    for (x = 0; x < TILES_MAP_EDITABLE_WIDTH; x++) {
        for (y = 0; y < TILES_MAP_EDITABLE_HEIGHT; y++) {
            count += map_count_x_at(level, x, y, callback);
        }
    }

    return (count);
}

static char this_pipe[TILES_MAP_EDITABLE_WIDTH][TILES_MAP_EDITABLE_HEIGHT];

static void pipe_flood (levelp level, int32_t x, int32_t y)
{
    if (!map_thing_is_pipe_at(level, x, y)) {
        return;
    }

    if (this_pipe[x][y] == '+') {
        return;
    }

    this_pipe[x][y] = '+';

    pipe_flood(level, x-1, y);
    pipe_flood(level, x+1, y);
    pipe_flood(level, x, y-1);
    pipe_flood(level, x, y+1);
}

boolean level_pipe_find_exit (levelp level,
                              int32_t ix, int32_t iy,
                              int32_t *exit_x, int32_t *exit_y)
{
    int32_t exits_x[TILES_MAP_EDITABLE_WIDTH];
    int32_t exits_y[TILES_MAP_EDITABLE_WIDTH];
    int32_t nexits;
    int32_t x;
    int32_t y;

    memset(this_pipe, ' ', sizeof(this_pipe));
    memset(exits_x, 0, sizeof(exits_x));
    memset(exits_y, 0, sizeof(exits_y));
    nexits = 0;

    pipe_flood(level, ix, iy);

    for (x = 1; x < TILES_MAP_EDITABLE_WIDTH-1; x++) {
        for (y = 1; y < TILES_MAP_EDITABLE_HEIGHT-1; y++) {

            if ((x == ix) && (y == iy)) {
                continue;
            }

            if (this_pipe[x][y] != '+') {
                continue;
            }

            if (level->end_pipe[x][y] != ' ') {
                exits_x[nexits] = x;
                exits_y[nexits] = y;
                nexits++;
            }
        }
    }

    if (!nexits) {
        return (false);
    }

    int32_t exit = rand() % nexits;

    *exit_x = exits_x[exit];
    *exit_y = exits_y[exit];

    return (true);
}

uint32_t level_count_is_player (levelp level)
{
    return (level_count_is_x(level, thing_template_is_player));
}

uint32_t level_count_is_monst (levelp level)
{
    return (level_count_is_x(level, thing_template_is_monst));
}

uint32_t level_count_is_snail (levelp level)
{
    return (level_count_is_x(level, thing_template_is_snail));
}

uint32_t level_count_is_wall (levelp level)
{
    return (level_count_is_x(level, thing_template_is_wall));
}

uint32_t level_count_is_star_yellow (levelp level)
{
    return (level_count_is_x(level, thing_template_is_star_yellow));
}

uint32_t level_count_is_rock (levelp level)
{
    return (level_count_is_x(level, thing_template_is_rock));
}

uint32_t level_count_is_water (levelp level)
{
    return (level_count_is_x(level, thing_template_is_water));
}

uint32_t level_count_is_lava (levelp level)
{
    return (level_count_is_x(level, thing_template_is_lava));
}

uint32_t level_count_is_xxx4 (levelp level)
{
    return (level_count_is_x(level, thing_template_is_xxx4));
}

uint32_t level_count_is_xxx5 (levelp level)
{
    return (level_count_is_x(level, thing_template_is_xxx5));
}

uint32_t level_count_is_xxx6 (levelp level)
{
    return (level_count_is_x(level, thing_template_is_xxx6));
}

uint32_t level_count_is_xxx7 (levelp level)
{
    return (level_count_is_x(level, thing_template_is_xxx7));
}

uint32_t level_count_is_xxx8 (levelp level)
{
    return (level_count_is_x(level, thing_template_is_xxx8));
}

uint32_t level_count_is_star (levelp level)
{
    return (level_count_is_x(level, thing_template_is_star));
}

uint32_t level_count_is_powerup_spam (levelp level)
{
    return (level_count_is_x(level, thing_template_is_powerup_spam));
}

uint32_t level_count_is_rock_0 (levelp level)
{
    return (level_count_is_x(level, thing_template_is_rock_0));
}

uint32_t level_count_is_car (levelp level)
{
    return (level_count_is_x(level, thing_template_is_car));
}

uint32_t level_count_is_transparent (levelp level)
{
    return (level_count_is_x(level, thing_template_is_transparent));
}

uint32_t level_count_is_radiant (levelp level)
{
    return (level_count_is_x(level, thing_template_is_radiant));
}

uint32_t level_count_is_star_black (levelp level)
{
    return (level_count_is_x(level, thing_template_is_star_black));
}

uint32_t level_count_is_star_purple (levelp level)
{
    return (level_count_is_x(level, thing_template_is_star_purple));
}

uint32_t level_count_is_explosion (levelp level)
{
    return (level_count_is_x(level, thing_template_is_explosion));
}

uint32_t level_count_is_spikes (levelp level)
{
    return (level_count_is_x(level, thing_template_is_spikes));
}

uint32_t level_count_is_star_red (levelp level)
{
    return (level_count_is_x(level, thing_template_is_star_red));
}

uint32_t level_count_is_star_blue (levelp level)
{
    return (level_count_is_x(level, thing_template_is_star_blue));
}

uint32_t level_count_is_seedpod (levelp level)
{
    return (level_count_is_x(level, thing_template_is_seedpod));
}

uint32_t level_count_is_bomb (levelp level)
{
    return (level_count_is_x(level, thing_template_is_bomb));
}

uint32_t level_count_is_spam (levelp level)
{
    return (level_count_is_x(level, thing_template_is_spam));
}

uint32_t level_count_is_road (levelp level)
{
    return (level_count_is_x(level, thing_template_is_road));
}

uint32_t level_count_is_pipe (levelp level)
{
    return (level_count_is_x(level, thing_template_is_pipe));
}

uint32_t level_count_is_item_removed_at_level_end (levelp level)
{
    return (level_count_is_x(level, thing_template_is_item_removed_at_level_end));
}

uint32_t level_count_is_scarable (levelp level)
{
    return (level_count_is_x(level, thing_template_is_scarable));
}

uint32_t level_count_is_shrunk_when_carried (levelp level)
{
    return (level_count_is_x(level, thing_template_is_shrunk_when_carried));
}

uint32_t level_count_is_hidden_from_editor (levelp level)
{
    return (level_count_is_x(level, thing_template_is_hidden_from_editor));
}

uint32_t level_count_is_follows_owner (levelp level)
{
    return (level_count_is_x(level, thing_template_is_follows_owner));
}

uint32_t level_count_is_powerup_rocket (levelp level)
{
    return (level_count_is_x(level, thing_template_is_powerup_rocket));
}

uint32_t level_count_is_left_as_corpse_on_death (levelp level)
{
    return (level_count_is_x(level, thing_template_is_left_as_corpse_on_death));
}

uint32_t level_count_is_item_perma (levelp level)
{
    return (level_count_is_x(level, thing_template_is_item_perma));
}

uint32_t level_count_is_esnail (levelp level)
{
    return (level_count_is_x(level, thing_template_is_esnail));
}

uint32_t level_count_is_item_is_hidden (levelp level)
{
    return (level_count_is_x(level, thing_template_is_item_hidden));
}

uint32_t level_count_is_bonus_letter (levelp level)
{
    return (level_count_is_x(level, thing_template_is_bonus_letter));
}

uint32_t level_count_is_thing (levelp level)
{
    return (level_count_is_x(level, thing_template_is_thing));
}

uint32_t level_count_is_plant (levelp level)
{
    return (level_count_is_x(level, thing_template_is_plant));
}

uint32_t level_count_is_food (levelp level)
{
    return (level_count_is_x(level, thing_template_is_food));
}

uint32_t level_count_is_floor (levelp level)
{
    return (level_count_is_x(level, thing_template_is_floor));
}

uint32_t level_count_is_exit (levelp level)
{
    return (level_count_is_x(level, thing_template_is_exit));
}

uint32_t level_count_is_letter (levelp level)
{
    return (level_count_is_x(level, thing_template_is_letter));
}
