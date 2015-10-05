/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */

#include "slre.h"

#include "main.h"
#include "thing.h"
#include "thing_tile.h"
#include "thing_tile_private.h"
#include "tile.h"
#include "time_util.h"
#include "command.h"

void thing_animate (thingp t)
{
    thing_tilep tile;
    thing_tilep otile;
    tree_rootp tiles;
    const char *command;
    tpp tp = thing_tp(t);
    widp w = t->wid;

    otile = tile = thing_current_tile(t);

    /*
     * Allow map join to choose the type of connector.
     */
    if (thing_is_joinable(t)) {
        if (!otile) {
            return;
        }
    }

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
                if (thing_tile_is_dead_on_end_of_anim(tile) ||
                    thing_tile_is_dead_on_end_of_anim_on_server_only(tile)) {
                    thing_dead(t, 0, "end of anim");
                }
            } else {
                if (thing_tile_is_dead_on_end_of_anim(tile)) {
                    if (!thing_tile_is_dead_on_end_of_anim_on_server_only(tile)) {
                        thing_dead(t, 0, "end of anim");
                    }
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
        } else if (thing_is_jumping(t) && 
                   thing_is_effect_rotate_2way(t)) {
            /*
             * Jumping and doesn't use directions, so just stick to jump 
             * animation frames/
             */
            if (!thing_tile_is_jumping(tile)) {
                tile = thing_tile_next(tiles, tile);
                continue;
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
        } else if (thing_is_joinable(t)) {
            if (otile) {
                if (thing_tile_is_join_block(otile) &&
                    !thing_tile_is_join_block(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_horiz(otile) &&
                    !thing_tile_is_join_horiz(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_vert(otile) &&
                    !thing_tile_is_join_vert(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_node(otile) &&
                    !thing_tile_is_join_node(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_left(otile) &&
                    !thing_tile_is_join_left(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_right(otile) &&
                    !thing_tile_is_join_right(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_top(otile) &&
                    !thing_tile_is_join_top(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_bot(otile) &&
                    !thing_tile_is_join_bot(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_tl(otile) &&
                    !thing_tile_is_join_tl(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_tr(otile) &&
                    !thing_tile_is_join_tr(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_bl(otile) &&
                    !thing_tile_is_join_bl(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_br(otile) &&
                    !thing_tile_is_join_br(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_t(otile) &&
                    !thing_tile_is_join_t(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_t90(otile) &&
                    !thing_tile_is_join_t90(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_t180(otile) &&
                    !thing_tile_is_join_t180(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_t270(otile) &&
                    !thing_tile_is_join_t270(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_x(otile) &&
                    !thing_tile_is_join_x(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_tl2(otile) &&
                    !thing_tile_is_join_tl2(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_tr2(otile) &&
                    !thing_tile_is_join_tr2(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_bl2(otile) &&
                    !thing_tile_is_join_bl2(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_br2(otile) &&
                    !thing_tile_is_join_br2(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_t_1(otile) &&
                    !thing_tile_is_join_t_1(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_t_2(otile) &&
                    !thing_tile_is_join_t_2(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_t_3(otile) &&
                    !thing_tile_is_join_t_3(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_t90_1(otile) &&
                    !thing_tile_is_join_t90_1(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_t90_2(otile) &&
                    !thing_tile_is_join_t90_2(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_t90_3(otile) &&
                    !thing_tile_is_join_t90_3(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_t180_1(otile) &&
                    !thing_tile_is_join_t180_1(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_t180_2(otile) &&
                    !thing_tile_is_join_t180_2(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_t180_3(otile) &&
                    !thing_tile_is_join_t180_3(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_t270_1(otile) &&
                    !thing_tile_is_join_t270_1(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_t270_2(otile) &&
                    !thing_tile_is_join_t270_2(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_t270_3(otile) &&
                    !thing_tile_is_join_t270_3(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_x(otile) &&
                    !thing_tile_is_join_x(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_x1(otile) &&
                    !thing_tile_is_join_x1(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_x1_270(otile) &&
                    !thing_tile_is_join_x1_270(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_x1_180(otile) &&
                    !thing_tile_is_join_x1_180(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_x1_90(otile) &&
                    !thing_tile_is_join_x1_90(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_x2(otile) &&
                    !thing_tile_is_join_x2(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_x2_270(otile) &&
                    !thing_tile_is_join_x2_270(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_x2_180(otile) &&
                    !thing_tile_is_join_x2_180(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_x2_90(otile) &&
                    !thing_tile_is_join_x2_90(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_x3(otile) &&
                    !thing_tile_is_join_x3(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_x3_180(otile) &&
                    !thing_tile_is_join_x3_180(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_x4(otile) &&
                    !thing_tile_is_join_x4(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_x4_270(otile) &&
                    !thing_tile_is_join_x4_270(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_x4_180(otile) &&
                    !thing_tile_is_join_x4_180(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_x4_90(otile) &&
                    !thing_tile_is_join_x4_90(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_horiz2(otile) &&
                    !thing_tile_is_join_horiz2(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }

                if (thing_tile_is_join_vert2(otile) &&
                    !thing_tile_is_join_vert2(tile)) {
                    tile = thing_tile_next(tiles, tile);
                    continue;
                }
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
        if (!tile->tile) {
            ERR("cannot find tile %s", thing_tile_name(tile));
            return;
        }
    }

#if 0
    if (tile && otile) {
        if(thing_is_joinable(t)) {
            CON("%s-> %s", thing_tile_name(otile), thing_tile_name(tile));
        }
    }
#endif

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
//CON("%s", tile->tilename);
        /*
         * Send a jump on the next move to the client?
         */
        if (thing_tile_begin_jump(tile)) {
            t->one_shot_move = true;
            t->want_to_jump = true;
        }
    } else {
        /*
         * End of jump on the client?
         */
        if (t->is_jumping) {
            if (!wid_is_moving(w)) {
                t->is_jumping = false;
//CON("end of jump");
            }
        }
    }
}
