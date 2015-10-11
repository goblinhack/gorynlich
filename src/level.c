/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>
#include <errno.h>
#include "slre.h"

#include "main.h"
#include "string_util.h"
#include "wid.h"
#include "level.h"
#include "thing.h"
#include "command.h"
#include "time_util.h"
#include "marshal.h"
#include "wid_game_map_server.h"
#include "wid_game_map_client.h"
#include "wid_map.h"
#include "map.h"
#include "timer.h"
#include "sound.h"
#include "wid_editor.h"
#include "socket_util.h"
#include "map_jigsaw.h"
#include "file.h"
#include "map.h"
#include "thing_shop.h"

static uint8_t level_init_done;
static uint8_t level_server_init_done;
static void level_reset_players(levelp level);
static void level_set_walls(levelp level);
static void level_update_incremental(levelp level, int force);

uint8_t level_init (void)
{
    if (level_init_done) {
        return (true);
    }

    level_init_done = true;

    return (true);
}

static uint8_t level_server_side_one_time_init (void)
{
    if (level_server_init_done) {
        return (true);
    }

    level_server_init_done = true;

    return (true);
}

void level_fini (void)
{
    if (level_init_done) {
        level_init_done = false;
    }

    if (server_timers) {
        ERR("server timers still set on level fini");
    }

    if (client_timers) {
        ERR("client timers still set on level fini");
    }
}

levelp level_new (widp map, 
                  level_pos_t level_pos, 
                  int is_editor, 
                  int is_map_editor, 
                  int on_server)
{
    levelp level;

    if (is_map_editor) {
        /*
         * Loading 100 levels takes time, so only do partial memset
         */
        level = (typeof(level)) mymalloc(sizeof(*level), "level");

        memset(&level->memzero_start, 0, (char*)&level->memzero_end - (char*)&level->memzero_start);
    } else {
        level = (typeof(level)) myzalloc(sizeof(*level), "level");
    }

    if (map) {
        level_set_map(level, map);
    }

    if ((level_pos.x == 66) && (level_pos.y == 66)) {
        level->is_test_level = true;
    }

    level_set_level_pos(level, level_pos);
    level_set_is_editor(level, is_editor);
    level_set_is_map_editor(level, is_map_editor);

    if (on_server) {
        level->logname = dynprintf("%d.%d[%p] (server)", 
                                   level_pos.y, level_pos.x, level);
    } else {
        level->logname = dynprintf("%d.%d[%p] (client)", 
                                   level_pos.y, level_pos.x, level);
    }

    level->on_server = on_server;

    LEVEL_LOG(level, "created");

    return (level);
}

void level_destroy (levelp *plevel, uint8_t keep_players)
{
    levelp level;

    if (!plevel) {
        ERR("no plevel");
    }

    level = *plevel;
    if (!level) {
        ERR("no level");
    }

    /*
     * Perhaps another thread is still using this level for map generation?
     */
    level->exit_request = 1;
    while (level->locked) {
        LOG("Level locked... waiting on AI thread to finish");
	SDL_Delay(1000);
    }
    level->exit_request = 0;

    /*
     * Kill all humans!
     */
    if ((level == client_level) || (level == server_level)) {
        things_level_destroyed(level, keep_players);
    }

    LEVEL_LOG(level, "destroy");

    if (level->title) {
        myfree((void*) level->title);
        level->title = 0;
    }

    if (level->destroy_reason) {
        myfree((void*) level->destroy_reason);
        level->destroy_reason = 0;
    }

    LEVEL_LOG(level, "destroyed");

    if (level->logname) {
        myfree((void*) level->logname);
        level->logname = 0;
    }

    /*
     * Ensure no stale pointers.
     */
    if (level == client_level) {
        if (client_timers) {
            action_timers_destroy(&client_timers);
        }

        client_level = 0;
    }

    if (level == server_level) {
        if (server_timers) {
            action_timers_destroy(&server_timers);
        }

        server_level = 0;
    }

    *plevel = 0;

    myfree(level);
}

static void level_update_slow (levelp level)
{
    map_fixup(level);

    shop_fixup();

    level_set_walls(level);

    /*
     * One time generate of expensive wander map
     */
    dmap_generate_map_wander(level);

    level_update_incremental(level, true /* force */);

    /*
     * Ensure things are updated at the start of the new level.
     */
    thing_tick_server_player_slow_all(true /* force */);
}

static void level_update_incremental (levelp level, int force)
{
    level_set_walls(level);

    /*
     * Regenerate player dmaps as things like doors may have been opened.
     */
    dmap_generate(level, force);
}

void level_load_new (void)
{
    LOG("Server: New level, %d.%d", 
        global_config.server_level_pos.y, 
        global_config.server_level_pos.x);

    char *tmp = dynprintf("%s%d.%d", LEVELS_PATH, 
                          global_config.server_level_pos.y, 
                          global_config.server_level_pos.x);

    /*
     * Mostly random levels.
     */
    int fixed = false;

    if ((global_config.server_level_pos.x == 66) && 
        (global_config.server_level_pos.y == 66)) {
        fixed = true;
    } else if ((global_config.server_level_pos.x == 2) && 
               (global_config.server_level_pos.y == 2)) {
        fixed = true;
    } else if ((global_config.server_level_pos.x == 3) && 
               (global_config.server_level_pos.y == 3)) {
        fixed = true;
    } else if ((global_config.server_level_pos.x == 4) && 
               (global_config.server_level_pos.y == 4)) {
        fixed = true;
    } else if ((global_config.server_level_pos.x == 5) && 
               (global_config.server_level_pos.y == 5)) {
        fixed = true;
    } else if ((global_config.server_level_pos.x == 6) && 
               (global_config.server_level_pos.y == 6)) {
        fixed = true;
    } else if ((global_config.server_level_pos.x == 7) && 
               (global_config.server_level_pos.y == 7)) {
        fixed = true;
    } else if ((global_config.server_level_pos.x == 8) && 
               (global_config.server_level_pos.y == 8)) {
        fixed = true;
    } else if ((global_config.server_level_pos.x == 9) && 
               (global_config.server_level_pos.y == 9)) {
        fixed = true;
    } else if ((global_config.server_level_pos.x == 10) && 
               (global_config.server_level_pos.y == 10)) {
        fixed = true;
    } else if ((global_config.server_level_pos.x == 7) && 
               (global_config.server_level_pos.y == 10)) {
        /*
         * squidling
         */
        fixed = true;
    } else if ((global_config.server_level_pos.x == 4) && 
               (global_config.server_level_pos.y == 5)) {
        /*
         * demon lair
         */
        fixed = true;
    }

    int r = (myrand() % 100);
    if ((r < 20) && file_exists(tmp)) {
        fixed = true;
    }

    if (fixed) {
        server_level = level_load(global_config.server_level_pos,
                                  wid_game_map_server_grid_container,
                                  false /* is_editor */,
                                  false /* is_map_editor */,
                                  true /* server */);
    } else {
        LOG("Level %s does not exist, create random level", tmp);

        server_level = level_load_random(global_config.server_level_pos,
                                         wid_game_map_server_grid_container,
                                         false /* is_editor */,
                                         false /* is_map_editor */,
                                         true /* server */);
    }

    myfree(tmp);

    if (!server_level) {
        ERR("failed to load server level %d.%d",
            global_config.server_level_pos.y,
            global_config.server_level_pos.x);
    }

    level_update_slow(server_level);

    level_pause(server_level);
}

