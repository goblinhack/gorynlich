/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file.
 */

#include "tree.h"

#define IS_JOIN_ALT_MAX 10
#define THING_TEMPLATES_CHUNK_COUNT_MAX 1024

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
    uint8_t is_xxx1:1;
    uint8_t is_xxx2:1;
    uint8_t is_xxx3:1;
    uint8_t is_xxx4:1;
    uint8_t is_xxx5:1;
    uint8_t is_xxx6:1;
    uint8_t is_xxx7:1;
    uint8_t is_xxx8:1;
    uint8_t is_star:1;
    uint8_t is_xxx10:1;
    uint8_t is_xxx11:1;
    uint8_t is_xxx12:1;
    uint8_t is_xxx13:1;
    uint8_t is_xxx14:1;
    uint8_t is_xxx15:1;
    uint8_t is_xxx16:1;
    uint8_t is_xxx17:1;
    uint8_t is_xxx18:1;
    uint8_t is_xxx19:1;
    uint8_t is_xxx20:1;
    uint8_t is_xxx21:1;
    uint8_t is_seedpod:1;
    uint8_t is_bomb:1;
    uint8_t is_spam:1;
    uint8_t is_door:1;
    uint8_t is_pipe:1;
    uint8_t is_xxx27:1;
    uint8_t is_scarable:1;
    uint8_t is_xxx29:1;
    uint8_t is_hidden_from_editor:1;
    uint8_t is_animated:1;
    uint8_t is_xxx32:1;
    uint8_t is_xxx33:1;
    uint8_t is_left_as_corpse_on_death:1;
    uint8_t is_xxx35:1;
    uint8_t is_item_hidden:1;
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

boolean thing_template_init(void);
void thing_template_fini(void);
thing_templatep thing_template_load(const char *file);
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

boolean thing_template_is_player(thing_templatep);
boolean thing_template_is_monst(thing_templatep);
boolean thing_template_is_wall(thing_templatep);
boolean thing_template_is_food(thing_templatep);
boolean thing_template_is_floor(thing_templatep);
boolean thing_template_is_exit(thing_templatep);
boolean thing_template_is_xxx1(thing_templatep);
boolean thing_template_is_xxx2(thing_templatep);
boolean thing_template_is_xxx3(thing_templatep);
boolean thing_template_is_xxx4(thing_templatep);
boolean thing_template_is_xxx5(thing_templatep);
boolean thing_template_is_xxx6(thing_templatep);
boolean thing_template_is_xxx7(thing_templatep);
boolean thing_template_is_xxx8(thing_templatep);
boolean thing_template_is_star(thing_templatep);
boolean thing_template_is_xxx10(thing_templatep);
boolean thing_template_is_xxx11(thing_templatep);
boolean thing_template_is_xxx12(thing_templatep);
boolean thing_template_is_xxx13(thing_templatep);
boolean thing_template_is_xxx14(thing_templatep);
boolean thing_template_is_xxx15(thing_templatep);
boolean thing_template_is_xxx16(thing_templatep);
boolean thing_template_is_xxx17(thing_templatep);
boolean thing_template_is_xxx18(thing_templatep);
boolean thing_template_is_xxx20(thing_templatep);
boolean thing_template_is_xxx19(thing_templatep);
boolean thing_template_is_xxx21(thing_templatep);
boolean thing_template_is_seedpod(thing_templatep);
boolean thing_template_is_bomb(thing_templatep);
boolean thing_template_is_spam(thing_templatep);
boolean thing_template_is_door(thing_templatep);
boolean thing_template_is_pipe(thing_templatep);
boolean thing_template_is_xxx27(thing_templatep);
boolean thing_template_is_scarable(thing_templatep);
boolean thing_template_is_xxx29(thing_templatep);
boolean thing_template_is_hidden_from_editor(thing_templatep);
boolean thing_template_is_animated(thing_templatep);
boolean thing_template_is_xxx32(thing_templatep);
boolean thing_template_is_xxx33(thing_templatep);
boolean thing_template_is_left_as_corpse_on_death(thing_templatep);
boolean thing_template_is_xxx35(thing_templatep);
boolean thing_template_is_item_hidden(thing_templatep);
boolean thing_template_is_plant(thing_templatep);
boolean thing_template_is_joinable(thing_templatep);
boolean thing_template_is_effect_sway(thing_templatep);
boolean thing_template_is_effect_pulse(thing_templatep);
boolean thing_template_is_effect_rotate_4way(thing_templatep);
boolean thing_template_is_effect_rotate_2way(thing_templatep);

