/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "thing.h"
#include "thing_timer.h"
#include "wid.h"
#include "wid_game_map_client.h"
#include "wid_game_map_server.h"
#include "map.h"
#include "level.h"
#include "time.h"
#include "string.h"
#include "wid_textbox.h"
#include "sound.h"
#include "socket.h"
#include "client.h"
#include "timer.h"
#include "tile.h"
#include "math.h"
#include "wid_hiscore.h"

uint16_t THING_WALL1;
uint16_t THING_ROCK1;
uint16_t THING_WALL2;
uint16_t THING_WALL3;
uint16_t THING_WALL4;
uint16_t THING_WALL5;
uint16_t THING_WALL6;
uint16_t THING_DOOR1;
uint16_t THING_DOOR2;
uint16_t THING_NOENTRY;
uint16_t THING_APPLE1;
uint16_t THING_APPLE2;
uint16_t THING_BANANA;
uint16_t THING_CHEESE;
uint16_t THING_BREAD;
uint16_t THING_FLOOR1;
uint16_t THING_FLOOR2;
uint16_t THING_FLOOR3;
uint16_t THING_FLOOR4;
uint16_t THING_FLOOR5;
uint16_t THING_FLOOR6;
uint16_t THING_WARRIOR;
uint16_t THING_VALKYRIE;
uint16_t THING_WIZARD;
uint16_t THING_ELF;
uint16_t THING_DWARF;
uint16_t THING_GHOST1;
uint16_t THING_GHOST2;
uint16_t THING_GHOST3;
uint16_t THING_DEMON;
uint16_t THING_MONK;
uint16_t THING_LIZARD;
uint16_t THING_DEATH;
uint16_t THING_WAND1_ANIM;
uint16_t THING_SWORD1_ANIM;
uint16_t THING_SWORD2_ANIM;
uint16_t THING_SWORD3_ANIM;
uint16_t THING_AXE1_ANIM;
uint16_t THING_AXE2_ANIM;
uint16_t THING_BOW1_ANIM;
uint16_t THING_SCYTHE1_ANIM;
uint16_t THING_ANIM_MAN;
uint16_t THING_ANIM_TREASURE_CHEST;
uint16_t THING_ANIM_EYES;
uint16_t THING_PIPE;
uint16_t THING_WEAPON_HIT1;
uint16_t THING_EXPLOSION1;
uint16_t THING_EXPLOSION2;
uint16_t THING_EXPLOSION3;
uint16_t THING_EXPLOSION4;
uint16_t THING_HIT_SUCCESS;
uint16_t THING_HIT_MISS;
uint16_t THING_BLOOD1;
uint16_t THING_BLOOD2;
uint16_t THING_POISON1;
uint16_t THING_POISON2;
uint16_t THING_POISON3;
uint16_t THING_POISON4;
uint16_t THING_CLOUDKILL1;
uint16_t THING_CLOUDKILL2;
uint16_t THING_CLOUDKILL3;
uint16_t THING_CLOUDKILL4;
uint16_t THING_BOMB;
uint16_t THING_SPAM;
uint16_t THING_POTION_MONSTICIDE;
uint16_t THING_POTION_FIRE;
uint16_t THING_POTION_CLOUDKILL;
uint16_t THING_POTION_LIFE;
uint16_t THING_POTION_SHIELD;
uint16_t THING_WATER;
uint16_t THING_WATER_POISON;
uint16_t THING_MASK1;
uint16_t THING_RING2;
uint16_t THING_RING3;
uint16_t THING_GEM1;
uint16_t THING_GEM2;
uint16_t THING_GEM3;
uint16_t THING_GEM4;
uint16_t THING_GEM5;
uint16_t THING_GEM6;
uint16_t THING_GEM7;
uint16_t THING_BRAZIER;
uint16_t THING_EXIT1;
uint16_t THING_EXIT2;
uint16_t THING_EXIT3;
uint16_t THING_EXIT4;
uint16_t THING_EXIT5;
uint16_t THING_EXIT6;
uint16_t THING_EXIT7;
uint16_t THING_GENERATOR1;
uint16_t THING_BONEPILE1;
uint16_t THING_BONEPILE2;
uint16_t THING_BONEPILE3;
uint16_t THING_GENERATOR5;
uint16_t THING_GENERATOR6;
uint16_t THING_GENERATOR7;
uint16_t THING_BOW1;
uint16_t THING_BOW2;
uint16_t THING_BOW3;
uint16_t THING_BOW4;
uint16_t THING_WAND_FIRE;
uint16_t THING_SWORD1;
uint16_t THING_SWORD2;
uint16_t THING_SWORD3;
uint16_t THING_SWORD4;
uint16_t THING_SWORD5;
uint16_t THING_SWORD6;
uint16_t THING_AXE1;
uint16_t THING_AXE2;
uint16_t THING_AXE3;
uint16_t THING_AXE4;
uint16_t THING_AXE5;
uint16_t THING_SCYTHE1;
uint16_t THING_KEY;
uint16_t THING_KEYS2;
uint16_t THING_KEYS3;
uint16_t THING_TORCH;
uint16_t THING_TORCHES;
uint16_t THING_COINS1;
uint16_t THING_CHEST1;
uint16_t THING_ARROW;
uint16_t THING_FIREBALL;

tree_root *server_player_things;
tree_root *client_player_things;

tree_root *server_active_things;
tree_root *client_active_things;

tree_root *server_boring_things;
tree_root *client_boring_things;

static int server_things_total;
static int client_things_total;

static uint32_t next_thing_id;
static uint32_t next_monst_thing_id;

/*
 * We reserve client things for local side explosion effects.
 */
static uint32_t next_client_thing_id = THING_ID_MAX;

thingp thing_server_ids[THING_ID_MAX];
thingp thing_client_ids[THING_CLIENT_ID_MAX];

static uint8_t thing_init_done;
static void thing_destroy_implicit(thingp t);
static void thing_map_check_empty(void);

/*
 * What things live on the map.
 */
thing_map thing_server_map;
thing_map thing_client_map;

uint8_t thing_init (void)
{
    thing_init_done = true;

    dmap_process_init();

    return (true);
}

void thing_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (thing_init_done) {
        thing_init_done = false;

        tree_destroy(&client_player_things, 
                     (tree_destroy_func)0);
        tree_destroy(&server_player_things, 
                     (tree_destroy_func)0);

        tree_destroy(&client_active_things, 
                     (tree_destroy_func)thing_destroy_implicit);
        tree_destroy(&server_active_things, 
                     (tree_destroy_func)thing_destroy_implicit);

        tree_destroy(&client_boring_things, 
                     (tree_destroy_func)thing_destroy_implicit);
        tree_destroy(&server_boring_things, 
                     (tree_destroy_func)thing_destroy_implicit);

        if (thing_timers) {
            action_timers_destroy(&thing_timers);
        }

        dmap_process_fini();

        thing_map_check_empty();
    }
}

void thing_update (thingp t)
{
    if (!t->on_server) {
        ERR("cannot update a thing %s for sending to the client on the client",
            thing_logname(t));
        return;
    }

    /*
     * Need an explicit update for boring things as we send them only on 
     * demand.
     */
    if (server_level) {
        /*
         * Can be called during level load, hence the check.
         */
        if (tp_is_boring(t->tp)) {
            server_level->need_boring_update = true;
        }
    }

    if (t->updated) {
        return;
    }

    t->updated++;

    /*
     * Update the weapon being carried.
     */
    thingp weapon_carry_anim = thing_weapon_carry_anim(t);
    if (weapon_carry_anim) {
        thing_update(weapon_carry_anim);
    }

    /*
     * Update the weapon being swung.
     */
    thingp weapon_swing_anim = thing_weapon_swing_anim(t);
    if (weapon_swing_anim) {
        thing_update(weapon_swing_anim);
    }
}

/*
 * Create a new thing.
 */
static void thing_try_to_flush_ids_ (thingp *ids)
{
    uint32_t i;

    for (i = 0; i < THING_ID_MAX; i++) {
        thingp t = ids[i];

        if (!t) {
            continue;
        }

        if (thing_is_explosion(t)) {
            thing_destroy(t, "too many things");
        }
    }
}

static void thing_try_to_flush_ids (void)
{
    thing_try_to_flush_ids_(thing_server_ids);
    thing_try_to_flush_ids_(thing_client_ids);
}

/*
 * Create a new thing.
 */
static void thing_try_to_flush_client_ids_ (thingp *ids)
{
    uint32_t i;

    for (i = THING_CLIENT_ID_MIN; i < THING_CLIENT_ID_MAX; i++) {
        thingp t = ids[i];

        if (!t) {
            continue;
        }

        if (thing_is_explosion(t)) {
            thing_destroy(t, "too many things");
        }
    }
}

static void thing_try_to_flush_client_ids (void)
{
    thing_try_to_flush_client_ids_(thing_client_ids);
}

static void thing_map_check_empty_ (thing_map *map, thingp *ids)
{
    uint32_t i;
    uint32_t x;
    uint32_t y;

    for (x = 0; x < MAP_WIDTH; x++) {
        for (y = 0; y < MAP_HEIGHT; y++) {
            thing_map_cell *cell = &map->cells[x][y];

            for (i = 0; i < cell->count; i++) {

                uint32_t m = cell->id[i];
                if (!m) {
                    continue;
                }

                thingp t = ids[m];

                ERR("thing ID %u still on map at %d,%d [%d]", m, x, y, i);

                ERR("thing ID %u %s still on map", m, thing_logname(t));
            }
        }
    }
}

static void thing_map_check_empty (void)
{
    thing_map_check_empty_(&thing_server_map, thing_server_ids);
    thing_map_check_empty_(&thing_client_map, thing_client_ids);
}

static void thing_map_dump_ (thing_map *map, const char *name)
{
    uint32_t i;
    uint32_t x;
    uint32_t y;

    FILE *fp;

    fp = fopen(name, "w");

    uint32_t width = 0;

    for (y = 0; y < MAP_HEIGHT; y++) {
        for (x = 0; x < MAP_WIDTH; x++) {
            thing_map_cell *cell = &map->cells[x][y];

            width = max(width, cell->count);
        }
    }

    for (y = 0; y < MAP_HEIGHT; y++) {
        for (x = 0; x < MAP_WIDTH; x++) {
            thing_map_cell *cell = &map->cells[x][y];

            for (i = 0; i < width; i++) {
                uint32_t m = cell->id[i];

                if (!m) {
                    fprintf(fp, "----- ");
                    continue;
                }
                
                fprintf(fp, "%5u ", m);
            }

            fprintf(fp, "|");
        }
        fprintf(fp, "\n");
    }
}

void thing_map_dump (void)
{
    thing_map_dump_(&thing_client_map, "client.map");
    thing_map_dump_(&thing_server_map, "server.map");
}

static void thing_map_sanity_ (thing_map *map, thingp *ids)
{
    uint32_t i;
    uint32_t x;
    uint32_t y;

    for (y = 0; y < MAP_HEIGHT; y++) {
        for (x = 0; x < MAP_WIDTH; x++) {
            thing_map_cell *cell = &map->cells[x][y];

            uint8_t found_end = 0;

            for (i = 0; i < MAP_THINGS_PER_CELL; i++) {
                uint32_t m = cell->id[i];

                if (!m) {
                    found_end = true;
                    continue;
                }

                if (found_end) {
                    thing_map_dump();

                    DIE("map elements are not contiguous at %d,%d", x, y);
                }

                thingp t = ids[m];

                if (!t) {
                    DIE("thing %p ID %u is invalid and on map", t, m);
                }

                verify(t);
            }
        }
    }
}

void thing_map_sanity (void)
{
    thing_map_sanity_(&thing_server_map, thing_server_ids);
    thing_map_sanity_(&thing_client_map, thing_client_ids);
}

void thing_map_remove (thingp t)
{
    int i;

    verify(t);

    int32_t x = t->map_x;
    int32_t y = t->map_y;

    /*
     * Check not on the map.
     */
    if ((x == -1) || (y == -1)) {
        return;
    }

    thing_map *map = thing_get_map(t);
    thing_map_cell *cell = &map->cells[x][y];

    if (!cell->count) {
        ERR("map count mismatch");
        return;
    }

    /*
     * Remove from the map.
     */
    for (i = 0; i < cell->count; i++) {
        uint32_t m = cell->id[i];
        if (m != t->thing_id) {
            continue;
        }

        if (i == cell->count - 1) {
            /*
             * Popping last element.
             */
            cell->id[i] = 0;
        } else {
            /*
             * Pop and swap last element.
             */
            cell->id[i] = cell->id[cell->count - 1];
            cell->id[cell->count - 1] = 0;
        }

        cell->count--;

        t->map_x = -1;
        t->map_y = -1;

        return;
    }

    DIE("did not find id %u/%s on map at %d,%d to remove", 
        t->thing_id, thing_logname(t), x, y);
}