/*
 * Routines done for both random and static levels.
 */
static void level_loaded_common (levelp level)
{
    LEVEL_LOG(level, "level loaded");

    /*
     * Activate any triggers that have no activators.
     */
    if (!level->is_editor && !level->is_map_editor) {
        level_trigger_activate_default_triggers(level);
    }
}

levelp level_load (level_pos_t level_pos, 
                   widp wid, 
                   int is_editor,
                   int is_map_editor,
                   int on_server)
{
    levelp level;

    level_server_side_one_time_init();

    level = level_new(wid, level_pos, is_editor, is_map_editor, on_server);

    level_set_is_paused(level, true);
    level_set_timestamp_started(level, time_get_time_ms());

    char *dir_and_file;

    dir_and_file = dynprintf("%s%d.%d", LEVELS_PATH, level_pos.y, level_pos.x);

    if (!file_exists(dir_and_file)) {
        myfree(dir_and_file);

        dir_and_file = dynprintf("data/levels/%d.%d", level_pos.y, level_pos.x);
    }

    LEVEL_LOG(level, "Level %s: loading", dir_and_file);

    int pass;
    int max_pass;

    if (level_is_map_editor(level) || level_is_editor(level)) {
        max_pass = 1;
    } else {
        /*
         * Need 2 passes for levels being read into the game. First pass is 
         * just to learn where the action triggers are for spawned things and 
         * the like.
         *
         * Second pass creates the things.
         */
        max_pass = 2;
    }

    for (pass = 0; pass < max_pass; pass++) {
        demarshal_p in;

        server_level_is_being_loaded = pass + 1;

        if (!(in = demarshal(dir_and_file))) {
            /*
             * Fail
             *
             * I'm assuming this means the end of the game
             *
            char *popup_str = dynprintf("Failed to load level %s: %s",
                                        dir_and_file,
                                        strerror(errno));

            MSG_BOX("%s", popup_str);
            myfree(popup_str);
             */
            myfree(dir_and_file);

            return (0);
        } else {
            /*
             * Success
             */
            if (!demarshal_level(in, level)) {
                char *popup_str = dynprintf("There were some errors "
                                            "while loading level %s: %s",
                                            dir_and_file,
                                            strerror(errno));

                MSG_BOX("%s", popup_str);
                myfree(popup_str);
            }

            demarshal_fini(in);
        }
    }

    server_level_is_being_loaded = 0;

    myfree(dir_and_file);

    if (!level_is_map_editor(level) &&
        !level_is_editor(level)) {
        level_update_slow(level);
    }

    level_set_is_paused(level, false);

    if (!level_is_map_editor(level) &&
        !level_is_editor(level)) {
        level_reset_players(level);
    }

    level_loaded_common(level);

    return (level);
}

levelp level_load_random (level_pos_t level_pos, 
                          widp wid, 
                          int is_editor,
                          int is_map_editor,
                          int on_server)
{
    levelp level;

    level_server_side_one_time_init();

    level = level_new(wid, 
                      level_pos, 
                      is_editor, 
                      is_map_editor, 
                      on_server);

    level_set_is_paused(level, true);
    level_set_timestamp_started(level, time_get_time_ms());

    LEVEL_LOG(level, "Level generating");

    int depth = ((level_pos.y - 1) * LEVELS_ACROSS) + level_pos.x;

    map_jigsaw_generate(level, wid, depth,
                        wid_game_map_server_replace_tile);

    level_update_slow(level);

    level_set_is_paused(level, false);
    level_reset_players(level);

    level_loaded_common(level);

    return (level);
}

const char *level_get_title (levelp level)
{
    verify(level);

    return (level->title);
}

void level_set_title (levelp level, const char *val)
{
    verify(level);

    if (level->title) {
        myfree(level->title);
    }

    level->title = dupstr(val, "level title");
}

const char *level_get_destroy_reason (levelp level)
{
    verify(level);

    return (level->destroy_reason);
}

void level_set_destroy_reason (levelp level, const char *val)
{
    verify(level);

    if (level->destroy_reason) {
        myfree(level->destroy_reason);
    }

    level->destroy_reason = dupstr(val, "level destroy_reason");
}

widp level_get_map (levelp level)
{
    verify(level);

    return (level->map);
}

void level_set_map (levelp level, widp wid)
{
    verify(level);

    level->map = wid;

    wid_set_client_context(wid, level);
}