int16_t thing_template_path_cost_is_exit(thing_templatep);
int16_t thing_template_path_cost_is_floor(thing_templatep);
int16_t thing_template_path_cost_is_food(thing_templatep);
int16_t thing_template_path_cost_is_monst(thing_templatep);
int16_t thing_template_path_cost_is_plant(thing_templatep);
int16_t thing_template_path_cost_is_player(thing_templatep);
int16_t thing_template_path_cost_is_xxx1(thing_templatep);
int16_t thing_template_path_cost_is_xxx2(thing_templatep);
int16_t thing_template_path_cost_is_xxx3(thing_templatep);
int16_t thing_template_path_cost_is_xxx4(thing_templatep);
int16_t thing_template_path_cost_is_xxx5(thing_templatep);
int16_t thing_template_path_cost_is_xxx6(thing_templatep);
int16_t thing_template_path_cost_is_xxx7(thing_templatep);
int16_t thing_template_path_cost_is_xxx8(thing_templatep);
int16_t thing_template_path_cost_is_star(thing_templatep);
int16_t thing_template_path_cost_is_xxx10(thing_templatep);
int16_t thing_template_path_cost_is_xxx11(thing_templatep);
int16_t thing_template_path_cost_is_xxx12(thing_templatep);
int16_t thing_template_path_cost_is_xxx13(thing_templatep);
int16_t thing_template_path_cost_is_xxx14(thing_templatep);
int16_t thing_template_path_cost_is_xxx15(thing_templatep);
int16_t thing_template_path_cost_is_xxx16(thing_templatep);
int16_t thing_template_path_cost_is_xxx17(thing_templatep);
int16_t thing_template_path_cost_is_xxx18(thing_templatep);
int16_t thing_template_path_cost_is_xxx20(thing_templatep);
int16_t thing_template_path_cost_is_xxx19(thing_templatep);
int16_t thing_template_path_cost_is_xxx21(thing_templatep);
int16_t thing_template_path_cost_is_seedpod(thing_templatep);
int16_t thing_template_path_cost_is_bomb(thing_templatep);
int16_t thing_template_path_cost_is_spam(thing_templatep);
int16_t thing_template_path_cost_is_door(thing_templatep);
int16_t thing_template_path_cost_is_pipe(thing_templatep);
int16_t thing_template_path_cost_is_xxx27(thing_templatep);
int16_t thing_template_path_cost_is_scarable(thing_templatep);
int16_t thing_template_path_cost_is_xxx29(thing_templatep);
int16_t thing_template_path_cost_is_hidden_from_editor(thing_templatep);
int16_t thing_template_path_cost_is_animated(thing_templatep);
int16_t thing_template_path_cost_is_xxx32(thing_templatep);
int16_t thing_template_path_cost_is_xxx33(thing_templatep);
int16_t thing_template_path_cost_is_left_as_corpse_on_death(thing_templatep);
int16_t thing_template_path_cost_is_xxx35(thing_templatep);
int16_t thing_template_path_cost_is_item_hidden(thing_templatep);
int16_t thing_template_path_cost_is_thing(thing_templatep);
int16_t thing_template_path_cost_is_joinable(thing_templatep);
int16_t thing_template_path_cost_is_wall(thing_templatep);
int16_t thing_template_path_cost_is_effect_sway(thing_templatep);
int16_t thing_template_path_cost_is_effect_pulse(thing_templatep);
int16_t thing_template_path_cost_is_effect_rotate_4way(thing_templatep);
int16_t thing_template_path_cost_is_effect_rotate_2way(thing_templatep);

tree_rootp thing_template_get_tiles(thing_templatep);
tree_rootp thing_template_get_tiles2(thing_templatep);

extern tree_rootp thing_templates;
extern tree_rootp thing_templates_create_order;

extern thing_template thing_templates_chunk[THING_TEMPLATES_CHUNK_COUNT_MAX];

static inline int16_t thing_template_to_id (thing_templatep t) 
{
    return (t->id);
}

static inline thing_templatep id_to_thing_template (uint16_t id) 
{
    return (&thing_templates_chunk[id]);
}
