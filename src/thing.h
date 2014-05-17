/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file.
 */

#pragma once

#include "tree.h"
#include "level_private.h"
#include <stdlib.h>

uint8_t thing_init(void);
void thing_fini(void);
thingp thing_server_new(levelp, const char *name);
thingp thing_client_new(uint32_t, thing_templatep);
void thing_restarted(thingp t, levelp level);
void thing_destroy(thingp, const char *why);
void thing_tick_all(void);
uint8_t thing_mob_spawn(thingp);
void thing_dead(thingp, thingp killer,
                const char *fmt, ...) __attribute__ ((format (printf, 3, 4)));
void thing_hit(thingp, thingp hitter, uint32_t damage,
               const char *fmt, ...) __attribute__ ((format (printf, 4, 5)));
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
widp thing_message(thingp t, const char *message);
void thing_animate(thingp);
void thing_handle_collisions(widp grid, thingp t);
uint8_t thing_hit_solid_obstacle(widp grid, thingp t, double nx, double ny);
uint8_t thing_server_hit_solid_obstacle(widp grid, 
                                        thingp t, double nx, double ny);
void thing_inc_powerup_spam_count(thingp t, uint8_t val);
void thing_dec_powerup_spam_count(thingp t, uint8_t val);
uint8_t thing_has_powerup_spam_count(thingp t);
void thing_inc_powerup2_count(thingp t, uint8_t val);
void thing_dec_powerup2_count(thingp t, uint8_t val);
uint8_t thing_has_powerup2_count(thingp t);
void thing_inc_powerup3_count(thingp t, uint8_t val);
void thing_dec_powerup3_count(thingp t, uint8_t val);
uint8_t thing_has_powerup3_count(thingp t);
void thing_inc_powerup4_count(thingp t, uint8_t val);
void thing_dec_powerup4_count(thingp t, uint8_t val);
uint8_t thing_has_powerup4_count(thingp t);
void thing_inc_powerup5_count(thingp t, uint8_t val);
void thing_dec_powerup5_count(thingp t, uint8_t val);
uint8_t thing_has_powerup5_count(thingp t);
void thing_inc_powerup6_count(thingp t, uint8_t val);
void thing_dec_powerup6_count(thingp t, uint8_t val);
uint8_t thing_has_powerup6_count(thingp t);
void thing_inc_powerup7_count(thingp t, uint8_t val);
void thing_dec_powerup7_count(thingp t, uint8_t val);
uint8_t thing_has_powerup7_count(thingp t);
void thing_inc_powerup8_count(thingp t, uint8_t val);
void thing_dec_powerup8_count(thingp t, uint8_t val);
uint8_t thing_has_powerup8_count(thingp t);
void thing_inc_powerup_rocket_count(thingp t, uint8_t val);
void thing_dec_powerup_rocket_count(thingp t, uint8_t val);
uint8_t thing_has_powerup_rocket_count(thingp t);
void thing_set_is_qqq1(thingp t, uint8_t val);
uint8_t thing_is_qqq1(thingp t);
void thing_set_is_qqq2(thingp t, uint8_t val);
uint8_t thing_is_qqq2(thingp t);
void thing_set_is_qqq3(thingp t, uint8_t val);
uint8_t thing_is_qqq3(thingp t);
void thing_set_is_qqq4(thingp t, uint8_t val);
uint8_t thing_is_qqq4(thingp t);
void thing_set_is_qqq5(thingp t, uint8_t val);
uint8_t thing_is_qqq5(thingp t);
void thing_set_is_qqq6(thingp t, uint8_t val);
uint8_t thing_is_qqq6(thingp t);
void thing_set_is_qqq7(thingp t, uint8_t val);
uint8_t thing_is_qqq7(thingp t);
void thing_set_is_chasing_target(thingp t, uint8_t val);
uint8_t thing_is_chasing_target(thingp t);
void thing_set_got_to_exit_first(thingp t, uint8_t val);
uint8_t thing_got_to_exit_first(thingp t);
void thing_set_redo_maze_search(thingp t, uint8_t val);
uint8_t thing_redo_maze_search(thingp t);

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

