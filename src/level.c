/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>
#include <errno.h>
#include <time.h>
#include "slre.h"

#include "main.h"
#include "string.h"
#include "marshal.h"
#include "wid.h"
#include "wid_popup.h"
#include "wid_textbox.h"
#include "gl.h"
#include "color.h"
#include "level.h"
#include "level_private.h"
#include "tile.h"
#include "player.h"
#include "thing.h"
#include "thing_template.h"
#include "thing_tile.h"
#include "command.h"
#include "wid_button.h"
#include "time.h"
#include "marshal.h"
#include "wid_game_map.h"
#include "wid_editor_map.h"
#include "sdl.h"
#include "map.h"
#include "timer.h"
#include "wid_hiscore.h"
#include "music.h"
#include "sound.h"
#include "wid_console.h"
#include "wid_editor.h"

static boolean level_command_dead(tokens_t *tokens, void *context);
static boolean level_init_done;
static void level_start_timers(levelp level);
boolean game_over;

boolean god_mode = false;
uint32_t start_level = 1;
uint32_t start_lives = 3;

/*
 * Various level timers.
 */
tree_rootp timers;

/*
 * Used so we only do the slow countdown first time on each level.
 */
static boolean level_command_lives (tokens_t *tokens, void *context)
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

static boolean level_command_play (tokens_t *tokens, void *context)
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
            level_restart(level);

            wid_hide(wid_console_window, 0);
        }
    }

    return (true);
}

static boolean level_command_god_mode (tokens_t *tokens, void *context)
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

