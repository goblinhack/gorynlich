/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */

#include "main.h"
#include "tree.h"
#include "thing.h"
#include "thing_template.h"
#include "wid_game_map_server.h"

tpp random_wall (void)
{
    int loop = 0;

    for (;;) {

        if (loop++ > 100000) {
            ERR("couldn't find random wall");
            return (0);
        }

        uint16_t id = myrand() % THING_MAX_ID;

        tpp tp = id_to_tp(id);

        if (tp_is_internal(tp)) {
            continue;
        }

        if (tp_is_corridor_wall(tp)) {
            continue;
        }

        if (tp_is_wall(tp)) {
            /*
             * Exclude lit walls as we run out of light sources
             */
            if (tp_is_light_source(tp)) {
                continue;
            }

            return (tp);
        }
    }
}

tpp random_corridor_wall (void)
{
    int loop = 0;

    for (;;) {

        if (loop++ > 100000) {
            ERR("couldn't find random wall");
            return (0);
        }

        uint16_t id = myrand() % THING_MAX_ID;

        tpp tp = id_to_tp(id);

        if (tp_is_internal(tp)) {
            continue;
        }

        if (tp_is_corridor_wall(tp)) {
            return (tp);
        }
    }
}

tpp random_door (void)
{
    int loop = 0;

    for (;;) {

        if (loop++ > 100000) {
            ERR("couldn't find random door");
            return (0);
        }

        uint16_t id = myrand() % THING_MAX_ID;

        tpp tp = id_to_tp(id);

        if (tp_is_internal(tp)) {
            continue;
        }

        if (tp_is_door(tp)) {
            return (tp);
        }
    }
}

tpp random_floor (void)
{
    int loop = 0;

    for (;;) {

        if (loop++ > 100000) {
            ERR("couldn't find random floor");
            return (0);
        }

        uint16_t id = myrand() % THING_MAX_ID;

        tpp tp = id_to_tp(id);

        if (tp_is_internal(tp)) {
            continue;
        }

        if (tp_is_shop_floor(tp)) {
            continue;
        }

        if (tp_is_light_source(tp)) {
            continue;
        }

        if (tp_is_corridor(tp)) {
            continue;
        }

        if (tp_is_floor(tp)) {
            return (tp);
        }
    }
}

tpp random_corridor (void)
{
    int loop = 0;

    for (;;) {

        if (loop++ > 100000) {
            ERR("couldn't find random floor");
            return (0);
        }

        uint16_t id = myrand() % THING_MAX_ID;

        tpp tp = id_to_tp(id);

        if (tp_is_internal(tp)) {
            continue;
        }

        if (tp_is_shop_floor(tp)) {
            continue;
        }

        if (tp_is_light_source(tp)) {
            continue;
        }

        if (tp_is_corridor(tp)) {
            return (tp);
        }
    }
}

tpp random_player (void)
{
    int loop = 0;

    for (;;) {

        if (loop++ > 100000) {
            ERR("couldn't find random player");
            return (0);
        }

        uint16_t id = myrand() % THING_MAX_ID;

        tpp tp = id_to_tp(id);

        if (tp_is_internal(tp)) {
            continue;
        }

        if (tp_is_player(tp)) {
            return (tp);
        }
    }
}

tpp random_exit (void)
{
    int loop = 0;

    for (;;) {

        if (loop++ > 10000000) {
            ERR("couldn't find random exit");
            return (0);
        }

        uint16_t id = myrand() % THING_MAX_ID;

        tpp tp = id_to_tp(id);

        if (tp_is_internal(tp)) {
            continue;
        }

        if (tp_is_exit(tp)) {
            return (tp);
        }
    }
}

tpp random_food (void)
{
    int loop = 0;

    for (;;) {

        if (loop++ > 100000) {
            ERR("couldn't find random food");
            return (0);
        }

        uint16_t id = myrand() % THING_MAX_ID;

        tpp tp = id_to_tp(id);

        if (tp_is_internal(tp)) {
            continue;
        }

        if (tp_is_food(tp)) {
            int r =  myrand() % 10000;
            if (r < tp_get_d10000_chance_of_appearing(tp)) {
                return (tp);
            }
        }
    }
}

tpp random_treasure (int shop_floor)
{
    int loop = 0;

    for (;;) {

        if (loop++ > 100000) {
            ERR("couldn't find random treasure");
            return (0);
        }

        uint16_t id = myrand() % THING_MAX_ID;

        tpp tp = id_to_tp(id);

        if (tp_is_internal(tp)) {
            continue;
        }

        /*
         * Valid for shops?
         */
        if (shop_floor && !tp_get_cost(tp)) {
            continue;
        }

        if (tp_is_treasure(tp)) {
            int r =  myrand() % 10000;
            if (r < tp_get_d10000_chance_of_appearing(tp)) {
                return (tp);
            }
        }
    }
}

