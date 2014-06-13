/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>
#include <errno.h>
#include "slre.h"

#include "main.h"
#include "math.h"
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
#include "wid_console.h"
#include "wid_editor.h"
#include "socket.h"
#include "level_private.h"

static uint8_t level_command_dead(tokens_t *tokens, void *context);
static uint8_t level_init_done;
static uint8_t level_server_init_done;
static void level_start_timers(levelp level);
uint8_t game_over;

uint8_t god_mode = false;
uint32_t start_level = 1;
uint32_t start_lives = 3;

/*
 * Various level timers.
 */
tree_rootp timers;

/*
 * Used so we only do the slow countdown first time on each level.
 */
static uint8_t level_command_lives (tokens_t *tokens, void *context)
{
    char *s = tokens->args[2];

    if (!s || (*s == '\0')) {
        start_lives = 1;
    } else {
        start_lives = strtol(s, 0, 10);
    }

    CON("Start lives set to %u", start_lives);

    return (true);
}

static uint8_t level_command_play (tokens_t *tokens, void *context)
{
    char *s = tokens->args[1];

    if (!s || (*s == '\0')) {
        start_level = 1;
    } else {
        start_level = strtol(s, 0, 10);
    }

    CON("Start level set to %u", start_level);

    if (player) {
        levelp level = thing_level(player);

        thing_set_level_no(player, start_level);

        CON("Player level set to %u", start_level);

        if (level) {
            wid_hide(wid_console_window, 0);
        }
    }

    return (true);
}

static uint8_t level_command_god_mode (tokens_t *tokens, void *context)
{
    char *s = tokens->args[2];

    if (!s || (*s == '\0')) {
        god_mode = 1;
    } else {
        god_mode = strtol(s, 0, 10) ? 1 : 0;
    }

    CON("God mode set to %u", god_mode);

    return (true);
}

uint8_t level_init (void)
{
    if (level_init_done) {
        return (true);
    }

    level_init_done = true;

    if (0) {
        command_add(level_command_play, 
                    "play [123456789]+", "TBD play level x");

        command_add(level_command_lives, 
                    "set lives [123456789]+", "TBD set player number of lives");
    }

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

    if (is_server) {
        command_add(level_command_god_mode, 
                    "set godmode [01]", "TBD enable/disable god mode");
    }

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

    if ((t->thing_template->id == THING_WEAPON_HIT1) ||
        (t->thing_template->id == THING_EXPLOSION1) ||
        (t->thing_template->id == THING_EXPLOSION2) ||
        (t->thing_template->id == THING_EXPLOSION3) ||
        (t->thing_template->id == THING_EXPLOSION4) ||
        (t->thing_template->id == THING_EXPLOSION5) ||
        (t->thing_template->id == THING_EXPLOSION6) ||
        (t->thing_template->id == THING_EXPLOSION7) ||
        (t->thing_template->id == THING_EXPLOSION8) ||
        (t->thing_template->id == THING_POISON1) ||
        (t->thing_template->id == THING_POISON2) ||
        (t->thing_template->id == THING_POISON3) ||
        (t->thing_template->id == THING_POISON4) ||
        (t->thing_template->id == THING_POISON5) ||
        (t->thing_template->id == THING_POISON6) ||
        (t->thing_template->id == THING_POISON7) ||
        (t->thing_template->id == THING_POISON8) ||
        (t->thing_template->id == THING_CLOUDKILL1) ||
        (t->thing_template->id == THING_CLOUDKILL2) ||
        (t->thing_template->id == THING_CLOUDKILL3) ||
        (t->thing_template->id == THING_CLOUDKILL4) ||
        (t->thing_template->id == THING_CLOUDKILL5) ||
        (t->thing_template->id == THING_CLOUDKILL6) ||
        (t->thing_template->id == THING_CLOUDKILL7) ||
        (t->thing_template->id == THING_CLOUDKILL8)) {
        thing_dead(t, 0 /* killer */, "finished blowing up");
        return (true);
    }

    if (thing_is_seedpod(t)) {
        thing_templatep thing_template = 
                thing_template_find("data/things/plant");

        wid_game_map_server_replace_tile(wid_game_map_server_grid_container,
                                         thing_grid_x(t),
                                         thing_grid_y(t),
                                         thing_template);

        sound_play_slime();

        thing_dead(t, 0 /* killer */, "turned into plant");
        return (true);
    }

    return (true);
}

