/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file.
 */


boolean map_is_item_hidden_at(levelp, int32_t x, int32_t y);
boolean map_is_key1_at(levelp, int32_t x, int32_t y);
boolean map_is_exit_at(levelp, int32_t x, int32_t y);
boolean map_is_floor_at(levelp, int32_t x, int32_t y);
boolean map_is_food_at(levelp, int32_t x, int32_t y);
boolean map_is_monst_at(levelp, int32_t x, int32_t y);
boolean map_is_boring_at(levelp, int32_t x, int32_t y);
boolean map_is_plant_at(levelp, int32_t x, int32_t y);
boolean map_is_player_at(levelp, int32_t x, int32_t y);
boolean map_is_wall_at(levelp, int32_t x, int32_t y);
boolean map_is_animated_at(levelp, int32_t x, int32_t y);
boolean map_is_xxx33_at(levelp, int32_t x, int32_t y);
boolean map_is_key_at(levelp, int32_t x, int32_t y);
boolean map_is_xxx3_at(levelp, int32_t x, int32_t y);
boolean map_is_xxx4_at(levelp, int32_t x, int32_t y);
boolean map_is_xxx5_at(levelp, int32_t x, int32_t y);
boolean map_is_xxx6_at(levelp, int32_t x, int32_t y);
boolean map_is_xxx7_at(levelp, int32_t x, int32_t y);
boolean map_is_xxx8_at(levelp, int32_t x, int32_t y);
boolean map_is_star_at(levelp, int32_t x, int32_t y);
boolean map_is_key0_at(levelp, int32_t x, int32_t y);
boolean map_is_key1_at(levelp, int32_t x, int32_t y);
boolean map_is_key2_at(levelp, int32_t x, int32_t y);
boolean map_is_key3_at(levelp, int32_t x, int32_t y);
boolean map_is_key4_at(levelp, int32_t x, int32_t y);
boolean map_is_key5_at(levelp, int32_t x, int32_t y);
boolean map_is_key6_at(levelp, int32_t x, int32_t y);
boolean map_is_key7_at(levelp, int32_t x, int32_t y);
boolean map_is_key8_at(levelp, int32_t x, int32_t y);
boolean map_is_xxx20_at(levelp, int32_t x, int32_t y);
boolean map_is_xxx20_at(levelp, int32_t x, int32_t y);
boolean map_is_xxx21_at(levelp, int32_t x, int32_t y);
boolean map_is_seedpod_at(levelp, int32_t x, int32_t y);
boolean map_is_bomb_at(levelp, int32_t x, int32_t y);
boolean map_is_spam_at(levelp, int32_t x, int32_t y);
boolean map_is_door_at(levelp, int32_t x, int32_t y);
boolean map_is_pipe_at(levelp, int32_t x, int32_t y);
boolean map_is_generator_at(levelp, int32_t x, int32_t y);
boolean map_is_scarable_at(levelp, int32_t x, int32_t y);
boolean map_is_xxx29_at(levelp, int32_t x, int32_t y);
boolean map_is_hidden_from_editor_at(levelp, int32_t x, int32_t y);
boolean map_is_projectile_at(levelp, int32_t x, int32_t y);

thing_templatep map_find_key1_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_is_item_hidden_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_exit_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_floor_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_food_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_is_projectile_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_monst_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_item_perma_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_plant_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_player_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_polymorphs_thing_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_star_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_wall_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_key_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_xxx2_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_xxx3_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_xxx4_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_xxx5_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_xxx6_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_xxx7_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_xxx8_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_is_star_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_powerup_spam_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_key1_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_car_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_star_green_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_star_cyan_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_star_black_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_star_purple_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_explosion_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_spikes_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_is_xxx20_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_is_xxx20_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_is_xxx21_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_seedpod_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_bomb_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_spam_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_door_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_pipe_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_generator_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_xxx28_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_is_xxx29_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_hidden_from_editor_at(levelp, int32_t x, int32_t y, widp *);

void level_open_door(levelp level, int32_t ix, int32_t iy);

thingp map_thing_is_item_hidden_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_thing_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_exit_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_floor_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_food_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_monst_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_boring_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_plant_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_player_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_wall_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_animated_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_xxx33_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_projectile_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_key_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_xxx3_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_xxx4_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_xxx5_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_xxx6_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_xxx7_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_xxx8_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_star_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_key0_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_key1_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_key2_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_key3_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_key4_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_key5_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_key6_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_key7_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_key8_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_xxx20_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_xxx20_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_xxx21_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_seedpod_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_bomb_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_spam_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_door_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_pipe_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_generator_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_scarable_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_xxx29_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_hidden_from_editor_at(levelp, int32_t x, int32_t y);

