/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "tree.h"
#include "thing_tile_private.h"
#include "thing_tile.h"
#include "tile.h"
#include "thing.h"
#include "marshal.h"

/*
 * Using static memory as these things never change once made.
 */
thing_template thing_templates_chunk[THING_MAX];
static uint32_t thing_templates_chunk_count;

tree_root *thing_templates;
tree_root *thing_templates_create_order;

static uint8_t thing_template_init_done;
static void thing_template_destroy_internal(thing_templatep t);

uint8_t thing_template_init (void)
{
    thing_template_init_done = true;

    return (true);
}

void thing_template_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (thing_template_init_done) {
        thing_template_init_done = false;

        tree_destroy(&thing_templates,
            (tree_destroy_func)thing_template_destroy_internal);

        tree_destroy(&thing_templates_create_order,
            (tree_destroy_func)0);
    }
}

/*
 * Fill up the tile cache for future fast finds of tile types.
 */
static void thing_template_fill_cache (thing_templatep t)
{
    thing_tilep thing_tile;
    uint32_t index;
    tilep tile;

    TREE_WALK_UNSAFE(t->tiles, thing_tile) {

        if (thing_tile->is_join_block) {
            index = IS_JOIN_BLOCK;
        } else if (thing_tile->is_join_t_1) {
            index = IS_JOIN_T_1;
        } else if (thing_tile->is_join_t_2) {
            index = IS_JOIN_T_2;
        } else if (thing_tile->is_join_t_3) {
            index = IS_JOIN_T_3;
        } else if (thing_tile->is_join_t90_1) {
            index = IS_JOIN_T90_1;
        } else if (thing_tile->is_join_t90_2) {
            index = IS_JOIN_T90_2;
        } else if (thing_tile->is_join_t90_3) {
            index = IS_JOIN_T90_3;
        } else if (thing_tile->is_join_t180_1) {
            index = IS_JOIN_T180_1;
        } else if (thing_tile->is_join_t180_2) {
            index = IS_JOIN_T180_2;
        } else if (thing_tile->is_join_t180_3) {
            index = IS_JOIN_T180_3;
        } else if (thing_tile->is_join_t270_1) {
            index = IS_JOIN_T270_1;
        } else if (thing_tile->is_join_t270_2) {
            index = IS_JOIN_T270_2;
        } else if (thing_tile->is_join_t270_3) {
            index = IS_JOIN_T270_3;
        } else if (thing_tile->is_join_block) {
            index = IS_JOIN_BLOCK;
        } else if (thing_tile->is_join_horiz) {
            index = IS_JOIN_HORIZ;
        } else if (thing_tile->is_join_vert) {
            index = IS_JOIN_VERT;
        } else if (thing_tile->is_join_node) {
            index = IS_JOIN_NODE;
        } else if (thing_tile->is_join_left) {
            index = IS_JOIN_LEFT;
        } else if (thing_tile->is_join_right) {
            index = IS_JOIN_RIGHT;
        } else if (thing_tile->is_join_top) {
            index = IS_JOIN_TOP;
        } else if (thing_tile->is_join_bot) {
            index = IS_JOIN_BOT;
        } else if (thing_tile->is_join_tl) {
            index = IS_JOIN_TL;
        } else if (thing_tile->is_join_tr) {
            index = IS_JOIN_TR;
        } else if (thing_tile->is_join_bl) {
            index = IS_JOIN_BL;
        } else if (thing_tile->is_join_br) {
            index = IS_JOIN_BR;
        } else if (thing_tile->is_join_t) {
            index = IS_JOIN_T;
        } else if (thing_tile->is_join_t90) {
            index = IS_JOIN_T90;
        } else if (thing_tile->is_join_t180) {
            index = IS_JOIN_T180;
        } else if (thing_tile->is_join_t270) {
            index = IS_JOIN_T270;
        } else if (thing_tile->is_join_x) {
            index = IS_JOIN_X;
        } else if (thing_tile->is_join_tl2) {
            index = IS_JOIN_TL2;
        } else if (thing_tile->is_join_tr2) {
            index = IS_JOIN_TR2;
        } else if (thing_tile->is_join_bl2) {
            index = IS_JOIN_BL2;
        } else if (thing_tile->is_join_br2) {
            index = IS_JOIN_BR2;
        } else if (thing_tile->is_join_x) {
            index = IS_JOIN_X;
        } else if (thing_tile->is_join_x1) {
            index = IS_JOIN_X1;
        } else if (thing_tile->is_join_x1_270) {
            index = IS_JOIN_X1_270;
        } else if (thing_tile->is_join_x1_180) {
            index = IS_JOIN_X1_180;
        } else if (thing_tile->is_join_x1_90) {
            index = IS_JOIN_X1_90;
        } else if (thing_tile->is_join_x2) {
            index = IS_JOIN_X2;
        } else if (thing_tile->is_join_x2_270) {
            index = IS_JOIN_X2_270;
        } else if (thing_tile->is_join_x2_180) {
            index = IS_JOIN_X2_180;
        } else if (thing_tile->is_join_x2_90) {
            index = IS_JOIN_X2_90;
        } else if (thing_tile->is_join_x3) {
            index = IS_JOIN_X3;
        } else if (thing_tile->is_join_x3_180) {
            index = IS_JOIN_X3_180;
        } else if (thing_tile->is_join_x4) {
            index = IS_JOIN_X4;
        } else if (thing_tile->is_join_x4_270) {
            index = IS_JOIN_X4_270;
        } else if (thing_tile->is_join_x4_180) {
            index = IS_JOIN_X4_180;
        } else if (thing_tile->is_join_x4_90) {
            index = IS_JOIN_X4_90;
        } else if (thing_tile->is_join_horiz2) {
            index = IS_JOIN_HORIZ2;
        } else if (thing_tile->is_join_vert2) {
            index = IS_JOIN_VERT2;
        } else {
            continue;
        }

        t->tilep_join[index][ t->tilep_join_count[index] ] = thing_tile;

        tile = tile_find(thing_tile_name(thing_tile));
        if (!tile) {
            DIE("no tile for join index %d for %s, tile name %s",
                index, t->short_name, thing_tile_name(thing_tile));
        }

        t->tilep_join_tile[index][ t->tilep_join_count[index] ] = tile;

        if (t->tilep_join_count[index]++ >= IS_JOIN_ALT_MAX) {
            DIE("too many tile join alternatives for %s", t->short_name);
        }
    }
}

