/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>
#include <errno.h>
#include "slre.h"

#include "main.h"
#include "string.h"
#include "wid.h"
#include "level.h"
#include "thing.h"
#include "command.h"
#include "time.h"
#include "marshal.h"
#include "wid_game_map_server.h"
#include "wid_game_map_client.h"
#include "wid_editor_map.h"
#include "map.h"
#include "timer.h"
#include "sound.h"
#include "wid_editor.h"
#include "socket.h"
#include "map_jigsaw.h"

static uint8_t level_command_dead(tokens_t *tokens, void *context);
static uint8_t level_init_done;
static uint8_t level_server_init_done;
static void level_start_timers(levelp level);
uint8_t game_over;

uint8_t level_init (void)
{
    if (level_init_done) {
        return (true);
    }

    level_init_done = true;

    return (true);
}

static uint8_t level_server_init (void)
{
    if (level_server_init_done) {
        return (true);
    }

    level_server_init_done = true;

    command_add(level_command_dead, 
                "dead", "internal command for thing suicide");

    return (true);
}

void level_fini (void)
{
    if (level_init_done) {
        level_init_done = false;
    }
}

/*
 * User has entered a command, run it
 */
static uint8_t level_command_dead (tokens_t *tokens, void *context)
{
    thingp t;

    if (!context) {
        return (false);
    }

    t = (typeof(t)) context;
    verify(t);

    if (thing_is_seedpod(t)) {
        tpp tp = tp_find("data/things/plant");

        wid_game_map_server_replace_tile(wid_game_map_server_grid_container,
                                         thing_grid_x(t),
                                         thing_grid_y(t),
                                         0, /* thing */
                                         tp,
                                         0 /* item */,
                                         0 /* stats */);

        sound_play_slime();

        thing_dead(t, 0 /* killer */, "turned into plant");
        return (true);
    }

    return (true);
}

levelp level_new (widp map, uint32_t level_no, int is_editor, int on_server)
{
    levelp level;

    level = (typeof(level)) myzalloc(sizeof(*level), "level");

    level_set_map(level, map);
    level_set_level_no(level, level_no);
    level_set_is_editor(level, is_editor);

    if (on_server) {
        level->logname = dynprintf("%u[%p] (server)", level_no, level);
    } else {
        level->logname = dynprintf("%u[%p] (client)", level_no, level);
    }

    level->on_server = on_server;

    LEVEL_LOG(level, "created");

    return (level);
}