tree_rootp map_all_things_is_item_hidden(levelp);
tree_rootp map_all_things_is_item_hidden_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_exit(levelp);
tree_rootp map_all_things_is_exit_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_floor(levelp);
tree_rootp map_all_things_is_floor_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_food(levelp);
tree_rootp map_all_things_is_food_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_monst(levelp);
tree_rootp map_all_things_is_monst_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_boring(levelp);
tree_rootp map_all_things_is_boring_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_plant(levelp);
tree_rootp map_all_things_is_plant_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_player(levelp);
tree_rootp map_all_things_is_player_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_wall(levelp);
tree_rootp map_all_things_is_wall_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_key(levelp);
tree_rootp map_all_things_is_animated(levelp);
tree_rootp map_all_things_is_animated_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_xxx33(levelp);
tree_rootp map_all_things_is_xxx33_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_key_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_xxx3(levelp);
tree_rootp map_all_things_is_xxx3_at(levelp, int32_t x, int32_t y);
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
tree_rootp map_all_things_is_key0(levelp);
tree_rootp map_all_things_is_key0_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_key1(levelp);
tree_rootp map_all_things_is_key1_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_key2(levelp);
tree_rootp map_all_things_is_key2_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_key3(levelp);
tree_rootp map_all_things_is_key3_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_key4(levelp);
tree_rootp map_all_things_is_key4_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_key5(levelp);
tree_rootp map_all_things_is_key5_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_key6(levelp);
tree_rootp map_all_things_is_key6_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_key7(levelp);
tree_rootp map_all_things_is_key7_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_key8(levelp);
tree_rootp map_all_things_is_key8_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_xxx20(levelp);
tree_rootp map_all_things_is_xxx20_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_xxx20(levelp);
tree_rootp map_all_things_is_xxx20_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_xxx21(levelp);
tree_rootp map_all_things_is_xxx21_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_seedpod(levelp);
tree_rootp map_all_things_is_seedpod_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_bomb(levelp);
tree_rootp map_all_things_is_bomb_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_spam(levelp);
tree_rootp map_all_things_is_spam_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_door(levelp);
tree_rootp map_all_things_is_door_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_pipe(levelp);
tree_rootp map_all_things_is_pipe_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_generator(levelp);
tree_rootp map_all_things_is_generator_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_scarable(levelp);
tree_rootp map_all_things_is_scarable_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_xxx29(levelp);
tree_rootp map_all_things_is_xxx29_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_hidden_from_editor(levelp);
tree_rootp map_all_things_is_hidden_from_editor_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_projectile(levelp);
tree_rootp map_all_things_is_projectile_at(levelp, int32_t x, int32_t y);

uint32_t level_count_is_item_is_hidden(levelp);
uint32_t level_count_is_exit(levelp);
uint32_t level_count_is_floor(levelp);
uint32_t level_count_is_food(levelp);
uint32_t level_count_is_monst(levelp);
uint32_t level_count_is_boring(levelp);
uint32_t level_count_is_plant(levelp);
uint32_t level_count_is_player(levelp);
uint32_t level_count_is_wall(levelp);
uint32_t level_count_is_xxx33(levelp);
uint32_t level_count_is_key(levelp);
uint32_t level_count_is_xxx3(levelp);
uint32_t level_count_is_xxx4(levelp);
uint32_t level_count_is_xxx5(levelp);
uint32_t level_count_is_xxx6(levelp);
uint32_t level_count_is_xxx7(levelp);
uint32_t level_count_is_xxx8(levelp);
uint32_t level_count_is_star(levelp);
uint32_t level_count_is_key0(levelp);
uint32_t level_count_is_key1(levelp);
uint32_t level_count_is_key2(levelp);
uint32_t level_count_is_key3(levelp);
uint32_t level_count_is_key4(levelp);
uint32_t level_count_is_key5(levelp);
uint32_t level_count_is_key6(levelp);
uint32_t level_count_is_key7(levelp);
uint32_t level_count_is_key8(levelp);
uint32_t level_count_is_xxx20(levelp);
uint32_t level_count_is_xxx20(levelp);
uint32_t level_count_is_xxx21(levelp);
uint32_t level_count_is_seedpod(levelp);
uint32_t level_count_is_bomb(levelp);
uint32_t level_count_is_spam(levelp);
uint32_t level_count_is_door(levelp);
uint32_t level_count_is_pipe(levelp);
uint32_t level_count_is_generator(levelp);
uint32_t level_count_is_scarable(levelp);
uint32_t level_count_is_xxx29(levelp);
uint32_t level_count_is_hidden_from_editor(levelp);
uint32_t level_count_is_projectile(levelp);

#include "tree.h"

typedef struct tree_thing_node_ {
    tree_key_pointer tree;
} tree_thing_node;

void map_fixup(levelp);
