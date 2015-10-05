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

static const char *default_trigger = "white";

/*
 * Find an existing trigger and which slot it is in
 */
int level_trigger2slot (levelp level, const char *name)
{
    if (!name) {
        name = default_trigger;
    }

    int i;

    /*
     * See if the trigger exists
     */
    for (i = 0; i < ARRAY_SIZE(level->trigger); i++) {
        if (!level->trigger[i].name) {
            break;
        }

        if (!strcmp(level->trigger[i].name, name)) {
            return (i);
        }
    }

    return (-1);
}

/*
 * Map a trigger to a slot number if it has not been done so already
 */
void level_trigger_alloc (levelp level, const char *name)
{
    if (!name) {
        name = default_trigger;
    }

    int i;

    if (level_trigger2slot(level, name) != -1) {
        return;
    }

    /*
     * See if the trigger exists
     */
    for (i = 0; i < ARRAY_SIZE(level->trigger); i++) {
        if (!level->trigger[i].name) {
            level->trigger[i].name = name;

            LEVEL_LOG(level, "Allocate trigger, %s", name);
            return;
        }
    }

    ERR("out of level triggers trying to add %s", name);
}

static void level_trigger_mark_activate (levelp level, const char *name)
{
    if (!name) {
        name = default_trigger;
    }

    int slot;

    slot = level_trigger2slot(level, name);
    if (slot == -1) {
        ERR("Cannot activate trigger %s", name);
    }

    level->trigger[slot].activated = 1;
}

static void level_trigger_mark_activate_exists (levelp level, const char *name)
{
    if (!name) {
        name = default_trigger;
    }

    int slot;

    slot = level_trigger2slot(level, name);
    if (slot == -1) {
        ERR("cannot activate name %s", name);
    }

    level->trigger[slot].activate_exists = 1;
}

int level_trigger_is_activated (levelp level, const char *name)
{
    if (!name) {
        name = default_trigger;
    }

    int slot;

    slot = level_trigger2slot(level, name);
    if (slot == -1) {
        ERR("cannot check if name %s is activated, never allocated", name);
    }

    return (level->trigger[slot].activated);
}

void level_trigger_activate (levelp level, const char *name)
{
    int x, y, z;
    int spawned = false;
    int zapped = false;

    if (level_trigger_is_activated(level, name)) {
        return;
    }

    LEVEL_LOG(level, "Activate trigger, %s", name);

    level_trigger_mark_activate(level, name);

    /*
     * Look for any items to be spawned.
     *
     * We don't need to activate movement tiles as they will be activated by 
     * the move tiles themselves during collision testing.
     */
    for (x = 0; x < MAP_WIDTH; x++) {
        for (y = 0; y < MAP_HEIGHT; y++) {

            level_map_tile *tile = 
                            &level->map_grid.tile[x][y][MAP_DEPTH_ACTIONS];

            tpp tile_tp = tile->tp;
            if (!tile_tp) {
                continue;
            }

            const char *it_trigger = tile->data.col_name;
            if (!it_trigger) {
                it_trigger = default_trigger;
            }

            if (strcmp(it_trigger, name)) {
                continue;
            }

            /*
             * Find the thing with the message to broadcast.
             */
            if (tp_is_action_text(tile_tp)) {
                thingp t = map_is_tp_at(level, x, y, tile_tp);
                if (t) {
                    if (!t->data) {
                        ERR("expecting data for thing %s", thing_logname(t));
                    }

                    if (t->data->text[0]) {
                        MSG_SERVER_SHOUT_AT_ALL_PLAYERS(POPUP,
                                                        0, 0, // x, y
                                                        "%s", t->data->text);
                    } else {
                        ERR("text trigger exists at %d %d and has no text", x, y);
                    }
                }
            }

            /*
             * Look for the original thing that it looks like we wanted to 
             * destroy. Then try and find it on the map.
             */
            tpp triggerd_tp = 0;

            for (z = MAP_DEPTH_ACTIONS - 1; z >= 0; z--) {
                triggerd_tp = level->map_grid.tile[x][y][z].tp;
                if (triggerd_tp) {
                    break;
                }
            }

            if (!triggerd_tp) {
                continue;
            }

            /*
             * Any sleeping things that need to be awoken? Actually they don't 
             * exist yet, so we need to create them. It's best not to waste 
             * resources with sleeping things until they are needed.
             */
            if (tp_is_action_sleep(tile_tp)) {
                LEVEL_LOG(level, "Spawn %s via movement trigger %s",
                          tp_name(triggerd_tp), name);

                widp w = wid_game_map_server_replace_tile(level_get_map(level),
                                                          x,
                                                          y,
                                                          0, /* thing */
                                                          triggerd_tp,
                                                          0, /* tpp data */
                                                          0 /* item */,
                                                          0 /* stats */);
                /*
                 * For things like bombs and the like, make them active.
                 */
                thingp t = wid_get_thing(w);
                if (t) {
                    thing_wake(t);
                    thing_make_active(t);
                }

                spawned = 1;
            }

            /*
             * Any things that need to be zapped?
             */
            if (tp_is_action_zap(tile_tp)) {
                thingp t = map_is_tp_at(level, x, y, triggerd_tp);
                if (t) {
                    LEVEL_LOG(level, "Kill %s via movement trigger %s",
                              thing_logname(t), name);

                    thing_dead(t, 0, "killed by zap trigger");

                    zapped = true;
                }
            }

            /*
             * Activate any blocks sitting on movement actions. This will
             * allow them to do collision testing with the action block and
             * then move.
             */
            if (tp_is_action_left(tile_tp)  ||
                tp_is_action_right(tile_tp) ||
                tp_is_action_up(tile_tp)    ||
                tp_is_action_down(tile_tp)) {

                thingp t = map_is_tp_at(level, x, y, triggerd_tp);
                if (t) {
                    LEVEL_LOG(level, "Active %s via movement trigger %s",
                              thing_logname(t), name);

                    level_trigger_move_thing(tile_tp, t);
                }
            }
        }
    }

    if (spawned) {
        sound_play_slime();
        level_set_needs_updating(level, true);
    }

    if (zapped) {
        level_set_needs_updating(level, true);
    }
}

