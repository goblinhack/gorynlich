/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
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
#include "wid_game_map.h"
#include "level.h"
#include "wid_console.h"
#include "time.h"
#include "color.h"

/*
 * If the map is swiping, we cannot move things. Or if the main menu is up, 
 * ignore events.
 */
static boolean level_ignore_events (void)
{
    if (level_game) {
        if (level_is_frozen(level_game)) {
            return (true);
        }
    }

    if (wid_game_map_window) {
        if (wid_ignore_for_events(wid_game_map_window)) {
            return (true);
        }
    }

    return (false);
}

void thing_tick_all (void)
{
    static uint32_t gridpixwidth;
    static uint32_t gridpixheight;
    static int32_t pulsate_delta = 2;
    static int32_t pulsate;
    boolean scared;
    levelp level;
    thingp t;

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
     * Console open? Allow debugging.
     */
    if (wid_console_input_line &&
        !wid_is_hidden(wid_console_input_line)) {
        return;
    }

    level = level_game;
    if (!level) {
        return;
    }

    /*
     * If the map is swiping, we cannot move things. Or if the main menu is 
     * up, ignore events.
     */
    if (level_ignore_events()) {
        return;
    }

    /*
     * If the window was moving we could not attach the widgets yet.
     * Now it has stopped moving, we can.
     */
    if (!wid_has_grid(wid_game_map_grid_container)) {
        wid_new_grid(wid_game_map_grid_container,
                     TILES_MAP_WIDTH,
                     TILES_MAP_HEIGHT, tile_width, tile_height);

        wid_attach_to_grid(wid_game_map_grid_container);
        if (!wid_has_grid(wid_game_map_grid_container)) {
            DIE("no grid");
        }
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

    TREE_WALK(things, t) {
        thing_templatep thing_template;
        boolean aligned_x;
        boolean aligned_y;
        thingp owner;
        widp w;

        /*
         * Sanity checks.
         */
        verify(t);
        thing_template = thing_get_template(t);

        owner = t->item_owner;
        if (owner) {
            verify(owner);
        }

        w = t->wid;
        if (w) {
            verify(w);

            wid_get_grid_coord(w, &t->grid_x, &t->grid_y,
                               &aligned_x, &aligned_y);
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

        /*
         * Is the level paused ?
         *
         * Keep this after animate, so dead things wiggle.
         */
        if (level_is_paused(t->level)) {
            if (w) {
                wid_move_stop(w);
            }

            continue;
        }

        /*
         * Thing has croaked it?
         */
        if (thing_is_dead(t)) {
            if (thing_is_left_as_corpse_on_death(t)) {
                thing_bury(t);
            } else {
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

            color c;

            c = RED;
            c.g = pulsate;
            c.b = pulsate;
            c.a = 255;

            wid_set_blit_outline(w, true);
            wid_set_color(w, WID_COLOR_BLIT_OUTLINE, c);
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
         * Pump the score in little increments as it looks cool.
         */
        uint32_t score_pump = thing_score_pump(t);

        if (score_pump) {
            uint32_t score_pump_big_step = 100;
            uint32_t score_pump_small_step = 15;

            if (score_pump > score_pump_big_step) {
                thing_set_score(t,
                    thing_score(t) + score_pump_big_step);

                thing_set_score_pump(t,
                    thing_score_pump(t) - score_pump_big_step);
            } else if (score_pump > score_pump_small_step) {
                thing_set_score(t,
                    thing_score(t) + score_pump_small_step);

                thing_set_score_pump(t,
                    thing_score_pump(t) - score_pump_small_step);
            } else {
                thing_set_score(t,
                    thing_score(t) + score_pump);

                thing_set_score_pump(t,
                    thing_score_pump(t) - score_pump);
            }

            if (player == t) {
                wid_game_map_score_update(level);
            }
        }

        /*
         * Look for collisions.
         */
        thing_collision(t, t->grid_x, t->grid_y);
        thing_collision(t, t->grid_x + 1, t->grid_y);
        thing_collision(t, t->grid_x, t->grid_y - 1);
        thing_collision(t, t->grid_x - 1, t->grid_y);
        thing_collision(t, t->grid_x, t->grid_y + 1);
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
        if (speed && w && !wid_is_moving(w)) {
            t->tl_x = wid_get_tl_x(w);
            t->br_x = wid_get_br_x(w);
            t->tl_y = wid_get_tl_y(w);
            t->br_y = wid_get_br_y(w);
            t->x = (t->tl_x + t->br_x) / 2;
            t->y = (t->tl_y + t->br_y) / 2;

            if (!gridpixwidth) {
                wid_get_grid_dim(w, &gridpixwidth, &gridpixheight);
            }

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
                t->is_dir_up = false;
                t->is_dir_down = false;
                t->is_dir_left = false;
                t->is_dir_right = false;

                widp wid_curr_floor = wid_grid_find_thing_template(
                                                wid_game_map_grid_container,
                                                t->grid_x,
                                                t->grid_y,
                                                thing_template_is_floor);
                if (!wid_curr_floor) {
                    DIE("not on a floor tile");
                }

                widp wid_next_floor = wid_grid_find_thing_template(
                                                wid_game_map_grid_container,
                                                nexthop_x,
                                                nexthop_y,
                                                thing_template_is_floor);
                if (!wid_next_floor) {
                    DIE("no floor tile to hpp to");
                }
 
                float this_floor_x, this_floor_y;
                wid_get_mxy(wid_curr_floor, &this_floor_x, &this_floor_y);

                float next_floor_x, next_floor_y;
                wid_get_mxy(wid_next_floor, &next_floor_x, &next_floor_y);

                if (!aligned_x) {
                    next_floor_y = this_floor_y;
                }

                if (!aligned_y) {
                    next_floor_x = this_floor_x;
                }

                if (next_floor_x > t->x) {
                    t->is_dir_right = true;
                } else if (next_floor_x < t->x) {
                    t->is_dir_left = true;
                } else if (next_floor_y > t->y) {
                    t->is_dir_down = true;
                } else if (next_floor_y < t->y) {
                    t->is_dir_up = true;
                } else {
                    THING_LOG(t,
                              "this %f %f next %f %f at %d %d align %d %d w %p %s",
                              this_floor_x,this_floor_y,
                              next_floor_x,next_floor_y,
                              t->x, t->y,
                              aligned_x, aligned_y,
                              w, wid_logname(w));

                    DIE("no next hop to go to");
                }

                wid_move_to_abs_centered_in(w,
                                            next_floor_x, next_floor_y, speed);
            }
        }

        /*
         * Does ths item follows the owner?
         */
        if (w && owner && owner->wid &&
            thing_template_is_follows_owner(thing_template)) {

            fpoint tl;
            fpoint br;

            wid_get_tl_br(owner->wid, &tl, &br); 

            float mx = (tl.x + br.x) / 2.0;
            float my = (tl.y + br.y) / 2.0;

            wid_rotate_immediate(w, wid_get_rotate(owner->wid));

            /*
             * Carried smaller and to the front of the carrier.
             */
            if (thing_template_is_shrunk_when_carried(thing_template)) {
                wid_scale_immediate(w, 0.5);
                my = ((tl.y + br.y) / 2.0) - ((br.y - tl.y) / 4.0);
            }

            wid_move_to_abs_centered(w, mx, my);
            wid_set_z_depth(w, 20);

            wid_flip_vert(w, wid_get_flip_vert(owner->wid));
            wid_flip_horiz(w, wid_get_flip_horiz(owner->wid));
        } else if (w) {
            if (t->is_dir_right) {
                if (thing_template_is_effect_rotate_2way(thing_template)) {
                    wid_rotate_immediate(w, 0);
                    wid_flip_vert(w, false);
                }
            } else if (t->is_dir_left) {
                if (thing_template_is_effect_rotate_2way(thing_template)) {
                    wid_rotate_immediate(w, 180);
                    wid_flip_vert(w, true);
                }
            } else if (t->is_dir_down) {
                if (thing_template_is_effect_rotate_4way(thing_template)) {
                    wid_rotate_immediate(w, 90);
                }
            } else if (t->is_dir_up) {
                if (thing_template_is_effect_rotate_4way(thing_template)) {
                    wid_rotate_immediate(w, 270);
                }
            }
        }
    }
}
