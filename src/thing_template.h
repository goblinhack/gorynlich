/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file.
 */

#pragma once

#define IS_JOIN_ALT_MAX 10

enum {
    IS_JOIN_BLOCK,
    IS_JOIN_HORIZ,
    IS_JOIN_VERT,
    IS_JOIN_NODE,
    IS_JOIN_LEFT,
    IS_JOIN_RIGHT,
    IS_JOIN_TOP,
    IS_JOIN_BOT,
    IS_JOIN_TL,
    IS_JOIN_TR,
    IS_JOIN_BL,
    IS_JOIN_BR,
    IS_JOIN_T,
    IS_JOIN_T90,
    IS_JOIN_T180,
    IS_JOIN_T270,
    IS_JOIN_X,
    IS_JOIN_TL2,
    IS_JOIN_TR2,
    IS_JOIN_BL2,
    IS_JOIN_BR2,
    IS_JOIN_T_1,
    IS_JOIN_T_2,
    IS_JOIN_T_3,
    IS_JOIN_T90_1,
    IS_JOIN_T90_2,
    IS_JOIN_T90_3,
    IS_JOIN_T180_1,
    IS_JOIN_T180_2,
    IS_JOIN_T180_3,
    IS_JOIN_T270_1,
    IS_JOIN_T270_2,
    IS_JOIN_T270_3,
    IS_JOIN_X1,
    IS_JOIN_X1_270,
    IS_JOIN_X1_180,
    IS_JOIN_X1_90,
    IS_JOIN_X2,
    IS_JOIN_X2_270,
    IS_JOIN_X2_180,
    IS_JOIN_X2_90,
    IS_JOIN_X3,
    IS_JOIN_X3_180,
    IS_JOIN_X4,
    IS_JOIN_X4_270,
    IS_JOIN_X4_180,
    IS_JOIN_X4_90,
    IS_JOIN_VERT2,
    IS_JOIN_HORIZ2,
    IS_JOIN_MAX,
};

