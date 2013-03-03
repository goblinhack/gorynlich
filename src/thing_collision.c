/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <SDL.h>

#include "main.h"
#include "thing.h"
#include "thing_template.h"
#include "wid.h"
#include "level.h"
#include "wid_tooltip.h"
#include "wid_game_map.h"
#include "sound.h"
#include "timer.h"

/*
 * Have we hit anything?
 */
void thing_collision (thingp t, int32_t x, int32_t y)
{
    thing_templatep me;
    thing_templatep it;
    levelp level;
    widp wid_next;
    widp wid_me;
    widp wid_it;
    thingp thing_it;

    verify(t);

    me = thing_get_template(t);

    if (!thing_template_is_player(me) &&
        !thing_template_is_plant(me) &&
        !thing_template_is_seedpod(me) &&
        !thing_template_is_car(me) &&
        !thing_template_is_bomb(me) &&
        !thing_template_is_esnail(me)) {
        return;
    }

    level = thing_level(t);
    verify(level);

    /*
     * Popped from the level?
     */
    wid_me = thing_wid(t);
    if (!wid_me) {
        return;
    }

    verify(wid_me);

    /*
     * Look for a floor tile where we can place stuff.
     */
    wid_it = wid_grid_find_first(wid_game_map_grid_container, x, y);
    while (wid_it) {
        verify(wid_it);

        wid_next = wid_grid_find_next(wid_game_map_grid_container,
                                      wid_it, x, y);
        if (wid_me == wid_it) {
            wid_it = wid_next;
            continue;
        }

        if (!wids_overlap(wid_me, wid_it)) {
            wid_it = wid_next;
            continue;
        }

        it = wid_get_thing_template(wid_it);
        if (!it) {
            wid_it = wid_next;
            continue;
        }

        thing_it = wid_get_thing(wid_it);
        if (!thing_it) {
            wid_it = wid_next;
            continue;
        }

        verify(thing_it);

        /*
         * Try to ignore the dead!
         */
        if (thing_is_dead(thing_it)) {
            wid_it = wid_next;
            continue;
        }

        /*
         * Ignore carried things.
         */
        if (thing_item_owner(thing_it)) {
            wid_it = wid_next;
            continue;
        }

        if (thing_template_is_plant(me)) {
            /*
             * Hits explosion.
             */
            if (thing_template_is_explosion(it)) {
                /*
                 * Powerup kills the plant.
                 */
                thing_dead(t, thing_it, "blown up");

                int32_t i;

                /*
                 * Spread seeds to the wind.
                 */
                for (i = 0; i < (rand() % 2) + 1; i++) {
                    action_timer_create(
                            &timers,
                            (action_timer_callback)level_place_plant_pod,
                            level,
                            "place plant",
                            ONESEC * 3, /* duration */
                            ONESEC * 5 /* jitter */);
                }

                return;
            }

            if (thing_template_is_car(it)) {
                thing_dead(t, thing_it, "run over");
                return;
            }
        }

        if (thing_template_is_seedpod(me)) {
            /*
             * Hits explosion.
             */
            if (thing_template_is_explosion(it)) {
                thing_dead(t, thing_it, "blown up");
                return;
            }

            if (thing_template_is_car(it)) {
                thing_dead(t, thing_it, "hit by a car");
                return;
            }
        }

        if (thing_template_is_car(me)) {
            /*
             * Hits explosion.
             */
            if (thing_template_is_explosion(it)) {
                thing_dead(t, thing_it, "blown up");
                return;
            }

            if (thing_template_is_bomb(it)) {
                level_place_explosion(level, thing_grid_x(t), thing_grid_y(t));
                thing_dead(thing_it, 0 /* killer */, "blew up");
                thing_dead(t, thing_it, "blown up");
                return;
            }
        }

        if (thing_template_is_bomb(me)) {
            /*
             * Hits explosion.
             */
            if (thing_template_is_explosion(it)) {
                level_place_explosion(level, thing_grid_x(t), thing_grid_y(t));
                thing_dead(thing_it, 0 /* killer */, "blew up");
                thing_dead(t, thing_it, "blown up");
                return;
            }
        }

        if (thing_template_is_player(me)) {
            /*
             * I am the player. I hit the enemy.
             */
            if (thing_template_is_esnail(it)) {
                /*
                 * But I have a rocket!
                 */
                if (thing_has_powerup_rocket_count(t)) {
                    /*
                     * I steal back letters and nuke the enemy.
                     */
                    thing_item_steal(thing_it, t, thing_template_is_letter);

                    thing_item_steal(thing_it, t, thing_template_is_bomb);
                } else {
                    /*
                     * No rocket. Enemy steals letters.
                     */
                    thing_item_steal(t, thing_it, thing_template_is_letter);

                    thing_item_steal(t, thing_it, thing_template_is_bomb);
                }

                wid_it = wid_next;
                continue;
            }

            /*
             * Player found the exit.
             */
            if (thing_template_is_exit(it)) {
                /*
                 * Is the exit open?
                 */
                if (level_is_exit_open(level)) {
                    thing_reached_exit(t);
                    return;
                } else {
                    /*
                     * Give a hint.
                     */
                    if (!level_get_warned_exit_not_open(level)) {
                        level_set_warned_exit_not_open(level, true);

                        wid_tooltip_large_transient("Collect all letters", 0);
                    }
                }
            }
        }

        if (thing_template_is_esnail(me)) {
            /*
             * Esnail found the exit.
             */
            if (thing_template_is_exit(it)) {
                /*
                 * Is the exit open?
                 */
                if (level_is_exit_open(level)) {
                    thing_reached_exit(t);
                    return;
                }
            }
        }

        if (thing_template_is_player(me) || thing_template_is_esnail(me)) {

            /*
             * Move to a random pipe endpoint.
             */
            if (thing_template_is_pipe(it)) {
                if (level_pipe_find_exit(level,
                                         thing_grid_x(t),
                                         thing_grid_y(t),
                                         &x, &y)) {
                    thing_teleport(t, x, y);
                    return;
                }
            }

            /*
             * Hit a plant.
             */
            if (thing_template_is_plant(it)) {
                if (thing_has_powerup_rocket_count(t)) {
                    /*
                     * Powerup kills the plant.
                     */
                    thing_dead(thing_it, t,
                            "vaporized by %s",
                            t ? thing_logname(t) :
                            thing_template_shortname(me));

                    sound_play_chomp2();

                    int32_t i;

                    for (i = 0; i < (rand() % 2) + 1; i++) {
                        action_timer_create(
                                &timers,
                                (action_timer_callback)level_place_plant_pod,
                                level,
                                "place plant",
                                ONESEC * 3, /* duration */
                                ONESEC * 5 /* jitter */);
                    }
                } else {
                    /*
                     * Plant kills the player or enemy.
                     */
                    thing_dead(t, thing_it,
                            "eaten by %s",
                            thing_it ? thing_logname(thing_it) :
                            thing_template_shortname(it));

                    sound_play_chomp();
                    return;
                }
            }

            /*
             * Hit a seedpod.
             */
            if (thing_template_is_seedpod(it)) {
                if (thing_has_powerup_rocket_count(t)) {
                    /*
                     * Powerup kills the seedpod.
                     */
                    thing_dead(thing_it, t,
                            "vaporized by %s",
                            t ? thing_logname(t) :
                            thing_template_shortname(me));

                    sound_play_chomp2();
                }
            }

            /*
             * Hit by a explosion.
             */
            if (thing_template_is_explosion(it)) {
                thing_dead(t, thing_it, "blown up");
                return;
            }

            /*
             * Hit by a car
             */
            if (thing_template_is_car(it)) {
                thing_dead(t, thing_it, "run over");
                return;
            }

            /*
             * Collect but leave on game.
             */
            if (thing_template_is_letter(it)) {
                sound_play_paper();

                thing_item_collect(t, thing_it);

                wid_it = wid_next;
                continue;
            }

            /*
             * Collect but leave on game.
             */
            if (thing_template_is_bomb(it)) {
                if (!thing_is_open(thing_it)) {
                    /*
                     * Esnail lights bombs.
                     */
                    if (thing_template_is_esnail(me)) {
                        thing_set_is_open(thing_it, true);
                    } else {
                        thing_item_collect(t, thing_it);
                        thing_set_wid(thing_it, 0);

                        sound_play_paper();

                        wid_it = wid_next;
                        continue;
                    }
                }
            }

            /*
             * Collect but remove from game.
             */
            if (thing_template_is_star(it) ||
                thing_template_is_bonus_letter(it)) {

                thing_item_collect(t, thing_it);
                thing_set_wid(thing_it, 0);

                sound_play_powerup();

                wid_it = wid_next;
                continue;
            }

            /*
             * Collect but remove from level.
             */
            if (thing_template_is_spam(it)) {

                thing_item_collect(t, thing_it);
                thing_set_wid(thing_it, 0);

                sound_play_spam();

                wid_it = wid_next;
                continue;
            }

            /*
             * Do not collect. Remove from level.
             */
            if (thing_template_is_food(it)) {

                thing_dead(thing_it, t, "eaten");
                thing_set_wid(thing_it, 0);

                sound_play_chomp2();

                wid_it = wid_next;
                continue;
            }
        }

        wid_it = wid_next;
    }
}
