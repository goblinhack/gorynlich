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
#include "thing_template_private.h"
#include "thing_tile.h"
#include "marshal.h"

void thing_tile_free (thing_tile *t)
{
    myfree(t->tile);

    if (t->command) {
        myfree(t->command);
    }
}

static void demarshal_thing_tile (demarshal_p ctx, thing_tile *t)
{
    GET_BRA(ctx);

    (void) demarshal_gotone(ctx);

    do {
        GET_OPT_NAMED_UINT32(ctx, "delay", t->delay);
        GET_OPT_NAMED_STRING(ctx, "tile", t->tile);
        GET_OPT_NAMED_STRING(ctx, "command", t->command);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_block", t->is_join_block);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_horiz", t->is_join_horiz);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_vert", t->is_join_vert);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_node", t->is_join_node);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_left", t->is_join_left);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_right", t->is_join_right);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_top", t->is_join_top);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_bot", t->is_join_bot);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_tl", t->is_join_tl);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_tr", t->is_join_tr);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_bl", t->is_join_bl);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_br", t->is_join_br);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_t", t->is_join_t);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_t90", t->is_join_t90);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_t180", t->is_join_t180);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_t270", t->is_join_t270);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_x", t->is_join_x);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_tl2", t->is_join_tl2);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_tr2", t->is_join_tr2);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_bl2", t->is_join_bl2);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_br2", t->is_join_br2);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_t_1", t->is_join_t_1);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_t_2", t->is_join_t_2);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_t_3", t->is_join_t_3);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_t90_1", t->is_join_t90_1);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_t90_2", t->is_join_t90_2);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_t90_3", t->is_join_t90_3);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_t180_1", t->is_join_t180_1);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_t180_2", t->is_join_t180_2);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_t180_3", t->is_join_t180_3);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_t270_1", t->is_join_t270_1);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_t270_2", t->is_join_t270_2);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_t270_3", t->is_join_t270_3);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_x1", t->is_join_x1);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_x1_270", t->is_join_x1_270);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_x1_180", t->is_join_x1_180);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_x1_90", t->is_join_x1_90);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_x2", t->is_join_x2);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_x2_270", t->is_join_x2_270);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_x2_180", t->is_join_x2_180);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_x2_90", t->is_join_x2_90);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_x3", t->is_join_x3);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_x3_180", t->is_join_x3_180);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_x4", t->is_join_x4);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_x4_270", t->is_join_x4_270);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_x4_180", t->is_join_x4_180);
        GET_OPT_NAMED_BITFIELD(ctx, "is_join_x4_90", t->is_join_x4_90);
        GET_OPT_NAMED_BITFIELD(ctx, "is_yyy1", t->is_yyy1);
        GET_OPT_NAMED_BITFIELD(ctx, "is_yyy2", t->is_yyy2);
        GET_OPT_NAMED_BITFIELD(ctx, "is_yyy3", t->is_yyy3);
        GET_OPT_NAMED_BITFIELD(ctx, "is_yyy4", t->is_yyy4);
        GET_OPT_NAMED_BITFIELD(ctx, "is_yyy5", t->is_yyy5);
        GET_OPT_NAMED_BITFIELD(ctx, "is_yyy6", t->is_yyy6);
        GET_OPT_NAMED_BITFIELD(ctx, "is_yyy7", t->is_yyy7);
        GET_OPT_NAMED_BITFIELD(ctx, "is_yyy8", t->is_yyy8);
        GET_OPT_NAMED_BITFIELD(ctx, "is_yyy9", t->is_yyy9);
        GET_OPT_NAMED_BITFIELD(ctx, "is_yyy10", t->is_yyy10);
        GET_OPT_NAMED_BITFIELD(ctx, "is_yyy11", t->is_yyy11);
        GET_OPT_NAMED_BITFIELD(ctx, "is_yyy12", t->is_yyy12);
        GET_OPT_NAMED_BITFIELD(ctx, "is_yyy13", t->is_yyy13);
        GET_OPT_NAMED_BITFIELD(ctx, "is_yyy14", t->is_yyy14);
        GET_OPT_NAMED_BITFIELD(ctx, "is_yyy15", t->is_yyy15);
        GET_OPT_NAMED_BITFIELD(ctx, "is_yyy16", t->is_yyy16);
        GET_OPT_NAMED_BITFIELD(ctx, "is_yyy17", t->is_yyy17);
        GET_OPT_NAMED_BITFIELD(ctx, "is_open", t->is_open);
        GET_OPT_NAMED_BITFIELD(ctx, "is_dead", t->is_dead);
        GET_OPT_NAMED_BITFIELD(ctx, "is_end_of_anim", t->is_end_of_anim);

    } while (demarshal_gotone(ctx));

    GET_KET(ctx);
}