static void level_set_walls (levelp level)
{
    int32_t x;
    int32_t y;
    int32_t z;

    widp w = wid_game_map_server_grid_container;

    memset(level->map_player_target_treat_doors_as_walls.walls, ' ',
           sizeof(level->map_player_target_treat_doors_as_walls.walls));
    memset(level->map_player_target_treat_doors_as_passable.walls, ' ',
           sizeof(level->map_player_target_treat_doors_as_passable.walls));

    memset(level->map_player_target_treat_levitating_over_and_doors_as_walls.walls, ' ',
           sizeof(level->map_player_target_treat_levitating_over_and_doors_as_walls.walls));
    memset(level->map_player_target_treat_levitating_over_and_doors_as_passable.walls, ' ',
           sizeof(level->map_player_target_treat_levitating_over_and_doors_as_passable.walls));

    memset(level->map_treasure_target_treat_doors_as_walls.walls, ' ',
           sizeof(level->map_treasure_target_treat_doors_as_walls.walls));
    memset(level->map_treasure_target_treat_doors_as_passable.walls, ' ',
           sizeof(level->map_treasure_target_treat_doors_as_passable.walls));

    memset(level->walls.walls, ' ',
           sizeof(level->walls.walls));
    memset(level->doors.walls, ' ',
           sizeof(level->doors.walls));

    for (z = MAP_DEPTH_WALL; z < MAP_DEPTH_OBJ_TOP; z++) {
        for (x = 0; x < MAP_WIDTH; x++) {
            for (y = 0; y < MAP_HEIGHT; y++) {
                tree_root **tree = 
                    w->grid->grid_of_trees[z] + (y * w->grid->width) + x;
                widgridnode *node;

                TREE_WALK_REVERSE_UNSAFE_INLINE(
                                    *tree, 
                                    node,
                                    tree_prev_tree_wid_compare_func) {

                    widp w = node->wid;

                    thingp t = wid_get_thing(w);
                    if (!t) {
                        continue;
                    }

                    /*
                     * Identify obstacles.
                     */
                    if (thing_is_wall(t)        ||
                        thing_is_rock(t)        ||
                        thing_is_door(t)        ||
                        thing_is_mob_spawner(t) ||
                        thing_is_teleport(t)    ||
                        thing_is_lava(t)        ||
                        thing_is_acid(t)        ||
                        thing_is_exit(t)) {
                        level->map_player_target_treat_doors_as_walls.walls[x][y] = '+';

                        level->map_treasure_target_treat_doors_as_walls.walls[x][y] = '+';
                    }

                    /*
                     * Same as above, but treat doors as passable.
                     */
                    if (thing_is_wall(t)        ||
                        thing_is_rock(t)        ||
                        thing_is_mob_spawner(t) ||
                        thing_is_teleport(t)    ||
                        thing_is_lava(t)        ||
                        thing_is_acid(t)        ||
                        thing_is_exit(t)) {
                        level->map_player_target_treat_doors_as_walls.walls[x][y] = '+';
                        level->map_player_target_treat_doors_as_passable.walls[x][y] = '+';

                        level->map_treasure_target_treat_doors_as_passable.walls[x][y] = '+';
                    }

                    /*
                     * Identify obstacles.
                     */
                    if (thing_is_wall(t)        ||
                        thing_is_rock(t)        ||
                        thing_is_door(t)        ||
                        thing_is_teleport(t)) {
                        level->map_player_target_treat_levitating_over_and_doors_as_walls.walls[x][y] = '+';
                    }

                    /*
                     * Same as above, but treat doors as passable.
                     */
                    if (thing_is_wall(t)        ||
                        thing_is_rock(t)        ||
                        thing_is_teleport(t)) {
                        level->map_player_target_treat_levitating_over_and_doors_as_walls.walls[x][y] = '+';
                        level->map_player_target_treat_levitating_over_and_doors_as_passable.walls[x][y] = '+';
                    }

                    if (thing_is_wall(t) ||
                        thing_is_rock(t) ||
                        thing_is_door(t)) {
                        level->doors.walls[x][y] = '+';
                    }
                }
            }
        }
    }
}

/*
 * New level; update players.
 */
void level_reset_players (levelp level)
{
    thingp t;

    /*
     * Rewield weapons at start.
     */
    LEVEL_LOG(level, "Reset players at start of level");

    TREE_WALK(server_active_things, t) {
        if (thing_is_player(t)) {
            tpp weapon = thing_weapon(t);
            if (weapon) {
                thing_wield(t, weapon);
                thing_update(t);
            }

            tpp shield = thing_shield(t);
            if (shield) {
                thing_wield_shield(t, shield);
                thing_update(t);
            }

            /*
             * Need this else we are in darkness at level start
             */
            thing_torch_update_count(t, true);
        }
    }
}

static void level_action_timer_unpause_level (void *context)
{
    levelp level;
    level = (typeof(level)) context;
    verify(level);

    level->pause_timer = 0;

    level_set_is_paused(level, false);
}

/*
 * Freeze all things for a short delay
 */
void level_pause (levelp level)
{
    if (!level->pause_timer) {

        static const char *messages[] = {
            "Go my little one!",
            "Meet your doom...",
            "Prepare for the end...",
            "Your end is nigh...",
            "Go forth!",
            "Make it so!",
            "Make it thusly!",
            "Get ready!",
        };

        MSG_SERVER_SHOUT_AT_ALL_PLAYERS(POPUP,
                                        0, 0, // x, y
                                        "%s", messages[myrand() % ARRAY_SIZE(messages)]);

        MSG_SERVER_SHOUT_AT_ALL_PLAYERS(INFO, 
                                       0, 0,
                                       "Press m or X button to use magic");

        if (level_has_shop(level)) {
            shop_on_level(0, 0);
        }

        level->pause_timer = 
                    action_timer_create(&server_timers,
                                        level_action_timer_unpause_level,
                                        0,
                                        level,
                                        "unpause level",
                                        ONESEC * 2, /* duration */
                                        0);

        level_set_is_paused(level, true);
    }
}

/*
 * Timer has fired indicating we should fade the level out.
 */
static void level_action_timer_end_level_first_phase_fade_out (void *context)
{
    levelp level;
    level = (typeof(level)) context;
    verify(level);

    level_set_is_ready_to_fade_out(level, true);
}

/*
 * Timer has fired indicating end the level now.
 */
static void level_action_timer_end_level_second_phase_destroy (void *context)
{
    levelp level;
    level = (typeof(level)) context;
    verify(level);

    level_set_is_ready_to_be_destroyed(level, true);
}