typedef struct tp_ {
    tree_key_string tree;
    tree_key_int tree2;

    uint16_t id;

    /*
     * Internal description of the thing.
     */
    char *short_name;

    /*
     * End user description of the thing.
     */
    char *tooltip;

    /*
     * What the thing throws.
     */
    thing_templatep fires;

    /*
     * What to create when you die. Usually a smaller monster.
     */
    char *polymorph_on_death;

    /*
     * When you collect a bundle of items it as carried as x of this.
     */
    char *carried_as;

    /*
     * What shade of light does this thing emit
     */
    char *light_tint;
    color light_color;

    /*
     * What to spawn when you die. Usually a generator or grave.
     */
    char *spawn_on_death;

    /*
     * How this weapon appears on screen when it is used.
     */
    char *weapon_carry_anim;

    char *weapon_swing_anim;

    /*
     * What to shout when an item is used.
     */
    char *message_on_use;

    /*
     * Periodically spawn something.
     */
    char *mob_spawn;

    /*
     * How many and of what we are carrying. This is the base items a thing
     * starts out with.
     */
    item_t base_items[THING_MAX];

    /*
     * In relation to other widgets, where are we.
     */
    uint8_t z_depth;
    uint8_t z_order;

    /*
     * Animation tiles.
     */
    tree_rootp tiles;
    tree_rootp tiles2;

    /*
     * Speed in milliseconds it takes to move one tile.
     */
    uint16_t speed;

    /*
     * Damage on hits.
     */
    uint16_t damage;

    /*
     * Lifespan in milliseconds.
     */
    uint16_t lifespan;

    /*
     * How far in tiles the thing can detect the monster.
     */
    uint8_t vision_distance;

    /*
     * Various bounties.
     */
    int32_t bonus_score_on_death;
    int32_t bonus_score_on_collect;
    int16_t bonus_hp_on_use;
    int16_t bonus_id_on_use;

    /*
     * Max hp, attack bonuses etc... Is overriden by thing values that can 
     * change. This is the base value however.
     */
    player_stats_t stats;

    uint32_t chance_of_appearing;
    uint32_t ppp2;
    uint32_t hp_per_level;
    uint32_t id_per_level;
    float light_radius;
    uint32_t quantity;
    uint32_t hit_priority;
    uint32_t weapon_fire_delay_tenths;
    uint32_t swing_distance_from_player;
    uint32_t tx_map_update_delay_thousandths;
    uint32_t can_be_hit_chance;
    uint32_t failure_chance;
    uint32_t hit_delay_tenths;
    uint32_t mob_spawn_delay_tenths;

    uint8_t is_exit:1;
    uint8_t is_floor:1;
    uint8_t is_food:1;
    uint8_t is_monst:1;
    uint8_t is_plant:1;
    uint8_t is_player:1;
    uint8_t is_key:1;
    uint8_t is_rock:1;
    uint8_t is_shadow_caster:1;
    uint8_t is_weapon:1;
    uint8_t is_treasure:1;
    uint8_t is_wearable:1;
    uint8_t is_fragile:1;
    uint8_t is_star:1;
    uint8_t is_animated_no_dir:1;
    uint8_t is_weapon_swing_effect:1;
    uint8_t is_light_source:1;
    uint8_t is_candle_light:1;
    uint8_t is_cats_eyes:1;
    uint8_t is_fire:1;
    uint8_t is_ring:1;
    uint8_t is_animation:1;
    uint8_t is_shadow_caster_soft:1;
    uint8_t is_poison:1;
    uint8_t is_carryable:1;
    uint8_t is_item_unusable:1;
    uint8_t is_valid_for_action_bar:1;
    uint8_t is_seedpod:1;
    uint8_t is_spam:1;
    uint8_t is_door:1;
    uint8_t is_pipe:1;
    uint8_t is_mob_spawner:1;
    uint8_t is_rrr1:1;
    uint8_t is_rrr2:1;
    uint8_t is_rrr3:1;
    uint8_t is_rrr4:1;
    uint8_t is_rrr5:1;
    uint8_t is_rrr6:1;
    uint8_t is_rrr7:1;
    uint8_t is_hand_item:1;
    uint8_t is_boots:1;
    uint8_t is_helmet:1;
    uint8_t is_armor:1;
    uint8_t is_given_randomly_at_start:1;
    uint8_t is_magical:1;
    uint8_t is_degradable:1;
    uint8_t is_cursed:1;
    uint8_t is_animate_only_when_moving:1;
    uint8_t is_warm_blooded:1;
    uint8_t is_rrr23:1;
    uint8_t is_stackable:1;
    uint8_t is_torch:1;
    uint8_t is_explosion:1;
    uint8_t is_hidden_from_editor:1;
    uint8_t is_animated:1;
    uint8_t is_collision_map_large:1;
    uint8_t is_collision_map_small:1;
    uint8_t is_collision_map_medium:1;
    uint8_t is_collision_map_tiny:1;
    uint8_t is_combustable:1;
    uint8_t is_projectile:1;
    uint8_t is_boring:1;
    uint8_t is_joinable:1;
    uint8_t is_wall:1;
    uint8_t is_effect_sway:1;
    uint8_t is_effect_pulse:1;
    uint8_t is_effect_rotate_4way:1;
    uint8_t is_effect_rotate_2way:1;

    thing_tilep tilep_join[IS_JOIN_MAX][IS_JOIN_ALT_MAX];
    tilep tilep_join_tile[IS_JOIN_MAX][IS_JOIN_ALT_MAX];
    uint8_t tilep_join_count[IS_JOIN_MAX];
} thing_template;

uint8_t tp_init(void);
void tp_fini(void);
thing_templatep tp_load(uint16_t *id, const char *file);
thing_templatep tp_find(const char *name);
thing_templatep tp_find_short_name(const char *name);
void demarshal_thing_template(demarshal_p ctx, thing_templatep);
void marshal_thing_template(marshal_p ctx, thing_templatep);
void thing_templates_marshal(marshal_p out);
uint8_t thing_test(int32_t argc, char *argv[]);
thing_templatep string2thing_template(const char **s);

const char *tp_name(thing_templatep);
const char *tp_short_name(thing_templatep);
thing_templatep tp_fires(thing_templatep);
const char *tp_polymorph_on_death(thing_templatep);
const char *tp_carried_as(thing_templatep);
const char *tp_light_tint(thing_templatep);
color tp_light_color(thing_templatep);
const char *tp_spawn_on_death(thing_templatep);
const char *tp_weapon_carry_anim(thing_templatep);
const char *tp_weapon_swing_anim(thing_templatep);
const char *tp_message_on_use(thing_templatep);
const char *tp_mob_spawn(thing_templatep);
const char *tp_get_tooltip(thing_templatep);
uint8_t tp_get_z_depth(thing_templatep);
uint8_t tp_get_z_order(thing_templatep);
uint32_t tp_get_speed(thing_templatep);
uint32_t tp_get_lifespan(thing_templatep);
uint16_t tp_get_damage(thing_templatep);
int32_t tp_get_bonus_score_on_death(thing_templatep);
uint32_t tp_get_vision_distance(thing_templatep);
int32_t tp_get_bonus_score_on_collect(thing_templatep);

uint32_t tp_get_stats_attack_melee(thing_templatep);
uint32_t tp_get_stats_attack_ranged(thing_templatep);
uint32_t tp_get_stats_attack_magical(thing_templatep);
uint32_t tp_get_stats_speed(thing_templatep);
uint32_t tp_get_stats_vision(thing_templatep);
uint32_t tp_get_stats_healing(thing_templatep);
uint32_t tp_get_stats_defense(thing_templatep);

