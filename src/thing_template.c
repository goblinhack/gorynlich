/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
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

tree_root *thing_templates;
tree_root *thing_templates_create_order;

static uint8_t tp_init_done;
static void tp_destroy_internal(tpp t);

uint8_t tp_init (void)
{
    tp_init_done = true;

    return (true);
}

void tp_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (tp_init_done) {
        tp_init_done = false;

        tree_destroy(&thing_templates,
            (tree_destroy_func)tp_destroy_internal);

        tree_destroy(&thing_templates_create_order,
            (tree_destroy_func)0);
    }
}

/*
 * Fill up the tile cache for future fast finds of tile types.
 */
static void tp_fill_cache (tpp t)
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
            ERR("no tile for join index %d for %s, tile name %s",
                index, t->short_name, thing_tile_name(thing_tile));
        }

        t->tilep_join_tile[index][ t->tilep_join_count[index] ] = tile;

        if (t->tilep_join_count[index]++ >= IS_JOIN_ALT_MAX) {
            ERR("too many tile join alternatives for %s", t->short_name);
        }
    }
}

tpp tp_load (uint16_t id, const char *name)
{
    tpp t;
    demarshal_p in;

    if (tp_find(name)) {
        ERR("thing template name [%s] already used", name);
    }

    if (!thing_templates) {
        thing_templates = tree_alloc(TREE_KEY_STRING, "TREE ROOT: thing");
    }

    if (!thing_templates_create_order) {
        thing_templates_create_order =
                        tree_alloc(TREE_KEY_INTEGER,
                                   "TREE ROOT: thing create order");
    }

    if (id >= THING_MAX - 1) {
        ERR("too many thing templates");
    }

    t = &thing_templates_chunk[id];
    t->tree.key = dupstr(name, "TREE KEY: thing");

    t->id = id;

    if (!tree_insert_static(thing_templates, &t->tree.node)) {
        ERR("thing template insert name [%s] failed", name);
    }

    in = demarshal(name);
    if (!in) {
        ERR("cannot demarshal template name [%s] failed", name);
    }

    demarshal_thing_template(in, t);
    demarshal_fini(in);

    static uint32_t create_order;

    t->tree2.key = create_order++;
    if (!tree_insert_static(thing_templates_create_order, &t->tree2.node)) {
        ERR("thing template insert create order [%s] failed", name);
    }

    tp_fill_cache(t);

    return (t);
}

static void tp_destroy_internal (tpp t)
{
    tree_destroy(&t->tiles, (tree_destroy_func)thing_tile_free);
    tree_destroy(&t->tiles2, (tree_destroy_func)thing_tile_free);

    if (t->tooltip) {
        myfree(t->tooltip);
    }

    if (t->short_name) {
        myfree(t->short_name);
    }

    if (t->raw_name) {
        myfree(t->raw_name);
    }

    if (t->polymorph_on_death) {
        myfree(t->polymorph_on_death);
    }

    if (t->carried_as) {
        myfree(t->carried_as);
    }

    if (t->light_tint) {
        myfree(t->light_tint);
    }

    if (t->spawn_on_death) {
        myfree(t->spawn_on_death);
    }

    if (t->explodes_as) {
        myfree(t->explodes_as);
    }

    if (t->sound_on_creation) {
        myfree(t->sound_on_creation);
    }

    if (t->sound_on_hitting_something) {
        myfree(t->sound_on_hitting_something);
    }

    if (t->sound_on_death) {
        myfree(t->sound_on_death);
    }

    if (t->sound_on_collect) {
        myfree(t->sound_on_collect);
    }

    if (t->sound_random) {
        myfree(t->sound_random);
    }

    if (t->weapon_carry_anim) {
        myfree(t->weapon_carry_anim);
    }

    if (t->weapon_swing_anim) {
        myfree(t->weapon_swing_anim);
    }

    if (t->shield_carry_anim) {
        myfree(t->shield_carry_anim);
    }

    if (t->magic_anim) {
        myfree(t->magic_anim);
    }

    if (t->message_on_use) {
        myfree(t->message_on_use);
    }

    int i;
    for (i = 0; i < t->mob_spawn_count; i++) {
        if (t->mob_spawn_what[i]) {
            myfree(t->mob_spawn_what[i]);
        }
    }
}

void thing_templates_marshal (marshal_p out)
{
    tree_root *tree;
    tpp t;

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
tpp tp_find (const char *name)
{
    thing_template target;
    tpp result;

    if (!name) {
        ERR("no name for thing find");
    }

    memset(&target, 0, sizeof(target));
    target.tree.key = (char*) name;

    result = (typeof(result)) tree_find(thing_templates, &target.tree.node);
    if (!result) {
        return (0);
    }

    return (result);
}

tpp tp_find_short_name (const char *name)
{
    tree_root *tree;
    tpp t;

    tree = thing_templates;

    TREE_WALK(tree, t) {
        if (!strcasecmp(name, t->short_name)) {
            return (t);
        }
    }

    ERR("did not find short template name \"%s\"", name);

    return (0);
}

/*
 * Given a string name, map to a thing template.
 */
tpp string2thing_template (const char **s)
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
    tpp target;

    memset(&find, 0, sizeof(find));
    find.tree.key = tmp;

    target = (typeof(target)) tree_find(thing_templates, &find.tree.node);
    if (!target) {
        ERR("unknown thing [%s]", tmp);
    }

    return (target);
}

static void demarshal_thing_carrying (demarshal_p ctx, tpp t)
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

        tpp c = tp_find(val);
        if (!c) {
            DIE("carried thing %s not found", val);
        }

        uint32_t id = tp_to_id(c);
        if (!id) {
            DIE("carried thing %s not in database", val);
        }

        t->base_items[id].quantity++;
        t->base_items[id].id = id;

        myfree(val);
    } while (demarshal_gotone(ctx));

    GET_KET(ctx);
}

static void demarshal_thing_mob_spawn (demarshal_p ctx, tpp t)
{
    if (!GET_PEEK_NAME(ctx, "mob_spawn")) {
        return;
    }

    GET_NAME(ctx, "mob_spawn");

    GET_BRA(ctx);

    (void) demarshal_gotone(ctx);

    while (GET_PEEK_UINT32(ctx)) {
        int chance;
        GET_INT32(ctx, chance);

        char *v;
        GET_STRING(ctx, v);

        if (t->mob_spawn_count == MAX_MOB_SPAWN) {
            DIE("thing %s has too many mob spawn values", v);
        }

        t->mob_spawn_what[t->mob_spawn_count] = v;
        t->mob_spawn_chance_d1000[t->mob_spawn_count++] = chance;
    } while (demarshal_gotone(ctx));

    GET_KET(ctx);
}

ENUM_DEF_C(MAP_DEPTH, map_depth)