void thing_map_add (thingp t, int32_t x, int32_t y)
{
    verify(t);

    if (!t->thing_id) {
        DIE("cannot add thing %s ID of 0", t->logname);
    }

    if (x < 0) {
        DIE("map underflow for thing %s", t->logname);
    }

    if (y < 0) {
        DIE("map y underflow for thing %s", t->logname);
    }

    if (x >= MAP_WIDTH) {
        DIE("map x overflow for thing %s", t->logname);
    }

    if (y >= MAP_HEIGHT) {
        DIE("map y overflow for thing %s", t->logname);
    }

    thingp *ids;
    if (t->on_server) {
        ids = thing_server_ids;
    } else {
        ids = thing_client_ids;
    }

    thing_map *map = thing_get_map(t);

    /*
     * Check not on the map.
     */
    if ((t->map_x != -1) || (t->map_y != -1)) {
        DIE("thing %s already on map at %d,%d", t->logname,
            t->map_x, t->map_y);
        return;
    }

    uint32_t i;

    /*
     * Sanity check we're not on already.
     */
    thing_map_cell *cell = &map->cells[x][y];

#ifdef ENABLE_MAP_SANITY
    for (i = 0; i < cell->count; i++) {
        uint32_t m = cell->id[i];

        if (!m) {
            continue;
        }

        /*
         * Something is on the map.
         */
        if (m == t->thing_id) {
            /*
             * It's us?
             */
            DIE("already found on map");
        }

        thingp p = ids[m];
        if (p == t) {
            DIE("already found thing %s on map", t->logname);
        }
    }
#endif

    if (cell->count == MAP_THINGS_PER_CELL) {
        /*
         * Try to find something we can boot out.
         */
        if (thing_is_explosion(t)) {
            /*
             * Don't bother. This is a transient thing.
             */
            return;
        }

        /*
         * This is a more important thing. Try and boot out something less
         * important.
         */
        for (i = 0; i < cell->count; i++) {
            uint32_t m = cell->id[i];
            if (!m) {
                DIE("expected to find a map id on the map here");
            }

            thingp p = ids[m];
            if (!p) {
                DIE("expected to find a thing on the map here");
            }

            if (thing_is_explosion(p) ||
                thing_is_weapon_swing_effect(p)) {
                thing_map_remove(p);
                break;
            }
        }
    }

    if (cell->count == MAP_THINGS_PER_CELL) {
        /*
         * We're hosed.
         */
        ERR("Server: Out of map slots trying to add %s", t->logname);

        for (i = 0; i < cell->count; i++) {
            uint32_t m = cell->id[i];
            thingp p = ids[m];

            LOG("  slot [%d] ID %u %s", i, m, p->logname);
        }

        return;
    }

    cell->id[cell->count] = t->thing_id;
    cell->count++;

    t->map_x = x;
    t->map_y = y;
}

/*
 * Create a new thing.
 */
thingp thing_server_new (const char *name, 
                         double x, double y,
                         thing_statsp stats)
{
    thingp t;
    tpp tp;

    tp = tp_find(name);
    if (!tp) {
        DIE("thing [%s] has no template", name);
    }

    if (!server_player_things) {
        server_player_things = 
            tree_alloc(TREE_KEY_INTEGER, "server_player_things");

        server_player_things->offset = STRUCT_OFFSET(struct thing_, tree2);
    }

    if (!server_active_things) {
        server_active_things = 
            tree_alloc(TREE_KEY_INTEGER, "server_active_things");
    }

    if (!server_boring_things) {
        server_boring_things = 
            tree_alloc(TREE_KEY_INTEGER, "server_boring_things");
    }

    t = (typeof(t)) myzalloc(sizeof(*t), "TREE NODE: thing");
    t->on_server = true;

    /*
     * Use a different base for monsters so that the IDs we create are going
     * to be contiguous and allows us to optimize when sending map updates.
     */
    uint32_t *next;
    uint32_t id;
    uint32_t min;
    uint32_t max;

    if (tp_is_monst(tp) ||
        tp_is_player(tp)) {
        next = &next_monst_thing_id;
        id = next_monst_thing_id;
        min = THING_ID_MAX / 2;
        max = THING_ID_MAX;
    } else {
        /*
         * Reserve thing id 0 for unused.
         */
        next = &next_thing_id;
        id = next_thing_id;
        min = 1;
        max = THING_ID_MAX / 2;
    }

    if (!id) {
        id = min;
    }

    /*
     * Find a free thing slot
     */
    int looped = 0;

    while (thing_server_ids[id]) {
        id++;
        if (id >= max) {
            id = min;
            looped++;

            /*
             * Try hard to reclaim space.
             */
            if (looped == 2) {
                thing_try_to_flush_ids();
            }

            if (looped == 3) {
                DIE("out of thing ids, min %u max %u!", min, max);
            }
        }
    }

    t->tree.key = id;
    thing_server_ids[id] = t;
    t->thing_id = id;
    if (!id) {
        DIE("sanity check, ID 0 never used min %u max %u", min, max);
    }

    *next = id + 1;
    if (*next >= max) {
        *next = min;
    }

    t->tp = tp;

    if (tp_is_player(tp)) {
        t->tree2.key = id;

        if (!tree_insert(server_player_things, &t->tree2.node)) {
            DIE("thing insert name [%s, %u] into server_player_things failed", 
                name, id);
        }

        t->on_server_player_things = true;
        t->tree2.node.is_static_mem = true;
    }

    if (tp_is_boring(tp)) {
        if (!tree_insert(server_boring_things, &t->tree.node)) {
            DIE("thing insert name [%s, %u] into server_boring_things failed", 
                name, id);
        }

        t->client_or_server_tree = server_boring_things;
    } else {
        if (!tree_insert(server_active_things, &t->tree.node)) {
            DIE("thing insert name [%s, %u] into server_active_things failed", 
                name, id);
        }

        t->on_active_list = true;
        t->client_or_server_tree = server_active_things;
    }

    t->logname = dynprintf("%s[%p, id %u] (server)", thing_short_name(t), t,
                           t->thing_id);

    /*
     * New items are top quality.
     */
    t->item.id = tp_to_id(tp);
    t->item.quality = THING_ITEM_QUALITY_MAX;
    t->item.quantity = tp_get_quantity(tp);
    t->item.cursed = tp_is_cursed(tp);

    /*
     * Start out with stats from the template.
     */
    if (stats) {
        memcpy(&t->stats, stats, sizeof(thing_stats));
    } else {
        memcpy(&t->stats, &tp->stats, sizeof(thing_stats));
    }

    t->stats.thing_id = id;

    /*
     * Start out with the items carried on the template if any. This is not 
     * done for players as they do this in the provisioning screen.
     */
    uint32_t i, j;

    if (!thing_is_player(t)) {
        for (i = 0; i < THING_MAX; i++) {
            for (j = 0; j < tp->base_items[i].quantity; j++) {
                thing_auto_collect(t, 0 /* it */, id_to_tp(i));
            }
        }
    }

    thing_wield_next_weapon(t);

    thing_server_init(t, x, y);

    if (t->on_server) {
        server_things_total++;
    } else {
        server_things_total++;
    }

    if (!thing_is_boring_noverify(t)) {
        if (t->on_server) {
            LOG("Server: created %s (total %d)",
                thing_logname(t), server_things_total);
        } else {
            LOG("Client: created %s (total %d)",
                thing_logname(t), client_things_total);
        }
    }

    return (t);
}

/*
 * Reinit this player on a new level
 */
void thing_server_init (thingp t, double x, double y)
{
    /*
     * Start out not on the map.
     */
    t->last_x = -1.0;
    t->last_y = -1.0;
    t->map_x = -1.0;
    t->map_y = -1.0;
    t->x = x;
    t->y = y;

    thing_update(t);

    /*
     * So we send a move update to the client.
     */
    t->last_tx = -1;
    t->last_ty = -1;
    t->first_update = true;

#if 0
    if (thing_is_player(t)) {
        /*
         * So the client sees any carried weapons at start.
         */
        t->needs_tx_player_update = true;
    }
#endif
}

/*
 * Create a new thing.
 */
thingp thing_client_new (uint32_t id, tpp tp)
{
    thingp t;

    if (!client_player_things) {
        client_player_things = 
            tree_alloc(TREE_KEY_INTEGER, "client_player_things");

        client_player_things->offset = STRUCT_OFFSET(struct thing_, tree2);
    }

    if (!client_active_things) {
        client_active_things = 
                        tree_alloc(TREE_KEY_INTEGER, "TREE ROOT: thing");
    }

    if (!client_boring_things) {
        client_boring_things = 
                        tree_alloc(TREE_KEY_INTEGER, "TREE ROOT: thing");
    }

    t = (typeof(t)) myzalloc(sizeof(*t), "TREE NODE: thing");
    t->map_x = -1.0;
    t->map_y = -1.0;

    t->tree.key = id;
    thing_client_ids[id] = t;
    t->tp = tp;
    t->on_server = false;

    if (tp_is_player(tp)) {
        t->tree2.key = id;

        if (!tree_insert(client_player_things, &t->tree2.node)) {
            DIE("thing insert id [%u] failed into client_player_things", id);
        }

        t->on_client_player_things = true;
        t->tree2.node.is_static_mem = true;
    }

    if (tp_is_boring(tp)) {
        if (!tree_insert(client_boring_things, &t->tree.node)) {
            DIE("thing insert id [%u] failed into client_boring_things", id);
        }

        t->client_or_server_tree = client_boring_things;
    } else {
        if (!tree_insert(client_active_things, &t->tree.node)) {
            DIE("thing insert id [%u] failed into client_active_things", id);
        }

        t->client_or_server_tree = client_active_things;
        t->on_active_list = true;
    }

    /*
     * Mirror the thing id of the server on the client.
     */
    t->thing_id = id;

    t->logname = dynprintf("%s[%p, id %u] (client)", thing_short_name(t), t,
                           t->thing_id);

    if (!thing_is_boring_noverify(t)) {
        if (t->on_server) {
            LOG("Server: created %s", thing_logname(t));
        } else {
            LOG("Client: created %s", thing_logname(t));
        }
    }

    return (t);
}

/*
 * Create a new thing that only lives on this client.
 */
thingp thing_client_local_new (tpp tp)
{
    /*
     * Use a different base for monsters so that the IDs we create are going
     * to be contiguous and allows us to optimize when sending map updates.
     */
    uint32_t *next;
    uint32_t id;
    uint32_t min;
    uint32_t max;

    min = THING_CLIENT_ID_MIN / 2;
    max = THING_CLIENT_ID_MAX;

    next = &next_client_thing_id;
    id = next_client_thing_id;

    if (!id) {
        id = min;
    }

    /*
     * Find a free thing slot
     */
    int looped = 0;

    while (thing_client_ids[id]) {
        id++;
        if (id >= max) {
            id = min;
            looped++;

            /*
             * Try hard to reclaim space.
             */
            if (looped == 2) {
                thing_try_to_flush_client_ids();
            }

            if (looped == 3) {
                DIE("out of thing ids, min %u max %u!", min, max);
            }
        }
    }

    if (!id) {
        DIE("sanity check, ID 0 never used min %u max %u", min, max);
    }

    *next = id + 1;
    if (*next >= max) {
        *next = min;
    }

    return (thing_client_new(id, tp));
}

/*
 * Find an existing thing.
 */
thingp thing_client_find (uint32_t id)
{
    thing target;
    thingp result;

    // memset(&target, 0, sizeof(target));
    target.tree.key = id;

    result = (typeof(result)) 
                    tree_find(client_active_things, &target.tree.node);
    if (!result) {
        result = (typeof(result)) 
                        tree_find(client_boring_things, &target.tree.node);
    }

    return (result);
}

/*
 * Find an existing thing.
 */
thingp thing_server_find (uint32_t id)
{
    thing target;
    thingp result;

    // memset(&target, 0, sizeof(target));
    target.tree.key = id;

    result = (typeof(result)) 
                    tree_find(server_active_things, &target.tree.node);
    if (!result) {
        result = (typeof(result)) 
                        tree_find(server_boring_things, &target.tree.node);
    }

    return (result);
}

void thing_restarted (thingp t, levelp level)
{
    verify(t);

    t->current_tile = 0;

    if (!thing_is_dead(t)) {
        return;
    }

    if (t->dead_reason) {
        myfree(t->dead_reason);
        t->dead_reason = 0;
    }

    thing_set_is_dead(t, false);

    /*
     * Record this thing opened the exit.
     */
    thing_set_opened_exit(t, false);
    thing_set_got_to_exit_first(t, false);
}

/*
 * Get rid of all the hooks to other things that this thing has. e.g. the
 * weapons it carries etc.
 */
