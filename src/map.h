/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file.
 */


uint8_t map_is_ring_at(levelp, int32_t x, int32_t y);
uint8_t map_is_weapon_swing_effect_at(levelp, int32_t x, int32_t y);
uint8_t map_is_exit_at(levelp, int32_t x, int32_t y);
uint8_t map_is_floor_at(levelp, int32_t x, int32_t y);
uint8_t map_is_food_at(levelp, int32_t x, int32_t y);
uint8_t map_is_monst_at(levelp, int32_t x, int32_t y);
uint8_t map_is_boring_at(levelp, int32_t x, int32_t y);
uint8_t map_is_plant_at(levelp, int32_t x, int32_t y);
uint8_t map_is_player_at(levelp, int32_t x, int32_t y);
uint8_t map_is_wall_at(levelp, int32_t x, int32_t y);
uint8_t map_is_rock_at(levelp, int32_t x, int32_t y);
uint8_t map_is_animated_at(levelp, int32_t x, int32_t y);
uint8_t map_is_combustable_at(levelp, int32_t x, int32_t y);
uint8_t map_is_key_at(levelp, int32_t x, int32_t y);
uint8_t map_is_rock_at(levelp, int32_t x, int32_t y);
uint8_t map_is_shadow_caster_at(levelp, int32_t x, int32_t y);
uint8_t map_is_weapon_at(levelp, int32_t x, int32_t y);
uint8_t map_is_treasure_at(levelp, int32_t x, int32_t y);
uint8_t map_is_wearable_at(levelp, int32_t x, int32_t y);
uint8_t map_is_fragile_at(levelp, int32_t x, int32_t y);
uint8_t map_is_star_at(levelp, int32_t x, int32_t y);
uint8_t map_is_animated_no_dir_at(levelp, int32_t x, int32_t y);
uint8_t map_is_weapon_swing_effect_at(levelp, int32_t x, int32_t y);
uint8_t map_is_light_source_at(levelp, int32_t x, int32_t y);
uint8_t map_is_candle_light_at(levelp, int32_t x, int32_t y);
uint8_t map_is_cats_eyes_at(levelp, int32_t x, int32_t y);
uint8_t map_is_fire_at(levelp, int32_t x, int32_t y);
uint8_t map_is_ring_at(levelp, int32_t x, int32_t y);
uint8_t map_is_animation_at(levelp, int32_t x, int32_t y);
uint8_t map_is_shadow_caster_soft_at(levelp, int32_t x, int32_t y);
uint8_t map_is_poison_at(levelp, int32_t x, int32_t y);
uint8_t map_is_item_unusable_at(levelp, int32_t x, int32_t y);
uint8_t map_is_item_unusable_at(levelp, int32_t x, int32_t y);
uint8_t map_is_valid_for_action_bar_at(levelp, int32_t x, int32_t y);
uint8_t map_is_seedpod_at(levelp, int32_t x, int32_t y);
uint8_t map_is_spam_at(levelp, int32_t x, int32_t y);
uint8_t map_is_door_at(levelp, int32_t x, int32_t y);
uint8_t map_is_pipe_at(levelp, int32_t x, int32_t y);
uint8_t map_is_mob_spawner_at(levelp, int32_t x, int32_t y);
uint8_t map_is_rrr1_at(levelp, int32_t x, int32_t y);
uint8_t map_is_rrr2_at(levelp, int32_t x, int32_t y);
uint8_t map_is_rrr3_at(levelp, int32_t x, int32_t y);
uint8_t map_is_rrr4_at(levelp, int32_t x, int32_t y);
uint8_t map_is_rrr5_at(levelp, int32_t x, int32_t y);
uint8_t map_is_rrr6_at(levelp, int32_t x, int32_t y);
uint8_t map_is_rrr7_at(levelp, int32_t x, int32_t y);
uint8_t map_is_rrr8_at(levelp, int32_t x, int32_t y);
uint8_t map_is_rrr9_at(levelp, int32_t x, int32_t y);
uint8_t map_is_rrr10_at(levelp, int32_t x, int32_t y);
uint8_t map_is_rrr11_at(levelp, int32_t x, int32_t y);
uint8_t map_is_rrr12_at(levelp, int32_t x, int32_t y);
uint8_t map_is_magical_at(levelp, int32_t x, int32_t y);
uint8_t map_is_rrr14_at(levelp, int32_t x, int32_t y);
uint8_t map_is_cursed_at(levelp, int32_t x, int32_t y);
uint8_t map_is_animate_only_when_moving_at(levelp, int32_t x, int32_t y);
uint8_t map_is_warm_blooded_at(levelp, int32_t x, int32_t y);
uint8_t map_is_click_to_drop_at(levelp, int32_t x, int32_t y);
uint8_t map_is_click_to_use_at(levelp, int32_t x, int32_t y);
uint8_t map_is_torch_at(levelp, int32_t x, int32_t y);
uint8_t map_is_explosion_at(levelp, int32_t x, int32_t y);
uint8_t map_is_hidden_from_editor_at(levelp, int32_t x, int32_t y);
uint8_t map_is_projectile_at(levelp, int32_t x, int32_t y);