thing_templatep thing_template_load (uint16_t *id, const char *name)
{
    thing_templatep t;
    demarshal_p in;

    if (thing_template_find(name)) {
        DIE("thing template name [%s] already used", name);
    }

    if (!thing_templates) {
        thing_templates = tree_alloc(TREE_KEY_STRING, "TREE ROOT: thing");
    }

    if (!thing_templates_create_order) {
        thing_templates_create_order =
                        tree_alloc(TREE_KEY_INTEGER,
                                   "TREE ROOT: thing create order");
    }

    if (thing_templates_chunk_count >= THING_MAX - 1) {
        DIE("too many thing templates");
    }

    t = &thing_templates_chunk[thing_templates_chunk_count];
    t->tree.key = dupstr(name, "TREE KEY: thing");

    *id = thing_templates_chunk_count;
    t->id = *id;

    thing_templates_chunk_count++;

    if (!tree_insert_static(thing_templates, &t->tree.node)) {
        DIE("thing template insert name [%s] failed", name);
    }

    in = demarshal(name);
    if (!in) {
        DIE("cannot demarshal template name [%s] failed", name);
    }

    demarshal_thing_template(in, t);
    demarshal_fini(in);

    static uint32_t create_order;

    t->tree2.key = create_order++;
    if (!tree_insert_static(thing_templates_create_order, &t->tree2.node)) {
        DIE("thing template insert create order [%s] failed", name);
    }

    thing_template_fill_cache(t);

    DBG("thing template: %s", name);

    return (t);
}