static void marshal_thing_tile (marshal_p ctx, thing_tile *t)
{
    PUT_BRA(ctx);

    PUT_NAMED_STRING(ctx,  "tile", t->tile);

    if (t->command) {
        PUT_NAMED_STRING(ctx,  "tile", t->command);
    }

    if (t->delay) {
        PUT_NAMED_UINT32(ctx, "delay", t->delay);
    }

    PUT_NAMED_BITFIELD(ctx, "is_join_block", t->is_join_block);
    PUT_NAMED_BITFIELD(ctx, "is_join_horiz", t->is_join_horiz);
    PUT_NAMED_BITFIELD(ctx, "is_join_vert", t->is_join_vert);
    PUT_NAMED_BITFIELD(ctx, "is_join_node", t->is_join_node);
    PUT_NAMED_BITFIELD(ctx, "is_join_left", t->is_join_left);
    PUT_NAMED_BITFIELD(ctx, "is_join_right", t->is_join_right);
    PUT_NAMED_BITFIELD(ctx, "is_join_top", t->is_join_top);
    PUT_NAMED_BITFIELD(ctx, "is_join_bot", t->is_join_bot);
    PUT_NAMED_BITFIELD(ctx, "is_join_tl", t->is_join_tl);
    PUT_NAMED_BITFIELD(ctx, "is_join_tr", t->is_join_tr);
    PUT_NAMED_BITFIELD(ctx, "is_join_bl", t->is_join_bl);
    PUT_NAMED_BITFIELD(ctx, "is_join_br", t->is_join_br);
    PUT_NAMED_BITFIELD(ctx, "is_join_t", t->is_join_t);
    PUT_NAMED_BITFIELD(ctx, "is_join_t90", t->is_join_t90);
    PUT_NAMED_BITFIELD(ctx, "is_join_t180", t->is_join_t180);
    PUT_NAMED_BITFIELD(ctx, "is_join_t270", t->is_join_t270);
    PUT_NAMED_BITFIELD(ctx, "is_join_x", t->is_join_x);
    PUT_NAMED_BITFIELD(ctx, "is_join_tl2", t->is_join_tl2);
    PUT_NAMED_BITFIELD(ctx, "is_join_tr2", t->is_join_tr2);
    PUT_NAMED_BITFIELD(ctx, "is_join_bl2", t->is_join_bl2);
    PUT_NAMED_BITFIELD(ctx, "is_join_br2", t->is_join_br2);
    PUT_NAMED_BITFIELD(ctx, "is_join_t_1", t->is_join_t_1);
    PUT_NAMED_BITFIELD(ctx, "is_join_t_2", t->is_join_t_2);
    PUT_NAMED_BITFIELD(ctx, "is_join_t_3", t->is_join_t_3);
    PUT_NAMED_BITFIELD(ctx, "is_join_t90_1", t->is_join_t90_1);
    PUT_NAMED_BITFIELD(ctx, "is_join_t90_2", t->is_join_t90_2);
    PUT_NAMED_BITFIELD(ctx, "is_join_t90_3", t->is_join_t90_3);
    PUT_NAMED_BITFIELD(ctx, "is_join_t180_1", t->is_join_t180_1);
    PUT_NAMED_BITFIELD(ctx, "is_join_t180_2", t->is_join_t180_2);
    PUT_NAMED_BITFIELD(ctx, "is_join_t180_3", t->is_join_t180_3);
    PUT_NAMED_BITFIELD(ctx, "is_join_t270_1", t->is_join_t270_1);
    PUT_NAMED_BITFIELD(ctx, "is_join_t270_2", t->is_join_t270_2);
    PUT_NAMED_BITFIELD(ctx, "is_join_t270_3", t->is_join_t270_3);
    PUT_NAMED_BITFIELD(ctx, "is_yyy1", t->is_yyy1);
    PUT_NAMED_BITFIELD(ctx, "is_yyy2", t->is_yyy2);
    PUT_NAMED_BITFIELD(ctx, "is_yyy3", t->is_yyy3);
    PUT_NAMED_BITFIELD(ctx, "is_yyy4", t->is_yyy4);
    PUT_NAMED_BITFIELD(ctx, "is_yyy5", t->is_yyy5);
    PUT_NAMED_BITFIELD(ctx, "is_yyy6", t->is_yyy6);
    PUT_NAMED_BITFIELD(ctx, "is_yyy7", t->is_yyy7);
    PUT_NAMED_BITFIELD(ctx, "is_yyy8", t->is_yyy8);
    PUT_NAMED_BITFIELD(ctx, "is_yyy9", t->is_yyy9);
    PUT_NAMED_BITFIELD(ctx, "is_yyy10", t->is_yyy10);
    PUT_NAMED_BITFIELD(ctx, "is_yyy11", t->is_yyy11);
    PUT_NAMED_BITFIELD(ctx, "is_yyy12", t->is_yyy12);
    PUT_NAMED_BITFIELD(ctx, "is_yyy13", t->is_yyy13);
    PUT_NAMED_BITFIELD(ctx, "is_yyy14", t->is_yyy14);
    PUT_NAMED_BITFIELD(ctx, "is_yyy15", t->is_yyy15);
    PUT_NAMED_BITFIELD(ctx, "is_yyy16", t->is_yyy16);
    PUT_NAMED_BITFIELD(ctx, "is_yyy17", t->is_yyy17);
    PUT_NAMED_BITFIELD(ctx, "is_open", t->is_open);
    PUT_NAMED_BITFIELD(ctx, "is_dead", t->is_dead);
    PUT_NAMED_BITFIELD(ctx, "is_end_of_anim", t->is_end_of_anim);

    PUT_KET(ctx);
}

