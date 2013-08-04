/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <SDL.h>
#include "glapi.h"

#include "main.h"
#include "gl.h"
#include "color.h"
#include "tex.h"
#include "wid.h"
#include "ttf.h"
#include "time.h"
#include "map.h"

/*
 * QUAD per array element.
 */
#define NUMBER_COORDS_PER_VERTEX 4

/*
 * x and y per element.
 */
#define NUMBER_DIMENSIONS_PER_COORD 2

/*
 * r,g,b,a per element
 */
#define NUMBER_COMPONENTS_PER_COLOR 4

static const uint32_t NUMBER_BYTES_PER_VERTICE =
                                            sizeof(GLfloat) *
                                            NUMBER_DIMENSIONS_PER_COORD +
                                            sizeof(GLfloat) *
                                            NUMBER_DIMENSIONS_PER_COORD +
                                            sizeof(GLfloat) *
                                            NUMBER_COMPONENTS_PER_COLOR;

static const uint32_t NUMBER_BYTES_PER_ARRAY_ELEM =
                                            NUMBER_BYTES_PER_VERTICE * 
                                            NUMBER_COORDS_PER_VERTEX;
/*
 * Two arrays, xy and uv.
 */
static const uint32_t NUMBER_ARRAY_ELEM_ARRAYS = 2;

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
 * map_display_init
 */
void map_display_init (map_frame_ctx_t *map)
{
    /*
     * Our array size requirements.
     */
    uint64_t gl_array_size_required;

    /*
     * If the screen size has changed or this is the first run, allocate our
     * buffer if our size requirements have changed.
     */
    gl_array_size_required = 10 * 1024 * 1024;

    /*
     * Requirements have changed for buffer space?
     */
    if (map->gl_array_size != gl_array_size_required) {
        map->gl_array_size = gl_array_size_required;

        if (map->gl_array_buf) {
            myfree(map->gl_array_buf);
        }

        map->gl_array_buf = myzalloc(gl_array_size_required, "GL xy buffer");
        map->gl_array_buf_end =
                (typeof(map->gl_array_buf_end))
                    ((char *)map->gl_array_buf) + gl_array_size_required;
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
        map->tex_float_height = 2 *
                        (1.0 / (float)map->tex_height) * map->tex_tile_height;
    }
}


/*
 * gl_push
 */
static void
gl_push (float **p,
         float *p_end,
         boolean *first,
         float tex_left,
         float tex_top,
         float tex_right,
         float tex_bottom,
         float left,
         float top,
         float right,
         float bottom,
         float r1, float g1, float b1, float a1,
         float r2, float g2, float b2, float a2,
         float r3, float g3, float b3, float a3,
         float r4, float g4, float b4, float a4)
{
    static float last_tex_right;
    static float last_tex_bottom;
    static float last_right;
    static float last_bottom;

    if (*p + 24 >= p_end) {
        DIE("overflow on gl bug");
    }

    if (!*first) {
        /*
         * If there is a break in the triangle strip then make a degenerate
         * triangle.
         */
        if ((last_right != left) || (last_bottom != bottom)) {
            gl_push_texcoord(p, last_tex_right, last_tex_bottom);
            gl_push_vertex(p, last_right, last_bottom);
            gl_push_rgba(p, r4, g4, b4, a4);

            gl_push_texcoord(p, tex_left,  tex_top);
            gl_push_vertex(p, left,  top);
            gl_push_rgba(p, r1, g1, b1, a1);
        }
    } else {
        *first = false;
    }

    gl_push_texcoord(p, tex_left,  tex_top);
    gl_push_vertex(p, left,  top);
    gl_push_rgba(p, r1, g1, b1, a1);

    gl_push_texcoord(p, tex_left,  tex_bottom);
    gl_push_vertex(p, left,  bottom);
    gl_push_rgba(p, r2, g2, b2, a2);

    gl_push_texcoord(p, tex_right, tex_top);
    gl_push_vertex(p, right, top);
    gl_push_rgba(p, r3, g3, b3, a3);

    gl_push_texcoord(p, tex_right, tex_bottom);
    gl_push_vertex(p, right, bottom);
    gl_push_rgba(p, r4, g4, b4, a4);

    last_tex_right = tex_right;
    last_tex_bottom = tex_bottom;
    last_right = right;
    last_bottom = bottom;
}

