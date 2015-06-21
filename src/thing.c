/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include "main.h"
#include "thing.h"
#include "thing_timer.h"
#include "wid_game_map_client.h"
#include "wid_game_map_server.h"
#include "map.h"
#include "level.h"
#include "time_util.h"
#include "string_util.h"
#include "sound.h"
#include "socket_util.h"
#include "client.h"
#include "timer.h"
#include "tile.h"
#include "wid_hiscore.h"
#include "wid_player_info.h"
#include "wid_player_inventory.h"
#include "wid_player_action.h"
#include "wid_choose_stats.h"
#include "thing_shop.h"

tree_root *server_player_things;
tree_root *client_player_things;

tree_root *server_active_things;
tree_root *client_active_things;

tree_root *server_boring_things;
tree_root *client_boring_things;

static int server_things_total;
static int client_things_total;

static int server_monst_things_total;
static int client_monst_things_total;

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

void thing_dump (const thingp t)
{
    const thing_statsp s = &t->stats;

    LOG("thing %s:", thing_logname(t));

    if (t->stats.pname[0]) {
        LOG("  %-20s %s", "name", t->stats.pname);
    }

    if (t->stats.pclass[0]) {
        LOG("  %-20s \"%s\"", "class", t->stats.pclass);
    }

    LOG("  %-20s %u", "thing_id", t->thing_id);

    thing_stats_dump(s);
}

