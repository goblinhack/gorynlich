/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file.
 */

#pragma once

#include "tree.h"
#include "wid.h"
#include "level_private.h"
#include "thing_template.h"
#include <stdlib.h>
#include <math.h>

uint8_t thing_init(void);
void thing_fini(void);
char *item2str(const item_t i);
void thing_dump(const thingp t);
void thing_update(thingp t);
void thing_map_sanity(void);
void thing_sanity(thingp);
void thing_map_dump(void);
thingp thing_server_new(const char *name, 
                        double x, double y,
                        thing_statsp stats);
void thing_server_init(thingp, double x, double y);
thingp thing_client_new(uint32_t, tpp);
thingp thing_client_local_new(tpp tp);
void thing_restarted(thingp t, levelp level);
void thing_destroy(thingp, const char *why);
void thing_tick_all(void);
void thing_tick_server_player_slow_all(int force);
uint8_t thing_mob_spawn(thingp);
thingp thing_mob_spawn_on_death(thingp);
void thing_make_active(thingp t);
void thing_make_inactive(thingp t);
void thing_wake(thingp t);
void thing_dead(thingp, thingp killer,
                const char *fmt, ...) __attribute__ ((format (printf, 3, 4)));
void thing_dying(thingp, thingp killer,
                 const char *fmt, ...) __attribute__ ((format (printf, 3, 4)));
int thing_hit(thingp, thingp hitter, uint32_t damage);
void thing_hide(thingp);
thingp thing_owner(thingp t);
uint8_t thing_is_visible(thingp);
void thing_leave_level(thingp);
void thing_join_level(thingp);
void thing_visible(thingp);
void things_level_destroyed(levelp, uint8_t keep_players);
int thing_is_player_or_owned_by_player(thingp t);
void demarshal_thing(demarshal_p ctx, thingp);
void marshal_thing(marshal_p ctx, thingp);
void thing_templates_marshal(marshal_p out);
void things_marshal(marshal_p out);
void thing_set_wid(thingp, widp);
void thing_map_remove(thingp t);
void thing_map_add(thingp t, int32_t x, int32_t y);
int32_t thing_grid_x(thingp);
int32_t thing_grid_y(thingp);
widp thing_wid(thingp);
const char *thing_dead_reason(thingp);
const char *thing_name(thingp);
const char *thing_logname(thingp);
const char *thing_short_name(thingp);
uint8_t thing_z_depth(thingp);
uint8_t thing_z_order(thingp);
tree_rootp thing_tile_tiles(thingp);
tree_rootp thing_tile_tiles2(thingp);
void thing_animate(thingp);
uint8_t thing_hit_solid_obstacle(widp grid, thingp t, double nx, double ny);
uint8_t thing_hit_any_obstacle(widp grid, thingp t, double nx, double ny);
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
void thing_set_is_light_source(thingp t, uint8_t val);
uint8_t thing_is_light_source(thingp t);
void thing_set_is_candle_light(thingp t, uint8_t val);
uint8_t thing_is_candle_light(thingp t);
void thing_set_is_key4(thingp t, uint8_t val);
uint8_t thing_is_key4(thingp t);
void thing_set_is_key5(thingp t, uint8_t val);
uint8_t thing_is_key5(thingp t);
void thing_set_is_sleeping(thingp t, uint8_t val);
void thing_set_is_collected(thingp t, uint8_t val);
uint8_t thing_is_collected(thingp t);
void thing_set_is_chasing_target(thingp t, uint8_t val);
uint8_t thing_is_chasing_target(thingp t);
void thing_set_got_to_exit_first(thingp t, uint8_t val);
uint8_t thing_got_to_exit_first(thingp t);
void thing_set_redo_maze_search(thingp t, uint8_t val);
uint8_t thing_redo_maze_search(thingp t);

void thing_set_dir_down(thingp t);
uint8_t thing_is_dir_down(thingp t);
void thing_set_dir_up(thingp t);
uint8_t thing_is_dir_up(thingp t);
void thing_set_dir_left(thingp t);
uint8_t thing_is_dir_left(thingp t);
void thing_set_dir_right(thingp t);
uint8_t thing_is_dir_right(thingp t);

void thing_set_dir_tl(thingp t);
uint8_t thing_is_dir_tl(thingp t);
void thing_set_dir_bl(thingp t);
uint8_t thing_is_dir_bl(thingp t);
void thing_set_dir_tr(thingp t);
uint8_t thing_is_dir_tr(thingp t);
void thing_set_dir_br(thingp t);
uint8_t thing_is_dir_br(thingp t);

void thing_set_opened_exit(thingp t, uint8_t val);
uint8_t thing_opened_exit(thingp t);
void thing_set_is_open(thingp t, uint8_t val);
void thing_set_is_projectile(thingp t, uint8_t val);
void thing_set_is_dead(thingp t, uint8_t val);
void thing_set_key20(thingp t, uint8_t val);
uint8_t thing_key20(thingp t);
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
void thing_reached_exit(thingp t, thingp exit);
void thing_place_timed(tpp t, 
                       double x,
                       double y,
                       uint32_t ms, 
                       uint32_t jitter,
                       uint8_t server_side);
void thing_place_and_destroy_timed(levelp,
                                   tpp t, 
                                   thingp owner,
                                   double x,
                                   double y,
                                   uint32_t ms, 
                                   uint32_t destroy_in, 
                                   uint32_t jitter,
                                   uint8_t server_side,
                                   uint8_t is_epicenter);
void thing_timer_destroy(thingp t, uint32_t destroy_in);
void thing_timer_place_and_destroy_callback(void *context);
void thing_timer_place_and_destroy_destroy_callback(void *context);
void thing_timer_place_callback(void *context);
void thing_timer_place_destroy_callback(void *context);
void thing_server_wid_update(thingp t, double x, double y, uint8_t is_new);
void thing_client_wid_update(thingp t, double x, double y, 
                             uint8_t smooth, uint8_t is_new);
void msg_server_shout_at_player(thingp t, const char *what);
uint8_t thing_use(thingp t, uint32_t id);

itemp thing_is_carrying_specific_item(thingp, uint32_t id);
typedef uint8_t (*tp_is)(tpp);
tpp thing_is_carrying_thing(thingp, tp_is);
uint32_t thing_is_carrying_thing_count(thingp, tp_is);

/*
 * thing_ai.c
 */
uint8_t thing_find_nexthop(thingp t, int32_t *x, int32_t *y);
void dmap_generate_map_wander(levelp level);
void thing_generate_dmaps(void);
void dmap_process_init(void);
void dmap_process_fini(void);
void dmap_generate_map(double x, double y);

typedef struct {
    tpp tp;
    levelp level;
    double x;
    double y;
    uint16_t destroy_in;

    /*
     * Server or client local thing?
     */
    uint8_t on_server:1;

    /*
     * Center of an explosion.
     */
    uint8_t is_epicenter:1;

    uint32_t owner_id;
    uint32_t thing_id;
} thing_place_context_t;