static void thing_template_destroy_internal (thing_templatep t)
{
    tree_destroy(&t->tiles, (tree_destroy_func)thing_tile_free);
    tree_destroy(&t->tiles2, (tree_destroy_func)thing_tile_free);

    if (t->tooltip) {
        myfree(t->tooltip);
    }

    if (t->short_name) {
        myfree(t->short_name);
    }

    if (t->polymorph_on_death) {
        myfree(t->polymorph_on_death);
    }

    if (t->mob_spawn) {
        myfree(t->mob_spawn);
    }
}

void thing_templates_marshal (marshal_p out)
{
    tree_root *tree;
    thing_templatep t;

    tree = thing_templates;

    TREE_WALK(tree, t) {
        marshal_thing_template(out, t);

        {
            marshal_p tmp;
            tmp = marshal(0); /* MY_STDOUT */
            marshal_thing_template(tmp, t);
            marshal_fini(tmp);
        }
    }
}

/*
 * Find an existing thing.
 */
thing_templatep thing_template_find (const char *name)
{
    thing_template target;
    thing_templatep result;

    if (!name) {
        DIE("no name for thing find");
    }

    memset(&target, 0, sizeof(target));
    target.tree.key = (char*) name;

    result = (typeof(result)) tree_find(thing_templates, &target.tree.node);
    if (!result) {
        return (0);
    }

    return (result);
}

thing_templatep thing_template_find_short_name (const char *name)
{
    tree_root *tree;
    thing_templatep t;

    tree = thing_templates;

    TREE_WALK(tree, t) {
        if (!strcmp(name, t->short_name)) {
            return (t);
        }
    }

    return (0);
}

/*
 * Given a string name, map to a thing template.
 */
thing_templatep string2thing_template (const char **s)
{
    static char tmp[MAXSTR];
    static const char *eo_tmp = tmp + MAXSTR;
    const char *c = *s;
    char *t = tmp;

    while (t < eo_tmp) {
        if ((*c == '\0') || (*c == '$')) {
            break;
        }

        *t++ = *c++;
    }

    if (c == eo_tmp) {
        return (0);
    }

    *t++ = '\0';
    *s += (t - tmp);

    thing_template find;
    thing_templatep target;

    memset(&find, 0, sizeof(find));
    find.tree.key = tmp;

    target = (typeof(target)) tree_find(thing_templates, &find.tree.node);
    if (!target) {
        DIE("unknown thing [%s]", tmp);
    }

    return (target);
}

static void demarshal_thing_carrying (demarshal_p ctx, thing_template *t)
{
    if (!GET_PEEK_NAME(ctx, "carrying")) {
        return;
    }

    GET_NAME(ctx, "carrying");

    GET_BRA(ctx);

    (void) demarshal_gotone(ctx);

    char *val;

    while (GET_PEEK_STRING(ctx, val)) {
        GET_STRING(ctx, val);

        thing_templatep c = thing_template_find(val);
        if (!c) {
            DIE("carried thing %s not found", val);
            continue;
        }

        uint32_t id = thing_template_to_id(c);
        if (!id) {
            DIE("carried thing %s not in database", val);
        }

        t->carrying[id]++;

        myfree(val);
    } while (demarshal_gotone(ctx));

    GET_KET(ctx);
}