tpp random_weapon (int shop_floor)
{
    int loop = 0;

    for (;;) {

        if (loop++ > 100000) {
            ERR("couldn't find random weapon");
            return (0);
        }

        uint16_t id = myrand() % THING_MAX_ID;

        tpp tp = id_to_tp(id);

        if (tp_is_internal(tp)) {
            continue;
        }

        /*
         * Valid for shops?
         */
        if (shop_floor && !tp_get_cost(tp)) {
            continue;
        }

        if (tp_is_weapon(tp)) {
            int r =  myrand() % 10000;
            if (r < tp_get_d10000_chance_of_appearing(tp)) {
                return (tp);
            }
        }
    }
}

tpp random_potion (int shop_floor)
{
    int loop = 0;

    for (;;) {

        if (loop++ > 100000) {
            ERR("couldn't find random potion");
            return (0);
        }

        uint16_t id = myrand() % THING_MAX_ID;

        tpp tp = id_to_tp(id);

        if (tp_is_internal(tp)) {
            continue;
        }

        /*
         * Valid for shops?
         */
        if (shop_floor && !tp_get_cost(tp)) {
            continue;
        }

        if (tp_is_potion(tp)) {
            int r =  myrand() % 10000;
            if (r < tp_get_d10000_chance_of_appearing(tp)) {
                return (tp);
            }
        }
    }
}

tpp random_rock (void)
{
    int loop = 0;

    for (;;) {

        if (loop++ > 100000) {
            ERR("couldn't find random rock");
            return (0);
        }

        uint16_t id = myrand() % THING_MAX_ID;

        tpp tp = id_to_tp(id);

        if (tp_is_internal(tp)) {
            continue;
        }

        if (tp_is_rock(tp)) {
            int r =  myrand() % 10000;
            if (r < tp_get_d10000_chance_of_appearing(tp)) {
                return (tp);
            }
        }
    }
}

tpp random_lava (void)
{
    int loop = 0;

    for (;;) {

        if (loop++ > 100000) {
            ERR("couldn't find random lava");
            return (0);
        }

        uint16_t id = myrand() % THING_MAX_ID;

        tpp tp = id_to_tp(id);

        if (tp_is_internal(tp)) {
            continue;
        }

        if (tp_is_lava(tp)) {
            return (tp);
        }
    }
}

tpp random_monst (int depth)
{
    int loop = 0;

    for (;;) {

        if (loop++ > 100000) {
            ERR("couldn't find random monst");
            return (0);
        }

        uint16_t id = myrand() % THING_MAX_ID;

        tpp tp = id_to_tp(id);

        if (!tp_is_monst(tp)) {
            continue;
        }

        if (tp_is_trap(tp)) {
            continue;
        }

        /*
         * Unique? like death?
         */
        if (!tp_get_d10000_chance_of_appearing(tp)) {
            continue;
        }

        if (depth < tp_get_min_appear_depth(tp)) {
            continue;
        }

        if (depth > tp_get_max_appear_depth(tp)) {
            continue;
        }

        int r = myrand() % 10000;
        if (r < (tp_get_d10000_chance_of_appearing(tp) + depth)) {
            return (tp);
        }
    }
}

tpp random_trap (int depth)
{
    int loop = 0;

    for (;;) {

        if (loop++ > 100000) {
            ERR("couldn't find random trap");
            return (0);
        }

        uint16_t id = myrand() % THING_MAX_ID;

        tpp tp = id_to_tp(id);

        if (!tp_is_trap(tp)) {
            continue;
        }

        int r = myrand() % 10000;
        if (r < (tp_get_d10000_chance_of_appearing(tp) + depth)) {
            return (tp);
        }
    }
}

tpp random_mob (int depth)
{
    int loop = 0;

    for (;;) {

        if (loop++ > 100000) {
            ERR("couldn't find random monst");
            return (0);
        }

        uint16_t id = myrand() % THING_MAX_ID;

        tpp tp = id_to_tp(id);

        if (tp_is_internal(tp)) {
            continue;
        }

        if (!tp_is_generator(tp)) {
            continue;
        }

        int r =  myrand() % 10000;
        if (r < (tp_get_d10000_chance_of_appearing(tp) + depth)) {
            return (tp);
        }
    }
}
