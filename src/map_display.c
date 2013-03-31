/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <SDL.h>
#include "glapi.h"

#include "main.h"
#include "gl.h"
#include "wid.h"
#include "wid_console.h"
#include "color.h"
#include "time.h"
#include "thing.h"
#include "sdl.h"
#include "init_fn.h"
#include "wid_splash.h"
#include "config.h"
#include "tex.h"
#include "ttf.h"
#include "slre.h"
#include "token.h"
#include "map_display.h"

map_frame_ctx_t *map;

/*
 * map_tile_to_tex_coords
 *
 * Given a tile in a tile array, return the tex co-ords.
 */
static inline void
map_tile_to_tex_coords (map_frame_ctx_t *map,
                        const uint16_t tile,
                        GLfloat *tex_left,
                        GLfloat *tex_right,
                        GLfloat *tex_top,
                        GLfloat *tex_bottom)
{
    const uint16_t tx = tile % map->tex_tiles_width; 
    const uint16_t ty = tile / map->tex_tiles_height; 

    *tex_left   = map->tex_float_width * tx;
    *tex_right  = *tex_left + map->tex_float_width;
    *tex_top    = map->tex_float_height * ty;
    *tex_bottom = *tex_top + map->tex_float_height;
}

/*
 * map_chunk_init
 */
static void map_chunk_init (map_frame_ctx_t *map, uint32_t chunk)
{
    const uint32_t sx = chunk * MAP_CHUNK_WIDTH;
    const uint32_t ex = (chunk+1) * MAP_CHUNK_WIDTH;
    const uint32_t sy = 0;
    const uint32_t ey = MAP_CHUNK_HEIGHT;
    uint32_t x;
    uint32_t y;
    uint32_t cnt = 0;

    for (x = sx; x < ex; x++) {
        for (y = sy; y < ey; y++) {
            map->tiles[x][y].tile = cnt++;
            if (cnt > 64*28) {
                cnt = 0;
            }
        }
    }
}

/*
 * map_chunks_init
 */
static void map_chunks_init (map_frame_ctx_t *map)
{
    uint32_t chunk;

    for (chunk = 0; chunk < MAP_CHUNKS_WIDTH; chunk++) {
        map_chunk_init(map, chunk);
    }
}

/*
 * map_gl_init
 */
static void map_gl_init (map_frame_ctx_t *map)
{
    /*
     * Our array size requirements.
     */
    uint32_t gl_array_size_required;

    /*
     * Screen size.
     */
    uint16_t width = global_config.video_pix_width;
    uint16_t height = global_config.video_pix_height;

    /*
     * If the screen size has changed or this is the first run, allocate our
     * buffer if our size requirements have changed.
     */
    gl_array_size_required =
                    width *
                    height *
                    NUMBER_BYTES_PER_ARRAY_ELEM *
                    NUMBER_ARRAY_ELEM_ARRAYS * 2; // for degenerate triangles

    /*
     * Requirements have changed for buffer space?
     */
    if (map->gl_array_size != gl_array_size_required) {
        map->gl_array_size = gl_array_size_required;

        if (map->gl_array_buf) {
            myfree(map->gl_array_buf);
        }

        map->gl_array_buf = myzalloc(gl_array_size_required, "GL xy buffer");
    }

    if (!map->tex) {
        map->tex = tex_find("sprites_small");
        if (!map->tex) {
            return;
        }

        map->bind = tex_get_gl_binding(map->tex);

        map->tex_width = tex_get_width(map->tex);
        map->tex_height = tex_get_height(map->tex);

        map->tex_tile_width = tex_get_tile_width(map->tex);
        map->tex_tile_height = tex_get_tile_height(map->tex);

        map->tex_tiles_width = tex_get_tiles_width(map->tex);
        map->tex_tiles_height = tex_get_tiles_height(map->tex);

        map->tex_float_width  =
                        (1.0 / (float)map->tex_width) * map->tex_tile_width;
        map->tex_float_height =
                        (1.0 / (float)map->tex_height) * map->tex_tile_height;
    }
}

/*
 * map_init
 */
void map_init (map_frame_ctx_t **map)
{
    *map = myzalloc(sizeof(map_frame_ctx_t), "map frame");

    map_gl_init(*map);

    map_chunks_init(*map);
}

