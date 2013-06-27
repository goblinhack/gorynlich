/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <SDL.h>

#include "main.h"
#include "tree.h"
#include "thing_tile_private.h"
#include "thing_template.h"
#include "thing_tile.h"
#include "tile.h"
#include "marshal.h"

/*
 * Using static memory as these things never change once made.
 */
thing_template thing_templates_chunk[THING_TEMPLATES_CHUNK_COUNT_MAX];
static uint32_t thing_templates_chunk_count;

tree_root *thing_templates;
tree_root *thing_templates_create_order;

static boolean thing_template_init_done;
static void thing_template_destroy_internal(thing_templatep t);

boolean thing_template_init (void)
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
 * Convert a join type into an index
 */
uint8_t thing_template_join_type_to_index (
            boolean is_join_block,
            boolean is_join_horiz,
            boolean is_join_vert,
            boolean is_join_node,
            boolean is_join_left,
            boolean is_join_right,
            boolean is_join_top,
            boolean is_join_bot,
            boolean is_join_tl,
            boolean is_join_tr,
            boolean is_join_bl,
            boolean is_join_br,
            boolean is_join_t,
            boolean is_join_t90,
            boolean is_join_t180,
            boolean is_join_t270,
            boolean is_join_x,
            boolean is_join_tl2,
            boolean is_join_tr2,
            boolean is_join_bl2,
            boolean is_join_br2,
            boolean is_join_t_1,
            boolean is_join_t_2,
            boolean is_join_t_3,
            boolean is_join_t90_1,
            boolean is_join_t90_2,
            boolean is_join_t90_3,
            boolean is_join_t180_1,
            boolean is_join_t180_2,
            boolean is_join_t180_3,
            boolean is_join_t270_1,
            boolean is_join_t270_2,
            boolean is_join_t270_3,
            boolean is_join_x1,
            boolean is_join_x1_270,
            boolean is_join_x1_180,
            boolean is_join_x1_90,
            boolean is_join_x2,
            boolean is_join_x2_270,
            boolean is_join_x2_180,
            boolean is_join_x2_90,
            boolean is_join_x3,
            boolean is_join_x3_180,
            boolean is_join_x4,
            boolean is_join_x4_270,
            boolean is_join_x4_180,
            boolean is_join_x4_90)
{
    if (is_join_block) {
        return (IS_JOIN_BLOCK);
    } else if (is_join_t_1) {
        return (IS_JOIN_T_1);
    } else if (is_join_t_2) {
        return (IS_JOIN_T_2);
    } else if (is_join_t_3) {
        return (IS_JOIN_T_3);
    } else if (is_join_t90_1) {
        return (IS_JOIN_T90_1);
    } else if (is_join_t90_2) {
        return (IS_JOIN_T90_2);
    } else if (is_join_t90_3) {
        return (IS_JOIN_T90_3);
    } else if (is_join_t180_1) {
        return (IS_JOIN_T180_1);
    } else if (is_join_t180_2) {
        return (IS_JOIN_T180_2);
    } else if (is_join_t180_3) {
        return (IS_JOIN_T180_3);
    } else if (is_join_t270_1) {
        return (IS_JOIN_T270_1);
    } else if (is_join_t270_2) {
        return (IS_JOIN_T270_2);
    } else if (is_join_t270_3) {
        return (IS_JOIN_T270_3);
    } else if (is_join_block) {
        return (IS_JOIN_BLOCK);
    } else if (is_join_horiz) {
        return (IS_JOIN_HORIZ);
    } else if (is_join_vert) {
        return (IS_JOIN_VERT);
    } else if (is_join_node) {
        return (IS_JOIN_NODE);
    } else if (is_join_left) {
        return (IS_JOIN_LEFT);
    } else if (is_join_right) {
        return (IS_JOIN_RIGHT);
    } else if (is_join_top) {
        return (IS_JOIN_TOP);
    } else if (is_join_bot) {
        return (IS_JOIN_BOT);
    } else if (is_join_tl) {
        return (IS_JOIN_TL);
    } else if (is_join_tr) {
        return (IS_JOIN_TR);
    } else if (is_join_bl) {
        return (IS_JOIN_BL);
    } else if (is_join_br) {
        return (IS_JOIN_BR);
    } else if (is_join_t) {
        return (IS_JOIN_T);
    } else if (is_join_t90) {
        return (IS_JOIN_T90);
    } else if (is_join_t180) {
        return (IS_JOIN_T180);
    } else if (is_join_t270) {
        return (IS_JOIN_T270);
    } else if (is_join_x) {
        return (IS_JOIN_X);
    } else if (is_join_tl2) {
        return (IS_JOIN_TL2);
    } else if (is_join_tr2) {
        return (IS_JOIN_TR2);
    } else if (is_join_bl2) {
        return (IS_JOIN_BL2);
    } else if (is_join_br2) {
        return (IS_JOIN_BR2);
    } else if (is_join_x) {
        return (IS_JOIN_X);
    } else if (is_join_x1) {
        return (IS_JOIN_X1);
    } else if (is_join_x1_270) {
        return (IS_JOIN_X1_270);
    } else if (is_join_x1_180) {
        return (IS_JOIN_X1_180);
    } else if (is_join_x1_90) {
        return (IS_JOIN_X1_90);
    } else if (is_join_x2) {
        return (IS_JOIN_X2);
    } else if (is_join_x2_270) {
        return (IS_JOIN_X2_270);
    } else if (is_join_x2_180) {
        return (IS_JOIN_X2_180);
    } else if (is_join_x2_90) {
        return (IS_JOIN_X2_90);
    } else if (is_join_x3) {
        return (IS_JOIN_X3);
    } else if (is_join_x3_180) {
        return (IS_JOIN_X3_180);
    } else if (is_join_x4) {
        return (IS_JOIN_X4);
    } else if (is_join_x4_270) {
        return (IS_JOIN_X4_270);
    } else if (is_join_x4_180) {
        return (IS_JOIN_X4_180);
    } else if (is_join_x4_90) {
        return (IS_JOIN_X4_90);
    } else {
        return (0);
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
        } else {
            continue;
        }

        t->tilep_join[index][ t->tilep_join_count[index] ] = thing_tile;

        tile = tile_find(thing_tile_name(thing_tile));
        if (!tile) {
            DIE("no tile for join index %d for %s, tile name %s",
                index, t->shortname, thing_tile_name(thing_tile));
        }

        t->tilep_join_tile[index][ t->tilep_join_count[index] ] = tile;

        if (t->tilep_join_count[index]++ >= IS_JOIN_ALT_MAX) {
            DIE("too many tile join alternatives for %s", t->shortname);
        }
    }
}

