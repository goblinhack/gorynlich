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

    if (!level->trigger[0].name) {
        level->trigger[0].name = default_trigger;
    }

    /*
     * See if the trigger exists
     */
    for (i = 0; i < ARRAY_SIZE(level->trigger); i++) {
        if (!level->trigger[i].name) {
            level->trigger[i].name = name;

            LOG("Allocate trigger, %s", name);
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

void level_trigger_activate (levelp level, thingp it)
{
    int x, y, z;
    int spawned = 0;

    const char *name = it->data.col_name;
    if (!name) {
        name = default_trigger;
    }


    if (level_trigger_is_activated(level, name)) {
        return;
    }

    level_trigger_mark_activate(level, name);

    LOG("Activate trigger, %s", name);

    /*
     * Look for any items to be spawned.
     *
     * We don't need to activate movement tiles as they will be activated by 
     * the move tiles themselves during collision testing.
     */
    for (x = 0; x < MAP_WIDTH; x++) {
        for (y = 0; y < MAP_HEIGHT; y++) {

            level_map_tile *tile = &level->map_grid.tile[x][y][MAP_DEPTH_ACTIONS];

            tpp it = tile->tp;
            if (!it) {
                continue;
            }

            const char *it_trigger = tile->data.col_name;
            if (it_trigger != name) {
                continue;
            }

            if (!tp_is_action_spawn(it)) {
                continue;
            }

            for (z = MAP_DEPTH_ACTIONS - 1; z > 0; z--) {
                tpp it = level->map_grid.tile[x][y][z].tp;
                if (!it) {
                    continue;
                }

                wid_game_map_server_replace_tile(level_get_map(level),
                                                 x,
                                                 y,
                                                 0, /* thing */
                                                 it,
                                                 0, /* tpp data */
                                                 0 /* item */,
                                                 0 /* stats */);
                spawned = 1;

                break;
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

    for (x = 0; x < MAP_WIDTH; x++) {
        for (y = 0; y < MAP_HEIGHT; y++) {

            level_map_tile *tile = &level->map_grid.tile[x][y][MAP_DEPTH_ACTIONS];

            tpp it = tile->tp;
            if (!it) {
                continue;
            }

            if (tp_is_action_trigger(it)) {
                const char *it_trigger = tile->data.col_name;

                LOG("Found level trigger, %s", it_trigger);

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
            LOG("Activate default trigger, %s", level->trigger[i].name);

            level->trigger[i].activated = 1;
        } else {
            /*
             * Else we wait to be activated.
             */
            LOG("Sleeping trigger, %s", level->trigger[i].name);
        }
    }
}