void demarshal_thing_tiles (demarshal_p ctx, thing_templatep t)
{
    static uint32_t id;

    if (!GET_PEEK_NAME(ctx, "tiles")) {
        return;
    }

    GET_NAME(ctx, "tiles");
    GET_BRA(ctx);

    t->tiles = tree_alloc(TREE_KEY_INTEGER, "TREE ROOT: thing tiles");

    while (GET_PEEK_NAME(ctx, "tile")) {
        GET_NAME(ctx, "tile");

        thing_tile *tile;

        tile = (typeof(tile)) myzalloc(sizeof(*tile),
                                       "TREE NODE: thing tile");
        tile->tree.key = id++;

        if (!tree_insert(t->tiles, &tile->tree.node)) {
            DIE("insert thing tiles node fail");
        }

        demarshal_thing_tile(ctx, tile);
    }

    GET_KET(ctx);
}

void marshal_thing_tiles (marshal_p ctx, thing_templatep t)
{
    thing_tile *tile;

    if (!t->tiles) {
        return;
    }

    PUT_NAME(ctx, "tiles");
    PUT_BRA(ctx);

    TREE_WALK(t->tiles, tile) {
        PUT_NAME(ctx, "tile");
        marshal_thing_tile(ctx, tile);
    }

    PUT_KET(ctx);
}

void demarshal_thing_tiles2 (demarshal_p ctx, thing_templatep t)
{
    static uint32_t id;

    if (!GET_PEEK_NAME(ctx, "tiles2")) {
        return;
    }

    GET_NAME(ctx, "tiles2");
    GET_BRA(ctx);

    t->tiles2 = tree_alloc(TREE_KEY_INTEGER, "TREE ROOT: thing tiles2");

    while (GET_PEEK_NAME(ctx, "tile")) {
        GET_NAME(ctx, "tile");

        thing_tile *tile;

        tile = (typeof(tile)) myzalloc(sizeof(*tile),
                                       "TREE NODE: thing tile");
        tile->tree.key = id++;

        if (!tree_insert(t->tiles2, &tile->tree.node)) {
            DIE("insert thing tiles2 node fail");
        }

        demarshal_thing_tile(ctx, tile);
    }

    GET_KET(ctx);
}

void marshal_thing_tiles2 (marshal_p ctx, thing_templatep t)
{
    thing_tile *tile;

    if (!t->tiles2) {
        return;
    }

    PUT_NAME(ctx, "tiles2");
    PUT_BRA(ctx);

    TREE_WALK(t->tiles2, tile) {
        PUT_NAME(ctx, "tile");
        marshal_thing_tile(ctx, tile);
    }

    PUT_KET(ctx);
}