void thing_set_opened_exit(thingp t, uint8_t val);
uint8_t thing_opened_exit(thingp t);
void thing_set_is_open(thingp t, uint8_t val);
void thing_set_is_projectile(thingp t, uint8_t val);
void thing_set_is_dead(thingp t, uint8_t val);
void thing_set_qqq20(thingp t, uint8_t val);
uint8_t thing_qqq20(thingp t);
const char *thing_name(thingp);
const char *thing_short_name(thingp);
const char *thing_tooltip(thingp);
uint8_t thing_z_depth(thingp);
uint8_t thing_z_order(thingp);
int32_t thing_player_cost(thingp);
int32_t thing_monst_cost(thingp);
double thing_speed(thingp);
tree_rootp thing_tiles(thingp);
tree_rootp thing_tiles2(thingp);

thing_tilep thing_current_tile(thingp t);
void thing_reached_exit(thingp t);
void thing_place(void *);
void thing_place_and_destroy_delayed(void *);
void thing_server_wid_update(thingp t, double x, double y, uint8_t is_new);
void thing_client_wid_update(thingp t, double x, double y, uint8_t smooth);
void thing_collect(thingp t, thing_templatep tmp);
uint8_t thing_use(thingp t, uint32_t id);
uint8_t thing_has(thingp t, uint32_t id);

/*
 * thing_ai.c
 */
uint8_t thing_find_nexthop(thingp t, int32_t *x, int32_t *y);
void dmap_generate_monst_map_wander(levelp level);
void thing_generate_dmaps(void);
void dmap_process_init(void);
void dmap_process_fini(void);

typedef struct {
    thing_templatep thing_template;
    levelp level;
    double x;
    double y;
    uint32_t destroy_in;
    thingp thing;
} thing_place_context_t;

void thing_teleport(thingp t, int32_t x, int32_t y);
thingp thing_client_find(uint32_t thing_id);

extern tree_root *server_player_things;
extern tree_root *client_player_things;

extern tree_rootp server_active_things;
extern tree_rootp client_active_things;

extern tree_rootp server_boring_things;
extern tree_rootp client_boring_things;

enum {
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
    THING_STATE_BIT_SHIFT_DIR_BIT0,
    THING_STATE_BIT_SHIFT_DIR_BIT1,
    THING_STATE_BIT_SHIFT_DIR_BIT2,
    THING_STATE_BIT_SHIFT_RESYNC,
    THING_STATE_BIT_SHIFT_IS_DEAD,
    THING_STATE_BIT_SHIFT_XY_PRESENT,
    THING_STATE_BIT_SHIFT_ID_DELTA_PRESENT,
    THING_STATE_BIT_SHIFT_ID_TEMPLATE_PRESENT,
};

uint8_t thing_server_move(thingp t,
                          double x,
                          double y,
                          const uint8_t up,
                          const uint8_t down,
                          const uint8_t left,
                          const uint8_t right,
                          const uint8_t fire);

void thing_client_move(thingp t,
                       double x,
                       double y,
                       const uint8_t up,
                       const uint8_t down,
                       const uint8_t left,
                       const uint8_t right,
                       const uint8_t fire);

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
extern uint16_t THING_GHOST1;
extern uint16_t THING_GHOST2;
extern uint16_t THING_GHOST3;
extern uint16_t THING_DEMON;
extern uint16_t THING_MONK;
extern uint16_t THING_LIZARD;
extern uint16_t THING_DEATH;
extern uint16_t THING_PLANT;
extern uint16_t THING_SEEDPOD;
extern uint16_t THING_PIPE;
extern uint16_t THING_EXPLOSION;
extern uint16_t THING_EXPLOSION1;
extern uint16_t THING_EXPLOSION2;
extern uint16_t THING_EXPLOSION3;
extern uint16_t THING_EXPLOSION4;
extern uint16_t THING_EXPLOSION5;
extern uint16_t THING_EXPLOSION6;
extern uint16_t THING_EXPLOSION7;
extern uint16_t THING_EXPLOSION8;
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
extern uint16_t THING_ARROW;
extern uint16_t THING_FIREBALL;

