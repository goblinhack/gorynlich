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
#include "color.h"
#include "sound.h"
#include "socket.h"
#include "client.h"

uint16_t THING_WALL;
uint16_t THING_WALL2;
uint16_t THING_WALL3;
uint16_t THING_WALL4;
uint16_t THING_WALL5;
uint16_t THING_DOOR;
uint16_t THING_NOENTRY;
uint16_t THING_FOOD;
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
uint16_t THING_GHOST;
uint16_t THING_DEMON;
uint16_t THING_MONK;
uint16_t THING_LIZARD;
uint16_t THING_DEATH;
uint16_t THING_PLANT;
uint16_t THING_SEEDPOD;
uint16_t THING_PIPE;
uint16_t THING_EXPLOSION;
uint16_t THING_BOMB;
uint16_t THING_SPAM;
uint16_t THING_POTION1;
uint16_t THING_POTION2;
uint16_t THING_POTION3;
uint16_t THING_POTION4;
uint16_t THING_POTION5;
uint16_t THING_WATER1;
uint16_t THING_WATER2;
uint16_t THING_MASK1;
uint16_t THING_MASK2;
uint16_t THING_MASK3;
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
uint16_t THING_KEYS1;
uint16_t THING_KEYS2;
uint16_t THING_KEYS3;
uint16_t THING_COINS1;
uint16_t THING_AMULET1;
uint16_t THING_CHEST1;

tree_root *server_player_things;
tree_root *client_player_things;

tree_root *server_active_things;
tree_root *client_active_things;

tree_root *server_boring_things;
tree_root *client_boring_things;

static uint32_t thing_id;
static thingp thing_ids[THING_ID_MAX];
static boolean thing_init_done;
static void thing_destroy_implicit(thingp t);

boolean thing_init (void)
{
    thing_init_done = true;

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
    }
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

    /*
     * Find a free thign slot
     */
    int looped = 0;
    while (thing_ids[++thing_id]) {
        thing_id++;
        if (thing_id >= THING_ID_MAX) {
            thing_id = 0;
            if (looped++) {
                DIE("out of thing ids!");
            }
        }
    }

    t->tree.key = thing_id;
    thing_ids[thing_id] = t;
    t->thing_id = thing_id;

    t->thing_template = thing_template;
    t->health = thing_template_get_health(thing_template);
    t->on_server = true;

    if (thing_template_is_player(thing_template)) {
        t->tree2.key = thing_id;

        if (!tree_insert(server_player_things, &t->tree2.node)) {
            DIE("thing insert name [%s] failed", name);
        }

        t->tree2.node.is_static_mem = true;
    }

    if (thing_template_is_boring(thing_template)) {
        if (!tree_insert(server_boring_things, &t->tree.node)) {
            DIE("thing insert name [%s] failed", name);
        }

        t->client_or_server_tree = server_boring_things;
    } else {
        if (!tree_insert(server_active_things, &t->tree.node)) {
            DIE("thing insert name [%s] failed", name);
        }

        t->on_active_list = true;
        t->client_or_server_tree = server_active_things;
    }

    if (level) {
        thing_set_level(t, level);
    }

    t->logname = dynprintf("%s[%p] (server)", thing_short_name(t), t);
    t->updated++;

    THING_DBG(t, "created");

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
    t->tree.key = id;
    t->thing_template = thing_template;
    t->on_server = false;

    if (thing_template_is_player(thing_template)) {
        t->tree2.key = thing_id;

        if (!tree_insert(client_player_things, &t->tree2.node)) {
            DIE("thing insert id [%u] failed", id);
        }

        t->tree2.node.is_static_mem = true;
    }

    if (thing_template_is_boring(thing_template)) {
        if (!tree_insert(client_boring_things, &t->tree.node)) {
            DIE("thing insert id [%u] failed", id);
        }

        t->client_or_server_tree = client_boring_things;
    } else {
        if (!tree_insert(client_active_things, &t->tree.node)) {
            DIE("thing insert id [%u] failed", id);
        }

        t->client_or_server_tree = client_active_things;
        t->on_active_list = true;
    }

    t->logname = dynprintf("%s[%p] (client)", thing_short_name(t), t);

    THING_DBG(t, "created");

    return (t);
}