const char *thing_tile_name (thing_tile *t)
{
    return (t->tile);
}

const char *thing_tile_command (thing_tile *t)
{
    return (t->command);
}

uint32_t thing_tile_delay (thing_tile *t)
{
    return (t->delay);
}

boolean thing_tile_is_join_block (thing_tilep t)
{
    return (t->is_join_block);
}

boolean thing_tile_is_join_horiz (thing_tilep t)
{
    return (t->is_join_horiz);
}

boolean thing_tile_is_join_vert (thing_tilep t)
{
    return (t->is_join_vert);
}

boolean thing_tile_is_join_node (thing_tilep t)
{
    return (t->is_join_node);
}

boolean thing_tile_is_join_left (thing_tilep t)
{
    return (t->is_join_left);
}

boolean thing_tile_is_join_right (thing_tilep t)
{
    return (t->is_join_right);
}

boolean thing_tile_is_join_top (thing_tilep t)
{
    return (t->is_join_top);
}

boolean thing_tile_is_join_bot (thing_tilep t)
{
    return (t->is_join_bot);
}

boolean thing_tile_is_join_tl (thing_tilep t)
{
    return (t->is_join_tl);
}

boolean thing_tile_is_join_tr (thing_tilep t)
{
    return (t->is_join_tr);
}

boolean thing_tile_is_join_bl (thing_tilep t)
{
    return (t->is_join_bl);
}

boolean thing_tile_is_join_br (thing_tilep t)
{
    return (t->is_join_br);
}

boolean thing_tile_is_join_t (thing_tilep t)
{
    return (t->is_join_t);
}

boolean thing_tile_is_join_t90 (thing_tilep t)
{
    return (t->is_join_t90);
}

boolean thing_tile_is_join_t180 (thing_tilep t)
{
    return (t->is_join_t180);
}

boolean thing_tile_is_join_t270 (thing_tilep t)
{
    return (t->is_join_t270);
}

boolean thing_tile_is_join_x (thing_tilep t)
{
    return (t->is_join_x);
}

boolean thing_tile_is_join_tl2 (thing_tilep t)
{
    return (t->is_join_tl2);
}

boolean thing_tile_is_join_tr2 (thing_tilep t)
{
    return (t->is_join_tr2);
}

boolean thing_tile_is_join_bl2 (thing_tilep t)
{
    return (t->is_join_bl2);
}

boolean thing_tile_is_join_br2 (thing_tilep t)
{
    return (t->is_join_br2);
}

boolean thing_tile_is_join_t_1 (thing_tilep t)
{
    return (t->is_join_t_1);
}

boolean thing_tile_is_join_t_2 (thing_tilep t)
{
    return (t->is_join_t_2);
}

boolean thing_tile_is_join_t_3 (thing_tilep t)
{
    return (t->is_join_t_3);
}

boolean thing_tile_is_join_t90_1 (thing_tilep t)
{
    return (t->is_join_t90_1);
}

boolean thing_tile_is_join_t90_2 (thing_tilep t)
{
    return (t->is_join_t90_2);
}

boolean thing_tile_is_join_t90_3 (thing_tilep t)
{
    return (t->is_join_t90_3);
}

boolean thing_tile_is_join_t180_1 (thing_tilep t)
{
    return (t->is_join_t180_1);
}

boolean thing_tile_is_join_t180_2 (thing_tilep t)
{
    return (t->is_join_t180_2);
}

boolean thing_tile_is_join_t180_3 (thing_tilep t)
{
    return (t->is_join_t180_3);
}

boolean thing_tile_is_join_t270_1 (thing_tilep t)
{
    return (t->is_join_t270_1);
}

boolean thing_tile_is_join_t270_2 (thing_tilep t)
{
    return (t->is_join_t270_2);
}

boolean thing_tile_is_join_t270_3 (thing_tilep t)
{
    return (t->is_join_t270_3);
}

boolean thing_tile_is_join_x1 (thing_tilep t)
{
    return (t->is_join_x1);
}

boolean thing_tile_is_join_x1_270 (thing_tilep t)
{
    return (t->is_join_x1_270);
}

boolean thing_tile_is_join_x1_180 (thing_tilep t)
{
    return (t->is_join_x1_180);
}

boolean thing_tile_is_join_x1_90 (thing_tilep t)
{
    return (t->is_join_x1_90);
}

