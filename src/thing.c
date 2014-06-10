/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>
#include <math.h>

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
#include "color.h"
#include "sound.h"
#include "socket.h"
#include "client.h"
#include "timer.h"
#include "math.h"

uint16_t THING_WALL;
uint16_t THING_WALL2;
uint16_t THING_WALL3;
uint16_t THING_WALL4;
uint16_t THING_WALL5;
uint16_t THING_DOOR;
uint16_t THING_NOENTRY;
uint16_t THING_APPLE;
uint16_t THING_APPLE_POISON;
uint16_t THING_BANANA;
uint16_t THING_CHEESE;
uint16_t THING_BREAD;
uint16_t THING_FLOOR;
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
uint16_t THING_PLANT;
uint16_t THING_SEEDPOD;
uint16_t THING_PIPE;
uint16_t THING_EXPLOSION;
uint16_t THING_EXPLOSION1;
uint16_t THING_EXPLOSION2;
uint16_t THING_EXPLOSION3;
uint16_t THING_EXPLOSION4;
uint16_t THING_EXPLOSION5;
uint16_t THING_EXPLOSION6;
uint16_t THING_EXPLOSION7;
uint16_t THING_EXPLOSION8;
uint16_t THING_POISON1;
uint16_t THING_POISON2;
uint16_t THING_POISON3;
uint16_t THING_POISON4;
uint16_t THING_POISON5;
uint16_t THING_POISON6;
uint16_t THING_POISON7;
uint16_t THING_POISON8;
uint16_t THING_CLOUDKILL1;
uint16_t THING_CLOUDKILL2;
uint16_t THING_CLOUDKILL3;
uint16_t THING_CLOUDKILL4;
uint16_t THING_CLOUDKILL5;
uint16_t THING_CLOUDKILL6;
uint16_t THING_CLOUDKILL7;
uint16_t THING_CLOUDKILL8;
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
uint16_t THING_EXIT1;
uint16_t THING_EXIT2;
uint16_t THING_EXIT3;
uint16_t THING_EXIT4;
uint16_t THING_EXIT5;
uint16_t THING_EXIT6;
uint16_t THING_EXIT7;
uint16_t THING_GENERATOR1;
uint16_t THING_GENERATOR2;
uint16_t THING_GENERATOR3;
uint16_t THING_GENERATOR4;
uint16_t THING_GENERATOR5;
uint16_t THING_GENERATOR6;
uint16_t THING_GENERATOR7;
uint16_t THING_BOW1;
uint16_t THING_BOW2;
uint16_t THING_BOW3;
uint16_t THING_BOW4;
uint16_t THING_WAND_FIRE;
uint16_t THING_KEY;
uint16_t THING_KEYS2;
uint16_t THING_KEYS3;
uint16_t THING_COINS1;
uint16_t THING_AMULET1;
uint16_t THING_CHEST1;
uint16_t THING_ARROW;
uint16_t THING_FIREBALL;

tree_root *server_player_things;
tree_root *client_player_things;

tree_root *server_active_things;
tree_root *client_active_things;

tree_root *server_boring_things;
tree_root *client_boring_things;

static uint32_t next_thing_id;
static uint32_t next_monst_thing_id;

thingp thing_server_ids[THING_ID_MAX];
thingp thing_client_ids[THING_ID_MAX];

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

