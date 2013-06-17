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

typedef boolean (*map_is_at_callback)(thing_templatep);

map_frame_ctx_t *map_ctx;

static void map_init_bounds(map_frame_ctx_t *map,
                            uint32_t map_width,
                            uint32_t map_height);
static void map_fixup(map_frame_ctx_t *map);

/*
 * map_init
 */
boolean map_init (void)
{
    map_ctx = myzalloc(sizeof(map_frame_ctx_t), "map frame");

    map_init_bounds(map_ctx, MAP_WIDTH, MAP_HEIGHT);

    map_display_init(map_ctx);

    map_init_tiles(map_ctx);

    map_fixup(map_ctx);

    map_ctx->lx = (MAP_WIDTH / 2) * TILE_WIDTH;
    map_ctx->ly = (MAP_HEIGHT / 2) * TILE_HEIGHT;
    map_ctx->lz = 1;

    map_move_delta_pixels(0, 0);

    map_display_wid_init();

    return (true);
}

/*
 * map_fini
 */
void map_fini (void)
{
    if (map_ctx) {
        myfree(map_ctx);
    }

    map_display_wid_fini();
}

/*
 * map_init_bounds
 */
static void map_init_bounds (map_frame_ctx_t *map,
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

    map->tiles_per_screen_x = width / TILE_WIDTH;
    map->tiles_per_screen_y = height / TILE_WIDTH;

    /*
     * Absolute map bounds.
     */
    map->min_px = 0;
    map->max_px = map->map_width - map->tiles_per_screen_x;
    map->min_py = 0;
    map->max_py = map->map_height - map->tiles_per_screen_y;

    map->min_px *= TILE_WIDTH;
    map->max_px *= TILE_WIDTH;
    map->min_py *= TILE_HEIGHT;
    map->max_py *= TILE_HEIGHT;

    /*
     * Where we start off on the map.
     */
    map->px = map->map_width / 2;
    map->px -= map->tiles_per_screen_x / 2;
    map->px *= TILE_WIDTH;

    map->py = map->map_height / 2;
    map->py -= map->tiles_per_screen_y / 2;
    map->py *= TILE_HEIGHT;
}

/*
 * map_move_delta_pixels
 *
 * Shift the map by some pixels.
 */
void map_move_delta_pixels (int32_t dx, int32_t dy)
{
    if (map_ctx->px + dx < map_ctx->min_px) {
        dx = 0;
    }

    if (map_ctx->px + dx > map_ctx->max_px) {
        dx = 0;
    }

    if (map_ctx->py + dy < map_ctx->min_py) {
        dy = 0;
    }

    if (map_ctx->py + dy > map_ctx->max_py) {
        dy = 0;
    }

    map_ctx->px += dx;
    map_ctx->py += dy;
    map_ctx->lx += dx;
    map_ctx->ly += dy;

    map_lightmap(map_ctx,
                 map_ctx->lx / TILE_WIDTH,
                 map_ctx->ly / TILE_HEIGHT,
                 MAP_DEPTH - 2,
                 DEFAULT_LIGHT_RAY_LENGTH,
                 true);
}

static boolean map_is_x_at (levelp level,
                            int32_t x, int32_t y, map_is_at_callback callback)
{
    thing_templatep thing_template;
    widp grid_wid;
    widp w;

    grid_wid = level_get_map(level);
    if (!grid_wid) {
        DIE("no grid wid");
    }

    /*
     * Look for a floor tile where we can place stuff.
     */
    w = wid_grid_find_first(grid_wid, x, y);
    while (w) {
        thingp thing_it = wid_get_thing(w);

        if (thing_it) {
            if (thing_item_owner(thing_it)) {
                w = wid_grid_find_next(grid_wid, w, x, y);
                continue;
            }

            if (thing_is_dead(thing_it)) {
                w = wid_grid_find_next(grid_wid, w, x, y);
                continue;
            }
        }

        thing_template = wid_get_thing_template(w);
        if (thing_template) {
            if ((*callback)(thing_template)) {
                return (true);
            }
        }

        w = wid_grid_find_next(grid_wid, w, x, y);
    }

    return (false);
}

static boolean map_count_x_at (levelp level,
                               int32_t x, int32_t y,
                               map_is_at_callback callback)
{
    thing_templatep thing_template;
    widp grid_wid;
    widp w;
    uint32_t count;

    count = 0;

    grid_wid = level_get_map(level);
    if (!grid_wid) {
        DIE("no grid wid");
    }

    /*
     * Look for a floor tile where we can place stuff.
     */
    w = wid_grid_find_first(grid_wid, x, y);
    while (w) {
        /*
         * If an item is being carried but is seen on the map then it is not 
         * really on the map.
         */
        thingp t = wid_get_thing(w);
        if (t) {
            if (thing_item_owner(t)) {
                w = wid_grid_find_next(grid_wid, w, x, y);
                continue;
            }
        }

        thing_template = wid_get_thing_template(w);
        if (thing_template) {
            if ((*callback)(thing_template)) {
                count++;
            }
        }

        w = wid_grid_find_next(grid_wid, w, x, y);
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

boolean map_is_star_green_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_star_green));
}

