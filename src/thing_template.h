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
    uint32_t speed;

    /*
     * Vitality.
     */
    uint32_t health;

    /*
     * Lifespan in milliseconds.
     */
    uint32_t lifespan;

    /*
     * Various bounties.
     */
    uint32_t score_on_death;
    uint32_t score_on_collect;

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
    uint32_t ppp15;
    uint32_t ppp16;
    uint32_t ppp17;
    uint32_t ppp18;
    uint32_t ppp19;
    uint32_t ppp20;

    uint8_t is_exit:1;
    uint8_t is_floor:1;
    uint8_t is_food:1;
    uint8_t is_monst:1;
    uint8_t is_plant:1;
    uint8_t is_player:1;
    uint8_t is_key:1;
    uint8_t is_collision_map_small:1;
    uint8_t is_xxx3:1;
    uint8_t is_xxx4:1;
    uint8_t is_xxx5:1;
    uint8_t is_xxx6:1;
    uint8_t is_xxx7:1;
    uint8_t is_xxx8:1;
    uint8_t is_star:1;
    uint8_t is_key0:1;
    uint8_t is_key1:1;
    uint8_t is_key2:1;
    uint8_t is_key3:1;
    uint8_t is_key4:1;
    uint8_t is_key5:1;
    uint8_t is_key6:1;
    uint8_t is_key7:1;
    uint8_t is_key8:1;
    uint8_t is_key9:1;
    uint8_t is_xxx20:1;
    uint8_t is_xxx21:1;
    uint8_t is_seedpod:1;
    uint8_t is_bomb:1;
    uint8_t is_spam:1;
    uint8_t is_door:1;
    uint8_t is_pipe:1;
    uint8_t is_generator:1;
    uint8_t is_scarable:1;
    uint8_t is_xxx29:1;
    uint8_t is_hidden_from_editor:1;
    uint8_t is_animated:1;
    uint8_t is_xxx32:1;
    uint8_t is_xxx33:1;
    uint8_t is_left_as_corpse_on_death:1;
    uint8_t is_boring:1;
    uint8_t is_item_hidden:1;
    uint8_t is_joinable:1;
    uint8_t is_wall:1;
    uint8_t is_effect_sway:1;
    uint8_t is_effect_pulse:1;
    uint8_t is_effect_rotate_4way:1;
    uint8_t is_effect_rotate_2way:1;
    uint8_t dmap_valid:1;

    thing_tilep tilep_join[IS_JOIN_MAX][IS_JOIN_ALT_MAX];
    tilep tilep_join_tile[IS_JOIN_MAX][IS_JOIN_ALT_MAX];
    uint8_t tilep_join_count[IS_JOIN_MAX];

    /*
     * djkstra map
     */
    int8_t dmap[TILES_MAP_WIDTH][TILES_MAP_HEIGHT];
    uint32_t dmap_checksum;
} thing_template;

boolean thing_template_init(void);
void thing_template_fini(void);
thing_templatep thing_template_load(uint16_t *id, const char *file);
thing_templatep thing_template_find(const char *name);
thing_templatep thing_template_find_short_name(const char *name);
void demarshal_thing_template(demarshal_p ctx, thing_templatep);
void marshal_thing_template(marshal_p ctx, thing_templatep);
void thing_templates_marshal(marshal_p out);
boolean thing_test(int32_t argc, char *argv[]);
thing_templatep string2thing_template(const char **s);

const char *thing_template_name(thing_templatep);
const char *thing_template_short_name(thing_templatep);
const char *thing_template_get_tooltip(thing_templatep);
uint8_t thing_template_get_z_depth(thing_templatep);
uint8_t thing_template_get_z_order(thing_templatep);
uint32_t thing_template_get_speed(thing_templatep);
uint32_t thing_template_get_lifespan(thing_templatep);
uint32_t thing_template_get_health(thing_templatep);
uint32_t thing_template_get_score_on_death(thing_templatep);
uint32_t thing_template_get_score_on_collect(thing_templatep);
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
uint32_t thing_template_get_ppp15(thing_templatep);
uint32_t thing_template_get_ppp16(thing_templatep);
uint32_t thing_template_get_ppp17(thing_templatep);
uint32_t thing_template_get_ppp18(thing_templatep);
uint32_t thing_template_get_ppp19(thing_templatep);
uint32_t thing_template_get_ppp20(thing_templatep);

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

static inline boolean thing_template_is_exit (thing_templatep t)
{
    return (t->is_exit);
}

static inline boolean thing_template_is_floor (thing_templatep t)
{
    return (t->is_floor);
}

static inline boolean thing_template_is_food (thing_templatep t)
{
    return (t->is_food);
}

static inline boolean thing_template_is_monst (thing_templatep t)
{
    return (t->is_monst);
}

