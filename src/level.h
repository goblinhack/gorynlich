/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file.
 */

uint8_t level_init(void);
void level_fini(void);

levelp level_new(widp,
                 level_pos_t, 
                 int is_editor, 
                 int is_map_editor, 
                 int on_server);
void level_destroy(levelp *, uint8_t keep_players);
void level_server_tick(levelp);
void level_client_tick(levelp);
void level_update_now(levelp level);
void level_load_new(void);
levelp level_load(level_pos_t,
                  widp, 
                  int is_editor, 
                  int is_map_editor, 
                  int on_server);
levelp level_load_random(level_pos_t level, 
                         widp, 
                         int is_editor, 
                         int is_map_editor, 
                         int on_server);
void level_pause(levelp level);
void marshal_level(marshal_p ctx, levelp level);
uint8_t demarshal_level(demarshal_p ctx, levelp l);
const char *level_get_title(levelp level);
void level_set_title(levelp level, const char *val);
const char *level_get_destroy_reason(levelp level);
void level_set_destroy_reason(levelp level, const char *val);
widp level_get_map(levelp level);
void level_set_map(levelp level, widp val);

level_pos_t level_get_level_pos(levelp level);
void level_set_level_pos(levelp level, level_pos_t val);

uint32_t level_get_seed(levelp level);
void level_set_seed(levelp level, uint32_t val);

const char *level_get_logname(levelp level);
uint32_t level_get_timestamp_started(levelp level);
void level_set_timestamp_started(levelp level, uint32_t val);
uint8_t level_needs_updating(levelp level);
void level_set_needs_updating(levelp level, uint8_t val);
uint8_t level_death_is_coming(levelp level);
void level_set_death_is_coming(levelp level, uint8_t val);
uint8_t level_death_is_coming_soon(levelp level);
void level_set_death_is_coming_soon(levelp level, uint8_t val);
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
uint8_t level_zzz16(levelp level);
void level_set_zzz16(levelp level, uint8_t val);
uint8_t level_is_death_summoned(levelp level);
void level_set_is_death_summoned(levelp level, uint8_t val);
uint8_t level_is_jesus_summoned(levelp level);
void level_set_is_jesus_summoned(levelp level, uint8_t val);
uint8_t level_is_blit_y_offset(levelp level);
void level_set_is_blit_y_offset(levelp level, uint8_t val);
uint8_t level_exit_reached_when_open(levelp level);
void level_set_exit_reached_when_open(levelp level, uint8_t val);
uint8_t level_get_warned_exit_not_open(levelp level);
void level_set_warned_exit_not_open(levelp level, uint8_t val);
uint8_t level_is_exit_open(levelp level);
void level_set_is_exit_open(levelp level, uint8_t val);
uint8_t level_is_editor(levelp level);
void level_set_is_editor(levelp level, uint8_t val);
uint8_t level_is_map_editor(levelp level);
void level_set_is_map_editor(levelp level, uint8_t val);
void level_set_exit_has_been_reached(levelp level, uint8_t val);
uint8_t level_exit_has_been_reached(levelp level);
void level_set_needs_fixup(levelp level, uint8_t val);
uint8_t level_needs_fixup(levelp level);
void level_set_is_ready_to_be_destroyed(levelp level, uint8_t val);
uint8_t level_is_ready_to_be_destroyed(levelp level);
void level_set_is_ready_to_fade_out(levelp level, uint8_t val);
uint8_t level_is_ready_to_fade_out(levelp level);

uint8_t level_is_paused(levelp level);
void level_set_is_paused(levelp level, uint8_t val);

void level_set_has_shop(levelp level, uint8_t val);
uint8_t level_has_shop(levelp level);

/*
 * level_trigger.c
 */
int level_trigger2slot(levelp level, const char *name);
void level_trigger_activate(levelp, const char *name);
void level_trigger_alloc(levelp level, const char *name);
int level_trigger_is_activated(levelp level, const char *name);
void level_trigger_activate_default_triggers(levelp);
void level_trigger_move_thing(tpp me, thingp t);