static void thing_remove_hooks (thingp t)
{
    verify(t);

    /*
     * We are owned by something. i.e. we are a sword.
     */
    if (t->owner_id) {
        thingp owner = thing_owner(t);
        if (!owner) {
            ERR("no owner for thing id %u", t->owner_id);
            return;
        }

        if (t->thing_id == owner->weapon_carry_anim_id) {
            owner->weapon_carry_anim_id = 0;
        }

        if (t->thing_id == owner->weapon_swing_anim_id) {
            owner->weapon_swing_anim_id = 0;

            /*
             * End of the swing animation, make the sword visible again.
             */
            thingp carry = thing_weapon_carry_anim(owner);
            if (carry) {
                /*
                 * But only if the owner is visible. They may have reached the 
                 * level.
                 */
                if (thing_is_visible(owner)) {
                    if (!t->on_server) {
                        thing_visible(carry);
                    }
                }
            }
        }

        t->owner_id = 0;
    }

    /*
     * Detach from the owner
     */
    if (t->on_server) {
        /*
         * We own things like a sword. i.e. we are a player.
         */
        if (t->weapon_carry_anim_id) {
            thingp item = thing_weapon_carry_anim(t);
            t->weapon_carry_anim_id = 0;
            verify(item);
            item->owner_id = 0;
            thing_dead(item, 0, "weapon carry anim owner killed");
        }

        if (t->weapon_swing_anim_id) {
            thingp item = thing_weapon_swing_anim(t);
            t->weapon_swing_anim_id = 0;
            verify(item);
            item->owner_id = 0;
            thing_dead(item, 0, "weapon swing anim owner killed");
        }
    }
}

static void thing_destroy_implicit (thingp t)
{
    thing_destroy(t, "end of game");
}

void thing_destroy (thingp t, const char *why)
{
    verify(t);

    if (!thing_is_boring_noverify(t)) {
        if (t->on_server) {
            LOG("Server: destroy %s (%s)", thing_logname(t), why);
        } else {
            LOG("Client: destroy %s (%s)", thing_logname(t), why);
        }
    }
 
    /*
     * Stop all timers.
     */
    thing_timers_destroy(t);

    if (t->timers) {
        action_timers_destroy(&t->timers);
    }

    if (!tree_remove(t->client_or_server_tree, &t->tree.node)) {
        DIE("thing template destroy name [%s] failed", thing_name(t));
    }

    t->client_or_server_tree = 0;
    t->on_active_list = false;

    if (t->on_client_player_things) {
        t->on_client_player_things = false;
        if (!tree_remove(client_player_things, &t->tree2.node)) {
            DIE("thing template destroy name [%s] failed (2)", 
                thing_name(t));
        }
    }

    if (t->on_server_player_things) {
        t->on_server_player_things = false;
        if (!tree_remove(server_player_things, &t->tree2.node)) {
            DIE("thing template destroy name [%s] failed (3)", 
                thing_name(t));
        }
    }

    /*
     * Detach from the owner
     */
    thing_remove_hooks(t);

    /*
     * Destroy the things weapon. Eventually drop a backpack.
     */
    thing_unwield(t);

    if (t->dead_reason) {
        myfree(t->dead_reason);
        t->dead_reason = 0;
    }

    if (t->wid) {
        thing_map_remove(t);
        thing_set_wid(t, 0);
    }

    if (t->logname) {
        myfree(t->logname);
        t->logname = 0;
    }

    if (t->on_server) {
        thing_server_ids[t->thing_id] = 0;
    } else {
        thing_client_ids[t->thing_id] = 0;
    }

    /*
     * If this is a player on the server, tell the client the player has 
     * croaked it.
     */
    socketp s;

    TREE_WALK(sockets, s) {
        aplayer *p = s->player;
        if (!s->player) {
            continue;
        }

        if (p->thing != t) {
            continue;
        }

        if (t->on_server) {
            p->thing = 0;

            LOG("Server: \"%s\" (ID %u) player died", t->stats.pname, p->key);

            char *tmp = dynprintf("%s died", t->stats.pname);
            socket_tx_server_shout_except_to(CRITICAL, tmp, p->socket);
            myfree(tmp);

            break;
        }
    }

    /*
     * Record that the client player may have died so we do not disconnect.
     */
    if (t == player) {
        player = 0;

        client_player_died = true;
    }

    if (t->on_server) {
        server_things_total--;
    } else {
        client_things_total--;
    }

    myfree(t);
}

static void thing_dead_ (thingp t, thingp killer, char *reason)
{
    /*
     * Why did I die!? 8(
     */
    if (t->dead_reason) {
        myfree(t->dead_reason);
        t->dead_reason = 0;
    }
    
    /*
     * Detach from the owner
     */
    thing_remove_hooks(t);

    if (reason) {
        t->dead_reason = reason;
    }

    if (thing_is_player(t)) {
        THING_LOG(t, "dead (%s)", reason);

        /*
         * We have the gravestone now, I don't think we need this.
         *
        THING_SHOUT_AT(t, CRITICAL, "Killed by %s", reason);
         */
        aplayer *p = t->player;
        if (p) {
            char *tmp = dynprintf("%s Killed by %s", t->stats.pname, reason);
            socket_tx_server_shout_except_to(CRITICAL, tmp, p->socket);
            myfree(tmp);
        }

        /*
         * Tell the poor player they've croaked it.
         */
        if (t->on_server) {
            if (!t->player) {
                ERR("no player socket to send hiscores too");
            } else if (t->player->socket) {
                hiscore_try_to_add(t->stats.pname, reason, t->score);

                socket_tx_server_hiscore(t->player->socket, 
                                         t->stats.pname,
                                         reason,
                                         t->score);
            }
        }
    }
}

void thing_dead (thingp t, thingp killer, const char *reason, ...)
{
    va_list args;

    verify(t);

    /*
     * Pre death server events.
     */
    if (t->on_server) {
        /*
         * If the reason of death was collection, some things we do not want
         * to do.
         */
        if (!t->is_collected) {
            /*
             * When it dies, doth it polymorph and thus avoid the reaper?
             * e.g. a mob spawner dying and creating a smaller one.
             */
            const char *polymorph = 
                    tp_polymorph_on_death(t->tp);
            if (polymorph) {
                tpp what = tp_find(polymorph);
                if (!what) {
                    DIE("could now find %s to polymorph into on %s death",
                        polymorph, thing_logname(t));
                }

                /*
                 * It doth polymorph.
                 */
                t->resync = 1;
                t->tp = what;
                t->stats.hp = tp_get_stats_max_hp(what);
                t->stats.magic = tp_get_stats_max_magic(what);
                thing_update(t);

                /*
                 * Update the name to reflect the new thing type.
                 */
                myfree(t->logname);
                t->logname = dynprintf("%s[%p, id %u] (server)", 
                                       thing_short_name(t), t,
                                       t->thing_id);

                /*
                 * No need to update active things they do it automatically
                 * in the ticker.
                 */
#if 0
                socket_server_tx_map_update(0, server_boring_things, 
                                            "polymorph dead thing boring");
#endif
                return;
            }

            /*
             * Or perhaps it does die, but spawns something else, like the
             * player dying and creating a mob spawner.
             */
            const char *spawn = 
                    tp_spawn_on_death(t->tp);
            if (spawn) {
                thing_mob_spawn_on_death(t);
            }
        }
    }

    /*
     * You only die once.
     */
    if (t->is_dead) {
        return;
    }

    thing_set_is_dead(t, true);

    /*
     * Post death server events.
     */
    if (t->on_server) {
        /*
         * If this is a dead player, then rethink AI targets.
         */
        if (thing_is_player(t)) {
            level_set_monst_map_treat_doors_as_passable(server_level);
            level_set_monst_map_treat_doors_as_walls(server_level);
        }

        /*
         * Bounty for the killer?
         */
        uint32_t score = tp_get_bonus_xp_on_death(
                                                thing_tp(t));
        if (score && killer) {
            thingp recipient = killer;

            /*
             * Did someone throw this weapon and gets the score?
             */
            if (killer->owner_id) {
                recipient = thing_owner(killer);
                verify(recipient);
            }

            thing_set_score(recipient, thing_score(recipient) + score);
        }
    }

    /*
     * Flash briefly red on death.
     */
    if (thing_is_monst(t) || 
        thing_is_mob_spawner(t) || 
        thing_is_wall(t) || 
        thing_is_door(t)) {

        widp w = t->wid;
        if (w) {
            wid_set_mode(w, WID_MODE_ACTIVE);
            if (!wid_is_hidden(w)) {
                wid_set_color(w, WID_COLOR_BLIT, RED);
            }
        }
    }

    /*
     * Stop bouncing or swaying.
     */
    if (t->wid) {
        if (tp_is_effect_pulse(t->tp)) {
            wid_scaling_to_pct_in(t->wid, 1.0, 1.0, 0, 0);
        }

        if (tp_is_effect_sway(t->tp)) {
            wid_rotate_to_pct_in(t->wid, 0, 0, 0, 0);
        }
    }

    /*
     * Log the means of death!
     */
    if (reason) {
        va_start(args, reason);
        thing_dead_(t, killer, dynvprintf(reason, args));
        va_end(args);
    } else {
        thing_dead_(t, killer, 0);
    }

    /*
     * Move the thing from the boring list to the active list and update it so 
     * that it gets sent to the client.
     */
    if (t->on_server) {
        thing_update(t);
    }

    if (!t->on_active_list) {
        if (!tree_remove(t->client_or_server_tree, &t->tree.node)) {
            DIE("thing move, remove boring [%s] failed", thing_name(t));
        }

        if (t->client_or_server_tree == server_boring_things) {
            if (!tree_insert(server_active_things, &t->tree.node)) {
                DIE("thing move, insert active [%s] failed", thing_name(t));
            }

            t->client_or_server_tree = server_active_things;
        } else if (t->client_or_server_tree == client_boring_things) {
            if (!tree_insert(client_active_things, &t->tree.node)) {
                DIE("thing move, insert active [%s] failed", thing_name(t));
            }

            t->client_or_server_tree = client_active_things;
        } else {
            DIE("bug, not on client or server list");
        }

        t->on_active_list = true;
    }
}

static int thing_hit_ (thingp t, 
                       thingp hitter, 
                       int32_t damage)
{
    verify(t);

    if (t->is_dead) {
        return (false);
    }

    /*
     * Take note of the hit so we can send an event to the client.
     */
    t->is_hit_miss = false;
    t->is_hit_success = true;
    if (damage > thing_get_stats_hp(t) / 10) {
        t->is_hit_crit = true;
    }

    thing_update(t);

    /*
     * Keep hitting until all damage is used up or the thing is dead.
     */
    while (damage > 0) {
        if (thing_get_stats_hp(t) <= damage) {
            damage -= thing_get_stats_hp(t);
            t->stats.hp = 0;

            /*
             * Record who dun it.
             */
            if (hitter) {
                thing_dead(t, hitter, "%s",
                           tp_short_name(hitter->tp));
            } else {
                thing_dead(t, hitter, "hit");
            }

            /*
             * Explodes on death ala Sith Lord? Only a lesser one, mind.
             */
            if (tp_is_combustable(t->tp)) {
                if (thing_is_monst(t)) {
                    level_place_small_cloudkill(thing_level(t),
                                                0, // owner
                                                t->x, t->y);
                } else {
                    level_place_small_explosion(thing_level(t),
                                                0, // owner
                                                t->x, t->y);
                }
            }

            /*
             * If polymorphed, hit again?
             */
            if (!thing_get_stats_hp(t)) {
                /*
                 * No it really died.
                 */
                break;
            }

            if (!damage) {
                /*
                 * If polymorphed, but we're out of damage.
                 */
                break;
            }
        } else {
            /*
             * A hit, but not enough to kill the thing.
             */
            t->stats.hp -= damage;

            if (thing_is_player(t)) {
                THING_LOG(t, "hit by (%s) for %u", 
                          thing_logname(hitter), damage);
            }

            damage = 0;
        }
    }

    /*
     * If a thing that modifies the level dies, update it.
     */
    if (thing_is_dead(t)) {
        /*
         * Destroying one door opens all doors.
         */
        if (t->on_server) {
            if (thing_is_door(t)) {
                level_open_door(server_level, t->x-1, t->y);
                level_open_door(server_level, t->x+1, t->y);
                level_open_door(server_level, t->x, t->y-1);
                level_open_door(server_level, t->x, t->y+1);
            }
        }

        if (thing_is_wall(t) || 
            thing_is_door(t) || 
            thing_is_pipe(t)) {
            level_update(server_level);
        }
    }

    return (true);
}