void thing_teleport(thingp t, int32_t x, int32_t y);
thingp thing_client_find(uint32_t thing_id);
thingp thing_server_find(uint32_t thing_id);

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
    /********************************************************************
     * Update msg_map_update if you add here
     ********************************************************************/
    THING_STATE_BIT_SHIFT_DIR_BIT0,
    THING_STATE_BIT_SHIFT_DIR_BIT1,
    THING_STATE_BIT_SHIFT_DIR_BIT2,
    THING_STATE_BIT_SHIFT_XY_PRESENT,
    THING_STATE_BIT_SHIFT_ID_DELTA_PRESENT,
    THING_STATE_BIT_SHIFT_ID_TEMPLATE_PRESENT,
    THING_STATE_BIT_SHIFT_EXT1_PRESENT,
    THING_STATE_BIT_SHIFT_EXT2_PRESENT,
    /********************************************************************
     * Update msg_map_update if you add here
     ********************************************************************/
};

enum {
    /********************************************************************
     * Update msg_map_update if you add here
     ********************************************************************/
    THING_STATE_BIT_SHIFT_EXT1_IS_DEAD,
    THING_STATE_BIT_SHIFT_EXT1_IS_ACTIVE,
    THING_STATE_BIT_SHIFT_EXT1_IS_SLEEPING,
    THING_STATE_BIT_SHIFT_EXT1_HAS_LEFT_LEVEL,
    THING_STATE_BIT_SHIFT_EXT1_EFFECT_PRESENT,
    THING_STATE_BIT_SHIFT_EXT1_WEAPON_ID_PRESENT,
    THING_STATE_BIT_SHIFT_EXT1_SHIELD_ID_PRESENT,
    THING_STATE_BIT_SHIFT_EXT1_WEAPON_SWUNG,
    /********************************************************************
     * Update msg_map_update if you add here
     ********************************************************************/
};

enum {
    /********************************************************************
     * Update msg_map_update if you add here
     ********************************************************************/
    THING_STATE_BIT_SHIFT_EXT2_RESYNC,
    THING_STATE_BIT_SHIFT_EXT2_TORCH_LIGHT_RADIUS,
    THING_STATE_BIT_SHIFT_EXT2_IS_JUMPING,
    THING_STATE_BIT_SHIFT_EXT2_COLOR,
    /********************************************************************
     * Update msg_map_update if you add here
     ********************************************************************/
};

enum {
    /*
     * Sorted in order of importance as only one effect can be sent to the 
     * client at a time.
     */
    THING_STATE_EFFECT_NONE,
    THING_STATE_EFFECT_IS_HIT_MISS,
    THING_STATE_EFFECT_IS_HIT_SUCCESS,
    THING_STATE_EFFECT_IS_POWER_UP,
    THING_STATE_EFFECT_IS_SHIELD,
    THING_STATE_EFFECT_IS_HIT_CRIT,
};

void thing_client_move(thingp t,
                       double x,
                       double y,
                       const uint8_t up,
                       const uint8_t down,
                       const uint8_t left,
                       const uint8_t right,
                       const uint8_t fire);

void thing_set_owner_id(thingp t, uint32_t owner_id);
void thing_set_owner(thingp t, thingp owner);

enum {
    THING_NONE,
    THING_WALL1,
    THING_WALL_LIT1,
    THING_ROCK1,
    THING_WALL2,
    THING_WALL3,
    THING_WALL4,
    THING_WALL5,
    THING_WALL6,
    THING_DOOR1,
    THING_DOOR2,
    THING_SAWBLADE1,
    THING_SAWBLADE2,
    THING_SAWBLADE3,
    THING_SAWBLADE4,
    THING_SAWBLADE5,
    THING_NOENTRY,
    THING_APPLE1,
    THING_APPLE2,
    THING_BANANA,
    THING_CHEESE,
    THING_BREAD,
    THING_FLOOR1,
    THING_FLOOR2,
    THING_FLOOR3,
    THING_FLOOR4,
    THING_FLOOR5,
    THING_FLOOR6,
    THING_WARRIOR,
    THING_VALKYRIE,
    THING_WIZARD,
    THING_ELF,
    THING_DWARF,
    THING_GHOST1,
    THING_GHOST2,
    THING_GHOST3,
    THING_SLIME1,
    THING_SLIME2,
    THING_SLIME3,
    THING_DEMON1,
    THING_BABY_CTHULU,
    THING_EYEBAWL,
    THING_DEMON,
    THING_MONK,
    THING_LIZARD,
    THING_DEATH,
    THING_WAND1_ANIM,
    THING_SWORD1_ANIM,
    THING_SWORD2_ANIM,
    THING_SWORD3_ANIM,
    THING_BOW1_ANIM,
    THING_AXE1_ANIM,
    THING_AXE2_ANIM,
    THING_SCYTHE1_ANIM,
    THING_ANIM_MAN,
    THING_ANIM_TREASURE_CHEST,
    THING_ANIM_EYES,
    THING_PIPE,
    THING_WEAPON_HIT1,
    THING_EXPLOSION1,
    THING_EXPLOSION2,
    THING_EXPLOSION3,
    THING_EXPLOSION4,
    THING_SHIELD1,
    THING_POWERUP1,
    THING_HIT_SUCCESS,
    THING_HIT_MISS,
    THING_HEART,
    THING_BLOOD1,
    THING_BLOOD2,
    THING_ACTION_LEFT,
    THING_ACTION_RIGHT,
    THING_ACTION_UP,
    THING_ACTION_DOWN,
    THING_ACTION_TRIGGER_HERO,
    THING_ACTION_TRIGGER_WALL,
    THING_ACTION_TRIGGER_MONST,
    THING_ACTION_SLEEP,
    THING_ACTION_TEXT,
    THING_ACTION_ZAP,
    THING_POISON1,
    THING_POISON2,
    THING_CLOUDKILL1,
    THING_SPAM,
    THING_POTION_MONSTICIDE,
    THING_POTION_FIRE,
    THING_POTION_CLOUDKILL,
    THING_POTION_LIFE,
    THING_POTION_SHIELD,
    THING_WATER,
    THING_WATER_POISON,
    THING_MASK1,
    THING_RING2,
    THING_RING3,
    THING_GEM1,
    THING_GEM2,
    THING_GEM3,
    THING_GEM4,
    THING_GEM5,
    THING_GEM6,
    THING_GEM7,
    THING_BRAZIER,
    THING_EXIT1,
    THING_EXIT2,
    THING_EXIT3,
    THING_EXIT4,
    THING_EXIT5,
    THING_EXIT6,
    THING_EXIT7,
    THING_GENERATOR1,
    THING_BONEPILE1,
    THING_BONEPILE2,
    THING_BONEPILE3,
    THING_GENERATOR5,
    THING_GENERATOR6,
    THING_GENERATOR7,
    THING_BOMB,
    THING_BOW1,
    THING_BOW2,
    THING_BOW3,
    THING_BOW4,
    THING_WAND_FIRE,
    THING_SWORD1,
    THING_SWORD2,
    THING_SWORD3,
    THING_SWORD4,
    THING_SWORD5,
    THING_SWORD6,
    THING_AXE1,
    THING_AXE2,
    THING_AXE3,
    THING_AXE4,
    THING_AXE5,
    THING_SCYTHE1,
    THING_KEY,
    THING_KEYS2,
    THING_KEYS3,
    THING_TORCH,
    THING_TORCHES,
    THING_COINS1,
    THING_CHEST1,
    THING_ARROW,
    THING_FIREBALL1,
    THING_FIREBALL2,
    THING_PLASMABALL1,
};

