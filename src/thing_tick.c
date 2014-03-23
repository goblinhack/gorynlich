/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "tree.h"
#include "thing_private.h"
#include "thing.h"
#include "thing_template.h"
#include "thing_timer.h"
#include "timer.h"
#include "wid.h"
#include "wid_game_map_client.h"
#include "wid_game_map_server.h"
#include "marshal.h"
#include "map.h"
#include "level.h"
#include "wid_console.h"
#include "time.h"
#include "string.h"
#include "color.h"
#include "sound.h"
#include "socket.h"

static void thing_tick_server_all (void)
{
    levelp level = server_level;
    if (!level) {
        return;
    }

    boolean scared;
    thingp t;

    /*
     * Allow level timers to fire.
     */
    level_tick(level);

    /*
     * Any timers waiting to fire?
     */
    if (thing_timers) {
        action_timers_tick(thing_timers);
    }

    /*
     * Do some things once per tich.
     */
    scared = false;

    if (player && thing_has_powerup_rocket_count(player)) {
        /*
         * Set monster scared flag.
         */
        scared = true;
    }

    TREE_WALK(server_active_things, t) {
        thing_templatep thing_template;
        widp w;

        /*
         * Sanity checks.
         */
        verify(t);
        thing_template = thing_get_template(t);

        w = t->wid;
        if (w) {
            verify(w);
        }

        /*
         * Thing has croaked it?
         */
        if (thing_is_dead(t)) {
            if (t->destroy_delay > 0) {
                if (--t->destroy_delay == 0) {
                    if (thing_is_left_as_corpse_on_death(t)) {
                        thing_bury(t);
                    } else {
                        thing_destroy(t, "died");
                    }
                }
            }
            continue;
        }

        /*
         * Thing is out of life?
         */
        if (thing_template_get_lifespan(thing_template)) {
            if (!t->timestamp_lifestamp) {
                /*
                 * When does this thing expire ?
                 */
                t->timestamp_lifestamp =
                        time_get_time_cached() +
                        thing_template_get_lifespan(thing_template);

                THING_LOG(t, "set end of life to %u", t->timestamp_lifestamp);

            } else if (time_get_time_cached() > t->timestamp_lifestamp) {
                thing_dead(t, 0, "out of life"); 
                continue;
            }
        }

        /*
         * Make things go faster if taking too long.
         */
        float speed = thing_template_get_speed(thing_template);

        if (level_is_hurryup(level)) {
            speed /= 2.0;
        }

        /*
         * Go faster!
         */
        if (w && thing_has_powerup_rocket_count(t)) {
            speed /= 2.0;
        }

        /*
         * Go slower!
         */
        if (thing_has_powerup_spam_count(t)) {
            speed *= 4.0;

            if (!t->timer_spam) {
                thing_action_timer_callback_spam(t);
            }
        }

        /*
         * Look for collisions.
         */
//        thing_handle_collisions(wid_game_map_server_grid_container, t);
        w = t->wid;

        /*
         * Died in a collision ? 8( Handle it next time around.
         */
        if (thing_is_dead(t)) {
            continue;
        }

        /*
         * If stopped moving, look for a next hop.
         */
        if (thing_is_monst(t) && speed && w && !wid_is_moving(w)) {
            LOG("NH");
            boolean look_for_nexthop = true;
            int32_t nexthop_x = -1;
            int32_t nexthop_y = -1;

            /*
             * Has something on the level changed that needs a research.
             */
            if (thing_redo_maze_search(t)) {
                thing_set_redo_maze_search(t, false);

                look_for_nexthop = true;
            }

            /*
             * Need to look for a nexthop? Or keep walking on?
             */
            boolean have_nexthop;
            if (look_for_nexthop) {
                have_nexthop = thing_find_nexthop(t, &nexthop_x, &nexthop_y);
            } else {
                have_nexthop = true;
            }

            if (have_nexthop) {
                t->dir = THING_DIR_NONE;

                widp wid_current_floor = wid_grid_find_thing_template(
                                            wid_game_map_server_grid_container,
                                            t->x,
                                            t->y,
                                            thing_template_is_floor);
                if (!wid_current_floor) {
                    DIE("not on a floor tile");
                }

                widp wid_next_floor = wid_grid_find_thing_template(
                                            wid_game_map_server_grid_container,
                                            nexthop_x,
                                            nexthop_y,
                                            thing_template_is_floor);
                if (!wid_next_floor) {
                    DIE("no floor tile to hpp to for %s", thing_logname(t));
                }
 
                double fnexthop_x = (double)nexthop_x;
                double fnexthop_y = (double)nexthop_y;

                if (fnexthop_x > t->x) {
                    if (fnexthop_y > t->y) {
                        thing_set_is_dir_tr(t);
                    } else if (fnexthop_y < t->y) {
                        thing_set_is_dir_br(t);
                    } else {
                        thing_set_is_dir_right(t);
                    }
                } else if (fnexthop_x < t->x) {
                    if (fnexthop_y > t->y) {
                        thing_set_is_dir_tl(t);
                    } else if (fnexthop_y < t->y) {
                        thing_set_is_dir_bl(t);
                    } else {
                        thing_set_is_dir_left(t);
                    }
                } else if (fnexthop_y > t->y) {
                    if (fnexthop_x > t->x) {
                        thing_set_is_dir_br(t);
                    } else if (fnexthop_x < t->x) {
                        thing_set_is_dir_bl(t);
                    } else {
                        thing_set_is_dir_down(t);
                    }
                } else if (fnexthop_y < t->y) {
                    if (fnexthop_x > t->x) {
                        thing_set_is_dir_tr(t);
                    } else if (fnexthop_x < t->x) {
                        thing_set_is_dir_br(t);
                    } else {
                        thing_set_is_dir_up(t);
                    }
                } else {
                    DIE("no next hop to go to");
                }

                double x = t->x + ((fnexthop_x - t->x) * 1);
                double y = t->y + ((fnexthop_y - t->y) * 1);
//                double x = t->x + ((fnexthop_x - t->x) * THING_COORD_MOVE);
//                double y = t->y + ((fnexthop_y - t->y) * THING_COORD_MOVE);

                thing_server_move(t,
                        x,
                        y,
                        thing_is_dir_up(t),
                        thing_is_dir_down(t),
                        thing_is_dir_left(t),
                        thing_is_dir_right(t));
            }
        }
    }

    socket_server_tx_map_update(0 /* all clients */, server_active_things);
}