thing_templatep map_find_key1_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_is_ring_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_exit_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_floor_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_food_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_is_projectile_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_monst_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_item_perma_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_plant_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_player_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_star_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_wall_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_rock_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_key_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_xxx2_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_xxx3_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_blocks_light_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_xxx5_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_xxx6_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_xxx7_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_is_fragile_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_is_star_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_powerup_spam_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_key1_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_light_source_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_candle_light_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_star_green_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_star_cyan_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_star_black_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_star_purple_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_explosion_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_spikes_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_is_item_unusable_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_is_item_unusable_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_is_valid_for_action_bar_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_seedpod_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_bomb_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_spam_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_door_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_pipe_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_generator_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_xxx28_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_is_explosion_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_hidden_from_editor_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_rrr1_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_rrr2_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_rrr3_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_rrr4_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_rrr5_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_rrr6_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_rrr7_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_rrr8_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_rrr9_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_rrr10_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_rrr11_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_rrr12_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_magical_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_rrr14_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_cursed_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_animate_only_when_moving_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_rrr17_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_rrr18_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_rrr19_at(levelp, int32_t x, int32_t y, widp *);
thing_templatep map_find_torch_at(levelp, int32_t x, int32_t y, widp *);

void level_open_door(levelp level, int32_t ix, int32_t iy);

thingp map_thing_is_ring_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_thing_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_exit_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_floor_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_food_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_monst_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_boring_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_plant_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_player_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_wall_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_rock_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_animated_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_combustable_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_projectile_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_key_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_rock_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_shadow_caster_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_weapon_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_treasure_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_wearable_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_fragile_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_star_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_animated_no_dir_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_weapon_swing_effect_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_light_source_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_candle_light_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_cats_eyes_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_fire_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_ring_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_animation_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_animation_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_poison_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_item_unusable_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_item_unusable_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_valid_for_action_bar_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_seedpod_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_bomb_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_spam_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_door_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_pipe_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_mob_spawner_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_rrr1_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_rrr2_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_rrr3_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_rrr4_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_rrr5_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_rrr6_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_rrr7_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_rrr8_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_rrr9_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_rrr10_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_rrr11_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_rrr12_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_magical_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_rrr14_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_cursed_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_animate_only_when_moving_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_warm_blooded_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_click_to_drop_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_click_to_use_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_torch_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_explosion_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_hidden_from_editor_at(levelp, int32_t x, int32_t y);