void demarshal_thing_template (demarshal_p ctx, tpp t)
{
    char *name;

    GET_BRA(ctx);

    GET_OPT_DEF_NAMED_STRING(ctx, "name", name, "<no name>");
    GET_OPT_DEF_NAMED_STRING(ctx, "short_name", t->short_name, "<no name>");
    GET_OPT_DEF_NAMED_STRING(ctx, "raw_name", t->raw_name, "<no name>");
    GET_OPT_DEF_NAMED_STRING(ctx, "tooltip", t->tooltip, "<no name>");

    demarshal_thing_carrying(ctx, t);
    demarshal_thing_mob_spawn(ctx, t);

    (void) demarshal_gotone(ctx);

    do {
        
        char *val;
        if (GET_OPT_NAMED_STRING(ctx, "fires", val)) {
            t->fires = tp_find(val);
            if (!t->fires) {
                DIE("cannot find %s for %s to fire",
                    val, t->short_name);
            }
            myfree(val);
        }

        GET_OPT_NAMED_STRING(ctx, "polymorph_on_death", t->polymorph_on_death);
        GET_OPT_NAMED_STRING(ctx, "carried_as", t->carried_as);
        GET_OPT_NAMED_STRING(ctx, "light_tint", t->light_tint);
        GET_OPT_NAMED_STRING(ctx, "spawn_on_death", t->spawn_on_death);
        GET_OPT_NAMED_STRING(ctx, "explodes_as", t->explodes_as);
        GET_OPT_NAMED_STRING(ctx, "sound_on_creation", t->sound_on_creation);
        GET_OPT_NAMED_STRING(ctx, "sound_on_hitting_something", t->sound_on_hitting_something);
        GET_OPT_NAMED_STRING(ctx, "sound_on_death", t->sound_on_death);
        GET_OPT_NAMED_STRING(ctx, "sound_on_collect", t->sound_on_collect);
        GET_OPT_NAMED_STRING(ctx, "sound_random", t->sound_random);
        GET_OPT_NAMED_STRING(ctx, "weapon_carry_anim", t->weapon_carry_anim);
        GET_OPT_NAMED_STRING(ctx, "shield_carry_anim", t->shield_carry_anim);
        GET_OPT_NAMED_STRING(ctx, "magic_anim", t->magic_anim);
        GET_OPT_NAMED_STRING(ctx, "weapon_swing_anim", t->weapon_swing_anim);
        GET_OPT_NAMED_STRING(ctx, "message_on_use", t->message_on_use);

        if (GET_PEEK_NAME(ctx, "z_depth")) {
            map_depth en = 0;
            GET_OPT_NAMED_ENUM(ctx, "z_depth", en, map_depth_str2val);
            t->z_depth = en;
            if (t->z_depth >= MAP_DEPTH_MAX) {
                ERR("%s has unknown depth", t->short_name); 
            }
        }

        GET_OPT_NAMED_UINT8(ctx, "z_order", t->z_order);
        GET_OPT_NAMED_UINT16(ctx, "speed", t->speed);
        GET_OPT_NAMED_UINT16(ctx, "damage", t->damage);
        GET_OPT_NAMED_UINT16(ctx, "cost", t->cost);
        GET_OPT_NAMED_UINT16(ctx, "lifespan", t->lifespan);
        GET_OPT_NAMED_UINT8(ctx, "vision_distance", t->vision_distance);
        GET_OPT_NAMED_UINT8(ctx, "approach_distance", t->approach_distance);
        GET_OPT_NAMED_INT32(ctx, "bonus_xp_on_death", t->bonus_xp_on_death);
        GET_OPT_NAMED_INT32(ctx, "bonus_cash_on_collect", t->bonus_cash_on_collect);
        GET_OPT_NAMED_INT16(ctx, "bonus_hp_on_use", t->bonus_hp_on_use);
        GET_OPT_NAMED_INT16(ctx, "bonus_magic_on_use", t->bonus_magic_on_use);
        GET_OPT_NAMED_UINT32(ctx, "d10000_chance_of_appearing", t->d10000_chance_of_appearing);
        GET_OPT_NAMED_UINT32(ctx, "ppp2", t->ppp2);
        GET_OPT_NAMED_UINT32(ctx, "ppp3", t->ppp3);
        GET_OPT_NAMED_UINT32(ctx, "ppp4", t->ppp4);
        GET_OPT_NAMED_UINT32(ctx, "ppp5", t->ppp5);
        GET_OPT_NAMED_UINT32(ctx, "ppp6", t->ppp6);
        GET_OPT_NAMED_UINT32(ctx, "ppp7", t->ppp7);
        GET_OPT_NAMED_UINT32(ctx, "min_appear_depth", t->min_appear_depth);
        GET_OPT_NAMED_UINT32(ctx, "max_appear_depth", t->max_appear_depth);
        GET_OPT_NAMED_UINT32(ctx, "jump_speed", t->jump_speed);

        int16_t tmp_int16 = 0;
        uint32_t tmp_uint32 = 0;

        if (GET_OPT_NAMED_INT16(ctx, "max_hp", tmp_int16)) {
            stats_set_max_hp(&t->stats, tmp_int16);
        }

        if (GET_OPT_NAMED_INT16(ctx, "max_magic", tmp_int16)) {
            stats_set_max_magic(&t->stats, tmp_int16);
        }

        if (GET_OPT_NAMED_UINT32(ctx, "stats_attack_melee", tmp_uint32)) {
            stats_set_attack_melee(&t->stats, tmp_uint32);
        }

        if (GET_OPT_NAMED_UINT32(ctx, "stats_attack_ranged", tmp_uint32)) {
            stats_set_attack_ranged(&t->stats, tmp_uint32);
        }

        if (GET_OPT_NAMED_UINT32(ctx, "stats_attack_magical", tmp_uint32)) {
            stats_set_attack_magical(&t->stats, tmp_uint32);
        }

        if (GET_OPT_NAMED_UINT32(ctx, "stats_speed", tmp_uint32)) {
            stats_set_speed(&t->stats, tmp_uint32);
        }

        if (GET_OPT_NAMED_UINT32(ctx, "stats_vision", tmp_uint32)) {
            stats_set_vision(&t->stats, tmp_uint32);
        }

        if (GET_OPT_NAMED_UINT32(ctx, "stats_healing", tmp_uint32)) {
            stats_set_healing(&t->stats, tmp_uint32);
        }

        if (GET_OPT_NAMED_UINT32(ctx, "stats_defense", tmp_uint32)) {
            stats_set_defense(&t->stats, tmp_uint32);
        }

        GET_OPT_NAMED_UINT32(ctx, "hp_per_level", t->hp_per_level);
        GET_OPT_NAMED_UINT32(ctx, "id_per_level", t->id_per_level);

        GET_OPT_NAMED_FLOAT(ctx, "light_radius", t->light_radius);
        GET_OPT_NAMED_FLOAT(ctx, "weapon_spread", t->weapon_spread);
        GET_OPT_NAMED_FLOAT(ctx, "weapon_density", t->weapon_density);
        GET_OPT_NAMED_FLOAT(ctx, "scale", t->scale);
        GET_OPT_NAMED_FLOAT(ctx, "explosion_radius", t->explosion_radius);
        GET_OPT_NAMED_FLOAT(ctx, "collision_radius", t->collision_radius);

        uint32_t quantity;

        if (GET_OPT_NAMED_UINT32(ctx, "quantity", quantity)) {
            t->item.quantity = quantity;
        }

        GET_OPT_NAMED_UINT32(ctx, "hit_priority", t->hit_priority);
        GET_OPT_NAMED_UINT32(ctx, "weapon_fire_delay_hundredths", t->weapon_fire_delay_hundredths);
        GET_OPT_NAMED_UINT32(ctx, "sound_random_delay_secs", t->sound_random_delay_secs);
        GET_OPT_NAMED_UINT32(ctx, "swing_distance_from_player", t->swing_distance_from_player);
        GET_OPT_NAMED_UINT32(ctx, "tx_map_update_delay_thousandths", t->tx_map_update_delay_thousandths);
        GET_OPT_NAMED_UINT32(ctx, "can_be_hit_chance", t->can_be_hit_chance);
        GET_OPT_NAMED_UINT32(ctx, "d10000_chance_of_breaking", t->d10000_chance_of_breaking);
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
        GET_OPT_NAMED_BITFIELD(ctx, "is_collision_map_large", t->is_collision_map_large);
        GET_OPT_NAMED_BITFIELD(ctx, "is_collision_map_small", t->is_collision_map_small);
        GET_OPT_NAMED_BITFIELD(ctx, "is_collision_map_medium", t->is_collision_map_medium);
        GET_OPT_NAMED_BITFIELD(ctx, "is_collision_map_tiny", t->is_collision_map_tiny);
        GET_OPT_NAMED_BITFIELD(ctx, "is_rock", t->is_rock);
        GET_OPT_NAMED_BITFIELD(ctx, "is_shadow_caster", t->is_shadow_caster);
        GET_OPT_NAMED_BITFIELD(ctx, "is_shadow_caster_soft", t->is_shadow_caster_soft);
        GET_OPT_NAMED_BITFIELD(ctx, "is_weapon", t->is_weapon);
        GET_OPT_NAMED_BITFIELD(ctx, "is_treasure", t->is_treasure);
        GET_OPT_NAMED_BITFIELD(ctx, "is_wearable", t->is_wearable);
        GET_OPT_NAMED_BITFIELD(ctx, "is_fragile", t->is_fragile);
        GET_OPT_NAMED_BITFIELD(ctx, "is_star", t->is_star);
        GET_OPT_NAMED_BITFIELD(ctx, "is_animated_no_dir", t->is_animated_no_dir);
        GET_OPT_NAMED_BITFIELD(ctx, "is_weapon_swing_effect", t->is_weapon_swing_effect);
        GET_OPT_NAMED_BITFIELD(ctx, "is_light_source", t->is_light_source);
        GET_OPT_NAMED_BITFIELD(ctx, "is_candle_light", t->is_candle_light);
        GET_OPT_NAMED_BITFIELD(ctx, "is_cats_eyes", t->is_cats_eyes);
        GET_OPT_NAMED_BITFIELD(ctx, "is_fire", t->is_fire);
        GET_OPT_NAMED_BITFIELD(ctx, "is_ring", t->is_ring);
        GET_OPT_NAMED_BITFIELD(ctx, "is_animation", t->is_animation);
        GET_OPT_NAMED_BITFIELD(ctx, "is_non_explosive_gas_cloud", t->is_non_explosive_gas_cloud);
        GET_OPT_NAMED_BITFIELD(ctx, "is_carryable", t->is_carryable);
        GET_OPT_NAMED_BITFIELD(ctx, "is_item_unusable", t->is_item_unusable);
        GET_OPT_NAMED_BITFIELD(ctx, "is_valid_for_action_bar", t->is_valid_for_action_bar);
        GET_OPT_NAMED_BITFIELD(ctx, "is_door", t->is_door);
        GET_OPT_NAMED_BITFIELD(ctx, "is_mob_spawner", t->is_mob_spawner);
        GET_OPT_NAMED_BITFIELD(ctx, "is_rrr1", t->is_rrr1);
        GET_OPT_NAMED_BITFIELD(ctx, "is_rrr2", t->is_rrr2);
        GET_OPT_NAMED_BITFIELD(ctx, "is_rrr3", t->is_rrr3);
        GET_OPT_NAMED_BITFIELD(ctx, "is_rrr4", t->is_rrr4);
        GET_OPT_NAMED_BITFIELD(ctx, "has_ability_burst_shot", t->has_ability_burst_shot);
        GET_OPT_NAMED_BITFIELD(ctx, "is_rrr6", t->is_rrr6);
        GET_OPT_NAMED_BITFIELD(ctx, "has_ability_triple_shot", t->has_ability_triple_shot);
        GET_OPT_NAMED_BITFIELD(ctx, "has_ability_double_shot", t->has_ability_double_shot);
        GET_OPT_NAMED_BITFIELD(ctx, "has_ability_reverse_shot", t->has_ability_reverse_shot);
        GET_OPT_NAMED_BITFIELD(ctx, "is_jesus", t->is_jesus);
        GET_OPT_NAMED_BITFIELD(ctx, "is_blit_y_offset", t->is_blit_y_offset);
        GET_OPT_NAMED_BITFIELD(ctx, "is_dirt", t->is_dirt);
        GET_OPT_NAMED_BITFIELD(ctx, "is_dragon", t->is_dragon);
        GET_OPT_NAMED_BITFIELD(ctx, "is_generator", t->is_generator);
        GET_OPT_NAMED_BITFIELD(ctx, "is_water", t->is_water);
        GET_OPT_NAMED_BITFIELD(ctx, "is_undead", t->is_undead);
        GET_OPT_NAMED_BITFIELD(ctx, "is_hidden", t->is_hidden);
        GET_OPT_NAMED_BITFIELD(ctx, "is_trap", t->is_trap);
        GET_OPT_NAMED_BITFIELD(ctx, "is_corridor_wall", t->is_corridor_wall);
        GET_OPT_NAMED_BITFIELD(ctx, "is_corridor", t->is_corridor);
        GET_OPT_NAMED_BITFIELD(ctx, "is_water_proof", t->is_water_proof);
        GET_OPT_NAMED_BITFIELD(ctx, "is_lava_proof", t->is_lava_proof);
        GET_OPT_NAMED_BITFIELD(ctx, "is_spider_proof", t->is_spider_proof);
        GET_OPT_NAMED_BITFIELD(ctx, "is_acid_proof", t->is_acid_proof);
        GET_OPT_NAMED_BITFIELD(ctx, "is_fires_when_angry", t->is_fires_when_angry);
        GET_OPT_NAMED_BITFIELD(ctx, "is_shopkeeper", t->is_shopkeeper);
        GET_OPT_NAMED_BITFIELD(ctx, "is_shop_floor", t->is_shop_floor);
        GET_OPT_NAMED_BITFIELD(ctx, "is_fireball", t->is_fireball);
        GET_OPT_NAMED_BITFIELD(ctx, "is_bullet", t->is_bullet);
        GET_OPT_NAMED_BITFIELD(ctx, "is_treasure_eater", t->is_treasure_eater);
        GET_OPT_NAMED_BITFIELD(ctx, "is_effect_fade_in_out", t->is_effect_fade_in_out);
        GET_OPT_NAMED_BITFIELD(ctx, "is_internal", t->is_internal);
        GET_OPT_NAMED_BITFIELD(ctx, "is_levitating", t->is_levitating);
        GET_OPT_NAMED_BITFIELD(ctx, "is_acid", t->is_acid);
        GET_OPT_NAMED_BITFIELD(ctx, "is_lava", t->is_lava);
        GET_OPT_NAMED_BITFIELD(ctx, "is_teleport", t->is_teleport);
        GET_OPT_NAMED_BITFIELD(ctx, "is_potion", t->is_potion);
        GET_OPT_NAMED_BITFIELD(ctx, "is_shield", t->is_shield);
        GET_OPT_NAMED_BITFIELD(ctx, "is_death", t->is_death);
        GET_OPT_NAMED_BITFIELD(ctx, "is_cobweb", t->is_cobweb);
        GET_OPT_NAMED_BITFIELD(ctx, "is_ethereal", t->is_ethereal);
        GET_OPT_NAMED_BITFIELD(ctx, "is_variable_size", t->is_variable_size);
        GET_OPT_NAMED_BITFIELD(ctx, "is_variable_size", t->is_variable_size);
        GET_OPT_NAMED_BITFIELD(ctx, "is_magical_weapon", t->is_magical_weapon);
        GET_OPT_NAMED_BITFIELD(ctx, "is_ranged_weapon", t->is_ranged_weapon);
        GET_OPT_NAMED_BITFIELD(ctx, "is_melee_weapon", t->is_melee_weapon);
        GET_OPT_NAMED_BITFIELD(ctx, "is_poison", t->is_poison);
        GET_OPT_NAMED_BITFIELD(ctx, "is_cloud_effect", t->is_cloud_effect);
        GET_OPT_NAMED_BITFIELD(ctx, "is_powerup", t->is_powerup);
        GET_OPT_NAMED_BITFIELD(ctx, "is_damaged_on_firing", t->is_damaged_on_firing);
        GET_OPT_NAMED_BITFIELD(ctx, "is_hard", t->is_hard);
        GET_OPT_NAMED_BITFIELD(ctx, "is_sleeping", t->is_sleeping);
        GET_OPT_NAMED_BITFIELD(ctx, "is_bomb", t->is_bomb);
        GET_OPT_NAMED_BITFIELD(ctx, "is_sawblade", t->is_sawblade);
        GET_OPT_NAMED_BITFIELD(ctx, "is_action_text", t->is_action_text);
        GET_OPT_NAMED_BITFIELD(ctx, "is_visible_on_debug_only", t->is_visible_on_debug_only);
        GET_OPT_NAMED_BITFIELD(ctx, "is_action", t->is_action);
        GET_OPT_NAMED_BITFIELD(ctx, "is_action_sleep", t->is_action_sleep);
        GET_OPT_NAMED_BITFIELD(ctx, "is_action_zap", t->is_action_zap);
        GET_OPT_NAMED_BITFIELD(ctx, "is_action_trigger", t->is_action_trigger);
        GET_OPT_NAMED_BITFIELD(ctx, "is_action_trigger_on_hero", t->is_action_trigger_on_hero);
        GET_OPT_NAMED_BITFIELD(ctx, "is_action_trigger_on_wall", t->is_action_trigger_on_wall);
        GET_OPT_NAMED_BITFIELD(ctx, "is_action_trigger_on_monst", t->is_action_trigger_on_monst);
        GET_OPT_NAMED_BITFIELD(ctx, "is_action_down", t->is_action_down);
        GET_OPT_NAMED_BITFIELD(ctx, "is_action_up", t->is_action_up);
        GET_OPT_NAMED_BITFIELD(ctx, "is_action_left", t->is_action_left);
        GET_OPT_NAMED_BITFIELD(ctx, "is_action_right", t->is_action_right);
        GET_OPT_NAMED_BITFIELD(ctx, "can_walk_through", t->can_walk_through);
        GET_OPT_NAMED_BITFIELD(ctx, "is_weapon_carry_anim", t->is_weapon_carry_anim);
        GET_OPT_NAMED_BITFIELD(ctx, "is_shield_carry_anim", t->is_shield_carry_anim);
        GET_OPT_NAMED_BITFIELD(ctx, "is_powerup_anim", t->is_powerup_anim);
        GET_OPT_NAMED_BITFIELD(ctx, "is_spell", t->is_spell);
        GET_OPT_NAMED_BITFIELD(ctx, "is_hand_item", t->is_hand_item);
        GET_OPT_NAMED_BITFIELD(ctx, "is_boots", t->is_boots);
        GET_OPT_NAMED_BITFIELD(ctx, "is_helmet", t->is_helmet);
        GET_OPT_NAMED_BITFIELD(ctx, "is_armour", t->is_armour);
        GET_OPT_NAMED_BITFIELD(ctx, "is_given_randomly_at_start", t->is_given_randomly_at_start);
        GET_OPT_NAMED_BITFIELD(ctx, "is_magical", t->is_magical);
        GET_OPT_NAMED_BITFIELD(ctx, "is_degradable", t->is_degradable);
        GET_OPT_NAMED_BITFIELD(ctx, "is_cursed", t->is_cursed);
        GET_OPT_NAMED_BITFIELD(ctx, "is_animate_only_when_moving", t->is_animate_only_when_moving);
        GET_OPT_NAMED_BITFIELD(ctx, "is_warm_blooded", t->is_warm_blooded);
        GET_OPT_NAMED_BITFIELD(ctx, "can_be_enchanted", t->can_be_enchanted);
        GET_OPT_NAMED_BITFIELD(ctx, "is_stackable", t->is_stackable);
        GET_OPT_NAMED_BITFIELD(ctx, "is_torch", t->is_torch);
        GET_OPT_NAMED_BITFIELD(ctx, "is_explosion", t->is_explosion);
        GET_OPT_NAMED_BITFIELD(ctx, "is_hidden_from_editor", t->is_hidden_from_editor);
        GET_OPT_NAMED_BITFIELD(ctx, "is_animated", t->is_animated);
        GET_OPT_NAMED_BITFIELD(ctx, "is_combustable", t->is_combustable);
        GET_OPT_NAMED_BITFIELD(ctx, "is_projectile", t->is_projectile);
        GET_OPT_NAMED_BITFIELD(ctx, "is_inactive", t->is_inactive);
        GET_OPT_NAMED_BITFIELD(ctx, "is_joinable", t->is_joinable);
        GET_OPT_NAMED_BITFIELD(ctx, "is_wall", t->is_wall);
        GET_OPT_NAMED_BITFIELD(ctx, "is_effect_sway", t->is_effect_sway);
        GET_OPT_NAMED_BITFIELD(ctx, "is_effect_pulse", t->is_effect_pulse);
        GET_OPT_NAMED_BITFIELD(ctx, "is_effect_rotate_4way", t->is_effect_rotate_4way);
        GET_OPT_NAMED_BITFIELD(ctx, "is_effect_rotate_2way", t->is_effect_rotate_2way);

    } while (demarshal_gotone(ctx));

    if (t->scale != 0.0) {
        if (t->collision_radius == 0) {
            t->collision_radius = t->scale;
        }
    }

    if (t->light_tint) {
        t->light_color = color_find(t->light_tint);
    } else {
        t->light_color = WHITE;
    }

    if (!t->item.quantity) {
        t->item.quantity = 1;
    }

    if (t->is_player || t->is_projectile) {
        t->tx_map_update_delay_thousandths = 
                        DELAY_THOUSANDTHS_TX_MAP_UPDATE_FAST;
    } else {
        t->tx_map_update_delay_thousandths = 
                        DELAY_THOUSANDTHS_TX_MAP_UPDATE_SLOW;
    }

    /*
     * Set up any stats not populated.
     */
    thing_stats_init(&t->stats);

    /*
     * Read the tiles tree.
     */
    demarshal_thing_tiles(ctx, t);
    demarshal_thing_tiles2(ctx, t);

    myfree(name);

    GET_KET(ctx);
}