typedef struct thing_ {
    tree_key_int tree;

    /*
     * For player things.
     */
    tree_key_int tree2;

    /*
     * Socket information for the player.
     */
    aplayerp player;

    /*
     * Max health, attack bpnuses etc...
     */
    thing_stats stats;

    /*
     * These values are not sent between server and client. Instead we send 
     * the weapon id alone and create local things for weapon swings.
     */
    uint16_t weapon_carry_anim_thing_id;
    uint16_t weapon_swing_anim_thing_id;
    uint16_t shield_anim_thing_id;

    /*
     * Weapon thing template.
     */
    tpp weapon;

    /*
     * Current shield
     */
    tpp shield;

    /*
     * Pointer to common settings for this thing.
     */
    tpp tp;

    /*
     * Item attrivutes that override template settings, like enchanted.
     */
    item_t item;

    /*
     * Name for debugging.
     */
    char *logname;

    /*
     * Unique id per thing.
     */
    uint16_t thing_id;

    /*
     * Who created this thing? e.g. who cast a spell?
     */
    uint16_t owner_thing_id;

    /*
     * How much damage per hit?
     */
    uint16_t damage;

    /*
     * Which djkstra map this thing is using.
     */
    level_walls *dmap;
    level_walls *dmap_wander;

    /*
     * Which tree are we on, server active? client boring? etc...
     */
    tree_rootp client_or_server_tree;

    /*
     * Widget for displaying thing.
     */
    widp wid;

    /*
     * Last death reason.
     */
    char *dead_reason;

    /*
     * Thing dead in x ms. The tree can be the global tree or an owner tree.
     */
    timerp timer_dead;
    tree_rootp timer_dead_tree;
    
    /*
     * If this thing is involved in any timed activities like an explosion.
     */
    tree_rootp timers;

    /*
     * Grid coordinates.
     */
    double x;
    double y;

    /*
     * Last anim frame position. To be able to detect moving things.
     */
    double anim_x;
    double anim_y;

    /*
     * Map grid co-ordinates.
     */
    int16_t map_x;
    int16_t map_y;

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
     * Data we read in along with the thing template.
     */
    thing_template_data *data;

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
     * When we last checked for hitting something.
     */
    uint32_t timestamp_collision;

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
     * When did we last send our state to the client?
     */
    uint32_t timestamp_tx_map_update;

    /*
     * Last time it fired at a player.
     */
    uint32_t timestamp_fired;

    /*
     * A counter to indicate the number of times we want to update the client 
     * with the state of this thing. To compensate for udp drops we might want 
     * to send a couple of updates for important events like death.
     */
    uint8_t updated;

    /*
     * The type of effect we plan to send to the client.
     */
    uint8_t effect;

    uint8_t dir:3;

    /*
     * How many rays of light are hitting this thing?
     */
    uint16_t lit;

    /*
     * How much light is this thing emitting. This cannot exceed the light 
     * strength of the template. This is used for torches and the like to see 
     * how much we can really see.
     */
    float torch_light_radius;
    uint32_t timestamp_torch;

    /*
     * For auto decrementing health if over max due to magical boost.
     */
    uint32_t timestamp_health;

    /*
     * Debugging this thing?
     */
    uint32_t debug:1;

    /*
     * First time sent to a client?
     */
    uint32_t first_update:1;

    uint32_t is_light_source:1;
    uint32_t is_candle_light:1;
    uint32_t is_sleeping:1;
    uint32_t is_collected:1;
    uint32_t got_to_exit_first:1;
    uint32_t opened_exit:1;
    uint32_t is_open:1;
    uint32_t is_dead:1;

    /*
     * Center of an explosion.
     */
    uint32_t is_epicenter:1;

    uint32_t has_left_level:1;
    uint32_t is_moving:1;
    uint32_t on_active_list:1;
    uint32_t on_server:1;
    uint32_t on_server_player_things:1;
    uint32_t on_client_player_things:1;

    /*
     * Swung a weapon
     */
    uint32_t needs_tx_weapon_swung:1;

    /*
     * Force client to server postion.
     */
    uint32_t needs_tx_refresh_xy_and_template_id:1;

    /*
     * Limit certain message types.
     */
    uint32_t message_open_door_need_key:1;

    /*
     * Do we have a light source?
     */
    uint32_t torch_light_radius_set:1;

    /*
     * Moves only occasionally?
     */
    uint32_t one_shot_move:1;

    /*
     * In the middle of a jump animation?
     */
    uint32_t is_jumping:1;
    uint32_t want_to_jump:1;
} thing;


#include "thing_template.h"
static inline tpp thing_tp (thingp t)
{
    verify(t);

    return (t->tp);
}

static inline uint8_t thing_is_open (thingp t)
{
    verify(t);

    return (t->is_open);
}

static inline uint8_t thing_is_dying (thingp t)
{
    return (tp_is_player(thing_tp(t)) && 
            (thing_stats_get_hp(t) < 0));
}

static inline uint8_t thing_is_dead_or_dying (thingp t)
{
    verify(t);

    return (t->is_dead || thing_is_dying(t));
}

static inline uint8_t thing_is_dead (thingp t)
{
    verify(t);

    return (t->is_dead);
}

static inline uint8_t thing_is_active (thingp t)
{
    verify(t);

    return (t->on_active_list);
}

static inline uint8_t thing_is_epicenter (thingp t)
{
    verify(t);

    return (t->is_epicenter);
}

static inline uint8_t thing_has_left_level (thingp t)
{
    verify(t);

    return (t->has_left_level);
}

static inline uint8_t thing_is_exit (thingp t)
{
    verify(t);

    return (tp_is_exit(thing_tp(t)));
}

static inline uint8_t thing_is_floor (thingp t)
{
    verify(t);

    return (tp_is_floor(thing_tp(t)));
}

static inline uint8_t thing_is_food (thingp t)
{
    verify(t);

    return (tp_is_food(thing_tp(t)));
}

static inline uint8_t thing_is_monst (thingp t)
{
    verify(t);

    return (tp_is_monst(thing_tp(t)));
}