boolean map_is_star_cyan_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_star_cyan));
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
    widp grid_wid;
    widp w;

    grid_wid = level_get_map(level);
    if (!grid_wid) {
        DIE("no grid wid");
    }

    /*
     * Look for a floor tile where we can place stuff.
     */
    w = wid_grid_find_first(grid_wid, x, y);
    while (w) {
        thingp thing_it = wid_get_thing(w);

        if (!thing_it) {
            w = wid_grid_find_next(grid_wid, w, x, y);
            continue;
        }

        if (thing_item_owner(thing_it)) {
            w = wid_grid_find_next(grid_wid, w, x, y);
            continue;
        }

        if (thing_item_owner(thing_it)) {
            w = wid_grid_find_next(grid_wid, w, x, y);
            continue;
        }

        if (thing_is_dead(thing_it)) {
            w = wid_grid_find_next(grid_wid, w, x, y);
            continue;
        }

        thing_template = wid_get_thing_template(w);
        if (thing_template) {
            if ((*callback)(thing_template)) {
                return (thing_it);
            }
        }

        w = wid_grid_find_next(grid_wid, w, x, y);
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

thingp map_thing_is_star_green_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_star_green));
}

thingp map_thing_is_star_cyan_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_star_cyan));
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

static tree_rootp map_all_things_is_x_at (levelp level,
                                          int32_t x, int32_t y,
                                          map_is_at_callback callback)
{
    thing_templatep thing_template;
    tree_thing_node *node;
    tree_rootp root;
    widp grid_wid;
    widp w;

    root = 0;

    grid_wid = level_get_map(level);
    if (!grid_wid) {
        DIE("no grid wid");
    }

    /*
     * Look for a floor tile where we can place stuff.
     */
    w = wid_grid_find_first(grid_wid, x, y);
    while (w) {
        thingp thing_it = wid_get_thing(w);

        if (!thing_it) {
            w = wid_grid_find_next(grid_wid, w, x, y);
            continue;
        }

        if (thing_item_owner(thing_it)) {
            w = wid_grid_find_next(grid_wid, w, x, y);
            continue;
        }

        if (thing_is_dead(thing_it)) {
            w = wid_grid_find_next(grid_wid, w, x, y);
            continue;
        }

        thing_template = wid_get_thing_template(w);
        if (thing_template) {
            if ((*callback)(thing_template)) {
                if (!root) {
                    root = tree_alloc(TREE_KEY_POINTER,
                                      "TREE ROOT: map find things");
                }

                node = (typeof(node))
                    myzalloc(sizeof(*node), "TREE NODE: map find thing");

                node->tree.key = (void*)thing_it;

                if (!tree_insert(root, &node->tree.node)) {
                    DIE("insert thingp %p", thing_it);
                }
            }
        }

        w = wid_grid_find_next(grid_wid, w, x, y);
    }

    return (root);
}

tree_rootp map_all_things_is_player_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_player));
}

tree_rootp map_all_things_is_monst_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_monst));
}

tree_rootp map_all_things_is_snail_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_snail));
}

tree_rootp map_all_things_is_wall_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_wall));
}

tree_rootp map_all_things_is_star_yellow_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_star_yellow));
}

tree_rootp map_all_things_is_rock_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_rock));
}

tree_rootp map_all_things_is_water_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_water));
}

tree_rootp map_all_things_is_lava_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_lava));
}

tree_rootp map_all_things_is_xxx4_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_xxx4));
}

tree_rootp map_all_things_is_xxx5_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_xxx5));
}

tree_rootp map_all_things_is_xxx6_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_xxx6));
}

tree_rootp map_all_things_is_xxx7_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_xxx7));
}

tree_rootp map_all_things_is_xxx8_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_xxx8));
}

tree_rootp map_all_things_is_star_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_star));
}

tree_rootp map_all_things_is_powerup_spam_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_powerup_spam));
}

tree_rootp map_all_things_is_rock_0_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_rock_0));
}

tree_rootp map_all_things_is_car_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_car));
}

tree_rootp map_all_things_is_star_green_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_star_green));
}

tree_rootp map_all_things_is_star_cyan_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_star_cyan));
}

tree_rootp map_all_things_is_star_black_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_star_black));
}

tree_rootp map_all_things_is_star_purple_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_star_purple));
}

tree_rootp map_all_things_is_explosion_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_explosion));
}