int thing_hit (thingp t, 
               thingp hitter, 
               uint32_t damage)
{
    thingp orig_hitter = hitter;

    verify(t);
    if (hitter) {
        verify(hitter);
    }

    if (t->is_dead) {
        return (false);
    }

    /*
     * Sanity check.
     */
    if (!t->on_server) {
        DIE("hits can only happen on the server");
        return (false);
    }

    if (hitter) {
        if (!hitter->on_server) {
            DIE("hits can only happen from hitter on the server");
            return (false);
        }
    }

    /*
     * If this is a thing on the edge of the level acting as a indestructable
     * wall, then don't allow it to be destroyed.
     */
    if (thing_is_wall(t) || thing_is_door(t) || thing_is_pipe(t)) {
        if ((t->x <= 0) || (t->x >= MAP_WIDTH - 1) ||
            (t->y <= 0) || (t->y >= MAP_HEIGHT - 1)) {
            return (false);
        }
    }

    /*
     * Check to see if this is a thing that can be damaged by the hitter.
     */
    if (hitter) {
        /*
         * Walls and doors and other solid object are not damaged by poison
         * or similar effects. Limit it to explosions and the like.
         */
        if (thing_is_door(t)            || 
            thing_is_wall(t)) {

            if (!thing_is_explosion(hitter)     &&
                !thing_is_projectile(hitter)    &&
                !thing_is_weapon_swing_effect(hitter)) {
                return (false);
            }
        }

        if (thing_is_weapon_swing_effect(hitter)) {
            if (!hitter->owner_id) {
                /*
                 * Happens with rapid swings as we only allow one active swing 
                 * per owner.
                 *
                ERR("swung weapon %s has no owner ID", thing_logname(hitter));
                 */
                return (false);
            }

            /*
             * Get the player swinging the weapon as the hitter.
             */
            hitter = thing_owner(hitter);
            if (!hitter) {
                ERR("weapon hitter %s owner id %u has no thing",
                    thing_logname(orig_hitter), orig_hitter->owner_id);
                return (false);
            }

            verify(hitter);

            /*
             * Don't let a sword hit too far away else we end up hitting
             * through walls! Actually I now like the effect. Keep it.
             */
#if 0
            double dist = DISTANCE(hitter->x, hitter->y, t->x, t->y);
            if (dist > 1.0) {
                /*
                 * Too far.
                 */
                return (false);
            }
#endif

            /*
             * Get the damage from the weapon being used to swing.
             */
            tpp weapon = thing_weapon(hitter);
            if (!weapon) {
                return (false);
            }

            if (!damage) {
                damage = tp_get_damage(weapon);
            }

        } else if (hitter->owner_id) {
            /*
             * Get the player firing the weapon as the hitter.
             */
            hitter = thing_owner(hitter);
            if (!hitter) {
                ERR("hitter %s owner id %u has no thing",
                    thing_logname(orig_hitter), orig_hitter->owner_id);
                return (false);
            }

            verify(hitter);
        }

        /*
         * Don't let our own potion hit ourselves!
         */
        if (hitter == t) {
            return (false);
        }

        /*
         * Don't allow one player's potion effect to kill another player.
         */
        if (thing_is_player(hitter) && thing_is_player(t)) {
            return (false);
        }

        /*
         * Take the damage from the weapon that hits first.
         */
        if (!damage) {
            damage = orig_hitter->damage;
            if (!damage) {
                damage = tp_get_damage(orig_hitter->tp);
            }
        }

        /*
         * If still no damage, use the thing that did the hitting.
         */
        if (!damage) {
            damage = hitter->damage;
            if (!damage) {
                damage = tp_get_damage(hitter->tp);
            }
        }
    }

    /*
     * Does the thing do damage?
     */
    if (!damage) {
        if (hitter) {
            damage = thing_tp(hitter)->damage;
        }
    }
    
    /*
     * Allow no more hits than x per second by the hitter.
     */
    if (orig_hitter) {
        /*
         * We want the orig hitter, i.e. the sword and not the playet.
         */
        uint32_t delay = 
            tp_get_hit_delay_tenths(orig_hitter->tp);

        if (delay) {
            if (!time_have_x_tenths_passed_since(delay, 
                                                 orig_hitter->timestamp_hit)) {

                return (false);
            }

            orig_hitter->timestamp_hit = time_get_time_cached();
        }

        /*
         * No killer to avoid giving a bonus to monsters!
         */
        if (thing_is_fragile(orig_hitter)) {
            thing_dead(orig_hitter, 0, "self destruct on hitting");
        }
    }

    /*
     * Flash briefly red on attempted hits.
     */
    if (thing_is_monst(t)               || 
        thing_is_mob_spawner(t)         || 
        thing_is_wall(t)                ||
        thing_is_door(t)) {

        /*
         * Assume missed due to the logic below where we detect chance.
         */
        if (orig_hitter && thing_is_explosion(orig_hitter)) {
            /*
             * No flashing
             */
        } else {
            t->is_hit_miss = true;
            thing_update(t);
        }
    }

    /*
     * Does the thing get off being hit.
     */
    uint32_t can_be_hit_chance = 
                    tp_get_can_be_hit_chance(t->tp);
    if (can_be_hit_chance) {
        uint32_t chance = rand() % (can_be_hit_chance + 1);

        if (chance > damage) {
            return (false);
        }
    }

    /*
     * Update the map for destruction of the scenery.
     */
    if (thing_is_wall(t) || thing_is_door(t) || thing_is_pipe(t)) {
        level_update(server_level);
    }

    int r;

    r = thing_hit_(t, hitter, damage);

    return (r);
}

thingp thing_owner (thingp t)
{
    verify(t);

    if (t->on_server) {
        if (t->owner_id) {
            thingp n = thing_server_ids[t->owner_id];
            if (!n) {
                DIE("no server thing for id %u for %s", t->owner_id,
                    thing_logname(t));
            }

            verify(n);
            return (n);
        }
    } else {
        if (t->owner_id) {
            thingp n = thing_client_ids[t->owner_id];
            if (!n) {
                DIE("no client thing for id %u for %s", t->owner_id,
                    thing_logname(t));
            }

            verify(n);
            return (n);
        }
    }

    return (0);
}

void thing_hide (thingp t)
{
    verify(t);

    widp w = t->wid;
    if (!w) {
        return;
    }

    if (t->on_server) {
        return;
    }

    if (wid_this_is_hidden(w)) {
        return;
    }

    wid_this_hide(w, 0);

    /*
     * Hide the weapon too or it just floats in the air.
     */
    thingp weapon_carry_anim = thing_weapon_carry_anim(t);
    if (weapon_carry_anim) {
        thing_hide(weapon_carry_anim);
    }
}

void thing_visible (thingp t)
{
    verify(t);

    widp w = t->wid;
    if (!w) {
        return;
    }

    if (t->on_server) {
        return;
    }

    if (!wid_this_is_hidden(w)) {
        return;
    }

    /*
     * If this thing has an owner, should the thing stay hidden?
     */
    thingp owner = thing_owner(t);
    if (owner) {
        if (t == thing_weapon_carry_anim(owner)) {
            if (thing_weapon_swing_anim(owner)) {
                /*
                 * Stay hidden until the weapon swing is done.
                 */
                return;
            }
        }
    }

    /*
     * Reveal the thing.
     */
    wid_this_visible(w, 0);

    /*
     * Reveal the weapon again too.
     */
    thingp weapon_carry_anim = thing_weapon_carry_anim(t);
    if (weapon_carry_anim) {
        thing_visible(weapon_carry_anim);
    }
}

uint8_t thing_is_visible (thingp t)
{
    verify(t);

    widp w = t->wid;
    if (w) {
        return (!wid_is_hidden(w));
    }

    return (false);
}

void thing_leave_level (thingp t)
{
    if (t->has_left_level) {
        return;
    }

    t->has_left_level = true;

#if 0
    if (thing_is_player(t)) {
        t->needs_tx_player_update = true;
    }
#endif

    level_set_exit_has_been_reached(server_level, true);

    /*
     * Make the weapon leave to
     */
    thingp weapon_carry_anim = thing_weapon_carry_anim(t);
    if (weapon_carry_anim) {
        thing_leave_level(weapon_carry_anim);
    }
}

void thing_join_level (thingp t)
{
    if (!t->has_left_level) {
        return;
    }

    t->has_left_level = false;

    /*
     * To force the client to move to the new start position.
     */
    t->resync = 1;

    t->needs_tx_player_update = true;

    /*
     * Make the weapon leave to
     */
    thingp weapon_carry_anim = thing_weapon_carry_anim(t);
    if (weapon_carry_anim) {
        thing_join_level(weapon_carry_anim);
    }
}

static void thing_effect_hit_crit (thingp t)
{
    verify(t);

    widp w = t->wid;
    if (w) {
        wid_set_mode(w, WID_MODE_ACTIVE);
        wid_set_color(w, WID_COLOR_BLIT, RED);
        if (thing_is_warm_blooded(t)) {
            level_place_blood_crit(client_level,
                                   0, // owner
                                   t->x, t->y);
        } else {
            level_place_hit_success(client_level,
                                    0, // owner
                                    t->x, t->y);
        }
    }
}

static void thing_effect_hit_success (thingp t)
{
    verify(t);

    widp w = t->wid;
    if (w) {
        wid_set_mode(w, WID_MODE_ACTIVE);
        wid_set_color(w, WID_COLOR_BLIT, RED);
        if (thing_is_warm_blooded(t)) {
            level_place_blood(client_level,
                              0, // owner
                              t->x, t->y);
        } else {
            level_place_hit_success(client_level,
                                    0, // owner
                                    t->x, t->y);
        }
    }
}

static void thing_effect_hit_miss (thingp t)
{
    verify(t);

    widp w = t->wid;
    if (w) {
        wid_set_mode(w, WID_MODE_ACTIVE);
        level_place_hit_miss(client_level,
                             0, // owner
                             t->x, t->y);
    }
}

void thing_reached_exit (thingp t)
{
    verify(t);

    /*
     * First to the exit?
     */
    if (!level_exit_reached_when_open(server_level)) {
        level_set_exit_reached_when_open(server_level, true);

        thing_set_got_to_exit_first(t, true);

        THING_LOG(t, "reached exit first");
    } else {
        THING_LOG(t, "reached exit");
    }

    /*
     * Pop from the level.
     */
    thing_set_wid(t, 0);

    sound_play_level_end();
}

int thing_is_player_or_owned_by_player (thingp t)
{
    if (thing_is_player(t)) {
        return (true);
    }

    /*
     * Keep owned weapons too.
     */
    thingp owner = thing_owner(t);
    if (owner) {
        if (thing_is_player(owner)) {
            return (true);
        }
    }

    return (false);
}

void things_level_destroyed (levelp level, uint8_t keep_players)
{
    thingp t;

    /*
     * Ensure no stale pointers.
     */
    if (level == server_level) {
        {
            TREE_WALK(server_active_things, t) {
                if (keep_players && 
                    thing_is_player_or_owned_by_player(t) &&
                    !thing_is_animation(t) &&
                    !thing_is_weapon_swing_effect(t)) {

                    thing_map_remove(t);
                    thing_set_wid(t, 0);
                    continue;
                }

                thing_destroy(t, "level destroyed");
            }
        }

        {
            TREE_WALK(server_boring_things, t) {
                thing_destroy(t, "level destroyed");
            }
        }

        {
            TREE_WALK(server_active_things, t) {
                if (keep_players && 
                    thing_is_player_or_owned_by_player(t)) {
                    continue;
                }

                DIE("thing still exists %s", thing_logname(t));
            }
        }

        {
            TREE_WALK(server_boring_things, t) {
                DIE("thing still exists %s", thing_logname(t));
            }
        }
    }

    if (level == client_level) {
        {
            TREE_WALK(client_active_things, t) {
                if (keep_players && 
                    thing_is_player_or_owned_by_player(t) &&
                    !thing_is_animation(t) &&
                    !thing_is_weapon_swing_effect(t)) {

                    thing_map_remove(t);
                    thing_set_wid(t, 0);
                    continue;
                }

                thing_destroy(t, "level destroyed");
            }
        }

        {
            TREE_WALK(client_boring_things, t) {
                thing_destroy(t, "level destroyed");
            }
        }

        {
            TREE_WALK(client_active_things, t) {
                if (keep_players && 
                    thing_is_player_or_owned_by_player(t)) {
                    continue;
                }

                DIE("thing still exists %s", thing_logname(t));
            }
        }

        {
            TREE_WALK(client_boring_things, t) {
                DIE("thing still exists %s", thing_logname(t));
            }
        }
    }
}

const char *thing_logname (thingp t)
{
    verify(t);

    return (t->logname);
}

const char *thing_short_name (thingp t)
{
    verify(t);

    return (tp_short_name(t->tp));
}

const char *thing_dead_reason (thingp t)
{
    verify(t);

    return (t->dead_reason);
}

uint8_t thing_z_depth (thingp t)
{
    verify(t);

    return (tp_get_z_depth(t->tp));
}

uint8_t thing_z_order (thingp t)
{
    verify(t);

    return (tp_get_z_order(t->tp));
}

uint32_t thing_score (thingp t)
{
    verify(t);

    return (t->score);
}

void thing_set_score (thingp t, uint32_t score)
{
    verify(t);

    t->score = score;

    if (thing_is_player(t)) {
        t->needs_tx_player_update = true;
    }
}

tree_rootp thing_tile_tiles (thingp t)
{
    verify(t);

    return (tp_get_tiles(t->tp));
}

tree_rootp thing_tile_tiles2 (thingp t)
{
    verify(t);

    return (tp_get_tiles2(t->tp));
}

widp thing_wid (thingp t)
{
    verify(t);

    return (t->wid);
}

void thing_set_wid (thingp t, widp w)
{
    verify(t);

    if (w) {
        verify(w);
    } else {
        if (t->wid) {
            verify(t->wid);
            wid_set_thing(t->wid, 0);
            wid_fade_out(t->wid, 100);
            wid_destroy_in(t->wid, 100);
        }
    }

    t->wid = w;
}

