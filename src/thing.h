/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file.
 */

boolean thing_init(void);
void thing_fini(void);
thingp thing_server_new(levelp, const char *name);
thingp thing_client_new(uint32_t, thing_templatep);
void thing_restarted(thingp t, levelp level);
void thing_destroy(thingp, const char *why);
void thing_tick_all(void);
void thing_bury(thingp);
void thing_dead(thingp, thingp killer,
                const char *fmt, ...) __attribute__ ((format (printf, 3, 4)));
void things_level_start(levelp);
void things_level_destroyed(levelp);
void demarshal_thing(demarshal_p ctx, thingp);
void marshal_thing(marshal_p ctx, thingp);
void thing_templates_marshal(marshal_p out);
void things_marshal(marshal_p out);
void thing_set_wid(thingp, widp);
int32_t thing_grid_x(thingp);
int32_t thing_grid_y(thingp);
widp thing_wid(thingp);
const char *thing_dead_reason(thingp);
thing_templatep thing_get_template(thingp t);
const char *thing_name(thingp);
const char *thing_logname(thingp);
const char *thing_short_name(thingp);
uint8_t thing_z_depth(thingp);
uint8_t thing_z_order(thingp);
tree_rootp thing_tile_tiles(thingp);
tree_rootp thing_tile_tiles2(thingp);
uint32_t thing_level_no(thingp);
void thing_set_level_no(thingp, uint32_t level);
levelp thing_level(thingp);
void thing_set_level(thingp, levelp level);
uint32_t thing_score(thingp);
void thing_set_score(thingp, uint32_t score);
uint32_t thing_score_pump(thingp);
widp thing_message(thingp t, const char *message);
void thing_set_score_pump(thingp, uint32_t score);
void thing_inc_score_pump(thingp, uint32_t score);
void thing_animate(thingp);
boolean thing_find_nexthop(thingp t, int32_t *x, int32_t *y);
void thing_handle_collisions(widp grid, thingp t);
boolean thing_hit_solid_obstacle(widp grid, thingp t, double nx, double ny);
void thing_inc_powerup_spam_count(thingp t, boolean val);
void thing_dec_powerup_spam_count(thingp t, boolean val);
boolean thing_has_powerup_spam_count(thingp t);
void thing_inc_powerup2_count(thingp t, boolean val);
void thing_dec_powerup2_count(thingp t, boolean val);
boolean thing_has_powerup2_count(thingp t);
void thing_inc_powerup3_count(thingp t, boolean val);
void thing_dec_powerup3_count(thingp t, boolean val);
boolean thing_has_powerup3_count(thingp t);
void thing_inc_powerup4_count(thingp t, boolean val);
void thing_dec_powerup4_count(thingp t, boolean val);
boolean thing_has_powerup4_count(thingp t);
void thing_inc_powerup5_count(thingp t, boolean val);
void thing_dec_powerup5_count(thingp t, boolean val);
boolean thing_has_powerup5_count(thingp t);
void thing_inc_powerup6_count(thingp t, boolean val);
void thing_dec_powerup6_count(thingp t, boolean val);
boolean thing_has_powerup6_count(thingp t);
void thing_inc_powerup7_count(thingp t, boolean val);
void thing_dec_powerup7_count(thingp t, boolean val);
boolean thing_has_powerup7_count(thingp t);
void thing_inc_powerup8_count(thingp t, boolean val);
void thing_dec_powerup8_count(thingp t, boolean val);
boolean thing_has_powerup8_count(thingp t);
void thing_inc_powerup_rocket_count(thingp t, boolean val);
void thing_dec_powerup_rocket_count(thingp t, boolean val);
boolean thing_has_powerup_rocket_count(thingp t);
void thing_set_is_qqq1(thingp t, boolean val);
boolean thing_is_qqq1(thingp t);
void thing_set_is_qqq2(thingp t, boolean val);
boolean thing_is_qqq2(thingp t);
void thing_set_is_qqq3(thingp t, boolean val);
boolean thing_is_qqq3(thingp t);
void thing_set_is_qqq4(thingp t, boolean val);
boolean thing_is_qqq4(thingp t);
void thing_set_is_qqq5(thingp t, boolean val);
boolean thing_is_qqq5(thingp t);
void thing_set_is_qqq6(thingp t, boolean val);
boolean thing_is_qqq6(thingp t);
void thing_set_is_qqq7(thingp t, boolean val);
boolean thing_is_qqq7(thingp t);
void thing_set_is_chasing_target(thingp t, boolean val);
boolean thing_is_chasing_target(thingp t);
void thing_set_got_to_exit_first(thingp t, boolean val);
boolean thing_got_to_exit_first(thingp t);
void thing_set_redo_maze_search(thingp t, boolean val);
boolean thing_redo_maze_search(thingp t);