levelp level_new (widp map, uint32_t level_no, int is_editor)
{
    levelp level;

    level = (typeof(level)) myzalloc(sizeof(*level), "level");

    level_set_map(level, map);
    level_set_level_no(level, level_no);
    level_set_is_editor(level, is_editor);

    level->logname = dynprintf("%u[%p]", level_no, level);

    LEVEL_LOG(level, "created");

    return (level);
}

void level_destroy (levelp *plevel)
{
    levelp level;

    if (!plevel) {
        DIE("no plevel");
    }

    level = *plevel;
    if (!level) {
        DIE("no level");
    }

    *plevel = 0;

    /*
     * Perhaps another thread is still using this level for map generation?
     */
    while (level->locked) {
        LOG("Level locked... waiting");
        sleep(1);
    }

    /*
     * Ensure no stale pointers.
     */
    if (level == client_level) {
        client_level = 0;
    }

    if (level == server_level) {
        server_level = 0;
    }

    /*
     * Kill all humans!
     */
    things_level_destroyed(level);

    LEVEL_LOG(level, "destroy now {");

    if (level->title) {
        myfree((void*) level->title);
    }

    if (level->destroy_reason) {
        myfree((void*) level->destroy_reason);
    }

    action_timers_destroy(&timers);

    LEVEL_LOG(level, "} destroy finished");

    if (level->logname) {
        myfree((void*) level->logname);
    }

    wid_game_map_server_wid_destroy();

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
}

void level_update (levelp level)
{
    level->need_map_update = 1;
}

