/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file.
 */

uint8_t level_init(void);
void level_fini(void);

levelp level_new(widp, uint32_t, int is_editor, int on_server);
void level_completed(levelp);
void level_destroy(levelp *, uint8_t keep_players);
void level_tick(levelp);
void level_update(levelp level);
void level_update_now(levelp level);
levelp level_load(uint32_t level, widp, int is_editor, int on_server);
void marshal_level(marshal_p ctx, levelp level);
uint8_t demarshal_level(demarshal_p ctx, levelp l);
const char *level_get_title(levelp level);
void level_set_title(levelp level, const char *val);
const char *level_get_destroy_reason(levelp level);
void level_set_destroy_reason(levelp level, const char *val);
widp level_get_map(levelp level);
void level_set_map(levelp level, widp val);
void level_set_walls(levelp level);
void level_set_monst_map_treat_doors_as_passable(levelp level);
void level_set_monst_map_treat_doors_as_walls(levelp level);
void level_set_player_map_treat_doors_as_walls(levelp level);
void level_set_doors(levelp level);
void level_set_pipes(levelp level);
void level_pipe_find_ends(levelp level);
uint8_t level_pipe_find_exit(levelp level,
                    int32_t ix, int32_t iy, int32_t *exit_x, int32_t *exit_y);
uint32_t level_get_level_no(levelp level);
const char *level_get_logname(levelp level);
void level_set_level_no(levelp level, uint32_t val);
uint32_t level_get_timestamp_started(levelp level);
void level_set_timestamp_started(levelp level, uint32_t val);
uint8_t level_is_hurryup(levelp level);
void level_set_is_hurryup(levelp level, uint8_t val);
uint8_t level_is_zzz1(levelp level);
void level_set_is_zzz1(levelp level, uint8_t val);
uint8_t level_is_zzz2(levelp level);
void level_set_is_zzz2(levelp level, uint8_t val);
uint8_t level_is_zzz3(levelp level);
void level_set_is_zzz3(levelp level, uint8_t val);
uint8_t level_is_zzz4(levelp level);
void level_set_is_zzz4(levelp level, uint8_t val);
uint8_t level_is_zzz5(levelp level);
void level_set_is_zzz5(levelp level, uint8_t val);
uint8_t level_is_zzz6(levelp level);
void level_set_is_zzz6(levelp level, uint8_t val);
uint8_t level_is_zzz7(levelp level);
void level_set_is_zzz7(levelp level, uint8_t val);
uint8_t level_is_zzz8(levelp level);
void level_set_is_zzz8(levelp level, uint8_t val);
uint8_t level_is_zzz9(levelp level);
void level_set_is_zzz9(levelp level, uint8_t val);
uint8_t level_is_zzz10(levelp level);
void level_set_is_zzz10(levelp level, uint8_t val);
uint8_t level_is_zzz11(levelp level);
void level_set_is_zzz11(levelp level, uint8_t val);
uint8_t level_is_zzz12(levelp level);
void level_set_is_zzz12(levelp level, uint8_t val);
uint8_t level_is_zzz13(levelp level);
void level_set_is_zzz13(levelp level, uint8_t val);
uint8_t level_is_zzz14(levelp level);
void level_set_is_zzz14(levelp level, uint8_t val);
uint8_t level_is_zzz15(levelp level);
void level_set_is_zzz15(levelp level, uint8_t val);
uint8_t level_is_zzz16(levelp level);
void level_set_is_zzz16(levelp level, uint8_t val);
uint8_t level_is_zzz17(levelp level);
void level_set_is_zzz17(levelp level, uint8_t val);
uint8_t level_exit_reached_when_open(levelp level);
void level_set_exit_reached_when_open(levelp level, uint8_t val);
uint8_t level_get_warned_exit_not_open(levelp level);
void level_set_warned_exit_not_open(levelp level, uint8_t val);
uint8_t level_is_exit_open(levelp level);
void level_set_is_exit_open(levelp level, uint8_t val);
uint8_t level_is_editor(levelp level);
void level_set_is_editor(levelp level, uint8_t val);
uint8_t level_is_paused(levelp level);
void level_set_is_completed(levelp level, uint8_t val);
uint8_t level_is_completed(levelp level);
void level_set_is_paused(levelp level, uint8_t val);
void level_place_plant_pod(levelp);
void level_place_explosion(levelp level, 
                           thingp owner,
                           double x, double y);
void level_place_small_explosion(levelp level, 
                                 thingp owner,
                                 double x, double y);
void level_place_potion_effect_fireball(levelp level, 
                                thingp owner,
                                double x, double y);
void level_place_potion_effect_poison(levelp level, 
                                thingp owner,
                                double x, double y);
void level_place_potion_effect_cloudkill(levelp level, 
                                thingp owner,
                                double x, double y);
extern uint8_t game_over;

extern uint32_t start_lives;
