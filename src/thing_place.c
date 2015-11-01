/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */

#include "main.h"
#include "thing.h"
#include "wid_game_map_server.h"

static widp thing_place_ (thingp t, 
                          tpp tp, 
                          itemp item_in, 
                          const int under, 
                          const int behind)
{
    double dx = 0;
    double dy = 0;
    item_t item = {0};

    if (item_in) {
        memcpy(&item, item_in, sizeof(item));
        item.quantity = tp_get_quantity(tp);
    } else {
        item.id = tp_to_id(tp);
        item.quality = THING_ITEM_QUALITY_MAX;
        item.quantity = tp_get_quantity(tp);
        item.cursed = tp_is_cursed(tp);
    }

    if (t && under) {
        double x;
        double y;

        thing_real_to_fmap(t, &x, &y);

        /*
         * Try to place in front of the player.
         */
        widp grid = wid_game_map_server_grid_container;
        if (!grid) {
            ERR("cannot place thing, no grid map");
        }

        if (!thing_hit_any_obstacle(grid, t, x, y)) {
            widp w = wid_game_map_server_replace_tile(grid, x, y,
                                                    0, /* thing */
                                                    tp,
                                                    0 /* tpp_data */,
                                                    &item,
                                                    0 /* stats */);
            return (w);
        }
    }

    thing_dir(t, &dx, &dy);

    /*
     * Sanity check we got one dir.
     */
    if ((dx == 0.0) && (dy == 0.0)) {
        dx = 1.0;
        dy = 1.0;
    }

    if (behind) {
        dx *= -1.0;
        dy *= -1.0;
    }

    dx /= 2.0;
    dy /= 2.0;

    double x;
    double y;
    thing_real_to_fmap(t, &x, &y);
    x += dx;
    y += dy;

    /*
     * Try to place in front of the player.
     */
    widp grid = wid_game_map_server_grid_container;
    if (!grid) {
        ERR("cannot place thing, no grid map");
    }

    if (!thing_hit_any_obstacle(grid, t, x, y)) {
        widp w = wid_game_map_server_replace_tile(grid, x, y,
                                                  0, /* thing */
                                                  tp,
                                                  0 /* tpp_data */,
                                                  &item,
                                                  0 /* stats */);
        return (w);
    }

    dx *= 2.0;
    dy *= 2.0;

    thing_real_to_fmap(t, &x, &y);

    x += dx;
    y += dy;

    /*
     * Try to place in front of the player.
     */
    if (!thing_hit_any_obstacle(grid, t, x, y)) {
        widp w = wid_game_map_server_replace_tile(grid, x, y,
                                                  0, /* thing */
                                                  tp,
                                                  0 /* tpp_data */,
                                                  &item,
                                                  0 /* stats */);
        return (w);
    }

    /*
     * Just place anywhere free.
     */
    for (dx = -1.0; dx <= 1.0; dx += 1.0) {
        for (dy = -1.0; dy <= 1.0; dy += 1.0) {
            if ((dx == 0.0) && (dy == 0.0)) {
                continue;
            }

            double x;
            double y;
            thing_real_to_fmap(t, &x, &y);
            x += dx;
            y += dy;

            if (x < 0) {
                continue;
            }
            if (y < 0) {
                continue;
            }

            if (x >= MAP_WIDTH) {
                continue;
            }
            if (y >= MAP_HEIGHT) {
                continue;
            }

            if (map_find_wall_at(server_level, x, y, 0) ||
                map_find_door_at(server_level, x, y, 0) ||
                map_find_crystal_at(server_level, x, y, 0) ||
                map_find_rock_at(server_level, x, y, 0)) {
                continue;
            }

            if (!thing_hit_any_obstacle(grid, t, x, y)) {
                widp w = wid_game_map_server_replace_tile(grid, x, y, 
                                                          0, /* thing */
                                                          tp,
                                                          0 /* tpp_data */,
                                                          &item,
                                                          0 /* stats */);
                return (w);
            }
        }
    }

    /*
     * Ok place on top of something else other than a wall
     */
    for (dx = -1.0; dx <= 1.0; dx += 1.0) {
        for (dy = -1.0; dy <= 1.0; dy += 1.0) {
            if ((dx == 0.0) && (dy == 0.0)) {
                continue;
            }

            double x;
            double y;
            thing_real_to_fmap(t, &x, &y);
            x += dx;
            y += dy;

            if (map_find_wall_at(server_level, x, y, 0) ||
                map_find_door_at(server_level, x, y, 0) ||
                map_find_crystal_at(server_level, x, y, 0) ||
                map_find_rock_at(server_level, x, y, 0)) {
                continue;
            }

            widp w = wid_game_map_server_replace_tile(grid, x, y, 
                                                      0, /* thing */
                                                      tp,
                                                      0 /* tpp_data */,
                                                      &item,
                                                      0 /* stats */);
            return (w);
        }
    }

    thing_real_to_fmap(t, &x, &y);

    /*
     * Last resort, just place on the player.
     */
    widp w = wid_game_map_server_replace_tile(grid, x, y, 
                                              0, /* thing */
                                              tp,
                                              0 /* tpp_data */,
                                              &item,
                                              0 /* stats */);
    return (w);
}

widp thing_place (thingp t, tpp tp, itemp item)
{
    return (thing_place_(t, tp, item, false /* under */, false /* behind */));
}

widp thing_place_behind (thingp t, tpp tp, itemp item)
{
    return (thing_place_(t, tp, item, false /* under */, true /* behind */));
}

widp thing_place_behind_or_under (thingp t, tpp tp, itemp item)
{
    return (thing_place_(t, tp, item, true /* under */, true /* behind */));
}