void demarshal_thing_template (demarshal_p ctx, thing_templatep t)
{
    char *name;

    GET_BRA(ctx);

    GET_OPT_DEF_NAMED_STRING(ctx, "name", name, "<no name>");
    GET_OPT_DEF_NAMED_STRING(ctx, "short_name", t->short_name, "<no name>");
    GET_OPT_DEF_NAMED_STRING(ctx, "tooltip", t->tooltip, "<no name>");

    demarshal_thing_carrying(ctx, t);

    (void) demarshal_gotone(ctx);

    do {
        
        char *val;
        if (GET_OPT_NAMED_STRING(ctx, "fires", val)) {
            t->fires = thing_template_find(val);
            if (!t->fires) {
                DIE("cannot find %s for %s to fire",
                    val, t->short_name);
            }
            myfree(val);
        }

        GET_OPT_NAMED_STRING(ctx, "polymorph_on_death", t->polymorph_on_death);
        GET_OPT_NAMED_STRING(ctx, "mob_spawn", t->mob_spawn);
        GET_OPT_NAMED_UINT8(ctx, "z_depth", t->z_depth);
        GET_OPT_NAMED_UINT8(ctx, "z_order", t->z_order);
        GET_OPT_NAMED_UINT16(ctx, "speed", t->speed);
        GET_OPT_NAMED_INT16(ctx, "health", t->health);
        GET_OPT_NAMED_UINT16(ctx, "damage", t->damage);
        GET_OPT_NAMED_UINT16(ctx, "lifespan", t->lifespan);
        GET_OPT_NAMED_UINT8(ctx, "vision_distance", t->vision_distance);
        GET_OPT_NAMED_INT16(ctx, "bonus_score_on_death", t->bonus_score_on_death);
        GET_OPT_NAMED_INT16(ctx, "bonus_score_on_collect", t->bonus_score_on_collect);
        GET_OPT_NAMED_INT16(ctx, "bonus_health_on_use", t->bonus_health_on_use);
        GET_OPT_NAMED_UINT32(ctx, "ppp1", t->ppp1);
        GET_OPT_NAMED_UINT32(ctx, "ppp2", t->ppp2);
        GET_OPT_NAMED_UINT32(ctx, "ppp3", t->ppp3);
        GET_OPT_NAMED_UINT32(ctx, "ppp4", t->ppp4);
        GET_OPT_NAMED_UINT32(ctx, "ppp5", t->ppp5);
        GET_OPT_NAMED_UINT32(ctx, "ppp6", t->ppp6);
        GET_OPT_NAMED_UINT32(ctx, "ppp7", t->ppp7);
        GET_OPT_NAMED_UINT32(ctx, "ppp8", t->ppp8);
        GET_OPT_NAMED_UINT32(ctx, "ppp9", t->ppp9);
        GET_OPT_NAMED_UINT32(ctx, "ppp10", t->ppp10);
        GET_OPT_NAMED_UINT32(ctx, "ppp11", t->ppp11);
        GET_OPT_NAMED_UINT32(ctx, "ppp12", t->ppp12);
        GET_OPT_NAMED_UINT32(ctx, "ppp13", t->ppp13);
        GET_OPT_NAMED_UINT32(ctx, "ppp14", t->ppp14);
        GET_OPT_NAMED_UINT32(ctx, "tx_map_update_delay_thousandths", t->tx_map_update_delay_thousandths);
        GET_OPT_NAMED_UINT32(ctx, "can_be_hit_chance", t->can_be_hit_chance);
        GET_OPT_NAMED_UINT32(ctx, "failure_chance", t->failure_chance);
        GET_OPT_NAMED_UINT32(ctx, "hit_delay_tenths", t->hit_delay_tenths);
        GET_OPT_NAMED_UINT32(ctx, "mob_spawn_delay_tenths", t->mob_spawn_delay_tenths);

        GET_OPT_NAMED_BITFIELD(ctx, "is_player", t->is_player);
        GET_OPT_NAMED_BITFIELD(ctx, "is_exit", t->is_exit);
        GET_OPT_NAMED_BITFIELD(ctx, "is_floor", t->is_floor);
        GET_OPT_NAMED_BITFIELD(ctx, "is_food", t->is_food);
        GET_OPT_NAMED_BITFIELD(ctx, "is_monst", t->is_monst);
        GET_OPT_NAMED_BITFIELD(ctx, "is_plant", t->is_plant);
        GET_OPT_NAMED_BITFIELD(ctx, "is_player", t->is_player);
        GET_OPT_NAMED_BITFIELD(ctx, "is_key", t->is_key);
        GET_OPT_NAMED_BITFIELD(ctx, "is_collision_map_medium", t->is_collision_map_medium);
        GET_OPT_NAMED_BITFIELD(ctx, "is_collision_map_large", t->is_collision_map_large);
        GET_OPT_NAMED_BITFIELD(ctx, "is_collision_map_tiny", t->is_collision_map_tiny);
        GET_OPT_NAMED_BITFIELD(ctx, "is_xxx3", t->is_xxx3);
        GET_OPT_NAMED_BITFIELD(ctx, "is_xxx4", t->is_xxx4);
        GET_OPT_NAMED_BITFIELD(ctx, "is_weapon", t->is_weapon);
        GET_OPT_NAMED_BITFIELD(ctx, "is_treasure", t->is_treasure);
        GET_OPT_NAMED_BITFIELD(ctx, "is_wearable", t->is_wearable);
        GET_OPT_NAMED_BITFIELD(ctx, "is_fragile", t->is_fragile);
        GET_OPT_NAMED_BITFIELD(ctx, "is_star", t->is_star);
        GET_OPT_NAMED_BITFIELD(ctx, "is_xxx5", t->is_xxx5);
        GET_OPT_NAMED_BITFIELD(ctx, "is_xxx6", t->is_xxx6);
        GET_OPT_NAMED_BITFIELD(ctx, "is_key2", t->is_key2);
        GET_OPT_NAMED_BITFIELD(ctx, "is_key3", t->is_key3);
        GET_OPT_NAMED_BITFIELD(ctx, "is_fire", t->is_fire);
        GET_OPT_NAMED_BITFIELD(ctx, "is_ring", t->is_ring);
        GET_OPT_NAMED_BITFIELD(ctx, "is_xxx9", t->is_xxx9);
        GET_OPT_NAMED_BITFIELD(ctx, "is_xxx9", t->is_xxx9);
        GET_OPT_NAMED_BITFIELD(ctx, "is_poison", t->is_poison);
        GET_OPT_NAMED_BITFIELD(ctx, "is_carryable", t->is_carryable);
        GET_OPT_NAMED_BITFIELD(ctx, "is_item_unusable", t->is_item_unusable);
        GET_OPT_NAMED_BITFIELD(ctx, "is_valid_for_shortcut_key", t->is_valid_for_shortcut_key);
        GET_OPT_NAMED_BITFIELD(ctx, "is_seedpod", t->is_seedpod);
        GET_OPT_NAMED_BITFIELD(ctx, "is_spam", t->is_spam);
        GET_OPT_NAMED_BITFIELD(ctx, "is_door", t->is_door);
        GET_OPT_NAMED_BITFIELD(ctx, "is_pipe", t->is_pipe);
        GET_OPT_NAMED_BITFIELD(ctx, "is_mob_spawner", t->is_mob_spawner);
        GET_OPT_NAMED_BITFIELD(ctx, "is_scarable", t->is_scarable);
        GET_OPT_NAMED_BITFIELD(ctx, "is_explosion", t->is_explosion);
        GET_OPT_NAMED_BITFIELD(ctx, "is_hidden_from_editor", t->is_hidden_from_editor);
        GET_OPT_NAMED_BITFIELD(ctx, "is_animated", t->is_animated);
        GET_OPT_NAMED_BITFIELD(ctx, "is_combustable", t->is_combustable);
        GET_OPT_NAMED_BITFIELD(ctx, "is_projectile", t->is_projectile);
        GET_OPT_NAMED_BITFIELD(ctx, "is_boring", t->is_boring);
        GET_OPT_NAMED_BITFIELD(ctx, "is_ring", t->is_ring);
        GET_OPT_NAMED_BITFIELD(ctx, "is_joinable", t->is_joinable);
        GET_OPT_NAMED_BITFIELD(ctx, "is_wall", t->is_wall);
        GET_OPT_NAMED_BITFIELD(ctx, "is_effect_sway", t->is_effect_sway);
        GET_OPT_NAMED_BITFIELD(ctx, "is_effect_pulse", t->is_effect_pulse);
        GET_OPT_NAMED_BITFIELD(ctx, "is_effect_rotate_4way", t->is_effect_rotate_4way);
        GET_OPT_NAMED_BITFIELD(ctx, "is_effect_rotate_2way", t->is_effect_rotate_2way);

    } while (demarshal_gotone(ctx));

    if (t->is_player || t->is_projectile) {
        t->tx_map_update_delay_thousandths = 
                        DELAY_THOUSANDTHS_TX_MAP_UPDATE_FAST;
    } else {
        t->tx_map_update_delay_thousandths = 
                        DELAY_THOUSANDTHS_TX_MAP_UPDATE_SLOW;
    }

    /*
     * Read the tiles tree.
     */
    demarshal_thing_tiles(ctx, t);
    demarshal_thing_tiles2(ctx, t);

    myfree(name);

    GET_KET(ctx);
}