typedef struct thing_ {
    tree_key_int tree;

    /*
     * For player things.
     */
    tree_key_int tree2;

    /*
     * Unique id per thing.
     */
    uint16_t thing_id;

    /*
     * Player current level or level of monster.
     */
    uint16_t level_no;

    /*
     * Scoring
     */
    uint32_t score;

    /*
     * How much damage per hit?
     */
    uint16_t damage;

    /*
     * Thing health.
     */
    uint16_t health;

    /*
     * Which djkstra map this thing is using.
     */
    level_walls *dmap;
    level_walls *dmap_wander;

    /*
     * Pointer to common settings for this thing.
     */
    thing_templatep thing_template;

    /*
     * Name for debugging.
     */
    char *logname;

    /*
     * Which tree are we on, server active? client boring? etc...
     */
    tree_rootp client_or_server_tree;

    /*
     * Widget for displaying thing.
     */
    widp wid;

    /*
     * What level is the thing on?
     */
    levelp level;

    /*
     * Last death reason.
     */
    char *dead_reason;

    /*
     * Periodic spam message.
     */
    timerp timer_spam;

    /*
     * Powerups
     */
    uint8_t powerup_spam_count;
    uint8_t powerup2_count;
    uint8_t powerup3_count;
    uint8_t powerup4_count;
    uint8_t powerup5_count;
    uint8_t powerup6_count;
    uint8_t powerup7_count;
    uint8_t powerup8_count;
    uint8_t powerup_rocket_count;

    /*
     * How many and of what we are carrying.
     */
    uint8_t carrying[THING_MAX];

    /*
     * Grid coordinates.
     */
    double x;
    double y;

    /*
     * Previous hop where we were. We use this to interpolate the real 
     * position on the server when moving.
     */
    double last_x;
    double last_y;

    /*
     * For moving
     */
    double dx;
    double dy;

    /*
     * Last co-ords sent to the client
     */
    uint8_t last_tx;
    uint8_t last_ty;

    /*
     * For animation.
     */
    thing_tilep current_tile;

    /*
     * When to change frame for animation.
     */
    uint32_t timestamp_change_to_next_frame;

    /*
     * When we last teleported.
     */
    uint32_t timestamp_teleport;

    /*
     * When we last thunked about direction.
     */
    uint32_t timestamp_ai;

    /*
     * When we last spawned.
     */
    uint32_t timestamp_mob_spawn;

    /*
     * When do I die? 8(
     */
    uint32_t timestamp_lifestamp;

    /*
     * When did I last hit?
     */
    uint32_t timestamp_hit;

    /*
     * A counter to indicate the number of times we want to update the client 
     * with the state of this thing. To compensate for udp drops we might want 
     * to send a couple of updates for important events like death.
     */
    uint8_t updated;

    uint8_t dir:3;

    /*
     * First time sent to a client?
     */
    uint32_t first_update:1;

    uint32_t is_qqq1:1;
    uint32_t is_qqq2:1;
    uint32_t is_qqq3:1;
    uint32_t is_qqq4:1;
    uint32_t is_qqq5:1;
    uint32_t is_qqq6:1;
    uint32_t is_qqq7:1;
    uint32_t got_to_exit_first:1;
    uint32_t opened_exit:1;
    uint32_t is_open:1;
    uint32_t is_dead:1;
    uint32_t on_active_list:1;
    uint32_t on_server:1;
    uint32_t on_server_player_things:1;
    uint32_t on_client_player_things:1;

    /*
     * Force client to server postion.
     */
    uint32_t resync:1;
} thing;

#include "thing_template.h"

static inline uint8_t thing_is_open (thingp t)
{
    verify(t);

    return (t->is_open);
}

static inline uint8_t thing_is_dead (thingp t)
{
    verify(t);

    return (t->is_dead);
}

static inline uint8_t thing_is_exit (thingp t)
{
    verify(t);

    return (thing_template_is_exit(thing_get_template(t)));
}

static inline uint8_t thing_is_floor (thingp t)
{
    verify(t);

    return (thing_template_is_floor(thing_get_template(t)));
}