void thing_set_is_dir_down(thingp t);
uint8_t thing_is_dir_down(thingp t);
void thing_set_is_dir_up(thingp t);
uint8_t thing_is_dir_up(thingp t);
void thing_set_is_dir_left(thingp t);
uint8_t thing_is_dir_left(thingp t);
void thing_set_is_dir_right(thingp t);
uint8_t thing_is_dir_right(thingp t);

void thing_set_is_dir_tl(thingp t);
uint8_t thing_is_dir_tl(thingp t);
void thing_set_is_dir_bl(thingp t);
uint8_t thing_is_dir_bl(thingp t);
void thing_set_is_dir_tr(thingp t);
uint8_t thing_is_dir_tr(thingp t);
void thing_set_is_dir_br(thingp t);
uint8_t thing_is_dir_br(thingp t);

void thing_set_opened_exit(thingp t, boolean val);
boolean thing_opened_exit(thingp t);
void thing_set_is_open(thingp t, boolean val);
boolean thing_is_open(thingp t);
void thing_set_is_buried(thingp t, boolean val);
boolean thing_is_buried(thingp t);
void thing_set_is_left_as_corpse_on_death(thingp t, boolean val);
boolean thing_is_left_as_corpse_on_death(thingp t);
void thing_set_is_dead(thingp t, boolean val);
boolean thing_is_dead(thingp t);
void thing_set_qqq20(thingp t, boolean val);
boolean thing_qqq20(thingp t);
const char *thing_name(thingp);
const char *thing_short_name(thingp);
const char *thing_tooltip(thingp);
uint8_t thing_z_depth(thingp);
uint8_t thing_z_order(thingp);
int32_t thing_player_cost(thingp);
int32_t thing_monst_cost(thingp);
uint32_t thing_speed(thingp);
tree_rootp thing_tiles(thingp);
tree_rootp thing_tiles2(thingp);