boolean level_init (void)
{
    level_init_done = true;

    if (is_server) {
        command_add(level_command_dead, 
                    "dead", "internal command for thing suicide");

        if (0) {
        command_add(level_command_god_mode, 
                    "set godmode [01]", "TBD enable/disable god mode");

        command_add(level_command_play, 
                    "play [123456789]+", "TBD play level x");

        command_add(level_command_lives, 
                    "set lives [123456789]+", "TBD set player number of lives");
        }
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
static boolean level_command_dead (tokens_t *tokens, void *context)
{
    thingp t;
    levelp level;

    if (!context) {
        return (false);
    }

    t = (typeof(t)) context;
    verify(t);

    level = thing_level(t);
    verify(level);

    if (thing_is_xxx17(t)) {
        thing_dead(t, 0 /* killer */, "finished blowing up");
        return (true);
    }

    if (thing_is_bomb(t)) {
        level_place_explosion(level, thing_grid_x(t), thing_grid_y(t));
        thing_dead(t, 0 /* killer */, "blew up");
        return (true);
    }


    if (thing_is_seedpod(t)) {
        thing_templatep thing_template = thing_template_find("data/things/plant");

        wid_game_map_replace_tile(wid_game_map_grid_container,
                                  thing_grid_x(t),
                                  thing_grid_y(t),
                                  0, /* give to player count */
                                  thing_template);

        sound_play_slime();

        thing_dead(t, 0 /* killer */, "turned into plant");
        return (true);
    }

    return (true);
}

levelp level_new (widp map, uint32_t level_no)
{
    levelp level;

    level = (typeof(level)) myzalloc(sizeof(*level), "level");

    level_set_map(level, map);
    level_set_is_editor(level, true);
    level_set_level_no(level, level_no);

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
     * Ensure no stale pointers.
     */
    if (level == level_game) {
        level_game = 0;
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

    wid_game_map_wid_destroy();

    myfree(level);
}

levelp level_load (uint32_t level_no, widp wid)
{
    levelp level;

    srand(level_no);

    level = level_new(wid, level_no);

    level_set_is_paused(level, true);
    level_set_is_editor(level, false);
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

        (void) wid_popup_error(popup_str);
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

            (void) wid_popup_error(popup_str);
            myfree(popup_str);
        }

        demarshal_fini(in);
    }

    myfree(dir_and_file);

    level_set_walls(level);
    level_set_monst_walls(level);
    level_set_doors(level);
    level_set_pipes(level);
    level_pipe_find_ends(level);

    level_set_is_paused(level, false);
    level_start_timers(level);

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

    for (x = 0; x < TILES_MAP_EDITABLE_WIDTH; x++) {
        for (y = 0; y < TILES_MAP_EDITABLE_HEIGHT; y++) {
            if (map_is_wall_at(level, x, y) ||
                !map_is_floor_at(level, x, y)) {
                level->walls[x][y] = '+';
            } else {
                level->walls[x][y] = ' ';
            }
        }
    }
}

/*
 * Or other things we collide with.
 */
void level_set_monst_walls (levelp level)
{
    int32_t x;
    int32_t y;

    for (x = 0; x < TILES_MAP_EDITABLE_WIDTH; x++) {
        for (y = 0; y < TILES_MAP_EDITABLE_HEIGHT; y++) {
            if (map_is_wall_at(level, x, y) ||
                map_is_bomb_at(level, x, y) ||
                map_is_exit_at(level, x, y) ||
                map_is_spam_at(level, x, y) ||
                !map_is_floor_at(level, x, y)) {
                level->monst_walls[x][y] = '+';
            } else {
                level->monst_walls[x][y] = ' ';
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

    for (x = 0; x < TILES_MAP_EDITABLE_WIDTH; x++) {
        for (y = 0; y < TILES_MAP_EDITABLE_HEIGHT; y++) {
            if (map_is_door_at(level, x, y)) {
                level->roads[x][y] = '+';
            } else {
                level->roads[x][y] = ' ';
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

    for (x = 0; x < TILES_MAP_EDITABLE_WIDTH; x++) {
        for (y = 0; y < TILES_MAP_EDITABLE_HEIGHT; y++) {
            if (map_is_pipe_at(level, x, y)) {
                level->pipes[x][y] = '+';
            } else {
                level->pipes[x][y] = ' ';
            }
        }
    }
}

void level_pipe_find_ends (levelp level)
{
    int32_t x;
    int32_t y;

    memset(level->end_pipe, ' ', sizeof(level->end_pipe));

    for (x = 1; x < TILES_MAP_EDITABLE_WIDTH-1; x++) {
        for (y = 1; y < TILES_MAP_EDITABLE_HEIGHT-1; y++) {

            if (level->pipes[x][y] == ' ') {
                continue;
            }

            level->walls[x][y] = '+';

            //  .
            // .x.
            // ?x?
            if ((level->pipes[x][y+1] == '+') &&
                (level->pipes[x][y-1] == ' ') &&
                (level->pipes[x+1][y] == ' ') &&
                (level->pipes[x+1][y] == ' ')) {
                level->end_pipe[x][y] = '+';
                level->walls[x][y] = ' ';
            }

            // ?x?
            // .x.
            //  .
            if ((level->pipes[x][y-1] == '+') &&
                (level->pipes[x][y+1] == ' ') &&
                (level->pipes[x+1][y] == ' ') &&
                (level->pipes[x+1][y] == ' ')) {
                level->end_pipe[x][y] = '+';
                level->walls[x][y] = ' ';
            }

            //  .?
            // .xx
            //  .?
            if ((level->pipes[x+1][y] == '+') &&
                (level->pipes[x-1][y] == ' ') &&
                (level->pipes[x][y-1] == ' ') &&
                (level->pipes[x][y+1] == ' ')) {
                level->end_pipe[x][y] = '+';
                level->walls[x][y] = ' ';
            }

            //  .?
            // .xx
            //  .?
            if ((level->pipes[x-1][y] == '+') &&
                (level->pipes[x+1][y] == ' ') &&
                (level->pipes[x][y-1] == ' ') &&
                (level->pipes[x][y+1] == ' ')) {
                level->walls[x][y] = ' ';
                level->end_pipe[x][y] = '+';
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
         i < TILES_MAP_EDITABLE_HEIGHT * TILES_MAP_EDITABLE_WIDTH; i++) {

        x = rand() % TILES_MAP_EDITABLE_WIDTH;
        y = rand() % TILES_MAP_EDITABLE_HEIGHT;

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

        wid_game_map_replace_tile(wid_game_map_grid_container,
                                  x,
                                  y,
                                  0, /* give to player count */
                                  thing_template);

        sound_play_slime();

        return;
    }
}

/*
 * Place an explosion
 */
static boolean level_place_explosion_at (levelp level,
                                         int32_t x, int32_t y, int32_t i)
{
    if (map_is_wall_at(level, x, y) ||
        map_is_pipe_at(level, x, y)) {
        return (false);
    }

    thing_place_context_t *context;

    context =
        (typeof(context)) myzalloc(sizeof(*context), "place thing");

    context->x = x;
    context->y = y;
    context->level = level;
    context->thing_template = thing_template_find("data/things/explosion");
    if (!context->thing_template) {
        DIE("no explosion");
    }

    action_timer_create(
            &timers,
            (action_timer_callback)thing_place,
            context,
            "place thing",
            i * 200,
            i * 100 /* jitter */);

    return (true);
}

/*
 * Place an explosion
 */
void level_place_explosion (levelp level, int32_t x, int32_t y)
{
    boolean u_ok = true;
    boolean d_ok = true;
    boolean l_ok = true;
    boolean r_ok = true;
    uint32_t i;

    level_place_explosion_at(level, x, y, 0);

    for (i = 0; i < 4; i++) {

        if (u_ok) {
            u_ok = level_place_explosion_at(level, x, y-i-1, i);
        }

        if (d_ok) {
            d_ok = level_place_explosion_at(level, x, y+i+1, i);
        }

        if (l_ok) {
            l_ok = level_place_explosion_at(level, x-i-1, y, i);
        }

        if (r_ok) {
            r_ok = level_place_explosion_at(level, x+i+1, y, i);
        }

        if (!u_ok && !d_ok && !l_ok && !r_ok) {
            return;
        }
    }
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

boolean level_is_hurryup (levelp level)
{
    verify(level);

    return (level->is_hurryup);
}

void level_set_is_hurryup (levelp level, boolean val)
{
    verify(level);

    level->is_hurryup = val;
}

boolean level_is_zzz1 (levelp level)
{
    verify(level);

    return (level->is_zzz1);
}

void level_set_is_zzz1 (levelp level, boolean val)
{
    verify(level);

    level->is_zzz1 = val;
}

boolean level_is_zzz2 (levelp level)
{
    verify(level);

    return (level->is_zzz2);
}

void level_set_is_zzz2 (levelp level, boolean val)
{
    verify(level);

    level->is_zzz2 = val;
}

boolean level_is_zzz3 (levelp level)
{
    verify(level);

    return (level->is_zzz3);
}

void level_set_is_zzz3 (levelp level, boolean val)
{
    verify(level);

    level->is_zzz3 = val;
}

boolean level_is_zzz4 (levelp level)
{
    verify(level);

    return (level->is_zzz4);
}

void level_set_is_zzz4 (levelp level, boolean val)
{
    verify(level);

    level->is_zzz4 = val;
}

boolean level_is_zzz5 (levelp level)
{
    verify(level);

    return (level->is_zzz5);
}

void level_set_is_zzz5 (levelp level, boolean val)
{
    verify(level);

    level->is_zzz5 = val;
}

boolean level_is_zzz6 (levelp level)
{
    verify(level);

    return (level->is_zzz6);
}

void level_set_is_zzz6 (levelp level, boolean val)
{
    verify(level);

    level->is_zzz6 = val;
}

boolean level_is_zzz7 (levelp level)
{
    verify(level);

    return (level->is_zzz7);
}

void level_set_is_zzz7 (levelp level, boolean val)
{
    verify(level);

    level->is_zzz7 = val;
}

boolean level_is_zzz8 (levelp level)
{
    verify(level);

    return (level->is_zzz8);
}

void level_set_is_zzz8 (levelp level, boolean val)
{
    verify(level);

    level->is_zzz8 = val;
}

boolean level_is_zzz9 (levelp level)
{
    verify(level);

    return (level->is_zzz9);
}

void level_set_is_zzz9 (levelp level, boolean val)
{
    verify(level);

    level->is_zzz9 = val;
}

boolean level_is_zzz10 (levelp level)
{
    verify(level);

    return (level->is_zzz10);
}

void level_set_is_zzz10 (levelp level, boolean val)
{
    verify(level);

    level->is_zzz10 = val;
}

boolean level_is_zzz11 (levelp level)
{
    verify(level);

    return (level->is_zzz11);
}

void level_set_is_zzz11 (levelp level, boolean val)
{
    verify(level);

    level->is_zzz11 = val;
}

boolean level_is_zzz12 (levelp level)
{
    verify(level);

    return (level->is_zzz12);
}

void level_set_is_zzz12 (levelp level, boolean val)
{
    verify(level);

    level->is_zzz12 = val;
}

boolean level_is_zzz13 (levelp level)
{
    verify(level);

    return (level->is_zzz13);
}

void level_set_is_zzz13 (levelp level, boolean val)
{
    verify(level);

    level->is_zzz13 = val;
}

boolean level_is_zzz14 (levelp level)
{
    verify(level);

    return (level->is_zzz14);
}

void level_set_is_zzz14 (levelp level, boolean val)
{
    verify(level);

    level->is_zzz14 = val;
}

boolean level_is_zzz15 (levelp level)
{
    verify(level);

    return (level->is_zzz15);
}

void level_set_is_zzz15 (levelp level, boolean val)
{
    verify(level);

    level->is_zzz15 = val;
}

boolean level_is_zzz16 (levelp level)
{
    verify(level);

    return (level->is_zzz16);
}

void level_set_is_zzz16 (levelp level, boolean val)
{
    verify(level);

    level->is_zzz16 = val;
}

boolean level_is_zzz17 (levelp level)
{
    verify(level);

    return (level->is_zzz17);
}

void level_set_is_zzz17 (levelp level, boolean val)
{
    verify(level);

    level->is_zzz17 = val;
}

boolean level_exit_reached_when_open (levelp level)
{
    verify(level);

    return (level->exit_reached_when_open);
}

void level_set_exit_reached_when_open (levelp level, boolean val)
{
    verify(level);

    level->exit_reached_when_open = val;
}

boolean level_get_warned_exit_not_open (levelp level)
{
    verify(level);

    return (level->warned_exit_not_open);
}

void level_set_warned_exit_not_open (levelp level, boolean val)
{
    verify(level);

    level->warned_exit_not_open = val;
}

boolean level_is_exit_open (levelp level)
{
    verify(level);

    return (level->is_exit_open);
}

void level_set_is_exit_open (levelp level, boolean val)
{
    verify(level);

    level->is_exit_open = val;
}

boolean level_is_editor (levelp level)
{
    verify(level);

    return (level->is_editor);
}

void level_set_is_editor (levelp level, boolean val)
{
    verify(level);

    level->is_editor = val;
}

boolean level_is_paused (levelp level)
{
    verify(level);

    return (level->is_paused);
}

void level_set_is_paused (levelp level, boolean val)
{
    verify(level);

    level->is_paused = val;
}

boolean level_is_frozen (levelp level)
{
    verify(level);

    return (level->is_frozen);
}

void level_set_is_frozen (levelp level, boolean val)
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

boolean demarshal_level (demarshal_p ctx, levelp level)
{
    boolean rc;
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
        GET_OPT_NAMED_BITFIELD(ctx, "exit_reached_when_open", level->exit_reached_when_open);
        GET_OPT_NAMED_BITFIELD(ctx, "warned_exit_not_open", level->warned_exit_not_open);
        GET_OPT_NAMED_BITFIELD(ctx, "is_exit_open", level->exit_reached_when_open);
    } while (demarshal_gotone(ctx));

    wid_editor_map_loading = true;

    if (level_is_editor(level)) {
        rc = demarshal_wid_grid(ctx, wid,
                                wid_editor_map_thing_replace_template);
    } else {
        rc = demarshal_wid_grid(ctx, wid,
                                wid_game_map_replace_tile);
    }

    wid_editor_map_loading = false;

    map_fixup(level);

    if (level_is_editor(level)) {
        wid_update(wid_editor_map_grid_container);
    } else {
        wid_update(wid_game_map_grid_container);
    }

    GET_KET(ctx);

    return (rc);

}
