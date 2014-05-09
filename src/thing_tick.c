/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "tree.h"
#include "thing.h"
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
#include "math.h"

static void thing_tick_server_all (void)
{
    levelp level = server_level;
    if (!level) {
        return;
    }

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
            /*
             * Wait until the last server update has been sent before burying 
             * the thing.
             */
            if (!t->updated) {
                thing_destroy(t, "died");
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

        thing_handle_collisions(wid_game_map_server_grid_container, t);

        /*
         * Died in a collision ? 8( Handle it next time around.
         */
        if (thing_is_dead(t)) {
            continue;
        }

        /*
         * Don't do stuff too often.
         */
        if (!time_have_x_tenths_passed_since(DELAY_TENTHS_THING_AI,
                                             t->timestamp_ai)) {
            continue;
        }

        /*
         * If a projectile, move it by the delta
         */
        if ((t->dx != 0) || (t->dy != 0)) {
            double fnexthop_x = t->x + t->dx;
            double fnexthop_y = t->y + t->dy;

            thing_server_move(t,
                    fnexthop_x,
                    fnexthop_y,
                    fnexthop_y < t->y,
                    fnexthop_y > t->y,
                    fnexthop_x < t->x,
                    fnexthop_x > t->x,
                    false);
        }

        /*
         * If stopped moving, look for a next hop.
         */
        w = t->wid;

        if (thing_is_monst(t) && speed && w && !wid_is_moving(w)) {
            int32_t nexthop_x = -1;
            int32_t nexthop_y = -1;

            /*
             * Need to look for a nexthop? Or keep walking on?
             */
            boolean have_nexthop = 
                            thing_find_nexthop(t, &nexthop_x, &nexthop_y);
            if (have_nexthop) {
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
                    LOG("no floor tile to hop to %d %d for %s", 
                        nexthop_x, nexthop_y, thing_logname(t));
                    continue;
                }
 
                double fnexthop_x = (double)nexthop_x;
                double fnexthop_y = (double)nexthop_y;

                if (!thing_server_move(t,
                                       fnexthop_x,
                                       fnexthop_y,
                                       fnexthop_y < t->y,
                                       fnexthop_y > t->y,
                                       fnexthop_x < t->x,
                                       fnexthop_x > t->x,
                                       false)) {
                    /*
                     * Hit an obstacle? Try a different map for the next time
                     * we find a next hop.
                     */
                    if (t->walls == &server_level->monst_map_consider_doors) {
                        t->walls = &server_level->monst_map_ignore_doors;
                    } else {
                        t->walls = &server_level->monst_map_consider_doors;
                    }
                }

                t->timestamp_ai = time_get_time_cached();
            }
        }
    }
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
            thing_destroy(t, "died");
            continue;
        }

        /*
         * If on the map.
         */
        if (w) {
            /*
             * Animate before death checks so dead things can wriggle.
             *
             * Need to check to avoid for things that are popped off the map, 
             * like stars.
             */
            if (thing_is_animated(t)) {
                thing_animate(t);
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
    if (server_level) {
        static uint32_t ts;

        if (time_have_x_tenths_passed_since(DELAY_TENTHS_THING_AI, ts)) {
            ts = time_get_time_cached();

            thing_generate_dmaps();
        }
    }

    thing_tick_server_all();

    if (server_level) {
        static uint32_t ts;

        if (time_have_x_tenths_passed_since(DELAY_TENTHS_TX_MAP_UPDATE, ts)) {
            ts = time_get_time_cached();

            socket_server_tx_map_update(0 /* all clients */, 
                                        server_active_things);
        }
    }

    if (player) {
        static uint32_t ts;

        if (time_have_x_thousandths_passed_since(DELAY_THOUSANDTHS_PLAYER_POLL, 
                                                 ts)) {
            ts = time_get_time_cached();

            wid_game_map_client_player_move();

            wid_game_map_client_scroll_adjust();
        }
    }

    thing_tick_client_all();
}