static inline uint8_t thing_is_mob_spawner (thingp t)
{
    verify(t);

    return (tp_is_mob_spawner(thing_tp(t)));
}

static inline uint8_t thing_is_plant (thingp t)
{
    verify(t);

    return (tp_is_plant(thing_tp(t)));
}

static inline uint8_t thing_is_player (thingp t)
{
    verify(t);

    return (tp_is_player(thing_tp(t)));
}

static inline uint8_t thing_is_key (thingp t)
{
    verify(t);

    return (tp_is_key(thing_tp(t)));
}

static inline uint8_t thing_is_collision_map_large (thingp t)
{
    verify(t);

    return (tp_is_collision_map_large(thing_tp(t)));
}

static inline uint8_t thing_is_collision_map_medium (thingp t)
{
    verify(t);

    return (tp_is_collision_map_medium(thing_tp(t)));
}

static inline uint8_t thing_is_collision_map_small (thingp t)
{
    verify(t);

    return (tp_is_collision_map_small(thing_tp(t)));
}

static inline uint8_t thing_is_collision_map_tiny (thingp t)
{
    verify(t);

    return (tp_is_collision_map_tiny(thing_tp(t)));
}

static inline uint8_t thing_is_rock (thingp t)
{
    verify(t);

    return (tp_is_rock(thing_tp(t)));
}

static inline uint8_t thing_is_shadow_caster (thingp t)
{
    verify(t);

    return (tp_is_shadow_caster(thing_tp(t)));
}

static inline uint8_t thing_is_shadow_caster_soft (thingp t)
{
    verify(t);

    return (tp_is_shadow_caster_soft(thing_tp(t)));
}

static inline uint8_t thing_is_weapon (thingp t)
{
    verify(t);

    return (tp_is_weapon(thing_tp(t)));
}

static inline uint8_t thing_is_carryable (thingp t)
{
    verify(t);

    return (tp_is_carryable(thing_tp(t)));
}

static inline uint8_t thing_is_treasure (thingp t)
{
    verify(t);

    return (tp_is_treasure(thing_tp(t)));
}

static inline uint8_t thing_is_wearable (thingp t)
{
    verify(t);

    return (tp_is_wearable(thing_tp(t)));
}

static inline uint8_t thing_is_fragile (thingp t)
{
    verify(t);

    return (tp_is_fragile(thing_tp(t)));
}

static inline uint8_t thing_is_star (thingp t)
{
    verify(t);

    return (tp_is_star(thing_tp(t)));
}

static inline uint8_t thing_is_animated_no_dir (thingp t)
{
    verify(t);

    return (tp_is_animated_no_dir(thing_tp(t)));
}

static inline uint8_t thing_is_weapon_swing_effect (thingp t)
{
    verify(t);

    return (tp_is_weapon_swing_effect(thing_tp(t)));
}

static inline uint8_t thing_is_fire (thingp t)
{
    verify(t);

    return (tp_is_fire(thing_tp(t)));
}

static inline uint8_t thing_is_cats_eyes (thingp t)
{
    verify(t);

    return (tp_is_cats_eyes(thing_tp(t)));
}

static inline uint8_t thing_is_ring (thingp t)
{
    verify(t);

    return (tp_is_ring(thing_tp(t)));
}

static inline uint8_t thing_is_animation (thingp t)
{
    verify(t);

    return (tp_is_animation(thing_tp(t)));
}

static inline uint8_t thing_is_non_explosive_gas_cloud (thingp t)
{
    verify(t);

    return (tp_is_non_explosive_gas_cloud(thing_tp(t)));
}

static inline uint8_t thing_is_item_unusable (thingp t)
{
    verify(t);

    return (tp_is_item_unusable(thing_tp(t)));
}

static inline uint8_t thing_is_valid_for_action_bar (thingp t)
{
    verify(t);

    return (tp_is_valid_for_action_bar(thing_tp(t)));
}

static inline uint8_t thing_is_seedpod (thingp t)
{
    verify(t);

    return (tp_is_seedpod(thing_tp(t)));
}

static inline uint8_t thing_is_spam (thingp t)
{
    verify(t);

    return (tp_is_spam(thing_tp(t)));
}

static inline uint8_t thing_is_door (thingp t)
{
    verify(t);

    return (tp_is_door(thing_tp(t)));
}

static inline uint8_t thing_is_pipe (thingp t)
{
    verify(t);

    return (tp_is_pipe(thing_tp(t)));
}

static inline uint8_t thing_is_rrr1 (thingp t)
{
    verify(t);

    return (tp_is_rrr1(thing_tp(t)));
}

static inline uint8_t thing_is_rrr2 (thingp t)
{
    verify(t);

    return (tp_is_rrr2(thing_tp(t)));
}

static inline uint8_t thing_is_rrr3 (thingp t)
{
    verify(t);

    return (tp_is_rrr3(thing_tp(t)));
}

static inline uint8_t thing_is_rrr4 (thingp t)
{
    verify(t);

    return (tp_is_rrr4(thing_tp(t)));
}

static inline uint8_t thing_is_rrr5 (thingp t)
{
    verify(t);

    return (tp_is_rrr5(thing_tp(t)));
}

static inline uint8_t thing_is_rrr6 (thingp t)
{
    verify(t);

    return (tp_is_rrr6(thing_tp(t)));
}

static inline uint8_t thing_is_rrr7 (thingp t)
{
    verify(t);

    return (tp_is_rrr7(thing_tp(t)));
}

static inline uint8_t thing_is_rrr8 (thingp t)
{
    verify(t);

    return (tp_is_rrr8(thing_tp(t)));
}

static inline uint8_t thing_is_ethereal (thingp t)
{
    verify(t);

    return (tp_is_ethereal(thing_tp(t)));
}

static inline uint8_t thing_is_variable_size (thingp t)
{
    verify(t);

    return (tp_is_variable_size(thing_tp(t)));
}

static inline uint8_t thing_is_magical_weapon (thingp t)
{
    verify(t);

    return (tp_is_magical_weapon(thing_tp(t)));
}

static inline uint8_t thing_is_ranged_weapon (thingp t)
{
    verify(t);

    return (tp_is_ranged_weapon(thing_tp(t)));
}

static inline uint8_t thing_is_melee_weapon (thingp t)
{
    verify(t);

    return (tp_is_melee_weapon(thing_tp(t)));
}

static inline uint8_t thing_is_poison (thingp t)
{
    verify(t);

    return (tp_is_poison(thing_tp(t)));
}

static inline uint8_t thing_is_cloud_effect (thingp t)
{
    verify(t);

    return (tp_is_cloud_effect(thing_tp(t)));
}

static inline uint8_t thing_is_powerup (thingp t)
{
    verify(t);

    return (tp_is_powerup(thing_tp(t)));
}

static inline uint8_t thing_is_damaged_on_firing (thingp t)
{
    verify(t);

    return (tp_is_damaged_on_firing(thing_tp(t)));
}

