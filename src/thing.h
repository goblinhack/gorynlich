/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
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

extern int server_things_total;
extern int client_things_total;
extern int server_monst_things_total;
extern int client_monst_things_total;

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
void thing_game_over(thingp);
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
void thing_animate(thingp);
uint8_t thing_hit_solid_obstacle(widp grid, thingp t, double nx, double ny);
uint8_t thing_hit_any_obstacle(widp grid, thingp t, double nx, double ny);
uint8_t thing_server_hit_solid_obstacle(widp grid, 
                                        thingp t, double nx, double ny);
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
void thing_set_is_angry(thingp t, uint8_t val);
void thing_set_is_projectile(thingp t, uint8_t val);
void thing_set_is_dead(thingp t, uint8_t val);
void thing_set_is_game_over(thingp t, uint8_t val);
const char *thing_name(thingp);
const char *thing_short_name(thingp);
const char *thing_tooltip(thingp);
uint8_t thing_z_depth(thingp);
uint8_t thing_z_order(thingp);
int32_t thing_player_cost(thingp);
int32_t thing_monst_cost(thingp);
double thing_speed(thingp);
tree_rootp thing_tiles(thingp);

thing_tilep thing_current_tile(thingp t);
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
int dmap_distance_to_player(int source_x, int source_y);
int dmap_distance_between_points(int target_x, int target_y, int source_x, int source_y);
uint8_t thing_find_nexthop(thingp t, int32_t *x, int32_t *y);
void dmap_process_init(void);
void dmap_process_fini(void);
void dmap_generate_map(double x, double y);
void dmap_generate_map_wander(levelp level);
void dmap_generate(levelp level, int force);

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
    THING_STATE_BIT_SHIFT_EXT2_MAGIC_ID_PRESENT,
    THING_STATE_BIT_SHIFT_EXT2_SCALE,
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
    THING_STATE_EFFECT_IS_FLAMES,
    THING_STATE_EFFECT_IS_BUBBLES,
    THING_STATE_EFFECT_IS_POWER_UP,
    THING_STATE_EFFECT_IS_HIT_CRIT,
};

void thing_client_move(thingp t,
                       double x,
                       double y,
                       const uint8_t up,
                       const uint8_t down,
                       const uint8_t left,
                       const uint8_t right,
                       const uint8_t fire,
                       const uint8_t magic);

void thing_set_owner_id(thingp t, uint32_t owner_id);
void thing_set_owner(thingp t, thingp owner);