void thing_inc_powerup_spam_count (thingp t, uint8_t val)
{
    verify(t);

    t->powerup_spam_count += val;
}

void thing_dec_powerup_spam_count (thingp t, uint8_t val)
{
    verify(t);

    if (!t->powerup_spam_count) {
        DIE("powerup decreemnt");
    }

    t->powerup_spam_count -= val;
}

uint8_t thing_has_powerup_spam_count (thingp t)
{
    verify(t);

    return (t->powerup_spam_count);
}

void thing_inc_powerup2_count (thingp t, uint8_t val)
{
    verify(t);

    t->powerup2_count += val;
}

void thing_dec_powerup2_count (thingp t, uint8_t val)
{
    verify(t);

    if (!t->powerup2_count) {
        DIE("powerup decreemnt");
    }

    t->powerup2_count -= val;
}

uint8_t thing_has_powerup2_count (thingp t)
{
    verify(t);

    return (t->powerup2_count);
}

void thing_inc_powerup3_count (thingp t, uint8_t val)
{
    verify(t);

    t->powerup3_count += val;
}

void thing_dec_powerup3_count (thingp t, uint8_t val)
{
    verify(t);

    if (!t->powerup3_count) {
        DIE("powerup decreemnt");
    }

    t->powerup3_count -= val;
}

uint8_t thing_has_powerup3_count (thingp t)
{
    verify(t);

    return (t->powerup3_count);
}

void thing_inc_powerup4_count (thingp t, uint8_t val)
{
    verify(t);

    t->powerup4_count += val;
}

void thing_dec_powerup4_count (thingp t, uint8_t val)
{
    verify(t);

    if (!t->powerup4_count) {
        DIE("powerup decreemnt");
    }

    t->powerup4_count -= val;
}

uint8_t thing_has_powerup4_count (thingp t)
{
    verify(t);

    return (t->powerup4_count);
}

void thing_inc_powerup5_count (thingp t, uint8_t val)
{
    verify(t);

    t->powerup5_count += val;
}

void thing_dec_powerup5_count (thingp t, uint8_t val)
{
    verify(t);

    if (!t->powerup5_count) {
        DIE("powerup decreemnt");
    }

    t->powerup5_count -= val;
}

uint8_t thing_has_powerup5_count (thingp t)
{
    verify(t);

    return (t->powerup5_count);
}

void thing_inc_powerup6_count (thingp t, uint8_t val)
{
    verify(t);

    t->powerup6_count += val;
}

void thing_dec_powerup6_count (thingp t, uint8_t val)
{
    verify(t);

    if (!t->powerup6_count) {
        DIE("powerup decreemnt");
    }

    t->powerup6_count -= val;
}

uint8_t thing_has_powerup6_count (thingp t)
{
    verify(t);

    return (t->powerup6_count);
}

void thing_inc_powerup7_count (thingp t, uint8_t val)
{
    verify(t);

    t->powerup7_count += val;
}

void thing_dec_powerup7_count (thingp t, uint8_t val)
{
    verify(t);

    if (!t->powerup7_count) {
        DIE("powerup decreemnt");
    }

    t->powerup7_count -= val;
}

uint8_t thing_has_powerup7_count (thingp t)
{
    verify(t);

    return (t->powerup7_count);
}

void thing_inc_powerup8_count (thingp t, uint8_t val)
{
    verify(t);

    t->powerup8_count += val;
}

void thing_dec_powerup8_count (thingp t, uint8_t val)
{
    verify(t);

    if (!t->powerup8_count) {
        DIE("powerup decreemnt");
    }

    t->powerup8_count -= val;
}

uint8_t thing_has_powerup8_count (thingp t)
{
    verify(t);

    return (t->powerup8_count);
}

void thing_inc_powerup_rocket_count (thingp t, uint8_t val)
{
    verify(t);

    t->powerup_rocket_count += val;

    THING_DBG(t, "powerup inc, rocket now %d", t->powerup_rocket_count);

    sound_play_rocket();
}

void thing_dec_powerup_rocket_count (thingp t, uint8_t val)
{
    verify(t);

    if (!t->powerup_rocket_count) {
        DIE("%s, powerup dec, rocket now %d", t->logname,
            t->powerup_rocket_count);
    }

    t->powerup_rocket_count -= val;

    THING_DBG(t, "powerup dec, rocket now %d", t->powerup_rocket_count);
}

uint8_t thing_has_powerup_rocket_count (thingp t)
{
    verify(t);

    return (t->powerup_rocket_count);
}

void thing_set_is_light_source (thingp t, uint8_t val)
{
    verify(t);

    t->is_light_source = val;
}

uint8_t thing_is_light_source (thingp t)
{
    verify(t);

    return (t->is_light_source);
}

void thing_set_is_candle_light (thingp t, uint8_t val)
{
    verify(t);

    t->is_candle_light = val;
}

uint8_t thing_is_candle_light (thingp t)
{
    verify(t);

    return (t->is_candle_light);
}

void thing_set_qqq6 (thingp t, uint8_t val)
{
    verify(t);

    t->qqq6 = val;
}

uint8_t thing_qqq6 (thingp t)
{
    verify(t);

    return (t->qqq6);
}

void thing_set_is_collected (thingp t, uint8_t val)
{
    verify(t);

    t->is_collected = val;
}

uint8_t thing_is_collected (thingp t)
{
    verify(t);

    return (t->is_collected);
}

void thing_set_got_to_exit_first (thingp t, uint8_t val)
{
    verify(t);

    t->got_to_exit_first = val;
}

uint8_t thing_got_to_exit_first (thingp t)
{
    verify(t);

    return (t->got_to_exit_first);
}

void thing_set_is_dir_down (thingp t)
{
    verify(t);

    if (thing_is_animated_no_dir(t)) {
        return;
    }

    if (t->dir != THING_DIR_DOWN) {
        t->dir = THING_DIR_DOWN;
        if (t->on_server) {
            thing_update(t);
        }
    }
}

uint8_t thing_is_dir_down (thingp t)
{
    verify(t);

    return (t->dir == THING_DIR_DOWN);
}

void thing_set_is_dir_up (thingp t)
{
    verify(t);

    if (thing_is_animated_no_dir(t)) {
        return;
    }

    if (t->dir != THING_DIR_UP) {
        t->dir = THING_DIR_UP;
        if (t->on_server) {
            thing_update(t);
        }
    }
}

uint8_t thing_is_dir_up (thingp t)
{
    verify(t);

    return (t->dir == THING_DIR_UP);
}

void thing_set_is_dir_left (thingp t)
{
    verify(t);

    if (thing_is_animated_no_dir(t)) {
        return;
    }

    if (t->dir != THING_DIR_LEFT) {
        t->dir = THING_DIR_LEFT;
        if (t->on_server) {
            thing_update(t);
        }
    }
}

uint8_t thing_is_dir_left (thingp t)
{
    verify(t);

    return (t->dir == THING_DIR_LEFT);
}

void thing_set_is_dir_right (thingp t)
{
    verify(t);

    if (thing_is_animated_no_dir(t)) {
        return;
    }

    if (t->dir != THING_DIR_RIGHT) {
        t->dir = THING_DIR_RIGHT;
        if (t->on_server) {
            thing_update(t);
        }
    }
}

uint8_t thing_is_dir_right (thingp t)
{
    verify(t);

    return (t->dir == THING_DIR_RIGHT);
}

void thing_set_is_dir_tl (thingp t)
{
    verify(t);

    if (thing_is_animated_no_dir(t)) {
        return;
    }

    if (t->dir != THING_DIR_TL) {
        t->dir = THING_DIR_TL;
        if (t->on_server) {
            thing_update(t);
        }
    }
}

uint8_t thing_is_dir_tl (thingp t)
{
    verify(t);

    return (t->dir == THING_DIR_TL);
}

void thing_set_is_dir_bl (thingp t)
{
    verify(t);

    if (thing_is_animated_no_dir(t)) {
        return;
    }

    if (t->dir != THING_DIR_BL) {
        t->dir = THING_DIR_BL;
        if (t->on_server) {
            thing_update(t);
        }
    }
}

uint8_t thing_is_dir_bl (thingp t)
{
    verify(t);

    return (t->dir == THING_DIR_BL);
}

void thing_set_is_dir_tr (thingp t)
{
    verify(t);

    if (thing_is_animated_no_dir(t)) {
        return;
    }

    if (t->dir != THING_DIR_TR) {
        t->dir = THING_DIR_TR;
        if (t->on_server) {
            thing_update(t);
        }
    }
}

uint8_t thing_is_dir_tr (thingp t)
{
    verify(t);

    return (t->dir == THING_DIR_TR);
}

void thing_set_is_dir_br (thingp t)
{
    verify(t);

    if (thing_is_animated_no_dir(t)) {
        return;
    }

    if (t->dir != THING_DIR_BR) {
        t->dir = THING_DIR_BR;
        if (t->on_server) {
            thing_update(t);
        }
    }
}

uint8_t thing_is_dir_br (thingp t)
{
    verify(t);

    return (t->dir == THING_DIR_BR);
}

void thing_set_opened_exit (thingp t, uint8_t val)
{
    THING_LOG(t, "opened exit");

    verify(t);

    t->opened_exit = val;
}

uint8_t thing_opened_exit (thingp t)
{
    verify(t);

    return (t->opened_exit);
}

void thing_set_is_open (thingp t, uint8_t val)
{
    verify(t);

    t->timestamp_change_to_next_frame = time_get_time_cached();

    t->is_open = val;
}

void thing_set_is_dead (thingp t, uint8_t val)
{
    verify(t);

    t->is_dead = val;
}

const char *thing_name (thingp t)
{
    verify(t);

    return (tp_short_name(t->tp));
}

const char * thing_tooltip (thingp t)
{
    verify(t);

    return (tp_get_tooltip(t->tp));
}

double thing_speed (thingp t)
{
    verify(t);

    return (((double)tp_get_speed(t->tp)));
}

tree_rootp thing_tiles (thingp t)
{
    verify(t);

    return (tp_get_tiles(t->tp));
}

tree_rootp thing_tiles2 (thingp t)
{
    verify(t);

    return (tp_get_tiles2(t->tp));
}

int32_t thing_grid_x (thingp t)
{
    verify(t);

    return (t->x);
}

int32_t thing_grid_y (thingp t)
{
    verify(t);

    return (t->y);
}

thing_tilep thing_current_tile (thingp t)
{
    verify(t);

    return (t->current_tile);
}

/*
 * Place a thing after a delay.
 */
void thing_place_timed (tpp tp, 
                        double x,
                        double y,
                        uint32_t ms, 
                        uint32_t jitter,
                        uint8_t on_server)
{
    thing_place_context_t *context;

    context = (typeof(context)) myzalloc(sizeof(*context), "place thing");

    context->x = x;
    context->y = y;
    context->level = server_level;
    context->tp = tp;
    context->on_server = on_server ? 1 : 0;

    action_timer_create(
            &server_timers,
            (action_timer_callback) thing_timer_place_callback,
            (action_timer_callback) thing_timer_place_destroy_callback,
            context,
            "place thing",
            ms,
            jitter);
}

/*
 * Place a thing after a delay.
 */
void thing_place_and_destroy_timed (tpp tp, 
                                    thingp owner,
                                    double x,
                                    double y,
                                    uint32_t ms, 
                                    uint32_t destroy_in, 
                                    uint32_t jitter,
                                    uint8_t on_server,
                                    uint8_t is_epicenter)
{
    thing_place_context_t *context;

    context = (typeof(context)) myzalloc(sizeof(*context), "place thing");

    context->x = x;
    context->y = y;
    context->level = server_level;
    context->destroy_in = destroy_in;
    context->tp = tp;
    context->on_server = on_server ? 1 : 0;
    context->is_epicenter = is_epicenter ? 1 : 0;

    if (owner) {
        context->owner_id = owner->thing_id;

        action_timer_create(
                &owner->timers,
                (action_timer_callback)
                    thing_timer_place_and_destroy_callback,
                (action_timer_callback)
                    thing_timer_place_and_destroy_destroy_callback,
                context,
                "place and destroy thing",
                ms,
                jitter);
    } else {
        action_timer_create(
                &server_timers,
                (action_timer_callback)
                    thing_timer_place_and_destroy_callback,
                (action_timer_callback)
                    thing_timer_place_and_destroy_destroy_callback,
                context,
                "place and destroy thing",
                ms,
                jitter);
    }
}

void thing_teleport (thingp t, int32_t x, int32_t y)
{
    if (time_get_time_cached() - t->timestamp_teleport < 500) {
        return;
    }

    t->timestamp_teleport = time_get_time_cached();

    widp wid_next_floor = wid_grid_find_thing_template(
                                    wid_game_map_server_grid_container,
                                    x,
                                    y,
                                    tp_is_floor);
    if (!wid_next_floor) {
        DIE("no floor tile to hpp to");
    }

    double next_floor_x, next_floor_y;

    wid_get_mxy(wid_next_floor, &next_floor_x, &next_floor_y);

    widp w = thing_wid(t);

    wid_move_end(w);
    wid_move_to_abs_centered_in(w, next_floor_x, next_floor_y, 0);

    sound_play_level_end();
}

