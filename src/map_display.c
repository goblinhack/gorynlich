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
#include "map_display.h"
#include "wid.h"

/*
 * QUAD per array element.
 */
#define NUMBER_COORDS_PER_VERTEX 4

/*
 * x and y per element.
 */
#define NUMBER_DIMENSIONS_PER_COORD 2

static const uint32_t NUMBER_BYTES_PER_VERTICE =
                                            sizeof(GLfloat) *
                                            NUMBER_DIMENSIONS_PER_COORD;

static const uint32_t NUMBER_BYTES_PER_ARRAY_ELEM =
                                            sizeof(GLfloat) *
                                            NUMBER_COORDS_PER_VERTEX *
                                            NUMBER_DIMENSIONS_PER_COORD;
/*
 * Two arrays, xy and uv.
 */
static const uint32_t NUMBER_ARRAY_ELEM_ARRAYS = 2;

typedef struct {
    uint16_t tile;
} map_tile_t;

#define MAP_CHUNK_WIDTH 1024
#define MAP_CHUNK_HEIGHT 4096
#define MAP_CHUNKS_WIDTH 3

/*
 * All the rendering info for one parallax frame of tiles.
 */
typedef struct {
    /*
     * All the tiles in this frame.
     */
    map_tile_t tiles[MAP_CHUNK_WIDTH * MAP_CHUNKS_WIDTH][MAP_CHUNK_HEIGHT];

    /*
     * This is the huge buffer that contains all arrays.
     */
    GLfloat *gl_array_buf;
    uint32_t gl_array_size;

    /*
     * Texture for this frame. One texture for all tiles here.
     */
    texp tex;
    int bind;

    /*
     * Size of the texture in pixels.
     */
    uint32_t tex_width;
    uint32_t tex_height;

    /*
     * Single tile size.
     */
    uint32_t tex_tile_width;
    uint32_t tex_tile_height;

    /*
     * How many tiles across and down.
     */
    uint32_t tex_tiles_width;
    uint32_t tex_tiles_height;

    /*
     * And the float size of that tile in the parent tex.
     */
    GLfloat tex_float_width;
    GLfloat tex_float_height;

} map_frame_ctx_t;

static map_frame_ctx_t *map;
static widp wid_map;
static uint16_t mx;
static uint16_t my;

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
 * wid_map_key_event
 */
static boolean wid_map_key_event (widp w, const SDL_keysym *key)
{
    switch (key->sym) {
        case SDLK_LEFT:
            mx--;
            return (true);

        case SDLK_RIGHT:
            mx++;
            return (true);

        case SDLK_UP:
            my--;
            return (true);

        case SDLK_DOWN:
            my++;
            return (true);

        default:
            break;
    }

    return (false);
}

/*
 * map_wid_create
 *
 * Event widget for all in game events.
 */
static void map_wid_create (void)
{
    if (wid_map) {
        return;
    }

    wid_map = wid_new_window("wid map");

    wid_set_no_shape(wid_map);

    fpoint tl = {0.0f, 0.0f};
    fpoint br = {1.0f, 1.0f};
    wid_set_tl_br_pct(wid_map, tl, br);
    wid_set_on_key_down(wid_map, wid_map_key_event);

    color col = BLACK;
    col.a = 0;
    glcolor(col);

    wid_set_mode(wid_map, WID_MODE_NORMAL);
    wid_set_color(wid_map, WID_COLOR_TL, col);
    wid_set_color(wid_map, WID_COLOR_BR, col);
    wid_set_color(wid_map, WID_COLOR_BG, col);

    wid_set_on_key_down(wid_map, wid_map_key_event);

    wid_update(wid_map);
}

/*
 * map_display
 *
 * Render one frame of the map.
 */
static void map_display_ (map_frame_ctx_t *map, uint16_t mx, uint16_t my)
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

    uint16_t cx = mx;
    uint16_t cy = my;

    left = 0;

    for (x = 0; x <= width - TILE_WIDTH; x += TILE_WIDTH, cx++) {

        right = left + TILE_WIDTH;
        top = 0;
        cy = my;
        y = 0;

        map_tile = &map->tiles[cx][cy];

        uint16_t tile = map_tile->tile;

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

/*
 * map_display
 *
 * Render one frame of the map.
 */
void map_display (void)
{
    if (!map) {
        return;
    }

    map_display_(map, mx, my);
}

/*
 * map_init
 */
boolean map_init (void)
{
    map = myzalloc(sizeof(map_frame_ctx_t), "map frame");

    map_gl_init(map);

    map_chunks_init(map);

    map_wid_create();

    return (true);
}

void map_fini (void)
{
    myfree(map);

    if (wid_map) {
        wid_destroy(&wid_map);
    }
}

