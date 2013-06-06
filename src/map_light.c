/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <SDL.h>

#include "main.h"
#include "map.h"
#include "geo.h"

typedef struct {
    float dist;
    int8_t x, y, z;
} map_light_cell;

static map_light_cell 
    map_light_cells[(MAX_LIGHT_SIZE*2) * (MAX_LIGHT_SIZE*2) * (MAP_DEPTH*2)];
static map_light_cell *map_light_cells_max =
    map_light_cells + ARRAY_SIZE(map_light_cells);

typedef struct {
    float dist;
    float shadow;
    int8_t x, y, z;
    uint8_t is_a_cell;
} map_light_shadow;

static map_light_shadow 
    map_light_shadows[(MAX_LIGHT_SIZE*2) * (MAX_LIGHT_SIZE*2) * (MAP_DEPTH*2) * 100];
static map_light_shadow *map_light_shadows_end;
static map_light_shadow *map_light_shadows_max =
    map_light_shadows + ARRAY_SIZE(map_light_shadows);

/*
 * Calculate shadow volumes for tiles at ever increasing distances from the
 * light source.
 */
void 
map_lightgen (map_frame_ctx_t *map, int32_t strength)
{
    map_light_cell *c;
    map_light_cell *d;
    map_light_cell *e;
    boolean unsorted;
    int32_t dx;
    int32_t dy;
    int32_t dz;

    LOG("%lu",sizeof(map_light_shadows));
    if (strength >= MAX_LIGHT_SIZE) {
        DIE("light is too strong!");
    }

    /*
     * Generate all possible cells that are in the light sphere.
     */
    c = map_light_cells;
    for (dz = -strength; dz <= strength; dz++) {
        if (dz < -(MAP_DEPTH - 1)) {
            continue;
        }

        if (dz > MAP_DEPTH - 1) {
            continue;
        }

        for (dy = -strength; dy <= strength; dy++) {
            for (dx = -strength; dx <= strength; dx++) {

                c->dist = DISTANCE3f(0.0,0.0,0.0,dx,dy,dz);
                if (c->dist > strength + 1) {
                    continue;
                }

                c->x = dx;
                c->y = dy;
                c->z = dz;

                c++;

                if (c >= map_light_cells_max) {
                    DIE("ran out of cell space");
                }
            }
        }
    }

    /*
     * Now sort them by distance.
     */
    e = c;
    unsorted = true;

    while (unsorted) {
        unsorted = false;

        c = map_light_cells;
        for (c = map_light_cells; c < e - 1; c++) {
            d = c + 1;

            if (c->dist > d->dist) {

                map_light_cell e = *d;
                *d = *c;
                *c = e; 

                unsorted = true;
            }
        }
    }

    /*
     * Now for each cell find out which cells shadow it.
     */
    map_light_shadow *o;
    map_light_cell *i;

    o = map_light_shadows;
    c = map_light_cells;

    int xxx;
    while (c < e) {
//printf("\n%ld ",e -c);
//fflush(stdout);
        o->shadow = 0.0;
        o->is_a_cell = true;
        o->dist = c->dist;
        o->x = c->x;
        o->y = c->y;
        o->z = c->z;

xxx = 0;
if ((o->x == 3) && (o->y == 3) && (o->z == -2)) {
    xxx = 1;
printf("%d,%d,%d  ", o->x,o->y,o->z);
}
        o++;

        if (o >= map_light_shadows_max) {
            DIE("ran out of shadow space");
        }

        line ray;

        fpoint3d light_source = {0.5, 0.5, 0.5};
        fpoint3d ray_end = {c->x+0.5, c->y+0.5, c->z+0.5};

        ray.P0 = light_source;
        ray.P1 = ray_end;

        /*
         * For all cells that are closer to the light source.
         */
        i = map_light_cells;
        while (i < c) {

            fpoint3d p0 = { i->x  , i->y  , i->z   };
            fpoint3d p1 = { i->x+1, i->y+1, i->z   };
            fpoint3d p2 = { i->x+1, i->y  , i->z   };
            fpoint3d p3 = { i->x  , i->y+1, i->z   };

            fpoint3d p4 = { i->x  , i->y  , i->z-1 };
            fpoint3d p5 = { i->x+1, i->y+1, i->z-1 };
            fpoint3d p6 = { i->x+1, i->y  , i->z-1 };
            fpoint3d p7 = { i->x  , i->y+1, i->z-1 };

            fpoint3d intersection;
            float distance;

            boolean intersects = cube_line_intersect(ray, 
                                               p0,
                                               p1,
                                               p2,
                                               p3,
                                               p4,
                                               p5,
                                               p6,
                                               p7,
                                               &intersection,
                                               &distance);

            if (!intersects) {
                i++;
                continue;
            }

//            if (distance > strength + 2) {
        if (xxx) {
                printf("ray %f,%f,%f to %f,%f,%f\n",
                       ray.P0.x,
                       ray.P0.y,
                       ray.P0.z,
                       ray.P1.x,
                       ray.P1.y,
                       ray.P1.z);
                printf("intersects at %f,%f,%f with cube at %d,%d,%d\n",
                       intersection.x, intersection.y, intersection.z,
                       i->x,i->y,i->z);

                LOG("light map broken, distance %f strength %d",
                    distance, strength);
            }

            /*
             * Shadow fades with distance from the obstacle.
             */
            float shadow;
            
            float s = ((float)strength * 2) - distance;
            if (distance <= 1) {
                shadow = 1.0;
            } else {
                shadow = 1.0 - (1.0 / (s));
                shadow = 1;
            }
//printf("%f(%f) ",shadow,s);
//fflush(stdout);

            o->shadow = shadow;
            o->is_a_cell = false;
            o->dist = distance;
            o->x = i->x;
            o->y = i->y;
            o->z = i->z;

//printf("shadow %f",shadow);
            o++;
            i++;

            if (o >= map_light_shadows_max) {
                DIE("ran out of shadow space");
            }
        }

        c++;
//printf("\n");
    }

    map_light_shadows_end = o;
}

