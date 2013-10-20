/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file.
 */

boolean map_is_item_hidden_at(levelp, int32_t x, int32_t y);
boolean map_is_thing_at(levelp, int32_t x, int32_t y);
boolean map_is_bonus_letter_at(levelp, int32_t x, int32_t y);
boolean map_is_rock_0_at(levelp, int32_t x, int32_t y);
boolean map_is_esnail_at(levelp, int32_t x, int32_t y);
boolean map_is_exit_at(levelp, int32_t x, int32_t y);
boolean map_is_floor_at(levelp, int32_t x, int32_t y);
boolean map_is_food_at(levelp, int32_t x, int32_t y);
boolean map_is_ladder_at(levelp, int32_t x, int32_t y);
boolean map_is_monst_at(levelp, int32_t x, int32_t y);
boolean map_is_item_perma_at(levelp, int32_t x, int32_t y);
boolean map_is_plant_at(levelp, int32_t x, int32_t y);
boolean map_is_player_at(levelp, int32_t x, int32_t y);
boolean map_is_snail_at(levelp, int32_t x, int32_t y);
boolean map_is_gem_0_at(levelp, int32_t x, int32_t y);
boolean map_is_wall_at(levelp, int32_t x, int32_t y);
boolean map_is_animated_at(levelp, int32_t x, int32_t y);
boolean map_is_follows_owner_at(levelp, int32_t x, int32_t y);
boolean map_is_powerup_rocket_at(levelp, int32_t x, int32_t y);
boolean map_is_rock_at(levelp, int32_t x, int32_t y);
boolean map_is_water_at(levelp, int32_t x, int32_t y);
boolean map_is_lava_at(levelp, int32_t x, int32_t y);
boolean map_is_gem_6_at(levelp, int32_t x, int32_t y);
boolean map_is_gem_7_at(levelp, int32_t x, int32_t y);
boolean map_is_boulder_at(levelp, int32_t x, int32_t y);
boolean map_is_xxx7_at(levelp, int32_t x, int32_t y);
boolean map_is_xxx8_at(levelp, int32_t x, int32_t y);
boolean map_is_star_at(levelp, int32_t x, int32_t y);
boolean map_is_powerup_spam_at(levelp, int32_t x, int32_t y);
boolean map_is_rock_0_at(levelp, int32_t x, int32_t y);
boolean map_is_car_at(levelp, int32_t x, int32_t y);
boolean map_is_transparent_at(levelp, int32_t x, int32_t y);
boolean map_is_radiant_at(levelp, int32_t x, int32_t y);
boolean map_is_gem_1_at(levelp, int32_t x, int32_t y);
boolean map_is_gem_2_at(levelp, int32_t x, int32_t y);
boolean map_is_explosion_at(levelp, int32_t x, int32_t y);
boolean map_is_spikes_at(levelp, int32_t x, int32_t y);
boolean map_is_gem_4_at(levelp, int32_t x, int32_t y);
boolean map_is_gem_4_at(levelp, int32_t x, int32_t y);
boolean map_is_gem_5_at(levelp, int32_t x, int32_t y);
boolean map_is_seedpod_at(levelp, int32_t x, int32_t y);
boolean map_is_bomb_at(levelp, int32_t x, int32_t y);
boolean map_is_spam_at(levelp, int32_t x, int32_t y);
boolean map_is_road_at(levelp, int32_t x, int32_t y);
boolean map_is_pipe_at(levelp, int32_t x, int32_t y);
boolean map_is_item_removed_at_level_end_at(levelp, int32_t x, int32_t y);
boolean map_is_scarable_at(levelp, int32_t x, int32_t y);
boolean map_is_shrunk_when_carried_at(levelp, int32_t x, int32_t y);
boolean map_is_hidden_from_editor_at(levelp, int32_t x, int32_t y);
boolean map_is_left_as_corpse_on_death_at(levelp, int32_t x, int32_t y);

