/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 *
 * This file precalculates thousands of light rays from a source and works out
 * the intersection points with map cubes. This gives us a pre-calculated 
 * light map for any position. This takes time, so we save the light map data
 * to a file and read it in on boot.
 */

#include <SDL.h>

#include "main.h"
#include "map.h"
#include "geo.h"
#include "file.h"
#include "mzip_file.h"

typedef struct {
    int8_t x, y, z;
    uint8_t dist:6;
    uint8_t shadow:1;
    uint8_t is_a_cell:1;
} map_light_shadow;

/*
 * This is a contiguous array of light cells. You have first a light cell
 * that is an actual cube we are wanting to work out all the shadows for.
 * Then all following cubes with is_a_cell set to false potentially light
 * blocking cells.
 *
 * The cells are sorted so that those closes to the light source are first.
 * Hence if you want a light source of x, you just keep walking and processing 
 * until you hit a is_a_cell of true. All cells beyond this would be outside
 * of your light source.
 */
static map_light_shadow *map_light_shadows_start;
static map_light_shadow *map_light_shadows_end;

/*
 * Generate a pre-calculated light map for the light source. We expect to
 * do this only once for a long light ray. Smaller light rays are all
 * contained within the larger light map file we generate. So generating
 * for length x also generates for rays length 1 .. x
 */