static void
map_tile_color (map_frame_ctx_t *map,
                int32_t x, int32_t y, int32_t z,
                float *r, float *g, float *b, float *a)
{
    //
    // z = 0 here, faking the depth
    //
    if (map_out_of_bounds(x, y, 0)) {
        *r = 0.0;
        *g = 0.0;
        *b = 0.0;
        *a = 1.0;
        return;
    }

    uint8_t lit = map->lit[x][y][0];

    if (lit < 1) {
        *r = 1.0;
        *g = 1.0;
        *b = 1.0;
        *a = 1.0;
    } else {
        float flit = ((float)lit) / 100.0;

        *r = flit;
        *g = flit;
        *b = flit;
        *a = 1.0;
    }
}

/*
 * map_display
 *
 * Render one frame of the map.
 */
static void map_display_ (map_frame_ctx_t *map)
{
    glBindTexture(GL_TEXTURE_2D, map->bind);

    /*
     * Where we are currently up to in writing to these buffers.
     */
    GLfloat *bufp;
    GLfloat *bufp_end;

    /*
     * Our array size requirements.
     */
    uint32_t nvertices = 0;

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

    int16_t depth = 1;

    /*
     * Screen size.
     */
    uint16_t width = global_config.video_pix_width;
    uint16_t height = global_config.video_pix_height + depth * TILE_HEIGHT;

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
    bufp_end = map->gl_array_buf_end;

    uint16_t cx_start = map->px / TILE_WIDTH;
    uint16_t cx;
    int16_t z;
    uint8_t pass;
    uint16_t cy = map->py / TILE_HEIGHT;
    uint16_t scy = cy;
    uint16_t tile;
    boolean first = true;
    float r1, g1, b1, a1;
    float r2, g2, b2, a2;
    float r3, g3, b3, a3;
    float r4, g4, b4, a4;

    cy = map->py / TILE_HEIGHT;

    for (y = 0; y <= height; y += TILE_HEIGHT, cy++) {
        /*
         * From bottom to top.
         */
        for (z = 0; z < depth; z++) {
            /*
             * First pass, bottom half of tile, i.e. fake vertical tile
             * Second pass, top half of tile, top flat file
             */
            for (pass = 0; pass < 2; pass++) {
                /*
                 * Smooth horiz scroll offset.
                 */
                top = TILE_HEIGHT - (map->py % TILE_HEIGHT);
                top -= TILE_HEIGHT;
                top += TILE_HEIGHT * (cy - scy);
                top -= (TILE_HEIGHT * z);

                if (pass == 0) {
                    top += TILE_HEIGHT;
                }

                bottom = top + TILE_HEIGHT;

                cx = cx_start;

                //
                // z = 0 here, faking the depth
                //
                map_tile = &map->tiles[cx][cy][0];
                tile = map_tile->tile;

                /*
                 * Smooth vert scroll offset.
                 */
                left = TILE_WIDTH - (map->px % TILE_WIDTH);
                left -= TILE_WIDTH;

                /*
                 * Draw entire row.
                 */
                for (x = 0; x <= width; x += TILE_WIDTH, cx++) {

                    //
                    // z = 0 here, faking the depth
                    //
                    map_tile = &map->tiles[cx][cy][0];
                    tile = map_tile->tile;
                    if (tile) {
                        right = left + TILE_WIDTH;

                        map_tile_to_tex_coords(map, tile,
                                               &tex_left,
                                               &tex_right,
                                               &tex_top,
                                               &tex_bottom);


                        float tex_height = tex_bottom - tex_top;

                        if (pass == 0) {
                            /*
                             * bottom half of tile, i.e. fake vertical tile
                             */
                            tex_top = tex_top + (tex_height / 2.0);

                            /* top left */
                            map_tile_color(map, cx,   cy+1,z,  &r1,&g1,&b1,&a1);
                            /* bottom left */
                            map_tile_color(map, cx,   cy+1,z-1,&r2,&g2,&b2,&a2);
                            /* top right */
                            map_tile_color(map, cx+1, cy+1,z,  &r3,&g3,&b3,&a3);
                            /* bottom right */
                            map_tile_color(map, cx+1, cy+1,z-1,&r4,&g4,&b4,&a4);
                        } else {
                            /*
                             * top half of tile, top flat file
                             */
                            tex_bottom = tex_top+(tex_height / 2.0);

                            /* top left */
                            map_tile_color(map, cx,   cy,   z, &r1,&g1,&b1,&a1);
                            /* bottom left */
                            map_tile_color(map, cx,   cy+1, z, &r2,&g2,&b2,&a2);
                            /* top right */
                            map_tile_color(map, cx+1, cy,   z, &r3,&g3,&b3,&a3);
                            /* bottom right */
                            map_tile_color(map, cx+1, cy+1, z, &r4,&g4,&b4,&a4);
                        }

                        gl_push(&bufp, 
                                bufp_end,
                                &first,
                                tex_left,
                                tex_top,
                                tex_right,
                                tex_bottom,
                                left,
                                top,
                                right,
                                bottom,
                                r1, g1, b1, a1,  /* top left */
                                r2, g2, b2, a2,  /* bottom left */
                                r3, g3, b3, a3,  /* top right */
                                r4, g4, b4, a4); /* bottom right */
                    }

                    left += TILE_WIDTH;
                }
            }
        }
    }

    /*
     * Display all the tiles selected above in one blast.
     */
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    nvertices = ((char*)bufp - (char*)map->gl_array_buf) /
                    NUMBER_BYTES_PER_VERTICE;

    glTexCoordPointer(
        NUMBER_DIMENSIONS_PER_COORD, // (u,v)
        GL_FLOAT,
        NUMBER_BYTES_PER_VERTICE,
        map->gl_array_buf);

    glVertexPointer(
        NUMBER_DIMENSIONS_PER_COORD, // (x,y)
        GL_FLOAT,
        NUMBER_BYTES_PER_VERTICE,
        ((char*)map->gl_array_buf) +
            sizeof(GLfloat) *        // skip (x,y)
            NUMBER_DIMENSIONS_PER_COORD);

    glColorPointer(
        NUMBER_COMPONENTS_PER_COLOR, // (r,g,b,a)
        GL_FLOAT,
        NUMBER_BYTES_PER_VERTICE,
        ((char*)map->gl_array_buf) +
            sizeof(GLfloat) *        // skip (x,y)
            NUMBER_DIMENSIONS_PER_COORD +
            sizeof(GLfloat) *        // skip (u,v)
            NUMBER_DIMENSIONS_PER_COORD);

    glBindTexture(GL_TEXTURE_2D, tex_get_gl_binding(map->tex));

    glDrawArrays(GL_TRIANGLE_STRIP, 0, nvertices);

    glBindTexture(GL_TEXTURE_2D, 0);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}