enum {
    THING_NONE,
    THING_WALL1,
    THING_WALL_LIT1,
    THING_CRYSTAL1,
    THING_CRYSTAL2,
    THING_CRYSTAL3,
    THING_CRYSTAL4,
    THING_CRYSTAL5,
    THING_CRYSTAL6,
    THING_CRYSTAL7,
    THING_ROCK1,
    THING_ROCK2,
    THING_ROCK3,
    THING_ROCK4,
    THING_ROCK5,
    THING_ROCK6,
    THING_ROCK7,
    THING_ROCK1a,
    THING_ROCK2a,
    THING_ROCK3a,
    THING_ROCK4a,
    THING_ROCK5a,
    THING_ROCK6a,
    THING_ROCK7a,
    THING_CORRIDOR_WALL1,
    THING_WALL2,
    THING_WALL3,
    THING_WALL4,
    THING_WALL5,
    THING_WALL6,
    THING_WALL7,
    THING_WALL8,
    THING_WALL9,
    THING_WALL10,
    THING_WALL11,
    THING_DOOR1,
    THING_DOOR2,
    THING_JUGGERNAUT,
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
    THING_PIZZA,
    THING_MUSHROOM1,
    THING_MUSHROOM2,
    THING_MUSHROOM3,
    THING_MUSHROOM4,
    THING_MUSHROOM5,
    THING_MUSHROOM6,
    THING_MUSHROOM7,
    THING_MUSHROOM8,
    THING_MUSHROOM9,
    THING_MUSHROOM10,
    THING_MUSHROOM11,
    THING_MUSHROOM12,
    THING_MUSHROOM13,
    THING_MUSHROOM14,
    THING_MUSHROOM15,
    THING_MUSHROOM16,
    THING_MUSHROOM17,
    THING_MUSHROOM18,
    THING_MUSHROOM19,
    THING_MUSHROOM20,
    THING_MUSHROOM21,
    THING_MUSHROOM22,
    THING_MUSHROOM23,
    THING_MUSHROOM24,
    THING_MUSHROOM25,
    THING_MUSHROOM26,
    THING_MUSHROOM27,
    THING_MUSHROOM28,
    THING_MUSHROOM29,
    THING_MUSHROOM30,
    THING_MUSHROOM31,
    THING_MUSHROOM32,
    THING_MUSHROOM33,
    THING_MUSHROOM34,
    THING_MUSHROOM35,
    THING_MUSHROOM36,
    THING_MUSHROOM37,
    THING_MUSHROOM38,
    THING_MUSHROOM39,
    THING_MUSHROOM40,
    THING_MUSHROOM41,
    THING_MUSHROOM42,
    THING_MUSHROOM43,
    THING_MUSHROOM44,
    THING_MUSHROOM45,
    THING_MUSHROOM46,
    THING_MUSHROOM47,
    THING_MUSHROOM48,
    THING_MUSHROOM49,
    THING_MUSHROOM50,
    THING_MUSHROOM51,
    THING_MUSHROOM52,
    THING_MUSHROOM53,
    THING_MUSHROOM54,
    THING_MUSHROOM55,
    THING_MUSHROOM56,
    THING_MUSHROOM57,
    THING_MUSHROOM58,
    THING_MUSHROOM59,
    THING_MUSHROOM60,
    THING_MUSHROOM61,
    THING_MUSHROOM62,
    THING_MUSHROOM63,
    THING_MUSHROOM64,
    THING_FOOD1,
    THING_FOOD2,
    THING_FOOD3,
    THING_FOOD4,
    THING_FOOD5,
    THING_FOOD6,
    THING_FOOD7,
    THING_FOOD8,
    THING_FOOD9,
    THING_FOOD10,
    THING_FOOD11,
    THING_FOOD12,
    THING_FOOD13,
    THING_FOOD14,
    THING_FOOD15,
    THING_FOOD16,
    THING_FOOD17,
    THING_FOOD18,
    THING_FOOD19,
    THING_FOOD20,
    THING_FOOD21,
    THING_FOOD22,
    THING_FOOD23,
    THING_FOOD24,
    THING_FOOD25,
    THING_FOOD26,
    THING_FOOD27,
    THING_FOOD28,
    THING_FOOD29,
    THING_FOOD30,
    THING_FOOD31,
    THING_FOOD32,
    THING_FOOD33,
    THING_FOOD34,
    THING_FOOD35,
    THING_FOOD36,
    THING_FOOD37,
    THING_FOOD38,
    THING_FOOD39,
    THING_FOOD40,
    THING_FOOD41,
    THING_FOOD42,
    THING_FOOD43,
    THING_FOOD44,
    THING_FOOD45,
    THING_FOOD46,
    THING_FOOD47,
    THING_FOOD48,
    THING_FOOD49,
    THING_FOOD50,
    THING_FOOD51,
    THING_FOOD52,
    THING_FOOD53,
    THING_FOOD54,
    THING_FOOD55,
    THING_FOOD56,
    THING_FOOD57,
    THING_FOOD58,
    THING_FOOD59,
    THING_FOOD60,
    THING_FOOD61,
    THING_FOOD62,
    THING_FOOD63,
    THING_FOOD64,
    THING_FOOD65,
    THING_FOOD66,
    THING_FOOD67,
    THING_FOOD68,
    THING_FOOD69,
    THING_FOOD70,
    THING_FOOD71,
    THING_FOOD72,
    THING_FOOD73,
    THING_FOOD74,
    THING_FOOD75,
    THING_FOOD76,
    THING_FOOD77,
    THING_FOOD78,
    THING_FOOD79,
    THING_FOOD80,
    THING_FOOD81,
    THING_FOOD82,
    THING_FOOD83,
    THING_FOOD84,
    THING_FOOD85,
    THING_FOOD86,
    THING_FOOD87,
    THING_FOOD88,
    THING_FOOD89,
    THING_FOOD90,
    THING_FOOD91,
    THING_FOOD92,
    THING_FOOD93,
    THING_FOOD94,
    THING_FOOD95,
    THING_FOOD96,
    THING_FOOD97,
    THING_FOOD98,
    THING_FOOD99,
    THING_FOOD100,
    THING_SPAM,
    THING_ALL_CHEESE,
    THING_BREAD,
    THING_SHOP_FLOOR1,
    THING_CORRIDOR1,
    THING_CORRIDOR2,
    THING_DIRT1,
    THING_FLOOR1,
    THING_FLOOR2,
    THING_FLOOR3,
    THING_FLOOR4,
    THING_FLOOR5,
    THING_FLOOR6,
    THING_FLOOR7,
    THING_FLOOR8,
    THING_FLOOR9,
    THING_FLOOR10,
    THING_FLOOR11,
    THING_FLOOR12,
    THING_FLOOR13,
    THING_FLOOR14,
    THING_FLOOR15,
    THING_FLOOR16,
    THING_FLOOR17,
    THING_FLOOR18,
    THING_FLOOR19,
    THING_FLOOR20,
    THING_FLOOR21,
    THING_FLOOR22,
    THING_FLOOR23,
    THING_FLOOR24,
    THING_FLOOR25,
    THING_FLOOR26,
    THING_FLOOR27,
    THING_FLOOR28,
    THING_FLOOR29,
    THING_FLOOR30,
    THING_FLOOR31,
    THING_FLOOR32,
    THING_FLOOR33,
    THING_FLOOR34,
    THING_FLOOR35,
    THING_FLOOR36,
    THING_FLOOR37,
    THING_FLOOR38,
    THING_FLOOR39,
    THING_FLOOR40,
    THING_FLOOR41,
    THING_FLOOR42,
    THING_FLOOR43,
    THING_FLOOR44,
    THING_FLOOR45,
    THING_FLOOR46,
    THING_FLOOR47,
    THING_FLOOR48,
    THING_FLOOR49,
    THING_FLOOR50,
    THING_FLOOR51,
    THING_FLOOR52,
    THING_FLOOR53,
    THING_FLOOR54,
    THING_FLOOR55,
    THING_FLOOR56,
    THING_FLOOR57,
    THING_FLOOR58,
    THING_FLOOR59,
    THING_FLOOR60,
    THING_FLOOR61,
    THING_FLOOR62,
    THING_FLOOR63,
    THING_FLOOR64,
    THING_FLOOR65,
    THING_FLOOR66,
    THING_FLOOR67,
    THING_FLOOR68,
    THING_WARRIOR,
    THING_VALKYRIE,
    THING_WIZARD,
    THING_SORCEROR,
    THING_SORCERORMOB,
    THING_ELF,
    THING_DWARF,
    THING_ZOMBIE1,
    THING_ZOMBIE2,
    THING_ZOMBIE3,
    THING_ZOMBIE4,
    THING_ZOMBIE5,
    THING_ZOMBIE_CORPSE1,
    THING_ZOMBIE_CORPSE2,
    THING_ZOMBIE_CORPSE3,
    THING_ZOMBIE_CORPSE4,
    THING_ZOMBIE_CORPSE5,
    THING_SKELETON1,
    THING_PLANTPOD,
    THING_PLANT,
    THING_ORC1,
    THING_ORC2,
    THING_ORC3,
    THING_ORC4,
    THING_ORC5,
    THING_SHOPKEEPER,
    THING_GHOST1,
    THING_GHOST2,
    THING_GHOST3,
    THING_GHOST4,
    THING_SLIME1,
    THING_SLIME2,
    THING_SLIME3,
    THING_SLIME4,
    THING_FUNGUY,
    THING_ORBAGLOOP,
    THING_SPLOOB,
    THING_DEMON1,
    THING_DEMON2,
    THING_DRAGON1,
    THING_DRAGON2,
    THING_DRAGON3,
    THING_DRAGON4,
    THING_SKULL1,
    THING_SKULL2,
    THING_SQUIDLING1,
    THING_SQUIDLING2,
    THING_EYEBAWL,
    THING_SPIDER1,
    THING_SPIDER2,
    THING_MUDMAN,
    THING_JESUS,
    THING_DEATH,
    THING_BABY_DEATH,
    THING_CLUB1_ANIM,
    THING_WAND1_ANIM,
    THING_WAND2_ANIM,
    THING_WAND3_ANIM,
    THING_WAND4_ANIM,
    THING_STICK1_ANIM,
    THING_STICK2_ANIM,
    THING_SHOTGUN_ANIM,
    THING_SWORD1_ANIM,
    THING_SWORD2_ANIM,
    THING_SWORD3_ANIM,
    THING_SWORD4_ANIM,
    THING_SWORD5_ANIM,
    THING_SWORD6_ANIM,
    THING_SWORD7_ANIM,
    THING_SWORD8_ANIM,
    THING_SWORD9_ANIM,
    THING_SWORD10_ANIM,
    THING_BOW1_ANIM,
    THING_AXE1_ANIM,
    THING_AXE2_ANIM,
    THING_SCYTHE1_ANIM,
    THING_ANIM_TREASURE_CHEST,
    THING_ANIM_EYES,
    THING_WEAPON_HIT1,
    THING_WEAPON_HIT2,
    THING_EXPLOSION1,
    THING_EXPLOSION2,
    THING_EXPLOSION3,
    THING_EXPLOSION4,
    THING_SMALL_EXPLOSION1,
    THING_SMALL_EXPLOSION2,
    THING_SMALL_EXPLOSION3,
    THING_SMALL_EXPLOSION4,
    THING_MED_EXPLOSION1,
    THING_MED_EXPLOSION2,
    THING_MED_EXPLOSION3,
    THING_MED_EXPLOSION4,
    THING_FIREBURST1,
    THING_FIREBURST2,
    THING_FIREBURST3,
    THING_FIREBURST4,
    THING_SHIELD1,
    THING_SHIELD1_ANIM,
    THING_DEATH_CLOUD,
    THING_DEATH_CLOUD_ANIM,
    THING_RING_CLOUD,
    THING_RING_CLOUD_ANIM,
    THING_JESUS_CLOUD,
    THING_JESUS_CLOUD_ANIM,
    THING_MAGIC1,
    THING_MAGIC_CLOUD,
    THING_POWERUP1,
    THING_HIT_SUCCESS,
    THING_HIT_MISS,
    THING_HEART,
    THING_BLOOD1,
    THING_BLOOD2,
    THING_SLIMEBALL,
    THING_FLAMES1,
    THING_BUBBLES1,
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
    THING_CLOUDKILL2,
    THING_POTION_JESUS,
    THING_POTION_MONSTICIDE,
    THING_POTION_FIRE,
    THING_POTION_CLOUDKILL,
    THING_POTION_LIFE,
    THING_POTION_SHIELD,
    THING_POTION_ANY, 
    THING_FOOD_ANY, 
    THING_MOB_ANY, 
    THING_MONST_ANY, 
    THING_TREASURE_ANY, 
    THING_WEAPON_ANY, 
    THING_WATER,
    THING_WATER_POISON,
    THING_RING1,
    THING_RING2,
    THING_RING3,
    THING_RING4,
    THING_RING5,
    THING_RING6,
    THING_TRAP1,
    THING_TRAP2,
    THING_TRAP3,
    THING_GEM1,
    THING_GEM2,
    THING_GEM3,
    THING_GEM4,
    THING_GEM5,
    THING_GEM6,
    THING_GEM7,
    THING_BRAZIER,
    THING_TELEPORT1,
    THING_LAVA1,
    THING_ACID1,
    THING_EXIT1,
    THING_GENERATOR1,
    THING_ZOMBIEMOB,
    THING_ORCMOB,
    THING_GHOSTMOB1,
    THING_GHOSTMOB2,
    THING_GHOSTMOB3,
    THING_GHOSTMOB4,
    THING_EYEBAWLMOB,
    THING_GOBBLER,
    THING_GOBBLERMOB,
    THING_BOMB,
    THING_BOW1,
    THING_BOW2,
    THING_BOW3,
    THING_BOW4,
    THING_WAND1,
    THING_WAND2,
    THING_WAND3,
    THING_WAND4,
    THING_CLUB1,
    THING_SHOTGUN,
    THING_PELLETS,
    THING_STICK1,
    THING_STICK2,
    THING_SWORD1,
    THING_SWORD2,
    THING_SWORD3,
    THING_SWORD4,
    THING_SWORD5,
    THING_SWORD6,
    THING_SWORD7,
    THING_SWORD8,
    THING_SWORD9,
    THING_SWORD10,
    THING_AXE1,
    THING_AXE2,
    THING_AXE3,
    THING_AXE4,
    THING_AXE5,
    THING_SCYTHE1,
    THING_KEY,
    THING_KEYS2,
    THING_KEYS3,
    THING_LIGHT,
    THING_TORCH,
    THING_TORCHES,
    THING_COINS1,
    THING_CHEST1,
    THING_CHEST2,
    THING_ARROW,
    THING_FIREBALL1,
    THING_FIREBALL2,
    THING_FIREBALL3,
    THING_FIREBALL4,
    THING_ENERGY1,
    THING_ENERGY2,
    THING_PLASMABALL1,
    THING_PLASMABALL2,
    THING_SILKBALL,
    THING_MUD,
    THING_COBWEB,
    THING_STICKYSLIME,
    THING_BOOTS1,
    THING_BOOTS2,
    THING_BOOTS3,
    THING_BOOTS4,
    THING_BOOTS5,
    THING_ARMOUR1,
    THING_ARMOUR2,
    THING_ARMOUR3,
    THING_ARMOUR4,
    THING_HELMET1,
    THING_HELMET2,
    THING_HELMET3,
    THING_HELMET4,

