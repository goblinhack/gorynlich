/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>
#include "slre.h"

#include "main.h"
#include "thing.h"
#include "thing_tile.h"
#include "wid.h"
#include "time.h"
#include "wid.h"

void wid_animate (widp w)
{
    if (!w->animate) {
        return;
    }

    tpp tp = wid_get_thing_template(w);
    if (!tp) {
        return;
    }

    if (!tp_is_animated(tp)) {
        return;
    }

    thingp t = wid_get_thing(w);

    thing_tilep tile;
    tree_rootp tiles;

    tile = w->current_tile;

    if (tile) {
        /*
         * If within the animate time of this frame, keep with it.
         */
        if (w->timestamp_change_to_next_frame > time_get_time_cached()) {
            return;
        }

        /*
         * Stop the animation here?
         */
        if (thing_tile_is_end_of_anim(tile)) {
            if (thing_tile_is_dead_on_end_of_anim(tile)) {
                return;
            }
            return;
        }
    }

    tiles = tp_get_tiles(tp);
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

        if (t && thing_is_dir_tl(t)) {
            if (!thing_tile_is_dir_tl(tile)) {
                tile = thing_tile_next(tiles, tile);
                continue;
            }
        } else if (t && thing_is_dir_bl(t)) {
            if (!thing_tile_is_dir_bl(tile)) {
                tile = thing_tile_next(tiles, tile);
                continue;
            }
        } else if (t && thing_is_dir_br(t)) {
            if (!thing_tile_is_dir_br(tile)) {
                tile = thing_tile_next(tiles, tile);
                continue;
            }
        } else if (t && thing_is_dir_tr(t)) {
            if (!thing_tile_is_dir_tr(tile)) {
                tile = thing_tile_next(tiles, tile);
                continue;
            }
        } else if (t && thing_is_dir_up(t)) {
            if (!thing_tile_is_dir_up(tile)) {
                tile = thing_tile_next(tiles, tile);
                continue;
            }
        } else if (t && thing_is_dir_down(t)) {
            if (!thing_tile_is_dir_down(tile)) {
                tile = thing_tile_next(tiles, tile);
                continue;
            }
        } else if (t && thing_is_dir_left(t)) {
            if (!thing_tile_is_dir_left(tile)) {
                tile = thing_tile_next(tiles, tile);
                continue;
            }
        } else if (t && thing_is_dir_right(t)) {
            if (!thing_tile_is_dir_right(tile)) {
                tile = thing_tile_next(tiles, tile);
                continue;
            }
        } else if (t && thing_is_open(t)) {
            if (!thing_tile_is_open(tile)) {
                tile = thing_tile_next(tiles, tile);
                continue;
            }
        } else if (t && thing_is_dead(t)) {
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
    w->current_tile = tile;

    wid_set_tilename(w, thing_tile_name(tile));

    /*
     * When does this tile expire ?
     */
    uint32_t delay = thing_tile_delay(tile);
    if (delay) {
        delay = rand() % delay;
    }

    w->timestamp_change_to_next_frame = time_get_time_cached() + delay;
}