tree_rootp map_all_things_is_spikes_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_spikes));
}

tree_rootp map_all_things_is_star_red_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_star_red));
}

tree_rootp map_all_things_is_star_blue_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_star_blue));
}

tree_rootp map_all_things_is_seedpod_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_seedpod));
}

tree_rootp map_all_things_is_bomb_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_bomb));
}

tree_rootp map_all_things_is_spam_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_spam));
}

tree_rootp map_all_things_is_road_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_road));
}

tree_rootp map_all_things_is_pipe_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_pipe));
}

tree_rootp map_all_things_is_item_removed_at_level_end_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_item_removed_at_level_end));
}

tree_rootp map_all_things_is_scarable_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_scarable));
}

tree_rootp map_all_things_is_shrunk_when_carried_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_shrunk_when_carried));
}

tree_rootp map_all_things_is_hidden_from_editor_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_hidden_from_editor));
}

tree_rootp map_all_things_is_animated_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_animated));
}

tree_rootp map_all_things_is_follows_owner_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_follows_owner));
}

tree_rootp map_all_things_is_powerup_rocket_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_powerup_rocket));
}

tree_rootp map_all_things_is_left_as_corpse_on_death_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_left_as_corpse_on_death));
}

tree_rootp map_all_things_is_item_perma_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_item_perma));
}

tree_rootp map_all_things_is_esnail_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_esnail));
}

tree_rootp map_all_things_is_item_hidden_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_item_hidden));
}

tree_rootp map_all_things_is_bonus_letter_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_bonus_letter));
}

tree_rootp map_all_things_is_thing_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_thing));
}

tree_rootp map_all_things_is_plant_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_plant));
}

tree_rootp map_all_things_is_food_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_food));
}

tree_rootp map_all_things_is_floor_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_floor));
}

tree_rootp map_all_things_is_exit_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_exit));
}

tree_rootp map_all_things_is_letter_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_letter));
}

static thing_templatep map_find_x_at (levelp level,
                                      int32_t x, int32_t y,
                                      map_is_at_callback callback,
                                      widp *wout)
{
    thing_templatep thing_template;
    widp grid_wid;
    widp w;

    grid_wid = level_get_map(level);
    if (!grid_wid) {
        DIE("no grid wid");
    }

    w = wid_grid_find_first(grid_wid, x, y);
    while (w) {
        thing_template = wid_get_thing_template(w);
        if (thing_template) {
            if ((*callback)(thing_template)) {
                if (wout) {
                    *wout = w;
                }

                return (thing_template);
            }
        }

        w = wid_grid_find_next(grid_wid, w, x, y);
    }

    return (0);
}

thing_templatep map_find_player_at (levelp level,
                                    int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_player, w));
}

thing_templatep map_find_monst_at (levelp level,
                                   int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_monst, w));
}

thing_templatep map_find_snail_at (levelp level,
                                   int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_snail, w));
}

thing_templatep map_find_wall_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_wall, w));
}

thing_templatep map_find_star_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_star_yellow, w));
}

thing_templatep map_find_rock_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_rock, w));
}

thing_templatep map_find_water_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_water, w));
}

thing_templatep map_find_lava_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_lava, w));
}

thing_templatep map_find_xxx4_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_xxx4, w));
}

thing_templatep map_find_xxx5_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_xxx5, w));
}

thing_templatep map_find_xxx6_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_xxx6, w));
}

thing_templatep map_find_xxx7_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_xxx7, w));
}

thing_templatep map_find_xxx8_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_xxx8, w));
}

thing_templatep map_find_is_star_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_star, w));
}

thing_templatep map_find_powerup_spam_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_powerup_spam, w));
}

thing_templatep map_find_rock_0_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_rock_0, w));
}

thing_templatep map_find_car_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_car, w));
}

thing_templatep map_find_star_green_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_star_green, w));
}

thing_templatep map_find_star_cyan_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_star_cyan, w));
}

thing_templatep map_find_star_black_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_star_black, w));
}

thing_templatep map_find_star_purple_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_star_purple, w));
}

thing_templatep map_find_explosion_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_explosion, w));
}

thing_templatep map_find_spikes_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_spikes, w));
}

thing_templatep map_find_is_star_red_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_star_red, w));
}

thing_templatep map_find_is_star_blue_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_star_blue, w));
}

thing_templatep map_find_seedpod_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_seedpod, w));
}

thing_templatep map_find_bomb_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_bomb, w));
}

thing_templatep map_find_spam_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_spam, w));
}

thing_templatep map_find_road_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_road, w));
}

thing_templatep map_find_pipe_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_pipe, w));
}

thing_templatep map_find_water7_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_item_removed_at_level_end, w));
}

thing_templatep map_find_water8_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_scarable, w));
}

thing_templatep map_find_is_shrunk_when_carried_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_shrunk_when_carried, w));
}