thingp map_thing_is_item_hidden_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_thing_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_bonus_letter_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_esnail_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_exit_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_floor_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_food_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_ladder_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_monst_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_item_perma_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_plant_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_player_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_snail_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_gem_0_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_wall_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_animated_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_follows_owner_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_powerup_rocket_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_left_as_corpse_on_death_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_rock_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_water_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_lava_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_gem_6_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_gem_7_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_boulder_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_xxx7_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_xxx8_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_star_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_powerup_spam_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_rock_0_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_car_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_transparent_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_radiant_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_gem_1_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_gem_2_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_explosion_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_spikes_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_gem_4_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_gem_4_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_gem_5_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_seedpod_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_bomb_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_spam_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_road_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_pipe_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_item_removed_at_level_end_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_scarable_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_shrunk_when_carried_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_hidden_from_editor_at(levelp, int32_t x, int32_t y);

uint32_t level_count_is_item_is_hidden(levelp);
uint32_t level_count_is_thing(levelp);
uint32_t level_count_is_bonus_letter(levelp);
uint32_t level_count_is_esnail(levelp);
uint32_t level_count_is_exit(levelp);
uint32_t level_count_is_floor(levelp);
uint32_t level_count_is_food(levelp);
uint32_t level_count_is_ladder(levelp);
uint32_t level_count_is_monst(levelp);
uint32_t level_count_is_item_perma(levelp);
uint32_t level_count_is_plant(levelp);
uint32_t level_count_is_player(levelp);
uint32_t level_count_is_snail(levelp);
uint32_t level_count_is_gem_0(levelp);
uint32_t level_count_is_wall(levelp);
uint32_t level_count_is_follows_owner(levelp);
uint32_t level_count_is_powerup_rocket(levelp);
uint32_t level_count_is_rock(levelp);
uint32_t level_count_is_water(levelp);
uint32_t level_count_is_lava(levelp);
uint32_t level_count_is_gem_6(levelp);
uint32_t level_count_is_gem_7(levelp);
uint32_t level_count_is_boulder(levelp);
uint32_t level_count_is_xxx7(levelp);
uint32_t level_count_is_xxx8(levelp);
uint32_t level_count_is_star(levelp);
uint32_t level_count_is_powerup_spam(levelp);
uint32_t level_count_is_rock_0(levelp);
uint32_t level_count_is_car(levelp);
uint32_t level_count_is_transparent(levelp);
uint32_t level_count_is_radiant(levelp);
uint32_t level_count_is_gem_1(levelp);
uint32_t level_count_is_gem_2(levelp);
uint32_t level_count_is_explosion(levelp);
uint32_t level_count_is_spikes(levelp);
uint32_t level_count_is_gem_4(levelp);
uint32_t level_count_is_gem_4(levelp);
uint32_t level_count_is_gem_5(levelp);
uint32_t level_count_is_seedpod(levelp);
uint32_t level_count_is_bomb(levelp);
uint32_t level_count_is_spam(levelp);
uint32_t level_count_is_road(levelp);
uint32_t level_count_is_pipe(levelp);
uint32_t level_count_is_item_removed_at_level_end(levelp);
uint32_t level_count_is_scarable(levelp);
uint32_t level_count_is_shrunk_when_carried(levelp);
uint32_t level_count_is_hidden_from_editor(levelp);
uint32_t level_count_is_left_as_corpse_on_death(levelp);

#include "tree.h"

typedef struct tree_thing_node_ {
    tree_key_pointer tree;
} tree_thing_node;

typedef struct {
    uint16_t template_id;
    uint16_t tile;
} map_tile_t;

/*
 * All the rendering info for one parallax frame of tiles.
 */