/*
 * Clean up the level. It's over! the exit was reached and a delay passed to 
 * warn the other players.
 */
static void level_finished (levelp level)
{
    level->end_level_second_phase_destroy_timer = 0;
    level->end_level_first_phase_fade_out_timer = 0;

    verify(level);

    /*
     * Is this needed ? Kind of obvious and overlaps with spending points 
     * update often.
     */
    LEVEL_LOG(level, "level finished");

    if (level->is_test_level) {
        LEVEL_LOG(level, "test level finished");
        level_destroy(&server_level, false /* keep players */);
        return;
    }

    thingp t;

    /*
     * Force the death of all things on the level.
     */
    { TREE_WALK(server_active_things, t) {
        if (!thing_is_player(t) &&
            !thing_is_shield_carry_anim(t)) {
            thing_leave_level(t);
            thing_set_is_dead(t, true);
            thing_update(t);
        }
    } }

    { TREE_WALK(server_boring_things, t) {
        thing_leave_level(t);
        thing_set_is_dead(t, true);
        thing_update(t);
    } }

    socket_server_tx_map_update(0, server_boring_things,
                                "level destroy boring things");
    socket_server_tx_map_update(0, server_active_things,
                                "level destroy active things");

    int game_over = level->game_over;

    wid_game_map_server_wid_destroy(true /* keep players */);

    level = 0;

    { TREE_WALK(server_active_things, t) {
        if (!thing_is_player_or_owned_by_player(t)) {
            ERR("players should be all that is left by now "
                "but we have %s", thing_logname(t));
            continue;
        }
    } }

    /*
     * LEVEL CAN BE NULL HERE
     */

    if (game_over) {
        return;
    }

    wid_game_map_server_wid_create();

    /*
     * Create the new level, random or otherwise.
     */
    level_load_new();

    /*
     * Move players to the new level.
     */
    { TREE_WALK(server_active_things, t) {
        if (!thing_is_player(t)) {
            continue;
        }

        thing_map_remove(t);

        wid_game_map_server_replace_tile(
                wid_game_map_server_grid_container,
                0, 0,
                t,
                t->tp,
                0 /* tpp_data */,
                0 /* item */,
                0 /* stats */);

        thing_join_level(t);
    } }

    socket_server_tx_map_update(0, server_boring_things,
                                "new level boring things");
    socket_server_tx_map_update(0, server_active_things,
                                "new level active things");

    level_update_slow(server_level);
}

void level_server_tick (levelp level)
{
    if (!level) {
        return;
    }

    verify(level);

    static uint32_t ts;

    if (!time_have_x_tenths_passed_since(1, ts)) {
        return;
    }

    ts = time_get_time_ms();

    /*
     * If the player has finished the level then popup a notice so all players 
     * know the end is nigh!
     */
    if (level_exit_has_been_reached(level)) {
        if (!level->end_level_first_phase_fade_out_timer) {
            if (level->game_over) {
                MSG_SERVER_SHOUT_AT_ALL_PLAYERS(POPUP,
                                                0, 0, // x, y
                                                "Quest completed!");
            } else {
#if 0
                /*
                 * We play a sound and this overlaps the spending point 
                 * awards, so ignore it.
                 */
                MSG_SERVER_SHOUT_AT_ALL_PLAYERS(POPUP,
                                                0, 0, // x, y
                                                "Level completed");
#endif
            }

            level->end_level_first_phase_fade_out_timer = 
                action_timer_create(
                            &server_timers,
                            level_action_timer_end_level_first_phase_fade_out,
                            0,
                            level,
                            "end level",
                            DELAY_LEVEL_END_HIDE,
                            ONESEC);

            level->end_level_second_phase_destroy_timer = 
                action_timer_create(
                            &server_timers,
                            level_action_timer_end_level_second_phase_destroy,
                            0,
                            level,
                            "end level",
                            DELAY_LEVEL_END_DESTROY,
                            ONESEC);
        }
    }

    /*
     * If the level is completely done, i.e. we popped up a notice and timed 
     * out and are ready to delete it, zap it now.
     */
    if (level_is_ready_to_be_destroyed(level)) {
        level_finished(level);
        return;
    }

    /*
     * Every now and again cause the ghosts to look at the level afresh so 
     * that if walls move then they now look through the gaps.
     */
    if (level_needs_updating(level)) {
        level_set_needs_updating(server_level, false);

        /*
         * For things like walls removed
         */
        level_update_incremental(level, true);
    } else {
        /*
         * Can avoid doing player dmap calc if player not moved
         */
        level_update_incremental(level, false);
    }
}

void level_client_tick (levelp level)
{
    if (!level) {
        return;
    }

    /*
     * Allow the first fixup to happen immediately and the rest deferred.
     */
    if (!level_needs_fixup(level)) {
        return;
    }

    if (!time_have_x_tenths_passed_since(1, level->timestamp_last_fixup)) {
        return;
    }

    map_fixup(level);

    level_set_needs_fixup(level, false);

    level->timestamp_last_fixup = time_get_time_ms();
}

level_pos_t level_get_level_pos (levelp level)
{
    verify(level);

    return (level->level_pos);
}

void level_set_level_pos (levelp level, level_pos_t val)
{
    verify(level);

    level->level_pos = val;
}

uint32_t level_get_seed (levelp level)
{
    verify(level);

    return (level->seed);
}

void level_set_seed (levelp level, uint32_t val)
{
    verify(level);

    level->seed = val;
}

uint32_t level_get_timestamp_started (levelp level)
{
    verify(level);

    return (level->timestamp_started);
}

void level_set_timestamp_started (levelp level, uint32_t val)
{
    verify(level);

    level->timestamp_started = val;
}

uint8_t level_needs_updating (levelp level)
{
    verify(level);

    return (level->needs_updating);
}

void level_set_needs_updating (levelp level, uint8_t val)
{
    verify(level);

    level->needs_updating = val;
}

uint8_t level_death_is_coming (levelp level)
{
    verify(level);

    return (level->death_is_coming);
}

void level_set_death_is_coming (levelp level, uint8_t val)
{
    verify(level);

    level->death_is_coming = val;
}

uint8_t level_death_is_coming_soon (levelp level)
{
    verify(level);

    return (level->death_is_coming_soon);
}