boolean thing_is_exit(thingp t);
boolean thing_is_floor(thingp t);
boolean thing_is_food(thingp t);
boolean thing_is_monst(thingp t);
boolean thing_is_plant(thingp t);
boolean thing_is_player(thingp t);
boolean thing_is_key(thingp t);
boolean thing_is_xxx2(thingp t);
boolean thing_is_xxx3(thingp t);
boolean thing_is_xxx4(thingp t);
boolean thing_is_xxx5(thingp t);
boolean thing_is_xxx6(thingp t);
boolean thing_is_xxx7(thingp t);
boolean thing_is_xxx8(thingp t);
boolean thing_is_star(thingp t);
boolean thing_is_key0(thingp t);
boolean thing_is_key1(thingp t);
boolean thing_is_key2(thingp t);
boolean thing_is_key3(thingp t);
boolean thing_is_key4(thingp t);
boolean thing_is_key5(thingp t);
boolean thing_is_key6(thingp t);
boolean thing_is_key7(thingp t);
boolean thing_is_key8(thingp t);
boolean thing_is_xxx20(thingp t);
boolean thing_is_xxx20(thingp t);
boolean thing_is_xxx21(thingp t);
boolean thing_is_seedpod(thingp t);
boolean thing_is_bomb(thingp t);
boolean thing_is_spam(thingp t);
boolean thing_is_door(thingp t);
boolean thing_is_pipe(thingp t);
boolean thing_is_scarable(thingp t);
boolean thing_is_xxx29(thingp t);
boolean thing_is_hidden_from_editor(thingp t);
boolean thing_is_animated(thingp t);
boolean thing_is_xxx33(thingp t);
boolean thing_is_left_as_corpse_on_death(thingp t);
boolean thing_is_thing(thingp t);
boolean thing_is_joinable(thingp t);
boolean thing_is_wall(thingp t);
boolean thing_is_effect_sway(thingp t);
boolean thing_is_effect_pulse(thingp t);
boolean thing_is_effect_rotate_4way(thingp t);
boolean thing_is_effect_rotate_2way(thingp t);
int16_t thing_path_cost_is_exit(thingp t);
int16_t thing_path_cost_is_floor(thingp t);
int16_t thing_path_cost_is_food(thingp t);
int16_t thing_path_cost_is_monst(thingp t);
int16_t thing_path_cost_is_plant(thingp t);
int16_t thing_path_cost_is_player(thingp t);
int16_t thing_path_cost_is_key(thingp t);
int16_t thing_path_cost_is_xxx2(thingp t);
int16_t thing_path_cost_is_xxx3(thingp t);
int16_t thing_path_cost_is_xxx4(thingp t);
int16_t thing_path_cost_is_xxx5(thingp t);
int16_t thing_path_cost_is_xxx6(thingp t);
int16_t thing_path_cost_is_xxx7(thingp t);
int16_t thing_path_cost_is_xxx8(thingp t);
int16_t thing_path_cost_is_star(thingp t);
int16_t thing_path_cost_is_key0(thingp t);
int16_t thing_path_cost_is_key1(thingp t);
int16_t thing_path_cost_is_key2(thingp t);
int16_t thing_path_cost_is_key3(thingp t);
int16_t thing_path_cost_is_key4(thingp t);
int16_t thing_path_cost_is_key5(thingp t);
int16_t thing_path_cost_is_key6(thingp t);
int16_t thing_path_cost_is_key7(thingp t);
int16_t thing_path_cost_is_key8(thingp t);
int16_t thing_path_cost_is_xxx20(thingp t);
int16_t thing_path_cost_is_xxx20(thingp t);
int16_t thing_path_cost_is_xxx21(thingp t);
int16_t thing_path_cost_is_seedpod(thingp t);
int16_t thing_path_cost_is_bomb(thingp t);
int16_t thing_path_cost_is_spam(thingp t);
int16_t thing_path_cost_is_door(thingp t);
int16_t thing_path_cost_is_pipe(thingp t);
int16_t thing_path_cost_is_scarable(thingp t);
int16_t thing_path_cost_is_xxx29(thingp t);
int16_t thing_path_cost_is_hidden_from_editor(thingp t);
int16_t thing_path_cost_is_animated(thingp t);
int16_t thing_path_cost_is_xxx33(thingp t);
int16_t thing_path_cost_is_left_as_corpse_on_death(thingp t);
int16_t thing_path_cost_is_thing(thingp t);
int16_t thing_path_cost_is_joinable(thingp t);
int16_t thing_path_cost_is_wall(thingp t);
int16_t thing_path_cost_is_effect_sway(thingp t);
int16_t thing_path_cost_is_effect_pulse(thingp t);
int16_t thing_path_cost_is_effect_rotate_4way(thingp t);
int16_t thing_path_cost_is_effect_rotate_2way(thingp t);
thing_tilep thing_current_tile(thingp t);
void thing_reached_exit(thingp t);
void thing_place(void *);
void thing_server_wid_update(thingp t, double x, double y);
void thing_client_wid_update(thingp t, double x, double y, boolean smooth);
void thing_collect(thingp t, thing_templatep tmp);
boolean thing_use(thingp t, uint32_t id);
boolean thing_has(thingp t, uint32_t id);

typedef struct {
    thing_templatep thing_template;
    levelp level;
    int32_t x;
    int32_t y;
} thing_place_context_t;

void thing_teleport(thingp t, int32_t x, int32_t y);
thingp thing_client_find(uint32_t thing_id);