void marshal_thing_template (marshal_p ctx, thing_templatep t)
{
    PUT_BRA(ctx);

    PUT_NAMED_STRING(ctx, "name", t->tree.key);
    PUT_NAMED_STRING(ctx, "short_name", t->short_name);
    PUT_NAMED_STRING(ctx, "tooltip", t->tooltip);
    if (t->fires) {
        PUT_NAMED_STRING(ctx, "fires", thing_template_name(t->fires));
    }
    PUT_NAMED_STRING(ctx, "polymorph_on_death", t->polymorph_on_death);
    PUT_NAMED_STRING(ctx, "mob_spawn", t->mob_spawn);
    PUT_NAMED_UINT8(ctx, "z_depth", t->z_depth);
    PUT_NAMED_UINT8(ctx, "z_order", t->z_order);
    PUT_NAMED_INT32(ctx, "speed", t->speed);
    PUT_NAMED_INT32(ctx, "health", t->health);
    PUT_NAMED_INT32(ctx, "damage", t->damage);
    PUT_NAMED_INT32(ctx, "lifespan", t->lifespan);
    PUT_NAMED_INT32(ctx, "bonus_score_on_death", t->bonus_score_on_death);
    PUT_NAMED_INT32(ctx, "vision_distance", t->vision_distance);
    PUT_NAMED_INT32(ctx, "bonus_score_on_collect", t->bonus_score_on_collect);
    PUT_NAMED_INT32(ctx, "ppp1", t->ppp1);
    PUT_NAMED_INT32(ctx, "ppp2", t->ppp2);
    PUT_NAMED_INT32(ctx, "ppp3", t->ppp3);
    PUT_NAMED_INT32(ctx, "ppp4", t->ppp4);
    PUT_NAMED_INT32(ctx, "ppp5", t->ppp5);
    PUT_NAMED_INT32(ctx, "ppp6", t->ppp6);
    PUT_NAMED_INT32(ctx, "ppp7", t->ppp7);
    PUT_NAMED_INT32(ctx, "ppp8", t->ppp8);
    PUT_NAMED_INT32(ctx, "ppp9", t->ppp9);
    PUT_NAMED_INT32(ctx, "ppp10", t->ppp10);
    PUT_NAMED_INT32(ctx, "ppp11", t->ppp11);
    PUT_NAMED_INT32(ctx, "ppp12", t->ppp12);
    PUT_NAMED_INT32(ctx, "ppp13", t->ppp13);
    PUT_NAMED_INT32(ctx, "ppp14", t->ppp14);
    PUT_NAMED_INT32(ctx, "bonus_health_on_use", t->bonus_health_on_use);
    PUT_NAMED_INT32(ctx, "tx_map_update_delay_thousandths", t->tx_map_update_delay_thousandths);
    PUT_NAMED_INT32(ctx, "can_be_hit_chance", t->can_be_hit_chance);
    PUT_NAMED_INT32(ctx, "failure_chance", t->failure_chance);
    PUT_NAMED_INT32(ctx, "hit_delay_tenths", t->hit_delay_tenths);
    PUT_NAMED_INT32(ctx, "mob_spawn_delay_tenths", t->mob_spawn_delay_tenths);

    PUT_NAMED_BITFIELD(ctx, "is_player", t->is_player);
    PUT_NAMED_BITFIELD(ctx, "is_exit", t->is_exit);
    PUT_NAMED_BITFIELD(ctx, "is_floor", t->is_floor);
    PUT_NAMED_BITFIELD(ctx, "is_food", t->is_food);
    PUT_NAMED_BITFIELD(ctx, "is_monst", t->is_monst);
    PUT_NAMED_BITFIELD(ctx, "is_plant", t->is_plant);
    PUT_NAMED_BITFIELD(ctx, "is_player", t->is_player);
    PUT_NAMED_BITFIELD(ctx, "is_key", t->is_key);
    PUT_NAMED_BITFIELD(ctx, "is_collision_map_tiny", t->is_collision_map_tiny);
    PUT_NAMED_BITFIELD(ctx, "is_collision_map_large", t->is_collision_map_large);
    PUT_NAMED_BITFIELD(ctx, "is_collision_map_medium", t->is_collision_map_medium);
    PUT_NAMED_BITFIELD(ctx, "is_xxx3", t->is_xxx3);
    PUT_NAMED_BITFIELD(ctx, "is_xxx4", t->is_xxx4);
    PUT_NAMED_BITFIELD(ctx, "is_weapon", t->is_weapon);
    PUT_NAMED_BITFIELD(ctx, "is_treasure", t->is_treasure);
    PUT_NAMED_BITFIELD(ctx, "is_wearable", t->is_wearable);
    PUT_NAMED_BITFIELD(ctx, "is_fragile", t->is_fragile);
    PUT_NAMED_BITFIELD(ctx, "is_star", t->is_star);
    PUT_NAMED_BITFIELD(ctx, "is_xxx5", t->is_xxx5);
    PUT_NAMED_BITFIELD(ctx, "is_xxx6", t->is_xxx6);
    PUT_NAMED_BITFIELD(ctx, "is_key2", t->is_key2);
    PUT_NAMED_BITFIELD(ctx, "is_key3", t->is_key3);
    PUT_NAMED_BITFIELD(ctx, "is_fire", t->is_fire);
    PUT_NAMED_BITFIELD(ctx, "is_ring", t->is_ring);
    PUT_NAMED_BITFIELD(ctx, "is_xxx9", t->is_xxx9);
    PUT_NAMED_BITFIELD(ctx, "is_xxx9", t->is_xxx9);
    PUT_NAMED_BITFIELD(ctx, "is_poison", t->is_poison);
    PUT_NAMED_BITFIELD(ctx, "is_carryable", t->is_carryable);
    PUT_NAMED_BITFIELD(ctx, "is_item_unusable", t->is_item_unusable);
    PUT_NAMED_BITFIELD(ctx, "is_valid_for_shortcut_key", t->is_valid_for_shortcut_key);
    PUT_NAMED_BITFIELD(ctx, "is_seedpod", t->is_seedpod);
    PUT_NAMED_BITFIELD(ctx, "is_spam", t->is_spam);
    PUT_NAMED_BITFIELD(ctx, "is_door", t->is_door);
    PUT_NAMED_BITFIELD(ctx, "is_pipe", t->is_pipe);
    PUT_NAMED_BITFIELD(ctx, "is_mob_spawner", t->is_mob_spawner);
    PUT_NAMED_BITFIELD(ctx, "is_scarable", t->is_scarable);
    PUT_NAMED_BITFIELD(ctx, "is_explosion", t->is_explosion);
    PUT_NAMED_BITFIELD(ctx, "is_hidden_from_editor", t->is_hidden_from_editor);
    PUT_NAMED_BITFIELD(ctx, "is_animated", t->is_animated);
    PUT_NAMED_BITFIELD(ctx, "is_combustable", t->is_combustable);
    PUT_NAMED_BITFIELD(ctx, "is_projectile", t->is_projectile);
    PUT_NAMED_BITFIELD(ctx, "is_boring", t->is_boring);
    PUT_NAMED_BITFIELD(ctx, "is_ring", t->is_ring);
    PUT_NAMED_BITFIELD(ctx, "is_joinable", t->is_joinable);
    PUT_NAMED_BITFIELD(ctx, "is_wall", t->is_wall);
    PUT_NAMED_BITFIELD(ctx, "is_effect_sway", t->is_effect_sway);
    PUT_NAMED_BITFIELD(ctx, "is_effect_pulse", t->is_effect_pulse);
    PUT_NAMED_BITFIELD(ctx, "is_effect_rotate_4way", t->is_effect_rotate_4way);
    PUT_NAMED_BITFIELD(ctx, "is_effect_rotate_2way", t->is_effect_rotate_2way);

    /*
     * Write the tiles tree.
     */
    marshal_thing_tiles(ctx, t);
    marshal_thing_tiles2(ctx, t);

    PUT_KET(ctx);
}