void level_set_death_is_coming_soon (levelp level, uint8_t val)
{
    verify(level);

    level->death_is_coming_soon = val;
}

uint8_t level_is_zzz1 (levelp level)
{
    verify(level);

    return (level->is_zzz1);
}

void level_set_is_zzz1 (levelp level, uint8_t val)
{
    verify(level);

    level->is_zzz1 = val;
}

uint8_t level_is_zzz2 (levelp level)
{
    verify(level);

    return (level->is_zzz2);
}

void level_set_is_zzz2 (levelp level, uint8_t val)
{
    verify(level);

    level->is_zzz2 = val;
}

uint8_t level_is_zzz3 (levelp level)
{
    verify(level);

    return (level->is_zzz3);
}

void level_set_is_zzz3 (levelp level, uint8_t val)
{
    verify(level);

    level->is_zzz3 = val;
}

uint8_t level_is_zzz4 (levelp level)
{
    verify(level);

    return (level->is_zzz4);
}

void level_set_is_zzz4 (levelp level, uint8_t val)
{
    verify(level);

    level->is_zzz4 = val;
}

uint8_t level_is_zzz5 (levelp level)
{
    verify(level);

    return (level->is_zzz5);
}

void level_set_is_zzz5 (levelp level, uint8_t val)
{
    verify(level);

    level->is_zzz5 = val;
}

uint8_t level_is_zzz6 (levelp level)
{
    verify(level);

    return (level->is_zzz6);
}

void level_set_is_zzz6 (levelp level, uint8_t val)
{
    verify(level);

    level->is_zzz6 = val;
}

uint8_t level_is_zzz7 (levelp level)
{
    verify(level);

    return (level->is_zzz7);
}

void level_set_is_zzz7 (levelp level, uint8_t val)
{
    verify(level);

    level->is_zzz7 = val;
}

uint8_t level_is_zzz8 (levelp level)
{
    verify(level);

    return (level->is_zzz8);
}

void level_set_is_zzz8 (levelp level, uint8_t val)
{
    verify(level);

    level->is_zzz8 = val;
}

uint8_t level_is_zzz9 (levelp level)
{
    verify(level);

    return (level->is_zzz9);
}

void level_set_is_zzz9 (levelp level, uint8_t val)
{
    verify(level);

    level->is_zzz9 = val;
}

uint8_t level_is_zzz10 (levelp level)
{
    verify(level);

    return (level->is_zzz10);
}

void level_set_is_zzz10 (levelp level, uint8_t val)
{
    verify(level);

    level->is_zzz10 = val;
}

uint8_t level_is_zzz11 (levelp level)
{
    verify(level);

    return (level->is_zzz11);
}

void level_set_is_zzz11 (levelp level, uint8_t val)
{
    verify(level);

    level->is_zzz11 = val;
}

uint8_t level_is_zzz12 (levelp level)
{
    verify(level);

    return (level->is_zzz12);
}

void level_set_is_zzz12 (levelp level, uint8_t val)
{
    verify(level);

    level->is_zzz12 = val;
}

uint8_t level_is_zzz13 (levelp level)
{
    verify(level);

    return (level->is_zzz13);
}

void level_set_is_zzz13 (levelp level, uint8_t val)
{
    verify(level);

    level->is_zzz13 = val;
}

uint8_t level_zzz16 (levelp level)
{
    verify(level);

    return (level->zzz16);
}

void level_set_zzz16 (levelp level, uint8_t val)
{
    verify(level);

    level->zzz16 = val;
}

uint8_t level_is_death_summoned (levelp level)
{
    verify(level);

    return (level->is_death_summoned);
}

void level_set_is_death_summoned (levelp level, uint8_t val)
{
    verify(level);

    level->is_death_summoned = val;
}

uint8_t level_is_jesus_summoned (levelp level)
{
    verify(level);

    return (level->is_jesus_summoned);
}

void level_set_is_jesus_summoned (levelp level, uint8_t val)
{
    verify(level);

    level->is_jesus_summoned = val;
}

uint8_t level_is_blit_y_offset (levelp level)
{
    verify(level);

    return (level->is_blit_y_offset);
}

void level_set_is_blit_y_offset (levelp level, uint8_t val)
{
    verify(level);

    level->is_blit_y_offset = val;
}

uint8_t level_exit_reached_when_open (levelp level)
{
    verify(level);

    return (level->exit_reached_when_open);
}

void level_set_exit_reached_when_open (levelp level, uint8_t val)
{
    verify(level);

    level->exit_reached_when_open = val;
}

uint8_t level_get_warned_exit_not_open (levelp level)
{
    verify(level);

    return (level->warned_exit_not_open);
}

void level_set_warned_exit_not_open (levelp level, uint8_t val)
{
    verify(level);

    level->warned_exit_not_open = val;
}

uint8_t level_is_exit_open (levelp level)
{
    verify(level);

    return (level->is_exit_open);
}

void level_set_is_exit_open (levelp level, uint8_t val)
{
    verify(level);

    level->is_exit_open = val;
}

uint8_t level_is_editor (levelp level)
{
    verify(level);

    return (level->is_editor);
}

void level_set_is_editor (levelp level, uint8_t val)
{
    verify(level);

    level->is_editor = val;
}

uint8_t level_is_map_editor (levelp level)
{
    verify(level);

    return (level->is_map_editor);
}

void level_set_is_map_editor (levelp level, uint8_t val)
{
    verify(level);

    level->is_map_editor = val;
}

uint8_t level_is_paused (levelp level)
{
    verify(level);

    return (level->is_paused > 0);
}

void level_set_is_paused (levelp level, uint8_t val)
{
    verify(level);

    if (val) {
        level->is_paused++;
    } else {
        if (level->is_paused) {
            level->is_paused--;
        }
    }
}

uint8_t level_has_shop (levelp level)
{
    verify(level);

    return (level->has_shop > 0);
}

void level_set_has_shop (levelp level, uint8_t val)
{
    verify(level);

    if (val) {
        level->has_shop++;
    } else {
        if (level->has_shop) {
            level->has_shop--;
        }
    }
}

