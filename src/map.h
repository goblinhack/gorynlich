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
boolean map_is_letter_at(levelp, int32_t x, int32_t y);
boolean map_is_monst_at(levelp, int32_t x, int32_t y);
boolean map_is_item_perma_at(levelp, int32_t x, int32_t y);
boolean map_is_plant_at(levelp, int32_t x, int32_t y);
boolean map_is_player_at(levelp, int32_t x, int32_t y);
boolean map_is_snail_at(levelp, int32_t x, int32_t y);
boolean map_is_star_yellow_at(levelp, int32_t x, int32_t y);
boolean map_is_wall_at(levelp, int32_t x, int32_t y);
boolean map_is_animated_at(levelp, int32_t x, int32_t y);
boolean map_is_follows_owner_at(levelp, int32_t x, int32_t y);
boolean map_is_powerup_rocket_at(levelp, int32_t x, int32_t y);
boolean map_is_rock_at(levelp, int32_t x, int32_t y);
boolean map_is_water_at(levelp, int32_t x, int32_t y);
boolean map_is_lava_at(levelp, int32_t x, int32_t y);
boolean map_is_xxx4_at(levelp, int32_t x, int32_t y);
boolean map_is_xxx5_at(levelp, int32_t x, int32_t y);
boolean map_is_xxx6_at(levelp, int32_t x, int32_t y);
boolean map_is_xxx7_at(levelp, int32_t x, int32_t y);
boolean map_is_xxx8_at(levelp, int32_t x, int32_t y);
boolean map_is_star_at(levelp, int32_t x, int32_t y);
boolean map_is_powerup_spam_at(levelp, int32_t x, int32_t y);
boolean map_is_rock_0_at(levelp, int32_t x, int32_t y);
boolean map_is_car_at(levelp, int32_t x, int32_t y);
boolean map_is_star_green_at(levelp, int32_t x, int32_t y);
boolean map_is_star_cyan_at(levelp, int32_t x, int32_t y);
boolean map_is_star_black_at(levelp, int32_t x, int32_t y);
boolean map_is_star_purple_at(levelp, int32_t x, int32_t y);
boolean map_is_explosion_at(levelp, int32_t x, int32_t y);
boolean map_is_spikes_at(levelp, int32_t x, int32_t y);
boolean map_is_star_red_at(levelp, int32_t x, int32_t y);
boolean map_is_star_red_at(levelp, int32_t x, int32_t y);
boolean map_is_star_blue_at(levelp, int32_t x, int32_t y);
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

thing_templatep map_find_bonus_letter_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_rock_0_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_is_item_hidden_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_esnail_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_exit_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_floor_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_food_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_is_left_as_corpse_on_death_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_letter_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_monst_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_item_perma_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_plant_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_player_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_snail_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_spawns_thing_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_star_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_wall_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_rock_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_water_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_lava_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_xxx4_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_xxx5_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_xxx6_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_xxx7_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_xxx8_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_is_star_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_powerup_spam_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_rock_0_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_car_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_star_green_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_star_cyan_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_star_black_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_star_purple_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_explosion_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_spikes_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_is_star_red_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_is_star_red_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_is_star_blue_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_seedpod_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_bomb_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_spam_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_road_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_pipe_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_water7_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_water8_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_is_shrunk_when_carried_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_hidden_from_editor_at(levelp, int32_t x, int32_t y, widp *);

thingp map_thing_is_item_hidden_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_thing_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_bonus_letter_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_esnail_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_exit_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_floor_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_food_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_letter_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_monst_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_item_perma_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_plant_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_player_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_snail_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_star_yellow_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_wall_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_animated_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_follows_owner_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_powerup_rocket_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_left_as_corpse_on_death_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_rock_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_water_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_lava_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_xxx4_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_xxx5_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_xxx6_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_xxx7_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_xxx8_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_star_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_powerup_spam_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_rock_0_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_car_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_star_green_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_star_cyan_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_star_black_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_star_purple_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_explosion_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_spikes_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_star_red_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_star_red_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_star_blue_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_seedpod_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_bomb_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_spam_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_road_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_pipe_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_item_removed_at_level_end_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_scarable_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_shrunk_when_carried_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_hidden_from_editor_at(levelp, int32_t x, int32_t y);