/*
 * Activate any triggers that don't have anything to activate them! i.e. a 
 * move trigger without any activate triogger.
 */
void level_trigger_activate_default_triggers (levelp level)
{
    int x, y;

    level_trigger_alloc(level, default_trigger);

    for (x = 0; x < MAP_WIDTH; x++) {
        for (y = 0; y < MAP_HEIGHT; y++) {

            level_map_tile *tile = 
                            &level->map_grid.tile[x][y][MAP_DEPTH_ACTIONS];

            tpp it = tile->tp;
            if (!it) {
                continue;
            }

            if (tp_is_action(it)) {
                const char *it_trigger = tile->data.col_name;

                LEVEL_LOG(level, "Found action color %s", it_trigger);

                level_trigger_alloc(level, it_trigger);
            }

            if (tp_is_action_trigger(it)) {
                const char *it_trigger = tile->data.col_name;

                LEVEL_LOG(level, "Found action trigger, %s", it_trigger);

                level_trigger_mark_activate_exists(level, it_trigger);
                continue;
            }
        }
    }

    int i;

    for (i = 0; i < ARRAY_SIZE(level->trigger); i++) {
        if (!level->trigger[i].name) {
            break;
        }

        if (!level->trigger[i].activate_exists) {
            /*
             * If nothing exists to activate it, activate it now.
             */
            LEVEL_LOG(level, "No action trigger exists, activate trigger, %s",
                      level->trigger[i].name);

            level_trigger_activate(level, level->trigger[i].name);
        } else {
            /*
             * Else we wait to be activated.
             */
            LEVEL_LOG(level, "Sleeping trigger, %s", level->trigger[i].name);
        }
    }
}

/*
 * A thing has stepped on a movement trigger. Make it move.
 */
void level_trigger_move_thing (tpp me, thingp t)
{
    thing_make_active(t);

    double speed = 1.0;

    /*
     * A trigger on top of a floor tile usually means the trigger is going to 
     * be used for something that moves under the tile. If we remove this 
     * check then the floor tile itself will move.
     */
    if (thing_is_dungeon_floor(t)) {
        return;
    }

    if (thing_is_world_floor(t)) {
        return;
    }

    if (tp_is_action_left(me)) {
        t->dx = -speed;
        t->dy = 0;
        thing_set_dir_left(t);
    } else if (tp_is_action_right(me)) {
        t->dx = speed;
        t->dy = 0;
        thing_set_dir_right(t);
    } else if (tp_is_action_up(me)) {
        t->dx = 0;
        t->dy = -speed;
        thing_set_dir_up(t);
    } else if (tp_is_action_down(me)) {
        t->dx = 0;
        t->dy = speed;
        thing_set_dir_down(t);
    }
}