levelp level_load (uint32_t level_no, widp wid, int is_editor)
{
    levelp level;

    srand(level_no);

    level = level_new(wid, level_no, is_editor);

    level_set_is_paused(level, true);
    level_set_timestamp_started(level, time_get_time_cached());

    char *dir_and_file;

    dir_and_file = dynprintf("data/levels/%u", level_no);

    demarshal_p in;

    LOG("Level %s: loading", dir_and_file);

    if (!(in = demarshal(dir_and_file))) {
        /*
         * Fail
         *
         * I'm assuming this means the end of the game
         *
        char *popup_str = dynprintf("Failed to load level %s: %s",
                                    dir_and_file,
                                    strerror(errno));

        MSGERR("%s", popup_str);
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

            MSGERR("%s", popup_str);
            myfree(popup_str);
        }

        demarshal_fini(in);
    }

    myfree(dir_and_file);

    level_update_now(level);

    level_set_is_paused(level, false);
    level_start_timers(level);

    level_server_init();

    LOG("Level loaded");

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
            if (map_is_wall_at(level, x, y) ||
                !map_is_floor_at(level, x, y)) {
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
                map_is_spam_at(level, x, y)         ||
                map_is_mob_spawner_at(level, x, y)  ||
                !map_is_floor_at(level, x, y)) {
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
                map_is_mob_spawner_at(level, x, y)  ||
                !map_is_floor_at(level, x, y)) {

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
                map_is_spam_at(level, x, y)         ||
                !map_is_floor_at(level, x, y)) {

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
    thing_templatep thing_template = 0;
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

        if (!map_is_floor_at(level, x, y)) {
            continue;
        }

        thing_template = thing_template_find("data/things/seedpod");

        wid_game_map_server_replace_tile(wid_game_map_server_grid_container,
                                         x,
                                         y,
                                         thing_template);

        sound_play_slime();

        return;
    }
}

/*
 * Place an explosion
 */
static uint8_t level_place_explosion_at (levelp level,
                                         thingp owner,
                                         double x, 
                                         double y, 
                                         double i,
                                         uint32_t nargs,
                                         va_list args)
{

    /*
     * Choose one of the things in the args list to place.
     */
    uint32_t r = (rand() % nargs) + 1;

    const char *name = 0;
    while (r--) {
        name = va_arg(args, char *);
    }

    if (!name) {
        DIE("cannot place explosion thing");
    }

    thing_templatep thing_template = thing_template_find(name);
    if (!thing_template) {
        DIE("no explosion");
    }

    thing_place_and_destroy_timed(thing_template,
                                  owner,
                                  x,
                                  y,
                                  i * 100, // place in
                                  500,     // destroy in
                                  10);     // jitter

    return (true);
}

/*
 * Place an explosion
 */
static void level_place_explosion_ (levelp level, 
                                    thingp owner,
                                    double x, 
                                    double y,
                                    uint32_t radius,
                                    uint32_t nargs, ...)
{
    va_list args;

    dmap_generate_player_map(x, y);

    uint32_t ix, iy;

    for (ix = 1; ix < MAP_WIDTH - 1; ix++) {
        for (iy = 1; iy < MAP_HEIGHT - 1; iy++) {
            int8_t dist = dmap_player_map_treat_doors_as_walls.walls[ix][iy];

            if (dist > radius) {
                continue;
            }

            double density = 0.5;
            double dx, dy;

            for (dx = -0.5; dx < 0.5; dx += density) {
                for (dy = -0.5; dy < 0.5; dy += density) {
                    double ex = ix + dx;
                    double ey = iy + dy;

                    va_start(args, nargs);

                    (void) level_place_explosion_at(level, 
                                                    owner,
                                                    ex, 
                                                    ey, 
                                                    dist,
                                                    nargs, args);
                    va_end(args);
                }
            }
        }
    }
}

void level_place_weapon_hit1 (levelp level, 
                              thingp owner,
                              double x, double y)
{
    thing_templatep thing_template = thing_template_find(
                                    "data/things/weapon_hit1");
    if (!thing_template) {
        DIE("no weapon hit effect");
    }

    thing_place_and_destroy_timed(thing_template,
                                  owner,
                                  x,
                                  y,
                                  0,       // place in
                                  500,     // destroy in
                                  10);     // jitter

}

void level_place_explosion (levelp level, 
                            thingp owner,
                            double x, double y)
{
    level_place_explosion_(level, 
                           owner,
                           x, y,
                           6, // radius
                           7, // nargs
                           "data/things/explosion1",
                           "data/things/explosion2",
                           "data/things/explosion3",
                           "data/things/explosion4",
                           "data/things/explosion5",
                           "data/things/explosion6",
                           "data/things/explosion7");
}

void level_place_small_explosion (levelp level, 
                                  thingp owner,
                                  double x, double y)
{
    level_place_explosion_(level, 
                           owner,
                           x, y,
                           3, // radius
                           7, // nargs
                           "data/things/explosion1",
                           "data/things/explosion2",
                           "data/things/explosion3",
                           "data/things/explosion4",
                           "data/things/explosion5",
                           "data/things/explosion6",
                           "data/things/explosion7");
}

void level_place_potion_effect_fireball (levelp level, 
                                         thingp owner,
                                         double x, double y)
{
    level_place_explosion_(level, 
                           owner,
                           x, y,
                           7, // radius
                           2, // nargs
                           "data/things/explosion1",
                           "data/things/explosion2",
                           "data/things/explosion3",
                           "data/things/explosion4",
                           "data/things/explosion5",
                           "data/things/explosion6",
                           "data/things/explosion7");
}

void level_place_potion_effect_poison (levelp level, 
                                       thingp owner,
                                       double x, double y)
{
    level_place_explosion_(level, 
                           owner,
                           x, y,
                           9, // radius
                           2, // nargs
                           "data/things/poison1",
                           "data/things/poison2",
                           "data/things/poison3",
                           "data/things/poison4",
                           "data/things/poison5",
                           "data/things/poison6",
                           "data/things/poison7");
}

void level_place_potion_effect_cloudkill (levelp level, 
                                       thingp owner,
                                       double x, double y)
{
    level_place_explosion_(level, 
                           owner,
                           x, y,
                           12, // radius
                           2, // nargs
                           "data/things/cloudkill1",
                           "data/things/cloudkill2",
                           "data/things/cloudkill3",
                           "data/things/cloudkill4",
                           "data/things/cloudkill5",
                           "data/things/cloudkill6",
                           "data/things/cloudkill7");
}

/*
 * Start any timers.
 */
void level_start_timers (levelp level)
{
}

/*
 * Check for expired timers. We fire one per loop.
 */
void level_tick (levelp level)
{
    if (timers) {
        action_timers_tick(timers);
    }

    if (level) {
        if (level->need_map_update) {
            level->need_map_update = 0;

            level_update_now(level);

            socket_server_tx_map_update(0, server_active_things);
            socket_server_tx_map_update(0, server_boring_things);
        }
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

uint8_t level_is_frozen (levelp level)
{
    verify(level);

    return (level->is_frozen);
}

void level_set_is_frozen (levelp level, uint8_t val)
{
    verify(level);

    level->is_frozen = val;
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

    wid_editor_map_loading = true;

    if (level_is_editor(level)) {
        rc = demarshal_wid_grid(ctx, wid,
                                wid_editor_map_thing_replace_template);
    } else {
        rc = demarshal_wid_grid(ctx, wid,
                                wid_game_map_server_replace_tile);
    }

    wid_editor_map_loading = false;

    map_fixup(level);

    if (level_is_editor(level)) {
        wid_update(wid_editor_map_grid_container);
    } else {
        wid_update(wid_game_map_server_grid_container);
    }

    GET_KET(ctx);

    return (rc);

}