static inline uint8_t thing_is_hard (thingp t)
{
    verify(t);

    return (tp_is_hard(thing_tp(t)));
}

static inline uint8_t thing_is_sleeping (thingp t)
{
    verify(t);

    return (t->is_sleeping);
}

static inline uint8_t thing_is_jumping (thingp t)
{
    verify(t);

    return (t->is_jumping);
}

static inline uint8_t thing_is_awake (thingp t)
{
    verify(t);

    return (!t->is_sleeping);
}

static inline uint8_t thing_is_bomb (thingp t)
{
    verify(t);

    return (tp_is_bomb(thing_tp(t)));
}

static inline uint8_t thing_is_sawblade (thingp t)
{
    verify(t);

    return (tp_is_sawblade(thing_tp(t)));
}

static inline uint8_t thing_is_action_text (thingp t)
{
    verify(t);

    return (tp_is_action_text(thing_tp(t)));
}

static inline uint8_t thing_is_visible_on_debug_only (thingp t)
{
    verify(t);

    return (tp_is_visible_on_debug_only(thing_tp(t)));
}

static inline uint8_t thing_is_action (thingp t)
{
    verify(t);

    return (tp_is_action(thing_tp(t)));
}

static inline uint8_t thing_is_action_sleep (thingp t)
{
    verify(t);

    return (tp_is_action_sleep(thing_tp(t)));
}

static inline uint8_t thing_is_action_zap (thingp t)
{
    verify(t);

    return (tp_is_action_zap(thing_tp(t)));
}

static inline uint8_t thing_is_action_trigger (thingp t)
{
    verify(t);

    return (tp_is_action_trigger(thing_tp(t)));
}

static inline uint8_t thing_is_action_trigger_on_wall (thingp t)
{
    verify(t);

    return (tp_is_action_trigger_on_wall(thing_tp(t)));
}

static inline uint8_t thing_is_action_trigger_on_hero (thingp t)
{
    verify(t);

    return (tp_is_action_trigger_on_hero(thing_tp(t)));
}

static inline uint8_t thing_is_action_trigger_on_monst (thingp t)
{
    verify(t);

    return (tp_is_action_trigger_on_monst(thing_tp(t)));
}

static inline uint8_t thing_is_action_down (thingp t)
{
    verify(t);

    return (tp_is_action_down(thing_tp(t)));
}

static inline uint8_t thing_is_action_up (thingp t)
{
    verify(t);

    return (tp_is_action_up(thing_tp(t)));
}

static inline uint8_t thing_is_action_left (thingp t)
{
    verify(t);

    return (tp_is_action_left(thing_tp(t)));
}

static inline uint8_t thing_is_action_right (thingp t)
{
    verify(t);

    return (tp_is_action_right(thing_tp(t)));
}

static inline uint8_t thing_can_walk_through (thingp t)
{
    verify(t);

    return (tp_can_walk_through(thing_tp(t)));
}

static inline uint8_t thing_is_weapon_carry_anim (thingp t)
{
    verify(t);

    return (tp_is_weapon_carry_anim(thing_tp(t)));
}

static inline uint8_t thing_is_powerup_anim (thingp t)
{
    verify(t);

    return (tp_is_powerup_anim(thing_tp(t)));
}

static inline uint8_t thing_is_spell (thingp t)
{
    verify(t);

    return (tp_is_spell(thing_tp(t)));
}

static inline uint8_t thing_is_hand_item (thingp t)
{
    verify(t);

    return (tp_is_hand_item(thing_tp(t)));
}

static inline uint8_t thing_is_boots (thingp t)
{
    verify(t);

    return (tp_is_boots(thing_tp(t)));
}

static inline uint8_t thing_is_helmet (thingp t)
{
    verify(t);

    return (tp_is_helmet(thing_tp(t)));
}

static inline uint8_t thing_is_armor (thingp t)
{
    verify(t);

    return (tp_is_armor(thing_tp(t)));
}

static inline uint8_t thing_is_given_randomly_at_start (thingp t)
{
    verify(t);

    return (tp_is_given_randomly_at_start(thing_tp(t)));
}

static inline uint8_t thing_is_magical (thingp t)
{
    verify(t);

    return (tp_is_magical(thing_tp(t)));
}

static inline uint8_t thing_is_degradable (thingp t)
{
    verify(t);

    return (tp_is_degradable(thing_tp(t)));
}

static inline uint8_t thing_is_cursed (thingp t)
{
    verify(t);

    return (tp_is_cursed(thing_tp(t)));
}

static inline uint8_t thing_is_animate_only_when_moving (thingp t)
{
    verify(t);

    return (tp_is_animate_only_when_moving(thing_tp(t)));
}

static inline uint8_t thing_is_warm_blooded (thingp t)
{
    verify(t);

    return (tp_is_warm_blooded(thing_tp(t)));
}

static inline uint8_t thing_can_be_enchanted (thingp t)
{
    verify(t);

    return (tp_can_be_enchanted(thing_tp(t)));
}

static inline uint8_t thing_is_stackable (thingp t)
{
    verify(t);

    return (tp_is_stackable(thing_tp(t)));
}

static inline uint8_t thing_is_torch (thingp t)
{
    verify(t);

    return (tp_is_torch(thing_tp(t)));
}

static inline uint8_t thing_is_explosion (thingp t)
{
    verify(t);

    return (tp_is_explosion(thing_tp(t)));
}

static inline uint8_t thing_is_hidden_from_editor (thingp t)
{
    verify(t);

    return (tp_is_hidden_from_editor(thing_tp(t)));
}

static inline uint8_t thing_is_animated (thingp t)
{
    verify(t);

    return (tp_is_animated(thing_tp(t)));
}

static inline uint8_t thing_is_combustable (thingp t)
{
    verify(t);

    return (tp_is_combustable(thing_tp(t)));
}

static inline uint8_t thing_is_projectile (thingp t)
{
    verify(t);

    return (tp_is_projectile(thing_tp(t)));
}

static inline uint8_t thing_is_joinable (thingp t)
{
    verify(t);

    return (tp_is_joinable(thing_tp(t)));
}

static inline uint8_t thing_is_wall (thingp t)
{
    verify(t);

    return (tp_is_wall(thing_tp(t)));
}

static inline uint8_t thing_is_effect_sway (thingp t)
{
    verify(t);

    return (tp_is_effect_sway(thing_tp(t)));
}

static inline uint8_t thing_is_effect_pulse (thingp t)
{
    verify(t);

    return (tp_is_effect_pulse(thing_tp(t)));
}

static inline uint8_t thing_is_effect_rotate_4way (thingp t)
{
    verify(t);

    return (tp_is_effect_rotate_4way(thing_tp(t)));
}

static inline uint8_t thing_is_effect_rotate_2way (thingp t)
{
    verify(t);

    return (tp_is_effect_rotate_2way(thing_tp(t)));
}

static inline uint8_t thing_is_open_noverify (thingp t)
{
    return (t->is_open);
}