uint8_t level_exit_has_been_reached (levelp level)
{
    verify(level);

    return (level->exit_has_been_reached);
}

void level_set_exit_has_been_reached (levelp level, uint8_t val)
{
    verify(level);

    if ((level->level_pos.x >= 10) && (level->level_pos.y >= 10)) {
        level->game_over = true;
        thingp t;
        { TREE_WALK(server_active_things, t) {
            if (level->is_test_level) {
                /*
                 * No hiscores in the test level!
                 */
                continue;
            }

            if (level->game_over) {
                if (thing_is_player(t)) {
                    thing_game_over(t);
                }
            }
        } }
    }
    
    level->exit_has_been_reached = val;
}

uint8_t level_needs_fixup (levelp level)
{
    verify(level);

    return (level->needs_fixup ? 1 : 0);
}

void level_set_needs_fixup (levelp level, uint8_t val)
{
    verify(level);

    level->needs_fixup = val;
}

uint8_t level_is_ready_to_be_destroyed (levelp level)
{
    verify(level);

    return (level->is_ready_to_be_destroyed);
}

void level_set_is_ready_to_be_destroyed (levelp level, uint8_t val)
{
    verify(level);

    level->is_ready_to_be_destroyed = val;
}

uint8_t level_is_ready_to_fade_out (levelp level)
{
    verify(level);

    return (level->is_ready_to_fade_out);
}

void level_set_is_ready_to_fade_out (levelp level, uint8_t val)
{
    verify(level);

    level->is_ready_to_fade_out = val;
}

const char *level_get_logname (levelp l)
{
    verify(l);

    return (l->logname);
}

static uint32_t level_count_is_x (levelp level, map_is_at_callback callback)
{
    uint32_t count;
    int32_t x;
    int32_t y;

    count = 0;

    for (x = 0; x < MAP_WIDTH; x++) {
        for (y = 0; y < MAP_HEIGHT; y++) {
            count += map_count_x_at(level, x, y, callback);
        }
    }

    return (count);
}

uint32_t level_count_is_player (levelp level)
{
    return (level_count_is_x(level, tp_is_player));
}

uint32_t level_count_is_monst (levelp level)
{
    return (level_count_is_x(level, tp_is_monst));
}

uint32_t level_count_is_wall (levelp level)
{
    return (level_count_is_x(level, tp_is_wall));
}

uint32_t level_count_is_key (levelp level)
{
    return (level_count_is_x(level, tp_is_key));
}

uint32_t level_count_is_rock (levelp level)
{
    return (level_count_is_x(level, tp_is_rock));
}

uint32_t level_count_is_shadow_caster (levelp level)
{
    return (level_count_is_x(level, tp_is_shadow_caster));
}

uint32_t level_count_is_weapon (levelp level)
{
    return (level_count_is_x(level, tp_is_weapon));
}

uint32_t level_count_is_treasure (levelp level)
{
    return (level_count_is_x(level, tp_is_treasure));
}

uint32_t level_count_is_wearable (levelp level)
{
    return (level_count_is_x(level, tp_is_wearable));
}

uint32_t level_count_is_fragile (levelp level)
{
    return (level_count_is_x(level, tp_is_fragile));
}

uint32_t level_count_is_star (levelp level)
{
    return (level_count_is_x(level, tp_is_star));
}

uint32_t level_count_is_animated_no_dir (levelp level)
{
    return (level_count_is_x(level, tp_is_animated_no_dir));
}

uint32_t level_count_is_weapon_swing_effect (levelp level)
{
    return (level_count_is_x(level, tp_is_weapon_swing_effect));
}

uint32_t level_count_is_light_source (levelp level)
{
    return (level_count_is_x(level, tp_is_light_source));
}

uint32_t level_count_is_candle_light (levelp level)
{
    return (level_count_is_x(level, tp_is_candle_light));
}

uint32_t level_count_is_cats_eyes (levelp level)
{
    return (level_count_is_x(level, tp_is_cats_eyes));
}

uint32_t level_count_is_fire (levelp level)
{
    return (level_count_is_x(level, tp_is_fire));
}

uint32_t level_count_is_ring (levelp level)
{
    return (level_count_is_x(level, tp_is_ring));
}

uint32_t level_count_is_animation (levelp level)
{
    return (level_count_is_x(level, tp_is_animation));
}

uint32_t level_count_is_non_explosive_gas_cloud (levelp level)
{
    return (level_count_is_x(level, tp_is_non_explosive_gas_cloud));
}

uint32_t level_count_is_item_unusable (levelp level)
{
    return (level_count_is_x(level, tp_is_item_unusable));
}

uint32_t level_count_is_valid_for_action_bar (levelp level)
{
    return (level_count_is_x(level, tp_is_valid_for_action_bar));
}

uint32_t level_count_is_door (levelp level)
{
    return (level_count_is_x(level, tp_is_door));
}

uint32_t level_count_is_mob_spawner (levelp level)
{
    return (level_count_is_x(level, tp_is_mob_spawner));
}

uint32_t level_count_is_acid (levelp level)
{
    return (level_count_is_x(level, tp_is_acid));
}

uint32_t level_count_is_lava (levelp level)
{
    return (level_count_is_x(level, tp_is_lava));
}

uint32_t level_count_is_teleport (levelp level)
{
    return (level_count_is_x(level, tp_is_teleport));
}

uint32_t level_count_is_potion (levelp level)
{
    return (level_count_is_x(level, tp_is_potion));
}

uint32_t level_count_is_shield (levelp level)
{
    return (level_count_is_x(level, tp_is_shield));
}

uint32_t level_count_is_death (levelp level)
{
    return (level_count_is_x(level, tp_is_death));
}

uint32_t level_count_is_cobweb (levelp level)
{
    return (level_count_is_x(level, tp_is_cobweb));
}

uint32_t level_count_is_ethereal (levelp level)
{
    return (level_count_is_x(level, tp_is_ethereal));
}

uint32_t level_count_is_variable_size (levelp level)
{
    return (level_count_is_x(level, tp_is_variable_size));
}

uint32_t level_count_is_magical_weapon (levelp level)
{
    return (level_count_is_x(level, tp_is_magical_weapon));
}