thing_templatep thing_template_load (const char *name)
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

    if (thing_templates_chunk_count >= THING_TEMPLATES_CHUNK_COUNT_MAX - 1) {
        DIE("too many thing templates");
    }

    t = &thing_templates_chunk[++thing_templates_chunk_count];
    t->id = thing_templates_chunk_count;
    t->tree.key = dupstr(name, "TREE KEY: thing");

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

    myfree(t->tooltip);
    myfree(t->shortname);
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
            tmp = marshal(0); /* stdout */
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

void demarshal_thing_template (demarshal_p ctx, thing_templatep t)
{
    char *name;

    GET_BRA(ctx);

    GET_OPT_DEF_NAMED_STRING(ctx, "name", name, "<no name>");
    GET_OPT_DEF_NAMED_STRING(ctx, "shortname", t->shortname, "<no name>");
    GET_OPT_DEF_NAMED_STRING(ctx, "tooltip", t->tooltip, "<no name>");

    (void) demarshal_gotone(ctx);

    do {
        GET_OPT_NAMED_UINT8(ctx, "z_depth", t->z_depth);
        GET_OPT_NAMED_UINT8(ctx, "z_order", t->z_order);
        GET_OPT_NAMED_UINT32(ctx, "speed", t->speed);
        GET_OPT_NAMED_UINT32(ctx, "lifespan", t->lifespan);
        GET_OPT_NAMED_UINT32(ctx, "score_on_death", t->score_on_death);
        GET_OPT_NAMED_UINT32(ctx, "score_on_collect", t->score_on_collect);
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
        GET_OPT_NAMED_UINT32(ctx, "ppp15", t->ppp15);
        GET_OPT_NAMED_UINT32(ctx, "ppp16", t->ppp16);
        GET_OPT_NAMED_UINT32(ctx, "ppp17", t->ppp17);
        GET_OPT_NAMED_UINT32(ctx, "ppp18", t->ppp18);
        GET_OPT_NAMED_UINT32(ctx, "ppp19", t->ppp19);
        GET_OPT_NAMED_UINT32(ctx, "ppp20", t->ppp20);

        GET_OPT_NAMED_BITFIELD(ctx, "is_player", t->is_player);
        GET_OPT_NAMED_BITFIELD(ctx, "is_exit", t->is_exit);
        GET_OPT_NAMED_BITFIELD(ctx, "is_floor", t->is_floor);
        GET_OPT_NAMED_BITFIELD(ctx, "is_food", t->is_food);
        GET_OPT_NAMED_BITFIELD(ctx, "is_letter", t->is_letter);
        GET_OPT_NAMED_BITFIELD(ctx, "is_monst", t->is_monst);
        GET_OPT_NAMED_BITFIELD(ctx, "is_plant", t->is_plant);
        GET_OPT_NAMED_BITFIELD(ctx, "is_player", t->is_player);
        GET_OPT_NAMED_BITFIELD(ctx, "is_snail", t->is_snail);
        GET_OPT_NAMED_BITFIELD(ctx, "is_star_yellow", t->is_star_yellow);
        GET_OPT_NAMED_BITFIELD(ctx, "is_rock", t->is_rock);
        GET_OPT_NAMED_BITFIELD(ctx, "is_water", t->is_water);
        GET_OPT_NAMED_BITFIELD(ctx, "is_lava", t->is_lava);
        GET_OPT_NAMED_BITFIELD(ctx, "is_xxx4", t->is_xxx4);
        GET_OPT_NAMED_BITFIELD(ctx, "is_xxx5", t->is_xxx5);
        GET_OPT_NAMED_BITFIELD(ctx, "is_xxx6", t->is_xxx6);
        GET_OPT_NAMED_BITFIELD(ctx, "is_xxx7", t->is_xxx7);
        GET_OPT_NAMED_BITFIELD(ctx, "is_xxx8", t->is_xxx8);
        GET_OPT_NAMED_BITFIELD(ctx, "is_star", t->is_star);
        GET_OPT_NAMED_BITFIELD(ctx, "is_powerup_spam", t->is_powerup_spam);
        GET_OPT_NAMED_BITFIELD(ctx, "is_rock_0", t->is_rock_0);
        GET_OPT_NAMED_BITFIELD(ctx, "is_car", t->is_car);
        GET_OPT_NAMED_BITFIELD(ctx, "is_star_green", t->is_star_green);
        GET_OPT_NAMED_BITFIELD(ctx, "is_star_cyan", t->is_star_cyan);
        GET_OPT_NAMED_BITFIELD(ctx, "is_star_black", t->is_star_black);
        GET_OPT_NAMED_BITFIELD(ctx, "is_star_purple", t->is_star_purple);
        GET_OPT_NAMED_BITFIELD(ctx, "is_explosion", t->is_explosion);
        GET_OPT_NAMED_BITFIELD(ctx, "is_spikes", t->is_spikes);
        GET_OPT_NAMED_BITFIELD(ctx, "is_star_pink", t->is_star_pink);
        GET_OPT_NAMED_BITFIELD(ctx, "is_star_red", t->is_star_red);
        GET_OPT_NAMED_BITFIELD(ctx, "is_star_blue", t->is_star_blue);
        GET_OPT_NAMED_BITFIELD(ctx, "is_seedpod", t->is_seedpod);
        GET_OPT_NAMED_BITFIELD(ctx, "is_bomb", t->is_bomb);
        GET_OPT_NAMED_BITFIELD(ctx, "is_spam", t->is_spam);
        GET_OPT_NAMED_BITFIELD(ctx, "is_road", t->is_road);
        GET_OPT_NAMED_BITFIELD(ctx, "is_pipe", t->is_pipe);
        GET_OPT_NAMED_BITFIELD(ctx, "is_item_removed_at_level_end", t->is_item_removed_at_level_end);
        GET_OPT_NAMED_BITFIELD(ctx, "is_scarable", t->is_scarable);
        GET_OPT_NAMED_BITFIELD(ctx, "is_shrunk_when_carried", t->is_shrunk_when_carried);
        GET_OPT_NAMED_BITFIELD(ctx, "is_hidden_from_editor", t->is_hidden_from_editor);
        GET_OPT_NAMED_BITFIELD(ctx, "is_animated", t->is_animated);
        GET_OPT_NAMED_BITFIELD(ctx, "is_follows_owner", t->is_follows_owner);
        GET_OPT_NAMED_BITFIELD(ctx, "is_powerup_rocket", t->is_powerup_rocket);
        GET_OPT_NAMED_BITFIELD(ctx, "is_left_as_corpse_on_death", t->is_left_as_corpse_on_death);
        GET_OPT_NAMED_BITFIELD(ctx, "is_item_perma", t->is_item_perma);
        GET_OPT_NAMED_BITFIELD(ctx, "is_esnail", t->is_esnail);
        GET_OPT_NAMED_BITFIELD(ctx, "is_item_hidden", t->is_item_hidden);
        GET_OPT_NAMED_BITFIELD(ctx, "is_bonus_letter", t->is_bonus_letter);
        GET_OPT_NAMED_BITFIELD(ctx, "is_thing", t->is_thing);
        GET_OPT_NAMED_BITFIELD(ctx, "is_joinable", t->is_joinable);
        GET_OPT_NAMED_BITFIELD(ctx, "is_wall", t->is_wall);
        GET_OPT_NAMED_BITFIELD(ctx, "is_effect_sway", t->is_effect_sway);
        GET_OPT_NAMED_BITFIELD(ctx, "is_effect_pulse", t->is_effect_pulse);
        GET_OPT_NAMED_BITFIELD(ctx, "is_effect_rotate_4way", t->is_effect_rotate_4way);
        GET_OPT_NAMED_BITFIELD(ctx, "is_effect_rotate_2way", t->is_effect_rotate_2way);

    } while (demarshal_gotone(ctx));

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
    PUT_NAMED_STRING(ctx, "shortname", t->shortname);
    PUT_NAMED_STRING(ctx, "tooltip", t->tooltip);
    PUT_NAMED_UINT8(ctx, "z_depth", t->z_depth);
    PUT_NAMED_UINT8(ctx, "z_order", t->z_order);
    PUT_NAMED_INT32(ctx, "speed", t->speed);
    PUT_NAMED_INT32(ctx, "lifespan", t->lifespan);
    PUT_NAMED_INT32(ctx, "score_on_death", t->score_on_death);
    PUT_NAMED_INT32(ctx, "score_on_collect", t->score_on_collect);
    PUT_NAMED_INT32(ctx, "score_on_ppp1", t->ppp1);
    PUT_NAMED_INT32(ctx, "score_on_ppp2", t->ppp2);
    PUT_NAMED_INT32(ctx, "score_on_ppp3", t->ppp3);
    PUT_NAMED_INT32(ctx, "score_on_ppp4", t->ppp4);
    PUT_NAMED_INT32(ctx, "score_on_ppp5", t->ppp5);
    PUT_NAMED_INT32(ctx, "score_on_ppp6", t->ppp6);
    PUT_NAMED_INT32(ctx, "score_on_ppp7", t->ppp7);
    PUT_NAMED_INT32(ctx, "score_on_ppp8", t->ppp8);
    PUT_NAMED_INT32(ctx, "score_on_ppp9", t->ppp9);
    PUT_NAMED_INT32(ctx, "score_on_ppp10", t->ppp10);
    PUT_NAMED_INT32(ctx, "score_on_ppp11", t->ppp11);
    PUT_NAMED_INT32(ctx, "score_on_ppp12", t->ppp12);
    PUT_NAMED_INT32(ctx, "score_on_ppp13", t->ppp13);
    PUT_NAMED_INT32(ctx, "score_on_ppp14", t->ppp14);
    PUT_NAMED_INT32(ctx, "score_on_ppp15", t->ppp15);
    PUT_NAMED_INT32(ctx, "score_on_ppp16", t->ppp16);
    PUT_NAMED_INT32(ctx, "score_on_ppp17", t->ppp17);
    PUT_NAMED_INT32(ctx, "score_on_ppp18", t->ppp18);
    PUT_NAMED_INT32(ctx, "score_on_ppp19", t->ppp19);
    PUT_NAMED_INT32(ctx, "score_on_ppp20", t->ppp20);

    PUT_NAMED_BITFIELD(ctx, "is_player", t->is_player);
    PUT_NAMED_BITFIELD(ctx, "is_exit", t->is_exit);
    PUT_NAMED_BITFIELD(ctx, "is_floor", t->is_floor);
    PUT_NAMED_BITFIELD(ctx, "is_food", t->is_food);
    PUT_NAMED_BITFIELD(ctx, "is_letter", t->is_letter);
    PUT_NAMED_BITFIELD(ctx, "is_monst", t->is_monst);
    PUT_NAMED_BITFIELD(ctx, "is_plant", t->is_plant);
    PUT_NAMED_BITFIELD(ctx, "is_player", t->is_player);
    PUT_NAMED_BITFIELD(ctx, "is_snail", t->is_snail);
    PUT_NAMED_BITFIELD(ctx, "is_star_yellow", t->is_star_yellow);
    PUT_NAMED_BITFIELD(ctx, "is_rock", t->is_rock);
    PUT_NAMED_BITFIELD(ctx, "is_water", t->is_water);
    PUT_NAMED_BITFIELD(ctx, "is_lava", t->is_lava);
    PUT_NAMED_BITFIELD(ctx, "is_xxx4", t->is_xxx4);
    PUT_NAMED_BITFIELD(ctx, "is_xxx5", t->is_xxx5);
    PUT_NAMED_BITFIELD(ctx, "is_xxx6", t->is_xxx6);
    PUT_NAMED_BITFIELD(ctx, "is_xxx7", t->is_xxx7);
    PUT_NAMED_BITFIELD(ctx, "is_xxx8", t->is_xxx8);
    PUT_NAMED_BITFIELD(ctx, "is_star", t->is_star);
    PUT_NAMED_BITFIELD(ctx, "is_powerup_spam", t->is_powerup_spam);
    PUT_NAMED_BITFIELD(ctx, "is_rock_0", t->is_rock_0);
    PUT_NAMED_BITFIELD(ctx, "is_car", t->is_car);
    PUT_NAMED_BITFIELD(ctx, "is_star_green", t->is_star_green);
    PUT_NAMED_BITFIELD(ctx, "is_star_cyan", t->is_star_cyan);
    PUT_NAMED_BITFIELD(ctx, "is_star_black", t->is_star_black);
    PUT_NAMED_BITFIELD(ctx, "is_star_purple", t->is_star_purple);
    PUT_NAMED_BITFIELD(ctx, "is_explosion", t->is_explosion);
    PUT_NAMED_BITFIELD(ctx, "is_spikes", t->is_spikes);
    PUT_NAMED_BITFIELD(ctx, "is_star_pink", t->is_star_pink);
    PUT_NAMED_BITFIELD(ctx, "is_star_red", t->is_star_red);
    PUT_NAMED_BITFIELD(ctx, "is_star_blue", t->is_star_blue);
    PUT_NAMED_BITFIELD(ctx, "is_seedpod", t->is_seedpod);
    PUT_NAMED_BITFIELD(ctx, "is_bomb", t->is_bomb);
    PUT_NAMED_BITFIELD(ctx, "is_spam", t->is_spam);
    PUT_NAMED_BITFIELD(ctx, "is_road", t->is_road);
    PUT_NAMED_BITFIELD(ctx, "is_pipe", t->is_pipe);
    PUT_NAMED_BITFIELD(ctx, "is_item_removed_at_level_end", t->is_item_removed_at_level_end);
    PUT_NAMED_BITFIELD(ctx, "is_scarable", t->is_scarable);
    PUT_NAMED_BITFIELD(ctx, "is_shrunk_when_carried", t->is_shrunk_when_carried);
    PUT_NAMED_BITFIELD(ctx, "is_hidden_from_editor", t->is_hidden_from_editor);
    PUT_NAMED_BITFIELD(ctx, "is_animated", t->is_animated);
    PUT_NAMED_BITFIELD(ctx, "is_follows_owner", t->is_follows_owner);
    PUT_NAMED_BITFIELD(ctx, "is_powerup_rocket", t->is_powerup_rocket);
    PUT_NAMED_BITFIELD(ctx, "is_left_as_corpse_on_death", t->is_left_as_corpse_on_death);
    PUT_NAMED_BITFIELD(ctx, "is_item_perma", t->is_item_perma);
    PUT_NAMED_BITFIELD(ctx, "is_esnail", t->is_esnail);
    PUT_NAMED_BITFIELD(ctx, "is_item_hidden", t->is_item_hidden);
    PUT_NAMED_BITFIELD(ctx, "is_bonus_letter", t->is_bonus_letter);
    PUT_NAMED_BITFIELD(ctx, "is_thing", t->is_thing);
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

const char *thing_template_shortname (thing_templatep t)
{
    return (t->shortname);
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

uint32_t thing_template_get_lifespan (thing_templatep t)
{
    return (t->lifespan);
}

uint32_t thing_template_get_score_on_death (thing_templatep t)
{
    return (t->score_on_death);
}

uint32_t thing_template_get_score_on_collect (thing_templatep t)
{
    return (t->score_on_collect);
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

uint32_t thing_template_get_ppp15 (thing_templatep t)
{
    return (t->ppp15);
}

uint32_t thing_template_get_ppp16 (thing_templatep t)
{
    return (t->ppp16);
}

uint32_t thing_template_get_ppp17 (thing_templatep t)
{
    return (t->ppp17);
}

uint32_t thing_template_get_ppp18 (thing_templatep t)
{
    return (t->ppp18);
}

uint32_t thing_template_get_ppp19 (thing_templatep t)
{
    return (t->ppp19);
}

uint32_t thing_template_get_ppp20 (thing_templatep t)
{
    return (t->ppp20);
}

tree_rootp thing_template_get_tiles (thing_templatep t)
{
    return (t->tiles);
}

tree_rootp thing_template_get_tiles2 (thing_templatep t)
{
    return (t->tiles2);
}

boolean thing_template_is_exit (thing_templatep t)
{
    return (t->is_exit);
}

boolean thing_template_is_floor (thing_templatep t)
{
    return (t->is_floor);
}

boolean thing_template_is_food (thing_templatep t)
{
    return (t->is_food);
}

boolean thing_template_is_letter (thing_templatep t)
{
    return (t->is_letter);
}

boolean thing_template_is_monst (thing_templatep t)
{
    return (t->is_monst);
}

boolean thing_template_is_plant (thing_templatep t)
{
    return (t->is_plant);
}

boolean thing_template_is_player (thing_templatep t)
{
    return (t->is_player);
}

boolean thing_template_is_snail (thing_templatep t)
{
    return (t->is_snail);
}

boolean thing_template_is_star_yellow (thing_templatep t)
{
    return (t->is_star_yellow);
}

boolean thing_template_is_rock (thing_templatep t)
{
    return (t->is_rock);
}

boolean thing_template_is_water (thing_templatep t)
{
    return (t->is_water);
}

boolean thing_template_is_lava (thing_templatep t)
{
    return (t->is_lava);
}

boolean thing_template_is_xxx4 (thing_templatep t)
{
    return (t->is_xxx4);
}

boolean thing_template_is_xxx5 (thing_templatep t)
{
    return (t->is_xxx5);
}

boolean thing_template_is_xxx6 (thing_templatep t)
{
    return (t->is_xxx6);
}

boolean thing_template_is_xxx7 (thing_templatep t)
{
    return (t->is_xxx7);
}

boolean thing_template_is_xxx8 (thing_templatep t)
{
    return (t->is_xxx8);
}

boolean thing_template_is_star (thing_templatep t)
{
    return (t->is_star);
}

boolean thing_template_is_powerup_spam (thing_templatep t)
{
    return (t->is_powerup_spam);
}

boolean thing_template_is_rock_0 (thing_templatep t)
{
    return (t->is_rock_0);
}

boolean thing_template_is_car (thing_templatep t)
{
    return (t->is_car);
}

boolean thing_template_is_star_green (thing_templatep t)
{
    return (t->is_star_green);
}

boolean thing_template_is_star_cyan (thing_templatep t)
{
    return (t->is_star_cyan);
}

boolean thing_template_is_star_black (thing_templatep t)
{
    return (t->is_star_black);
}

boolean thing_template_is_star_purple (thing_templatep t)
{
    return (t->is_star_purple);
}

boolean thing_template_is_explosion (thing_templatep t)
{
    return (t->is_explosion);
}

boolean thing_template_is_spikes (thing_templatep t)
{
    return (t->is_spikes);
}

boolean thing_template_is_star_red (thing_templatep t)
{
    return (t->is_star_red);
}

boolean thing_template_is_star_pink (thing_templatep t)
{
    return (t->is_star_pink);
}

boolean thing_template_is_star_blue (thing_templatep t)
{
    return (t->is_star_blue);
}

boolean thing_template_is_seedpod (thing_templatep t)
{
    return (t->is_seedpod);
}

boolean thing_template_is_bomb (thing_templatep t)
{
    return (t->is_bomb);
}

boolean thing_template_is_spam (thing_templatep t)
{
    return (t->is_spam);
}

boolean thing_template_is_road (thing_templatep t)
{
    return (t->is_road);
}

boolean thing_template_is_pipe (thing_templatep t)
{
    return (t->is_pipe);
}

boolean thing_template_is_item_removed_at_level_end (thing_templatep t)
{
    return (t->is_item_removed_at_level_end);
}

boolean thing_template_is_scarable (thing_templatep t)
{
    return (t->is_scarable);
}

boolean thing_template_is_shrunk_when_carried (thing_templatep t)
{
    return (t->is_shrunk_when_carried);
}

boolean thing_template_is_hidden_from_editor (thing_templatep t)
{
    return (t->is_hidden_from_editor);
}

boolean thing_template_is_animated (thing_templatep t)
{
    return (t->is_animated);
}

boolean thing_template_is_follows_owner (thing_templatep t)
{
    return (t->is_follows_owner);
}

boolean thing_template_is_powerup_rocket (thing_templatep t)
{
    return (t->is_powerup_rocket);
}

boolean thing_template_is_left_as_corpse_on_death (thing_templatep t)
{
    return (t->is_left_as_corpse_on_death);
}

boolean thing_template_is_item_perma (thing_templatep t)
{
    return (t->is_item_perma);
}

boolean thing_template_is_esnail (thing_templatep t)
{
    return (t->is_esnail);
}

boolean thing_template_is_item_hidden (thing_templatep t)
{
    return (t->is_item_hidden);
}

boolean thing_template_is_bonus_letter (thing_templatep t)
{
    return (t->is_bonus_letter);
}

boolean thing_template_is_thing (thing_templatep t)
{
    return (t->is_thing);
}

boolean thing_template_is_joinable (thing_templatep t)
{
    return (t->is_joinable);
}

boolean thing_template_is_wall (thing_templatep t)
{
    return (t->is_wall);
}

boolean thing_template_is_effect_sway (thing_templatep t)
{
    return (t->is_effect_sway);
}

boolean thing_template_is_effect_pulse (thing_templatep t)
{
    return (t->is_effect_pulse);
}

boolean thing_template_is_effect_rotate_4way (thing_templatep t)
{
    return (t->is_effect_rotate_4way);
}

boolean thing_template_is_effect_rotate_2way (thing_templatep t)
{
    return (t->is_effect_rotate_2way);
}