const char *thing_template_name (thing_templatep t)
{
    return (t->tree.key);
}

const char *thing_template_short_name (thing_templatep t)
{
    return (t->short_name);
}

thing_templatep thing_template_fires (thing_templatep t)
{
    return (t->fires);
}

const char *thing_template_polymorph_on_death (thing_templatep t)
{
    return (t->polymorph_on_death);
}

const char *thing_template_mob_spawn (thing_templatep t)
{
    return (t->mob_spawn);
}

const char *thing_template_get_tooltip (thing_templatep t)
{
    return (t->tooltip);
}

uint8_t thing_template_get_z_depth (thing_templatep t)
{
    return (t->z_depth);
}

uint8_t thing_template_get_z_order (thing_templatep t)
{
    return (t->z_order);
}

uint32_t thing_template_get_speed (thing_templatep t)
{
    return (t->speed);
}

int16_t thing_template_get_health (thing_templatep t)
{
    return (t->health);
}

uint32_t thing_template_get_damage (thing_templatep t)
{
    return (t->damage);
}

uint32_t thing_template_get_lifespan (thing_templatep t)
{
    return (t->lifespan);
}

int16_t thing_template_get_bonus_score_on_death (thing_templatep t)
{
    return (t->bonus_score_on_death);
}

uint32_t thing_template_get_vision_distance (thing_templatep t)
{
    return (t->vision_distance);
}