static void thing_tick_client_all (void)
{
    thingp t;
    levelp level = client_level;
    if (!level) {
        return;
    }

    static int32_t pulsate_delta = 2;
    static int32_t pulsate;
    boolean scared;

    pulsate += pulsate_delta;

    if (pulsate > 255) {
        pulsate = 255;
        pulsate_delta = -pulsate_delta;
    }

    if (pulsate < 0) {
        pulsate = 6;
        pulsate_delta = -pulsate_delta;
    }

    /*
     * Allow level timers to fire.
     */
    level_tick(level);

    /*
     * Any timers waiting to fire?
     */
    if (thing_timers) {
        action_timers_tick(thing_timers);
    }

    TREE_WALK(client_active_things, t) {
        thing_templatep thing_template;
        widp w;

        /*
         * Sanity checks.
         */
        verify(t);
        thing_template = thing_get_template(t);

        w = t->wid;
        if (w) {
            verify(w);
        }

        /*
         * Thing has croaked it?
         */
        if (thing_is_dead(t)) {
            if (t->destroy_delay > 0) {
                if (--t->destroy_delay == 0) {
                    if (thing_is_left_as_corpse_on_death(t)) {
                        thing_bury(t);
                    } else {
                        thing_destroy(t, "died");
                    }
                }
            }
            continue;
        }

        /*
         * If on the map.
         */
        if (w) {
            wid_set_blit_outline(w, false);

            /*
             * Animate before death checks so dead things can wriggle.
             *
             * Need to check to avoid for things that are popped off the map, 
             * like stars.
             */
            if (thing_is_animated(t)) {
                thing_animate(t);
            }

            /*
             * Make the monsters flash.
             */
            if (scared && thing_is_scarable(t)) {
                color c;

                c.r = pulsate;
                c.g = pulsate;
                c.b = 255;
                c.a = 255;

                wid_set_blit_outline(w, true);
                wid_set_color(w, WID_COLOR_BLIT_OUTLINE, c);
            }
        }

        if (w) {
            switch (t->dir) {
            case THING_DIR_LEFT:
                if (thing_template_is_effect_rotate_2way(thing_template)) {
                    wid_rotate_immediate(w, 180);
                    wid_flip_vert(w, true);
                }
                break;
            case THING_DIR_RIGHT:
                if (thing_template_is_effect_rotate_2way(thing_template)) {
                    wid_rotate_immediate(w, 0);
                    wid_flip_vert(w, false);
                }
                break;
            case THING_DIR_UP:
                if (thing_template_is_effect_rotate_4way(thing_template)) {
                    wid_rotate_immediate(w, 270);
                }
                break;
            case THING_DIR_DOWN:
                if (thing_template_is_effect_rotate_4way(thing_template)) {
                    wid_rotate_immediate(w, 90);
                }
                break;
            case THING_DIR_TL:
                if (thing_template_is_effect_rotate_2way(thing_template)) {
                    wid_rotate_immediate(w, 180);
                    wid_flip_vert(w, true);
                }
                if (thing_template_is_effect_rotate_4way(thing_template)) {
                    wid_rotate_immediate(w, 270);
                }
                break;
            case THING_DIR_BL:
                if (thing_template_is_effect_rotate_2way(thing_template)) {
                    wid_rotate_immediate(w, 180);
                    wid_flip_vert(w, true);
                }
                if (thing_template_is_effect_rotate_4way(thing_template)) {
                    wid_rotate_immediate(w, 90);
                }
                break;
            case THING_DIR_TR:
                if (thing_template_is_effect_rotate_2way(thing_template)) {
                    wid_rotate_immediate(w, 0);
                    wid_flip_vert(w, false);
                }
                if (thing_template_is_effect_rotate_4way(thing_template)) {
                    wid_rotate_immediate(w, 270);
                }
                break;
            case THING_DIR_BR:
                if (thing_template_is_effect_rotate_2way(thing_template)) {
                    wid_rotate_immediate(w, 0);
                    wid_flip_vert(w, false);
                }
                if (thing_template_is_effect_rotate_4way(thing_template)) {
                    wid_rotate_immediate(w, 90);
                }
                break;
            }
        }
    }
}

void thing_tick_all (void)
{
    thing_tick_server_all();
    thing_tick_client_all();

    if (player) {
        static uint32_t ts;

        if (time_have_x_hundredths_passed_since(THING_PLAYER_POLL_SPEED, ts)) {
            ts = time_get_time_cached();
            wid_game_map_client_player_move();
        }

        wid_game_map_client_scroll_adjust();
    }
}