static void thing_move (thingp t, double x, double y)
{
    verify(t);

    if ((t->last_x == -1.0) && (t->last_y == -1.0)) {
        t->last_x = x;
        t->last_y = y;
    } else {
        t->last_x = t->x;
        t->last_y = t->y;
    }

    t->x = x;
    t->y = y;
}

static void thing_server_wid_move (thingp t, double x, double y, uint8_t is_new)
{
    thing_move(t, x, y);

    /*
     * Off the map? Perhaps between levels.
     */
    if (!t->wid) {
        return;
    }

    x *= server_tile_width;
    y *= server_tile_height;

    x += server_tile_width / 2;
    y += server_tile_height / 2;

    fpoint tl = { x, y };
    fpoint br = { x, y };

    double base_tile_width =
            ((1.0f / ((double)TILES_SCREEN_WIDTH)) *
                (double)global_config.video_gl_width);

    double base_tile_height =
            ((1.0f / ((double)TILES_SCREEN_HEIGHT)) *
                (double)global_config.video_gl_height);

    /*
     * Work out the tile size in a percentage of the screen.
     */
    br.x += base_tile_width;
    br.y += base_tile_height;

    /*
     * Now center the tile.
     */
    tl.x -= base_tile_width / 2.0;
    tl.y -= base_tile_height / 2.0;

    br.x -= base_tile_width / 2.0;
    br.y -= base_tile_height / 2.0;

    /*
     * Now the tile itself has a shadow that is 1/4 of the pixels.
     * The center is 24x24 and with shadow it is 32x32. We need to
     * stretch the tile so it overlaps so the inner 24x24 if seamless.
     */
    double tile_width = ((br.x - tl.x) / 
                         (double)TILE_PIX_WIDTH) * 
                            (double)TILE_PIX_WITH_SHADOW_WIDTH;

    double tile_height = ((br.y - tl.y) / 
                         (double)TILE_PIX_HEIGHT) * 
                            (double)TILE_PIX_WITH_SHADOW_HEIGHT;

    tl.y -= tile_height / 4.0;
    br.x += tile_width / 4.0;

    if (is_new || 
        thing_is_player(t) ||
        thing_is_weapon(t) ||
        thing_is_weapon_swing_effect(t)) {
        wid_set_tl_br(t->wid, tl, br);
    } else {
        wid_move_to_abs_in(t->wid, tl.x, tl.y, 1000.0 / thing_speed(t));
    }
}

void thing_server_wid_update (thingp t, double x, double y, uint8_t is_new)
{
    thing_server_wid_move(t, x, y, is_new);

    /*
     * Make the weapon follow the thing.
     */
    thingp weapon_carry_anim = thing_weapon_carry_anim(t);
    if (weapon_carry_anim) {
        thing_server_wid_move(weapon_carry_anim, x, y, is_new);
    }

    /*
     * Make the weapon being swung follow the thing.
     */
    thingp weapon_swing_anim = thing_weapon_swing_anim(t);
    if (weapon_swing_anim) {
        double dx = 0;
        double dy = 0;

        thing_weapon_swing_offset(t, &dx, &dy);
        thing_server_wid_move(weapon_swing_anim, x + dx, y + dy, is_new);
    }
}

static void thing_client_wid_move (thingp t, double x, double y, 
                                   uint8_t smooth)
{
    double dist = DISTANCE(t->x, t->y, x, y);

    if (smooth) {
        if (dist == 0.0) {
            return;
        }
    }

    thing_move(t, x, y);

    x *= client_tile_width;
    y *= client_tile_height;

    x += client_tile_width / 2;
    y += client_tile_height / 2;

    fpoint tl = { x, y };
    fpoint br = { x, y };

    double base_tile_width =
            ((1.0f / ((double)TILES_SCREEN_WIDTH)) *
                (double)global_config.video_gl_width);

    double base_tile_height =
            ((1.0f / ((double)TILES_SCREEN_HEIGHT)) *
                (double)global_config.video_gl_height);

    tilep tile = wid_get_tile(t->wid);
    double tw = tile_get_width(tile);
    double th = tile_get_height(tile);
    double scale_x = tw / TILE_WIDTH; 
    double scale_y = th / TILE_HEIGHT; 

    if (scale_x > 1) {
        base_tile_width *= scale_x;
        base_tile_height *= scale_y;
    }

    br.x += base_tile_width / 2.0;
    br.y += base_tile_height / 2.0;
    tl.x -= base_tile_width / 2.0;
    tl.y -= base_tile_height / 2.0;

    /*
     * Now the tile itself has a shadow that is 1/4 of the pixels.
     * The center is 24x24 and with shadow it is 32x32. We need to
     * stretch the tile so it overlaps so the inner 24x24 if seamless.
     */
    double tile_width = ((br.x - tl.x) / 
                         (double)TILE_PIX_WIDTH) * 
                            (double)TILE_PIX_WITH_SHADOW_WIDTH;

    double tile_height = ((br.y - tl.y) / 
                         (double)TILE_PIX_HEIGHT) * 
                            (double)TILE_PIX_WITH_SHADOW_HEIGHT;

    if (scale_x == 1) {
        tl.y -= tile_height / 4.0;
        br.x += tile_width / 4.0;
    }

    /*
     * Off the map? Perhaps between levels.
     */
    if (!t->wid) {
        return;
    }

    /*
     * Make the weapon follow the thing.
     */
    if (smooth) {
        double time_step = dist;
        double speed = thing_speed(t);

        /*
         * If a weapon is being carried, it has no speed. Move at the same 
         * speed as the carrier.
         */
        if (!speed) {
            thingp owner = thing_owner(t);
            if (owner) {
                speed = thing_speed(owner);
            }
        }

        if (!speed) {
            speed = 1;
        }

        double ms = (1000.0 / speed) / (1.0 / time_step);

        ms *= THING_MOVE_NETWORK_LATENCY_FUDGE;

        wid_move_to_abs_in(t->wid, tl.x, tl.y, ms);
    } else {
        wid_set_tl_br(t->wid, tl, br);
    }
}

void thing_client_wid_update (thingp t, double x, double y, uint8_t smooth)
{
    thing_client_wid_move(t, x, y, smooth);

    /*
     * Update the weapon being carried.
     */
    thingp weapon_carry_anim = thing_weapon_carry_anim(t);
    if (weapon_carry_anim) {
        thing_client_wid_move(weapon_carry_anim, x, y, smooth);
    }

    /*
     * Update the weapon being swung.
     */
    thingp weapon_swing_anim = thing_weapon_swing_anim(t);
    if (weapon_swing_anim) {
        double dx = 0;
        double dy = 0;

        thing_weapon_swing_offset(t, &dx, &dy);
        thing_client_wid_move(weapon_swing_anim, x + dx, y + dy, false);
    }
}

void socket_server_tx_map_update (socketp p, tree_rootp tree, const char *type)
{
    /*
     * The client should be told when the level is hidden and will destroy
     * all things on its end to save us sending a cleanup.
     */
    if (server_level) {
        if (level_is_ready_to_fade_out(server_level)) {
            return;
        }
    }

    /*
     * If no players, then send nothing.
     */
    {
        uint8_t players = 0;
        socketp sp;

        TREE_WALK_UNSAFE(sockets, sp) {
            if (!sp->player) {
                continue;
            }

            players++;
        }

        /*
         * No one playing yet?
         */
        if (!players) {
            return;
        }
    }

    /*
     * Allocate a fresh packet.
     */
    UDPpacket *packet = socket_alloc_msg();
    uint8_t *eodata = ((uint8_t*)packet->data) + MAX_PACKET_SIZE;
    uint8_t *odata = packet->data;
    uint8_t *data = packet->data;
    *data++ = MSG_SERVER_MAP_UPDATE;
    thingp t;

    uint32_t last_id;

    last_id = 0;

    TREE_WALK_UNSAFE(tree, t) {

        verify(t);

        tpp tp = t->tp;

        /*
         * As an optimization do not send dead events for explosions. Let the 
         * client destroy those on its own to save sending loads of events.
         */
        if (tp_is_explosion(tp)) {
            /*
             * Only send the center of a location, the client will then 
             * emulate the blast without us needing to send lots of thing IDs.
             */
            if (!t->is_epicenter) {
                t->updated--;
                continue;
            }

            if (!t->first_update) {
                t->updated--;
                continue;
            }
        }

        if (!t->first_update) {
            /*
             * Only send animations at the start. Let them time out on the 
             * client.
             */
            if (tp_is_weapon_swing_effect(tp)) {
                t->updated--;
                continue;
            }
        }

        /*
         * If updating to all sockets, decrement the update counter for this 
         * thing. We only send updates on modified things.
         */
        if (!p) {
            /*
             * No change to the thing? Nothing to send.
             */
            if (!t->updated) {
                continue;
            }

            /*
             * There is a change, but don't send too often.
             */
            if (thing_is_animation(t)) {
                /*
                 * Send thing animation changes all the time so that weapons
                 * stay close to the player.
                 */
            } else if (thing_is_dead(t)) {
                /*
                 * Send dead changes all the time as we use that on level end
                 */
            } else if (!time_have_x_thousandths_passed_since(
                    tp_get_tx_map_update_delay_thousandths(
                                                            tp),
                    t->timestamp_tx_map_update)) {
                continue;
            }

            t->timestamp_tx_map_update = time_get_time_cached();

            t->updated--;
        }

        /*
         * Work out what we are going to send.
         */
        uint8_t template_id = tp_to_id(tp);
        uint32_t id = t->thing_id;
        uint8_t tx;
        uint8_t ty;

        widp w = thing_wid(t);
        if (w) {
            tx = (uint8_t)(int)((t->x * ((double)256)) / MAP_WIDTH);
            ty = (uint8_t)(int)((t->y * ((double)256)) / MAP_HEIGHT);
        } else {
            tx = 0xFF;
            ty = 0xFF;
        }

        uint8_t state = t->dir | 
            ((t->resync         ? 1 : 0) << 
                THING_STATE_BIT_SHIFT_RESYNC) |
            ((t->is_dead        ? 1 : 0) << 
                THING_STATE_BIT_SHIFT_EXT_PRESENT) |
            ((t->has_left_level ? 1 : 0) << 
                THING_STATE_BIT_SHIFT_EXT_PRESENT) |
            ((t->owner_id       ? 1 : 0) << 
                THING_STATE_BIT_SHIFT_EXT_PRESENT) |
            ((t->is_hit_crit    ? 1 : 0) << 
                THING_STATE_BIT_SHIFT_EXT_PRESENT) |
            ((t->is_hit_success ? 1 : 0) << 
                THING_STATE_BIT_SHIFT_EXT_PRESENT) |
            ((t->is_hit_miss    ? 1 : 0) << 
                THING_STATE_BIT_SHIFT_EXT_PRESENT);

        /*
         * WARING: Keep the above and below in sync.
         */
        const uint8_t ext =
            ((t->is_dead        ? 1 : 0) << 
                THING_STATE_BIT_SHIFT_EXT_IS_DEAD) |
            ((t->has_left_level ? 1 : 0) << 
                THING_STATE_BIT_SHIFT_EXT_HAS_LEFT_LEVEL) |
            ((t->owner_id       ? 1 : 0) << 
                THING_STATE_BIT_SHIFT_EXT_OWNER_ID_PRESENT) |
            ((t->is_hit_crit ? 1 : 0) << 
                THING_STATE_BIT_SHIFT_EXT_IS_HIT_CRIT) |
            ((t->is_hit_success ? 1 : 0) << 
                THING_STATE_BIT_SHIFT_EXT_IS_HIT_SUCCESS) |
            ((t->is_hit_miss    ? 1 : 0) << 
                THING_STATE_BIT_SHIFT_EXT_IS_HIT_MISS);

        t->is_hit_crit = 0;
        t->is_hit_success = 0;
        t->is_hit_miss = 0;

        /*
         * Do we need to encode the thing template? Yes if this is the first 
         * update or sending to a new client.
         */
        if (t->first_update || p) {
            state |= 1 << THING_STATE_BIT_SHIFT_ID_TEMPLATE_PRESENT;
            state |= 1 << THING_STATE_BIT_SHIFT_XY_PRESENT;
        }

        /*
         * Send co-ordinates if we moved since last send?
         */
        if ((tx != t->last_tx) || (ty != t->last_ty)) {
            state |= 1 << THING_STATE_BIT_SHIFT_XY_PRESENT;
        }

        if (t->resync || t->is_dead) {
            state |= 1 << THING_STATE_BIT_SHIFT_ID_TEMPLATE_PRESENT;
            state |= 1 << THING_STATE_BIT_SHIFT_XY_PRESENT;
        }

        if ((tx == 0xFF) && (ty == 0xFF)) {
            /*
             * Do not send.
             */
            state &= ~(1 << THING_STATE_BIT_SHIFT_XY_PRESENT);
        }

        /*
         * If the ID is close to the previous one, send a delta instead.
         */
        if (id - last_id <= 255) {
            state |= 1 << THING_STATE_BIT_SHIFT_ID_DELTA_PRESENT;
        }

        /*
         * Write the data.
         */
        *data++ = state;

        if (state & (1 << THING_STATE_BIT_SHIFT_ID_DELTA_PRESENT)) {
            *data++ = id - last_id;
        } else {
            SDLNet_Write16(id, data);               
            data += sizeof(uint16_t);
        }

        if (state & (1 << THING_STATE_BIT_SHIFT_ID_TEMPLATE_PRESENT)) {
            *data++ = template_id;
        }

        if (state & (1 << THING_STATE_BIT_SHIFT_EXT_PRESENT)) {
            *data++ = ext;
        }

        if (state & (1 << THING_STATE_BIT_SHIFT_XY_PRESENT)) {
            *data++ = tx;
            *data++ = ty;
        }

        t->last_tx = tx;
        t->last_ty = ty;
        t->resync = 0;
        t->first_update = false;

        if (ext & (1 << THING_STATE_BIT_SHIFT_EXT_OWNER_ID_PRESENT)) {
            SDLNet_Write16(t->owner_id, data);               
            data += sizeof(uint16_t);
        }

        if (data + sizeof(msg_map_update) < eodata) {
            /*
             * Can fit more in.
             */
            last_id = id;
            continue;
        }

        last_id = 0;

        /*
         * We reached the limit for this packet? Send now.
         */
        packet->len = data - odata;

        /*
         * Broadcast to all clients.
         */
        socketp sp;

        TREE_WALK_UNSAFE(sockets, sp) {
            if (p && (p != sp)) {
                continue;
            }

            if (!sp->player) {
                continue;
            }

            write_address(packet, socket_get_remote_ip(sp));
            socket_tx_msg(sp, packet);
        }
            
        /*
         * Reuse the same packet.
         */
        data = packet->data;
        *data++ = MSG_SERVER_MAP_UPDATE;
    }

    /*
     * Any left over, send them now.
     */
    if (data != odata) {
        socketp sp;

        packet->len = data - odata;

        TREE_WALK_UNSAFE(sockets, sp) {
            if (!sp->player) {
                continue;
            }

            write_address(packet, socket_get_remote_ip(sp));
            socket_tx_msg(sp, packet);
        }
    }

    socket_free_msg(packet);
}