tree_rootp map_all_things_is_item_hidden(levelp);
tree_rootp map_all_things_is_item_hidden_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_thing(levelp);
tree_rootp map_all_things_is_thing_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_bonus_letter(levelp);
tree_rootp map_all_things_is_bonus_letter_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_esnail(levelp);
tree_rootp map_all_things_is_esnail_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_exit(levelp);
tree_rootp map_all_things_is_exit_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_floor(levelp);
tree_rootp map_all_things_is_floor_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_food(levelp);
tree_rootp map_all_things_is_food_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_letter(levelp);
tree_rootp map_all_things_is_letter_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_monst(levelp);
tree_rootp map_all_things_is_monst_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_item_perma(levelp);
tree_rootp map_all_things_is_item_perma_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_plant(levelp);
tree_rootp map_all_things_is_plant_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_player(levelp);
tree_rootp map_all_things_is_player_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_snail(levelp);
tree_rootp map_all_things_is_snail_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_star_yellow(levelp);
tree_rootp map_all_things_is_star_yellow_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_wall(levelp);
tree_rootp map_all_things_is_wall_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_rock(levelp);
tree_rootp map_all_things_is_animated(levelp);
tree_rootp map_all_things_is_animated_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_follows_owner(levelp);
tree_rootp map_all_things_is_follows_owner_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_powerup_rocket(levelp);
tree_rootp map_all_things_is_powerup_rocket_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_rock_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_water(levelp);
tree_rootp map_all_things_is_water_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_lava(levelp);
tree_rootp map_all_things_is_lava_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_xxx4(levelp);
tree_rootp map_all_things_is_xxx4_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_xxx5(levelp);
tree_rootp map_all_things_is_xxx5_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_xxx6(levelp);
tree_rootp map_all_things_is_xxx6_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_xxx7(levelp);
tree_rootp map_all_things_is_xxx7_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_xxx8(levelp);
tree_rootp map_all_things_is_xxx8_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_star(levelp);
tree_rootp map_all_things_is_star_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_powerup_spam(levelp);
tree_rootp map_all_things_is_powerup_spam_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_rock_0(levelp);
tree_rootp map_all_things_is_rock_0_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_car(levelp);
tree_rootp map_all_things_is_car_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_star_green(levelp);
tree_rootp map_all_things_is_star_green_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_star_cyan(levelp);
tree_rootp map_all_things_is_star_cyan_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_star_black(levelp);
tree_rootp map_all_things_is_star_black_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_star_purple(levelp);
tree_rootp map_all_things_is_star_purple_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_explosion(levelp);
tree_rootp map_all_things_is_explosion_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_spikes(levelp);
tree_rootp map_all_things_is_spikes_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_star_red(levelp);
tree_rootp map_all_things_is_star_red_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_star_red(levelp);
tree_rootp map_all_things_is_star_red_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_star_blue(levelp);
tree_rootp map_all_things_is_star_blue_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_seedpod(levelp);
tree_rootp map_all_things_is_seedpod_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_bomb(levelp);
tree_rootp map_all_things_is_bomb_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_spam(levelp);
tree_rootp map_all_things_is_spam_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_road(levelp);
tree_rootp map_all_things_is_road_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_pipe(levelp);
tree_rootp map_all_things_is_pipe_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_item_removed_at_level_end(levelp);
tree_rootp map_all_things_is_item_removed_at_level_end_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_scarable(levelp);
tree_rootp map_all_things_is_scarable_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_shrunk_when_carried(levelp);
tree_rootp map_all_things_is_shrunk_when_carried_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_hidden_from_editor(levelp);
tree_rootp map_all_things_is_hidden_from_editor_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_left_as_corpse_on_death(levelp);
tree_rootp map_all_things_is_left_as_corpse_on_death_at(levelp, int32_t x, int32_t y);