thing_templatep map_find_hidden_from_editor_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_hidden_from_editor, w));
}

thing_templatep map_find_is_left_as_corpse_on_death_at (levelp level,
                                   int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_left_as_corpse_on_death, w));
}

thing_templatep map_find_item_perma_at (levelp level,
                                   int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_item_perma, w));
}

thing_templatep map_find_esnail_at (levelp level,
                                   int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_esnail, w));
}

thing_templatep map_find_is_item_hidden_at (levelp level,
                                   int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_item_hidden, w));
}

thing_templatep map_find_bonus_letter_at (levelp level,
                                   int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_bonus_letter, w));
}

thing_templatep map_find_spawns_thing_at (levelp level,
                                          int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_thing, w));
}

thing_templatep map_find_plant_at (levelp level,
                                   int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_plant, w));
}

thing_templatep map_find_food_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_food, w));
}

thing_templatep map_find_floor_at (levelp level,
                                   int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_floor, w));
}

thing_templatep map_find_exit_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_exit, w));
}

thing_templatep map_find_letter_at (levelp level,
                                    int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_letter, w));
}

/*
 * Join up adjactent blocks.
 */
static void 
map_fixup (map_frame_ctx_t *map)
{
    int32_t x;
    int32_t y;
    uint32_t z;
    int32_t dx;
    int32_t dy;
    thing_templatep nbrs[3][3];
    boolean is_join_block;
    boolean is_join_horiz;
    boolean is_join_vert;
    boolean is_join_node;
    boolean is_join_left;
    boolean is_join_right;
    boolean is_join_top;
    boolean is_join_bot;
    boolean is_join_tl;
    boolean is_join_tr;
    boolean is_join_bl;
    boolean is_join_br;
    boolean is_join_t;
    boolean is_join_t90;
    boolean is_join_t180;
    boolean is_join_t270;
    boolean is_join_x;
    boolean is_join_tl2;
    boolean is_join_tr2;
    boolean is_join_bl2;
    boolean is_join_br2;
    boolean is_join_t_1;
    boolean is_join_t_2;
    boolean is_join_t_3;
    boolean is_join_t90_1;
    boolean is_join_t90_2;
    boolean is_join_t90_3;
    boolean is_join_t180_1;
    boolean is_join_t180_2;
    boolean is_join_t180_3;
    boolean is_join_t270_1;
    boolean is_join_t270_2;
    boolean is_join_t270_3;
    boolean is_join_x1;
    boolean is_join_x1_270;
    boolean is_join_x1_180;
    boolean is_join_x1_90;
    boolean is_join_x2;
    boolean is_join_x2_270;
    boolean is_join_x2_180;
    boolean is_join_x2_90;
    boolean is_join_x3;
    boolean is_join_x3_180;
    boolean is_join_x4;
    boolean is_join_x4_270;
    boolean is_join_x4_180;
    boolean is_join_x4_90;

    for (z = 0; z < MAP_DEPTH; z++) {
        for (x = 0; x < MAP_WIDTH; x++) {
            for (y = 0; y < MAP_HEIGHT; y++) {

                thing_templatep thing_template = 
                    map->tiles[x][y][z].thing_template;

                if ((thing_template != ROCK_0) &&
                    (thing_template != LAVA_0) &&
                    (thing_template != WATER_0) &&
                    (thing_template != BRICK_0) &&
                    (thing_template != BRICK_1) &&
                    (thing_template != BRICK_2) &&
                    (thing_template != BRICK_3) &&
                    (thing_template != BRICK_4) &&
                    (thing_template != BRICK_5) &&
                    (thing_template != BRICK_6) &&
                    (thing_template != BRICK_7) &&
                    (thing_template != BRICK_8)) {
                    continue;
                }

                for (dx = -1; dx <= 1; dx++) {
                    for (dy = -1; dy <= 1; dy++) {

                        if (x + dx < 0) {
                            continue;
                        }

                        if (y + dy < 0) {
                            continue;
                        }

                        if (x + dx >= MAP_WIDTH) {
                            continue;
                        }

                        if (y + dy >= MAP_HEIGHT) {
                            continue;
                        }

                        thing_templatep a_thing_template = 
                            map->tiles[x + dx][y + dy][z].thing_template;

                        if ((a_thing_template != ROCK_0) &&
                            (a_thing_template != LAVA_0) &&
                            (a_thing_template != WATER_0) &&
                            (a_thing_template != BRICK_0) &&
                            (a_thing_template != BRICK_1) &&
                            (a_thing_template != BRICK_2) &&
                            (a_thing_template != BRICK_3) &&
                            (a_thing_template != BRICK_4) &&
                            (a_thing_template != BRICK_5) &&
                            (a_thing_template != BRICK_6) &&
                            (a_thing_template != BRICK_7) &&
                            (a_thing_template != BRICK_8)) {
                            nbrs[dx + 1][dy + 1] = NULL;
                            continue;
                        }

                        nbrs[dx + 1][dy + 1] = a_thing_template;
                    }
                }

                is_join_block = false;
                is_join_horiz = false;
                is_join_vert = false;
                is_join_node = false;
                is_join_left = false;
                is_join_right = false;
                is_join_top = false;
                is_join_bot = false;
                is_join_tl = false;
                is_join_tr = false;
                is_join_bl = false;
                is_join_br = false;
                is_join_t = false;
                is_join_t90 = false;
                is_join_t180 = false;
                is_join_t270 = false;
                is_join_x = false;
                is_join_tl2 = false;
                is_join_tr2 = false;
                is_join_bl2 = false;
                is_join_br2 = false;
                is_join_t_1 = false;
                is_join_t_2 = false;
                is_join_t_3 = false;
                is_join_t90_1 = false;
                is_join_t90_2 = false;
                is_join_t90_3 = false;
                is_join_t180_1 = false;
                is_join_t180_2 = false;
                is_join_t180_3 = false;
                is_join_t270_1 = false;
                is_join_t270_2 = false;
                is_join_t270_3 = false;
                is_join_x = false;
                is_join_x1 = false;
                is_join_x1_270 = false;
                is_join_x1_180 = false;
                is_join_x1_90 = false;
                is_join_x2 = false;
                is_join_x2_270 = false;
                is_join_x2_180 = false;
                is_join_x2_90 = false;
                is_join_x3 = false;
                is_join_x3_180 = false;
                is_join_x4 = false;
                is_join_x4_270 = false;
                is_join_x4_180 = false;
                is_join_x4_90 = false;

                thing_templatep a = nbrs[0][0];
                thing_templatep b = nbrs[1][0];
                thing_templatep c = nbrs[2][0];
                thing_templatep d = nbrs[0][1];
                thing_templatep e = nbrs[1][1];
                thing_templatep f = nbrs[2][1];
                thing_templatep g = nbrs[0][2];
                thing_templatep h = nbrs[1][2];
                thing_templatep i = nbrs[2][2];

                char A = (a != NULL) ? '1' : ' ';
                char B = (b != NULL) ? '1' : ' ';
                char C = (c != NULL) ? '1' : ' ';
                char D = (d != NULL) ? '1' : ' ';
                char E = (e != NULL) ? '1' : ' ';
                char F = (f != NULL) ? '1' : ' ';
                char G = (g != NULL) ? '1' : ' ';
                char H = (h != NULL) ? '1' : ' ';
                char I = (i != NULL) ? '1' : ' ';

                uint64_t score;
                uint64_t best = 0;
                boolean *best_block = 0;

#define BLOCK(arr, block_type)                                              \
                if ((arr[0] == '1') && (A == ' ') ||                        \
                    (arr[1] == '1') && (B == ' ') ||                        \
                    (arr[2] == '1') && (C == ' ') ||                        \
                    (arr[3] == '1') && (D == ' ') ||                        \
                    (arr[4] == '1') && (E == ' ') ||                        \
                    (arr[5] == '1') && (F == ' ') ||                        \
                    (arr[6] == '1') && (G == ' ') ||                        \
                    (arr[7] == '1') && (H == ' ') ||                        \
                    (arr[8] == '1') && (I == ' ')) {                        \
                } else {                                                    \
                    score =                                                 \
                        ((uint64_t) (arr[0] == A) ? 1 : 0) * 100000000LLU + \
                        ((uint64_t) (arr[1] == B) ? 1 : 0) * 10000000LLU  + \
                        ((uint64_t) (arr[2] == C) ? 1 : 0) * 1000000LLU   + \
                        ((uint64_t) (arr[3] == D) ? 1 : 0) * 100000LLU    + \
                        ((uint64_t) (arr[4] == E) ? 1 : 0) * 10000LLU     + \
                        ((uint64_t) (arr[5] == F) ? 1 : 0) * 1000LLU      + \
                        ((uint64_t) (arr[6] == G) ? 1 : 0) * 100LLU       + \
                        ((uint64_t) (arr[7] == H) ? 1 : 0) * 10LLU        + \
                        ((uint64_t) (arr[8] == I) ? 1 : 0);                 \
                                                                            \
                    if (score > best) {                                     \
                        best = score;                                       \
                        best_block = &block_type;                           \
                    }                                                       \
                }                                                           \

                BLOCK("111"
                      "111"
                      "111", is_join_block)

                BLOCK("   "
                      " 1 "
                      "   ", is_join_node)

                BLOCK("   "
                      " 11"
                      "   ", is_join_left)

                BLOCK("   "
                      " 1 "
                      " 1 ", is_join_top)

                BLOCK("   "
                      "11 "
                      "   ", is_join_right)

                BLOCK(" 1 "
                      " 1 "
                      "   ", is_join_bot)

                BLOCK("   "
                      "111"
                      "   ", is_join_horiz)

                BLOCK(" 1 "
                      " 1 "
                      " 1 ", is_join_vert)

                BLOCK("   "
                      " 11"
                      " 11", is_join_tl2)

                BLOCK(" 11"
                      " 11"
                      "   ", is_join_bl2)

                BLOCK("11 "
                      "11 "
                      "   ", is_join_br2)

                BLOCK("   "
                      "11 "
                      "11 ", is_join_tr2)

                BLOCK("   "
                      " 11"
                      " 1 ", is_join_tl)

                BLOCK(" 1 "
                      " 11"
                      "   ", is_join_bl)

                BLOCK(" 1 "
                      "11 "
                      "   ", is_join_br)

                BLOCK("   "
                      "11 "
                      " 1 ", is_join_tr)

                BLOCK("11 "
                      "11 "
                      "11 ", is_join_t90_3)

                BLOCK("111"
                      "111"
                      "   ", is_join_t180_3)

                BLOCK(" 11"
                      " 11"
                      " 11", is_join_t270_3)

                BLOCK("   "
                      "111"
                      "111", is_join_t_3)

                BLOCK(" 1 "
                      " 11"
                      " 1 ", is_join_t270)

                BLOCK(" 1 "
                      "111"
                      "   ", is_join_t180)

                BLOCK(" 1 "
                      "11 "
                      " 1 ", is_join_t90)

                BLOCK("   "
                      "111"
                      " 1 ", is_join_t)

                BLOCK(" 11"
                      " 11"
                      " 1 ", is_join_t270_2)

                BLOCK("11 "
                      "111"
                      "   ", is_join_t180_2)

                BLOCK(" 1 "
                      "11 "
                      "11 ", is_join_t90_2)

                BLOCK("   "
                      "111"
                      " 11", is_join_t_2)

                BLOCK(" 1 "
                      " 11"
                      " 11", is_join_t270_1)

                BLOCK(" 11"
                      "111"
                      "   ", is_join_t180_1)

                BLOCK("11 "
                      "11 "
                      " 1 ", is_join_t90_1)

                BLOCK("   "
                      "111"
                      "11 ", is_join_t_1)

                BLOCK(" 1 "
                      "111"
                      " 1 ", is_join_x)

                BLOCK(" 1 "
                      "111"
                      " 11", is_join_x1)

                BLOCK(" 11"
                      "111"
                      " 1 ", is_join_x1_270)

                BLOCK("11 "
                      "111"
                      " 1 ", is_join_x1_180)

                BLOCK(" 1 "
                      "111"
                      "11 ", is_join_x1_90)

                BLOCK(" 1 "
                      "111"
                      "111", is_join_x2)

                BLOCK(" 11"
                      "111"
                      " 11", is_join_x2_270)

                BLOCK("111"
                      "111"
                      " 1 ", is_join_x2_180)

                BLOCK("11 "
                      "111"
                      "11 ", is_join_x2_90)

                BLOCK(" 11"
                      "111"
                      "11 ", is_join_x3)

                BLOCK("11 "
                      "111"
                      " 11", is_join_x3_180)

                BLOCK(" 11"
                      "111"
                      "111", is_join_x4)

                BLOCK("111"
                      "111"
                      " 11", is_join_x4_270)

                BLOCK("111"
                      "111"
                      "11 ", is_join_x4_180)

                BLOCK("11 "
                      "111"
                      "111", is_join_x4_90)

                if (z < MAP_DEPTH - 2) {
                    if (map->tiles[x][y][z + 1].thing_template ==
                        thing_template) {

                        is_join_block = false;
                        best_block = &is_join_block;
                    }
                }

                if (best_block == NULL) {
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

                *best_block = true;

                thing_tilep thing_tile = thing_tile_find(e,
                            is_join_block,
                            is_join_horiz,
                            is_join_vert,
                            is_join_node,
                            is_join_left,
                            is_join_right,
                            is_join_top,
                            is_join_bot,
                            is_join_tl,
                            is_join_tr,
                            is_join_bl,
                            is_join_br,
                            is_join_t,
                            is_join_t90,
                            is_join_t180,
                            is_join_t270,
                            is_join_x,
                            is_join_tl2,
                            is_join_tr2,
                            is_join_bl2,
                            is_join_br2,
                            is_join_t_1,
                            is_join_t_2,
                            is_join_t_3,
                            is_join_t90_1,
                            is_join_t90_2,
                            is_join_t90_3,
                            is_join_t180_1,
                            is_join_t180_2,
                            is_join_t180_3,
                            is_join_t270_1,
                            is_join_t270_2,
                            is_join_t270_3,
                            is_join_x1,
                            is_join_x1_270,
                            is_join_x1_180,
                            is_join_x1_90,
                            is_join_x2,
                            is_join_x2_270,
                            is_join_x2_180,
                            is_join_x2_90,
                            is_join_x3,
                            is_join_x3_180,
                            is_join_x4,
                            is_join_x4_270,
                            is_join_x4_180,
                            is_join_x4_90);

                if (!thing_tile) {

                    is_join_block = false;
                    is_join_horiz = false;
                    is_join_vert = false;
                    is_join_node = true;
                    is_join_left = false;
                    is_join_right = false;
                    is_join_top = false;
                    is_join_bot = false;
                    is_join_tl = false;
                    is_join_tr = false;
                    is_join_bl = false;
                    is_join_br = false;
                    is_join_t = false;
                    is_join_t90 = false;
                    is_join_t180 = false;
                    is_join_t270 = false;
                    is_join_x = false;
                    is_join_tl2 = false;
                    is_join_tr2 = false;
                    is_join_bl2 = false;
                    is_join_br2 = false;
                    is_join_t_1 = false;
                    is_join_t_2 = false;
                    is_join_t_3 = false;
                    is_join_t90_1 = false;
                    is_join_t90_2 = false;
                    is_join_t90_3 = false;
                    is_join_t180_1 = false;
                    is_join_t180_2 = false;
                    is_join_t180_3 = false;
                    is_join_t270_1 = false;
                    is_join_t270_2 = false;
                    is_join_t270_3 = false;
                    is_join_x = false;
                    is_join_x1 = false;
                    is_join_x1_270 = false;
                    is_join_x1_180 = false;
                    is_join_x1_90 = false;
                    is_join_x2 = false;
                    is_join_x2_270 = false;
                    is_join_x2_180 = false;
                    is_join_x2_90 = false;
                    is_join_x3 = false;
                    is_join_x3_180 = false;
                    is_join_x4 = false;
                    is_join_x4_270 = false;
                    is_join_x4_180 = false;
                    is_join_x4_90 = false;

                    thing_tile = thing_tile_find(e,
                                is_join_block,
                                is_join_horiz,
                                is_join_vert,
                                is_join_node,
                                is_join_left,
                                is_join_right,
                                is_join_top,
                                is_join_bot,
                                is_join_tl,
                                is_join_tr,
                                is_join_bl,
                                is_join_br,
                                is_join_t,
                                is_join_t90,
                                is_join_t180,
                                is_join_t270,
                                is_join_x,
                                is_join_tl2,
                                is_join_tr2,
                                is_join_bl2,
                                is_join_br2,
                                is_join_t_1,
                                is_join_t_2,
                                is_join_t_3,
                                is_join_t90_1,
                                is_join_t90_2,
                                is_join_t90_3,
                                is_join_t180_1,
                                is_join_t180_2,
                                is_join_t180_3,
                                is_join_t270_1,
                                is_join_t270_2,
                                is_join_t270_3,
                                is_join_x1,
                                is_join_x1_270,
                                is_join_x1_180,
                                is_join_x1_90,
                                is_join_x2,
                                is_join_x2_270,
                                is_join_x2_180,
                                is_join_x2_90,
                                is_join_x3,
                                is_join_x3_180,
                                is_join_x4,
                                is_join_x4_270,
                                is_join_x4_180,
                                is_join_x4_90);

                    if (!thing_tile) {
                        DIE("no joinable tile for %s", thing_template_name(e));
                    }
                }

                const char *tilename = thing_tile_name(thing_tile);
                if (!tilename) {
                    DIE("no tilename for %s", thing_template_name(e));
                }

                tilep tile = tile_find(tilename);
                if (!tile) {
                    DIE("no tile for %s", thing_template_name(e));
                }

                uint32_t index = tile_get_index(tile);

                map->tiles[x][y][z].tile = index;
                map->tiles[x][y][z].thing_template = thing_template;
            }
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

uint32_t level_count_is_star_green (levelp level)
{
    return (level_count_is_x(level, thing_template_is_star_green));
}

uint32_t level_count_is_star_cyan (levelp level)
{
    return (level_count_is_x(level, thing_template_is_star_cyan));
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

static tree_rootp map_all_things_is_x (levelp level,
                                       map_is_at_callback callback)
{
    thing_templatep thing_template;
    tree_thing_node *node;
    tree_rootp root;
    widp grid_wid;
    widp w;
    int32_t x;
    int32_t y;

    root = 0;

    grid_wid = level_get_map(level);
    if (!grid_wid) {
        DIE("no grid wid");
    }

    for (y = 0; y < TILES_MAP_EDITABLE_HEIGHT; y++) {
        for (x = 0; x < TILES_MAP_EDITABLE_WIDTH; x++) {

            /*
             * Look for a floor tile where we can place stuff.
             */
            w = wid_grid_find_first(grid_wid, x, y);
            while (w) {
                thingp thing_it = wid_get_thing(w);

                if (!thing_it) {
                    w = wid_grid_find_next(grid_wid, w, x, y);
                    continue;
                }

                if (thing_item_owner(thing_it)) {
                    w = wid_grid_find_next(grid_wid, w, x, y);
                    continue;
                }

                if (thing_is_dead(thing_it)) {
                    w = wid_grid_find_next(grid_wid, w, x, y);
                    continue;
                }

                thing_template = wid_get_thing_template(w);
                if (thing_template) {
                    if ((*callback)(thing_template)) {
                        if (!root) {
                            root = tree_alloc(TREE_KEY_POINTER,
                                            "TREE ROOT: map find things");
                        }

                        node = (typeof(node))
                            myzalloc(sizeof(*node), "TREE NODE: map find thing");

                        node->tree.key = (void*)thing_it;

                        if (!tree_insert(root, &node->tree.node)) {
                            DIE("insert thingp %p", thing_it);
                        }
                    }
                }

                w = wid_grid_find_next(grid_wid, w, x, y);
            }

        }
    }

    return (root);
}

tree_rootp map_all_things_is_player (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_player));
}

tree_rootp map_all_things_is_monst (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_monst));
}

tree_rootp map_all_things_is_snail (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_snail));
}