uint32_t tp_get_chance_of_appearing(thing_templatep);
uint32_t tp_get_ppp2(thing_templatep);
float tp_get_light_radius(thing_templatep);
uint32_t tp_get_quantity(thing_templatep);
uint32_t tp_get_hit_priority(thing_templatep);
uint32_t tp_get_weapon_fire_delay_tenths(thing_templatep);
uint32_t tp_get_swing_distance_from_player(thing_templatep);

int16_t tp_get_stats_max_hp(thing_templatep);
uint32_t tp_get_hp_per_level(thing_templatep);
int16_t tp_get_bonus_hp_on_use(thing_templatep);

int16_t tp_get_stats_max_id(thing_templatep);
uint32_t tp_get_id_per_level(thing_templatep);
int16_t tp_get_bonus_id_on_use(thing_templatep);

uint32_t tp_get_tx_map_update_delay_thousandths(thing_templatep);
uint32_t tp_get_can_be_hit_chance(thing_templatep);
uint32_t tp_get_failure_chance(thing_templatep);
uint32_t tp_get_hit_delay_tenths(thing_templatep);
uint32_t tp_get_mob_spawn_delay_tenths(thing_templatep);

tree_rootp tp_get_tiles(thing_templatep);
tree_rootp tp_get_tiles2(thing_templatep);

extern tree_rootp thing_templates;
extern tree_rootp thing_templates_create_order;

static inline int16_t tp_to_id (thing_templatep t) 
{
    return (t->id);
}

static inline thing_templatep id_to_tp (uint16_t id) 
{
    extern thing_template thing_templates_chunk[THING_MAX];

    if (id >= THING_MAX) {
        DIE("overflow, bad template id %u", id);
    }

    return (&thing_templates_chunk[id]);
}

static inline uint8_t tp_is_exit (thing_templatep t)
{
    return (t->is_exit);
}

static inline uint8_t tp_is_floor (thing_templatep t)
{
    return (t->is_floor);
}

static inline uint8_t tp_is_food (thing_templatep t)
{
    return (t->is_food);
}

static inline uint8_t tp_is_monst (thing_templatep t)
{
    return (t->is_monst);
}

static inline uint8_t tp_is_plant (thing_templatep t)
{
    return (t->is_plant);
}

static inline uint8_t tp_is_player (thing_templatep t)
{
    return (t->is_player);
}

static inline uint8_t tp_is_key (thing_templatep t)
{
    return (t->is_key);
}

static inline uint8_t tp_is_rock (thing_templatep t)
{
    return (t->is_rock);
}

static inline uint8_t tp_is_shadow_caster (thing_templatep t)
{
    return (t->is_shadow_caster);
}

static inline uint8_t tp_is_weapon (thing_templatep t)
{
    return (t->is_weapon);
}

static inline uint8_t tp_is_treasure (thing_templatep t)
{
    return (t->is_treasure);
}

static inline uint8_t tp_is_wearable (thing_templatep t)
{
    return (t->is_wearable);
}

static inline uint8_t tp_is_fragile (thing_templatep t)
{
    return (t->is_fragile);
}

static inline uint8_t tp_is_star (thing_templatep t)
{
    return (t->is_star);
}

static inline uint8_t tp_is_animated_no_dir (thing_templatep t)
{
    return (t->is_animated_no_dir);
}

static inline uint8_t tp_is_weapon_swing_effect (thing_templatep t)
{
    return (t->is_weapon_swing_effect);
}

static inline uint8_t tp_is_light_source (thing_templatep t)
{
    return (t->is_light_source);
}

static inline uint8_t tp_is_candle_light (thing_templatep t)
{
    return (t->is_candle_light);
}

static inline uint8_t tp_is_cats_eyes (thing_templatep t)
{
    return (t->is_cats_eyes);
}

static inline uint8_t tp_is_fire (thing_templatep t)
{
    return (t->is_fire);
}

static inline uint8_t tp_is_ring (thing_templatep t)
{
    return (t->is_ring);
}

static inline uint8_t tp_is_animation (thing_templatep t)
{
    return (t->is_animation);
}

static inline uint8_t tp_is_shadow_caster_soft (thing_templatep t)
{
    return (t->is_shadow_caster_soft);
}

static inline uint8_t tp_is_collision_map_large (thing_templatep t)
{
    return (t->is_collision_map_large);
}

static inline uint8_t tp_is_poison (thing_templatep t)
{
    return (t->is_poison);
}

static inline uint8_t tp_is_item_unusable (thing_templatep t)
{
    return (t->is_item_unusable);
}