static inline uint8_t thing_is_dead_noverify (thingp t)
{
    return (t->is_dead);
}

static inline uint8_t thing_is_dead_or_dying_noverify (thingp t)
{
    return t->is_dead || thing_is_dying(t);
}

static inline uint8_t thing_is_exit_noverify (thingp t)
{
    return (t->tp->is_exit);
}

static inline uint8_t thing_is_floor_noverify (thingp t)
{
    return (t->tp->is_floor);
}

static inline uint8_t thing_is_food_noverify (thingp t)
{
    return (t->tp->is_food);
}

static inline uint8_t thing_is_monst_noverify (thingp t)
{
    return (t->tp->is_monst);
}

static inline uint8_t thing_is_plant_noverify (thingp t)
{
    return (t->tp->is_plant);
}

static inline uint8_t thing_is_player_noverify (thingp t)
{
    return (t->tp->is_player);
}

static inline uint8_t thing_is_key_noverify (thingp t)
{
    return (t->tp->is_key);
}

static inline uint8_t thing_is_collision_map_medium_noverify (thingp t)
{
    return (t->tp->is_collision_map_medium);
}

static inline uint8_t thing_is_collision_map_small_noverify (thingp t)
{
    return (t->tp->is_collision_map_small);
}

static inline uint8_t thing_is_collision_map_tiny_noverify (thingp t)
{
    return (t->tp->is_collision_map_tiny);
}

static inline uint8_t thing_is_rock_noverify (thingp t)
{
    return (t->tp->is_rock);
}

static inline uint8_t thing_is_shadow_caster_noverify (thingp t)
{
    return (t->tp->is_shadow_caster);
}

static inline uint8_t thing_is_shadow_caster_soft_noverify (thingp t)
{
    return (t->tp->is_shadow_caster_soft);
}

static inline uint8_t thing_is_weapon_noverify (thingp t)
{
    return (t->tp->is_weapon);
}

static inline uint8_t thing_is_treasure_noverify (thingp t)
{
    return (t->tp->is_treasure);
}

static inline uint8_t thing_is_wearable_noverify (thingp t)
{
    return (t->tp->is_wearable);
}

static inline uint8_t thing_is_fragile_noverify (thingp t)
{
    return (t->tp->is_fragile);
}

static inline uint8_t thing_is_star_noverify (thingp t)
{
    return (t->tp->is_star);
}

static inline uint8_t thing_is_animated_no_dir_noverify (thingp t)
{
    return (t->tp->is_animated_no_dir);
}

static inline uint8_t thing_is_weapon_swing_effect_noverify (thingp t)
{
    return (t->tp->is_weapon_swing_effect);
}

static inline uint8_t thing_is_light_source_noverify (thingp t)
{
    return (t->tp->is_light_source);
}

static inline uint8_t thing_is_inactive_noverify (thingp t)
{
    return (t->tp->is_inactive);
}

static inline uint8_t thing_is_candle_light_noverify (thingp t)
{
    return (t->tp->is_candle_light);
}

static inline uint8_t thing_is_cats_eyes_noverify (thingp t)
{
    return (t->tp->is_cats_eyes);
}

static inline uint8_t thing_is_fire_noverify (thingp t)
{
    return (t->tp->is_fire);
}

static inline uint8_t thing_is_ring_noverify (thingp t)
{
    return (t->tp->is_ring);
}

static inline uint8_t thing_is_animation_noverify (thingp t)
{
    return (t->tp->is_animation);
}

static inline uint8_t thing_is_non_explosive_gas_cloud_noverify (thingp t)
{
    return (t->tp->is_non_explosive_gas_cloud);
}

static inline uint8_t thing_is_item_unusable_noverify (thingp t)
{
    return (t->tp->is_item_unusable);
}

static inline uint8_t thing_is_valid_for_action_bar_noverify (thingp t)
{
    return (t->tp->is_valid_for_action_bar);
}

static inline uint8_t thing_is_seedpod_noverify (thingp t)
{
    return (t->tp->is_seedpod);
}

static inline uint8_t thing_is_spam_noverify (thingp t)
{
    return (t->tp->is_spam);
}

static inline uint8_t thing_is_door_noverify (thingp t)
{
    return (t->tp->is_door);
}

static inline uint8_t thing_is_pipe_noverify (thingp t)
{
    return (t->tp->is_pipe);
}

static inline uint8_t thing_is_rrr1_noverify (thingp t)
{
    return (t->tp->is_rrr1);
}

static inline uint8_t thing_is_rrr2_noverify (thingp t)
{
    return (t->tp->is_rrr2);
}

static inline uint8_t thing_is_rrr3_noverify (thingp t)
{
    return (t->tp->is_rrr3);
}

static inline uint8_t thing_is_rrr4_noverify (thingp t)
{
    return (t->tp->is_rrr4);
}

static inline uint8_t thing_is_rrr5_noverify (thingp t)
{
    return (t->tp->is_rrr5);
}

static inline uint8_t thing_is_rrr6_noverify (thingp t)
{
    return (t->tp->is_rrr6);
}

static inline uint8_t thing_is_rrr7_noverify (thingp t)
{
    return (t->tp->is_rrr7);
}

static inline uint8_t thing_is_rrr8_noverify (thingp t)
{
    return (t->tp->is_rrr8);
}

static inline uint8_t thing_is_ethereal_noverify (thingp t)
{
    return (t->tp->is_ethereal);
}

static inline uint8_t thing_is_variable_size_noverify (thingp t)
{
    return (t->tp->is_variable_size);
}

static inline uint8_t thing_is_magical_weapon_noverify (thingp t)
{
    return (t->tp->is_magical_weapon);
}

static inline uint8_t thing_is_ranged_weapon_noverify (thingp t)
{
    return (t->tp->is_ranged_weapon);
}

static inline uint8_t thing_is_melee_weapon_noverify (thingp t)
{
    return (t->tp->is_melee_weapon);
}

static inline uint8_t thing_is_poison_noverify (thingp t)
{
    return (t->tp->is_poison);
}

static inline uint8_t thing_is_cloud_effect_noverify (thingp t)
{
    return (t->tp->is_cloud_effect);
}

static inline uint8_t thing_is_powerup_noverify (thingp t)
{
    return (t->tp->is_powerup);
}

static inline uint8_t thing_is_damaged_on_firing_noverify (thingp t)
{
    return (t->tp->is_damaged_on_firing);
}

static inline uint8_t thing_is_hard_noverify (thingp t)
{
    return (t->tp->is_hard);
}

static inline uint8_t thing_is_sleeping_noverify (thingp t)
{
    return (t->tp->is_sleeping);
}

static inline uint8_t thing_is_bomb_noverify (thingp t)
{
    return (t->tp->is_bomb);
}

static inline uint8_t thing_is_sawblade_noverify (thingp t)
{
    return (t->tp->is_sawblade);
}