tree_rootp map_all_things_is_wall (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_wall));
}

tree_rootp map_all_things_is_star_yellow (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_star_yellow));
}

tree_rootp map_all_things_is_rock (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_rock));
}

tree_rootp map_all_things_is_water (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_water));
}

tree_rootp map_all_things_is_lava (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_lava));
}

tree_rootp map_all_things_is_xxx4 (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_xxx4));
}

tree_rootp map_all_things_is_xxx5 (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_xxx5));
}

tree_rootp map_all_things_is_xxx6 (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_xxx6));
}

tree_rootp map_all_things_is_xxx7 (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_xxx7));
}

tree_rootp map_all_things_is_xxx8 (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_xxx8));
}

tree_rootp map_all_things_is_star (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_star));
}

tree_rootp map_all_things_is_powerup_spam (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_powerup_spam));
}

tree_rootp map_all_things_is_rock_0 (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_rock_0));
}

tree_rootp map_all_things_is_car (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_car));
}

tree_rootp map_all_things_is_star_green (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_star_green));
}

tree_rootp map_all_things_is_star_cyan (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_star_cyan));
}

tree_rootp map_all_things_is_star_black (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_star_black));
}

tree_rootp map_all_things_is_star_purple (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_star_purple));
}

tree_rootp map_all_things_is_explosion (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_explosion));
}