/*
 * map_display_debug
 *
 * Render one frame of the map.
 */
static void 
map_display_debug (map_frame_ctx_t *map, uint32_t x, uint32_t y)
{
    if (!map) {
        return;
    }

    static char text[40] = {0};

    snprintf(text, sizeof(text), "(%d,%d) .. (%d,%d)",
             map->px / TILE_WIDTH,
             map->py / TILE_HEIGHT,
             (map->px / TILE_WIDTH) + map->tiles_per_screen_x,
             (map->py / TILE_HEIGHT) + map->tiles_per_screen_y);

    glcolor(RED);

    ttf_puts(small_font, text, x, y, 1.0, 1.0, true);

    float map_scale = 10;

    float x1 = 0;
    float x2 = ((float)map->map_width) / map_scale;
    float y1 = 0;
    float y2 = ((float)map->map_height) / map_scale;

    x1 += x;
    x2 += x;
    y1 += y + 20;
    y2 += y + 20;

    glBindTexture(GL_TEXTURE_2D, 0);

    color c = CYAN;
    c.a = 200;
    glcolor(c);
    gl_blitquad(x1,y1,x2,y2);

    x1 = map->px / TILE_WIDTH;
    x2 = x1 + map->tiles_per_screen_x;

    y1 = map->py / TILE_HEIGHT;
    y2 = y1 + map->tiles_per_screen_y;

    x1 /= map_scale;
    x2 /= map_scale;
    y1 /= map_scale;
    y2 /= map_scale;

    x1 += x;
    x2 += x;
    y1 += y + 20;
    y2 += y + 20;

    c = GREEN;
    c.a = 200;
    glcolor(c);
    gl_blitquad(x1,y1,x2,y2);
}

/*
 * map_display
 *
 * Render one frame of the map.
 */
void map_display (void)
{
    if (!map_ctx) {
        return;
    }

    color c = WHITE;
    glcolor(c);

    map_display_(map_ctx);

    if (fps_enabled) {
        map_display_debug(map_ctx, 100, 0);
    }
}