static inline uint8_t thing_is_food (thingp t)
{
    verify(t);

    return (thing_template_is_food(thing_get_template(t)));
}

static inline uint8_t thing_is_monst (thingp t)
{
    verify(t);

    return (thing_template_is_monst(thing_get_template(t)));
}

static inline uint8_t thing_is_generator (thingp t)
{
    verify(t);

    return (thing_template_is_generator(thing_get_template(t)));
}

static inline uint8_t thing_is_plant (thingp t)
{
    verify(t);

    return (thing_template_is_plant(thing_get_template(t)));
}

static inline uint8_t thing_is_player (thingp t)
{
    verify(t);

    return (thing_template_is_player(thing_get_template(t)));
}

static inline uint8_t thing_is_key (thingp t)
{
    verify(t);

    return (thing_template_is_key(thing_get_template(t)));
}

static inline uint8_t thing_is_collision_map_player (thingp t)
{
    verify(t);

    return (thing_template_is_collision_map_player(thing_get_template(t)));
}

static inline uint8_t thing_is_collision_map_monst (thingp t)
{
    verify(t);

    return (thing_template_is_collision_map_monst(thing_get_template(t)));
}

static inline uint8_t thing_is_collision_map_weapon (thingp t)
{
    verify(t);

    return (thing_template_is_collision_map_weapon(thing_get_template(t)));
}

static inline uint8_t thing_is_xxx3 (thingp t)
{
    verify(t);

    return (thing_template_is_xxx3(thing_get_template(t)));
}

static inline uint8_t thing_is_xxx4 (thingp t)
{
    verify(t);

    return (thing_template_is_xxx4(thing_get_template(t)));
}

static inline uint8_t thing_is_xxx5 (thingp t)
{
    verify(t);

    return (thing_template_is_xxx5(thing_get_template(t)));
}

static inline uint8_t thing_is_xxx6 (thingp t)
{
    verify(t);

    return (thing_template_is_xxx6(thing_get_template(t)));
}

static inline uint8_t thing_is_xxx7 (thingp t)
{
    verify(t);

    return (thing_template_is_xxx7(thing_get_template(t)));
}

static inline uint8_t thing_is_killed_on_hitting_player (thingp t)
{
    verify(t);

    return (thing_template_is_killed_on_hitting_player(thing_get_template(t)));
}

static inline uint8_t thing_is_star (thingp t)
{
    verify(t);

    return (thing_template_is_star(thing_get_template(t)));
}

static inline uint8_t thing_is_key0 (thingp t)
{
    verify(t);

    return (thing_template_is_key0(thing_get_template(t)));
}

static inline uint8_t thing_is_key1 (thingp t)
{
    verify(t);

    return (thing_template_is_key1(thing_get_template(t)));
}

static inline uint8_t thing_is_key2 (thingp t)
{
    verify(t);

    return (thing_template_is_key2(thing_get_template(t)));
}

static inline uint8_t thing_is_key3 (thingp t)
{
    verify(t);

    return (thing_template_is_key3(thing_get_template(t)));
}

static inline uint8_t thing_is_key4 (thingp t)
{
    verify(t);

    return (thing_template_is_key4(thing_get_template(t)));
}

static inline uint8_t thing_is_key5 (thingp t)
{
    verify(t);

    return (thing_template_is_key5(thing_get_template(t)));
}

static inline uint8_t thing_is_key6 (thingp t)
{
    verify(t);

    return (thing_template_is_key6(thing_get_template(t)));
}

static inline uint8_t thing_is_key7 (thingp t)
{
    verify(t);

    return (thing_template_is_key7(thing_get_template(t)));
}

static inline uint8_t thing_is_key8 (thingp t)
{
    verify(t);

    return (thing_template_is_key8(thing_get_template(t)));
}

static inline uint8_t thing_is_xxx20 (thingp t)
{
    verify(t);

    return (thing_template_is_xxx20(thing_get_template(t)));
}

static inline uint8_t thing_is_xxx21 (thingp t)
{
    verify(t);

    return (thing_template_is_xxx21(thing_get_template(t)));
}