static void thing_map_check_empty_ (thing_map *map, thingp *ids)
{
    uint32_t i;
    uint32_t x;
    uint32_t y;

    for (x = 0; x < MAP_WIDTH; x++) {
        for (y = 0; y < MAP_HEIGHT; y++) {
            thing_map_cell *cell = &map->cells[x][y];

            for (i = 0; i < cell->count; i++) {

                uint16_t m = cell->id[i];
                if (!m) {
                    continue;
                }

                thingp t = ids[m];

                ERR("thing id %d still on map at %d,%d [%d]", m, x, y, i);

                ERR("thing id %d %s still on map", m, thing_logname(t));
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
                uint16_t m = cell->id[i];

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

            uint8_t found_start = 0;
            uint8_t found_end = 0;

            for (i = 0; i < MAP_THINGS_PER_CELL; i++) {
                uint16_t m = cell->id[i];

                if (!m) {
                    found_end = true;
                    continue;
                }

                if (found_end) {
                    thing_map_dump();

                    DIE("map elements are not contiguous at %d,%d", x, y);
                }

                found_start = true;

                thingp t = ids[m];

                if (!t) {
                    DIE("thing %p id %d is invalid and on map", t, m);
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
    uint32_t i;

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
//LOG("rem %s to %s map", thing_logname(t), map == &thing_server_map ?  
//"server" : "client");
    thing_map_cell *cell = &map->cells[x][y];

    if (!cell->count) {
        ERR("map count mismatch");
        return;
    }

    /*
     * Remove from the map.
     */
    for (i = 0; i < cell->count; i++) {
        uint16_t m = cell->id[i];
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
        DIE("cannot add ID of 0");
    }

    if (x < 0) {
        DIE("map underflow");
    }

    if (y < 0) {
        DIE("map y underflow");
    }

    if (x >= MAP_WIDTH) {
        DIE("map x overflow");
    }

    if (y >= MAP_HEIGHT) {
        DIE("map y overflow");
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
        uint16_t m = cell->id[i];

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
            uint16_t m = cell->id[i];
            if (!m) {
                DIE("expected to find a map id on the map here");
            }

            thingp p = ids[m];
            if (!p) {
                DIE("expected to find a thing on the map here");
            }

            if (thing_is_explosion(p)) {
                thing_map_remove(p);
                break;
            }
        }
    }

    if (cell->count == MAP_THINGS_PER_CELL) {
        /*
         * We're hosed.
         */
        ERR("out of map slots trying to add %s", t->logname);

        for (i = 0; i < cell->count; i++) {
            uint16_t m = cell->id[i];
            thingp p = ids[m];

            LOG("  slot [%d] id %d %s", i, m, p->logname);
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
thingp thing_server_new (levelp level, const char *name)
{
    thingp t;
    thing_templatep thing_template;

    thing_template = thing_template_find(name);
    if (!thing_template) {
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
     * Start out not on the map.
     */
    t->last_x = -1.0;
    t->last_y = -1.0;
    t->map_x = -1.0;
    t->map_y = -1.0;
    t->x = -1.0;
    t->y = -1.0;

    /*
     * Use a different base for monsters so that the IDs we create are going
     * to be contiguous and allows us to optimize when sending map updates.
     */
    uint32_t *next;
    uint32_t id;
    uint32_t min;
    uint32_t max;

    if (thing_template_is_monst(thing_template) ||
        thing_template_is_player(thing_template)) {
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

    *next = id;

    t->thing_template = thing_template;
    t->health = thing_template_get_health(thing_template);

    if (thing_template_is_player(thing_template)) {
        t->tree2.key = id;

        if (!tree_insert(server_player_things, &t->tree2.node)) {
            DIE("thing insert name [%s, %u] into server_player_things failed", 
                name, id);
        }

        t->on_server_player_things = true;
        t->tree2.node.is_static_mem = true;
    }

    if (thing_template_is_boring(thing_template)) {
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

    if (level) {
        thing_set_level(t, level);
    }

    t->logname = dynprintf("%s[%p] (server)", thing_short_name(t), t);
    if (!t->updated) {
        t->updated++;
    }

    /*
     * So we send a move update to the client.
     */
    t->last_tx = -1;
    t->last_ty = -1;
    t->first_update = true;

    /*
     * Start out with the items carried on the template if any.
     */
    if (thing_template_is_carryable(thing_template)) {
        uint32_t i;

        for (i = 0; i < THING_MAX; i++) {
            if (thing_template->carrying[i]) {
                thing_collect(t, 0 /* it */, id_to_thing_template(i));
            }
        }
    }

    if (thing_is_player(t)) {
        /*
         * So the client sees any carried weapons at start.
         */
        t->needs_tx_player_update = true;

        THING_LOG(t, "created on server");
    }

    return (t);
}

/*
 * Create a new thing.
 */
thingp thing_client_new (uint32_t id, thing_templatep thing_template)
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
    t->thing_template = thing_template;
    t->on_server = false;

    if (thing_template_is_player(thing_template)) {
        t->tree2.key = id;

        if (!tree_insert(client_player_things, &t->tree2.node)) {
            DIE("thing insert id [%u] failed into client_player_things", id);
        }

        t->on_client_player_things = true;
        t->tree2.node.is_static_mem = true;
    }

    if (thing_template_is_boring(thing_template)) {
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

    t->logname = dynprintf("%s[%p] (client)", thing_short_name(t), t);

    if (thing_is_player(t)) {
        THING_LOG(t, "created on client");
    }

    return (t);
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

    thing_set_level(t, level);

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

static void thing_destroy_implicit (thingp t)
{
    thing_destroy(t, "end of game");
}

void thing_destroy (thingp t, const char *why)
{
    verify(t);

    if (thing_is_player(t)) {
        THING_LOG(t, "destroy (%s)", why);
    }
 
    if (!tree_remove(t->client_or_server_tree, &t->tree.node)) {
        DIE("thing template destroy name [%s] failed", thing_name(t));
    }

    t->client_or_server_tree = 0;
    t->on_active_list = false;

    if (t->on_client_player_things) {
        t->on_client_player_things = false;
        if (!tree_remove(client_player_things, &t->tree2.node)) {
            DIE("thing template destroy name [%s] failed", thing_name(t));
        }
    }

    if (t->on_server_player_things) {
        t->on_server_player_things = false;
        if (!tree_remove(server_player_things, &t->tree2.node)) {
            DIE("thing template destroy name [%s] failed", thing_name(t));
        }
    }

    /*
     * Stop all timers.
     */
    thing_timers_destroy(t);

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

            LOG("\"%s\" player died", p->name);

            char *tmp = dynprintf("%s died", p->name);
            socket_tx_server_shout(CRITICAL, tmp);
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
    
    if (reason) {
        t->dead_reason = reason;
    }

    if (thing_is_player(t)) {
        THING_LOG(t, "dead (%s)", reason);

        THING_SHOUT_AT(t, CRITICAL, "Killed by %s", reason);
    }
}

void thing_dead (thingp t, thingp killer, const char *reason, ...)
{
    va_list args;

    verify(t);

    if (god_mode) {
        if (thing_is_player(t)) {
            return;
        }
    }

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
             */
            const char *polymorph = 
                            thing_template_polymorph_on_death(t->thing_template);
            if (polymorph) {
                thing_templatep what = thing_template_find(polymorph);

                if (what) {
                    /*
                     * It doth.
                     */
                    t->resync = 1;
                    t->thing_template = what;
                    t->health = thing_template_get_health(what);
                    t->updated++;

                    if (!t->updated) {
                        t->updated++;
                    }

                    socket_server_tx_map_update(0, server_boring_things);
                    return;
                }
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
        uint32_t score = thing_template_get_bonus_score_on_death(
                                                thing_get_template(t));
        if (score && killer) {
            thingp recipient = killer;

            /*
             * Did someone throw this weapon and gets the score?
             */
            if (killer->owner_id) {
                recipient = thing_server_ids[killer->owner_id];
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
            wid_set_color(w, WID_COLOR_BLIT, RED);
        }
    }

    /*
     * Stop bouncing or swaying.
     */
    if (t->wid) {
        if (thing_template_is_effect_pulse(t->thing_template)) {
            wid_scaling_to_pct_in(t->wid, 1.0, 1.0, 0, 0);
        }

        if (thing_template_is_effect_sway(t->thing_template)) {
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
    if (!t->updated) {
        t->updated++;
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

static void thing_hit_ (thingp t, 
                        thingp hitter, 
                        uint32_t damage, 
                        char *reason)
{
    verify(t);

    if (t->is_dead) {
        return;
    }

    /*
     * Take note of the hit so we can send an event to the client.
     */
    t->is_hit_success = true;
    t->is_hit_miss = false;
    if (!t->updated) {
        t->updated++;
    }

    /*
     * Keep hitting until all damage is used up or the thing is dead.
     */
    while (damage > 0) {
        if (t->health <= damage) {
            /*
             * Record who dun it.
             */
            if (hitter) {
                thing_dead(t, hitter, "%s",
                           thing_template_short_name(hitter->thing_template));
            } else {
                thing_dead(t, hitter, "hit");
            }

            /*
             * Explodes on death ala Sith Lord? Only a lesser one, mind.
             */
            if (thing_template_is_combustable(t->thing_template)) {
                level_place_small_explosion(t->level, 
                                            0, // owner
                                            t->x, t->y);
            }

            t->health = 0;
            damage -= t->health;

            /*
             * If polymorphed, hit again?
             */
            if (!t->health) {
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
            t->health -= damage;

            if (thing_is_player(t)) {
                THING_LOG(t, "hit (%s) for %u", reason, damage);
            }

            damage = 0;
        }
    }

    /*
     * If a thing that modifies the level dies, update it.
     */
    if (thing_is_dead(t)) {
        if (thing_is_wall(t) || 
            thing_is_door(t) || 
            thing_is_pipe(t)) {
            level_update(server_level);
        }
    }
}

void thing_hit (thingp t, 
                thingp hitter, 
                uint32_t damage,
                const char *reason, ...)
{
    va_list args;

    verify(t);

    if (god_mode) {
        if (thing_is_player(t)) {
            return;
        }
    }

    if (t->is_dead) {
        return;
    }

    /*
     * If this is a thing on the edge of the level acting as a indestrucatble
     * wall, then don't allow it to be destroyed.
     */
    if (thing_is_wall(t) || thing_is_door(t) || thing_is_pipe(t)) {
        if ((t->x <= 0) || (t->x >= MAP_WIDTH - 1) ||
            (t->y <= 0) || (t->y >= MAP_HEIGHT - 1)) {
            return;
        }
    }

    /*
     * Check to see if this is a thing tht can be damaged by the hitter.
     */
    if (hitter) {
        /*
         * Walls and doors and other solid object are not damaged by poison
         * or similar effects. Limit it to explosions and the like.
         */
        if (thing_is_door(t)            || 
            thing_is_mob_spawner(t)     || 
            thing_is_wall(t)) {

            if (!thing_is_explosion(hitter) &&
                !thing_is_projectile(hitter)) {
                return;
            }
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
        t->is_hit_miss = true;
        if (!t->updated) {
            t->updated++;
        }
    }

    /*
     * Does the thing do damage?
     */
    if (!damage) {
        if (hitter) {
            damage = thing_get_template(hitter)->damage;
        }
    }
    
    /*
     * Update the map for destruction of the scenery.
     */
    if (thing_is_wall(t) || thing_is_door(t) || thing_is_pipe(t)) {
        level_update(server_level);
    }

    /*
     * Does the thing get off being hit.
     */
    uint32_t can_be_hit_chance = 
                    thing_template_get_can_be_hit_chance(t->thing_template);
    if (can_be_hit_chance) {
        uint32_t chance = rand() % can_be_hit_chance;

        if (chance > damage) {
            return;
        }
    }

    /*
     * Allow no more hits than x per second by the hitter.
     */
    if (hitter) {
        /*
         * Have we ran into our own spell effect? Cast no damage on ourselves.
         */
        if (hitter->owner_id == t->thing_id) {
            return;
        }

        uint32_t delay = 
            thing_template_get_hit_delay_tenths(hitter->thing_template);

        if (delay) {
            if (!time_have_x_tenths_passed_since(delay, 
                                                 hitter->timestamp_hit)) {
                return;
            }

            hitter->timestamp_hit = time_get_time_cached();
        }

        /*
         * No killer to avoid giving a bonus to monsters!
         */
        if (thing_is_fragile(hitter)) {
            thing_dead(hitter, 0, "self destruct on hitting");
        }
    }

    if (reason) {
        char *tmp = dynvprintf(reason, args);

        va_start(args, reason);
        thing_hit_(t, hitter, damage, tmp);
        va_end(args);

        myfree(tmp);
    } else {
        thing_hit_(t, hitter, damage, 0);
    }
}

static void thing_effect_hit_success (thingp t)
{
    verify(t);

    widp w = t->wid;
    if (w) {
        wid_set_mode(w, WID_MODE_ACTIVE);
        wid_set_color(w, WID_COLOR_BLIT, RED);
    }
}

static void thing_effect_hit_miss (thingp t)
{
    verify(t);

    widp w = t->wid;
    if (w) {
        wid_set_mode(w, WID_MODE_ACTIVE);
        wid_set_color(w, WID_COLOR_BLIT, GRAY);
    }
}

void thing_reached_exit (thingp t)
{
    levelp level;

    verify(t);

    level = thing_level(t);
    verify(level);

    /*
     * First to the exit?
     */
    if (!level_exit_reached_when_open(level)) {
        level_set_exit_reached_when_open(level, true);

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

void things_level_start (levelp level)
{
    thingp t;

    {
        TREE_WALK_UNSAFE(server_active_things, t) {
            thing_set_level(t, level);
        }
    }

    {
        TREE_WALK_UNSAFE(server_boring_things, t) {
            thing_set_level(t, level);
        }
    }
}

void things_level_destroyed (levelp level)
{
    thingp t;

    {
        TREE_WALK(server_active_things, t) {
            thing_destroy(t, "level destroyed");
        }
    }

    {
        TREE_WALK(server_boring_things, t) {
            thing_destroy(t, "level destroyed");
        }
    }

    {
        TREE_WALK(client_active_things, t) {
            thing_destroy(t, "level destroyed");
        }
    }

    {
        TREE_WALK(client_boring_things, t) {
            thing_destroy(t, "level destroyed");
        }
    }
}

thing_templatep thing_get_template (thingp t)
{
    verify(t);

    return (t->thing_template);
}

const char *thing_logname (thingp t)
{
    verify(t);

    return (t->logname);
}

const char *thing_short_name (thingp t)
{
    verify(t);

    return (thing_template_short_name(t->thing_template));
}

const char *thing_dead_reason (thingp t)
{
    verify(t);

    return (t->dead_reason);
}

uint8_t thing_z_depth (thingp t)
{
    verify(t);

    return (thing_template_get_z_depth(t->thing_template));
}

uint8_t thing_z_order (thingp t)
{
    verify(t);

    return (thing_template_get_z_order(t->thing_template));
}

uint32_t thing_level_no (thingp t)
{
    verify(t);

    return (t->level_no);
}

void thing_set_level_no (thingp t, uint32_t level)
{
    verify(t);

    t->level_no = level;
}

levelp thing_level (thingp t)
{
    verify(t);

    return (t->level);
}

void thing_set_level (thingp t, levelp level)
{
    verify(t);

    t->level = level;
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

widp thing_message (thingp t, const char *message)
{
    verify(t);

    if (!thing_is_player(t)) {
        return (0);
    }
    
    if (!t->wid) {
        return (0);
    }

    widp w = wid_textbox(wid_game_map_client_window,
                         &wid_score, message, 0.5, 0.03, med_font);

    wid_set_no_shape(w);
    wid_set_color(wid_score, WID_COLOR_TEXT, YELLOW);

    /*
     * Move the text over the thing.
     */
    double mx, my;
    wid_get_mxy(t->wid, &mx, &my);

    uint32_t lifespan = ONESEC * 3;

    /*
     * Fade out.
     */
    wid_fade_out(w, lifespan);

    /*
     * Self destroy.
     */
    wid_destroy_in(w, lifespan / 4);

    /*
     * Float up from the thing.
     */
    double floater = global_config.video_gl_height / TILES_SCREEN_HEIGHT;

    wid_move_to_abs_centered(w, mx, my - floater / 2.0);
    wid_move_to_abs_centered_in(w, mx, my - floater * 1.5, lifespan);

    /*
     * And stay in front.
     */
    wid_set_z_depth(w, 100);

    return (wid_score);
}

tree_rootp thing_tile_tiles (thingp t)
{
    verify(t);

    return (thing_template_get_tiles(t->thing_template));
}

tree_rootp thing_tile_tiles2 (thingp t)
{
    verify(t);

    return (thing_template_get_tiles2(t->thing_template));
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

    if (w) {
        THING_DBG(t, "set wid %p/%s", w, wid_logname(w));
    }
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

void thing_set_is_qqq1 (thingp t, uint8_t val)
{
    verify(t);

    t->is_qqq1 = val;
}

uint8_t thing_is_qqq1 (thingp t)
{
    verify(t);

    return (t->is_qqq1);
}

void thing_set_is_qqq2 (thingp t, uint8_t val)
{
    verify(t);

    t->is_qqq2 = val;
}

uint8_t thing_is_qqq2 (thingp t)
{
    verify(t);

    return (t->is_qqq2);
}

void thing_set_is_qqq3 (thingp t, uint8_t val)
{
    verify(t);

    t->is_qqq3 = val;
}

uint8_t thing_is_qqq3 (thingp t)
{
    verify(t);

    return (t->is_qqq3);
}

void thing_set_is_qqq4 (thingp t, uint8_t val)
{
    verify(t);

    t->is_qqq4 = val;
}

uint8_t thing_is_qqq4 (thingp t)
{
    verify(t);

    return (t->is_qqq4);
}

void thing_set_is_qqq5 (thingp t, uint8_t val)
{
    verify(t);

    t->is_qqq5 = val;
}

uint8_t thing_is_qqq5 (thingp t)
{
    verify(t);

    return (t->is_qqq5);
}

void thing_set_is_qqq6 (thingp t, uint8_t val)
{
    verify(t);

    t->is_qqq6 = val;
}

uint8_t thing_is_qqq6 (thingp t)
{
    verify(t);

    return (t->is_qqq6);
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

    if (t->dir != THING_DIR_DOWN) {
        if (!t->updated) {
            t->updated++;
        }

        t->dir = THING_DIR_DOWN;
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

    if (t->dir != THING_DIR_UP) {
        t->updated++;
        if (!t->updated) {
            t->updated++;
        }

        if (!t->updated) {
            t->updated++;
        }

        t->dir = THING_DIR_UP;
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

    if (t->dir != THING_DIR_LEFT) {
        t->updated++;
        if (!t->updated) {
            t->updated++;
        }

        t->dir = THING_DIR_LEFT;
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

    if (t->dir != THING_DIR_RIGHT) {
        t->updated++;
        if (!t->updated) {
            t->updated++;
        }

        t->dir = THING_DIR_RIGHT;
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

    if (t->dir != THING_DIR_TL) {
        t->updated++;
        if (!t->updated) {
            t->updated++;
        }

        t->dir = THING_DIR_TL;
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

    if (t->dir != THING_DIR_BL) {
        t->updated++;
        if (!t->updated) {
            t->updated++;
        }

        t->dir = THING_DIR_BL;
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

    if (t->dir != THING_DIR_TR) {
        t->updated++;
        if (!t->updated) {
            t->updated++;
        }

        t->dir = THING_DIR_TR;
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

    if (t->dir != THING_DIR_BR) {
        t->updated++;
        if (!t->updated) {
            t->updated++;
        }

        t->dir = THING_DIR_BR;
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

    return (thing_template_short_name(t->thing_template));
}

const char * thing_tooltip (thingp t)
{
    verify(t);

    return (thing_template_get_tooltip(t->thing_template));
}

double thing_speed (thingp t)
{
    verify(t);

    return (((double)thing_template_get_speed(t->thing_template)));
}

tree_rootp thing_tiles (thingp t)
{
    verify(t);

    return (thing_template_get_tiles(t->thing_template));
}

tree_rootp thing_tiles2 (thingp t)
{
    verify(t);

    return (thing_template_get_tiles2(t->thing_template));
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
void thing_place_timed (thing_templatep thing_template, 
                        double x,
                        double y,
                        uint32_t ms, 
                        uint32_t jitter)
{
    thing_place_context_t *context;

    context = (typeof(context)) myzalloc(sizeof(*context), "place thing");

    context->x = x;
    context->y = y;
    context->level = server_level;
    context->thing_template = thing_template;

    action_timer_create(
            &timers,
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
void thing_place_and_destroy_timed (thing_templatep thing_template, 
                                    thingp owner,
                                    double x,
                                    double y,
                                    uint32_t ms, 
                                    uint32_t destroy_in, 
                                    uint32_t jitter)
{
    thing_place_context_t *context;

    context = (typeof(context)) myzalloc(sizeof(*context), "place thing");

    context->x = x;
    context->y = y;
    context->level = server_level;
    context->destroy_in = destroy_in;
    context->thing_template = thing_template;
    if (owner) {
        context->owner_id = owner->thing_id;
    }

    action_timer_create(
            &timers,
            (action_timer_callback)
                thing_timer_place_and_destroy_callback,
            (action_timer_callback)
                thing_timer_place_and_destroy_destroy_callback,
            context,
            "place and destroy thing",
            ms,
            jitter);
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
                                    thing_template_is_floor);
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

void thing_server_wid_update (thingp t, double x, double y, uint8_t is_new)
{
    thing_move(t, x, y);

    x *= server_tile_width;
    y *= server_tile_height;

    x += server_tile_width / 2;
    y += server_tile_height / 2;

    fpoint tl = { x, y };
    fpoint br = { x, y };

    double base_tile_width =
            ((1.0f / ((double)TILES_SCREEN_WIDTH) / TILES_SERVER_SCALE) *
                (double)global_config.video_gl_width);

    double base_tile_height =
            ((1.0f / ((double)TILES_SCREEN_HEIGHT) / TILES_SERVER_SCALE) *
                (double)global_config.video_gl_height);

    br.x += base_tile_width;
    br.y += base_tile_height;

    br.x += base_tile_width / 4.0;
    br.y += base_tile_height / 4.0;

    br.x += base_tile_width / 6.0;
    br.y += base_tile_height / 4.0;

    tl.x -= base_tile_height / 2.0;
    br.x -= base_tile_width / 2.0;

    tl.x += base_tile_height / 8.0;
    br.x += base_tile_width / 8.0;

    tl.y -= base_tile_height / 2.0;
    br.y -= base_tile_width / 2.0;

    tl.y -= base_tile_height / 4.0;
    br.y -= base_tile_width / 4.0;

    if (is_new || thing_is_player(t)) {
        wid_set_tl_br(t->wid, tl, br);
    } else {
        wid_move_to_abs_in(t->wid, tl.x, tl.y, 1000.0 / thing_speed(t));
    }
}

void thing_client_wid_update (thingp t, double x, double y, uint8_t smooth)
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
            ((1.0f / ((double)TILES_SCREEN_WIDTH) / TILES_CLIENT_SCALE) *
                (double)global_config.video_gl_width);

    double base_tile_height =
            ((1.0f / ((double)TILES_SCREEN_HEIGHT) / TILES_CLIENT_SCALE) *
                (double)global_config.video_gl_height);

    br.x += base_tile_width;
    br.y += base_tile_height;

    br.x += base_tile_width / 4.0;
    br.y += base_tile_height / 4.0;

    br.x += base_tile_width / 6.0;
    br.y += base_tile_height / 4.0;

    tl.x -= base_tile_height / 2.0;
    br.x -= base_tile_width / 2.0;

    tl.x += base_tile_height / 8.0;
    br.x += base_tile_width / 8.0;

    tl.y -= base_tile_height / 2.0;
    br.y -= base_tile_width / 2.0;

    tl.y -= base_tile_height / 4.0;
    br.y -= base_tile_width / 4.0;

    if (smooth) {
        double time_step = dist;
        double ms = (1000.0 / thing_speed(t)) / (1.0 / time_step);

        wid_move_to_abs_in(t->wid, tl.x, tl.y, ms);
    } else {
        wid_set_tl_br(t->wid, tl, br);
    }
}

void socket_server_tx_map_update (socketp p, tree_rootp tree)
{
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

    uint16_t last_id;

    last_id = 0;

    TREE_WALK_UNSAFE(tree, t) {

        verify(t);

        thing_templatep thing_template = t->thing_template;

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
            if (!time_have_x_thousandths_passed_since(
                    thing_template_get_tx_map_update_delay_thousandths(
                                                            thing_template),
                    t->timestamp_tx_map_update)) {
                continue;
            }

            t->timestamp_tx_map_update = time_get_time_cached();

            t->updated--;
        }

        /*
         * As an optimization do not send dead events for explosions. Let the
         * client destroy those on its own to save sending loads of events.
         */
        if (thing_is_dead(t)) {
            if (thing_template_is_explosion(thing_template)) {
                continue;
            }
        }

        /*
         * Work out what we are going to send.
         */
        uint8_t template_id = thing_template_to_id(thing_template);
        uint16_t id = t->thing_id;
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
            ((t->is_hit_success ? 1 : 0) << 
                THING_STATE_BIT_SHIFT_EXT_PRESENT) |
            ((t->is_hit_miss    ? 1 : 0) << 
                THING_STATE_BIT_SHIFT_EXT_PRESENT);

        const uint8_t ext =
            ((t->is_dead        ? 1 : 0) << 
                THING_STATE_BIT_SHIFT_EXT_IS_DEAD) |
            ((t->is_hit_success ? 1 : 0) << 
                THING_STATE_BIT_SHIFT_EXT_IS_HIT_SUCCESS) |
            ((t->is_hit_miss    ? 1 : 0) << 
                THING_STATE_BIT_SHIFT_EXT_IS_HIT_MISS);

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
    uint8_t need_fixup = false;
    verify(s);

    uint8_t *eodata = data + packet->len - 1;
    uint16_t last_id = 0;

    while (data < eodata) {
        uint8_t state = *data++;
        uint8_t ext;
        uint8_t template_id;
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
             * Full template ID update.
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

        t = thing_client_find(id);
        if (!t) {
            if (template_id == (uint8_t)-1) {
                /*
                 * This could happen due to packet loss and we have no way
                 * to rebuild the thing without a resend. Need a way to ask
                 * for a resync.
                 */
                ERR("received unknown thing %u, need resync", id);
                continue;
            }

            if ((ext & (1 << THING_STATE_BIT_SHIFT_EXT_IS_DEAD))) {
                /*
                 * Don't create the thing if already dead.
                 */
                continue;
            }

            thing_templatep thing_template = 
                    id_to_thing_template(template_id);

            t = thing_client_new(id, thing_template);

            need_fixup = need_fixup ||
                thing_template_is_wall(thing_template) ||
                thing_template_is_pipe(thing_template) ||
                thing_template_is_door(thing_template);
        } else {
            if (template_id != (uint8_t)-1) {
                /*
                 * Update the template ID so things can polymorph.
                 */
                thing_templatep thing_template = 
                        id_to_thing_template(template_id);

                t->thing_template = thing_template;

                /*
                 * Polymorph the thing. This is not needed if the thing is
                 * animated. But if not, we do.
                 */
                if (!thing_is_animated(t) && !thing_is_joinable(t)) {
                    widp w = thing_wid(t);

                    if (w) {
                        wid_set_thing_template(w, thing_template);
                    }
                }

                need_fixup = need_fixup ||
                    thing_template_is_wall(thing_template) ||
                    thing_template_is_pipe(thing_template) ||
                    thing_template_is_door(thing_template);
            }
        }

        /*
         * Get the thing direction.
         */
        t->dir = state & 0x7;

        /*
         * Mirror the thing id of the server on the client.
         */
        t->thing_id = id;

        /*
         * Move the thing?
         */
        if (state & (1 << THING_STATE_BIT_SHIFT_XY_PRESENT)) {
            widp w = thing_wid(t);
            if (w) {
                if (t == player) {
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

                        thing_client_wid_update(t, x, y, true /* smooth */);
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

                        thing_client_wid_update(t, x, y, true /* smooth */);
                    }
                } else if (on_map) {
                    thing_client_wid_update(t, x, y, true /* smooth */);
                }
            } else {
                if (t->is_dead || 
                    (ext & (1 << THING_STATE_BIT_SHIFT_EXT_IS_DEAD))) {
                    /*
                     * Already dead? No new tile.
                     */
                } else if (!on_map) {
                    /*
                     * Popped off the map.
                     */
                } else {
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

        if (ext & (1 << THING_STATE_BIT_SHIFT_EXT_IS_DEAD)) {
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

    memcpy(data, t->carrying, sizeof(t->carrying));
    data += sizeof(t->carrying);

    if (t->weapon) {
        *data++ = thing_template_to_id(t->weapon);
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

    memcpy(t->carrying, data, sizeof(t->carrying));
    data += sizeof(t->carrying);

    id = *data++;
    if (id) {
        t->weapon = id_to_thing_template(id);
    } else {
        t->weapon = 0;
    }
}

static void thing_common_move (thingp t,
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

    thing_common_move(t, &x, &y, up, down, left, right);

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
     * Use the currently wielded weapon. Or perhaps the thing has an
     * intrinsic weapon ability?
     */
    thing_templatep weapon = t->weapon;
    if (!weapon) {
        THING_SHOUT_AT(t, WARNING, "You have no weapon");
        return;
    }

    /*
     * Check if the weapon reaches its end of warranty.
     */
    if (weapon == t->weapon) {
        uint32_t failure_chance = thing_template_get_failure_chance(weapon);

        if (failure_chance) {
            if ((rand() % failure_chance) == 0) {
                thing_item_destroyed(t, weapon);
                return;
            }
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

    thing_templatep projectile = thing_template_fires(weapon);
    if (!projectile) {
        ERR("weapon %s has no projectile", thing_template_name(weapon));
        return;
    }

    widp w = wid_game_map_server_replace_tile(
                                    wid_game_map_server_grid_container,
                                    x,
                                    y,
                                    projectile);

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

    thing_common_move(t, &x, &y, up, down, left, right);

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

            if (!t->updated) {
                t->updated++;
            }

            t->resync = 1;

            return (false);
        }
    }

    thing_common_move(t, &x, &y, up, down, left, right);

    thing_server_wid_update(t, x, y, false /* is_new */);
    if (!t->updated) {
        t->updated++;
    }

    thing_handle_collisions(wid_game_map_server_grid_container, t);

    if (fire) {
        thing_fire(t, up, down, left, right);
    }

    return (true);
}

void thing_server_action (thingp t,
                          uint8_t action,
                          uint16_t item)
{
    widp grid = wid_game_map_server_grid_container;

    thing_templatep thing_template = id_to_thing_template(item);
    if (!thing_template) {
        ERR("Unkown item use request, id %u", item);
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
    case PLAYER_ACTION_USE:
        if (!thing_is_carrying(t, item)) {
            /*
             * Sneaky.
             */
            THING_SHOUT_AT(t, WARNING, "You do not have that item");
            return;
        }

        if (thing_template_is_weapon(thing_template)) {
            thing_wield(t, thing_template);
            return;
        }

        if (item == THING_POTION_FIRE) {
            level_place_potion_effect_fireball(server_level, t, t->x, t->y);
            break;
        } else if (item == THING_POTION_MONSTICIDE) {
            level_place_potion_effect_poison(server_level, t, t->x, t->y);
            break;
        } else if (item == THING_POTION_CLOUDKILL) {
            level_place_potion_effect_cloudkill(server_level, t, t->x, t->y);
            break;
        }

        const char *message = thing_template_message_on_use(thing_template);
        if (message) {
            THING_SHOUT_AT(t, INFO, "%s", message);
            break;
        }

        /*
         * Failed to use.
         */
        THING_SHOUT_AT(t, WARNING, "Failed to use the %s", 
                       thing_template_short_name(thing_template));
        return;

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
            if (wid_game_map_server_replace_tile(grid, x, y, thing_template)) {
                socket_server_tx_map_update(0, server_boring_things);
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
                                                         thing_template)) {
                        socket_server_tx_map_update(0, server_boring_things);
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
        ERR("Unkown player action %u on item id %u", action, item);
        return;
    }

done:

    switch (action) {
    case PLAYER_ACTION_USE:
        thing_used(t, thing_template);
        break;

    case PLAYER_ACTION_DROP:
        thing_drop(t, thing_template);
        break;
    }
}