static inline uint8_t thing_is_action_text_noverify (thingp t)
{
    return (t->tp->is_action_text);
}

static inline uint8_t thing_is_visible_on_debug_only_noverify (thingp t)
{
    return (t->tp->is_visible_on_debug_only);
}

static inline uint8_t thing_is_action_sleep_noverify (thingp t)
{
    return (t->tp->is_action_sleep);
}

static inline uint8_t thing_is_action_zap_noverify (thingp t)
{
    return (t->tp->is_action_zap);
}

static inline uint8_t thing_is_action_trigger_noverify (thingp t)
{
    return (t->tp->is_action_trigger);
}

static inline uint8_t thing_is_action_trigger_on_monst_noverify (thingp t)
{
    return (t->tp->is_action_trigger_on_monst);
}

static inline uint8_t thing_is_action_trigger_on_hero_noverify (thingp t)
{
    return (t->tp->is_action_trigger_on_hero);
}

static inline uint8_t thing_is_action_trigger_on_wall_noverify (thingp t)
{
    return (t->tp->is_action_trigger_on_wall);
}

static inline uint8_t thing_is_action_down_noverify (thingp t)
{
    return (t->tp->is_action_down);
}

static inline uint8_t thing_is_action_up_noverify (thingp t)
{
    return (t->tp->is_action_up);
}

static inline uint8_t thing_is_action_left_noverify (thingp t)
{
    return (t->tp->is_action_left);
}

static inline uint8_t thing_is_action_right_noverify (thingp t)
{
    return (t->tp->is_action_right);
}

static inline uint8_t thing_can_walk_through_noverify (thingp t)
{
    return (t->tp->can_walk_through);
}

static inline uint8_t thing_is_weapon_carry_anim_noverify (thingp t)
{
    return (t->tp->is_weapon_carry_anim);
}

static inline uint8_t thing_is_powerup_anim_noverify (thingp t)
{
    return (t->tp->is_powerup_anim);
}

static inline uint8_t thing_is_spell_noverify (thingp t)
{
    return (t->tp->is_spell);
}

static inline uint8_t thing_is_hand_item_noverify (thingp t)
{
    return (t->tp->is_hand_item);
}

static inline uint8_t thing_is_boots_noverify (thingp t)
{
    return (t->tp->is_boots);
}

static inline uint8_t thing_is_helmet_noverify (thingp t)
{
    return (t->tp->is_helmet);
}

static inline uint8_t thing_is_armor_noverify (thingp t)
{
    return (t->tp->is_armor);
}

static inline uint8_t thing_is_given_randomly_at_start_noverify (thingp t)
{
    return (t->tp->is_given_randomly_at_start);
}

static inline uint8_t thing_is_magical_noverify (thingp t)
{
    return (t->tp->is_magical);
}

static inline uint8_t thing_is_degradable_noverify (thingp t)
{
    return (t->tp->is_degradable);
}

static inline uint8_t thing_is_cursed_noverify (thingp t)
{
    return (t->item.cursed);
}

static inline uint8_t thing_is_animate_only_when_moving_noverify (thingp t)
{
    return (t->tp->is_animate_only_when_moving);
}

static inline uint8_t thing_is_warm_blooded_noverify (thingp t)
{
    return (t->tp->is_warm_blooded);
}

static inline uint8_t thing_can_be_enchanted_noverify (thingp t)
{
    return (t->tp->can_be_enchanted);
}

static inline uint8_t thing_is_stackable_noverify (thingp t)
{
    return (t->tp->is_stackable);
}

static inline uint8_t thing_is_torch_noverify (thingp t)
{
    return (t->tp->is_torch);
}

static inline uint8_t thing_is_explosion_noverify (thingp t)
{
    return (t->tp->is_explosion);
}

static inline uint8_t thing_is_hidden_from_editor_noverify (thingp t)
{
    return (t->tp->is_hidden_from_editor);
}

static inline uint8_t thing_is_animated_noverify (thingp t)
{
    return (t->tp->is_animated);
}

static inline uint8_t thing_is_combustable_noverify (thingp t)
{
    return (t->tp->is_combustable);
}

static inline uint8_t thing_is_projectile_noverify (thingp t)
{
    return (t->tp->is_projectile);
}

static inline uint8_t thing_is_joinable_noverify (thingp t)
{
    return (t->tp->is_joinable);
}

static inline uint8_t thing_is_wall_noverify (thingp t)
{
    return (t->tp->is_wall);
}

static inline uint8_t thing_is_effect_sway_noverify (thingp t)
{
    return (t->tp->is_effect_sway);
}

static inline uint8_t thing_is_effect_pulse_noverify (thingp t)
{
    return (t->tp->is_effect_pulse);
}

static inline uint8_t thing_is_effect_rotate_4way_noverify (thingp t)
{
    return (t->tp->is_effect_rotate_4way);
}

static inline uint8_t thing_is_effect_rotate_2way_noverify (thingp t)
{
    return (t->tp->is_effect_rotate_2way);
}

typedef struct {
    uint8_t count;
    uint32_t id[MAP_THINGS_PER_CELL];
} thing_map_cell;

typedef struct {
    thing_map_cell cells[MAP_WIDTH][MAP_HEIGHT];
} thing_map;

extern thing_map thing_server_map;
extern thing_map thing_client_map;
extern levelp server_level;
extern levelp client_level;

static inline thing_map *thing_get_map (thingp t)
{
    if (t && t->on_server) {
        return (&thing_server_map);
    }
    return (&thing_client_map);
}

static inline thing_map *level_get_thing_map (levelp level)
{
    if (level == server_level) {
        return (&thing_server_map);
    }
    return (&thing_client_map);
}

extern thingp thing_server_ids[THING_ID_MAX];
extern thingp thing_client_ids[THING_CLIENT_ID_MAX];

static inline thingp thing_server_id (uint32_t id) 
{
    if (id > THING_ID_MAX) {
        DIE("overflow in looking up server IDs, ID %u", id);
    }

    if (!thing_server_ids[id]) {
        DIE("looking for client thing ID %u, not found", id);
    }

    verify(thing_server_ids[id]);

    return (thing_server_ids[id]);
}

static inline thingp thing_client_id (uint32_t id) 
{
    if (id > THING_CLIENT_ID_MAX) {
        DIE("overflow in looking up client IDs, ID %u", id);
    }

    if (!thing_client_ids[id]) {
        DIE("looking for client thing ID %u, not found", id);
    }

    verify(thing_client_ids[id]);

    return (thing_client_ids[id]);
}

static inline levelp thing_level (thingp t)
{
    extern levelp server_level;
    extern levelp client_level;

    if (t->on_server) {
        if (!server_level) {
            DIE("server level not found for thing on server");
        }
        return (server_level);
    }

    if (!client_level) {
        DIE("client level not found for thing on client");
    }

    return (client_level);
}

/*
 * thing_weapon.h
 */