tree_rootp map_all_things_is_ring(levelp);
tree_rootp map_all_things_is_ring_at(levelp, int32_t x, int32_t y);
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
tree_rootp map_all_things_is_combustable(levelp);
tree_rootp map_all_things_is_combustable_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_key_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_rock(levelp);
tree_rootp map_all_things_is_rock_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_shadow_caster(levelp);
tree_rootp map_all_things_is_shadow_caster_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_weapon(levelp);
tree_rootp map_all_things_is_weapon_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_treasure(levelp);
tree_rootp map_all_things_is_treasure_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_wearable(levelp);
tree_rootp map_all_things_is_wearable_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_fragile(levelp);
tree_rootp map_all_things_is_fragile_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_star(levelp);
tree_rootp map_all_things_is_star_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_animated_no_dir(levelp);
tree_rootp map_all_things_is_animated_no_dir_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_weapon_swing_effect(levelp);
tree_rootp map_all_things_is_weapon_swing_effect_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_light_source(levelp);
tree_rootp map_all_things_is_light_source_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_candle_light(levelp);
tree_rootp map_all_things_is_candle_light_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_cats_eyes(levelp);
tree_rootp map_all_things_is_cats_eyes_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_fire(levelp);
tree_rootp map_all_things_is_fire_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_ring(levelp);
tree_rootp map_all_things_is_ring_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_animation(levelp);
tree_rootp map_all_things_is_animation_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_animation(levelp);
tree_rootp map_all_things_is_animation_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_poison(levelp);
tree_rootp map_all_things_is_poison_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_item_unusable(levelp);
tree_rootp map_all_things_is_item_unusable_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_item_unusable(levelp);
tree_rootp map_all_things_is_item_unusable_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_valid_for_action_bar(levelp);
tree_rootp map_all_things_is_valid_for_action_bar_at(levelp, int32_t x, int32_t y);
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
tree_rootp map_all_things_is_mob_spawner(levelp);
tree_rootp map_all_things_is_mob_spawner_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_rrr1(levelp);
tree_rootp map_all_things_is_rrr2(levelp);
tree_rootp map_all_things_is_rrr3(levelp);
tree_rootp map_all_things_is_rrr4(levelp);
tree_rootp map_all_things_is_rrr5(levelp);
tree_rootp map_all_things_is_rrr6(levelp);
tree_rootp map_all_things_is_rrr7(levelp);
tree_rootp map_all_things_is_rrr8(levelp);
tree_rootp map_all_things_is_rrr9(levelp);
tree_rootp map_all_things_is_rrr10(levelp);
tree_rootp map_all_things_is_rrr11(levelp);
tree_rootp map_all_things_is_rrr12(levelp);
tree_rootp map_all_things_is_magical(levelp);
tree_rootp map_all_things_is_rrr14(levelp);
tree_rootp map_all_things_is_cursed(levelp);
tree_rootp map_all_things_is_animate_only_when_moving(levelp);
tree_rootp map_all_things_is_warm_blooded(levelp);
tree_rootp map_all_things_is_click_to_drop(levelp);
tree_rootp map_all_things_is_click_to_use(levelp);
tree_rootp map_all_things_is_torch(levelp);
tree_rootp map_all_things_is_rrr1_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_rrr2_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_rrr3_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_rrr4_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_rrr5_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_rrr6_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_rrr7_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_rrr8_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_rrr9_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_rrr10_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_rrr11_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_rrr12_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_magical_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_rrr14_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_cursed_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_animate_only_when_moving_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_warm_blooded_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_click_to_drop_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_click_to_use_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_torch_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_explosion(levelp);
tree_rootp map_all_things_is_explosion_at(levelp, int32_t x, int32_t y);
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
uint32_t level_count_is_combustable(levelp);
uint32_t level_count_is_key(levelp);
uint32_t level_count_is_rock(levelp);
uint32_t level_count_is_shadow_caster(levelp);
uint32_t level_count_is_weapon(levelp);
uint32_t level_count_is_treasure(levelp);
uint32_t level_count_is_wearable(levelp);
uint32_t level_count_is_fragile(levelp);
uint32_t level_count_is_star(levelp);
uint32_t level_count_is_animated_no_dir(levelp);
uint32_t level_count_is_weapon_swing_effect(levelp);
uint32_t level_count_is_light_source(levelp);
uint32_t level_count_is_candle_light(levelp);
uint32_t level_count_is_cats_eyes(levelp);
uint32_t level_count_is_fire(levelp);
uint32_t level_count_is_ring(levelp);
uint32_t level_count_is_animation(levelp);
uint32_t level_count_is_animation(levelp);
uint32_t level_count_is_poison(levelp);
uint32_t level_count_is_item_unusable(levelp);
uint32_t level_count_is_item_unusable(levelp);
uint32_t level_count_is_valid_for_action_bar(levelp);
uint32_t level_count_is_seedpod(levelp);
uint32_t level_count_is_bomb(levelp);
uint32_t level_count_is_spam(levelp);
uint32_t level_count_is_door(levelp);
uint32_t level_count_is_pipe(levelp);
uint32_t level_count_is_mob_spawner(levelp);
uint32_t level_count_is_rrr1(levelp);
uint32_t level_count_is_rrr2(levelp);
uint32_t level_count_is_rrr3(levelp);
uint32_t level_count_is_rrr4(levelp);
uint32_t level_count_is_rrr5(levelp);
uint32_t level_count_is_rrr6(levelp);
uint32_t level_count_is_rrr7(levelp);
uint32_t level_count_is_rrr8(levelp);
uint32_t level_count_is_rrr9(levelp);
uint32_t level_count_is_rrr10(levelp);
uint32_t level_count_is_rrr11(levelp);
uint32_t level_count_is_rrr12(levelp);
uint32_t level_count_is_magical(levelp);
uint32_t level_count_is_rrr14(levelp);
uint32_t level_count_is_cursed(levelp);
uint32_t level_count_is_animate_only_when_moving(levelp);
uint32_t level_count_is_warm_blooded(levelp);
uint32_t level_count_is_click_to_drop(levelp);
uint32_t level_count_is_click_to_use(levelp);
uint32_t level_count_is_torch(levelp);
uint32_t level_count_is_explosion(levelp);
uint32_t level_count_is_hidden_from_editor(levelp);
uint32_t level_count_is_projectile(levelp);

#include "tree.h"

typedef struct tree_thing_node_ {
    tree_key_pointer tree;
} tree_thing_node;

void map_fixup(levelp);