void level_destroy (levelp *plevel, uint8_t keep_players)
{
    levelp level;

    if (!plevel) {
        DIE("no plevel");
    }

    level = *plevel;
    if (!level) {
        DIE("no level");
    }

    /*
     * Perhaps another thread is still using this level for map generation?
     */
    level->exit_request = 1;
    while (level->locked) {
        LOG("Level locked... waiting on AI thread to finish");
        sleep(1);
    }
    level->exit_request = 0;

    /*
     * Kill all humans!
     */
    things_level_destroyed(level, keep_players);

    LEVEL_LOG(level, "destroy");

    if (level->title) {
        myfree((void*) level->title);
    }

    if (level->destroy_reason) {
        myfree((void*) level->destroy_reason);
    }

    LEVEL_LOG(level, "destroyed");

    if (level->logname) {
        myfree((void*) level->logname);
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

void level_update_now (levelp level)
{
    map_fixup(level);

    level_set_walls(level);
    level_set_monst_map_treat_doors_as_passable(level);
    level_set_monst_map_treat_doors_as_walls(level);
    level_set_player_map_treat_doors_as_walls(level);
    level_set_doors(level);
    level_set_pipes(level);
    level_pipe_find_ends(level);

    /*
     * One time generate of expensive wander map
     */
    dmap_generate_monst_map_wander(level);

    /*
     * Ensure things are updated at the start of the new level.
     */
    thing_tick_server_player_slow_all();
}

void level_update (levelp level)
{
    level->need_map_update = 1;
}

levelp level_load (uint32_t level_no, 
                   widp wid, 
                   int is_editor,
                   int on_server)
{
    levelp level;

    level = level_new(wid, level_no, is_editor, on_server);

    level_set_is_paused(level, true);
    level_set_timestamp_started(level, time_get_time_cached());

    char *dir_and_file;

    dir_and_file = dynprintf("data/levels/%u", level_no);

    demarshal_p in;

    LEVEL_LOG(level, "Level %s: loading", dir_and_file);

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

    myfree(dir_and_file);

    level_update_now(level);

    level_set_is_paused(level, false);
    level_start_timers(level);

    level_server_init();

    LEVEL_LOG(level, "Level loaded");

    return (level);
}

levelp level_load_random (uint32_t level_no, 
                          widp wid, 
                          int is_editor,
                          int on_server)
{
    levelp level;

    level = level_new(wid, level_no, is_editor, on_server);

    level_set_is_paused(level, true);
    level_set_timestamp_started(level, time_get_time_cached());

    LEVEL_LOG(level, "Level generating");

    map_jigsaw_generate(wid);

    level_update_now(level);

    level_set_is_paused(level, false);
    level_start_timers(level);

    level_server_init();

    LEVEL_LOG(level, "Level loaded");

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

/*
 * Or other things we collide with.
 */
void level_set_walls (levelp level)
{
    int32_t x;
    int32_t y;

    for (x = 0; x < MAP_WIDTH; x++) {
        for (y = 0; y < MAP_HEIGHT; y++) {
            if (map_is_wall_at(level, x, y)) {
                level->walls.walls[x][y] = '+';
            } else {
                level->walls.walls[x][y] = ' ';
            }
        }
    }
}

/*
 * Or other things we collide with.
 */
void level_set_monst_map_treat_doors_as_passable (levelp level)
{
    int32_t x;
    int32_t y;

    for (x = 0; x < MAP_WIDTH; x++) {
        for (y = 0; y < MAP_HEIGHT; y++) {
            if (map_is_wall_at(level, x, y)         ||
                map_is_exit_at(level, x, y)         ||
                map_is_spam_at(level, x, y)) {
                /*
                 * Obstacle.
                 */
                level->monst_map_treat_doors_as_passable.walls[x][y] = '+';
            } else {
                /*
                 * Note, doors are considered as empty space.
                 */
                level->monst_map_treat_doors_as_passable.walls[x][y] = ' ';
            }
        }
    }
}

/*
 * Or other things we collide with.
 */
void level_set_monst_map_treat_doors_as_walls (levelp level)
{
    int32_t x;
    int32_t y;

    for (x = 0; x < MAP_WIDTH; x++) {
        for (y = 0; y < MAP_HEIGHT; y++) {
            if (map_is_wall_at(level, x, y)         ||
                map_is_door_at(level, x, y)         ||
                map_is_exit_at(level, x, y)         ||
                map_is_spam_at(level, x, y)         ||
                map_is_mob_spawner_at(level, x, y)) {

                /*
                 * Considered as walls.
                 */
                level->monst_map_treat_doors_as_walls.walls[x][y] = '+';
            } else {
                level->monst_map_treat_doors_as_walls.walls[x][y] = ' ';
            }
        }
    }
}

/*
 * Used in flood filling explosions
 */
void level_set_player_map_treat_doors_as_walls (levelp level)
{
    int32_t x;
    int32_t y;

    for (x = 0; x < MAP_WIDTH; x++) {
        for (y = 0; y < MAP_HEIGHT; y++) {
            if (map_is_wall_at(level, x, y)         ||
                map_is_door_at(level, x, y)         ||
                map_is_exit_at(level, x, y)         ||
                map_is_spam_at(level, x, y)) {

                /*
                 * Considered as walls for explosions.
                 */
                level->player_map_treat_doors_as_walls.walls[x][y] = '+';
            } else {
                level->player_map_treat_doors_as_walls.walls[x][y] = ' ';
            }
        }
    }
}

/*
 * Or other things we collide with.
 */
void level_set_doors (levelp level)
{
    int32_t x;
    int32_t y;

    for (x = 0; x < MAP_WIDTH; x++) {
        for (y = 0; y < MAP_HEIGHT; y++) {
            if (map_is_door_at(level, x, y)) {
                level->roads.walls[x][y] = '+';
            } else {
                level->roads.walls[x][y] = ' ';
            }
        }
    }
}

/*
 * Or other things we collide with.
 */
void level_set_pipes (levelp level)
{
    int32_t x;
    int32_t y;

    for (x = 0; x < MAP_WIDTH; x++) {
        for (y = 0; y < MAP_HEIGHT; y++) {
            if (map_is_pipe_at(level, x, y)) {
                level->pipes.walls[x][y] = '+';
            } else {
                level->pipes.walls[x][y] = ' ';
            }
        }
    }
}

void level_pipe_find_ends (levelp level)
{
    int32_t x;
    int32_t y;

    memset(&level->end_pipe, ' ', sizeof(level->end_pipe));

    for (x = 1; x < MAP_WIDTH-1; x++) {
        for (y = 1; y < MAP_HEIGHT-1; y++) {

            if (level->pipes.walls[x][y] == ' ') {
                continue;
            }

            level->walls.walls[x][y] = '+';

            //  .
            // .x.
            // ?x?
            if ((level->pipes.walls[x][y+1] == '+') &&
                (level->pipes.walls[x][y-1] == ' ') &&
                (level->pipes.walls[x+1][y] == ' ') &&
                (level->pipes.walls[x+1][y] == ' ')) {
                level->end_pipe.walls[x][y] = '+';
                level->walls.walls[x][y] = ' ';
            }

            // ?x?
            // .x.
            //  .
            if ((level->pipes.walls[x][y-1] == '+') &&
                (level->pipes.walls[x][y+1] == ' ') &&
                (level->pipes.walls[x+1][y] == ' ') &&
                (level->pipes.walls[x+1][y] == ' ')) {
                level->end_pipe.walls[x][y] = '+';
                level->walls.walls[x][y] = ' ';
            }

            //  .?
            // .xx
            //  .?
            if ((level->pipes.walls[x+1][y] == '+') &&
                (level->pipes.walls[x-1][y] == ' ') &&
                (level->pipes.walls[x][y-1] == ' ') &&
                (level->pipes.walls[x][y+1] == ' ')) {
                level->end_pipe.walls[x][y] = '+';
                level->walls.walls[x][y] = ' ';
            }

            //  .?
            // .xx
            //  .?
            if ((level->pipes.walls[x-1][y] == '+') &&
                (level->pipes.walls[x+1][y] == ' ') &&
                (level->pipes.walls[x][y-1] == ' ') &&
                (level->pipes.walls[x][y+1] == ' ')) {
                level->walls.walls[x][y] = ' ';
                level->end_pipe.walls[x][y] = '+';
            }
        }
    }
}

/*
 * Place a plant pod
 */
void level_place_plant_pod (levelp level)
{
    tpp tp = 0;
    uint32_t i;
    int32_t x;
    int32_t y;

    for (i = 0;
         i < MAP_HEIGHT * MAP_WIDTH; i++) {

        x = rand() % MAP_WIDTH;
        y = rand() % MAP_HEIGHT;

        if (map_is_wall_at(level, x, y)) {
            continue;
        }

        if (map_is_pipe_at(level, x, y)) {
            continue;
        }

        if (map_is_player_at(level, x, y)) {
            continue;
        }

        if (map_is_exit_at(level, x, y)) {
            continue;
        }

        tp = tp_find("data/things/seedpod");

        wid_game_map_server_replace_tile(wid_game_map_server_grid_container,
                                         x,
                                         y,
                                         0, /* thing */
                                         tp,
                                         0 /* item */,
                                         0 /* stats */);

        sound_play_slime();

        return;
    }
}

/*
 * Start any timers.
 */
void level_start_timers (levelp level)
{
    thingp t;

    /*
     * Rewield weapons at start.
     */
    TREE_WALK(server_active_things, t) {
        if (thing_is_player(t)) {
            tpp weapon = thing_weapon(t);
            if (weapon) {
                thing_wield(t, weapon);
            }
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
            "Get ready!",
            "Meep!",
        };

        socket_tx_server_shout(POPUP,
                               messages[rand() % ARRAY_SIZE(messages)]);

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

    thingp t;

    /*
     * Force the death of all things on the level.
     */
    { TREE_WALK(server_active_things, t) {
        if (!thing_is_player(t)) {
            thing_leave_level(t);
            t->is_dead = true;
            thing_update(t);
        }
    } }

    { TREE_WALK(server_boring_things, t) {
        thing_leave_level(t);
        t->is_dead = true;
        thing_update(t);
    } }

    socket_server_tx_map_update(0, server_boring_things,
                                "level destroy boring things");
    socket_server_tx_map_update(0, server_active_things,
                                "level destroy active things");

    wid_game_map_server_wid_destroy(true /* keep players */);

    { TREE_WALK(server_active_things, t) {
        if (!thing_is_player_or_owned_by_player(t)) {
            ERR("players should be all that is left by now "
                "but we have %s", thing_logname(t));
            continue;
        }
    } }

    wid_game_map_server_wid_create();

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
                0 /* item */,
                0 /* stats */);

        thing_join_level(t);
    } }

    socket_server_tx_map_update(0, server_boring_things,
                                "new level boring things");
    socket_server_tx_map_update(0, server_active_things,
                                "new level active things");

    level_update_now(server_level);
}

/*
 * Check for expired timers. We fire one per loop.
 */
void level_tick (levelp level)
{
    if (!level) {
        return;
    }

    /*
     * Did some walls get zapped and we need to rebuild connectors?
     */
    if (level->need_map_update) {
        level->need_map_update = 0;
        level->need_boring_update = 0;

        level_update_now(level);

        socket_server_tx_map_update(0, server_active_things,
                                    "level map update active things");
        /*
         * Let the players know their item lists.
         */
        socket_tx_server_status();

        socket_server_tx_map_update(0, server_boring_things,
                                    "level map update boring things");
    }

    /*
     * Sparks on a wall?
     */
    if (level->need_boring_update) {
        level->need_boring_update = 0;
        socket_server_tx_map_update(0, server_boring_things,
                                    "boring update needed");
    }

    /*
     * If the player has finished the level then popup a notice so all players 
     * know the end is nigh!
     */
    if (level_exit_has_been_reached(level)) {
        if (!level->end_level_first_phase_fade_out_timer) {
            socket_tx_server_shout(POPUP, "Level completed");

            level->end_level_first_phase_fade_out_timer = 
                action_timer_create(&server_timers,
                                    level_action_timer_end_level_first_phase_fade_out,
                                    0,
                                    level,
                                    "end level",
                                    DELAY_LEVEL_END_HIDE,
                                    ONESEC);

            level->end_level_second_phase_destroy_timer = 
                action_timer_create(&server_timers,
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
    }
}

uint32_t level_get_level_no (levelp level)
{
    verify(level);

    return (level->level_no);
}

void level_set_level_no (levelp level, uint32_t val)
{
    verify(level);

    level->level_no = val;
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

uint8_t level_is_hurryup (levelp level)
{
    verify(level);

    return (level->is_hurryup);
}

void level_set_is_hurryup (levelp level, uint8_t val)
{
    verify(level);

    level->is_hurryup = val;
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

uint8_t level_is_zzz14 (levelp level)
{
    verify(level);

    return (level->is_zzz14);
}

void level_set_is_zzz14 (levelp level, uint8_t val)
{
    verify(level);

    level->is_zzz14 = val;
}

uint8_t level_is_zzz15 (levelp level)
{
    verify(level);

    return (level->is_zzz15);
}

void level_set_is_zzz15 (levelp level, uint8_t val)
{
    verify(level);

    level->is_zzz15 = val;
}

uint8_t level_is_zzz16 (levelp level)
{
    verify(level);

    return (level->is_zzz16);
}

void level_set_is_zzz16 (levelp level, uint8_t val)
{
    verify(level);

    level->is_zzz16 = val;
}

uint8_t level_is_zzz17 (levelp level)
{
    verify(level);

    return (level->is_zzz17);
}

void level_set_is_zzz17 (levelp level, uint8_t val)
{
    verify(level);

    level->is_zzz17 = val;
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

uint8_t level_is_paused (levelp level)
{
    verify(level);

    return (level->is_paused);
}

void level_set_is_paused (levelp level, uint8_t val)
{
    verify(level);

    level->is_paused = val;
}

uint8_t level_exit_has_been_reached (levelp level)
{
    verify(level);

    return (level->exit_has_been_reached);
}

void level_set_exit_has_been_reached (levelp level, uint8_t val)
{
    verify(level);

    level->exit_has_been_reached = val;
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

void marshal_level (marshal_p ctx, levelp level)
{
    if (!level) {
        return;
    }

    PUT_BRA(ctx);

    PUT_NAMED_STRING(ctx, "title", level->title);
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
    PUT_NAMED_BITFIELD(ctx, "is_zzz14", level->is_zzz14);
    PUT_NAMED_BITFIELD(ctx, "is_zzz15", level->is_zzz15);
    PUT_NAMED_BITFIELD(ctx, "is_zzz16", level->is_zzz16);
    PUT_NAMED_BITFIELD(ctx, "is_zzz17", level->is_zzz17);
    PUT_NAMED_BITFIELD(ctx, "exit_reached_when_open", level->exit_reached_when_open);
    PUT_NAMED_BITFIELD(ctx, "warned_exit_not_open", level->warned_exit_not_open);
    PUT_NAMED_BITFIELD(ctx, "is_exit_open", level->exit_reached_when_open);

    marshal_wid_grid(ctx, level->map);

    PUT_KET(ctx);
}

uint8_t demarshal_level (demarshal_p ctx, levelp level)
{
    uint8_t rc;
    widp wid;

    if (!level) {
        return (false);
    }

    GET_BRA(ctx);

    char *tmp;
    GET_OPT_NAMED_STRING(ctx, "title", tmp);
    level_set_title(level, tmp);
    myfree(tmp);

    wid = level_get_map(level);
    if (!wid) {
        DIE("no map for level");
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
        GET_OPT_NAMED_BITFIELD(ctx, "is_zzz14", level->is_zzz14);
        GET_OPT_NAMED_BITFIELD(ctx, "is_zzz15", level->is_zzz15);
        GET_OPT_NAMED_BITFIELD(ctx, "is_zzz16", level->is_zzz16);
        GET_OPT_NAMED_BITFIELD(ctx, "is_zzz17", level->is_zzz17);
        GET_OPT_NAMED_BITFIELD(ctx, "exit_reached_when_open", 
                               level->exit_reached_when_open);
        GET_OPT_NAMED_BITFIELD(ctx, "warned_exit_not_open", 
                               level->warned_exit_not_open);
        GET_OPT_NAMED_BITFIELD(ctx, "is_exit_open", 
                               level->exit_reached_when_open);
    } while (demarshal_gotone(ctx));

    server_level_is_being_loaded = true;

    if (level_is_editor(level)) {
        rc = demarshal_wid_grid(ctx, wid,
                                wid_editor_map_thing_replace_template);
    } else {
        rc = demarshal_wid_grid(ctx, wid,
                                wid_game_map_server_replace_tile);
    }

    server_level_is_being_loaded = false;

    map_fixup(level);

    if (level_is_editor(level)) {
        wid_update(wid_editor_map_grid_container);
    } else {
        wid_update(wid_game_map_server_grid_container);
    }

    GET_KET(ctx);

    return (rc);

}