void thing_update (thingp t)
{
    if (!t->on_server) {
        ERR("cannot update a thing %s for sending to the client on the client",
            thing_logname(t));
        return;
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
        if (!thing_is_dead_or_dying(weapon_carry_anim)) {
            thing_update(weapon_carry_anim);
        }
    }

    /*
     * Update the shield being carried.
     */
    thingp shield_carry_anim = thing_shield_carry_anim(t);
    if (shield_carry_anim) {
        if (!thing_is_dead_or_dying(shield_carry_anim)) {
            thing_update(shield_carry_anim);
        }
    }

    /*
     * Update the weapon being swung.
     */
    thingp weapon_swing_anim = thing_weapon_swing_anim(t);
    if (weapon_swing_anim) {
        if (!thing_is_dead_or_dying(weapon_swing_anim)) {
            thing_update(weapon_swing_anim);
        }
    }

    /*
     * Update the mawic being used.
     */
    thingp magic_anim = thing_magic_anim(t);
    if (magic_anim) {
        if (!thing_is_dead_or_dying(magic_anim)) {
            thing_update(magic_anim);
        }
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

        if (thing_is_cloud_effect(t)) {
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

        if (thing_is_cloud_effect(t)) {
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

                    ERR("map elements are not contiguous at %d,%d", x, y);
                }

                thingp t = ids[m];

                if (!t) {
                    ERR("thing %p ID %u is invalid and on map", t, m);
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

void thing_sanity (thingp t)
{
    verify(t);

    /*
     * On the client owners may be missing due to order of deletion
     * of objects.
     */
    if (t->on_server) {
        thingp tmp;
        tmp = thing_owner(t);
        if (tmp) {
            verify(tmp);
        }
    }

    thingp tmp = thing_weapon_carry_anim(t);
    if (tmp) {
        verify(tmp);
    }

    tmp = thing_shield_carry_anim(t);
    if (tmp) {
        verify(tmp);
    }

    tmp = thing_weapon_swing_anim(t);
    if (tmp) {
        verify(tmp);
    }

    tmp = thing_magic_anim(t);
    if (tmp) {
        verify(tmp);
    }
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

    ERR("did not find id %u/%s on map at %d,%d to remove",
        t->thing_id, thing_logname(t), x, y);
}

void thing_map_add (thingp t, int32_t x, int32_t y)
{
    verify(t);

    if (!t->thing_id) {
        ERR("cannot add thing %s ID of 0", t->logname);
    }

    if (x < 0) {
        ERR("map underflow for thing %s", t->logname);
    }

    if (y < 0) {
        ERR("map y underflow for thing %s", t->logname);
    }

    if (x >= MAP_WIDTH) {
        ERR("map x overflow for thing %s", t->logname);
    }

    if (y >= MAP_HEIGHT) {
        ERR("map y overflow for thing %s", t->logname);
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
        ERR("thing %s already on map at %d,%d", t->logname,
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
            ERR("already found on map");
        }

        thingp p = ids[m];
        if (p == t) {
            ERR("already found thing %s on map", t->logname);
        }
    }
#endif

    if (cell->count == MAP_THINGS_PER_CELL) {
        /*
         * Try to find something we can boot out.
         */
        if (thing_is_cloud_effect(t)    ||
            thing_is_explosion(t)       ||
            thing_is_weapon_swing_effect(t)) {
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
                ERR("expected to find a map id on the map here");
            }

            thingp p = ids[m];
            if (!p) {
                ERR("expected to find a thing on the map here");
            }

            if (thing_is_cloud_effect(p)    ||
                thing_is_explosion(p)       ||
                thing_is_weapon_swing_effect(p)) {
                /*
                 * Kick out this transient thing.
                 */
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
        ERR("thing [%s] has no template", name);
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
     * Make sure we don't treat changes to these stats with a client version
     * bump.
     */
    thing_stats_set_on_server(t, t->on_server);
    if (stats) {
        stats_set_on_server(stats, t->on_server);
    }

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
                ERR("out of thing ids, min %u max %u!", min, max);
            }
        }
    }

    t->tree.key = id;
    thing_server_ids[id] = t;
    t->thing_id = id;
    if (!id) {
        ERR("sanity check, ID 0 never used min %u max %u", min, max);
    }

    *next = id + 1;
    if (*next >= max) {
        *next = min;
    }

    t->tp = tp;

    if (tp_is_player(tp)) {
        t->tree2.key = id;

        if (!tree_insert(server_player_things, &t->tree2.node)) {
            ERR("thing insert name [%s, %u] into server_player_things failed",
                name, id);
        }

        t->on_server_player_things = true;
        t->tree2.node.is_static_mem = true;
    }

    if (tp_is_inactive(tp)) {
        if (!tree_insert(server_boring_things, &t->tree.node)) {
            ERR("thing insert name [%s, %u] into server_boring_things failed",
                name, id);
        }

        t->client_or_server_tree = server_boring_things;
    } else {
        if (!tree_insert(server_active_things, &t->tree.node)) {
            ERR("thing insert name [%s, %u] into server_active_things failed",
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
    t->is_sleeping = tp_is_sleeping(tp);

    /*
     * Start out with stats from the template.
     */
    if (stats) {
        memcpy(&t->stats, stats, sizeof(thing_stats));
    } else {
        memcpy(&t->stats, &tp->stats, sizeof(thing_stats));

        thing_stats_set_on_server(t, t->on_server);
    }

    t->stats.thing_id = id;

    /*
     * Start our with max stats.
     */
    thing_stats_set_hp(t, tp_get_stats_max_hp(tp));
    thing_stats_set_magic(t, tp_get_stats_max_magic(tp));

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

    if (thing_is_monst(t)) {
        if (t->on_server) {
            server_monst_things_total++;
        } else {
            client_monst_things_total++;
        }
    }

    if (t->on_server) {
        server_things_total++;
    } else {
        server_things_total++;
    }

    /*
     * Too boring to log about?
     */
    if (!thing_is_inactive_noverify(t)      &&
        !thing_is_cloud_effect(t)           &&
        !thing_is_weapon_swing_effect(t)    &&
        !thing_is_weapon_carry_anim(t)) {

        if (t->on_server) {
            THING_LOG(t, "created (total %d monst %d)",
                      server_things_total, server_monst_things_total);
        } else {
            THING_LOG(t, "created (total %d monst %d)",
                      client_things_total, client_monst_things_total);
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

    thing_round(t, &x, &y);
    t->x = x;
    t->y = y;

    thing_update(t);

    /*
     * So we send a move update to the client.
     */
    t->last_tx = -1;
    t->last_ty = -1;
    t->first_update = true;
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

    /*
     * We need to set the location of this thing when in combined client
     * server mode.
     */
    t->on_server = false;
    thing_stats_set_on_server(t, t->on_server);

    t->map_x = -1.0;
    t->map_y = -1.0;

    t->tree.key = id;
    thing_client_ids[id] = t;
    t->tp = tp;
    t->is_sleeping = tp_is_sleeping(tp);

    if (tp_is_player(tp)) {
        t->tree2.key = id;

        if (!tree_insert(client_player_things, &t->tree2.node)) {
            ERR("thing insert id [%u] failed into client_player_things", id);
        }

        t->on_client_player_things = true;
        t->tree2.node.is_static_mem = true;
    }

    if (tp_is_inactive(tp)) {
        if (!tree_insert(client_boring_things, &t->tree.node)) {
            ERR("thing insert id [%u] failed into client_boring_things", id);
        }

        t->client_or_server_tree = client_boring_things;
    } else {
        if (!tree_insert(client_active_things, &t->tree.node)) {
            ERR("thing insert id [%u] failed into client_active_things", id);
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

    /*
     * Too boring to log about?
     */
    if (!thing_is_inactive_noverify(t)      &&
        !thing_is_cloud_effect(t)           &&
        !thing_is_weapon_swing_effect(t)    &&
        !thing_is_weapon_carry_anim(t)) {
        THING_LOG(t, "created");
    }

    thing_stats_set_on_server(t, t->on_server);

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

    min = THING_CLIENT_ID_MIN;
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
                ERR("out of thing ids, min %u max %u!", min, max);
            }
        }
    }

    if (!id) {
        ERR("sanity check, ID 0 never used min %u max %u", min, max);
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

    if (!thing_is_dead_or_dying(t)) {
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
    thingp owner = 0;

    if (0) {
        THING_LOG(t, "remove hooks");
    }

    if (t->owner_thing_id) {
        owner = thing_owner(t);
    }

    if (t->owner_thing_id && owner) {
        if (0) {
            THING_LOG(t, "detach from owner %s", thing_logname(owner));
        }

        if (t->thing_id == owner->weapon_carry_anim_thing_id) {
            thing_unwield(owner);

            if (0) {
                THING_LOG(t, "detach from carry anim owner %s", thing_logname(owner));
            }

            thing_set_weapon_carry_anim(owner, 0);
        }

        if (t->thing_id == owner->weapon_swing_anim_thing_id) {
            if (0) {
                THING_LOG(t, "detach from swing anim owner %s", thing_logname(owner));
            }

            thing_set_weapon_swing_anim(owner, 0);

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

        if (t->thing_id == owner->shield_carry_anim_thing_id) {
            thing_unwield_shield(owner);

            if (0) {
                THING_LOG(t, "detach from carry anim owner %s", thing_logname(owner));
            }

            thing_set_shield_carry_anim(owner, 0);
        }

        if (t->thing_id == owner->magic_anim_thing_id) {
            thing_unwield_magic(owner);

            thing_set_magic_anim(owner, 0);
        }

        thing_set_owner(t, 0);
    }

    /*
     * We own things like a sword. i.e. we are a player.
     */
    if (t->weapon_carry_anim_thing_id) {
        thingp item = thing_weapon_carry_anim(t);
        thing_set_weapon_carry_anim(t, 0);
        verify(item);
        thing_set_owner(item, 0);
        thing_dead(item, 0, "weapon carry anim owner killed");
    }

    if (t->shield_carry_anim_thing_id) {
        thingp item = thing_shield_carry_anim(t);
        thing_set_shield_carry_anim(t, 0);
        verify(item);
        thing_set_owner(item, 0);
        thing_dead(item, 0, "shield carry anim owner killed");
    }

    if (t->weapon_swing_anim_thing_id) {
        thingp item = thing_weapon_swing_anim(t);
        thing_set_weapon_swing_anim(t, 0);
        verify(item);
        thing_set_owner(item, 0);
        thing_dead(item, 0, "weapon swing anim owner killed");
    }

    if (t->magic_anim_thing_id) {
        thingp item = thing_magic_anim(t);
        thing_set_magic_anim(t, 0);
        verify(item);
        thing_set_owner(item, 0);
        thing_dead(item, 0, "magic anim owner killed");
    }

    /*
     * If the inventory is being shown, we need to remove it and make a copy
     * of the contents as the thing that holds this information is about to
     * be destroyed.
     */
    if (!t->on_server) {
        if (thing_is_player(t)) {
            wid_player_info_hide(true /* fast */);
            wid_player_inventory_hide(true /* fast */);
            wid_player_action_hide(true /* fast */);
            wid_choose_stats_hide();

            memcpy(&global_config.dead_stats, &t->stats, sizeof(thing_stats));

            wid_player_action_visible(&global_config.dead_stats,
                                      true /* fast */);
        }
    }

    /*
     * Some things have lots of things they own
     */
    if (t->owned_count) {
        THING_LOG(t, "remove remaining %d owned things", t->owned_count);

        thingp i;

        { TREE_WALK(server_active_things, i) {
            thingp owner = thing_owner(i);
            if (owner && (owner == t)) {
                thing_set_owner(i, 0);
            }
        } }

        { TREE_WALK(server_boring_things, i) {
            thingp owner = thing_owner(i);
            if (owner && (owner == t)) {
                thing_set_owner(i, 0);
            }
        } }
    }
}

static void thing_destroy_implicit (thingp t)
{
    thing_destroy(t, "end of game");
}

void thing_destroy (thingp t, const char *why)
{
    verify(t);

    if (!thing_is_inactive_noverify(t)      &&
        !thing_is_cloud_effect(t)           &&
        !thing_is_weapon_swing_effect(t)    &&
        !thing_is_weapon_carry_anim(t)) {
        THING_LOG(t, "destroyed, why(%s)", why);
    }

    if (thing_is_player(t)) {
        THING_LOG(t, "destroy player");

        thing_dump(t);
    }

    /*
     * Stop all timers.
     */
    thing_timers_destroy(t);

    if (t->timers) {
        action_timers_destroy(&t->timers);
    }

    /*
     * Remove from the active or boring list.
     */
    if (!tree_remove(t->client_or_server_tree, &t->tree.node)) {
        ERR("thing template destroy name [%s] failed", thing_name(t));
    }

    t->client_or_server_tree = 0;
    t->on_active_list = false;

    if (t->on_client_player_things) {
        t->on_client_player_things = false;
        if (!tree_remove(client_player_things, &t->tree2.node)) {
            ERR("thing template destroy name [%s] failed (2)",
                thing_name(t));
        }
    }

    if (t->on_server_player_things) {
        t->on_server_player_things = false;
        if (!tree_remove(server_player_things, &t->tree2.node)) {
            ERR("thing template destroy name [%s] failed (3)",
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
    thing_unwield_shield(t);
    thing_unwield_magic(t);

    if (t->dead_reason) {
        myfree(t->dead_reason);
        t->dead_reason = 0;
    }

    if (t->wid) {
        thing_map_remove(t);
        thing_set_wid(t, 0);
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
    gsocketp s;

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

            THING_LOG(t, "player died: \"%s\"", t->stats.pname);

            char *tmp = dynprintf("%s died", t->stats.pname);
            socket_tx_server_shout_at_all_players_except(p->socket, 
                                                         CRITICAL, tmp);
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

    if (thing_is_monst(t)) {
        if (t->on_server) {
            server_monst_things_total--;
        } else {
            client_monst_things_total--;
        }
    }

    if (t->on_server) {
        server_things_total--;
    } else {
        client_things_total--;
    }

    if (t->data) {
        myfree(t->data);
    }

    if (t->logname) {
        myfree(t->logname);
        t->logname = 0;
    }

    myfree(t);
}

static void thing_dead_ (thingp t, char *reason)
{
    /*
     * Detach from the owner
     */
    thing_remove_hooks(t);

    /*
     * Why did I die!? 8(
     */
    if (reason) {
        if (t->dead_reason) {
            myfree(t->dead_reason);
            t->dead_reason = 0;
        }

        t->dead_reason = reason;
    } else {
        reason = t->dead_reason;
    }

    if (!reason) {
        ERR("thing %s dead for no reason? why? why? why?!", thing_logname(t));
    }

    /*
     * Replace the logname
     */
    char *new_logname = dynprintf("%s (dead, %s)",
                                  t->logname, reason);
    myfree(t->logname);
    t->logname = new_logname;

    if (thing_is_floor(t) || 
        thing_is_door(t) || 
        thing_is_cloud_effect(t) || 
        thing_is_wall(t)) {
        /*
         * Too boring to log.
         */
    } else {
        THING_LOG(t, "dead why(%s)", reason);
    }

    if (thing_is_player(t)) {
        /*
         * We have the gravestone now, I don't think we need this.
         *
        MSG_SERVER_SHOUT_AT_PLAYER(t, CRITICAL, "Killed by %s", reason);
         */
        aplayer *p = t->player;
        if (p) {
            char *tmp = dynprintf("%s Killed by %s", t->stats.pname, reason);
            socket_tx_server_shout_at_all_players_except(p->socket, 
                                                         CRITICAL, tmp);
            myfree(tmp);
        }

        /*
         * Tell the poor player they've croaked it.
         */
        if (t->on_server) {
            if (!t->player) {
                ERR("no player socket to send hiscores too");
            } else if (t->player->socket) {
                hiscore_try_to_add(t->stats.pname, reason, t->stats.xp);

                socket_tx_server_hiscore(t->player->socket,
                                         t->stats.pname,
                                         reason,
                                         t->stats.xp);
            }
        }
    }
}

void thing_dead (thingp t, thingp killer, const char *reason, ...)
{
    /*
     * If in a shop, this might be the shopkeeper.
     */
    thingp owner = thing_owner(t);

    /*
     * If an arrow, this might be an elf.
     */
    thingp real_killer = 0;

    if (killer) {
        real_killer = thing_owner(killer);
        if (!real_killer) {
            real_killer = killer;
        }
    }

    va_list args;

    verify(t);

    tpp tp = thing_tp(t);

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
            const char *polymorph = tp_polymorph_on_death(tp);

            if (thing_is_sawblade(t) && killer) {
                /*
                 * Skip polymorph if there is a killer. We want the blades to
                 * just vanish and not get more bloody. That only happens if
                 * there is no killer and we force a polymorph.
                 */
            } else if (polymorph) {
                tpp what = tp_find(polymorph);
                if (!what) {
                    ERR("could now find %s to polymorph into on %s death",
                        polymorph, thing_logname(t));
                }

                /*
                 * It doth polymorph.
                 */
                t->needs_tx_refresh_xy_and_template_id = 1;
                t->tp = what;
                thing_stats_set_hp(t, tp_get_stats_max_hp(what));
                thing_stats_set_magic(t, tp_get_stats_max_magic(what));
                thing_update(t);

                /*
                 * Update the name to reflect the new thing type.
                 */
                myfree(t->logname);
                t->logname = dynprintf("%s[%p, id %u] (server)",
                                       thing_short_name(t), t,
                                       t->thing_id);
                return;
            }

            /*
             * Or perhaps it does die, but spawns something else, like the
             * player dying and creating a mob spawner.
             */
            const char *spawn = tp_spawn_on_death(tp);
            if (spawn) {
                thingp newt = thing_mob_spawn_on_death(t);

                /*
                 * If this is the player death then give the gravestone a lot
                 * of health or it can be immediately killed by a lingering
                 * explosion that killed the player too.
                 */
                if (newt && thing_is_player(t)) {
                    newt->stats.hp = 200;
                }
            }
        }
    }

    /*
     * You only die once.
     */
    if (thing_is_dead(t)) {
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
            level_set_walls(server_level);
        }

        /*
         * Bounty for the killer?
         */
        uint32_t score = tp_get_bonus_xp_on_death(tp);
        if (score && real_killer) {
            /*
             * Did someone throw this weapon and gets the score?
             */
            int32_t val = tp_get_bonus_xp_on_death(tp);

            if (val) {
                thing_modify_xp(real_killer, val);

                if (thing_is_player(real_killer)) {
#if 0
                    if (thing_is_cloud_effect(killer)) {
                        /*
                         * Too many packets if we kill a lot of things in one
                         * go.
                         *
                         * But it looks nice... 8)
                         */
                    } else {
#endif
                        MSG_SERVER_SHOUT_OVER_THING(POPUP, t,
                                                    "%%%%font=%s$%%%%fg=%s$+%d",
                                                    "large", "gold",
                                                    val);
#if 0
                    }
#endif
                }
            }
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
     * Boom! If this bomb is not being collected then make it blow up.
     */
    if (t->on_server) {
#if 0
if (thing_is_treasure(t)) {
CON("%s destroyed",thing_logname(t));
if (owner) {
CON("  %s owner is keeper",thing_logname(owner));
}
if (killer) {
CON("  %s killer ",thing_logname(killer));
}
if (real_killer) {
CON("  %s real_killer ",thing_logname(real_killer));
}
}
#endif
        if (!t->is_collected) {
            if (thing_is_bomb(t)        || 
                thing_is_fireball(t)    ||
                thing_is_bullet(t)) {
                level_place_explosion(thing_level(t),
                                      0, /* owner */
                                      t->tp,
                                      t->x, t->y);
            }

            /*
             * Breaking stuff in a shop? bad idea.
             */
            if (thing_is_treasure(t)) {
                if (owner && thing_is_shopkeeper(owner)) {
                    if (thing_is_player(real_killer)) {
                        shop_break_message(real_killer, owner);
                    } else {
                        shop_whodunnit_break_message(real_killer, owner);
                    }
                }
            }
        } else {
            /*
             * Collecting a thing?
             */
            if (thing_is_treasure(t)) {
                if (owner && thing_is_shopkeeper(owner)) {
                    if (thing_is_player(real_killer)) {
                        shop_collect_message(real_killer, t);
                    }
                }
            }
        }
    }

    /*
     * Stop bouncing or swaying.
     */
    if (t->wid) {
        if (tp_is_effect_pulse(tp)) {
            wid_scaling_to_pct_in(t->wid, 1.0, 1.0, 0, 0);
        }

        if (tp_is_effect_sway(tp)) {
            wid_rotate_to_pct_in(t->wid, 0, 0, 0, 0);
        }
    }

    /*
     * Log the means of death!
     */
    if (reason) {
        va_start(args, reason);
        thing_dead_(t, dynvprintf(reason, args));
        va_end(args);
    } else {
        thing_dead_(t, 0);
    }

    /*
     * Move the thing from the boring list to the active list and update it so
     * that it gets sent to the client.
     */
    if (t->on_server) {
        thing_update(t);

        /*
         * Send the players an update of their status so the client gets the
         * final score.
         */
        if (thing_is_player(t)) {
            socket_tx_server_status(t->player->socket);
        }
    }

    thing_make_active(t);
}

/*
 * Things live on a boring list or an active list. Boring things like walls
 * don't do much and can be ignored. Sometimes though, boring things need to
 * spring into life and move. This routine moves them to the active list.
 */
void thing_make_active (thingp t)
{
    verify(t);

    if (t->on_active_list) {
        return;
    }

    if (!t->client_or_server_tree) {
        return;
    }

    if (!tree_remove(t->client_or_server_tree, &t->tree.node)) {
        ERR("thing move, remove boring list [%s] failed", thing_name(t));
    }

    if (t->client_or_server_tree == server_boring_things) {
        if (!tree_insert(server_active_things, &t->tree.node)) {
            ERR("thing move, insert active list [%s] failed", thing_name(t));
        }

        t->client_or_server_tree = server_active_things;
    } else if (t->client_or_server_tree == client_boring_things) {
        if (!tree_insert(client_active_things, &t->tree.node)) {
            ERR("thing move, insert active list [%s] failed", thing_name(t));
        }

        t->client_or_server_tree = client_active_things;
    } else {
        ERR("bug, not on client or server list");
    }

    t->on_active_list = true;
}

void thing_wake (thingp t)
{
    verify(t);

    if (!t->is_sleeping) {
        return;
    }

    THING_LOG(t, "wake");

    thing_set_is_sleeping(t, false);
}

static void thing_dying_ (thingp t, thingp killer, char *reason)
{
    /*
     * Why did I die!? 8(
     */
    if (reason) {
        if (t->dead_reason) {
            myfree(t->dead_reason);
            t->dead_reason = 0;
        }

        t->dead_reason = reason;
    }

    if (thing_is_player(t)) {
        THING_LOG(t, "dying (%s)", reason);
    }
}

void thing_dying (thingp t, thingp killer, const char *reason, ...)
{
    va_list args;

    verify(t);

    /*
     * Post death server events.
     */
    if (t->on_server) {
        /*
         * If this is a dead player, then rethink AI targets.
         */
        if (thing_is_player(t)) {
            level_set_walls(server_level);
        }
    } else {
        /*
         * Move the weapon behind the poor thing.
         */
        thing_set_weapon_placement(t);
    }

    /*
     * Log the means of death!
     */
    if (reason) {
        va_start(args, reason);
        thing_dying_(t, killer, dynvprintf(reason, args));
        va_end(args);
    } else {
        thing_dying_(t, killer, 0);
    }
}

static int thing_hit_ (thingp t, 
                       thingp orig_hitter, 
                       thingp real_hitter, 
                       thingp hitter, 
                       int32_t damage)
{
    int32_t orig_damage = damage;

    verify(t);

    /*
     * Cruel to let things keep on hitting you when you're dead
     */
    if (thing_is_dead_or_dying(t)) {
        return (false);
    }

    if (!damage) {
        /*
         * Could be a spider silkball
         */
        return (false);
    }

    t->timestamp_last_attacked = time_get_time_ms();

    /*
     * Take note of the hit so we can send an event to the client.
     *
     * Send this for both players and monsters so we see monsters being 
     * pumelled!
     */
    thing_server_effect(t, THING_STATE_EFFECT_IS_HIT_SUCCESS);

    if (damage > thing_stats_get_hp(t) / 10) {
        thing_server_effect(t, THING_STATE_EFFECT_IS_HIT_CRIT);
    }

    damage = thing_stats_get_total_damage_minus_defense(t, hitter, damage);

    /*
     * Keep hitting until all damage is used up or the thing is dead.
     */
    THING_LOG(t, "hit, hp %d, damage %d", thing_stats_get_hp(t), damage);

    /*
     * Clash of swords?
     */
    const char *sound = tp_sound_hitting(orig_hitter->tp);
    if (sound) {
        if (thing_is_player(real_hitter)) {
            MSG_SERVER_SHOUT_AT_PLAYER(SOUND, real_hitter, "%s", sound);
        }
    }

    while (damage > 0) {
        if (thing_stats_get_hp(t) <= damage) {
            damage -= thing_stats_get_hp(t);

            /*
             * Start the death countdown
             */
            thing_stats_set_hp(t, 0);

            THING_LOG(t, "teriminal hit, hp %d, damage %d", 
                      thing_stats_get_hp(t), damage);

            /*
             * Record who dun it.
             */
            if (thing_is_player(t)) {
                thing_dying(t, orig_hitter, "%s", tp_short_name(real_hitter->tp));
            } else {
                thing_dead(t, orig_hitter, "%s", tp_short_name(real_hitter->tp));
            }

            /*
             * Explodes on death ala Sith Lord? Only a lesser one, mind.
             */
            if (tp_is_combustable(t->tp)) {
                level_place_explosion(thing_level(t),
                                      0, // owner
                                      t->tp,
                                      t->x, t->y);
            }

            if (0) {
                THING_LOG(t, "hit by (%s) for %u, now dead",
                          thing_logname(orig_hitter), damage);
            }

            /*
             * If polymorphed, hit again?
             */
            if (!thing_stats_get_hp(t)) {
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
            thing_stats_modify_hp(t, -damage);

            if (thing_stats_get_hp(t) < 0) {
                thing_stats_set_hp(t, 0);
            }

            THING_LOG(t, "hit by (%s) for %u, hp now %d",
                      thing_logname(orig_hitter), damage,
                      t->stats.hp);

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
    } else {
        if (orig_damage > 0) {
            const char *color = "white";
            const char *font = "small";

            if (orig_damage > 20) {
                font = "vlarge";
            } else if (orig_damage > 10) {
                font = "large";
            } else if (orig_damage > 5) {
                font = "medium";
            } else if (orig_damage > 2) {
                font = "small";
            } else {
                font = "vsmall";
            }

            if (thing_is_player(t)) {
                color = "red";
            }

            MSG_SERVER_SHOUT_OVER_THING(POPUP, t,
                                        "%%%%font=%s$%%%%fg=%s$-%d",
                                        font, color, orig_damage);
        }
    }

    return (true);
}

int thing_hit (thingp t, thingp hitter, uint32_t damage)
{
    thingp orig_hitter = hitter;

    /*
     * If an arrow, this might be an elf.
     */
    thingp real_hitter = 0;

    if (hitter) {
        real_hitter = thing_owner(hitter);
        if (!real_hitter) {
            real_hitter = hitter;
        }
    }

    tpp weapon = 0;

#if 0
    if (hitter && thing_owner(hitter)) {
        THING_CON(t, "is being hit by %s, owner %s",
                  thing_logname(hitter), 
                  thing_logname(thing_owner(hitter)));
    } else {
        THING_CON(t, "is being hit by %s",
                  thing_logname(hitter));
    }
#endif

    verify(t);
    if (hitter) {
        verify(hitter);
    }

    /*
     * Cruel to let things keep on hitting you when you're dead
     */
    if (thing_is_dead_or_dying(t)) {
        return (false);
    }

    if (thing_is_dead_or_dying(hitter)) {
        /*
         * This case is hit if a ghost runs into a player. The ghost takes
         * damage. We don't want the player to keep absorbing hits when
         * already dead though.
         */
        return (false);
    }

    /*
     * Sanity check.
     */
    if (!t->on_server) {
        ERR("hits can only happen on the server");
        return (false);
    }

    if (hitter) {
        if (!hitter->on_server) {
            ERR("hits can only happen from hitter on the server");
            return (false);
        }
    }

    /*
     * If the player has a shield, let the shield take the hit.
     */
    if (t->shield) {
        thing_server_effect(t, THING_STATE_EFFECT_IS_HIT_MISS);
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
                /*
                 * Not something that typically damages walls.
                 */
                return (false);
            }
        }

        if (thing_is_weapon_swing_effect(hitter)) {
            if (!hitter->owner_thing_id) {
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
            weapon = thing_weapon(hitter);
            if (!weapon) {
                return (false);
            }

            if (!damage) {
                damage = tp_get_damage(weapon);
            }

        } else if (hitter->owner_thing_id) {
            /*
             * Get the player firing the weapon as the hitter.
             */
            hitter = thing_owner(hitter);
            if (!hitter) {
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

        if (orig_hitter &&
            tp_to_id(orig_hitter->tp) == THING_SILKBALL) {
            /*
             * No inherited spider damage from silk ball.
             */
        } else {
            /*
             * Take the damage from the weapon that hits first.
             */
            if (!damage) {
                if (orig_hitter) {
                    damage = thing_stats_get_total_damage(orig_hitter);
                }
            }

            /*
             * If still no damage, use the thing that did the hitting.
             */
            if (!damage) {
                if (hitter) {
                    damage = thing_stats_get_total_damage(hitter);
                }
            }
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
            if (!time_have_x_tenths_passed_since(
                    delay, orig_hitter->timestamp_i_attacked)) {

                return (false);
            }

            orig_hitter->timestamp_i_attacked = time_get_time_ms();
        }
    }

    /*
     * Flash briefly red on attempted hits.
     */
    if (thing_is_monst(t)               ||
        thing_is_mob_spawner(t)         ||
        thing_is_wall(t)                ||
        thing_is_powerup(t)             ||
        thing_is_sawblade(t)            ||
        thing_is_door(t)) {

        /*
         * Assume missed due to the logic below where we detect chance.
         */
        if (orig_hitter && thing_is_cloud_effect(orig_hitter)) {
            /*
             * No flashing
             */
        } else {
            thing_server_effect(t, THING_STATE_EFFECT_IS_HIT_MISS);
        }
    }

    /*
     * Check if the weapon reaches its end of warranty.
     *
     * I think this should be before the miss chance so if we are hitting at
     * rock and keep missing then we damage the weapon.
     */
    if (weapon) {
        thing_weapon_check_for_wear_damage(t, hitter, weapon);
    }

    /*
     * Does the thing get off being hit.
     */
    uint32_t can_be_hit_chance = tp_get_can_be_hit_chance(t->tp);
    if (can_be_hit_chance) {
        uint32_t chance = myrand() % (can_be_hit_chance + 1);

        if (chance > damage) {
            return (false);
        }
    }

    int r;

    r = thing_hit_(t, orig_hitter, real_hitter, hitter, damage);

    /*
     * Do we need to kill the original hitter?
     */
    if (orig_hitter) {
        if (thing_is_fragile(orig_hitter)) {
            /*
             * Sawblades get more covered in blood each time they kill
             * something that is warm blooded. But we don't want that to
             * happen for damage from say a bomb. However if the damage is
             * really high then we just stop the blade.
             */
            if (thing_is_sawblade(orig_hitter)) {
                if (thing_is_warm_blooded(t)) {
                    /*
                     * Move to the next most bloody blade
                     */
                    thing_dead(orig_hitter, 0, "blood splatter");
                } else {
                    /*
                     * Keep on spinning those blades.
                     */
                }
            } else {
                thing_dead(orig_hitter, 0, "self destruct on hitting");
            }
        }
    }

    return (r);
}

thingp thing_owner (thingp t)
{
    verify(t);

    if (t->on_server) {
        if (t->owner_thing_id) {
            thingp n = thing_server_ids[t->owner_thing_id];
            if (!n) {
                /*
                 * for things like missiles, it is accepted that the owner may
                 * be dead but the missile keeps on going.
                 */
                if (thing_is_projectile(t)) {
                    t->owner_thing_id = 0;

                    THING_LOG(t, "no server owner thing found for owner id %u for %s",
                              t->owner_thing_id, thing_logname(t));
                    return (0);
                } else {
                    ERR("no server owner thing found for owner id %u for %s",
                        t->owner_thing_id, thing_logname(t));
                }
            }

            verify(n);
            return (n);
        }
    } else {
        if (t->owner_thing_id) {
            thingp n = thing_client_ids[t->owner_thing_id];
            if (!n) {
                ERR("no client owner thing found for owner id %u for %s",
                    t->owner_thing_id, thing_logname(t));
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

    /*
     * Hide the shield too or it just floats in the air.
     */
    thingp shield_carry_anim = thing_shield_carry_anim(t);
    if (shield_carry_anim) {
        thing_hide(shield_carry_anim);
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

    /*
     * Reveal the shield again too.
     */
    thingp shield_carry_anim = thing_shield_carry_anim(t);
    if (shield_carry_anim) {
        thing_visible(shield_carry_anim);
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

    level_set_exit_has_been_reached(server_level, true);

    /*
     * Make the weapon leave too
     */
    thingp weapon_carry_anim = thing_weapon_carry_anim(t);
    if (weapon_carry_anim) {
        thing_leave_level(weapon_carry_anim);
    }

    thingp shield_carry_anim = thing_shield_carry_anim(t);
    if (shield_carry_anim) {
        thing_leave_level(shield_carry_anim);
    }

    thingp magic_anim = thing_magic_anim(t);
    if (magic_anim) {
        thing_leave_level(magic_anim);
    }

    /*
     * Still in a shop? Sneaky.
     */
    t->money_owed = 0;
    t->in_shop_owned_by_thing_id = 0;
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
    t->needs_tx_refresh_xy_and_template_id = 1;

    /*
     * Make the weapon leave to
     */
    thingp weapon_carry_anim = thing_weapon_carry_anim(t);
    if (weapon_carry_anim) {
        thing_join_level(weapon_carry_anim);
    }

    thingp shield_carry_anim = thing_shield_carry_anim(t);
    if (shield_carry_anim) {
        thing_join_level(shield_carry_anim);
    }

    thingp magic_anim = thing_magic_anim(t);
    if (magic_anim) {
        thing_join_level(magic_anim);
    }
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
                    !thing_is_weapon_carry_anim(t) &&
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

                ERR("thing still exists %s", thing_logname(t));
            }
        }

        {
            TREE_WALK(server_boring_things, t) {
                ERR("thing still exists %s", thing_logname(t));
            }
        }
    }

    if (level == client_level) {
        {
            TREE_WALK(client_active_things, t) {
                if (thing_is_player(t)) {
                    THING_LOG(t, "cleaning up player for end of level");
                    thing_sheath(t);
                    thing_shield_sheath(t);
                    thing_magic_sheath(t);
                }

                if (keep_players &&
                    thing_is_player_or_owned_by_player(t) &&
                    !thing_is_animation(t) &&
                    !thing_is_shield_carry_anim(t) &&
                    !thing_is_weapon_carry_anim(t) &&
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

                ERR("thing still exists %s", thing_logname(t));
            }
        }

        {
            TREE_WALK(client_boring_things, t) {
                ERR("thing still exists %s", thing_logname(t));
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

void thing_set_is_sleeping (thingp t, uint8_t val)
{
    verify(t);

    t->is_sleeping = val;
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

void thing_set_dir_down (thingp t)
{
    verify(t);

    if (thing_is_animated_no_dir(t)) {
        return;
    }

    if (t->dir != THING_DIR_DOWN) {
        t->dir = THING_DIR_DOWN;
        if (t->on_server) {
            thing_update(t);
        } else {
            thing_set_weapon_placement(t);
        }
    }
}

uint8_t thing_is_dir_down (thingp t)
{
    verify(t);

    return (t->dir == THING_DIR_DOWN);
}

void thing_set_dir_up (thingp t)
{
    verify(t);

    if (thing_is_animated_no_dir(t)) {
        return;
    }

    if (t->dir != THING_DIR_UP) {
        t->dir = THING_DIR_UP;
        if (t->on_server) {
            thing_update(t);
        } else {
            thing_set_weapon_placement(t);
        }
    }
}

uint8_t thing_is_dir_up (thingp t)
{
    verify(t);

    return (t->dir == THING_DIR_UP);
}

void thing_set_dir_left (thingp t)
{
    verify(t);

    if (thing_is_animated_no_dir(t)) {
        return;
    }

    if (t->dir != THING_DIR_LEFT) {
        t->dir = THING_DIR_LEFT;
        if (t->on_server) {
            thing_update(t);
        } else {
            thing_set_weapon_placement(t);
        }
    }
}

uint8_t thing_is_dir_left (thingp t)
{
    verify(t);

    return (t->dir == THING_DIR_LEFT);
}

void thing_set_dir_right (thingp t)
{
    verify(t);

    if (thing_is_animated_no_dir(t)) {
        return;
    }

    if (t->dir != THING_DIR_RIGHT) {
        t->dir = THING_DIR_RIGHT;
        if (t->on_server) {
            thing_update(t);
        } else {
            thing_set_weapon_placement(t);
        }
    }
}

uint8_t thing_is_dir_right (thingp t)
{
    verify(t);

    return (t->dir == THING_DIR_RIGHT);
}

void thing_set_dir_tl (thingp t)
{
    verify(t);

    if (thing_is_animated_no_dir(t)) {
        return;
    }

    if (t->dir != THING_DIR_TL) {
        t->dir = THING_DIR_TL;
        if (t->on_server) {
            thing_update(t);
        } else {
            thing_set_weapon_placement(t);
        }
    }
}

uint8_t thing_is_dir_tl (thingp t)
{
    verify(t);

    return (t->dir == THING_DIR_TL);
}

void thing_set_dir_bl (thingp t)
{
    verify(t);

    if (thing_is_animated_no_dir(t)) {
        return;
    }

    if (t->dir != THING_DIR_BL) {
        t->dir = THING_DIR_BL;
        if (t->on_server) {
            thing_update(t);
        } else {
            thing_set_weapon_placement(t);
        }
    }
}

uint8_t thing_is_dir_bl (thingp t)
{
    verify(t);

    return (t->dir == THING_DIR_BL);
}

void thing_set_dir_tr (thingp t)
{
    verify(t);

    if (thing_is_animated_no_dir(t)) {
        return;
    }

    if (t->dir != THING_DIR_TR) {
        t->dir = THING_DIR_TR;
        if (t->on_server) {
            thing_update(t);
        } else {
            thing_set_weapon_placement(t);
        }
    }
}

uint8_t thing_is_dir_tr (thingp t)
{
    verify(t);

    return (t->dir == THING_DIR_TR);
}

void thing_set_dir_br (thingp t)
{
    verify(t);

    if (thing_is_animated_no_dir(t)) {
        return;
    }

    if (t->dir != THING_DIR_BR) {
        t->dir = THING_DIR_BR;
        if (t->on_server) {
            thing_update(t);
        } else {
            thing_set_weapon_placement(t);
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

    t->timestamp_change_to_next_frame = time_get_time_ms();

    t->is_open = val;
}

void thing_set_is_angry (thingp t, uint8_t val)
{
    verify(t);

    t->timestamp_change_to_next_frame = time_get_time_ms();

    t->is_angry = val;
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

    if (t->is_jumping) {
        return (((double)tp_get_jump_speed(t->tp)));
    }

    return (((double)thing_stats_get_total_speed(t)));
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
void thing_place_and_destroy_timed (levelp level,
                                    tpp tp,
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
    } else if (server_level && (level == server_level)) {
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
    } else if (client_level && (level == client_level)) {
        action_timer_create(
                &client_timers,
                (action_timer_callback)
                    thing_timer_place_and_destroy_callback,
                (action_timer_callback)
                    thing_timer_place_and_destroy_destroy_callback,
                context,
                "place and destroy thing",
                ms,
                jitter);
    } else {
        ERR("don't know where to place explosion");
    }
}

void thing_teleport (thingp t, int32_t x, int32_t y)
{
    if (time_get_time_ms() - t->timestamp_teleport < 500) {
        return;
    }

    t->timestamp_teleport = time_get_time_ms();

    widp wid_next_floor = wid_grid_find_thing_template(
                                    wid_game_map_server_grid_container,
                                    x,
                                    y,
                                    tp_is_floor);
    if (!wid_next_floor) {
        ERR("no floor tile to hpp to");
    }

    double next_floor_x, next_floor_y;

    wid_get_mxy(wid_next_floor, &next_floor_x, &next_floor_y);

    widp w = thing_wid(t);

    wid_move_end(w);
    wid_move_to_abs_centered_in(w, next_floor_x, next_floor_y, 0);

    sound_play_level_end();
}

void thing_move (thingp t, double x, double y)
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

void socket_server_tx_map_update (gsocketp p, tree_rootp tree, const char *type)
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
        gsocketp sp;

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
    UDPpacket *packet = packet_alloc();
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
        if (tp_is_cloud_effect(tp)) {
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

        /*
         * Don't sent local effects to the client to save bandwidth. The
         * client can infer the swing and animations.
         */
        if (tp_is_weapon_swing_effect(tp)   ||
            tp_is_powerup_anim(tp)          ||
            tp_is_shield_carry_anim(tp)     ||
            tp_is_weapon_carry_anim(tp)) {
            t->updated--;
            continue;
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
            } else if (thing_is_dead(t)) {
                /*
                 * Send dead changes all the time as we use that on level end
                 */
            } else if (!time_have_x_thousandths_passed_since(
                    tp_get_tx_map_update_delay_thousandths(tp),
                    t->timestamp_tx_map_update)) {
                continue;
            }

            t->timestamp_tx_map_update = time_get_time_ms();

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
            double rx = t->x;
            double ry = t->y;
            thing_round(t, &rx, &ry);
            tx = (uint8_t)(int)((rx * ((double)256)) / MAP_WIDTH);
            ty = (uint8_t)(int)((ry * ((double)256)) / MAP_HEIGHT);
        } else {
            tx = 0xFF;
            ty = 0xFF;
        }

        uint8_t state = t->dir;

        /*
         * WARING: Keep the above and below in sync.
         */
        uint8_t ext1 =
            ((t->is_dead                              ? 1 : 0) <<
                THING_STATE_BIT_SHIFT_EXT1_IS_DEAD)             |
            ((t->on_active_list                       ? 1 : 0) <<
                THING_STATE_BIT_SHIFT_EXT1_IS_ACTIVE)           |
            ((t->is_sleeping                          ? 1 : 0) <<
                THING_STATE_BIT_SHIFT_EXT1_IS_SLEEPING)         |
            ((t->has_left_level                       ? 1 : 0) <<
                THING_STATE_BIT_SHIFT_EXT1_HAS_LEFT_LEVEL)      |
            ((t->weapon                               ? 1 : 0) <<
                THING_STATE_BIT_SHIFT_EXT1_WEAPON_ID_PRESENT)   |
            ((t->shield                               ? 1 : 0) <<
                THING_STATE_BIT_SHIFT_EXT1_SHIELD_ID_PRESENT)   |
            ((t->effect         ? 1 : 0) <<
                THING_STATE_BIT_SHIFT_EXT1_EFFECT_PRESENT);

        uint8_t ext2 =
            ((t->torch_light_radius_set               ? 1 : 0) <<
                THING_STATE_BIT_SHIFT_EXT2_TORCH_LIGHT_RADIUS)  |
            ((t->is_jumping                           ? 1 : 0) <<
                THING_STATE_BIT_SHIFT_EXT2_IS_JUMPING)          |
            (((t->data && t->data->col_name)          ? 1 : 0) <<
                THING_STATE_BIT_SHIFT_EXT2_COLOR)               |
            (((t->scale)                              ? 1 : 0) <<
                THING_STATE_BIT_SHIFT_EXT2_SCALE)               |
            ((t->magic_anim                           ? 1 : 0) <<
                THING_STATE_BIT_SHIFT_EXT2_MAGIC_ID_PRESENT)    |
            ((t->needs_tx_refresh_xy_and_template_id  ? 1 : 0) <<
                THING_STATE_BIT_SHIFT_EXT2_RESYNC);

        /*
         * Once the jump is sent to the client, turn it off.
         */
        t->is_jumping = false;

        if (t->needs_tx_weapon_swung) {
            t->needs_tx_weapon_swung = false;
            ext1 |= 1 << THING_STATE_BIT_SHIFT_EXT1_WEAPON_SWUNG;
        }

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

        if (t->needs_tx_refresh_xy_and_template_id || t->is_dead) {
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

        if (ext1) {
            state |= 1 << THING_STATE_BIT_SHIFT_EXT1_PRESENT;
        }

        if (ext2) {
            state |= 1 << THING_STATE_BIT_SHIFT_EXT2_PRESENT;
        }

        /*
         * Write the data.
         */
        *data++ = state;

        if (state & (1 << THING_STATE_BIT_SHIFT_ID_DELTA_PRESENT)) {
            *data++ = id - last_id;
//CON("T id       0x%02x", *(data-1));
        } else {
            SDLNet_Write16(id, data);
            data += sizeof(uint16_t);
//CON("T id       0x%04x", id);
        }

        if (state & (1 << THING_STATE_BIT_SHIFT_ID_TEMPLATE_PRESENT)) {
            *data++ = template_id;
//CON("  template 0x%02x", template_id);
        }

        if (ext1) {
            *data++ = ext1;
//CON("  ext1     0x%02x", ext1);
        }

        if (ext2) {
            *data++ = ext2;
//CON("  ext2     0x%02x", ext2);
        }

        if (state & (1 << THING_STATE_BIT_SHIFT_XY_PRESENT)) {
            *data++ = tx;
//CON("  tx       0x%02x", tx);
            *data++ = ty;
//CON("  ty       0x%02x", ty);
        }

        if (ext1 & (1 << THING_STATE_BIT_SHIFT_EXT1_WEAPON_ID_PRESENT)) {
            *data++ = tp_to_id(t->weapon);
//CON("  weapon   %02x", tp_to_id(t->weapon));
        }

        if (ext1 & (1 << THING_STATE_BIT_SHIFT_EXT1_SHIELD_ID_PRESENT)) {
            *data++ = tp_to_id(t->shield);
//CON("  owner    0x%04x", t->owner_thing_id);
        }

        if (ext2 & (1 << THING_STATE_BIT_SHIFT_EXT2_MAGIC_ID_PRESENT)) {
            *data++ = tp_to_id(t->magic_anim);
//CON("  owner    0x%04x", t->owner_thing_id);
        }

        if (ext2 & (1 << THING_STATE_BIT_SHIFT_EXT2_TORCH_LIGHT_RADIUS)) {
            *data++ = (uint8_t) ((int) (t->torch_light_radius * 4.0));
//CON("tx  torch    %f -> %d",t->torch_light_radius, *(data - 1));
        }

        if (ext2 & (1 << THING_STATE_BIT_SHIFT_EXT2_COLOR)) {
            *data++ = t->data->col.r;
            *data++ = t->data->col.g;
            *data++ = t->data->col.b;
//CON("tx  color    %d,%d,%d -> %d",t->data->col.r, t->data->col.g, t->data->col.b, *(data - 1));
        }

        if (ext2 & (1 << THING_STATE_BIT_SHIFT_EXT2_SCALE)) {
            *data++ = (uint8_t) (int) (((double)t->scale) * 32.0);
        }

        if (ext1 & (1 << THING_STATE_BIT_SHIFT_EXT1_EFFECT_PRESENT)) {
            *data++ = t->effect;
//CON("tx  effetc   %d", t->effect);
            t->effect = 0;
        }

        t->last_tx = tx;
        t->last_ty = ty;
        t->needs_tx_refresh_xy_and_template_id = 0;
        t->first_update = false;

//THING_CON(t, "tx");
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
        gsocketp sp;

        if (packet->len > PACKET_LEN_COMPRESS_THRESHOLD) {
            packet_compress(packet);
        }

        TREE_WALK_UNSAFE(sockets, sp) {
            if (p && (p != sp)) {
                continue;
            }

            if (!sp->player) {
                continue;
            }

            if (!sp->server_side_client) {
                continue;
            }

            write_address(packet, socket_get_remote_ip(sp));

            UDPpacket *dup = packet_dup(packet);

            socket_tx_enqueue(sp, &dup);
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
    if (data - odata > 1) {
        gsocketp sp;

        packet->len = data - odata;

        if (packet->len > PACKET_LEN_COMPRESS_THRESHOLD) {
            packet_compress(packet);
        }

        TREE_WALK_UNSAFE(sockets, sp) {
            if (!sp->player) {
                continue;
            }

            if (!sp->server_side_client) {
                continue;
            }

            write_address(packet, socket_get_remote_ip(sp));

            UDPpacket *dup = packet_dup(packet);

            socket_tx_enqueue(sp, &dup);
        }
    }

    packet_free(packet);
}

void socket_client_rx_map_update (gsocketp s, UDPpacket *packet, uint8_t *data)
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

    uint8_t *eodata = data + packet->len - 1;
    uint16_t last_id = 0;

//CON("rx map update:");
//hex_dump_log(data, 0, packet->len);
    while (data < eodata) {
//CON("rx map element:");
//hex_dump_log(data, 0, 10);
        uint8_t state = *data++;
        uint8_t ext1;
        uint8_t ext2;
        uint8_t template_id;
        uint8_t weapon_id;
        uint8_t shield_id;
        uint8_t magic_id;
        uint8_t weapon_swung;
        uint8_t torch_light_radius_present;
        uint8_t effect;
        float torch_light_radius;
        uint16_t id;
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
//CON("R id       0x%02x",id);
        } else {
            /*
             * Full ID update.
             */
            id = SDLNet_Read16(data);
            data += sizeof(uint16_t);
//CON("R id       0x%04x", id);
        }
        last_id = id;

        if (state & (1 << THING_STATE_BIT_SHIFT_ID_TEMPLATE_PRESENT)) {
            /*
             * Full template ID update.
             */
            template_id = *data++;
//CON("  template 0x%02x", template_id);
        } else {
            template_id = -1;
        }

        if (state & (1 << THING_STATE_BIT_SHIFT_EXT1_PRESENT)) {
            /*
             * Extensions present.
             */
            ext1 = *data++;
//CON("  ext1     0x%02x", ext1);
        } else {
            ext1 = 0;
        }

        if (state & (1 << THING_STATE_BIT_SHIFT_EXT2_PRESENT)) {
            /*
             * Extensions present.
             */
            ext2 = *data++;
//CON("  ext2     0x%02x", ext2);
        } else {
            ext2 = 0;
        }

        if (state & (1 << THING_STATE_BIT_SHIFT_XY_PRESENT)) {
            /*
             * Full move update.
             */
            tx = *data++;
            ty = *data++;
//CON("  tx       0x%02x", tx);
//CON("  ty       0x%02x", ty);

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

        if (ext1 & (1 << THING_STATE_BIT_SHIFT_EXT1_WEAPON_ID_PRESENT)) {
            weapon_id = *data++;
            if (!weapon_id) {
                ERR("THING_STATE_BIT_SHIFT_EXT1_WEAPON_ID_PRESENT set but no weapon");
            }
//CON("  weapon   0x%02x",weapon_id);
        } else {
            weapon_id = 0;
        }

        if (ext1 & (1 << THING_STATE_BIT_SHIFT_EXT1_SHIELD_ID_PRESENT)) {
            shield_id = *data++;
//CON("  owner    0x%04x", owner_id);
            if (!shield_id) {
                ERR("THING_STATE_BIT_SHIFT_EXT1_SHIELD_ID_PRESENT set but no owner");
            }
        } else {
            shield_id = 0;
        }

        if (ext2 & (1 << THING_STATE_BIT_SHIFT_EXT2_MAGIC_ID_PRESENT)) {
            magic_id = *data++;
//CON("  owner    0x%04x", owner_id);
            if (!magic_id) {
                ERR("THING_STATE_BIT_SHIFT_EXT2_MAGIC_ID_PRESENT set but no owner");
            }
        } else {
            magic_id = 0;
        }

        if (ext2 & (1 << THING_STATE_BIT_SHIFT_EXT2_TORCH_LIGHT_RADIUS)) {
            torch_light_radius = ((float) (*data++)) / 4.0;
            torch_light_radius_present = true;
//CON("  torch light %d -> %f", *(data - 1), torch_light_radius);
        } else {
            torch_light_radius_present = false;
        }

        color c;
        if (ext2 & (1 << THING_STATE_BIT_SHIFT_EXT2_COLOR)) {
            c.r = *data++;
            c.g = *data++;
            c.b = *data++;
        }

        double scale = 1.0;
        if (ext2 & (1 << THING_STATE_BIT_SHIFT_EXT2_SCALE)) {
            scale = ((double)((int)*data++)) / 32.0;
        }

        if (ext1 & (1 << THING_STATE_BIT_SHIFT_EXT1_EFFECT_PRESENT)) {
            effect = *data++;
        } else {
            effect = 0;
        }

//CON("rx id %d",id);
        t = thing_client_find(id);
        if (ext1 & (1 << THING_STATE_BIT_SHIFT_EXT1_WEAPON_SWUNG)) {
            weapon_swung = true;
//CON("  weapon swung");
        } else {
            weapon_swung = false;
        }

        tpp tp = 0;

        if (!t) {
            if (template_id == (uint8_t)-1) {
                /*
                 * It's okay if it is dead, we can ignore this as the server
                 * is just making sure we got the dead hint.
                 */
                if (ext1 & (1 << THING_STATE_BIT_SHIFT_EXT1_IS_DEAD)) {
                    LOG("Client: received DEAD unknown thing %u", id);
                    continue;
                }

                /*
                 * This could happen due to packet loss and we have no way
                 * to rebuild the thing without a resend. Need a way to ask
                 * for a resync.
                 */
                ERR("Client: received unknown thing %u", id);
                continue;
            }

            tp = id_to_tp(template_id);

            t = thing_client_new(id, tp);

//CON("rx id %d create thing",id);
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
                tp = id_to_tp(template_id);

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

                if (ext1 & (1 << THING_STATE_BIT_SHIFT_EXT1_HAS_LEFT_LEVEL)) {
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

        /*
         * Wield weapons before calling thing_client_wid_update which will
         * also move the weapons.
         */
        if (weapon_id) {
            thing_wield(t, id_to_tp(weapon_id));
        } else if (t->weapon) {
            thing_unwield(t);
        }

        if (shield_id) {
            thing_wield_shield(t, id_to_tp(shield_id));
        } else if (t->shield) {
            thing_unwield_shield(t);
        }

        if (magic_id) {
            thing_wield_magic(t, id_to_tp(magic_id));
        } else if (t->magic_anim) {
            thing_unwield_magic(t);
        }

        if (weapon_swung) {
            thing_swing(t);

            if (!(state & (1 << THING_STATE_BIT_SHIFT_XY_PRESENT))) {
                thing_client_wid_update(t, t->x, t->y, false /* smooth */,
                                        false /* is new */);
            }
        }

        /*
         * Move the thing?
         */
        if (state & (1 << THING_STATE_BIT_SHIFT_XY_PRESENT)) {
            widp w = thing_wid(t);
            if (w) {
                if (t == player) {
//THING_CON(t, "rx %f %f",x,y);
                    /*
                     * Local echo only.
                     */
                    if ((ext2 & (1 << THING_STATE_BIT_SHIFT_EXT2_RESYNC))) {
                        /*
                         * Check we are roughly where the server thinks we
                         * are. If wildly out of whack, correct our viewpoint.
                         */
                        THING_LOG(t, "%s server asked for resync", t->logname);
                        THING_LOG(t, "  server %f %f", t->x, t->y);
                        THING_LOG(t, "  client %f %f", x, y);

                        thing_client_wid_update(t, x, y, false /* smooth */,
                                                false /* is new */);

                        wid_game_map_client_scroll_adjust(1);
                    } else if ((fabs(x-t->x) > THING_MAX_SERVER_DISCREPANCY) ||
                               (fabs(y-t->y) > THING_MAX_SERVER_DISCREPANCY)) {
                        /*
                         * Check we are roughly where the server thinks we
                         * are. If wildly out of whack, correct our viewpoint.
                         */
                        THING_LOG(t, "%s out of sync with server, correcting ",
                                    t->logname);
                        THING_LOG(t, "  server %f %f", t->x, t->y);
                        THING_LOG(t, "  client %f %f", x, y);

                        thing_client_wid_update(t, x, y, false /* smooth */,
                                                false /* is new */);

                        wid_game_map_client_scroll_adjust(1);
                    }
                } else if (on_map) {
                    /*
                     * Move something which is not the local player. Could
                     * be another player or monster etc...
                     */
                    if (ext2 & (1 << THING_STATE_BIT_SHIFT_EXT2_IS_JUMPING)) {
                        t->is_jumping = true;
#if 0
if (t->is_jumping) {
    CON("%s jump to %f,%f",thing_logname(t),x,y);
} else {
    CON("%s move to %f,%f",thing_logname(t),x,y);
}
#endif
                    }

                    thing_client_wid_update(t, x, y, true /* smooth */,
                                            false /* is new */);

                    if (!need_fixup &&
                        (thing_is_wall(t) ||
                         thing_is_pipe(t) ||
                         thing_is_door(t))) {
                        need_fixup = true;
                    }
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
                    if (thing_is_cloud_effect(t)) {
                        t->is_epicenter = true;
                    }

                    /*
                     * Thing has no wid. Make one.
                     */
                    wid_game_map_client_replace_tile(
                                            wid_game_map_client_grid_container,
                                            x, y, t, 0);
                }
            }

            w = thing_wid(t);
            if (ext2 & (1 << THING_STATE_BIT_SHIFT_EXT2_COLOR)) {
                c.a = 255;
                wid_set_color(w, WID_COLOR_BLIT, c);
            }

            if (scale != 1.0) {
                t->scale = scale;

                wid_scaling_blit_to_pct_in(w, scale, scale, 500, 9999999);
            }
        }

        if (ext1 & (1 << THING_STATE_BIT_SHIFT_EXT1_EFFECT_PRESENT)) {
            thing_client_effect(t, effect);
        }

        if (ext1 & (1 << THING_STATE_BIT_SHIFT_EXT1_HAS_LEFT_LEVEL)) {
            thing_hide(t);
        } else {
            thing_visible(t);
        }

        if (torch_light_radius_present) {
            t->torch_light_radius = torch_light_radius;
            t->torch_light_radius_set = true;
        }

        if (ext1 & (1 << THING_STATE_BIT_SHIFT_EXT1_IS_DEAD)) {
//CON("rx %s dead",thing_logname(t));
            thing_dead(t, 0, "server killed");
        }

        if (ext1 & (1 << THING_STATE_BIT_SHIFT_EXT1_IS_ACTIVE)) {
//CON("rx %s active",thing_logname(t));
            if (!t->on_active_list) {
                thing_make_active(t);
            }
        }

        if (!(ext1 & (1 << THING_STATE_BIT_SHIFT_EXT1_IS_SLEEPING))) {
//CON("rx %s awake",thing_logname(t));
            if (t->is_sleeping) {
                thing_wake(t);
            }
        }
    }

    if (need_fixup) {
        levelp level;

        level =
            (typeof(level)) wid_get_client_context(
                                        wid_game_map_client_grid_container);

        level_set_needs_fixup(level, true);
    }
}

void thing_move_set_dir (thingp t,
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
            thing_set_dir_tl(t);
        } else if (right) {
            thing_set_dir_tr(t);
        } else {
            thing_set_dir_up(t);
        }
    } else if (down) {
        if (left) {
            thing_set_dir_bl(t);
        } else if (right) {
            thing_set_dir_br(t);
        } else {
            thing_set_dir_down(t);
        }
    } else if (left) {
        thing_set_dir_left(t);
    } else if (right) {
        thing_set_dir_right(t);
    }
}

void thing_set_owner_id (thingp t, uint32_t owner_id)
{
    thingp owner;

    if (!owner_id) {
        thing_set_owner(t, 0);
        return;
    }

    if (t->on_server) {
        owner = thing_server_find(owner_id);
    } else {
        owner = thing_client_find(owner_id);
    }

    thing_set_owner(t, owner);
}

void thing_set_owner (thingp t, thingp owner)
{
    if (owner) {
        verify(owner);
    }

    thingp old_owner = thing_owner(t);
    if (old_owner) {
        if (old_owner == owner) {
            return;
        }

        if (owner) {
            THING_LOG(t, "owner change %s->%s",
                      thing_logname(old_owner), thing_logname(owner));
        } else {
            if (0) {
                THING_LOG(t, "remove owner %s", thing_logname(old_owner));
            }
        }
    } else {
        if (owner) {
            if (0) {
                THING_LOG(t, "owner %s", thing_logname(owner));
            }
        }
    }

    if (owner) {
        t->owner_thing_id = owner->thing_id;

        owner->owned_count++;
    } else {
        t->owner_thing_id = 0;

        if (old_owner) {
            old_owner->owned_count--;
        }
    }
}

uint8_t thing_wid_is_inactive (widp w)
{
    thingp t = wid_get_thing(w);

    return (t->on_active_list ? 0 : 1);
}

uint8_t thing_wid_is_active (widp w)
{
    thingp t = wid_get_thing(w);

    return (t->on_active_list ? 1 : 0);
}

