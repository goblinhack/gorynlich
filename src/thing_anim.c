/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>
#include "slre.h"

#include "main.h"
#include "tree.h"
#include "thing.h"
#include "thing_private.h"
#include "thing_template.h"
#include "thing_tile.h"
#include "wid.h"
#include "marshal.h"
#include "time.h"
#include "command.h"

void thing_animate (thingp t)
{
    thing_tilep tile;
    tree_rootp tiles;
    const char *command;

    tile = thing_current_tile(t);

    if (tile) {
        /*
         * If within the animate time of this frame, keep with it.
         */
        if (t->timestamp_change_to_next_frame > time_get_time_cached()) {
            return;
        }

        /*
         * Stop the animation here?
         */
        if (thing_tile_is_end_of_anim(tile)) {
            return;
        }
    }

    tiles = thing_tile_tiles(t);
    if (!tiles) {
        return;
    }

    /*
     * Get the next tile.
     */
    if (tile) {
        tile = thing_tile_next(tiles, tile);
    }

    /*
     * Find a tile that matches the things current mode.
     */
    uint32_t size = tree_root_size(tiles);
    uint32_t tries = 0;

    while (tries < size) {
        tries++;

        /*
         * Cater for wraps.
         */
        if (!tile) {
            tile = thing_tile_first(tiles);
        }

        if (thing_is_open(t)) {
            if (!thing_tile_is_open(tile)) {
                tile = thing_tile_next(tiles, tile);
                continue;
            }
        } else if (thing_is_dead(t)) {
            if (!thing_tile_is_dead(tile)) {
                tile = thing_tile_next(tiles, tile);
                continue;
            }
        } else {
            if (thing_tile_is_dead(tile)) {
                tile = thing_tile_next(tiles, tile);
                continue;
            }

            if (thing_tile_is_open(tile)) {
                tile = thing_tile_next(tiles, tile);
                continue;
            }
        }

        break;
    }

    if (!tile) {
        return;
    }

    /*
     * Use this tile!
     */
    t->current_tile = tile;

    wid_set_tilename(t->wid, thing_tile_name(tile));

    /*
     * When does this tile expire ?
     */
    t->timestamp_change_to_next_frame =
                    time_get_time_cached() + thing_tile_delay(tile);

    command = thing_tile_command(tile);
    if (command) {
        command_handle(command,
                       0 /* expanded text */,
                       false /* show ambiguous */,
                       false /* show complete */,
                       true /* execute command */,
                       (void*) t /* context */);
    }
}