static inline uint8_t thing_is_seedpod (thingp t)
{
    verify(t);

    return (thing_template_is_seedpod(thing_get_template(t)));
}

static inline uint8_t thing_is_bomb (thingp t)
{
    verify(t);

    return (thing_template_is_bomb(thing_get_template(t)));
}

static inline uint8_t thing_is_spam (thingp t)
{
    verify(t);

    return (thing_template_is_spam(thing_get_template(t)));
}

static inline uint8_t thing_is_door (thingp t)
{
    verify(t);

    return (thing_template_is_door(thing_get_template(t)));
}

static inline uint8_t thing_is_pipe (thingp t)
{
    verify(t);

    return (thing_template_is_pipe(thing_get_template(t)));
}

static inline uint8_t thing_is_scarable (thingp t)
{
    verify(t);

    return (thing_template_is_scarable(thing_get_template(t)));
}

static inline uint8_t thing_is_xxx29 (thingp t)
{
    verify(t);

    return (thing_template_is_xxx29(thing_get_template(t)));
}

static inline uint8_t thing_is_hidden_from_editor (thingp t)
{
    verify(t);

    return (thing_template_is_hidden_from_editor(thing_get_template(t)));
}

static inline uint8_t thing_is_animated (thingp t)
{
    verify(t);

    return (thing_template_is_animated(thing_get_template(t)));
}

static inline uint8_t thing_explode_on_death (thingp t)
{
    verify(t);

    return (thing_template_explode_on_death(thing_get_template(t)));
}

static inline uint8_t thing_is_projectile (thingp t)
{
    verify(t);

    return (thing_template_is_projectile(thing_get_template(t)));
}

static inline uint8_t thing_is_joinable (thingp t)
{
    verify(t);

    return (thing_template_is_joinable(thing_get_template(t)));
}

static inline uint8_t thing_is_wall (thingp t)
{
    verify(t);

    return (thing_template_is_wall(thing_get_template(t)));
}

static inline uint8_t thing_is_effect_sway (thingp t)
{
    verify(t);

    return (thing_template_is_effect_sway(thing_get_template(t)));
}

static inline uint8_t thing_is_effect_pulse (thingp t)
{
    verify(t);

    return (thing_template_is_effect_pulse(thing_get_template(t)));
}

static inline uint8_t thing_is_effect_rotate_4way (thingp t)
{
    verify(t);

    return (thing_template_is_effect_rotate_4way(thing_get_template(t)));
}

static inline uint8_t thing_is_effect_rotate_2way (thingp t)
{
    verify(t);

    return (thing_template_is_effect_rotate_2way(thing_get_template(t)));
}

static inline uint8_t thing_is_open_fast (thingp t)
{
    return (t->is_open);
}

static inline uint8_t thing_is_dead_fast (thingp t)
{
    return (t->is_dead);
}

static inline uint8_t thing_is_exit_fast (thingp t)
{
    return (t->thing_template->is_exit);
}

static inline uint8_t thing_is_floor_fast (thingp t)
{
    return (t->thing_template->is_floor);
}

static inline uint8_t thing_is_food_fast (thingp t)
{
    return (t->thing_template->is_food);
}

static inline uint8_t thing_is_monst_fast (thingp t)
{
    return (t->thing_template->is_monst);
}

static inline uint8_t thing_is_plant_fast (thingp t)
{
    return (t->thing_template->is_plant);
}

static inline uint8_t thing_is_player_fast (thingp t)
{
    return (t->thing_template->is_player);
}

static inline uint8_t thing_is_key_fast (thingp t)
{
    return (t->thing_template->is_key);
}

static inline uint8_t thing_is_collision_map_player_fast (thingp t)
{
    return (t->thing_template->is_collision_map_player);
}

static inline uint8_t thing_is_collision_map_monst_fast (thingp t)
{
    return (t->thing_template->is_collision_map_monst);
}

static inline uint8_t thing_is_collision_map_weapon_fast (thingp t)
{
    return (t->thing_template->is_collision_map_weapon);
}

static inline uint8_t thing_is_xxx3_fast (thingp t)
{
    return (t->thing_template->is_xxx3);
}

