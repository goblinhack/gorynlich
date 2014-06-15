/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file.
 */

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

typedef struct thing_template_ {
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
    uint8_t carrying[THING_MAX];

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
     * Vitality.
     */
    int16_t health;

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
    int16_t bonus_health_on_use;

    uint32_t ppp1;
    uint32_t ppp2;
    uint32_t ppp3;
    uint32_t ppp4;
    uint32_t ppp5;
    uint32_t ppp6;
    uint32_t ppp7;
    uint32_t ppp8;
    uint32_t ppp9;
    uint32_t ppp10;
    uint32_t ppp11;
    uint32_t ppp12;
    uint32_t ppp13;
    uint32_t ppp14;
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
    uint8_t is_xxx3:1;
    uint8_t is_xxx4:1;
    uint8_t is_weapon:1;
    uint8_t is_treasure:1;
    uint8_t is_wearable:1;
    uint8_t is_fragile:1;
    uint8_t is_star:1;
    uint8_t is_animated_no_dir:1;
    uint8_t is_weapon_swing_effect:1;
    uint8_t is_key2:1;
    uint8_t is_key3:1;
    uint8_t is_fire:1;
    uint8_t is_ring:1;
    uint8_t is_animation:1;
    uint8_t is_xxx10:1;
    uint8_t is_poison:1;
    uint8_t is_carryable:1;
    uint8_t is_item_unusable:1;
    uint8_t is_valid_for_shortcut_key:1;
    uint8_t is_seedpod:1;
    uint8_t is_spam:1;
    uint8_t is_door:1;
    uint8_t is_pipe:1;
    uint8_t is_mob_spawner:1;
    uint8_t is_scarable:1;
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

uint8_t thing_template_init(void);
void thing_template_fini(void);
thing_templatep thing_template_load(uint16_t *id, const char *file);
thing_templatep thing_template_find(const char *name);
thing_templatep thing_template_find_short_name(const char *name);
void demarshal_thing_template(demarshal_p ctx, thing_templatep);
void marshal_thing_template(marshal_p ctx, thing_templatep);
void thing_templates_marshal(marshal_p out);
uint8_t thing_test(int32_t argc, char *argv[]);
thing_templatep string2thing_template(const char **s);

const char *thing_template_name(thing_templatep);
const char *thing_template_short_name(thing_templatep);
thing_templatep thing_template_fires(thing_templatep);
const char *thing_template_polymorph_on_death(thing_templatep);
const char *thing_template_spawn_on_death(thing_templatep);
const char *thing_template_weapon_carry_anim(thing_templatep);
const char *thing_template_weapon_swing_anim(thing_templatep);
const char *thing_template_message_on_use(thing_templatep);
const char *thing_template_mob_spawn(thing_templatep);
const char *thing_template_get_tooltip(thing_templatep);
uint8_t thing_template_get_z_depth(thing_templatep);
uint8_t thing_template_get_z_order(thing_templatep);
uint32_t thing_template_get_speed(thing_templatep);
uint32_t thing_template_get_lifespan(thing_templatep);
int16_t thing_template_get_health(thing_templatep);
uint32_t thing_template_get_damage(thing_templatep);
int32_t thing_template_get_bonus_score_on_death(thing_templatep);
uint32_t thing_template_get_vision_distance(thing_templatep);
int32_t thing_template_get_bonus_score_on_collect(thing_templatep);
uint32_t thing_template_get_ppp1(thing_templatep);
uint32_t thing_template_get_ppp2(thing_templatep);
uint32_t thing_template_get_ppp3(thing_templatep);
uint32_t thing_template_get_ppp4(thing_templatep);
uint32_t thing_template_get_ppp5(thing_templatep);
uint32_t thing_template_get_ppp6(thing_templatep);
uint32_t thing_template_get_ppp7(thing_templatep);
uint32_t thing_template_get_ppp8(thing_templatep);
uint32_t thing_template_get_ppp9(thing_templatep);
uint32_t thing_template_get_ppp10(thing_templatep);
uint32_t thing_template_get_ppp11(thing_templatep);
uint32_t thing_template_get_ppp12(thing_templatep);
uint32_t thing_template_get_ppp13(thing_templatep);
uint32_t thing_template_get_ppp14(thing_templatep);
int16_t thing_template_get_bonus_health_on_use(thing_templatep);
uint32_t thing_template_get_tx_map_update_delay_thousandths(thing_templatep);
uint32_t thing_template_get_can_be_hit_chance(thing_templatep);
uint32_t thing_template_get_failure_chance(thing_templatep);
uint32_t thing_template_get_hit_delay_tenths(thing_templatep);
uint32_t thing_template_get_mob_spawn_delay_tenths(thing_templatep);

tree_rootp thing_template_get_tiles(thing_templatep);
tree_rootp thing_template_get_tiles2(thing_templatep);

extern tree_rootp thing_templates;
extern tree_rootp thing_templates_create_order;

static inline int16_t thing_template_to_id (thing_templatep t) 
{
    return (t->id);
}

static inline thing_templatep id_to_thing_template (uint16_t id) 
{
    extern thing_template thing_templates_chunk[THING_MAX];

    if (id >= THING_MAX) {
        DIE("overflow, id %u", id);
    }

    return (&thing_templates_chunk[id]);
}

static inline uint8_t thing_template_is_exit (thing_templatep t)
{
    return (t->is_exit);
}

static inline uint8_t thing_template_is_floor (thing_templatep t)
{
    return (t->is_floor);
}

static inline uint8_t thing_template_is_food (thing_templatep t)
{
    return (t->is_food);
}

static inline uint8_t thing_template_is_monst (thing_templatep t)
{
    return (t->is_monst);
}

static inline uint8_t thing_template_is_plant (thing_templatep t)
{
    return (t->is_plant);
}

static inline uint8_t thing_template_is_player (thing_templatep t)
{
    return (t->is_player);
}

static inline uint8_t thing_template_is_key (thing_templatep t)
{
    return (t->is_key);
}

static inline uint8_t thing_template_is_xxx3 (thing_templatep t)
{
    return (t->is_xxx3);
}

static inline uint8_t thing_template_is_xxx4 (thing_templatep t)
{
    return (t->is_xxx4);
}

static inline uint8_t thing_template_is_weapon (thing_templatep t)
{
    return (t->is_weapon);
}

static inline uint8_t thing_template_is_treasure (thing_templatep t)
{
    return (t->is_treasure);
}

static inline uint8_t thing_template_is_wearable (thing_templatep t)
{
    return (t->is_wearable);
}

static inline uint8_t thing_template_is_fragile (thing_templatep t)
{
    return (t->is_fragile);
}

static inline uint8_t thing_template_is_star (thing_templatep t)
{
    return (t->is_star);
}

static inline uint8_t thing_template_is_animated_no_dir (thing_templatep t)
{
    return (t->is_animated_no_dir);
}

static inline uint8_t thing_template_is_weapon_swing_effect (thing_templatep t)
{
    return (t->is_weapon_swing_effect);
}

static inline uint8_t thing_template_is_key2 (thing_templatep t)
{
    return (t->is_key2);
}

static inline uint8_t thing_template_is_key3 (thing_templatep t)
{
    return (t->is_key3);
}

static inline uint8_t thing_template_is_fire (thing_templatep t)
{
    return (t->is_fire);
}

static inline uint8_t thing_template_is_ring (thing_templatep t)
{
    return (t->is_ring);
}

static inline uint8_t thing_template_is_animation (thing_templatep t)
{
    return (t->is_animation);
}

static inline uint8_t thing_template_is_xxx10 (thing_templatep t)
{
    return (t->is_xxx10);
}

static inline uint8_t thing_template_is_collision_map_large (thing_templatep t)
{
    return (t->is_collision_map_large);
}

static inline uint8_t thing_template_is_poison (thing_templatep t)
{
    return (t->is_poison);
}

static inline uint8_t thing_template_is_item_unusable (thing_templatep t)
{
    return (t->is_item_unusable);
}

static inline uint8_t thing_template_is_carryable (thing_templatep t)
{
    return (t->is_carryable);
}

static inline uint8_t thing_template_is_valid_for_shortcut_key (thing_templatep t)
{
    return (t->is_valid_for_shortcut_key);
}

static inline uint8_t thing_template_is_seedpod (thing_templatep t)
{
    return (t->is_seedpod);
}

static inline uint8_t thing_template_is_spam (thing_templatep t)
{
    return (t->is_spam);
}

static inline uint8_t thing_template_is_door (thing_templatep t)
{
    return (t->is_door);
}

static inline uint8_t thing_template_is_pipe (thing_templatep t)
{
    return (t->is_pipe);
}

static inline uint8_t thing_template_is_mob_spawner (thing_templatep t)
{
    return (t->is_mob_spawner);
}

static inline uint8_t thing_template_is_scarable (thing_templatep t)
{
    return (t->is_scarable);
}

static inline uint8_t thing_template_is_explosion (thing_templatep t)
{
    return (t->is_explosion);
}

static inline uint8_t thing_template_is_hidden_from_editor (thing_templatep t)
{
    return (t->is_hidden_from_editor);
}

static inline uint8_t thing_template_is_animated (thing_templatep t)
{
    return (t->is_animated);
}

static inline uint8_t thing_template_is_collision_map_tiny (thing_templatep t)
{
    return (t->is_collision_map_tiny);
}

static inline uint8_t thing_template_is_collision_map_medium (thing_templatep t)
{
    return (t->is_collision_map_medium);
}

static inline uint8_t thing_template_is_collision_map_small (thing_templatep t)
{
    return (t->is_collision_map_small);
}

static inline uint8_t thing_template_is_combustable (thing_templatep t)
{
    return (t->is_combustable);
}

static inline uint8_t thing_template_is_projectile (thing_templatep t)
{
    return (t->is_projectile);
}

static inline uint8_t thing_template_is_boring (thing_templatep t)
{
    return (t->is_boring);
}

static inline uint8_t thing_template_is_joinable (thing_templatep t)
{
    return (t->is_joinable);
}

static inline uint8_t thing_template_is_wall (thing_templatep t)
{
    return (t->is_wall);
}

static inline uint8_t thing_template_is_effect_sway (thing_templatep t)
{
    return (t->is_effect_sway);
}

static inline uint8_t thing_template_is_effect_pulse (thing_templatep t)
{
    return (t->is_effect_pulse);
}

static inline uint8_t thing_template_is_effect_rotate_4way (thing_templatep t)
{
    return (t->is_effect_rotate_4way);
}

static inline uint8_t thing_template_is_effect_rotate_2way (thing_templatep t)
{
    return (t->is_effect_rotate_2way);
}