static inline itemp thing_has_inventory_item (const thingp t, 
                                              const uint32_t id)
{
    return (thing_stats_has_inventory_item(&t->stats, id, 0));
}

static inline itemp thing_has_action_bar_item (const thingp t,
                                               const uint32_t id) 
{
    return (thing_stats_has_action_bar_item(&t->stats, id, 0));
}

static inline itemp thing_has_worn_item (const thingp t, 
                                         const uint32_t id)
{
    return (thing_stats_has_worn_item(&t->stats, id, 0));
}

static inline itemp thing_has_item (const thingp t,
                                    const uint32_t id)
{
    return (thing_stats_has_item(&t->stats, id, 0));
}

static inline itemp thing_weapon_item (const thingp t)
{
    itemp item = 
        thing_has_action_bar_item(t, 
            t->stats.action_bar[t->stats.action_bar_index].id);

    return (item);
}

static inline tpp thing_weapon (const thingp t)
{
    if (t->weapon) {
        return (t->weapon);
    }

    itemp item = thing_weapon_item(t);
    if (!item) {
        return (0);
    }

    if (!item->id) {
        return (0);
    }

    return (id_to_tp(item->id));
}

static inline tpp thing_shield (const thingp t)
{
    return (t->shield);
}

/*
 * Only a certain resolution of thing can be represented on the client.
 * Convert a floating point value to client rounded value.
 */
static inline void thing_round (thingp t, double *x, double *y)
{
    const double scale = 256 / MAP_WIDTH;

    *x = round(*x * scale) / scale;
    *y = round(*y * scale) / scale;
}

/*
 * thing.c
 */
void thing_move(thingp t, double x, double y);

void thing_move_set_dir(thingp t,
                        double *x,
                        double *y,
                        uint8_t up,
                        uint8_t down,
                        uint8_t left,
                        uint8_t right);

uint8_t thing_wid_is_inactive(widp w);
uint8_t thing_wid_is_active(widp w);

/*
 * thing_server.c
 */
uint8_t thing_server_move(thingp t,
                          double x,
                          double y,
                          const uint8_t up,
                          const uint8_t down,
                          const uint8_t left,
                          const uint8_t right,
                          const uint8_t fire);

void thing_server_action(thingp t,
                         uint8_t action,
                         uint32_t action_bar_index);

/*
 * thing_effect.c
 */
void thing_server_effect(thingp t, int effect);
void thing_client_effect(thingp t, int effect);

/*
 * thing_item.c
 */
void thing_auto_collect(thingp t, thingp it, tpp tp);
void thing_item_collect(thingp t, thingp it, tpp tp);
void thing_used(thingp t, tpp tp);
void thing_drop(thingp t, tpp tp);
int thing_wear_out(thingp t, tpp tp);
void thing_item_destroyed(thingp t, tpp tp);
void thing_wield_next_weapon(thingp t);

/*
 * thing_collision.c
 */
void 
thingp_get_interpolated_position(const thingp t, double *x, double *y);

/*
 * thing_place.c
 */
widp thing_place(thingp t, tpp tp, itemp item);
widp thing_place_behind(thingp t, tpp tp, itemp item);
widp thing_place_behind_or_under(thingp t, tpp tp, itemp item);
void thing_dir(thingp t, double *dx, double *dy);

/*
 * thing_torch.c
 */
void thing_torch_update_count(thingp t, int force);
void thing_torch_tick(thingp t);

/*
 * thing_shield.c
 */
thingp thing_shield_anim(thingp t);
void thing_unwield_shield(thingp t);
void thing_wield_shield(thingp t, tpp shield);
widp thing_get_shield_anim_wid(thingp t);
void thing_set_shield_anim_id(thingp t, uint32_t shield_anim_id);
void thing_set_shield_anim(thingp t, thingp shield_anim);
void thing_shield_sheath(thingp t);

/*
 * thing_weapon.c
 */
void thing_unwield(thingp t);
void thing_weapon_sheath(thingp t);
void thing_wield(thingp t, tpp tp);
void thing_swing(thingp t);
void thing_weapon_swing_offset(thingp t, double *dx, double *dy);
thingp thing_weapon_carry_anim(thingp t);
thingp thing_weapon_swing_anim(thingp t);
void thing_set_weapon_placement(thingp t);
widp thing_get_weapon_carry_anim_wid(thingp t);
void thing_weapon_worn_out(thingp owner, tpp weapon);
void thing_weapon_check_for_wear_damage(thingp target, 
                                       thingp hitter, 
                                       tpp weapon);
void thing_weapon_check_for_damage_on_firing(thingp hitter,
                                             tpp weapon);
void thing_set_weapon_carry_anim_id(thingp t, uint32_t weapon_carry_anim_id);
void thing_set_weapon_carry_anim(thingp t, thingp weapon_carry_anim);
void thing_set_weapon_swing_anim_id(thingp t, uint32_t weapon_swing_anim_id);
void thing_set_weapon_swing_anim(thingp t, thingp weapon_swing_anim);

/*
 * thing_explosion.c
 */
extern int level_explosion_flash_effect;

void level_place_explosion_at(levelp level,
                              thingp owner,
                              double ox, 
                              double oy, 
                              double x, 
                              double y, 
                              uint8_t dist,
                              uint8_t epicenter,
                              uint32_t nargs,
                              va_list args);
void level_place_explosion(levelp level, 
                           thingp owner,
                           tpp explodes_as,
                           double x, double y);
void explosion_flood(levelp level, uint8_t x, uint8_t y);

/*
 * thing_blood.c
 */
void level_place_hit_success(levelp level, 
                         thingp owner,
                         double x, double y);
void level_place_hit_miss(levelp level, 
                         thingp owner,
                         double x, double y);
void level_place_blood(levelp level, 
                       thingp owner,
                       double x, double y);
void level_place_blood_crit(levelp level, 
                            thingp owner,
                            double x, double y);
/*
 * thing_bomb.c
 */
thingp level_place_bomb(levelp level, 
                        thingp owner,
                        double x, double y);
/*
 * thing_collision.c
 */
void thing_handle_collisions(widp grid, thingp t);

/*
 * thing_fire.c
 */
void thing_fire_at_target(thingp me);

void thing_server_fire(thingp t,
                       const uint8_t up,
                       const uint8_t down,
                       const uint8_t left,
                       const uint8_t right);
/*
 * thing_damage.c
 */
int32_t thing_stats_get_total_damage(thingp t);

/*
 * thing_speed.c
 */
double thing_stats_get_total_speed(thingp t);

/*
 * thing_damage.c
 */
int32_t thing_stats_get_total_damage_minus_defense(thingp t, 
                                                   thingp hitter, 
                                                   int32_t damage);

/*
 * thing_vision.c
 */
double thing_stats_get_total_vision(thingp t, double vision);

/*
 * thing_health.c
 */
void thing_health_tick(thingp t);

/*
 * thing_magic.c
 */
void thing_magic_tick(thingp t);