void marshal_thing_template (marshal_p ctx, tpp t)
{
    PUT_BRA(ctx);

    PUT_NAMED_STRING(ctx, "name", t->tree.key);
    PUT_NAMED_STRING(ctx, "short_name", t->short_name);
    PUT_NAMED_STRING(ctx, "raw_name", t->raw_name);
    PUT_NAMED_STRING(ctx, "tooltip", t->tooltip);

    if (t->fires) {
        PUT_NAMED_STRING(ctx, "fires", tp_name(t->fires));
    }

    PUT_NAMED_STRING(ctx, "polymorph_on_death", t->polymorph_on_death);
    PUT_NAMED_STRING(ctx, "carried_as", t->carried_as);
    PUT_NAMED_STRING(ctx, "light_tint", t->light_tint);
    PUT_NAMED_STRING(ctx, "spawn_on_death", t->spawn_on_death);
    PUT_NAMED_STRING(ctx, "explodes_as", t->explodes_as);
    PUT_NAMED_STRING(ctx, "sound_on_creation", t->sound_on_creation);
    PUT_NAMED_STRING(ctx, "sound_on_hitting_something", t->sound_on_hitting_something);
    PUT_NAMED_STRING(ctx, "sound_on_death", t->sound_on_death);
    PUT_NAMED_STRING(ctx, "sound_on_collect", t->sound_on_collect);
    PUT_NAMED_STRING(ctx, "sound_random", t->sound_random);
    PUT_NAMED_STRING(ctx, "weapon_carry_anim", t->weapon_carry_anim);
    PUT_NAMED_STRING(ctx, "shield_carry_anim", t->shield_carry_anim);
    PUT_NAMED_STRING(ctx, "magic_anim", t->magic_anim);
    PUT_NAMED_STRING(ctx, "weapon_swing_anim", t->weapon_swing_anim);
    PUT_NAMED_STRING(ctx, "message_on_use", t->message_on_use);
    PUT_NAMED_UINT8(ctx, "z_depth", t->z_depth);
    PUT_NAMED_UINT8(ctx, "z_order", t->z_order);
    PUT_NAMED_INT32(ctx, "speed", t->speed);
    PUT_NAMED_INT32(ctx, "damage", t->damage);
    PUT_NAMED_INT32(ctx, "cost", t->cost);
    PUT_NAMED_INT32(ctx, "lifespan", t->lifespan);
    PUT_NAMED_INT32(ctx, "bonus_xp_on_death", t->bonus_xp_on_death);
    PUT_NAMED_INT32(ctx, "vision_distance", t->vision_distance);
    PUT_NAMED_INT32(ctx, "approach_distance", t->approach_distance);
    PUT_NAMED_INT32(ctx, "bonus_cash_on_collect", t->bonus_cash_on_collect);
    PUT_NAMED_INT32(ctx, "d10000_chance_of_appearing", t->d10000_chance_of_appearing);
    PUT_NAMED_INT32(ctx, "ppp2", t->ppp2);
    PUT_NAMED_INT32(ctx, "ppp3", t->ppp3);
    PUT_NAMED_INT32(ctx, "ppp4", t->ppp4);
    PUT_NAMED_INT32(ctx, "ppp5", t->ppp5);
    PUT_NAMED_INT32(ctx, "ppp6", t->ppp6);
    PUT_NAMED_INT32(ctx, "ppp7", t->ppp7);
    PUT_NAMED_INT32(ctx, "min_appear_depth", t->min_appear_depth);
    PUT_NAMED_INT32(ctx, "max_appear_depth", t->max_appear_depth);
    PUT_NAMED_INT32(ctx, "jump_speed", t->jump_speed);
    PUT_NAMED_INT32(ctx, "max_hp", t->stats.max_hp);
    PUT_NAMED_INT32(ctx, "max_magic", t->stats.max_magic);
    PUT_NAMED_INT32(ctx, "stats_attack_melee", t->stats.attack_melee);
    PUT_NAMED_INT32(ctx, "stats_attack_ranged", t->stats.attack_ranged);
    PUT_NAMED_INT32(ctx, "stats_attack_magical", t->stats.attack_magical);
    PUT_NAMED_INT32(ctx, "stats_speed", t->stats.speed);
    PUT_NAMED_INT32(ctx, "stats_vision", t->stats.vision);
    PUT_NAMED_INT32(ctx, "stats_healing", t->stats.healing);
    PUT_NAMED_INT32(ctx, "stats_defense", t->stats.defense);
    PUT_NAMED_INT32(ctx, "hp_per_level", t->hp_per_level);
    PUT_NAMED_INT32(ctx, "id_per_level", t->id_per_level);
    PUT_NAMED_FLOAT(ctx, "light_radius", t->light_radius);
    PUT_NAMED_FLOAT(ctx, "weapon_spread", t->weapon_spread);
    PUT_NAMED_FLOAT(ctx, "weapon_density", t->weapon_density);
    PUT_NAMED_FLOAT(ctx, "scale", t->scale);
    PUT_NAMED_FLOAT(ctx, "explosion_radius", t->explosion_radius);
    PUT_NAMED_FLOAT(ctx, "collision_radius", t->collision_radius);
    PUT_NAMED_INT32(ctx, "quantity", t->item.quantity);
    PUT_NAMED_INT32(ctx, "hit_priority", t->hit_priority);
    PUT_NAMED_INT32(ctx, "weapon_fire_delay_hundredths", t->weapon_fire_delay_hundredths);
    PUT_NAMED_INT32(ctx, "sound_random_delay_secs", t->sound_random_delay_secs);
    PUT_NAMED_INT32(ctx, "swing_distance_from_player", t->swing_distance_from_player);
    PUT_NAMED_INT32(ctx, "bonus_hp_on_use", t->bonus_hp_on_use);
    PUT_NAMED_INT32(ctx, "bonus_magic_on_use", t->bonus_magic_on_use);
    PUT_NAMED_INT32(ctx, "tx_map_update_delay_thousandths", t->tx_map_update_delay_thousandths);
    PUT_NAMED_INT32(ctx, "can_be_hit_chance", t->can_be_hit_chance);
    PUT_NAMED_INT32(ctx, "d10000_chance_of_breaking", t->d10000_chance_of_breaking);
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
    PUT_NAMED_BITFIELD(ctx, "is_collision_map_large", t->is_collision_map_large);
    PUT_NAMED_BITFIELD(ctx, "is_collision_map_tiny", t->is_collision_map_tiny);
    PUT_NAMED_BITFIELD(ctx, "is_collision_map_medium", t->is_collision_map_medium);
    PUT_NAMED_BITFIELD(ctx, "is_collision_map_small", t->is_collision_map_small);
    PUT_NAMED_BITFIELD(ctx, "is_rock", t->is_rock);
    PUT_NAMED_BITFIELD(ctx, "is_shadow_caster", t->is_shadow_caster);
    PUT_NAMED_BITFIELD(ctx, "is_shadow_caster_soft", t->is_shadow_caster_soft);
    PUT_NAMED_BITFIELD(ctx, "is_weapon", t->is_weapon);
    PUT_NAMED_BITFIELD(ctx, "is_treasure", t->is_treasure);
    PUT_NAMED_BITFIELD(ctx, "is_wearable", t->is_wearable);
    PUT_NAMED_BITFIELD(ctx, "is_fragile", t->is_fragile);
    PUT_NAMED_BITFIELD(ctx, "is_star", t->is_star);
    PUT_NAMED_BITFIELD(ctx, "is_animated_no_dir", t->is_animated_no_dir);
    PUT_NAMED_BITFIELD(ctx, "is_weapon_swing_effect", t->is_weapon_swing_effect);
    PUT_NAMED_BITFIELD(ctx, "is_light_source", t->is_light_source);
    PUT_NAMED_BITFIELD(ctx, "is_candle_light", t->is_candle_light);
    PUT_NAMED_BITFIELD(ctx, "is_cats_eyes", t->is_cats_eyes);
    PUT_NAMED_BITFIELD(ctx, "is_fire", t->is_fire);
    PUT_NAMED_BITFIELD(ctx, "is_ring", t->is_ring);
    PUT_NAMED_BITFIELD(ctx, "is_animation", t->is_animation);
    PUT_NAMED_BITFIELD(ctx, "is_non_explosive_gas_cloud", t->is_non_explosive_gas_cloud);
    PUT_NAMED_BITFIELD(ctx, "is_carryable", t->is_carryable);
    PUT_NAMED_BITFIELD(ctx, "is_item_unusable", t->is_item_unusable);
    PUT_NAMED_BITFIELD(ctx, "is_valid_for_action_bar", t->is_valid_for_action_bar);
    PUT_NAMED_BITFIELD(ctx, "is_door", t->is_door);
    PUT_NAMED_BITFIELD(ctx, "is_mob_spawner", t->is_mob_spawner);
    PUT_NAMED_BITFIELD(ctx, "is_rrr1", t->is_rrr1);
    PUT_NAMED_BITFIELD(ctx, "is_rrr2", t->is_rrr2);
    PUT_NAMED_BITFIELD(ctx, "is_rrr3", t->is_rrr3);
    PUT_NAMED_BITFIELD(ctx, "is_rrr4", t->is_rrr4);
    PUT_NAMED_BITFIELD(ctx, "has_ability_burst_shot", t->has_ability_burst_shot);
    PUT_NAMED_BITFIELD(ctx, "is_rrr6", t->is_rrr6);
    PUT_NAMED_BITFIELD(ctx, "has_ability_triple_shot", t->has_ability_triple_shot);
    PUT_NAMED_BITFIELD(ctx, "has_ability_double_shot", t->has_ability_double_shot);
    PUT_NAMED_BITFIELD(ctx, "has_ability_reverse_shot", t->has_ability_reverse_shot);
    PUT_NAMED_BITFIELD(ctx, "is_jesus", t->is_jesus);
    PUT_NAMED_BITFIELD(ctx, "is_blit_y_offset", t->is_blit_y_offset);
    PUT_NAMED_BITFIELD(ctx, "is_dirt", t->is_dirt);
    PUT_NAMED_BITFIELD(ctx, "is_dragon", t->is_dragon);
    PUT_NAMED_BITFIELD(ctx, "is_generator", t->is_generator);
    PUT_NAMED_BITFIELD(ctx, "is_water", t->is_water);
    PUT_NAMED_BITFIELD(ctx, "is_undead", t->is_undead);
    PUT_NAMED_BITFIELD(ctx, "is_hidden", t->is_hidden);
    PUT_NAMED_BITFIELD(ctx, "is_trap", t->is_trap);
    PUT_NAMED_BITFIELD(ctx, "is_corridor_wall", t->is_corridor_wall);
    PUT_NAMED_BITFIELD(ctx, "is_corridor", t->is_corridor);
    PUT_NAMED_BITFIELD(ctx, "is_water_proof", t->is_water_proof);
    PUT_NAMED_BITFIELD(ctx, "is_lava_proof", t->is_lava_proof);
    PUT_NAMED_BITFIELD(ctx, "is_spider_proof", t->is_spider_proof);
    PUT_NAMED_BITFIELD(ctx, "is_acid_proof", t->is_acid_proof);
    PUT_NAMED_BITFIELD(ctx, "is_fires_when_angry", t->is_fires_when_angry);
    PUT_NAMED_BITFIELD(ctx, "is_shopkeeper", t->is_shopkeeper);
    PUT_NAMED_BITFIELD(ctx, "is_shop_floor", t->is_shop_floor);
    PUT_NAMED_BITFIELD(ctx, "is_fireball", t->is_fireball);
    PUT_NAMED_BITFIELD(ctx, "is_bullet", t->is_bullet);
    PUT_NAMED_BITFIELD(ctx, "is_treasure_eater", t->is_treasure_eater);
    PUT_NAMED_BITFIELD(ctx, "is_effect_fade_in_out", t->is_effect_fade_in_out);
    PUT_NAMED_BITFIELD(ctx, "is_internal", t->is_internal);
    PUT_NAMED_BITFIELD(ctx, "is_levitating", t->is_levitating);
    PUT_NAMED_BITFIELD(ctx, "is_acid", t->is_acid);
    PUT_NAMED_BITFIELD(ctx, "is_lava", t->is_lava);
    PUT_NAMED_BITFIELD(ctx, "is_teleport", t->is_teleport);
    PUT_NAMED_BITFIELD(ctx, "is_potion", t->is_potion);
    PUT_NAMED_BITFIELD(ctx, "is_shield", t->is_shield);
    PUT_NAMED_BITFIELD(ctx, "is_death", t->is_death);
    PUT_NAMED_BITFIELD(ctx, "is_cobweb", t->is_cobweb);
    PUT_NAMED_BITFIELD(ctx, "is_ethereal", t->is_ethereal);
    PUT_NAMED_BITFIELD(ctx, "is_variable_size", t->is_variable_size);
    PUT_NAMED_BITFIELD(ctx, "is_variable_size", t->is_variable_size);
    PUT_NAMED_BITFIELD(ctx, "is_magical_weapon", t->is_magical_weapon);
    PUT_NAMED_BITFIELD(ctx, "is_ranged_weapon", t->is_ranged_weapon);
    PUT_NAMED_BITFIELD(ctx, "is_melee_weapon", t->is_melee_weapon);
    PUT_NAMED_BITFIELD(ctx, "is_poison", t->is_poison);
    PUT_NAMED_BITFIELD(ctx, "is_cloud_effect", t->is_cloud_effect);
    PUT_NAMED_BITFIELD(ctx, "is_powerup", t->is_powerup);
    PUT_NAMED_BITFIELD(ctx, "is_damaged_on_firing", t->is_damaged_on_firing);
    PUT_NAMED_BITFIELD(ctx, "is_hard", t->is_hard);
    PUT_NAMED_BITFIELD(ctx, "is_sleeping", t->is_sleeping);
    PUT_NAMED_BITFIELD(ctx, "is_bomb", t->is_bomb);
    PUT_NAMED_BITFIELD(ctx, "is_sawblade", t->is_sawblade);
    PUT_NAMED_BITFIELD(ctx, "is_action_text", t->is_action_text);
    PUT_NAMED_BITFIELD(ctx, "is_visible_on_debug_only", t->is_visible_on_debug_only);
    PUT_NAMED_BITFIELD(ctx, "is_action", t->is_action);
    PUT_NAMED_BITFIELD(ctx, "is_action_sleep", t->is_action_sleep);
    PUT_NAMED_BITFIELD(ctx, "is_action_zap", t->is_action_zap);
    PUT_NAMED_BITFIELD(ctx, "is_action_trigger_on_hero", t->is_action_trigger_on_hero);
    PUT_NAMED_BITFIELD(ctx, "is_action_trigger_on_monst", t->is_action_trigger_on_monst);
    PUT_NAMED_BITFIELD(ctx, "is_action_trigger_on_wall", t->is_action_trigger_on_wall);
    PUT_NAMED_BITFIELD(ctx, "is_action_down", t->is_action_down);
    PUT_NAMED_BITFIELD(ctx, "is_action_up", t->is_action_up);
    PUT_NAMED_BITFIELD(ctx, "is_action_left", t->is_action_left);
    PUT_NAMED_BITFIELD(ctx, "is_action_right", t->is_action_right);
    PUT_NAMED_BITFIELD(ctx, "can_walk_through", t->can_walk_through);
    PUT_NAMED_BITFIELD(ctx, "is_weapon_carry_anim", t->is_weapon_carry_anim);
    PUT_NAMED_BITFIELD(ctx, "is_powerup_anim", t->is_powerup_anim);
    PUT_NAMED_BITFIELD(ctx, "is_spell", t->is_spell);
    PUT_NAMED_BITFIELD(ctx, "is_hand_item", t->is_hand_item);
    PUT_NAMED_BITFIELD(ctx, "is_boots", t->is_boots);
    PUT_NAMED_BITFIELD(ctx, "is_helmet", t->is_helmet);
    PUT_NAMED_BITFIELD(ctx, "is_armour", t->is_armour);
    PUT_NAMED_BITFIELD(ctx, "is_given_randomly_at_start", t->is_given_randomly_at_start);
    PUT_NAMED_BITFIELD(ctx, "is_magical", t->is_magical);
    PUT_NAMED_BITFIELD(ctx, "is_degradable", t->is_degradable);
    PUT_NAMED_BITFIELD(ctx, "is_cursed", t->is_cursed);
    PUT_NAMED_BITFIELD(ctx, "is_animate_only_when_moving", t->is_animate_only_when_moving);
    PUT_NAMED_BITFIELD(ctx, "is_warm_blooded", t->is_warm_blooded);
    PUT_NAMED_BITFIELD(ctx, "can_be_enchanted", t->can_be_enchanted);
    PUT_NAMED_BITFIELD(ctx, "is_stackable", t->is_stackable);
    PUT_NAMED_BITFIELD(ctx, "is_torch", t->is_torch);
    PUT_NAMED_BITFIELD(ctx, "is_explosion", t->is_explosion);
    PUT_NAMED_BITFIELD(ctx, "is_hidden_from_editor", t->is_hidden_from_editor);
    PUT_NAMED_BITFIELD(ctx, "is_animated", t->is_animated);
    PUT_NAMED_BITFIELD(ctx, "is_combustable", t->is_combustable);
    PUT_NAMED_BITFIELD(ctx, "is_projectile", t->is_projectile);
    PUT_NAMED_BITFIELD(ctx, "is_inactive", t->is_inactive);
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

const char *tp_name (tpp t)
{
    return (t->tree.key);
}

const char *tp_short_name (tpp t)
{
    return (t->short_name);
}

const char *tp_raw_name (tpp t)
{
    return (t->raw_name);
}

tpp tp_fires (tpp t)
{
    return (t->fires);
}

const char *tp_polymorph_on_death (tpp t)
{
    return (t->polymorph_on_death);
}

const char *tp_carried_as (tpp t)
{
    return (t->carried_as);
}

const char *tp_light_tint (tpp t)
{
    return (t->light_tint);
}

color tp_light_color (tpp t)
{
    return (t->light_color);
}

const char *tp_spawn_on_death (tpp t)
{
    return (t->spawn_on_death);
}

const char *tp_explodes_as (tpp t)
{
    return (t->explodes_as);
}

const char *tp_sound_on_creation (tpp t)
{
    return (t->sound_on_creation);
}

const char *tp_sound_on_hitting_something (tpp t)
{
    return (t->sound_on_hitting_something);
}

const char *tp_sound_on_death (tpp t)
{
    return (t->sound_on_death);
}

const char *tp_sound_on_collect (tpp t)
{
    return (t->sound_on_collect);
}

const char *tp_weapon_carry_anim (tpp t)
{
    return (t->weapon_carry_anim);
}

const char *tp_weapon_swing_anim (tpp t)
{
    return (t->weapon_swing_anim);
}

const char *tp_shield_carry_anim (tpp t)
{
    return (t->shield_carry_anim);
}

const char *tp_magic_anim (tpp t)
{
    return (t->magic_anim);
}

const char *tp_message_on_use (tpp t)
{
    return (t->message_on_use);
}

const char *tp_mob_spawn (tpp t)
{
    if (!t->mob_spawn_count) {
        return (0);
    }

    for (;;) {
        int i = myrand() % t->mob_spawn_count;
        int r = myrand() % 1000;

        if (r < t->mob_spawn_chance_d1000[i]) {
            return (t->mob_spawn_what[i]);
        }
    }
}

const char *tp_get_tooltip (tpp t)
{
    return (t->tooltip);
}

uint8_t tp_get_z_depth (tpp t)
{
    return (t->z_depth);
}

uint8_t tp_get_z_order (tpp t)
{
    return (t->z_order);
}

uint32_t tp_get_speed (tpp t)
{
    return (t->speed);
}

uint16_t tp_get_damage (tpp t)
{
    return (t->damage);
}

uint16_t tp_get_cost (tpp t)
{
    return (t->cost);
}

uint32_t tp_get_lifespan (tpp t)
{
    return (t->lifespan);
}

int32_t tp_get_bonus_xp_on_death (tpp t)
{
    return (t->bonus_xp_on_death);
}

uint32_t tp_get_vision_distance (tpp t)
{
    return (t->vision_distance);
}

uint32_t tp_get_approach_distance (tpp t)
{
    return (t->approach_distance);
}

int32_t tp_get_bonus_cash_on_collect (tpp t)
{
    return (t->bonus_cash_on_collect);
}

uint32_t tp_get_d10000_chance_of_appearing (tpp t)
{
    return (t->d10000_chance_of_appearing);
}

uint32_t tp_get_ppp2 (tpp t)
{
    return (t->ppp2);
}

uint32_t tp_get_ppp3 (tpp t)
{
    return (t->ppp3);
}

uint32_t tp_get_ppp4 (tpp t)
{
    return (t->ppp4);
}

uint32_t tp_get_ppp5 (tpp t)
{
    return (t->ppp5);
}

uint32_t tp_get_ppp6 (tpp t)
{
    return (t->ppp6);
}

uint32_t tp_get_ppp7 (tpp t)
{
    return (t->ppp7);
}

uint32_t tp_get_min_appear_depth (tpp t)
{
    return (t->min_appear_depth);
}

uint32_t tp_get_max_appear_depth (tpp t)
{
    return (t->max_appear_depth);
}

uint32_t tp_get_jump_speed (tpp t)
{
    return (t->jump_speed);
}

int16_t tp_get_stats_max_hp (tpp t)
{
    return (t->stats.max_hp);
}

int16_t tp_get_stats_max_magic (tpp t)
{
    return (t->stats.max_magic);
}

uint32_t tp_get_stats_attack_melee (tpp t)
{
    return (t->stats.attack_melee);
}

uint32_t tp_get_stats_attack_ranged (tpp t)
{
    return (t->stats.attack_ranged);
}

uint32_t tp_get_stats_attack_magical (tpp t)
{
    return (t->stats.attack_magical);
}

uint32_t tp_get_stats_speed (tpp t)
{
    return (t->stats.speed);
}

uint32_t tp_get_stats_vision (tpp t)
{
    return (t->stats.vision);
}

uint32_t tp_get_stats_healing (tpp t)
{
    return (t->stats.healing);
}

uint32_t tp_get_stats_cash (tpp t)
{
    return (t->stats.cash);
}

uint32_t tp_get_stats_defense (tpp t)
{
    return (t->stats.defense);
}

uint32_t tp_get_hp_per_level (tpp t)
{
    return (t->hp_per_level);
}

uint32_t tp_get_id_per_level (tpp t)
{
    return (t->id_per_level);
}

double tp_get_light_radius (tpp t)
{
    return (t->light_radius);
}

double tp_get_weapon_density (tpp t)
{
    return (t->weapon_density);
}

double tp_get_weapon_spread (tpp t)
{
    return (t->weapon_spread);
}

double tp_get_scale (tpp t)
{
    if (!t->scale) {
        return (1.0);
    }

    return (t->scale);
}

double tp_get_explosion_radius (tpp t)
{
    if (!t->explosion_radius) {
        return (1.0);
    }

    return (t->explosion_radius);
}

double tp_get_collision_radius (tpp t)
{
    return (t->collision_radius);
}

uint32_t tp_get_quantity (tpp t)
{
    return (t->item.quantity);
}

uint32_t tp_get_hit_priority (tpp t)
{
    return (t->hit_priority);
}

uint32_t tp_get_weapon_fire_delay_hundredths (tpp t)
{
    return (t->weapon_fire_delay_hundredths);
}

uint32_t tp_get_sound_random_delay_secs (tpp t)
{
    return (t->sound_random_delay_secs);
}

uint32_t tp_get_swing_distance_from_player (tpp t)
{
    return (t->swing_distance_from_player);
}

int16_t tp_get_bonus_hp_on_use (tpp t)
{
    return (t->bonus_hp_on_use);
}

int16_t tp_get_bonus_magic_on_use (tpp t)
{
    return (t->bonus_magic_on_use);
}

uint32_t tp_get_tx_map_update_delay_thousandths (tpp t)
{
    return (t->tx_map_update_delay_thousandths);
}

uint32_t tp_get_can_be_hit_chance (tpp t)
{
    return (t->can_be_hit_chance);
}

uint32_t tp_get_d10000_chance_of_breaking (tpp t)
{
    return (t->d10000_chance_of_breaking);
}

uint32_t tp_get_hit_delay_tenths (tpp t)
{
    return (t->hit_delay_tenths);
}

uint32_t tp_get_mob_spawn_delay_tenths (tpp t)
{
    return (t->mob_spawn_delay_tenths);
}

tree_rootp tp_get_tiles (tpp t)
{
    return (t->tiles);
}

tree_rootp tp_get_tiles2 (tpp t)
{
    return (t->tiles2);
}