    THING_GRASS1,

    THING_STONE1,
    THING_STONE2,
    THING_STONE3,
    THING_STONE4,
    THING_STONE5,
    THING_STONE6,
    THING_STONE7,
    THING_STONE8,

    THING_SOIL1,
    THING_SOIL2,
    THING_SOIL3,
    THING_SOIL4,
    THING_SOIL5,
    THING_SOIL6,
    THING_SOIL7,
    THING_SOIL8,

    THING_SNOW1,
    THING_SNOW2,
    THING_SNOW3,
    THING_SNOW4,
    THING_SNOW5,
    THING_SNOW6,
    THING_SNOW7,
    THING_SNOW8,

    THING_WATER1,
    THING_WATER2,
    THING_WATER3,
    THING_WATER4,
    THING_WATER5,
    THING_WATER6,
    THING_WATER7,
    THING_WATER8,

    THING_SAND1,
    THING_SAND2,
    THING_SAND3,
    THING_SAND4,
    THING_SAND5,
    THING_SAND6,
    THING_SAND7,
    THING_SAND8,

    THING_MAX_ID,
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
    uint16_t shield_carry_anim_thing_id;
    uint16_t magic_anim_thing_id;

    /*
     * Weapon thing template.
     */
    tpp weapon;

    /*
     * Current shield or magic effect
     */
    tpp shield;
    tpp magic_anim;

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
     * How many things this thing owns.
     */
    uint16_t owned_count;

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
     * How close for collision detection.
     */
    double collision_radius;

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
     * Thing to be shown scaled on the client
     */
    double scale;