static inline uint8_t thing_is_xxx4_fast (thingp t)
{
    return (t->thing_template->is_xxx4);
}

static inline uint8_t thing_is_xxx5_fast (thingp t)
{
    return (t->thing_template->is_xxx5);
}

static inline uint8_t thing_is_xxx6_fast (thingp t)
{
    return (t->thing_template->is_xxx6);
}

static inline uint8_t thing_is_xxx7_fast (thingp t)
{
    return (t->thing_template->is_xxx7);
}

static inline uint8_t thing_is_killed_on_hitting_player_fast (thingp t)
{
    return (t->thing_template->is_killed_on_hitting_player);
}

static inline uint8_t thing_is_star_fast (thingp t)
{
    return (t->thing_template->is_star);
}

static inline uint8_t thing_is_key0_fast (thingp t)
{
    return (t->thing_template->is_key0);
}

static inline uint8_t thing_is_key1_fast (thingp t)
{
    return (t->thing_template->is_key1);
}

static inline uint8_t thing_is_key2_fast (thingp t)
{
    return (t->thing_template->is_key2);
}

static inline uint8_t thing_is_key3_fast (thingp t)
{
    return (t->thing_template->is_key3);
}

static inline uint8_t thing_is_key4_fast (thingp t)
{
    return (t->thing_template->is_key4);
}

static inline uint8_t thing_is_key5_fast (thingp t)
{
    return (t->thing_template->is_key5);
}

static inline uint8_t thing_is_key6_fast (thingp t)
{
    return (t->thing_template->is_key6);
}

static inline uint8_t thing_is_key7_fast (thingp t)
{
    return (t->thing_template->is_key7);
}

static inline uint8_t thing_is_key8_fast (thingp t)
{
    return (t->thing_template->is_key8);
}

static inline uint8_t thing_is_xxx20_fast (thingp t)
{
    return (t->thing_template->is_xxx20);
}

static inline uint8_t thing_is_xxx21_fast (thingp t)
{
    return (t->thing_template->is_xxx21);
}

static inline uint8_t thing_is_seedpod_fast (thingp t)
{
    return (t->thing_template->is_seedpod);
}

static inline uint8_t thing_is_bomb_fast (thingp t)
{
    return (t->thing_template->is_bomb);
}

static inline uint8_t thing_is_spam_fast (thingp t)
{
    return (t->thing_template->is_spam);
}

static inline uint8_t thing_is_door_fast (thingp t)
{
    return (t->thing_template->is_door);
}

static inline uint8_t thing_is_pipe_fast (thingp t)
{
    return (t->thing_template->is_pipe);
}

static inline uint8_t thing_is_scarable_fast (thingp t)
{
    return (t->thing_template->is_scarable);
}

static inline uint8_t thing_is_xxx29_fast (thingp t)
{
    return (t->thing_template->is_xxx29);
}

static inline uint8_t thing_is_hidden_from_editor_fast (thingp t)
{
    return (t->thing_template->is_hidden_from_editor);
}

static inline uint8_t thing_is_animated_fast (thingp t)
{
    return (t->thing_template->is_animated);
}

static inline uint8_t thing_explode_on_death_fast (thingp t)
{
    return (t->thing_template->explode_on_death);
}

static inline uint8_t thing_is_projectile_fast (thingp t)
{
    return (t->thing_template->is_projectile);
}

static inline uint8_t thing_is_joinable_fast (thingp t)
{
    return (t->thing_template->is_joinable);
}

static inline uint8_t thing_is_wall_fast (thingp t)
{
    return (t->thing_template->is_wall);
}

static inline uint8_t thing_is_effect_sway_fast (thingp t)
{
    return (t->thing_template->is_effect_sway);
}

static inline uint8_t thing_is_effect_pulse_fast (thingp t)
{
    return (t->thing_template->is_effect_pulse);
}

static inline uint8_t thing_is_effect_rotate_4way_fast (thingp t)
{
    return (t->thing_template->is_effect_rotate_4way);
}

static inline uint8_t thing_is_effect_rotate_2way_fast (thingp t)
{
    return (t->thing_template->is_effect_rotate_2way);
}
