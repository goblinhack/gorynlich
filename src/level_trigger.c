/*
 * Copyright (C) 2011 Neil McGill
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

static const char *default_trigger = "default_trigger";

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

    DIE("out of level triggers trying to add %s", name);
}

static void level_trigger_mark_activate (levelp level, const char *name)
{
    if (!name) {
        name = default_trigger;
    }

    int slot;

    slot = level_trigger2slot(level, name);
    if (slot == -1) {
        DIE("Cannot activate trigger %s", name);
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
        DIE("cannot activate name %s", name);
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
        DIE("cannot check if name %s is activated, never allocated", name);
    }

    return (level->trigger[slot].activated);
}

void level_trigger_activate (levelp level, const char *name)
{
    int x, y, z;
    int spawned = 0;

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

            if (it_trigger != name) {
                continue;
            }

            if (tp_is_action_spawn(tile_tp)) {
                for (z = MAP_DEPTH_ACTIONS - 1; z > 0; z--) {
                    tpp spawn = level->map_grid.tile[x][y][z].tp;
                    if (!spawn) {
                        continue;
                    }

                    wid_game_map_server_replace_tile(level_get_map(level),
                                                    x,
                                                    y,
                                                    0, /* thing */
                                                    spawn,
                                                    0, /* tpp data */
                                                    0 /* item */,
                                                    0 /* stats */);
                    spawned = 1;
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

                thing_map *map = &thing_server_map;
                thing_map_cell *cell = &map->cells[x][y];

                uint32_t i;
                for (i = 0; i < cell->count; i++) {
                    thingp t;
                    
                    t = thing_server_id(cell->id[i]);

                    if (thing_is_wall(t)) {
                        LEVEL_LOG(level, "Active %s via movement trigger %s",
                                  thing_logname(t), name);

                        level_trigger_move_thing(tile_tp, t);
                    }
                }
            }
        }
    }

    if (spawned) {
        sound_play_slime();
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