uint32_t level_count_is_ranged_weapon (levelp level)
{
    return (level_count_is_x(level, tp_is_ranged_weapon));
}

uint32_t level_count_is_melee_weapon (levelp level)
{
    return (level_count_is_x(level, tp_is_melee_weapon));
}

uint32_t level_count_is_poison (levelp level)
{
    return (level_count_is_x(level, tp_is_poison));
}

uint32_t level_count_is_cloud_effect (levelp level)
{
    return (level_count_is_x(level, tp_is_cloud_effect));
}

uint32_t level_count_is_powerup (levelp level)
{
    return (level_count_is_x(level, tp_is_powerup));
}

uint32_t level_count_is_damaged_on_firing (levelp level)
{
    return (level_count_is_x(level, tp_is_damaged_on_firing));
}

uint32_t level_count_is_hard (levelp level)
{
    return (level_count_is_x(level, tp_is_hard));
}

uint32_t level_count_is_sleeping (levelp level)
{
    return (level_count_is_x(level, tp_is_sleeping));
}

uint32_t level_count_is_bomb (levelp level)
{
    return (level_count_is_x(level, tp_is_bomb));
}

uint32_t level_count_is_sawblade (levelp level)
{
    return (level_count_is_x(level, tp_is_sawblade));
}

uint32_t level_count_is_action_text (levelp level)
{
    return (level_count_is_x(level, tp_is_action_text));
}

uint32_t level_count_is_visible_on_debug_only (levelp level)
{
    return (level_count_is_x(level, tp_is_visible_on_debug_only));
}

uint32_t level_count_is_action (levelp level)
{
    return (level_count_is_x(level, tp_is_action));
}

uint32_t level_count_can_walk_through (levelp level)
{
    return (level_count_is_x(level, tp_can_walk_through));
}

uint32_t level_count_is_weapon_carry_anim (levelp level)
{
    return (level_count_is_x(level, tp_is_weapon_carry_anim));
}

uint32_t level_count_is_spell (levelp level)
{
    return (level_count_is_x(level, tp_is_spell));
}

uint32_t level_count_is_hand_item (levelp level)
{
    return (level_count_is_x(level, tp_is_hand_item));
}

uint32_t level_count_is_boots (levelp level)
{
    return (level_count_is_x(level, tp_is_boots));
}

uint32_t level_count_is_helmet (levelp level)
{
    return (level_count_is_x(level, tp_is_helmet));
}

uint32_t level_count_is_armor (levelp level)
{
    return (level_count_is_x(level, tp_is_armor));
}

uint32_t level_count_is_given_randomly_at_start (levelp level)
{
    return (level_count_is_x(level, tp_is_given_randomly_at_start));
}

uint32_t level_count_is_magical (levelp level)
{
    return (level_count_is_x(level, tp_is_magical));
}

uint32_t level_count_is_degradable (levelp level)
{
    return (level_count_is_x(level, tp_is_degradable));
}

uint32_t level_count_is_cursed (levelp level)
{
    return (level_count_is_x(level, tp_is_cursed));
}

uint32_t level_count_is_animate_only_when_moving (levelp level)
{
    return (level_count_is_x(level, tp_is_animate_only_when_moving));
}

uint32_t level_count_is_warm_blooded (levelp level)
{
    return (level_count_is_x(level, tp_is_warm_blooded));
}

uint32_t level_count_can_be_enchanted (levelp level)
{
    return (level_count_is_x(level, tp_can_be_enchanted));
}

uint32_t level_count_is_stackable (levelp level)
{
    return (level_count_is_x(level, tp_is_stackable));
}

uint32_t level_count_is_torch (levelp level)
{
    return (level_count_is_x(level, tp_is_torch));
}

uint32_t level_count_is_explosion (levelp level)
{
    return (level_count_is_x(level, tp_is_explosion));
}

uint32_t level_count_is_hidden_from_editor (levelp level)
{
    return (level_count_is_x(level, tp_is_hidden_from_editor));
}

uint32_t level_count_is_combustable (levelp level)
{
    return (level_count_is_x(level, tp_is_combustable));
}

uint32_t level_count_is_projectile (levelp level)
{
    return (level_count_is_x(level, tp_is_projectile));
}

uint32_t level_count_is_inactive (levelp level)
{
    return (level_count_is_x(level, tp_is_inactive));
}

uint32_t level_count_is_item_is_hidden (levelp level)
{
    return (level_count_is_x(level, tp_is_ring));
}

uint32_t level_count_is_plant (levelp level)
{
    return (level_count_is_x(level, tp_is_plant));
}

uint32_t level_count_is_food (levelp level)
{
    return (level_count_is_x(level, tp_is_food));
}

uint32_t level_count_is_dungeon_floor (levelp level)
{
    return (level_count_is_x(level, tp_is_dungeon_floor));
}

uint32_t level_count_is_exit (levelp level)
{
    return (level_count_is_x(level, tp_is_exit));
}

static thingp this_door[MAP_WIDTH][MAP_HEIGHT];

static void door_flood (levelp level, int32_t x, int32_t y)
{
    if (this_door[x][y]) {
        return;
    }

    if (!(this_door[x][y] = map_thing_is_door_at(level, x, y))) {
        this_door[x][y] = (thingp) (void*)-1;
        return;
    }

    door_flood(level, x-1, y);
    door_flood(level, x+1, y);
    door_flood(level, x, y-1);
    door_flood(level, x, y+1);
}

void level_open_door (levelp level, int32_t ix, int32_t iy)
{
    int32_t x;
    int32_t y;

    memset(this_door, 0, sizeof(this_door));

    door_flood(level, ix, iy);

    for (x = 0; x < MAP_WIDTH; x++) {
        for (y = 0; y < MAP_HEIGHT; y++) {
            if (!this_door[x][y]) {
                continue;
            }

            if (this_door[x][y] == (void*)-1) {
                continue;
            }

            thing_dead(this_door[x][y], 0 /* killer */, "open");
        }
    }

    level_update_incremental(level, true /* force */);

    /*
     * Removal of doors would have set this flag.
     */
    level_set_needs_updating(server_level, false);

    /*
     * Send the update quickly to the client. Don't wait for the things to
     * tick. The doors should be on the active list now anyway as they were
     * killed above.
     */
    socket_server_tx_map_update(0 /* all clients */, server_active_things,
                                "open door");

    MSG_SERVER_SHOUT_AT_ALL_PLAYERS(SOUND, ix, iy, "door");
}