/*
 * Find an existing new thing.
 */
thingp thing_client_find (uint32_t thing_id)
{
    thing target;
    thingp result;

    // memset(&target, 0, sizeof(target));
    target.tree.key = thing_id;

    result = (typeof(result)) 
                    tree_find(client_active_things, &target.tree.node);
    if (!result) {
        result = (typeof(result)) 
                        tree_find(client_boring_things, &target.tree.node);
    }

    return (result);
}

void thing_restarted (thingp t, levelp level)
{
    verify(t);

    thing_set_level(t, level);

    t->current_tile = 0;

    memset(t->visited, 0, sizeof(t->visited));

    if (!thing_is_dead(t) || !thing_is_buried(t)) {
        return;
    }

    if (t->dead_reason) {
        myfree(t->dead_reason);
        t->dead_reason = 0;
    }

    thing_set_is_dead(t, false);
    thing_set_is_buried(t, false);

    /*
     * Reset last map position.
     */
    t->last_x = 0;
    t->last_y = 0;

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

    THING_DBG(t, "destroy (%s)", why);
 
    if (!tree_remove(t->client_or_server_tree, &t->tree.node)) {
        DIE("thing template destroy name [%s] failed", thing_name(t));
    }

    t->client_or_server_tree = 0;
    t->on_active_list = false;

    /*
     * Stop all timers.
     */
    thing_timers_destroy(t);

    if (t->dead_reason) {
        myfree(t->dead_reason);
        t->dead_reason = 0;
    }

    if (t->wid) {
        thing_set_wid(t, 0);
    }

    if (t->logname) {
        myfree(t->logname);
        t->logname = 0;
    }

    if (t == player) {
        player = 0;
    }

    thing_ids[t->thing_id] = 0;

    myfree(t);
}

void thing_bury (thingp t)
{
    verify(t);

    if (!thing_is_dead(t)) {
        DIE("can't bury %s if not dead!", thing_logname(t));
        return;
    }

    if (thing_is_buried(t)) {
        return;
    }

    thing_set_is_buried(t, true);
    THING_DBG(t, "buried");
}