static inline boolean thing_template_is_plant (thing_templatep t)
{
    return (t->is_plant);
}

static inline boolean thing_template_is_player (thing_templatep t)
{
    return (t->is_player);
}

static inline boolean thing_template_is_key (thing_templatep t)
{
    return (t->is_key);
}

static inline boolean thing_template_is_collision_map_small (thing_templatep t)
{
    return (t->is_collision_map_small);
}

static inline boolean thing_template_is_xxx3 (thing_templatep t)
{
    return (t->is_xxx3);
}

static inline boolean thing_template_is_xxx4 (thing_templatep t)
{
    return (t->is_xxx4);
}

static inline boolean thing_template_is_xxx5 (thing_templatep t)
{
    return (t->is_xxx5);
}

static inline boolean thing_template_is_xxx6 (thing_templatep t)
{
    return (t->is_xxx6);
}

static inline boolean thing_template_is_xxx7 (thing_templatep t)
{
    return (t->is_xxx7);
}

static inline boolean thing_template_is_xxx8 (thing_templatep t)
{
    return (t->is_xxx8);
}

static inline boolean thing_template_is_star (thing_templatep t)
{
    return (t->is_star);
}

static inline boolean thing_template_is_key0 (thing_templatep t)
{
    return (t->is_key0);
}

static inline boolean thing_template_is_key1 (thing_templatep t)
{
    return (t->is_key1);
}

static inline boolean thing_template_is_key2 (thing_templatep t)
{
    return (t->is_key2);
}

static inline boolean thing_template_is_key3 (thing_templatep t)
{
    return (t->is_key3);
}

static inline boolean thing_template_is_key4 (thing_templatep t)
{
    return (t->is_key4);
}

static inline boolean thing_template_is_key5 (thing_templatep t)
{
    return (t->is_key5);
}

static inline boolean thing_template_is_key6 (thing_templatep t)
{
    return (t->is_key6);
}

static inline boolean thing_template_is_key7 (thing_templatep t)
{
    return (t->is_key7);
}

static inline boolean thing_template_is_key8 (thing_templatep t)
{
    return (t->is_key8);
}

static inline boolean thing_template_is_xxx20 (thing_templatep t)
{
    return (t->is_xxx20);
}

static inline boolean thing_template_is_key9 (thing_templatep t)
{
    return (t->is_key9);
}

static inline boolean thing_template_is_xxx21 (thing_templatep t)
{
    return (t->is_xxx21);
}

static inline boolean thing_template_is_seedpod (thing_templatep t)
{
    return (t->is_seedpod);
}

static inline boolean thing_template_is_bomb (thing_templatep t)
{
    return (t->is_bomb);
}

static inline boolean thing_template_is_spam (thing_templatep t)
{
    return (t->is_spam);
}

static inline boolean thing_template_is_door (thing_templatep t)
{
    return (t->is_door);
}

static inline boolean thing_template_is_pipe (thing_templatep t)
{
    return (t->is_pipe);
}

static inline boolean thing_template_is_generator (thing_templatep t)
{
    return (t->is_generator);
}

static inline boolean thing_template_is_scarable (thing_templatep t)
{
    return (t->is_scarable);
}

static inline boolean thing_template_is_xxx29 (thing_templatep t)
{
    return (t->is_xxx29);
}

static inline boolean thing_template_is_hidden_from_editor (thing_templatep t)
{
    return (t->is_hidden_from_editor);
}

static inline boolean thing_template_is_animated (thing_templatep t)
{
    return (t->is_animated);
}

static inline boolean thing_template_is_xxx32 (thing_templatep t)
{
    return (t->is_xxx32);
}

static inline boolean thing_template_is_xxx33 (thing_templatep t)
{
    return (t->is_xxx33);
}

static inline boolean thing_template_is_left_as_corpse_on_death (thing_templatep t)
{
    return (t->is_left_as_corpse_on_death);
}

static inline boolean thing_template_is_boring (thing_templatep t)
{
    return (t->is_boring);
}

static inline boolean thing_template_is_item_hidden (thing_templatep t)
{
    return (t->is_item_hidden);
}

static inline boolean thing_template_is_joinable (thing_templatep t)
{
    return (t->is_joinable);
}

static inline boolean thing_template_is_wall (thing_templatep t)
{
    return (t->is_wall);
}

static inline boolean thing_template_is_effect_sway (thing_templatep t)
{
    return (t->is_effect_sway);
}

static inline boolean thing_template_is_effect_pulse (thing_templatep t)
{
    return (t->is_effect_pulse);
}

static inline boolean thing_template_is_effect_rotate_4way (thing_templatep t)
{
    return (t->is_effect_rotate_4way);
}

static inline boolean thing_template_is_effect_rotate_2way (thing_templatep t)
{
    return (t->is_effect_rotate_2way);
}