uint32_t level_count_is_item_is_hidden(levelp);
uint32_t level_count_is_thing(levelp);
uint32_t level_count_is_bonus_letter(levelp);
uint32_t level_count_is_esnail(levelp);
uint32_t level_count_is_exit(levelp);
uint32_t level_count_is_floor(levelp);
uint32_t level_count_is_food(levelp);
uint32_t level_count_is_letter(levelp);
uint32_t level_count_is_monst(levelp);
uint32_t level_count_is_item_perma(levelp);
uint32_t level_count_is_plant(levelp);
uint32_t level_count_is_player(levelp);
uint32_t level_count_is_snail(levelp);
uint32_t level_count_is_star_yellow(levelp);
uint32_t level_count_is_wall(levelp);
uint32_t level_count_is_follows_owner(levelp);
uint32_t level_count_is_powerup_rocket(levelp);
uint32_t level_count_is_rock(levelp);
uint32_t level_count_is_water(levelp);
uint32_t level_count_is_lava(levelp);
uint32_t level_count_is_xxx4(levelp);
uint32_t level_count_is_xxx5(levelp);
uint32_t level_count_is_xxx6(levelp);
uint32_t level_count_is_xxx7(levelp);
uint32_t level_count_is_xxx8(levelp);
uint32_t level_count_is_star(levelp);
uint32_t level_count_is_powerup_spam(levelp);
uint32_t level_count_is_rock_0(levelp);
uint32_t level_count_is_car(levelp);
uint32_t level_count_is_star_green(levelp);
uint32_t level_count_is_star_cyan(levelp);
uint32_t level_count_is_star_black(levelp);
uint32_t level_count_is_star_purple(levelp);
uint32_t level_count_is_explosion(levelp);
uint32_t level_count_is_spikes(levelp);
uint32_t level_count_is_star_red(levelp);
uint32_t level_count_is_star_red(levelp);
uint32_t level_count_is_star_blue(levelp);
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
    uint16_t tile;
    float lit;
    thing_tilep thing_tile;
    thing_templatep thing_template;
} map_tile_t;

#define MAP_WIDTH_LOG           8
#define MAP_WIDTH               (1 << MAP_WIDTH_LOG)
#define MAP_HEIGHT              MAP_WIDTH
#define MAP_DEPTH               8

/*
 * All the rendering info for one parallax frame of tiles.
 */
typedef struct {
    /*
     * Main light.
     */
    int32_t lx;
    int32_t ly;
    int32_t lz;

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
    uint32_t tex_tiles_width;
    uint32_t tex_tiles_height;

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
    map_tile_t tiles[MAP_WIDTH][MAP_HEIGHT][MAP_DEPTH];
    map_tile_t tiles_copy[MAP_WIDTH][MAP_HEIGHT][MAP_DEPTH];

    /*
     * This is the huge buffer that contains the vertex and tex arrays.
     */
    float *gl_array_buf;
    float *gl_array_buf_end;
    uint32_t gl_array_size;

} map_frame_ctx_t;

extern map_frame_ctx_t *map_ctx;

/*
 * map.c
 */
void map_move_delta_pixels(int32_t dx, int32_t dy);

boolean map_init(void);

void map_fini(void);

void map_lightmap(map_frame_ctx_t *map,
                  int32_t lx, int32_t ly, int32_t lz,
                  int32_t strength,
                  boolean first_light);

thing_templatep map_set(map_frame_ctx_t *map,
                        int32_t x, int32_t y, int32_t z,
                        thing_templatep thing_template);

thing_templatep map_get(map_frame_ctx_t *map,
                        int32_t x, int32_t y, int32_t z);

/*
 * cave.c
 */
void cave_gen(map_frame_ctx_t *map, thing_templatep rock, int32_t z);

/*
 * fractal.c
 */
void fractal_gen(map_frame_ctx_t *map,
                 float stdev,
                 float stdev_shrink_factor,
                 thing_templatep rock,
                 thing_templatep rock2);

/*
 * map_display.c
 */
void map_display(void);
void map_display_init(map_frame_ctx_t *map);

/*
 * map_display_wid.c
 */
void map_display_wid_init(void);
void map_display_wid_fini(void);

/*
 * map_tiles.
 */
void map_init_tiles(map_frame_ctx_t *map);

static inline boolean map_out_of_bounds (int32_t x, int32_t y, int32_t z)
{
    if (z < 0) {
        return (true);
    }

    if (z >= MAP_DEPTH) {
        return (true);
    }

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