typedef struct map_t_ {
    /*
     * Main light.
     */
    int32_t lx;
    int32_t ly;

    /*
     * The top left corner pixel of the map.
     */
    int32_t px;
    int32_t py;

    /*
     * The tile extents of the map.
     */
    int32_t min_px;
    int32_t max_px;
    int32_t min_py;
    int32_t max_py;

    /*
     * All the tiles in this frame.
     */
    uint32_t map_width;
    uint32_t map_height;

    /*
     * Texture for this frame. One texture for all tiles here.
     */
    texp tex;
    int bind;

    /*
     * Size of the texture in pixels.
     */
    uint32_t tex_width;
    uint32_t tex_height;

    /*
     * Single tile size.
     */
    uint32_t tex_tile_width;
    uint32_t tex_tile_height;

    /*
     * How many tiles across and down.
     */
    uint32_t tex_tiles_across;
    uint32_t tex_tiles_down;

    /*
     * And the float size of that tile in the parent tex.
     */
    float tex_float_width;
    float tex_float_height;

    /*
     * How many tiles on screen at a time?
     */
    uint16_t tiles_per_screen_x;
    uint16_t tiles_per_screen_y;

    /*
     * The map.
     */
    map_tile_t tiles_copy[MAP_WIDTH][MAP_HEIGHT];
    map_tile_t tiles[MAP_WIDTH][MAP_HEIGHT];
    thingp things[MAP_WIDTH][MAP_HEIGHT];
    uint8_t lit[MAP_WIDTH][MAP_HEIGHT];

    /*
     * This is the huge buffer that contains the vertex and tex arrays.
     */
    float *gl_array_buf;
    float *gl_array_buf_end;
    uint32_t gl_array_size;

} map_t;

extern map_t *MAP;

/*
 * map.c
 */
void map_move_delta_pixels(int32_t dx, int32_t dy);

boolean map_init(void);

void map_fini(void);

void map_lightmap(map_t *map, int32_t lx, int32_t ly,
                  int32_t strength, boolean first_light);

void map_light_radiant(map_t *map, int32_t lx, int32_t ly);

uint16_t map_set_thing_template(map_t *map, int32_t x, int32_t y, 
                                uint16_t thing_template);
/*
 * cave.c
 */
void cave_gen(map_t *map, uint16_t rock,
              int8_t z,
              uint8_t map_fill_prob,
              uint8_t map_r1,
              uint8_t map_r2,
              uint8_t map_generations);

/*
 * fractal.c
 */
void fractal_gen(map_t *map,
                 float stdev,
                 float stdev_shrink_factor,
                 uint16_t rock,
                 uint16_t rock2);

/*
 * map_display.c
 */
void map_display(void);
void map_display_init(map_t *map);

/*
 * map_display_wid.c
 */
void map_display_wid_init(void);
void map_display_wid_fini(void);

/*
 * map_tiles.
 */
void map_init_tiles(map_t *map);
void map_combine(map_t *map);

static inline boolean map_out_of_bounds (int32_t x, int32_t y)
{
    if (x < 0) {
        return (true);
    }

    if (y < 0) {
        return (true);
    }

    if (x >= MAP_WIDTH) {
        return (true);
    }

    if (y >= MAP_HEIGHT) {
        return (true);
    }

    return (false);
}

static inline boolean map_almost_out_of_xy_bounds (int32_t x, int32_t y)
{
    if (x < 1) {
        return (true);
    }

    if (y < 1) {
        return (true);
    }

    if (x >= MAP_WIDTH-1) {
        return (true);
    }

    if (y >= MAP_HEIGHT-1) {
        return (true);
    }

    return (false);
}

/*
 * map_get_unsafe
 */
static inline uint16_t map_get_unsafe (map_t *map, int32_t x, int32_t y)
{
    return (map->tiles[x][y].template_id);
}

/*
 * map_get_light
 */
static inline uint8_t map_get_light (map_t *map, int32_t x, int32_t y)
{
    if (map_out_of_bounds(x, y)) {
        DIE("out of bounds on get map at %d,%d", x, y);
        return (0);
    }

    return (map->lit[x][y]);
}

/*
 * map_get_light
 */
static inline void 
map_set_light (map_t *map, int32_t x, int32_t y, uint8_t lit)
{
    if (map_out_of_bounds(x, y)) {
        DIE("out of bounds on set map at %d,%d", x, y);
        return;
    }

    map->lit[x][y] = lit;
}

uint16_t map_get(map_t *map, int32_t x, int32_t y);
thing_templatep map_get_thing_template(map_t *map, int32_t x, int32_t y);