/*
 * map_display
 *
 * Our main rendering loop.
 */
void map_display (map_frame_ctx_t *map, const uint32_t mx, const uint32_t my)
{
    glBindTexture(GL_TEXTURE_2D, map->bind);
    glcolor(WHITE);

    /*
     * Where we are currently up to in writing to these buffers.
     */
    GLfloat *bufp;

    /*
     * Our array size requirements.
     */
    uint32_t nvertices;

    /*
     * Individual co-ordinates for each tile.
     */
    GLfloat left;
    GLfloat right;
    GLfloat top;
    GLfloat bottom;

    GLfloat tex_left;
    GLfloat tex_right;
    GLfloat tex_top;
    GLfloat tex_bottom;

    /*
     * Screen size.
     */
    uint16_t width = global_config.video_pix_width;
    uint16_t height = global_config.video_pix_height;

    /*
     * Temps
     */
    uint16_t x;
    uint16_t y;
    map_tile_t *map_tile;

    tex_left   = 0;
    tex_right  = map->tex_float_width;
    tex_top    = 0;
    tex_bottom = map->tex_float_height;

    bufp = map->gl_array_buf;

    uint32_t cx = mx;
    uint32_t cy = my;

    left = 0;

    for (x = 0; x <= width - TILE_WIDTH; x += TILE_WIDTH, cx++) {

        right = left + TILE_WIDTH;
        top = 0;
        cy = my;
        y = 0;

        map_tile = &map->tiles[cx][cy];

        uint32_t tile = map_tile->tile;

        map_tile_to_tex_coords(map, tile,
                               &tex_left,
                               &tex_right,
                               &tex_top,
                               &tex_bottom);

        /*
         * Repeat the first vertex so we create a degenerate triangle.
         */
        if (cx != mx) {
            gl_push_texcoord(&bufp, tex_left,  tex_top);
            gl_push_vertex(&bufp, left,  top);
        }

        for (y = 0;
             y <= height - TILE_HEIGHT;
             y += TILE_HEIGHT, cy++, map_tile++) {

            bottom = top + TILE_HEIGHT;

            tile = map_tile->tile;

            map_tile_to_tex_coords(map, tile,
                                   &tex_left,
                                   &tex_right,
                                   &tex_top,
                                   &tex_bottom);

            /*
             * Repeat the first vertex so we create a degenerate triangle.
             */
            gl_push_texcoord(&bufp, tex_left,  tex_top);
            gl_push_vertex(&bufp, left,  top);

            gl_push_texcoord(&bufp, tex_right, tex_top);
            gl_push_vertex(&bufp, right, top);

            gl_push_texcoord(&bufp, tex_left,  tex_bottom);
            gl_push_vertex(&bufp, left,  bottom);

            gl_push_texcoord(&bufp, tex_right, tex_bottom);
            gl_push_vertex(&bufp, right, bottom);

            top += TILE_HEIGHT;
        }

        /*
         * Repeat the last vertex so we create a degenerate triangle.
         */
        gl_push_texcoord(&bufp, tex_right, tex_bottom);
        gl_push_vertex(&bufp, right, bottom);

        left += TILE_WIDTH;
    }

    /*
     * Display all the tiles selected above in one blast.
     */
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    nvertices = (bufp - map->gl_array_buf) /
                    (NUMBER_DIMENSIONS_PER_COORD * NUMBER_ARRAY_ELEM_ARRAYS);

    glTexCoordPointer(
        NUMBER_DIMENSIONS_PER_COORD, // (x,y)
        GL_FLOAT,
        NUMBER_BYTES_PER_VERTICE * 2,
        map->gl_array_buf);

    glVertexPointer(
        NUMBER_DIMENSIONS_PER_COORD, // (x,y)
        GL_FLOAT,
        NUMBER_BYTES_PER_VERTICE * 2,
        ((char*)map->gl_array_buf) + NUMBER_BYTES_PER_VERTICE);

    glBindTexture(GL_TEXTURE_2D, tex_get_gl_binding(map->tex));

    glDrawArrays(GL_TRIANGLE_STRIP, 0, nvertices);

    glBindTexture(GL_TEXTURE_2D, 0);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}