int16_t thing_template_get_bonus_score_on_collect (thing_templatep t)
{
    return (t->bonus_score_on_collect);
}

uint32_t thing_template_get_ppp1 (thing_templatep t)
{
    return (t->ppp1);
}

uint32_t thing_template_get_ppp2 (thing_templatep t)
{
    return (t->ppp2);
}

uint32_t thing_template_get_ppp3 (thing_templatep t)
{
    return (t->ppp3);
}

uint32_t thing_template_get_ppp4 (thing_templatep t)
{
    return (t->ppp4);
}

uint32_t thing_template_get_ppp5 (thing_templatep t)
{
    return (t->ppp5);
}

uint32_t thing_template_get_ppp6 (thing_templatep t)
{
    return (t->ppp6);
}

uint32_t thing_template_get_ppp7 (thing_templatep t)
{
    return (t->ppp7);
}

uint32_t thing_template_get_ppp8 (thing_templatep t)
{
    return (t->ppp8);
}

uint32_t thing_template_get_ppp9 (thing_templatep t)
{
    return (t->ppp9);
}

uint32_t thing_template_get_ppp10 (thing_templatep t)
{
    return (t->ppp10);
}

uint32_t thing_template_get_ppp11 (thing_templatep t)
{
    return (t->ppp11);
}