    /*
     * Last co-ords sent to the client
     */
    uint16_t last_tx;
    uint16_t last_ty;

    /*
     * For animation.
     */
    thing_tilep current_tile;

    /*
     * Data we read in along with the thing template.
     */
    thing_template_data *data;

    /*
     * Last time we were teleported.
     */
    uint32_t timestamp_last_teleport;

    /*
     * Last time we entered a shop.
     */
    uint32_t timestamp_last_shop_enter;

    /*
     * Last time we were attacked.
     */
    uint32_t timestamp_last_attacked;

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
     * When did I last hit?
     */
    uint32_t timestamp_i_attacked;

    /*
     * When did we last send our state to the client?
     */
    uint32_t timestamp_tx_map_update;

    /*
     * When life sputtered into this corpse
     */
    uint32_t timestamp_born;

    /*
     * Last time it fired at a player.
     */
    uint32_t timestamp_fired;

    /*
     * Last utterance
     */
    uint32_t timestamp_sound_random;

    /*
     * Last time we added magic to our build up of power!
     */
    uint32_t timestamp_magic_powerup;

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

    /*
     * e.g. IS_JOIN_BLOCK
     */
    uint8_t join_index;

    uint8_t dir:3;

    /*
     * How many rays of light are hitting this thing?
     */
    uint16_t lit;

    /*
     * Builds up as player holds down the magic key until it is fired.
     */
    uint16_t magic_powerup;

    /*
     * Used to detect exit...
     */
    uint16_t in_shop_owned_by_thing_id;

    /*
     * Money to be paid before leaving a shop
     */
    int money_owed;

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
    uint32_t is_dying:1;
    uint32_t is_angry:1;

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

    /*
     * Local abilities that override that of the template
     */
    uint8_t has_ability_reverse_shot:1;
    uint8_t has_ability_burst_shot:1;
    uint8_t has_ability_triple_shot:1;
    uint8_t has_ability_double_shot:1;

    uint8_t has_ability_reverse_swing:1;
    uint8_t has_ability_double_damage_swing:1;
    uint8_t has_ability_triple_speed_swing:1;
    uint8_t has_ability_double_speed_swing:1;