void socket_client_rx_map_update (socketp s, UDPpacket *packet, uint8_t *data)
{
    /*
     * Check we don't receive an update before we're ready for it. This can
     * happen on rejoins.
     */
    if (!wid_game_map_client_grid_container) {
        return;
    }

    uint8_t need_fixup = false;
    verify(s);

    /*
     * Cache the local player weapon. We do not accept updates for it as
     * we locally echo the weapon and player.
     */
    thingp weapon_carry_anim;
    if (player) {
        weapon_carry_anim = thing_weapon_carry_anim(player);
    } else {
        weapon_carry_anim = 0;
    }

    thingp weapon_swing_anim;
    if (player) {
        weapon_swing_anim = thing_weapon_swing_anim(player);
    } else {
        weapon_swing_anim = 0;
    }

    uint8_t *eodata = data + packet->len - 1;
    uint16_t last_id = 0;

    while (data < eodata) {
        uint8_t state = *data++;
        uint8_t ext;
        uint8_t template_id;
        uint16_t id;
        uint16_t owner_id = 0;
        uint8_t on_map;
        thingp t;
        double x;
        double y;
        uint8_t tx;
        uint8_t ty;

        if (state & (1 << THING_STATE_BIT_SHIFT_ID_DELTA_PRESENT)) {
            /*
             * Delta ID update.
             */
            id = *data++ + last_id;
        } else {
            /*
             * Full ID update.
             */
            id = SDLNet_Read16(data);
            data += sizeof(uint16_t);
        }
        last_id = id;

        if (state & (1 << THING_STATE_BIT_SHIFT_ID_TEMPLATE_PRESENT)) {
            /*
             * Full template ID update.
             */
            template_id = *data++;
        } else {
            template_id = -1;
        }

        if (state & (1 << THING_STATE_BIT_SHIFT_EXT_PRESENT)) {
            /*
             * Extensions present.
             */
            ext = *data++;
        } else {
            ext = 0;
        }

        if (state & (1 << THING_STATE_BIT_SHIFT_XY_PRESENT)) {
            /*
             * Full move update.
             */
            tx = *data++;
            ty = *data++;

            x = ((double)tx) / (double) (256 / MAP_WIDTH);
            y = ((double)ty) / (double) (256 / MAP_HEIGHT);
        } else {
            tx = 0xFF;
            ty = 0xFF;
            x = -1;
            y = -1;
        }

        if ((tx == 0xFF) && (ty == 0xFF)) {
            on_map = false;
        } else {
            on_map = true;
        }

        if (ext & (1 << THING_STATE_BIT_SHIFT_EXT_OWNER_ID_PRESENT)) {
            /*
             * Owner ID present
             */
            owner_id = SDLNet_Read16(data);
            data += sizeof(uint16_t);
        }

        t = thing_client_find(id);
        if (!t) {
            if (template_id == (uint8_t)-1) {
                /*
                 * This could happen due to packet loss and we have no way
                 * to rebuild the thing without a resend. Need a way to ask
                 * for a resync.
                 */
                ERR("Client: received unknown thing %u, need resync", id);
                continue;
            }

            tpp tp = 
                    id_to_tp(template_id);

            t = thing_client_new(id, tp);

            if (!need_fixup &&
                (tp_is_wall(tp) ||
                 tp_is_pipe(tp) ||
                 tp_is_door(tp))) {
                need_fixup = true;
            }
        } else {
            if (template_id != (uint8_t)-1) {
                /*
                 * Update the template ID so things can polymorph.
                 */
                tpp tp = id_to_tp(template_id);

                t->tp = tp;

                /*
                 * Polymorph the thing. This is not needed if the thing is
                 * animated. But if not, we do.
                 */
                if (!thing_is_animated(t) && !thing_is_joinable(t)) {
                    widp w = thing_wid(t);

                    if (w) {
                        wid_set_thing_template(w, tp);
                    }
                }

                if (ext & (1 << THING_STATE_BIT_SHIFT_EXT_HAS_LEFT_LEVEL)) {
                    /*
                     * If this thing is leaving the level, no need to update
                     * the map if it is a wall as all walls are leaving.
                     */
                } else {
                    if (!need_fixup &&
                        (tp_is_wall(tp) ||
                         tp_is_pipe(tp) ||
                         tp_is_door(tp))) {
                        need_fixup = true;
                    }
                }
            }
        }

        /*
         * Get the thing direction.
         */
        t->dir = state & 0x7;
        t->owner_id = owner_id;

        /*
         * Move the thing?
         */
        if (state & (1 << THING_STATE_BIT_SHIFT_XY_PRESENT)) {
            widp w = thing_wid(t);
            if (w) {
                if ((t == player) || 
                    ((t == weapon_carry_anim) && weapon_carry_anim) ||
                    ((t == weapon_swing_anim) && weapon_swing_anim)) {
                    /*
                     * Local echo only.
                     */
                    if ((state & (1 << THING_STATE_BIT_SHIFT_RESYNC))) {
                        /*
                         * Check we are roughly where the server thinks we 
                         * are. If wildly out of whack, correct our viewpoint.
                         */
                        THING_LOG(t, "%s server asked for resync", t->logname);
                        THING_LOG(t, "  server %f %f", t->x, t->y);
                        THING_LOG(t, "  client %f %f", x, y);

                        thing_client_wid_update(t, x, y, false /* smooth */);

                        wid_game_map_client_scroll_adjust(1);
                    } else 
                        if ((fabs(x-t->x) > THING_MAX_SERVER_DISCREPANCY * 2) ||
                            (fabs(y-t->y) > THING_MAX_SERVER_DISCREPANCY * 2)) {
                        /*
                         * Check we are roughly where the server thinks we 
                         * are. If wildly out of whack, correct our viewpoint.
                         */
                        THING_LOG(t, "%s out of sync with server, correcting ",
                                  t->logname);
                        THING_LOG(t, "  server %f %f", t->x, t->y);
                        THING_LOG(t, "  client %f %f", x, y);

                        thing_client_wid_update(t, x, y, false /* smooth */);

                        wid_game_map_client_scroll_adjust(1);
                    }
                } else if (on_map) {
                    /*
                     * Move something which is not the local player. Could
                     * be another player or monster etc...
                     */
                    thing_client_wid_update(t, x, y, true /* smooth */);
                }
            } else {
                if (!on_map) {
                    /*
                     * Popped off the map.
                     */
                } else {
                    /*
                     * We are only ever told about epicenters of explosions 
                     * for efficency.
                     */
                    if (thing_is_explosion(t)) {
                        t->is_epicenter = true;
                    }
                    
                    /*
                     * Thing has no wid. Make one.
                     */
                    wid_game_map_client_replace_tile(
                                            wid_game_map_client_grid_container,
                                            x, y, t);
                }
            }
        }

        if (ext & (1 << THING_STATE_BIT_SHIFT_EXT_IS_HIT_MISS)) {
            thing_effect_hit_miss(t);
        }

        if (ext & (1 << THING_STATE_BIT_SHIFT_EXT_IS_HIT_SUCCESS)) {
            thing_effect_hit_success(t);
        }

        if (ext & (1 << THING_STATE_BIT_SHIFT_EXT_IS_HIT_CRIT)) {
            thing_effect_hit_crit(t);
        }

        if (ext & (1 << THING_STATE_BIT_SHIFT_EXT_HAS_LEFT_LEVEL)) {
            thing_hide(t);
        } else {
            thing_visible(t);
        }

        if (ext & (1 << THING_STATE_BIT_SHIFT_EXT_IS_DEAD)) {
//LOG("rx %s dead",thing_logname(t));
            thing_dead(t, 0, "server killed");
        }
    }

    if (need_fixup) {
        levelp level;

        level = 
            (typeof(level)) wid_get_client_context(
                                        wid_game_map_client_grid_container);
        verify(level);

        map_fixup(level);
    }
}

#if 0
void socket_server_tx_player_update (thingp t)
{
    if (!thing_is_player(t)) {
        ERR("trying to send player update from not player thing %s",
            thing_logname(t));
        return;
    }

    /*
     * Allocate a fresh packet.
     */
    UDPpacket *packet = socket_alloc_msg();
    uint8_t *odata = packet->data;
    uint8_t *data = packet->data;

    *data++ = MSG_SERVER_PLAYER_UPDATE;

    SDLNet_Write16(t->thing_id, data);               
    data += sizeof(uint16_t);

    SDLNet_Write16(t->weapon_carry_anim_id, data);               
    data += sizeof(uint16_t);

    SDLNet_Write16(t->weapon_swing_anim_id, data);               
    data += sizeof(uint16_t);

    memcpy(data, &t->stats, sizeof(t->stats));
    data += sizeof(t->stats);

    if (t->weapon) {
        *data++ = tp_to_id(t->weapon);
    } else {
        *data++ = 0;
    }

    packet->len = data - odata;

    /*
     * Broadcast to all clients.
     */
    socketp sp;

    TREE_WALK_UNSAFE(sockets, sp) {
        if (!sp->player) {
            continue;
        }

        write_address(packet, socket_get_remote_ip(sp));
        socket_tx_msg(sp, packet);
    }
        
    socket_free_msg(packet);
}

void socket_client_rx_player_update (socketp s, UDPpacket *packet, 
                                     uint8_t *data)
{
    verify(s);

    uint16_t id = SDLNet_Read16(data);
    data += sizeof(uint16_t);

    thingp t = thing_client_find(id);
    if (!t) {
        ERR("thing id from server, id %u not found", id);
        return;
    }

    t->weapon_carry_anim_id = SDLNet_Read16(data);
    data += sizeof(uint16_t);

    t->weapon_swing_anim_id = SDLNet_Read16(data);
    data += sizeof(uint16_t);

    if (t->weapon_carry_anim_id) {
        thingp item = thing_weapon_carry_anim(t);
        if (item) {
            item->dir = t->dir;
            item->owner_id = t->thing_id;
        }
    }

    if (t->weapon_swing_anim_id) {
        thingp item = thing_weapon_swing_anim(t);
        if (item) {
            item->dir = t->dir;
            item->owner_id = t->thing_id;
        }
    }

    memcpy(&t->stats, data, sizeof(t->stats));
    data += sizeof(t->stats);

    id = *data++;
    if (id) {
        t->weapon = id_to_tp(id);
    } else {
        t->weapon = 0;
    }

    /*
     * If swinging a weapon now, hide the carried weapon until the swing is 
     * over.
     */
    if (t->weapon_swing_anim_id) {
        thingp carry = thing_weapon_carry_anim(t);
        if (carry) {
            thing_hide(carry);
        }
    }
}
#endif