uint32_t thing_template_get_ppp12 (thing_templatep t)
{
    return (t->ppp12);
}

uint32_t thing_template_get_ppp13 (thing_templatep t)
{
    return (t->ppp13);
}

uint32_t thing_template_get_ppp14 (thing_templatep t)
{
    return (t->ppp14);
}

int16_t thing_template_get_bonus_health_on_use (thing_templatep t)
{
    return (t->bonus_health_on_use);
}

uint32_t thing_template_get_tx_map_update_delay_thousandths (thing_templatep t)
{
    return (t->tx_map_update_delay_thousandths);
}

uint32_t thing_template_get_can_be_hit_chance (thing_templatep t)
{
    return (t->can_be_hit_chance);
}

uint32_t thing_template_get_failure_chance (thing_templatep t)
{
    return (t->failure_chance);
}

uint32_t thing_template_get_hit_delay_tenths (thing_templatep t)
{
    return (t->hit_delay_tenths);
}

uint32_t thing_template_get_mob_spawn_delay_tenths (thing_templatep t)
{
    return (t->mob_spawn_delay_tenths);
}

tree_rootp thing_template_get_tiles (thing_templatep t)
{
    return (t->tiles);
}

tree_rootp thing_template_get_tiles2 (thing_templatep t)
{
    return (t->tiles2);
}

