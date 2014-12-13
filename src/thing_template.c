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
            DIE("no tile for join index %d for %s, tile name %s",
                index, t->short_name, thing_tile_name(thing_tile));
        }

        t->tilep_join_tile[index][ t->tilep_join_count[index] ] = tile;

        if (t->tilep_join_count[index]++ >= IS_JOIN_ALT_MAX) {
            DIE("too many tile join alternatives for %s", t->short_name);
        }
    }
}

tpp tp_load (uint16_t *id, const char *name)
{
    tpp t;
    demarshal_p in;

    if (tp_find(name)) {
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

    tp_fill_cache(t);

    DBG("thing template: %s", name);

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

    if (t->weapon_carry_anim) {
        myfree(t->weapon_carry_anim);
    }

    if (t->weapon_swing_anim) {
        myfree(t->weapon_swing_anim);
    }

    if (t->message_on_use) {
        myfree(t->message_on_use);
    }

    if (t->mob_spawn) {
        myfree(t->mob_spawn);
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

    DIE("did not find short template name %s", name);

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
        DIE("unknown thing [%s]", tmp);
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

void demarshal_thing_template (demarshal_p ctx, tpp t)
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
        GET_OPT_NAMED_STRING(ctx, "weapon_carry_anim", t->weapon_carry_anim);
        GET_OPT_NAMED_STRING(ctx, "weapon_swing_anim", t->weapon_swing_anim);
        GET_OPT_NAMED_STRING(ctx, "message_on_use", t->message_on_use);
        GET_OPT_NAMED_STRING(ctx, "mob_spawn", t->mob_spawn);
        GET_OPT_NAMED_UINT8(ctx, "z_depth", t->z_depth);
        GET_OPT_NAMED_UINT8(ctx, "z_order", t->z_order);
        GET_OPT_NAMED_UINT16(ctx, "speed", t->speed);
        GET_OPT_NAMED_UINT16(ctx, "damage", t->damage);
        GET_OPT_NAMED_UINT16(ctx, "lifespan", t->lifespan);
        GET_OPT_NAMED_UINT8(ctx, "vision_distance", t->vision_distance);
        GET_OPT_NAMED_INT32(ctx, "bonus_xp_on_death", t->bonus_xp_on_death);
        GET_OPT_NAMED_INT32(ctx, "bonus_cash_on_collect", t->bonus_cash_on_collect);
        GET_OPT_NAMED_INT16(ctx, "bonus_hp_on_use", t->bonus_hp_on_use);
        GET_OPT_NAMED_INT16(ctx, "bonus_magic_on_use", t->bonus_magic_on_use);
        GET_OPT_NAMED_UINT32(ctx, "d10000_chance_of_appearing", t->d10000_chance_of_appearing);
        GET_OPT_NAMED_UINT32(ctx, "ppp2", t->ppp2);

        int16_t tmp_int16 = 0;
        uint32_t tmp_uint32 = 0;

        if (GET_OPT_NAMED_INT16(ctx, "max_hp", tmp_int16)) {
            t->stats.max_hp = tmp_int16;
        }

        if (GET_OPT_NAMED_INT16(ctx, "max_magic", tmp_int16)) {
            t->stats.max_magic = tmp_int16;
        }

        if (GET_OPT_NAMED_UINT32(ctx, "stats_attack_melee", tmp_uint32)) {
            t->stats.attack_melee = tmp_uint32;
        }

        if (GET_OPT_NAMED_UINT32(ctx, "stats_attack_ranged", tmp_uint32)) {
            t->stats.attack_ranged = tmp_uint32;
        }

        if (GET_OPT_NAMED_UINT32(ctx, "stats_attack_magical", tmp_uint32)) {
            t->stats.attack_magical = tmp_uint32;
        }

        if (GET_OPT_NAMED_UINT32(ctx, "stats_speed", tmp_uint32)) {
            t->stats.speed = tmp_uint32;
        }

        if (GET_OPT_NAMED_UINT32(ctx, "stats_vision", tmp_uint32)) {
            t->stats.vision = tmp_uint32;
        }

        if (GET_OPT_NAMED_UINT32(ctx, "stats_healing", tmp_uint32)) {
            t->stats.healing = tmp_uint32;
        }

        if (GET_OPT_NAMED_UINT32(ctx, "stats_defense", tmp_uint32)) {
            t->stats.defense = tmp_uint32;
        }

        GET_OPT_NAMED_UINT32(ctx, "hp_per_level", t->hp_per_level);
        GET_OPT_NAMED_UINT32(ctx, "id_per_level", t->id_per_level);

        GET_OPT_NAMED_FLOAT(ctx, "light_radius", t->light_radius);

        uint32_t quantity;

        if (GET_OPT_NAMED_UINT32(ctx, "quantity", quantity)) {
            t->item.quantity = quantity;
        }

        GET_OPT_NAMED_UINT32(ctx, "hit_priority", t->hit_priority);
        GET_OPT_NAMED_UINT32(ctx, "weapon_fire_delay_tenths", t->weapon_fire_delay_tenths);
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
        GET_OPT_NAMED_BITFIELD(ctx, "is_poison", t->is_poison);
        GET_OPT_NAMED_BITFIELD(ctx, "is_carryable", t->is_carryable);
        GET_OPT_NAMED_BITFIELD(ctx, "is_item_unusable", t->is_item_unusable);
        GET_OPT_NAMED_BITFIELD(ctx, "is_valid_for_action_bar", t->is_valid_for_action_bar);
        GET_OPT_NAMED_BITFIELD(ctx, "is_seedpod", t->is_seedpod);
        GET_OPT_NAMED_BITFIELD(ctx, "is_spam", t->is_spam);
        GET_OPT_NAMED_BITFIELD(ctx, "is_door", t->is_door);
        GET_OPT_NAMED_BITFIELD(ctx, "is_pipe", t->is_pipe);
        GET_OPT_NAMED_BITFIELD(ctx, "is_mob_spawner", t->is_mob_spawner);
        GET_OPT_NAMED_BITFIELD(ctx, "is_rrr1", t->is_rrr1);
        GET_OPT_NAMED_BITFIELD(ctx, "is_rrr2", t->is_rrr2);
        GET_OPT_NAMED_BITFIELD(ctx, "is_rrr3", t->is_rrr3);
        GET_OPT_NAMED_BITFIELD(ctx, "is_rrr4", t->is_rrr4);
        GET_OPT_NAMED_BITFIELD(ctx, "is_rrr5", t->is_rrr5);
        GET_OPT_NAMED_BITFIELD(ctx, "is_rrr6", t->is_rrr6);
        GET_OPT_NAMED_BITFIELD(ctx, "is_spell", t->is_spell);
        GET_OPT_NAMED_BITFIELD(ctx, "is_hand_item", t->is_hand_item);
        GET_OPT_NAMED_BITFIELD(ctx, "is_boots", t->is_boots);
        GET_OPT_NAMED_BITFIELD(ctx, "is_helmet", t->is_helmet);
        GET_OPT_NAMED_BITFIELD(ctx, "is_armor", t->is_armor);
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
        GET_OPT_NAMED_BITFIELD(ctx, "is_boring", t->is_boring);
        GET_OPT_NAMED_BITFIELD(ctx, "is_joinable", t->is_joinable);
        GET_OPT_NAMED_BITFIELD(ctx, "is_wall", t->is_wall);
        GET_OPT_NAMED_BITFIELD(ctx, "is_effect_sway", t->is_effect_sway);
        GET_OPT_NAMED_BITFIELD(ctx, "is_effect_pulse", t->is_effect_pulse);
        GET_OPT_NAMED_BITFIELD(ctx, "is_effect_rotate_4way", t->is_effect_rotate_4way);
        GET_OPT_NAMED_BITFIELD(ctx, "is_effect_rotate_2way", t->is_effect_rotate_2way);

    } while (demarshal_gotone(ctx));

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
    PUT_NAMED_STRING(ctx, "tooltip", t->tooltip);

    if (t->fires) {
        PUT_NAMED_STRING(ctx, "fires", tp_name(t->fires));
    }

    PUT_NAMED_STRING(ctx, "polymorph_on_death", t->polymorph_on_death);
    PUT_NAMED_STRING(ctx, "carried_as", t->carried_as);
    PUT_NAMED_STRING(ctx, "light_tint", t->light_tint);
    PUT_NAMED_STRING(ctx, "spawn_on_death", t->spawn_on_death);
    PUT_NAMED_STRING(ctx, "weapon_carry_anim", t->weapon_carry_anim);
    PUT_NAMED_STRING(ctx, "weapon_swing_anim", t->weapon_swing_anim);
    PUT_NAMED_STRING(ctx, "message_on_use", t->message_on_use);
    PUT_NAMED_STRING(ctx, "mob_spawn", t->mob_spawn);
    PUT_NAMED_UINT8(ctx, "z_depth", t->z_depth);
    PUT_NAMED_UINT8(ctx, "z_order", t->z_order);
    PUT_NAMED_INT32(ctx, "speed", t->speed);
    PUT_NAMED_INT32(ctx, "damage", t->damage);
    PUT_NAMED_INT32(ctx, "lifespan", t->lifespan);
    PUT_NAMED_INT32(ctx, "bonus_xp_on_death", t->bonus_xp_on_death);
    PUT_NAMED_INT32(ctx, "vision_distance", t->vision_distance);
    PUT_NAMED_INT32(ctx, "bonus_cash_on_collect", t->bonus_cash_on_collect);
    PUT_NAMED_INT32(ctx, "d10000_chance_of_appearing", t->d10000_chance_of_appearing);
    PUT_NAMED_INT32(ctx, "ppp2", t->ppp2);
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
    PUT_NAMED_INT32(ctx, "light_radius", t->light_radius);
    PUT_NAMED_INT32(ctx, "quantity", t->item.quantity);
    PUT_NAMED_INT32(ctx, "hit_priority", t->hit_priority);
    PUT_NAMED_INT32(ctx, "weapon_fire_delay_tenths", t->weapon_fire_delay_tenths);
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
    PUT_NAMED_BITFIELD(ctx, "is_poison", t->is_poison);
    PUT_NAMED_BITFIELD(ctx, "is_carryable", t->is_carryable);
    PUT_NAMED_BITFIELD(ctx, "is_item_unusable", t->is_item_unusable);
    PUT_NAMED_BITFIELD(ctx, "is_valid_for_action_bar", t->is_valid_for_action_bar);
    PUT_NAMED_BITFIELD(ctx, "is_seedpod", t->is_seedpod);
    PUT_NAMED_BITFIELD(ctx, "is_spam", t->is_spam);
    PUT_NAMED_BITFIELD(ctx, "is_door", t->is_door);
    PUT_NAMED_BITFIELD(ctx, "is_pipe", t->is_pipe);
    PUT_NAMED_BITFIELD(ctx, "is_mob_spawner", t->is_mob_spawner);
    PUT_NAMED_BITFIELD(ctx, "is_rrr1", t->is_rrr1);
    PUT_NAMED_BITFIELD(ctx, "is_rrr2", t->is_rrr2);
    PUT_NAMED_BITFIELD(ctx, "is_rrr3", t->is_rrr3);
    PUT_NAMED_BITFIELD(ctx, "is_rrr4", t->is_rrr4);
    PUT_NAMED_BITFIELD(ctx, "is_rrr5", t->is_rrr5);
    PUT_NAMED_BITFIELD(ctx, "is_rrr6", t->is_rrr6);
    PUT_NAMED_BITFIELD(ctx, "is_spell", t->is_spell);
    PUT_NAMED_BITFIELD(ctx, "is_hand_item", t->is_hand_item);
    PUT_NAMED_BITFIELD(ctx, "is_boots", t->is_boots);
    PUT_NAMED_BITFIELD(ctx, "is_helmet", t->is_helmet);
    PUT_NAMED_BITFIELD(ctx, "is_armor", t->is_armor);
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

const char *tp_name (tpp t)
{
    return (t->tree.key);
}

const char *tp_short_name (tpp t)
{
    return (t->short_name);
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

const char *tp_weapon_carry_anim (tpp t)
{
    return (t->weapon_carry_anim);
}

const char *tp_weapon_swing_anim (tpp t)
{
    return (t->weapon_swing_anim);
}

const char *tp_message_on_use (tpp t)
{
    return (t->message_on_use);
}

const char *tp_mob_spawn (tpp t)
{
    return (t->mob_spawn);
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

float tp_get_light_radius (tpp t)
{
    return (t->light_radius);
}

uint32_t tp_get_quantity (tpp t)
{
    return (t->item.quantity);
}

uint32_t tp_get_hit_priority (tpp t)
{
    return (t->hit_priority);
}

uint32_t tp_get_weapon_fire_delay_tenths (tpp t)
{
    return (t->weapon_fire_delay_tenths);
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