    uint8_t has_ability_rage:1;
    uint8_t has_ability_perma_rage:1;
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

static inline uint8_t thing_is_angry (thingp t)
{
    verify(t);

    return (t->is_angry);
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

static inline uint8_t thing_is_dungeon_floor (thingp t)
{
    verify(t);

    return (tp_is_dungeon_floor(thing_tp(t)));
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

static inline uint8_t thing_is_collision_map_player_sized (thingp t)
{
    verify(t);

    return (tp_is_collision_map_player_sized(thing_tp(t)));
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

static inline double thing_collision_radius (thingp t)
{
    verify(t);

    if (t->collision_radius > 0) {
        return (t->collision_radius);
    }

    return (tp_get_collision_radius(thing_tp(t)));
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

static inline uint8_t thing_is_door (thingp t)
{
    verify(t);

    return (tp_is_door(thing_tp(t)));
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

static inline uint8_t thing_is_crystal (thingp t)
{
    verify(t);

    return (tp_is_crystal(thing_tp(t)));
}

static inline uint8_t thing_is_rrr8 (thingp t)
{
    verify(t);

    return (tp_is_rrr8(thing_tp(t)));
}

static inline uint8_t thing_is_rrr9 (thingp t)
{
    verify(t);

    return (tp_is_rrr9(thing_tp(t)));
}

static inline uint8_t thing_is_rrr10 (thingp t)
{
    verify(t);

    return (tp_is_rrr10(thing_tp(t)));
}

static inline uint8_t thing_is_rrr11 (thingp t)
{
    verify(t);

    return (tp_is_rrr11(thing_tp(t)));
}

static inline uint8_t thing_is_rrr12 (thingp t)
{
    verify(t);

    return (tp_is_rrr12(thing_tp(t)));
}

static inline uint8_t thing_is_rrr13 (thingp t)
{
    verify(t);

    return (tp_is_rrr13(thing_tp(t)));
}

static inline uint8_t thing_is_mud (thingp t)
{
    verify(t);

    return (tp_is_mud(thing_tp(t)));
}

static inline uint8_t thing_is_not_animated_in_item_bar (thingp t)
{
    verify(t);

    return (tp_is_not_animated_in_item_bar(thing_tp(t)));
}

static inline uint8_t thing_is_conical_breath_attack (thingp t)
{
    verify(t);

    return (tp_is_conical_breath_attack(thing_tp(t)));
}

static inline uint8_t thing_is_rrr17 (thingp t)
{
    verify(t);

    return (tp_is_rrr17(thing_tp(t)));
}

static inline uint8_t thing_is_stickyslime (thingp t)
{
    verify(t);

    return (tp_is_stickyslime(thing_tp(t)));
}

static inline uint8_t thing_is_corpse (thingp t)
{
    verify(t);

    return (tp_is_corpse(thing_tp(t)));
}

static inline uint8_t thing_is_spawns_under (thingp t)
{
    verify(t);

    return (tp_is_spawns_under(thing_tp(t)));
}

static inline uint8_t thing_is_juggernaut (thingp t)
{
    verify(t);

    return (tp_is_juggernaut(thing_tp(t)));
}

static inline uint8_t thing_is_wanderer (thingp t)
{
    verify(t);

    return (tp_is_wanderer(thing_tp(t)));
}

static inline uint8_t thing_is_rrr24 (thingp t)
{
    verify(t);

    return (tp_is_rrr24(thing_tp(t)));
}

static inline uint8_t thing_is_world_floor (thingp t)
{
    verify(t);

    return (tp_is_world_floor(thing_tp(t)));
}

static inline uint8_t thing_is_soil (thingp t)
{
    verify(t);

    return (tp_is_soil(thing_tp(t)));
}

static inline uint8_t thing_is_sand (thingp t)
{
    verify(t);

    return (tp_is_sand(thing_tp(t)));
}

static inline uint8_t thing_is_stone (thingp t)
{
    verify(t);

    return (tp_is_stone(thing_tp(t)));
}

static inline uint8_t thing_is_snow (thingp t)
{
    verify(t);

    return (tp_is_snow(thing_tp(t)));
}

static inline uint8_t thing_is_grass (thingp t)
{
    verify(t);

    return (tp_is_grass(thing_tp(t)));
}

static inline uint8_t thing_is_life_saving (thingp t)
{
    verify(t);

    return (tp_is_life_saving(thing_tp(t)));
}

static inline uint8_t thing_is_single_mob_spawner (thingp t)
{
    verify(t);

    return (tp_is_single_mob_spawner(thing_tp(t)));
}

static inline uint8_t thing_is_jesus (thingp t)
{
    verify(t);

    return (tp_is_jesus(thing_tp(t)));
}

static inline uint8_t thing_is_blit_y_offset (thingp t)
{
    verify(t);

    return (tp_is_blit_y_offset(thing_tp(t)));
}

static inline uint8_t thing_is_dirt (thingp t)
{
    verify(t);

    return (tp_is_dirt(thing_tp(t)));
}

static inline uint8_t thing_is_dragon (thingp t)
{
    verify(t);

    return (tp_is_dragon(thing_tp(t)));
}

static inline uint8_t thing_is_water (thingp t)
{
    verify(t);

    return (tp_is_water(thing_tp(t)));
}

static inline uint8_t thing_is_undead (thingp t)
{
    verify(t);

    return (tp_is_undead(thing_tp(t)));
}

static inline uint8_t thing_is_hidden (thingp t)
{
    verify(t);

    return (tp_is_hidden(thing_tp(t)));
}

static inline uint8_t thing_is_trap (thingp t)
{
    verify(t);

    return (tp_is_trap(thing_tp(t)));
}

static inline uint8_t thing_is_corridor_wall (thingp t)
{
    verify(t);

    return (tp_is_corridor_wall(thing_tp(t)));
}

static inline uint8_t thing_is_corridor (thingp t)
{
    verify(t);

    return (tp_is_corridor(thing_tp(t)));
}

static inline uint8_t thing_is_water_proof (thingp t)
{
    verify(t);

    return (tp_is_water_proof(thing_tp(t)));
}

static inline uint8_t thing_is_lava_proof (thingp t)
{
    verify(t);

    return (tp_is_lava_proof(thing_tp(t)));
}

static inline uint8_t thing_is_spider_proof (thingp t)
{
    verify(t);

    return (tp_is_spider_proof(thing_tp(t)));
}

static inline uint8_t thing_is_acid_proof (thingp t)
{
    verify(t);

    return (tp_is_acid_proof(thing_tp(t)));
}

static inline uint8_t thing_is_fires_when_angry (thingp t)
{
    verify(t);

    return (tp_is_fires_when_angry(thing_tp(t)));
}

static inline uint8_t thing_is_shopkeeper (thingp t)
{
    verify(t);

    return (tp_is_shopkeeper(thing_tp(t)));
}

static inline uint8_t thing_is_shop_floor (thingp t)
{
    verify(t);

    return (tp_is_shop_floor(thing_tp(t)));
}

static inline uint8_t thing_is_fireball (thingp t)
{
    verify(t);

    return (tp_is_fireball(thing_tp(t)));
}

static inline uint8_t thing_is_bullet (thingp t)
{
    verify(t);

    return (tp_is_bullet(thing_tp(t)));
}

static inline uint8_t thing_is_treasure_eater (thingp t)
{
    verify(t);

    return (tp_is_treasure_eater(thing_tp(t)));
}

static inline uint8_t thing_is_effect_fade_in_out (thingp t)
{
    verify(t);

    return (tp_is_effect_fade_in_out(thing_tp(t)));
}

static inline uint8_t thing_is_internal (thingp t)
{
    verify(t);

    return (tp_is_internal(thing_tp(t)));
}

static inline uint8_t thing_is_levitating (thingp t)
{
    verify(t);

    return (tp_is_levitating(thing_tp(t)));
}

static inline uint8_t thing_is_acid (thingp t)
{
    verify(t);

    return (tp_is_acid(thing_tp(t)));
}

static inline uint8_t thing_is_lava (thingp t)
{
    verify(t);

    return (tp_is_lava(thing_tp(t)));
}

static inline uint8_t thing_is_teleport (thingp t)
{
    verify(t);

    return (tp_is_teleport(thing_tp(t)));
}

static inline uint8_t thing_is_potion (thingp t)
{
    verify(t);

    return (tp_is_potion(thing_tp(t)));
}

static inline uint8_t thing_is_shield (thingp t)
{
    verify(t);

    return (tp_is_shield(thing_tp(t)));
}

static inline uint8_t thing_is_death (thingp t)
{
    verify(t);

    return (tp_is_death(thing_tp(t)));
}

static inline uint8_t thing_is_cobweb (thingp t)
{
    verify(t);

    return (tp_is_cobweb(thing_tp(t)));
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

static inline uint8_t thing_is_shield_carry_anim (thingp t)
{
    verify(t);

    return (tp_is_shield_carry_anim(thing_tp(t)));
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

static inline uint8_t thing_is_effect_rotate_2way (thingp t)
{
    verify(t);

    return (tp_is_effect_rotate_2way(thing_tp(t)));
}

static inline uint8_t thing_is_open_noverify (thingp t)
{
    return (t->is_open);
}

static inline uint8_t thing_is_angry_noverify (thingp t)
{
    return (t->is_angry);
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

static inline uint8_t thing_is_dungeon_floor_noverify (thingp t)
{
    return (t->tp->is_dungeon_floor);
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

static inline uint8_t thing_is_collision_map_player_sized_noverify (thingp t)
{
    return (t->tp->is_collision_map_player_sized);
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

static inline uint8_t thing_is_door_noverify (thingp t)
{
    return (t->tp->is_door);
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

static inline uint8_t thing_is_crystal_noverify (thingp t)
{
    return (t->tp->is_crystal);
}

static inline uint8_t thing_is_rrr8_noverify (thingp t)
{
    return (t->tp->is_rrr8);
}

static inline uint8_t thing_is_rrr9_noverify (thingp t)
{
    return (t->tp->is_rrr9);
}

static inline uint8_t thing_is_rrr10_noverify (thingp t)
{
    return (t->tp->is_rrr10);
}

static inline uint8_t thing_is_rrr11_noverify (thingp t)
{
    return (t->tp->is_rrr11);
}

static inline uint8_t thing_is_rrr12_noverify (thingp t)
{
    return (t->tp->is_rrr12);
}

static inline uint8_t thing_is_rrr13_noverify (thingp t)
{
    return (t->tp->is_rrr13);
}

static inline uint8_t thing_is_mud_noverify (thingp t)
{
    return (t->tp->is_mud);
}

static inline uint8_t thing_is_not_animated_in_item_bar_noverify (thingp t)
{
    return (t->tp->is_not_animated_in_item_bar);
}

static inline uint8_t thing_is_conical_breath_attack_noverify (thingp t)
{
    return (t->tp->is_conical_breath_attack);
}

static inline uint8_t thing_is_rrr17_noverify (thingp t)
{
    return (t->tp->is_rrr17);
}

static inline uint8_t thing_is_stickyslime_noverify (thingp t)
{
    return (t->tp->is_stickyslime);
}

static inline uint8_t thing_is_corpse_noverify (thingp t)
{
    return (t->tp->is_corpse);
}

static inline uint8_t thing_is_spawns_under_noverify (thingp t)
{
    return (t->tp->is_spawns_under);
}

static inline uint8_t thing_is_juggernaut_noverify (thingp t)
{
    return (t->tp->is_juggernaut);
}

static inline uint8_t thing_is_wanderer_noverify (thingp t)
{
    return (t->tp->is_wanderer);
}

static inline uint8_t thing_is_rrr24_noverify (thingp t)
{
    return (t->tp->is_rrr24);
}

static inline uint8_t thing_is_world_floor_noverify (thingp t)
{
    return (t->tp->is_world_floor);
}

static inline uint8_t thing_is_soil_noverify (thingp t)
{
    return (t->tp->is_soil);
}

static inline uint8_t thing_is_sand_noverify (thingp t)
{
    return (t->tp->is_sand);
}

static inline uint8_t thing_is_stone_noverify (thingp t)
{
    return (t->tp->is_stone);
}

static inline uint8_t thing_is_snow_noverify (thingp t)
{
    return (t->tp->is_snow);
}

static inline uint8_t thing_is_grass_noverify (thingp t)
{
    return (t->tp->is_grass);
}

static inline uint8_t thing_is_life_saving_noverify (thingp t)
{
    return (t->tp->is_life_saving);
}

static inline uint8_t thing_has_ability_burst_shot_noverify (thingp t)
{
    return (t->tp->has_ability_burst_shot);
}

static inline uint8_t thing_is_single_mob_spawner_noverify (thingp t)
{
    return (t->tp->is_single_mob_spawner);
}

static inline uint8_t thing_has_ability_triple_shot_noverify (thingp t)
{
    return (t->tp->has_ability_triple_shot);
}

static inline uint8_t thing_has_ability_double_shot_noverify (thingp t)
{
    return (t->tp->has_ability_double_shot);
}

static inline uint8_t thing_has_ability_reverse_shot_noverify (thingp t)
{
    return (t->tp->has_ability_reverse_shot);
}

static inline uint8_t thing_is_jesus_noverify (thingp t)
{
    return (t->tp->is_jesus);
}

static inline uint8_t thing_is_blit_y_offset_noverify (thingp t)
{
    return (t->tp->is_blit_y_offset);
}

static inline uint8_t thing_is_dirt_noverify (thingp t)
{
    return (t->tp->is_dirt);
}

static inline uint8_t thing_is_dragon_noverify (thingp t)
{
    return (t->tp->is_dragon);
}

static inline uint8_t thing_is_mob_spawner_noverify (thingp t)
{
    return (t->tp->is_mob_spawner);
}

static inline uint8_t thing_is_water_noverify (thingp t)
{
    return (t->tp->is_water);
}

static inline uint8_t thing_is_undead_noverify (thingp t)
{
    return (t->tp->is_undead);
}

static inline uint8_t thing_is_hidden_noverify (thingp t)
{
    return (t->tp->is_hidden);
}

static inline uint8_t thing_is_trap_noverify (thingp t)
{
    return (t->tp->is_trap);
}

static inline uint8_t thing_is_corridor_wall_noverify (thingp t)
{
    return (t->tp->is_corridor_wall);
}

static inline uint8_t thing_is_corridor_noverify (thingp t)
{
    return (t->tp->is_corridor);
}

static inline uint8_t thing_is_water_proof_noverify (thingp t)
{
    return (t->tp->is_water_proof);
}

static inline uint8_t thing_is_lava_proof_noverify (thingp t)
{
    return (t->tp->is_lava_proof);
}

static inline uint8_t thing_is_spider_proof_noverify (thingp t)
{
    return (t->tp->is_spider_proof);
}

static inline uint8_t thing_is_acid_proof_noverify (thingp t)
{
    return (t->tp->is_acid_proof);
}

static inline uint8_t thing_is_fires_when_angry_noverify (thingp t)
{
    return (t->tp->is_fires_when_angry);
}

static inline uint8_t thing_is_shopkeeper_noverify (thingp t)
{
    return (t->tp->is_shopkeeper);
}

static inline uint8_t thing_is_shop_floor_noverify (thingp t)
{
    return (t->tp->is_shop_floor);
}

static inline uint8_t thing_is_fireball_noverify (thingp t)
{
    return (t->tp->is_fireball);
}

static inline uint8_t thing_is_bullet_noverify (thingp t)
{
    return (t->tp->is_bullet);
}

static inline uint8_t thing_is_treasure_eater_noverify (thingp t)
{
    return (t->tp->is_treasure_eater);
}

static inline uint8_t thing_is_effect_fade_in_out_noverify (thingp t)
{
    return (t->tp->is_effect_fade_in_out);
}

static inline uint8_t thing_is_internal_noverify (thingp t)
{
    return (t->tp->is_internal);
}

static inline uint8_t thing_is_levitating_noverify (thingp t)
{
    return (t->tp->is_levitating);
}

static inline uint8_t thing_is_acid_noverify (thingp t)
{
    return (t->tp->is_acid);
}

static inline uint8_t thing_is_lava_noverify (thingp t)
{
    return (t->tp->is_lava);
}

static inline uint8_t thing_is_teleport_noverify (thingp t)
{
    return (t->tp->is_teleport);
}

static inline uint8_t thing_is_potion_noverify (thingp t)
{
    return (t->tp->is_potion);
}

static inline uint8_t thing_is_shield_noverify (thingp t)
{
    return (t->tp->is_shield);
}

static inline uint8_t thing_is_death_noverify (thingp t)
{
    return (t->tp->is_death);
}

static inline uint8_t thing_is_cobweb_noverify (thingp t)
{
    return (t->tp->is_cobweb);
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

static inline uint8_t thing_is_shield_carry_anim_noverify (thingp t)
{
    return (t->tp->is_shield_carry_anim);
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

static inline tpp thing_armor (const thingp t)
{
    thing_statsp s = &t->stats;

    item_t item = s->worn[THING_WORN_ARMOR];

    if (!item.id) {
        return (0);
    }

    return (id_to_tp(item.id));
}

static inline tpp thing_helmet (const thingp t)
{
    thing_statsp s = &t->stats;

    item_t item = s->worn[THING_WORN_HELMET];

    if (!item.id) {
        return (0);
    }

    return (id_to_tp(item.id));
}

static inline tpp thing_boots (const thingp t)
{
    thing_statsp s = &t->stats;

    item_t item = s->worn[THING_WORN_BOOTS];

    if (!item.id) {
        return (0);
    }

    return (id_to_tp(item.id));
}

static inline tpp thing_arm_left (const thingp t)
{
    thing_statsp s = &t->stats;

    item_t item = s->worn[THING_WORN_ARM_LEFT];

    if (!item.id) {
        return (0);
    }

    return (id_to_tp(item.id));
}

static inline tpp thing_arm_right (const thingp t)
{
    thing_statsp s = &t->stats;

    item_t item = s->worn[THING_WORN_ARM_RIGHT];

    if (!item.id) {
        return (0);
    }

    return (id_to_tp(item.id));
}

static inline tpp thing_shield (const thingp t)
{
    return (t->shield);
}

static inline tpp thing_magic (const thingp t)
{
    return (t->magic_anim);
}

static inline int thing_has_ability_double_shot (const thingp t)
{
    if (t->has_ability_double_shot) {
        return (true);
    }

    return (tp_has_ability_double_shot(t->tp));
}

static inline int thing_has_ability_reverse_shot (const thingp t)
{
    if (t->has_ability_reverse_shot) {
        return (true);
    }

    return (tp_has_ability_reverse_shot(t->tp));
}

static inline int thing_has_ability_triple_shot (const thingp t)
{
    if (t->has_ability_triple_shot) {
        return (true);
    }

    return (tp_has_ability_triple_shot(t->tp));
}

static inline int thing_has_ability_burst_shot (const thingp t)
{
    if (t->has_ability_burst_shot) {
        return (true);
    }

    return (tp_has_ability_burst_shot(t->tp));
}

static inline int thing_has_ability_double_speed_swing (const thingp t)
{
    if (t->has_ability_double_speed_swing) {
        return (true);
    }

    return (tp_has_ability_double_speed_swing(t->tp));
}

static inline int thing_has_ability_reverse_swing (const thingp t)
{
    if (t->has_ability_reverse_swing) {
        return (true);
    }

    return (tp_has_ability_reverse_swing(t->tp));
}

static inline int thing_has_ability_triple_speed_swing (const thingp t)
{
    if (t->has_ability_triple_speed_swing) {
        return (true);
    }

    return (tp_has_ability_triple_speed_swing(t->tp));
}

static inline int thing_has_ability_double_damage_swing (const thingp t)
{
    if (t->has_ability_double_damage_swing) {
        return (true);
    }

    return (tp_has_ability_double_damage_swing(t->tp));
}

static inline int thing_has_ability_rage (const thingp t)
{
    if (t->has_ability_rage) {
        return (true);
    }

    return (tp_has_ability_rage(t->tp));
}

static inline int thing_has_ability_perma_rage (const thingp t)
{
    if (t->has_ability_perma_rage) {
        return (true);
    }

    return (tp_has_ability_perma_rage(t->tp));
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
                          const uint8_t fire,
                          const uint8_t magic);

void thing_server_action(thingp t,
                         uint8_t action,
                         const uint32_t action_bar_index,
                         const int change_selection_only);

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

/*
 * thing_dir.c
 */
void thing_dir(thingp t, double *dx, double *dy);
int thing_angle_to_dir(double dx, double dy);

/*
 * thing_torch.c
 */
void thing_torch_update_count(thingp t, int force);
void thing_torch_tick(thingp t);

/*
 * thing_death.c
 */
void thing_death_tick(void);
uint8_t thing_death_spawn(void);

/*
 * thing_jesus.c
 */
uint8_t thing_jesus_spawn(void);

/*
 * thing_shield.c
 */
void thing_unwield_shield(thingp t);
void thing_shield_sheath(thingp t);
void thing_wield_shield(thingp t, tpp tp);
thingp thing_shield_carry_anim(thingp t);
void thing_set_shield_placement(thingp t);
widp thing_get_shield_carry_anim_wid(thingp t);
void thing_set_shield_carry_anim_id(thingp t, uint32_t shield_carry_anim_id);
void thing_set_shield_carry_anim(thingp t, thingp shield_carry_anim);

/*
 * thing_weapon.c
 */
void thing_unwield(thingp t);
void thing_sheath(thingp t);
void thing_swing(thingp t);
void thing_wield(thingp t, tpp tp);
void thing_weapon_sheath(thingp t);
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
                              uint8_t is_epicenter,
                              const char *epicenter,
                              uint32_t nargs,
                              va_list args);
void level_place_explosion(levelp level, 
                           thingp owner,
                           tpp explodes_as,
                           double ox, double oy,
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
void level_place_flames(levelp level, 
                       thingp owner,
                       double x, double y);
void level_place_bubbles(levelp level, 
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
void level_place_light(levelp level, double x, double y);

/*
 * thing_health.c
 */
void thing_health_tick(thingp t);

/*
 * thing_xp.c
 */
void thing_modify_xp(thingp t, int val);

/*
 * thing_spending_points.c
 */
void thing_modify_spending_points(thingp t, int val);
void thing_stats_check_for_changes(thingp t);

/*
 * thing_magic.c
 */
void thing_server_magic_powerup(thingp t);
void thing_server_magic_fire(thingp t);
thingp thing_magic_anim(thingp t);
void thing_unwield_magic(thingp t);
void thing_wield_magic(thingp t, tpp magic);
widp thing_get_magic_anim_wid(thingp t);
void thing_set_magic_anim_id(thingp t, uint32_t magic_anim_id);
void thing_set_magic_anim(thingp t, thingp magic_anim);
void thing_magic_sheath(thingp t);
void thing_magic_tick(thingp t);

/*
 * thing_teleport.c
 */
void thing_reached_teleport(thingp t, thingp teleport);

/*
 * thing_exit.c
 */
void thing_reached_exit(thingp t, thingp exit);

/*
 * Only a certain resolution of thing can be represented on the client.
 * Convert a floating point value to client rounded value.
 */
static inline void thing_round (thingp t, double *x, double *y)
{
    const double scale = 256.0;

    *x = round(*x * scale) / scale;
    *y = round(*y * scale) / scale;
}

/*
 * round the thing coords and find out what floor tile we are really on.
 * we use the bottom of the tile as that is where the 'feet' commonly are
 */
static inline void thing_real_to_map (thingp t, int *x, int *y)
{
    *x = (t->x + 0.5);
    *y = (t->y + 0.9);
}

static inline void thing_real_to_fmap (thingp t, double *x, double *y)
{
    *x = (t->x + 0.5);
    *y = (t->y + 0.9);

    thing_round(t, x, y);
}

static inline void real_to_map (double ix, double iy, int *x, int *y)
{
    *x = (ix + 0.5);
    *y = (iy + 0.9);
}