tree_rootp map_all_things_is_spikes (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_spikes));
}

tree_rootp map_all_things_is_star_red (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_star_red));
}

tree_rootp map_all_things_is_star_blue (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_star_blue));
}

tree_rootp map_all_things_is_seedpod (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_seedpod));
}

tree_rootp map_all_things_is_bomb (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_bomb));
}

tree_rootp map_all_things_is_spam (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_spam));
}

tree_rootp map_all_things_is_road (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_road));
}

tree_rootp map_all_things_is_pipe (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_pipe));
}

tree_rootp map_all_things_is_item_removed_at_level_end (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_item_removed_at_level_end));
}

tree_rootp map_all_things_is_scarable (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_scarable));
}

tree_rootp map_all_things_is_shrunk_when_carried (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_shrunk_when_carried));
}

tree_rootp map_all_things_is_hidden_from_editor (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_hidden_from_editor));
}

tree_rootp map_all_things_is_animated (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_animated));
}

tree_rootp map_all_things_is_follows_owner (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_follows_owner));
}

tree_rootp map_all_things_is_powerup_rocket (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_powerup_rocket));
}

tree_rootp map_all_things_is_left_as_corpse_on_death (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_left_as_corpse_on_death));
}

tree_rootp map_all_things_is_item_perma (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_item_perma));
}

tree_rootp map_all_things_is_esnail (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_esnail));
}

tree_rootp map_all_things_is_item_hidden (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_item_hidden));
}

tree_rootp map_all_things_is_bonus_letter (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_bonus_letter));
}

tree_rootp map_all_things_is_thing (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_thing));
}

tree_rootp map_all_things_is_plant (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_plant));
}

tree_rootp map_all_things_is_food (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_food));
}

tree_rootp map_all_things_is_floor (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_floor));
}

tree_rootp map_all_things_is_exit (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_exit));
}

tree_rootp map_all_things_is_letter (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_letter));
}