static inline void 
map_light_generate_raytrace_map (map_frame_ctx_t *map, int32_t strength)
{
    /*
     * Generate a sphere of cubes around the light source and work out
     * all the cubes that could potentially block light from the source.
     */
    typedef struct {
        int8_t x, y, z;
        float dist;
    } map_light_cell;

    /*
     * These cells are our source of sorted-by-distance cubes that we are
     * going to work out shadows for.
     */
    static map_light_cell 
        map_light_cells[(DEFAULT_LIGHT_RAY_LENGTH*2) *
        (DEFAULT_LIGHT_RAY_LENGTH*2) * (MAP_DEPTH*2)];

    static map_light_cell *map_light_cells_max =
        map_light_cells + ARRAY_SIZE(map_light_cells);

    /*
     * Make a large guesstimate about the number of cells that we may end
     * up generating.
     */
    static map_light_shadow 
        map_light_shadows[(DEFAULT_LIGHT_RAY_LENGTH*2) *
        (DEFAULT_LIGHT_RAY_LENGTH*2) * (MAP_DEPTH*2) * 100];

    static map_light_shadow *map_light_shadows_max =
        map_light_shadows + ARRAY_SIZE(map_light_shadows);

    /*
     * To cut down the number of cells we consider as shadows, we do a quick
     * calc based on the distance of a point from the light ray line segment.
     * This is quicker than calculating all the cube intersections. If it's
     * too far plus a fudge factor, ignore it.
     */
    float cutoff = sqrt(2.0)/2.0 + 0.5;

    /*
     * This is where the light starts. We don't use 0 as we want it to be
     * symmetrical and hence half way into cybe. For z is has to be > 0 as
     * we count z from 0 (ground) to ceiling and if z was negative then
     * tiles at the same height would shadow each other.
     */
    fpoint3d light = {-0.5, -0.5, 0.5};

    map_light_cell *c;
    map_light_cell *d;
    map_light_cell *e;
    int32_t dx;
    int32_t dy;
    int32_t dz;

    if (strength > DEFAULT_LIGHT_RAY_LENGTH) {
        DIE("light is too strong!");
    }

    /*
     * Generate all possible cells that are in the light sphere.
     */
    c = map_light_cells;
    for (dz = -strength; dz < strength; dz++) {
        /*
         * Trim the sphere vertically. We do not need to look to high as
         * we only render so many cubes.
         */
        if (dz <= -(MAP_DEPTH - 1)) {
            continue;
        }

        if (dz > MAP_DEPTH - 1) {
            continue;
        }

        for (dy = -strength; dy < strength; dy++) {
            for (dx = -strength; dx < strength; dx++) {

                /*
                 * Ignore cells outside of the light sphere.
                 */
                c->dist = DISTANCE3f(light.x, light.y, light.z, dx, dy, dz);
                if (c->dist > strength) {
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
     * Now sort them by distance from th light source.
     */
    e = c;
    boolean unsorted = true;

    /*
     * Keep looping until sorted. Dumb, slow, but works.
     */
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
     * Now for each cell find out which cells shadow it. This is the slow bit.
     */
    map_light_shadow *o;
    map_light_cell *i;

    o = map_light_shadows;
    c = map_light_cells;

    while (c < e) {
        o->shadow = 0;
        o->is_a_cell = true;
        o->dist = c->dist;
        o->x = c->x;
        o->y = c->y;
        o->z = c->z;

        o++;

        /*
         * Sanity.
         */
        if (o >= map_light_shadows_max) {
            DIE("ran out of shadow space");
        }

        fpoint3d light_source = {light.x, light.y, light.z};

        /*
         * This has a bit of fudge to make the light look centered. It may
         * not be exactly correct! but looks good.
         */
        fpoint3d ray_end = {
            ((float)c->x) + (light.x / 2.0),
            ((float)c->y) + (light.y / 2.0),
            ((float)c->z) + (light.z / 2.0)};

        line ray;

        ray.P0 = light_source;
        ray.P1 = ray_end;

        /*
         * For all cells that are closer to the light source.
         */
        i = map_light_cells;
        while (i < c) {

            /*
             * Do a quick distance from light ray check. This is faster than
             * the cube intersection below.
             */
            fpoint3d p = { i->x - light.x, i->y - light.y, i->z - light.z };
            float dist;

            if (!dist_point_line(p, ray.P0, ray.P1, &dist)) {
                i++;
                continue;
            }

            if (dist > cutoff) {
                i++;
                continue;
            }

            /*
             * Ok, now do the slow check as we think this light ray 
             * intersects.
             */
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
                /*
                 * Light misses this cube.
                 */
                i++;
                continue;
            }

            /*
             * This cell could block the light for the source. Record that
             * fact.
             */
            o->shadow = 1;
            o->is_a_cell = false;
            o->dist = distance;
            o->x = i->x;
            o->y = i->y;
            o->z = i->z;
            o++;

            if (o >= map_light_shadows_max) {
                DIE("ran out of shadow space");
            }

            i++;
        }

        c++;
    }

    /*
     * Record the end of the array of shadows we generated.
     */
    map_light_shadows_end = o;

    /*
     * Now dump the shadow array to disk. Compress the file.
     */
    uint8_t *buf = (uint8_t *) map_light_shadows;
    int32_t len = (map_light_shadows_end - map_light_shadows) *
                    sizeof(map_light_shadow);

    LOG("writing light map, %d bytes", len);

#define WRITE_COMPRESSED
#ifdef WRITE_COMPRESSED
    mzip_file_write("../data/map/map_light.data", buf, &len);
#else
    file_write("../data/map/map_light.data", buf, len);
#endif

    LOG("wrote light map, %d bytes", len);
}

/*
 * Either generate a light map, or use the pre-calculated one.
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

    /*
     * First time either load the pre-gen map or make it from scratch.
     */
    static bool done;

    if (!done) {
        done = true;

#ifdef ENABLE_GEN_LIGHT_MAP_FILE
        map_light_generate_raytrace_map(map_ctx, DEFAULT_LIGHT_RAY_LENGTH);
#endif

        /*
         * Now read in the pre-gen map file and uncompress it.
         */
        int32_t len;

        LOG("Loading light map\n");

#ifdef WRITE_COMPRESSED
        map_light_shadows_start = (typeof(map_light_shadows_start))
            mzip_file_read("data/map/map_light.data", &len);
#else
        map_light_shadows_start = (typeof(map_light_shadows_start))
            file_read("data/map/map_light.data", &len);
#endif

        LOG("Load  light map %d bytes\n",len);

        map_light_shadows_end = (typeof(map_light_shadows_end))
            (((uint8_t *) map_light_shadows_start) + len);
    }

    /*
     * Clear out the light map. Assume all cells are in darkness. Only do this 
     * if this is the first light source in the render scene.
     */
#if 0
    if (first_light) {
        for (z = 0; z < MAP_DEPTH; z++) {
            for (x = 0; x < MAP_WIDTH; x++) {
                for (y = 0; y < MAP_HEIGHT; y++) {
                    map->tiles[x][y][z].lit = 0.0;
                }
            }
        }
    }
#endif

    /*
     * The light cell is always lit.
     */
    map->tiles[lx][ly][lz].lit = 1;

    map_light_shadow *s = map_light_shadows_start;

    while (s < map_light_shadows_end) {
        x = lx + s->x;
        y = ly + s->y;
        z = lz + s->z;
        s++;

        /*
         * Do we want to skip this chain of shadow cells?
         */
        boolean skip;

        /*
         * If this cell is oob then jump to the next one.
         */
        if (map_out_of_bounds(x, y, z)) {
            skip = true;
        } else if (s->dist > strength + 1) {
            /*
             * Never go beyond the edge of the light.
             */
            skip = true;
        } else {
            skip = false;
        }

        if (skip) {
            /*
             * We need to skip all the shadow cells in the array for this lit
             * cell still.
             */
            while (s < map_light_shadows_end) {
                /*
                 * Could add a skip count to the first cell if speed is an 
                 * issue?
                 */
                if (s->is_a_cell) {
                    break;
                }

                s++;
            }

            continue;
        }

        /*
         * Add up the shadows from all obstacles.
         */
        float total_shadow = 0.0;
        float max_shadow = 3.00;

        while (s < map_light_shadows_end) {
            /*
             * Start of the next cell shadow datadequqnce?
             */
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

            /*
             * Change this increment for light that ends around obstacles.
             */
            total_shadow += s->shadow;
            if (total_shadow >= max_shadow) {
                skip = true;
            }

            s++;
        }

        float lit;

        if (total_shadow > max_shadow) {
            lit = 0.0;

            map->tiles[x][y][z].lit = 0;
        } else {
            lit = (max_shadow - total_shadow) / max_shadow;

            /*
             * Make the light fade away at the edges.
             */
            float f = s->dist / ((float)strength + 1);
            if (f > 0.5) {
                f = f - 0.5;
                f = f * 2;
                lit -= f;
                if (lit < 0.0) {
                    lit = 0.0;
                }
            }

            map->tiles[x][y][z].lit = (100.0 * lit);
        }
    }
}