static inline uint8_t tp_is_carryable (thing_templatep t)
{
    return (t->is_carryable);
}

static inline uint8_t tp_is_valid_for_action_bar (thing_templatep t)
{
    return (t->is_valid_for_action_bar);
}

static inline uint8_t tp_is_seedpod (thing_templatep t)
{
    return (t->is_seedpod);
}

static inline uint8_t tp_is_spam (thing_templatep t)
{
    return (t->is_spam);
}

static inline uint8_t tp_is_door (thing_templatep t)
{
    return (t->is_door);
}

static inline uint8_t tp_is_pipe (thing_templatep t)
{
    return (t->is_pipe);
}

static inline uint8_t tp_is_mob_spawner (thing_templatep t)
{
    return (t->is_mob_spawner);
}

static inline uint8_t tp_is_rrr1 (thing_templatep t)
{
    return (t->is_rrr1);
}

static inline uint8_t tp_is_rrr2 (thing_templatep t)
{
    return (t->is_rrr2);
}

static inline uint8_t tp_is_rrr3 (thing_templatep t)
{
    return (t->is_rrr3);
}

static inline uint8_t tp_is_rrr4 (thing_templatep t)
{
    return (t->is_rrr4);
}

static inline uint8_t tp_is_rrr5 (thing_templatep t)
{
    return (t->is_rrr5);
}

static inline uint8_t tp_is_rrr6 (thing_templatep t)
{
    return (t->is_rrr6);
}

static inline uint8_t tp_is_rrr7 (thing_templatep t)
{
    return (t->is_rrr7);
}

static inline uint8_t tp_is_hand_item (thing_templatep t)
{
    return (t->is_hand_item);
}

static inline uint8_t tp_is_boots (thing_templatep t)
{
    return (t->is_boots);
}

static inline uint8_t tp_is_helmet (thing_templatep t)
{
    return (t->is_helmet);
}

static inline uint8_t tp_is_armor (thing_templatep t)
{
    return (t->is_armor);
}

static inline uint8_t tp_is_given_randomly_at_start (thing_templatep t)
{
    return (t->is_given_randomly_at_start);
}

static inline uint8_t tp_is_magical (thing_templatep t)
{
    return (t->is_magical);
}

static inline uint8_t tp_is_degradable (thing_templatep t)
{
    return (t->is_degradable);
}

static inline uint8_t tp_is_cursed (thing_templatep t)
{
    return (t->is_cursed);
}

static inline uint8_t tp_is_animate_only_when_moving (thing_templatep t)
{
    return (t->is_animate_only_when_moving);
}

static inline uint8_t tp_is_warm_blooded (thing_templatep t)
{
    return (t->is_warm_blooded);
}

static inline uint8_t tp_is_rrr23 (thing_templatep t)
{
    return (t->is_rrr23);
}

static inline uint8_t tp_is_stackable (thing_templatep t)
{
    return (t->is_stackable);
}

static inline uint8_t tp_is_torch (thing_templatep t)
{
    return (t->is_torch);
}

static inline uint8_t tp_is_explosion (thing_templatep t)
{
    return (t->is_explosion);
}

static inline uint8_t tp_is_hidden_from_editor (thing_templatep t)
{
    return (t->is_hidden_from_editor);
}

static inline uint8_t tp_is_animated (thing_templatep t)
{
    return (t->is_animated);
}

static inline uint8_t tp_is_collision_map_tiny (thing_templatep t)
{
    return (t->is_collision_map_tiny);
}

static inline uint8_t tp_is_collision_map_medium (thing_templatep t)
{
    return (t->is_collision_map_medium);
}

static inline uint8_t tp_is_collision_map_small (thing_templatep t)
{
    return (t->is_collision_map_small);
}

static inline uint8_t tp_is_combustable (thing_templatep t)
{
    return (t->is_combustable);
}

static inline uint8_t tp_is_projectile (thing_templatep t)
{
    return (t->is_projectile);
}

static inline uint8_t tp_is_boring (thing_templatep t)
{
    return (t->is_boring);
}

static inline uint8_t tp_is_joinable (thing_templatep t)
{
    return (t->is_joinable);
}

static inline uint8_t tp_is_wall (thing_templatep t)
{
    return (t->is_wall);
}

static inline uint8_t tp_is_effect_sway (thing_templatep t)
{
    return (t->is_effect_sway);
}

static inline uint8_t tp_is_effect_pulse (thing_templatep t)
{
    return (t->is_effect_pulse);
}

static inline uint8_t tp_is_effect_rotate_4way (thing_templatep t)
{
    return (t->is_effect_rotate_4way);
}

static inline uint8_t tp_is_effect_rotate_2way (thing_templatep t)
{
    return (t->is_effect_rotate_2way);
}