extern tree_rootp server_active_things;
extern tree_rootp client_active_things;
extern tree_rootp server_boring_things;
extern tree_rootp client_boring_things;

enum {
    THING_DIR_NONE,
    THING_DIR_DOWN,
    THING_DIR_UP,
    THING_DIR_LEFT,
    THING_DIR_RIGHT,
    THING_DIR_TL,
    THING_DIR_BL,
    THING_DIR_TR,
    THING_DIR_BR,
};

enum {
    THING_STATE_BIT_SHIFT_UP,
    THING_STATE_BIT_SHIFT_DOWN,
    THING_STATE_BIT_SHIFT_LEFT,
    THING_STATE_BIT_SHIFT_RIGHT,
    THING_STATE_BIT_SHIFT_RESYNC,
    THING_STATE_BIT_SHIFT_IS_DEAD,
    THING_STATE_BIT_SHIFT_IS_BURIED,
};

void thing_server_move(thingp t,
                       double x,
                       double y,
                       const boolean up,
                       const boolean down,
                       const boolean left,
                       const boolean rigth);

void thing_client_move(thingp t,
                       double x,
                       double y,
                       const boolean up,
                       const boolean down,
                       const boolean left,
                       const boolean rigth);

extern uint16_t THING_WALL;
extern uint16_t THING_WALL2;
extern uint16_t THING_WALL3;
extern uint16_t THING_WALL4;
extern uint16_t THING_WALL5;
extern uint16_t THING_DOOR;
extern uint16_t THING_NOENTRY;
extern uint16_t THING_FOOD;
extern uint16_t THING_FLOOR;
extern uint16_t THING_FLOOR2;
extern uint16_t THING_FLOOR3;
extern uint16_t THING_FLOOR4;
extern uint16_t THING_FLOOR5;
extern uint16_t THING_FLOOR6;
extern uint16_t THING_WARRIOR;
extern uint16_t THING_VALKYRIE;
extern uint16_t THING_WIZARD;
extern uint16_t THING_ELF;
extern uint16_t THING_DWARF;
extern uint16_t THING_GHOST;
extern uint16_t THING_DEMON;
extern uint16_t THING_MONK;
extern uint16_t THING_LIZARD;
extern uint16_t THING_DEATH;
extern uint16_t THING_PLANT;
extern uint16_t THING_SEEDPOD;
extern uint16_t THING_PIPE;
extern uint16_t THING_EXPLOSION;
extern uint16_t THING_BOMB;
extern uint16_t THING_SPAM;
extern uint16_t THING_POTION1;
extern uint16_t THING_POTION2;
extern uint16_t THING_POTION3;
extern uint16_t THING_POTION4;
extern uint16_t THING_POTION5;
extern uint16_t THING_WATER1;
extern uint16_t THING_WATER2;
extern uint16_t THING_MASK1;
extern uint16_t THING_MASK2;
extern uint16_t THING_MASK3;
extern uint16_t THING_GEM1;
extern uint16_t THING_GEM2;
extern uint16_t THING_GEM3;
extern uint16_t THING_GEM4;
extern uint16_t THING_GEM5;
extern uint16_t THING_GEM6;
extern uint16_t THING_GEM7;
extern uint16_t THING_EXIT1;
extern uint16_t THING_EXIT2;
extern uint16_t THING_EXIT3;
extern uint16_t THING_EXIT4;
extern uint16_t THING_EXIT5;
extern uint16_t THING_EXIT6;
extern uint16_t THING_EXIT7;
extern uint16_t THING_GENERATOR1;
extern uint16_t THING_GENERATOR2;
extern uint16_t THING_GENERATOR3;
extern uint16_t THING_GENERATOR4;
extern uint16_t THING_GENERATOR5;
extern uint16_t THING_GENERATOR6;
extern uint16_t THING_GENERATOR7;
extern uint16_t THING_KEYS1;
extern uint16_t THING_KEYS2;
extern uint16_t THING_KEYS3;
extern uint16_t THING_COINS1;
extern uint16_t THING_AMULET1;
extern uint16_t THING_CHEST1;
