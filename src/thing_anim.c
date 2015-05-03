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
#include "thing_tile_private.h"
#include "tile.h"
#include "wid.h"
#include "time_util.h"
#include "command.h"

void thing_animate (thingp t)
{
    thing_tilep tile;
    tree_rootp tiles;
    const char *command;
    tpp tp = thing_tp(t);
    widp w = t->wid;

    tile = thing_current_tile(t);

    if (tile) {
        /*
         * If within the animate time of this frame, keep with it.
         */
        if (t->timestamp_change_to_next_frame > time_get_time_ms()) {
            return;
        }

        /*
         * Stop the animation here?
         */
        if (thing_tile_is_end_of_anim(tile)) {
            if (t->on_server) {
                if (thing_tile_is_dead_on_end_of_anim_on_server_only(tile)) {
                    thing_dead(t, 0, "end of anim");
                    return;
                }
            } else {
                if (thing_tile_is_dead_on_end_of_anim(tile)) {
                    thing_dead(t, 0, "end of anim");
                    return;
                }
            }

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
        if ((t->anim_x != t->x) || (t->anim_y != t->y)) {
            t->anim_x = t->x;
            t->anim_y = t->y;
            t->is_moving = true;
        } else {
            t->is_moving = false;
        }

        if (thing_is_animate_only_when_moving_noverify(t) &&
            /*
             * Let dying things decay. I guess they sort of move... urgh.
             */
            !thing_is_dead_or_dying(t) &&
            !t->is_moving) {
            /*
             * Same tile.
             */
        } else {
            tile = thing_tile_next(tiles, tile);
        }
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

        if (thing_is_sleeping(t)) {
            if (!thing_tile_is_sleeping(tile)) {
                tile = thing_tile_next(tiles, tile);
                continue;
            }
        } else if (thing_is_dying(t)) {
            if (!thing_tile_is_dying(tile)) {
                tile = thing_tile_next(tiles, tile);
                continue;
            }
        } else if (thing_tile_is_dying(tile)) {
            tile = thing_tile_next(tiles, tile);
            continue;
        } else if (tp_is_effect_rotate_2way(tp)) {
            if (wid_is_moving(w)) {
                if (!thing_tile_is_moving(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }
            }

            if (!wid_is_moving(w)) {
                if (thing_tile_is_moving(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }
            }
        } else if (tp->has_dir_anim && thing_is_dir_tl(t)) {
            if (!thing_tile_is_dir_tl(tile)) {
                tile = thing_tile_next(tiles, tile);
                continue;
            }
        } else if (tp->has_dir_anim && thing_is_dir_bl(t)) {
            if (!thing_tile_is_dir_bl(tile)) {
                tile = thing_tile_next(tiles, tile);
                continue;
            }
        } else if (tp->has_dir_anim && thing_is_dir_br(t)) {
            if (!thing_tile_is_dir_br(tile)) {
                tile = thing_tile_next(tiles, tile);
                continue;
            }
        } else if (tp->has_dir_anim && thing_is_dir_tr(t)) {
            if (!thing_tile_is_dir_tr(tile)) {
                tile = thing_tile_next(tiles, tile);
                continue;
            }
        } else if (tp->has_dir_anim && thing_is_dir_up(t)) {
            if (!thing_tile_is_dir_up(tile)) {
                tile = thing_tile_next(tiles, tile);
                continue;
            }
        } else if (tp->has_dir_anim && thing_is_dir_down(t)) {
            if (!thing_tile_is_dir_down(tile)) {
                tile = thing_tile_next(tiles, tile);
                continue;
            }
        } else if (tp->has_dir_anim && thing_is_dir_left(t)) {
            if (!thing_tile_is_dir_left(tile)) {
                tile = thing_tile_next(tiles, tile);
                continue;
            }
        } else if (tp->has_dir_anim && thing_is_dir_right(t)) {
            if (!thing_tile_is_dir_right(tile)) {
                tile = thing_tile_next(tiles, tile);
                continue;
            }
        } else if (thing_is_open(t)) {
            if (!thing_tile_is_open(tile)) {
                tile = thing_tile_next(tiles, tile);
                continue;
            }
        } else {
            if (thing_tile_is_sleeping(tile)) {
                tile = thing_tile_next(tiles, tile);
                continue;
            }

            if (thing_tile_is_dying(tile)) {
                tile = thing_tile_next(tiles, tile);
                continue;
            }

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

    if (!tile->tile) {
        tile->tile = tile_find(thing_tile_name(tile));
    }

    wid_set_tile(t->wid, tile->tile);

    /*
     * When does this tile expire ?
     */
    uint32_t delay = thing_tile_delay(tile);
    if (delay) {
        delay = myrand() % delay;
    }

    t->timestamp_change_to_next_frame = time_get_time_ms() + delay;

    command = thing_tile_command(tile);
    if (command) {
        command_handle(command,
                       0 /* expanded text */,
                       false /* show ambiguous */,
                       false /* show complete */,
                       true /* execute command */,
                       (void*) t /* context */);
    }

    /*
     * These are actions to do on the server. If they appear on the client it 
     * is only because we sync those values over.
     */
    if (t->on_server) {
        /*
         * Send a jump on the next move to the client?
         */
        if (thing_tile_begin_jump(tile)) {
            t->one_shot_move = true;
            t->is_jumping = true;
        }
    } else {
        /*
         * End of jump on the client?
         */
        if (!wid_is_moving(w)) {
            t->is_jumping = false;
        }
    }
}