void marshal_level (marshal_p ctx, levelp level)
{
    if (!level) {
        return;
    }

    PUT_BRA(ctx);

    if (!level->title) {
        PUT_NAMED_STRING(ctx, "title", "unnamed level");
    } else {
        PUT_NAMED_STRING(ctx, "title", level->title);
    }

    PUT_NAMED_BITFIELD(ctx, "is_zzz1", level->is_zzz1);
    PUT_NAMED_BITFIELD(ctx, "is_zzz2", level->is_zzz2);
    PUT_NAMED_BITFIELD(ctx, "is_zzz3", level->is_zzz3);
    PUT_NAMED_BITFIELD(ctx, "is_zzz4", level->is_zzz4);
    PUT_NAMED_BITFIELD(ctx, "is_zzz5", level->is_zzz5);
    PUT_NAMED_BITFIELD(ctx, "is_zzz6", level->is_zzz6);
    PUT_NAMED_BITFIELD(ctx, "is_zzz7", level->is_zzz7);
    PUT_NAMED_BITFIELD(ctx, "is_zzz8", level->is_zzz8);
    PUT_NAMED_BITFIELD(ctx, "is_zzz9", level->is_zzz9);
    PUT_NAMED_BITFIELD(ctx, "is_zzz10", level->is_zzz10);
    PUT_NAMED_BITFIELD(ctx, "is_zzz11", level->is_zzz11);
    PUT_NAMED_BITFIELD(ctx, "is_zzz12", level->is_zzz12);
    PUT_NAMED_BITFIELD(ctx, "is_zzz13", level->is_zzz13);
    PUT_NAMED_BITFIELD(ctx, "zzz16", level->zzz16);
    PUT_NAMED_BITFIELD(ctx, "is_death_summoned", level->is_death_summoned);
    PUT_NAMED_BITFIELD(ctx, "is_jesus_summoned", level->is_jesus_summoned);
    PUT_NAMED_BITFIELD(ctx, "is_blit_y_offset", level->is_blit_y_offset);
    PUT_NAMED_BITFIELD(ctx, "exit_reached_when_open", level->exit_reached_when_open);
    PUT_NAMED_BITFIELD(ctx, "warned_exit_not_open", level->warned_exit_not_open);
    PUT_NAMED_BITFIELD(ctx, "is_exit_open", level->exit_reached_when_open);
}

uint8_t demarshal_level (demarshal_p ctx, levelp level)
{
    uint8_t rc;
    widp wid;

    if (!level) {
        return (false);
    }

    GET_BRA(ctx);

    char *tmp = 0;
    GET_OPT_NAMED_STRING(ctx, "title", tmp);
    if (!tmp) {
        ERR("no level title for level");
    } else {
        level_set_title(level, tmp);
        myfree(tmp);
    }

    wid = level_get_map(level);
    if (!wid) {
        ERR("no map for level");
    }

    do {
        GET_OPT_NAMED_BITFIELD(ctx, "is_zzz1", level->is_zzz1);
        GET_OPT_NAMED_BITFIELD(ctx, "is_zzz2", level->is_zzz2);
        GET_OPT_NAMED_BITFIELD(ctx, "is_zzz3", level->is_zzz3);
        GET_OPT_NAMED_BITFIELD(ctx, "is_zzz4", level->is_zzz4);
        GET_OPT_NAMED_BITFIELD(ctx, "is_zzz5", level->is_zzz5);
        GET_OPT_NAMED_BITFIELD(ctx, "is_zzz6", level->is_zzz6);
        GET_OPT_NAMED_BITFIELD(ctx, "is_zzz7", level->is_zzz7);
        GET_OPT_NAMED_BITFIELD(ctx, "is_zzz8", level->is_zzz8);
        GET_OPT_NAMED_BITFIELD(ctx, "is_zzz9", level->is_zzz9);
        GET_OPT_NAMED_BITFIELD(ctx, "is_zzz10", level->is_zzz10);
        GET_OPT_NAMED_BITFIELD(ctx, "is_zzz11", level->is_zzz11);
        GET_OPT_NAMED_BITFIELD(ctx, "is_zzz12", level->is_zzz12);
        GET_OPT_NAMED_BITFIELD(ctx, "is_zzz13", level->is_zzz13);
        GET_OPT_NAMED_BITFIELD(ctx, "zzz16", level->zzz16);
        GET_OPT_NAMED_BITFIELD(ctx, "is_death_summoned", level->is_death_summoned);
        GET_OPT_NAMED_BITFIELD(ctx, "is_jesus_summoned", level->is_jesus_summoned);
        GET_OPT_NAMED_BITFIELD(ctx, "is_blit_y_offset", level->is_blit_y_offset);
        GET_OPT_NAMED_BITFIELD(ctx, "exit_reached_when_open", 
                               level->exit_reached_when_open);
        GET_OPT_NAMED_BITFIELD(ctx, "warned_exit_not_open", 
                               level->warned_exit_not_open);
        GET_OPT_NAMED_BITFIELD(ctx, "is_exit_open", 
                               level->exit_reached_when_open);
    } while (demarshal_gotone(ctx));

    if (level_is_map_editor(level)) {
        rc = demarshal_wid_grid(ctx, wid,
                                wid_editor_level_map_thing_replace_template);
    } else if (level_is_editor(level)) {
        rc = demarshal_wid_grid(ctx, wid,
                                wid_editor_replace_template);
    } else {
        rc = demarshal_wid_grid(ctx, wid,
                                wid_game_map_server_replace_tile);
    }

    if (level_is_map_editor(level)) {
        /*
         * No widget to update
         */
    } else if (level_is_editor(level)) {
        /*
         * No widget to update
         */
    } else {
        map_fixup(level);
        wid_update(wid_game_map_server_grid_container);
    }

    GET_KET(ctx);

    return (rc);
}