static void thing_move_set_dir (thingp t,
                                double *x,
                                double *y,
                                uint8_t up,
                                uint8_t down,
                                uint8_t left,
                                uint8_t right)
{
    double ox = t->x;
    double oy = t->y;

    if (*x < 0) {
        *x = 0;
    }

    if (*y < 0) {
        *y = 0;
    }

    if (*x > MAP_WIDTH - 1) {
        *x = MAP_WIDTH - 1;
    }

    if (*y > MAP_HEIGHT - 1) {
        *y = MAP_HEIGHT - 1;
    }

    if (*x > ox) {
        right = true;
    }

    if (*x < ox) {
        left = true;
    }

    if (*y > oy) {
        down = true;
    }

    if (*y < oy) {
        up = true;
    }

    if (up) {
        if (left) {
            thing_set_is_dir_tl(t);
        } else if (right) {
            thing_set_is_dir_tr(t);
        } else {
            thing_set_is_dir_up(t);
        }
    } else if (down) {
        if (left) {
            thing_set_is_dir_bl(t);
        } else if (right) {
            thing_set_is_dir_br(t);
        } else {
            thing_set_is_dir_down(t);
        }
    } else if (left) {
        thing_set_is_dir_left(t);
    } else if (right) {
        thing_set_is_dir_right(t);
    }
}

void thing_client_move (thingp t,
                        double x,
                        double y,
                        const uint8_t up,
                        const uint8_t down,
                        const uint8_t left,
                        const uint8_t right,
                        const uint8_t fire)
{
    widp grid = wid_game_map_client_grid_container;

    if (t->wid) {
        if (thing_hit_solid_obstacle(grid, t, x, y)) {
            if (!thing_hit_solid_obstacle(grid, t, x, t->y)) {
                y = t->y;
            } else if (!thing_hit_solid_obstacle(grid, t, t->x, y)) {
                x = t->x;
            } else {
                x = t->x;
                y = t->y;
            }
        }
    }

    thing_move_set_dir(t, &x, &y, up, down, left, right);

    /*
     * Move the weapon too.
     */
    thingp weapon_carry_anim = thing_weapon_carry_anim(t);
    if (weapon_carry_anim) {
        thing_move_set_dir(weapon_carry_anim, &x, &y, up, down, left, right);
    }

    thingp weapon_swing_anim = thing_weapon_swing_anim(t);
    if (weapon_swing_anim) {
        thing_move_set_dir(weapon_swing_anim, &x, &y, up, down, left, right);
    }

    /*
     * If no widget yet then this can be a dummy move during thing creation
     * just to set the weapon anim correctly.
     */
    if (!t->wid) {
        return;
    }

    /*
     * Oddly doing smooth moving makes it more jumpy when scrolling.
     */
    thing_client_wid_update(t, x, y, false);

    socket_tx_player_move(client_joined_server, t, up, down, left, right, 
                          fire);
}

void thing_fire (thingp t,
                 const uint8_t up,
                 const uint8_t down,
                 const uint8_t left,
                 const uint8_t right)
{
    /*
     * Cannot fire until we're on a level.
     */
    if (!t->wid) {
        THING_LOG(t, "cannot fire yet, not on the level");
        return;
    }

    /*
     * Use the currently wielded weapon.
     */
    tpp weapon = thing_weapon(t);
    if (!weapon) {
        THING_LOG(t, "has no weapon, cannot fire");
        return;
    }

    /*
     * Check if the weapon reaches its end of warranty.
     */
    uint32_t d10000_chance_of_breaking = 
                    tp_get_d10000_chance_of_breaking(weapon);

    if (d10000_chance_of_breaking) {
        if ((rand() % 10000) <= d10000_chance_of_breaking) {
            thing_wear_out(t, weapon);
            THING_LOG(t, "damage weapon");
            return;
        }
    }

    double dx, dy;
    double dist_from_player = 0.1;

    /*
     * Try current direction.
     */
    dx = 0.0;
    dy = 0.0;

    /*
     * If the player is moving too then the weapon has a bit more
     * speed than if thrown when stationary.
     */
    if (down) {
        dy = dist_from_player;
    }

    if (up) {
        dy = -dist_from_player;
    }

    if (right) {
        dx = dist_from_player;
    }

    if (left) {
        dx = -dist_from_player;
    }

    /*
     * If no dir, then try the last thing dir.
     */
    if ((dx == 0) && (dy == 0)) {
        if (thing_is_dir_down(t)) {
            dy = dist_from_player;
        }

        if (thing_is_dir_up(t)) {
            dy = -dist_from_player;
        }

        if (thing_is_dir_right(t)) {
            dx = dist_from_player;
        }

        if (thing_is_dir_left(t)) {
            dx = -dist_from_player;
        }

        if (thing_is_dir_tl(t)) {
            dx = -dist_from_player;
            dy = -dist_from_player;
        }

        if (thing_is_dir_tr(t)) {
            dx = dist_from_player;
            dy = -dist_from_player;
        }

        if (thing_is_dir_bl(t)) {
            dx = -dist_from_player;
            dy = dist_from_player;
        }

        if (thing_is_dir_br(t)) {
            dx = dist_from_player;
            dy = dist_from_player;
        }
    }

    /*
     * Fire from the player position plus the initial delta so it looks like 
     * it comes from outside of the body.
     */
    double x = t->x;
    double y = t->y;

    x += dx;
    y += dy;

    tpp projectile = tp_fires(weapon);
    if (!projectile) {
        /*
         * Might be a sword.
         */
        thing_swing(t);
        return;
    }

    widp w = wid_game_map_server_replace_tile(
                                    wid_game_map_server_grid_container,
                                    x,
                                    y,
                                    0, /* thing */
                                    projectile,
                                    0 /* item */,
                                    0 /* stats */);

    thingp p = wid_get_thing(w);

    /*
     * Make sure we keep track of who fired so we can award scores.
     */
    p->owner_id = t->thing_id;

    /*
     * Round up say -0.7 to -1.0
     */
    dx *= 10.0;
    dy *= 10.0;
    dx /= (dist_from_player * 10.0);
    dy /= (dist_from_player * 10.0);

    p->dx = dx;
    p->dy = dy;
    p->dir = t->dir;

    double fnexthop_x = p->x + p->dx;
    double fnexthop_y = p->y + p->dy;

    thing_server_move(p,
            fnexthop_x,
            fnexthop_y,
            fnexthop_y < p->y,
            fnexthop_y > p->y,
            fnexthop_x < p->x,
            fnexthop_x > p->x,
            false);
}

uint8_t thing_server_move (thingp t,
                           double x,
                           double y,
                           const uint8_t up,
                           const uint8_t down,
                           const uint8_t left,
                           const uint8_t right,
                           const uint8_t fire)
{
    widp grid = wid_game_map_server_grid_container;

    thing_move_set_dir(t, &x, &y, up, down, left, right);

    /*
     * Move the weapon too.
     */
    thingp weapon_carry_anim = thing_weapon_carry_anim(t);
    if (weapon_carry_anim) {
        thing_move_set_dir(weapon_carry_anim, &x, &y, up, down, left, right);
    }

    thingp weapon_swing_anim = thing_weapon_swing_anim(t);
    if (weapon_swing_anim) {
        thing_move_set_dir(weapon_swing_anim, &x, &y, up, down, left, right);
    }

    if (thing_hit_solid_obstacle(grid, t, x, y)) {
        if ((x != t->x) &&
            !thing_hit_solid_obstacle(grid, t, x, t->y)) {
            y = t->y;
        } else if ((y != t->y) &&
                   !thing_hit_solid_obstacle(grid, t, t->x, y)) {
            x = t->x;
        } else {
            if (thing_is_player(t)) {
                THING_LOG(t, "client collision, ignore move");
                THING_LOG(t, "  server %f %f", t->x, t->y);
                THING_LOG(t, "  client %f %f", x, y);
            }

            return (false);
        }
    }

    if (thing_is_player(t)) {
        if ((fabs(x - t->x) > THING_MAX_SERVER_DISCREPANCY) ||
            (fabs(y - t->y) > THING_MAX_SERVER_DISCREPANCY)) {
            /*
             * Client is cheating?
             */
            THING_LOG(t, "client moved too much, ignore move");
            THING_LOG(t, "  server %f %f", t->x, t->y);
            THING_LOG(t, "  client %f %f", x, y);

            thing_update(t);
            t->resync = 1;

            return (false);
        }
    }

    thing_move_set_dir(t, &x, &y, up, down, left, right);

    /*
     * Move the weapon too.
     */
    if (weapon_carry_anim) {
        thing_move_set_dir(weapon_carry_anim, &x, &y, up, down, left, right);
    }

    if (weapon_swing_anim) {
        thing_move_set_dir(weapon_swing_anim, &x, &y, up, down, left, right);
    }

    if (fire) {
        thing_fire(t, up, down, left, right);
    }

    thing_server_wid_update(t, x, y, false /* is_new */);
    thing_update(t);

    thing_handle_collisions(wid_game_map_server_grid_container, t);

    return (true);
}

void thing_server_action (thingp t,
                          uint8_t action,
                          uint32_t action_bar_index)
{
    widp grid = wid_game_map_server_grid_container;

    if (action_bar_index >= THING_ACTION_BAR_MAX) {
        ERR("invalid action bar slot %u", action_bar_index);
        return;
    }

    itemp item = &t->stats.action_bar[action_bar_index];
    if (!item->id) {
        THING_SHOUT_AT(t, WARNING, "No item in that slot to use");
        return;
    }

    tpp tp = id_to_tp(item->id);
    if (!tp) {
        ERR("Unkown item use request, id %u", item->id);
        return;
    }

    if (!t->player) {
        ERR("no player to handle action");
        return;
    }

    socketp s = t->player->socket;
    if (!s) {
        ERR("no player socket to handle action");
        return;
    }

    switch (action) {
    case PLAYER_ACTION_USE: {
        if (tp_is_weapon(tp)) {
            thing_wield(t, tp);
            return;
        }

        if (item->id == THING_POTION_FIRE) {
            level_place_fireball(server_level, t, t->x, t->y);
            break;
        } else if (item->id == THING_POTION_MONSTICIDE) {
            level_place_poison(server_level, t, t->x, t->y);
            break;
        } else if (item->id == THING_POTION_CLOUDKILL) {
            level_place_cloudkill(server_level, t, t->x, t->y);
            break;
        }

        const char *message = tp_message_on_use(tp);
        if (message) {
            THING_SHOUT_AT(t, INFO, "%s", message);
            break;
        }

        /*
         * Failed to use.
         */
        THING_SHOUT_AT(t, WARNING, "Failed to use the %s", 
                       tp_short_name(tp));
        return;
    }

    case PLAYER_ACTION_DROP: {
        double dx = 0;
        double dy = 0;

        if (thing_is_dir_down(t)) {
            dy = 1.0;
        }

        if (thing_is_dir_up(t)) {
            dy = -1.0;
        }

        if (thing_is_dir_right(t)) {
            dx = 1.0;
        }

        if (thing_is_dir_left(t)) {
            dx = -1.0;
        }

        if (thing_is_dir_tl(t)) {
            dx = -1.0;
            dy = -1.0;
        }

        if (thing_is_dir_tr(t)) {
            dx = 1.0;
            dy = -1.0;
        }

        if (thing_is_dir_bl(t)) {
            dx = -1.0;
            dy = 1.0;
        }

        if (thing_is_dir_br(t)) {
            dx = 1.0;
            dy = 1.0;
        }

        /*
         * Sanity check we got one dir.
         */
        if ((dx == 0.0) && (dy == 0.0)) {
            dx = 1.0;
            dy = 1.0;
        }

        double x = t->x + dx;
        double y = t->y + dy;

        /*
         * Try to place in front of the player.
         */
        if (!thing_hit_any_obstacle(grid, t, x, y)) {
            if (wid_game_map_server_replace_tile(grid, x, y,
                                                 0, /* thing */
                                                 tp,
                                                 item,
                                                 0 /* stats */)) {
#if 0
                socket_server_tx_map_update(0, server_boring_things,
                                            "item drop");
#endif
                break;
            }
        }

        /*
         * Just place anywhere free.
         */
        for (dx = -1.0; dx <= 1.0; dx += 1.0) {
            for (dy = -1.0; dy <= 1.0; dy += 1.0) {
                double x = t->x + dx;
                double y = t->y + dy;

                if ((dx == 0.0) && (dy == 0.0)) {
                    continue;
                }

                if (!thing_hit_any_obstacle(grid, t, x, y)) {
                    if (wid_game_map_server_replace_tile(grid, x, y, 
                                                         0, /* thing */
                                                         tp,
                                                         item,
                                                         0 /* stats */)) {
#if 0
                        socket_server_tx_map_update(0, server_boring_things,
                                                    "item dropped");
#endif
                        goto done;
                    }
                }

            }
        }

        /*
         * Urk!
         */
        THING_SHOUT_AT(t, INFO, "Drop failed");

        /*
         * Failed to drop.
         */
        }
        return;

    default:
        ERR("Unkown player action %u on action bar item %u", 
            action, 
            action_bar_index);
        return;
    }

done:

    switch (action) {
    case PLAYER_ACTION_USE:
        thing_used(t, tp);
        break;

    case PLAYER_ACTION_DROP:
        thing_drop(t, tp);
        break;
    }
}