static void thing_dead_ (thingp t, thingp killer, char *reason)
{
    verify(t);

    if (t->is_dead) {
        return;
    }

    thing_set_is_dead(t, true);

    /*
     * Bounty for the killer?
     */
    if (killer) {
        uint32_t score = thing_template_get_score_on_death(
                                                thing_get_template(t));

        if (score) {
            thing_set_score(killer, thing_score(killer) + score);
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
     * Why did I die!? 8(
     */
    if (t->dead_reason) {
        myfree(t->dead_reason);
        t->dead_reason = 0;
    }
    
    if (reason) {
        t->dead_reason = reason;
    }

    if (!thing_is_left_as_corpse_on_death(t)) {
        /*
         * Pop from the level.
         */
        thing_set_wid(t, 0);
    }

    THING_DBG(t, "dead (%s)", reason);
}

void thing_dead (thingp t, thingp killer, const char *reason, ...)
{
    verify(t);

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

    if (reason) {
        va_start(args, reason);
        thing_dead_(t, killer, dynvprintf(reason, args));
        va_end(args);
    } else {
        thing_dead_(t, killer, 0);
    }

    t->updated++;
    t->updated++;

    /*
     * If we use update + 1 it means we will have time to send both updates
     * and then kill the thing.
     */
    t->destroy_delay = 3;

    if (thing_is_player(t)) {
        if (t->on_server) {
            if (!tree_remove(server_player_things, &t->tree2.node)) {
                DIE("thing move, remove server player [%s] failed", 
                    thing_name(t));
            }
        } else {
            if (!tree_remove(client_player_things, &t->tree2.node)) {
                DIE("thing move, remove client player [%s] failed", 
                    thing_name(t));
            }
        }
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
        TREE_WALK(server_active_things, t) {
            thing_set_level(t, level);
        }
    }

    {
        TREE_WALK(server_boring_things, t) {
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
    wid_destroy_in(w, lifespan / 2);

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

void thing_inc_score_pump (thingp t, uint32_t delta)
{
    verify(t);

    t->score_pump += delta;

    char *tmp = dynprintf("%6u", delta);
    thing_message(t, tmp);
    myfree(tmp);
}

void thing_set_score_pump (thingp t, uint32_t score_pump)
{
    verify(t);

    t->score_pump = score_pump;
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
            wid_destroy(&t->wid);
        }
    }

    t->wid = w;

    if (w) {
        THING_DBG(t, "set wid %p/%s", w, wid_logname(w));
    }
}

void thing_inc_powerup_spam_count (thingp t, boolean val)
{
    verify(t);

    t->powerup_spam_count += val;
}

void thing_dec_powerup_spam_count (thingp t, boolean val)
{
    verify(t);

    if (!t->powerup_spam_count) {
        DIE("powerup decreemnt");
    }

    t->powerup_spam_count -= val;
}

boolean thing_has_powerup_spam_count (thingp t)
{
    verify(t);

    return (t->powerup_spam_count);
}

void thing_inc_powerup2_count (thingp t, boolean val)
{
    verify(t);

    t->powerup2_count += val;
}

void thing_dec_powerup2_count (thingp t, boolean val)
{
    verify(t);

    if (!t->powerup2_count) {
        DIE("powerup decreemnt");
    }

    t->powerup2_count -= val;
}

boolean thing_has_powerup2_count (thingp t)
{
    verify(t);

    return (t->powerup2_count);
}

void thing_inc_powerup3_count (thingp t, boolean val)
{
    verify(t);

    t->powerup3_count += val;
}

void thing_dec_powerup3_count (thingp t, boolean val)
{
    verify(t);

    if (!t->powerup3_count) {
        DIE("powerup decreemnt");
    }

    t->powerup3_count -= val;
}

boolean thing_has_powerup3_count (thingp t)
{
    verify(t);

    return (t->powerup3_count);
}

void thing_inc_powerup4_count (thingp t, boolean val)
{
    verify(t);

    t->powerup4_count += val;
}

void thing_dec_powerup4_count (thingp t, boolean val)
{
    verify(t);

    if (!t->powerup4_count) {
        DIE("powerup decreemnt");
    }

    t->powerup4_count -= val;
}

boolean thing_has_powerup4_count (thingp t)
{
    verify(t);

    return (t->powerup4_count);
}

void thing_inc_powerup5_count (thingp t, boolean val)
{
    verify(t);

    t->powerup5_count += val;
}

void thing_dec_powerup5_count (thingp t, boolean val)
{
    verify(t);

    if (!t->powerup5_count) {
        DIE("powerup decreemnt");
    }

    t->powerup5_count -= val;
}

boolean thing_has_powerup5_count (thingp t)
{
    verify(t);

    return (t->powerup5_count);
}

void thing_inc_powerup6_count (thingp t, boolean val)
{
    verify(t);

    t->powerup6_count += val;
}

void thing_dec_powerup6_count (thingp t, boolean val)
{
    verify(t);

    if (!t->powerup6_count) {
        DIE("powerup decreemnt");
    }

    t->powerup6_count -= val;
}

boolean thing_has_powerup6_count (thingp t)
{
    verify(t);

    return (t->powerup6_count);
}

void thing_inc_powerup7_count (thingp t, boolean val)
{
    verify(t);

    t->powerup7_count += val;
}

void thing_dec_powerup7_count (thingp t, boolean val)
{
    verify(t);

    if (!t->powerup7_count) {
        DIE("powerup decreemnt");
    }

    t->powerup7_count -= val;
}

boolean thing_has_powerup7_count (thingp t)
{
    verify(t);

    return (t->powerup7_count);
}

void thing_inc_powerup8_count (thingp t, boolean val)
{
    verify(t);

    t->powerup8_count += val;
}

void thing_dec_powerup8_count (thingp t, boolean val)
{
    verify(t);

    if (!t->powerup8_count) {
        DIE("powerup decreemnt");
    }

    t->powerup8_count -= val;
}

boolean thing_has_powerup8_count (thingp t)
{
    verify(t);

    return (t->powerup8_count);
}

void thing_inc_powerup_rocket_count (thingp t, boolean val)
{
    verify(t);

    t->powerup_rocket_count += val;

    THING_LOG(t, "powerup inc, rocket now %d", t->powerup_rocket_count);

    sound_play_rocket();
}

void thing_dec_powerup_rocket_count (thingp t, boolean val)
{
    verify(t);

    if (!t->powerup_rocket_count) {
        DIE("%s, powerup dec, rocket now %d", t->logname,
            t->powerup_rocket_count);
    }

    t->powerup_rocket_count -= val;

    THING_LOG(t, "powerup dec, rocket now %d", t->powerup_rocket_count);
}

boolean thing_has_powerup_rocket_count (thingp t)
{
    verify(t);

    return (t->powerup_rocket_count);
}

void thing_set_is_qqq1 (thingp t, boolean val)
{
    verify(t);

    t->is_qqq1 = val;
}

boolean thing_is_qqq1 (thingp t)
{
    verify(t);

    return (t->is_qqq1);
}

void thing_set_is_qqq2 (thingp t, boolean val)
{
    verify(t);

    t->is_qqq2 = val;
}

boolean thing_is_qqq2 (thingp t)
{
    verify(t);

    return (t->is_qqq2);
}

void thing_set_is_qqq3 (thingp t, boolean val)
{
    verify(t);

    t->is_qqq3 = val;
}

boolean thing_is_qqq3 (thingp t)
{
    verify(t);

    return (t->is_qqq3);
}

void thing_set_is_qqq4 (thingp t, boolean val)
{
    verify(t);

    t->is_qqq4 = val;
}

boolean thing_is_qqq4 (thingp t)
{
    verify(t);

    return (t->is_qqq4);
}

void thing_set_is_qqq5 (thingp t, boolean val)
{
    verify(t);

    t->is_qqq5 = val;
}

boolean thing_is_qqq5 (thingp t)
{
    verify(t);

    return (t->is_qqq5);
}

void thing_set_is_qqq6 (thingp t, boolean val)
{
    verify(t);

    t->is_qqq6 = val;
}

boolean thing_is_qqq6 (thingp t)
{
    verify(t);

    return (t->is_qqq6);
}

void thing_set_is_qqq7 (thingp t, boolean val)
{
    verify(t);

    t->is_qqq7 = val;
}

boolean thing_is_qqq7 (thingp t)
{
    verify(t);

    return (t->is_qqq7);
}

void thing_set_got_to_exit_first (thingp t, boolean val)
{
    verify(t);

    t->got_to_exit_first = val;
}

boolean thing_got_to_exit_first (thingp t)
{
    verify(t);

    return (t->got_to_exit_first);
}

void thing_set_is_dir_down (thingp t)
{
    verify(t);

    if (t->dir != THING_DIR_DOWN) {
        t->updated++;
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
        t->dir = THING_DIR_BR;
    }
}

uint8_t thing_is_dir_br (thingp t)
{
    verify(t);

    return (t->dir == THING_DIR_BR);
}

void thing_set_opened_exit (thingp t, boolean val)
{
    THING_LOG(t, "opened exit");

    verify(t);

    t->opened_exit = val;
}

boolean thing_opened_exit (thingp t)
{
    verify(t);

    return (t->opened_exit);
}

void thing_set_is_open (thingp t, boolean val)
{
    verify(t);

    t->timestamp_change_to_next_frame = time_get_time_cached();

    t->is_open = val;
}

void thing_set_is_buried (thingp t, boolean val)
{
    verify(t);

    t->is_buried = val;
}

void thing_set_is_dead (thingp t, boolean val)
{
    verify(t);

    t->is_dead = val;
}

const char * thing_name (thingp t)
{
    verify(t);

    return (thing_template_name(t->thing_template));
}

const char * thing_tooltip (thingp t)
{
    verify(t);

    return (thing_template_get_tooltip(t->thing_template));
}

uint32_t thing_speed (thingp t)
{
    verify(t);

    return (thing_template_get_speed(t->thing_template));
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

void thing_place (void *context)
{
    thing_place_context_t *place;

    place = (typeof(place)) context;

    wid_game_map_server_replace_tile(wid_game_map_server_grid_container,
                                     place->x,
                                     place->y,
                                     place->thing_template);

    if (thing_template_is_key7(place->thing_template)) {
        sound_play_explosion();
    }

    myfree(context);
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

void thing_server_wid_update (thingp t, double x, double y, boolean is_new)
{
    verify(t);

    t->x = x;
    t->y = y;

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
        wid_move_to_abs_in(t->wid, tl.x, tl.y, THING_MONST_SPEED);
    }
}

void thing_client_wid_update (thingp t, double x, double y, boolean smooth)
{
    verify(t);

    t->x = x;
    t->y = y;

    verify(t);

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
        if (thing_is_monst(t)) {
            wid_move_to_abs_in(t->wid, tl.x, tl.y, THING_MONST_SPEED);
        } else {
            wid_move_to_abs_in(t->wid, tl.x, tl.y, 20);
        }
    } else {
        wid_set_tl_br(t->wid, tl, br);
    }
}

void socket_server_tx_map_update (socketp p, tree_rootp tree)
{
    /*
     * Allocate a fresh packet.
     */
    UDPpacket *packet = socket_alloc_msg();
    uint8_t *odata = packet->data;
    uint8_t *data = packet->data;
    *data++ = MSG_MAP_UPDATE;

    /*
     * This is the count of the number of updates we sqeeze into each packet.
     */
    uint16_t packed = 0;

    /*
     * And this is th max per packet.
     */
    static const uint16_t max_pack = 
        (MAX_PACKET_SIZE - sizeof(msg_map_update)) / sizeof(msg_thing_update);

    thingp t;

    TREE_WALK(tree, t) {
        /*
         * If updating to all sockets, decrement the update counter for this 
         * thing. We only send updates on modified things.
         */
        if (!p) {
            if (!t->updated) {
                continue;
            }

            t->updated--;
        }

        uint8_t up = false;
        uint8_t down = false;
        uint8_t left = false;
        uint8_t right = false;

        switch (t->dir) {
        case THING_DIR_LEFT:
            left = true;
            break;
        case THING_DIR_RIGHT:
            right = true;
            break;
        case THING_DIR_UP:
            up = true;
            break;
        case THING_DIR_DOWN:
            down = true;
            break;
        case THING_DIR_TL:
            left = true;
            up = true;
            break;
        case THING_DIR_BL:
            down = true;
            left = true;
            break;
        case THING_DIR_TR:
            up = true;
            right = true;
            break;
        case THING_DIR_BR:
            right = true;
            down = true;
            break;
        }

        uint8_t state = 
                ((up            ? 1 : 0) << THING_STATE_BIT_SHIFT_UP) |
                ((down          ? 1 : 0) << THING_STATE_BIT_SHIFT_DOWN) |
                ((left          ? 1 : 0) << THING_STATE_BIT_SHIFT_LEFT) |
                ((right         ? 1 : 0) << THING_STATE_BIT_SHIFT_RIGHT) |
                ((t->resync     ? 1 : 0) << THING_STATE_BIT_SHIFT_RESYNC) |
                ((t->is_dead    ? 1 : 0) << THING_STATE_BIT_SHIFT_IS_DEAD) |
                ((t->is_buried  ? 1 : 0) << THING_STATE_BIT_SHIFT_IS_BURIED);

        t->resync = 0;

        if (t->is_dead || t->resync || t->is_buried) {
            /*
             * Always send.
             */
        } else if (state == t->last_state) {
            /*
             * No change in direction or state. Only send an update 
             * occasionally.
             */
            if (!time_have_x_hundredths_passed_since(5, t->timestamp_update)) {
                continue;
            }
        } else {
            t->last_state = state;
        }

        t->timestamp_update = time_get_time_cached();

        /*
         * We squeeze the template ID in where we can
         */
        uint16_t template_id = thing_template_to_id(t->thing_template);
        uint16_t thing_id = t->thing_id | ((template_id & 0x0f) << 12);
        uint16_t tx;
        uint16_t ty;

        widp w = thing_wid(t);
        if (w) {
            tx = (t->x * THING_COORD_SCALE);
            ty = (t->y * THING_COORD_SCALE);
        } else {
            tx = -1;
            ty = -1;
        }

        tx &= 0x3fff;
        ty &= 0x3fff;
        tx |= ((template_id & 0x30) >> 4) << 14;
        ty |= ((template_id & 0xc0) >> 4) << 14;

        /*
         * Now pack the raw data.
         */
        *data++ = state;

        SDLNet_Write16(thing_id, data);               
        data += sizeof(uint16_t);

        SDLNet_Write16((uint16_t) tx, data);               
        data += sizeof(uint16_t);
            
        SDLNet_Write16((uint16_t) ty, data);               
        data += sizeof(uint16_t);

        packed++;

        if (packed < max_pack) {
            /*
             * Can fit more in.
             */
            continue;
        }

        /*
         * We reached the limit for this packet? Send now.
         */
        packet->len = data - odata;
        packed = 0;

        /*
         * Broadcast to all clients.
         */
        socketp sp;

        TREE_WALK(sockets, sp) {
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
        *data++ = MSG_MAP_UPDATE;
    }

    /*
     * Any left over, send them now.
     */
    if (packed) {
        socketp sp;

        packet->len = data - odata;

        TREE_WALK(sockets, sp) {
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
    boolean need_fixup = false;
    verify(s);

    uint8_t *eodata = data + packet->len - 1;

    while (data < eodata) {
        uint8_t state = *data++;

        uint16_t thing_id = SDLNet_Read16(data);
        data += sizeof(uint16_t);

        uint16_t tx = SDLNet_Read16(data);
        data += sizeof(uint16_t);

        uint16_t ty = SDLNet_Read16(data);
        data += sizeof(uint16_t);

        /*
         * We squeeze the template ID in where we can
         */
        uint8_t template_id = 
                (thing_id >> 12) | ((tx >> 14) << 4) | ((ty >> 14) << 6);
        thing_id &= 0x0fff;
        tx &= 0x3fff;
        ty &= 0x3fff;

        boolean on_map;
        double x;
        double y;

        if ((tx == 0x3fff) || (y == 0x3fff)) {
            on_map = false;
            x = -1;
            y = -1;
        } else{
            on_map = true;
            x = ((double)tx) / THING_COORD_SCALE;
            y = ((double)ty) / THING_COORD_SCALE;
        }

        thingp t = thing_client_find(thing_id);
        if (!t) {
            thing_templatep thing_template = 
                    id_to_thing_template(template_id);

            t = thing_client_new(thing_id, thing_template);

            need_fixup = need_fixup ||
                thing_template_is_wall(thing_template) ||
                thing_template_is_pipe(thing_template) ||
                thing_template_is_door(thing_template);
        }

        uint8_t up =    (state & (1 << THING_STATE_BIT_SHIFT_UP))    ? 1 : 0;
        uint8_t down =  (state & (1 << THING_STATE_BIT_SHIFT_DOWN))  ? 1 : 0;
        uint8_t left =  (state & (1 << THING_STATE_BIT_SHIFT_LEFT))  ? 1 : 0;
        uint8_t right = (state & (1 << THING_STATE_BIT_SHIFT_RIGHT)) ? 1 : 0;

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

        if (on_map) {
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
                        THING_LOG(t, "%s server asked for resync",
                                  t->logname);
                        THING_LOG(t, "  server %f %f", t->x, t->y);
                        THING_LOG(t, "  client %f %f", x, y);

                        t->x = x;
                        t->y = y;

                        thing_client_wid_update(t, x, y, true /* smooth */);
                    } else 
                        if ((fabs(x - t->x) > THING_MAX_SERVER_DISCREPANCY * 2) ||
                            (fabs(y - t->y) > THING_MAX_SERVER_DISCREPANCY * 2)) {
                        /*
                         * Check we are roughly where the server thinks we 
                         * are. If wildly out of whack, correct our viewpoint.
                         */
                        THING_LOG(t, "%s out of sync with server, correcting ",
                                  t->logname);
                        THING_LOG(t, "  server %f %f", t->x, t->y);
                        THING_LOG(t, "  client %f %f", x, y);

                        t->x = x;
                        t->y = y;

                        thing_client_wid_update(t, x, y, true /* smooth */);
                    }
                } else {
                    thing_client_wid_update(t, x, y, true /* smooth */);
                }
            } else {
                if (t->is_dead) {
                    /*
                     * Already dead? No new tile.
                     */
                } else {
                    wid_game_map_client_replace_tile(
                                            wid_game_map_client_grid_container,
                                            x, y, t);
                }
            }
        }

        if (state & (1 << THING_STATE_BIT_SHIFT_IS_DEAD)) {
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
    /*
     * Allocate a fresh packet.
     */
    UDPpacket *packet = socket_alloc_msg();
    uint8_t *odata = packet->data;
    uint8_t *data = packet->data;

    *data++ = MSG_PLAYER_UPDATE;

    SDLNet_Write32(t->tree.key, data);               
    data += sizeof(uint32_t);

    memcpy(data, t->carrying, sizeof(t->carrying));
    data += sizeof(t->carrying);

    packet->len = data - odata;

    /*
     * Broadcast to all clients.
     */
    socketp sp;

    TREE_WALK(sockets, sp) {
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

    uint8_t *eodata = data + packet->len - 1;

    uint32_t thing_id = SDLNet_Read32(data);
    data += sizeof(uint32_t);

    thingp t = thing_client_find(thing_id);
    if (!t) {
        ERR("thing id from server, id %u not found", thing_id);
        return;
    }

    memcpy(t->carrying, data, eodata - data);
}

static void thing_common_move (thingp t,
                               double *x,
                               double *y,
                               boolean up,
                               boolean down,
                               boolean left,
                               boolean right)
{
    double ox = t->x;
    double oy = t->y;

    if (*x < 0) {
        *x = 0;
    }

    if (*y < 0) {
        *y = 0;
    }

    if (*x > TILES_MAP_WIDTH - 1) {
        *x = TILES_MAP_WIDTH - 1;
    }

    if (*y > TILES_MAP_HEIGHT - 1) {
        *y = TILES_MAP_HEIGHT - 1;
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
                        const boolean up,
                        const boolean down,
                        const boolean left,
                        const boolean right)
{
    widp grid = wid_game_map_client_grid_container;

    if (thing_hit_solid_obstacle(grid, t, x, y)) {
        if (!thing_hit_solid_obstacle(grid, t, x, t->y)) {
            y = t->y;
        } else if (!thing_hit_solid_obstacle(grid, t, t->x, y)) {
            x = t->x;
        } else {
            return;
        }
    }

    thing_common_move(t, &x, &y, up, down, left, right);

    /*
     * Oddly doing smooth moving makes it more jumpy when scrolling.
     */
    thing_client_wid_update(t, x, y, false);

    socket_tx_client_move(client_joined_server, t, up, down, left, right);
}

void thing_server_move (thingp t,
                        double x,
                        double y,
                        const boolean up,
                        const boolean down,
                        const boolean left,
                        const boolean right)
{
    widp grid = wid_game_map_server_grid_container;

    thing_common_move(t, &x, &y, up, down, left, right);

    if (thing_hit_solid_obstacle(grid, t, x, y)) {
        if (!thing_hit_solid_obstacle(grid, t, x, t->y)) {
            y = t->y;
        } else if (!thing_hit_solid_obstacle(grid, t, t->x, y)) {
            x = t->x;
        } else {
            if (thing_is_player(t)) {
                THING_LOG(t, "client collision, ignore move");
                THING_LOG(t, "  server %f %f", t->x, t->y);
                THING_LOG(t, "  client %f %f", x, y);
            }
            return;
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

            t->updated++;
            t->resync = 1;
            return;
        }
    }

    thing_common_move(t, &x, &y, up, down, left, right);

    thing_server_wid_update(t, x, y, false /* is_new */);
    t->updated++;

    thing_handle_collisions(wid_game_map_server_grid_container, t);
}

void thing_collect (thingp t, thing_templatep tmp)
{
    uint32_t id;

    id = thing_template_to_id(tmp);

    THING_LOG(t, "collects %s", thing_template_name(tmp));

    t->carrying[id]++;

    socket_server_tx_player_update(t);
}

boolean thing_has (thingp t, uint32_t id)
{
    if (!t->carrying[id]) {
        return (false);
    }

    return (true);
}