boolean thing_tile_is_join_x2 (thing_tilep t)
{
    return (t->is_join_x2);
}

boolean thing_tile_is_join_x2_270 (thing_tilep t)
{
    return (t->is_join_x2_270);
}

boolean thing_tile_is_join_x2_180 (thing_tilep t)
{
    return (t->is_join_x2_180);
}

boolean thing_tile_is_join_x2_90 (thing_tilep t)
{
    return (t->is_join_x2_90);
}

boolean thing_tile_is_join_x3 (thing_tilep t)
{
    return (t->is_join_x3);
}

boolean thing_tile_is_join_x3_180 (thing_tilep t)
{
    return (t->is_join_x3_180);
}

boolean thing_tile_is_join_x4 (thing_tilep t)
{
    return (t->is_join_x4);
}

boolean thing_tile_is_join_x4_270 (thing_tilep t)
{
    return (t->is_join_x4_270);
}

boolean thing_tile_is_join_x4_180 (thing_tilep t)
{
    return (t->is_join_x4_180);
}

boolean thing_tile_is_join_x4_90 (thing_tilep t)
{
    return (t->is_join_x4_90);
}

boolean thing_tile_is_yyy1 (thing_tilep t)
{
    return (t->is_yyy1);
}

boolean thing_tile_is_yyy2 (thing_tilep t)
{
    return (t->is_yyy2);
}

boolean thing_tile_is_yyy3 (thing_tilep t)
{
    return (t->is_yyy3);
}

boolean thing_tile_is_yyy4 (thing_tilep t)
{
    return (t->is_yyy4);
}

boolean thing_tile_is_yyy5 (thing_tilep t)
{
    return (t->is_yyy5);
}

boolean thing_tile_is_yyy6 (thing_tilep t)
{
    return (t->is_yyy6);
}

boolean thing_tile_is_yyy7 (thing_tilep t)
{
    return (t->is_yyy7);
}

boolean thing_tile_is_yyy8 (thing_tilep t)
{
    return (t->is_yyy8);
}

boolean thing_tile_is_yyy9 (thing_tilep t)
{
    return (t->is_yyy9);
}

boolean thing_tile_is_yyy10 (thing_tilep t)
{
    return (t->is_yyy10);
}

boolean thing_tile_is_yyy11 (thing_tilep t)
{
    return (t->is_yyy11);
}

boolean thing_tile_is_yyy12 (thing_tilep t)
{
    return (t->is_yyy12);
}

boolean thing_tile_is_yyy13 (thing_tilep t)
{
    return (t->is_yyy13);
}

boolean thing_tile_is_yyy14 (thing_tilep t)
{
    return (t->is_yyy14);
}

boolean thing_tile_is_yyy15 (thing_tilep t)
{
    return (t->is_yyy15);
}

boolean thing_tile_is_yyy16 (thing_tilep t)
{
    return (t->is_yyy16);
}

boolean thing_tile_is_yyy17 (thing_tilep t)
{
    return (t->is_yyy17);
}

boolean thing_tile_is_open (thing_tilep t)
{
    return (t->is_open);
}

boolean thing_tile_is_dead (thing_tilep t)
{
    return (t->is_dead);
}

boolean thing_tile_is_end_of_anim (thing_tilep t)
{
    return (t->is_end_of_anim);
}

thing_tilep thing_tile_find (thing_templatep t, uint32_t index, tilep *tile)
{
    uint32_t which;
    
    if (!t->tilep_join_count[index]) {
        return (0);
    }

    which = rand() % t->tilep_join_count[index];

    *tile = t->tilep_join_tile[index][which];

    return (t->tilep_join[index][which]);
}

thing_tilep thing_tile_first (tree_rootp root)
{
    thing_tilep thing_tile;

    thing_tile = (typeof(thing_tile)) tree_root_first(root);

    if (thing_tile) {
        verify(thing_tile);
    }

    return (thing_tile);
}

thing_tilep thing_tile_next (tree_rootp root, thing_tilep in)
{
    thing_tile cursor;
    thing_tilep next;

    verify(in);

    memset(&cursor, 0, sizeof(cursor));
    cursor.tree.key = in->tree.key;

    next = (typeof(next)) tree_get_next(root,
                                        tree_root_top(root),
                                        &cursor.tree.node);
    if (next) {
        verify(next);
    }

    return (next);
}