/*
 * Calculate shadow volumes for tiles at ever increasing distances from the
 * light source.
 */
void 
map_lightmap (map_frame_ctx_t *map,
              int32_t lx,
              int32_t ly,
              int32_t lz,
              int32_t strength,
              boolean first_light)
{
    int32_t x;
    int32_t y;
    int32_t z;
    static boolean done;

    if (!done) {
        done = true;
        map_lightgen(map_ctx, MAX_LIGHT_SIZE - 1);
    }

    /*
     * Clear out the light map. Assume all cells are in darkness. Only do this 
     * if this is the first light source in the render scene.
     */
    if (first_light) {
        for (z = 0; z < MAP_DEPTH; z++) {
            for (x = 0; x < MAP_WIDTH; x++) {
                for (y = 0; y < MAP_HEIGHT; y++) {
                    map->tiles[x][y][z].lit = 0.0;
                }
            }
        }
    }

    /*
     * The light cell is always lit.
     */
    map->tiles[lx][ly][lz].lit = 1;

    map_light_shadow *s = map_light_shadows;

    while (s < map_light_shadows_end) {
        x = lx + s->x;
        y = ly + s->y;
        z = lz + s->z;

#if 0
        if (!map->tiles[x][y][z].tile) {
            s++;

            while (s < map_light_shadows_end) {
                if (s->is_a_cell) {
                    break;
                }

                s++;
            }

            continue;
        }
#endif

        s++;

        /*
         * If this cell is oob then jump to the next one.
         */
        if (map_out_of_bounds(x, y, z)) {
            while (s < map_light_shadows_end) {
                if (s->is_a_cell) {
                    break;
                }

                s++;
            }

            continue;
        }

        /*
         * Never go beyond the edge of the light.
         */
        boolean skip = false;
        if (s->dist > strength + 1) {
            skip = true;
            continue;
        }

        /*
         * Add up the shadows from all obstacles.
         */
        float total_shadow = 0.0;

        while (s < map_light_shadows_end) {
            if (s->is_a_cell) {
                break;
            }

            if (skip) {
                s++;
                continue;
            }

            int32_t cx = lx + s->x;
            int32_t cy = ly + s->y;
            int32_t cz = lz + s->z;

            if (!map->tiles[cx][cy][cz].tile) {
                s++;
                continue;
            }

            total_shadow += s->shadow;
            s++;
        }

        float max_shadow = 3.0;

        if (total_shadow > max_shadow) {
            map->tiles[x][y][z].lit = 0.0;
        } else {
            map->tiles[x][y][z].lit = (max_shadow - total_shadow) / max_shadow;
        }
    }
}
