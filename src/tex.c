/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <SDL.h>
#include "glapi.h"
#include "stb_image.h"

#include "main.h"
#include "glapi.h"
#include "tex.h"
#include "ramdisk.h"
#include "tree.h"
#include "pixel.h"
#include "color.h"
#include "math_util.h"

typedef struct tex_ {
    tree_key_string tree;
    uint32_t width;
    uint32_t height;
    /*
     * Size of each tile.
     */
    uint32_t tile_width;
    uint32_t tile_height;
    /*
     * How many tiles across and down.
     */
    uint32_t tiles_width;
    uint32_t tiles_height;
    int32_t gl_surface_binding;
    SDL_Surface *surface;
} tex;

tree_root *textures;

static uint8_t tex_init_done;

static texp tex_black_and_white(SDL_Surface *in,
                                uint32_t tile_width,
                                uint32_t tile_height,
                                const char *file,
                                const char *name);
uint8_t tex_init (void)
{
    tex_init_done = true;

    return (true);
}

static void tex_destroy (tex *t)
{
    SDL_FreeSurface(t->surface);

    GLuint gl_surface_binding;
    gl_surface_binding = t->gl_surface_binding;

    if (!HEADLESS) {
        glDeleteTextures(1, &gl_surface_binding);
    }

    oldptr(t->surface);
}

void tex_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (tex_init_done) {
        tex_init_done = false;

        tree_destroy(&textures, (tree_destroy_func)tex_destroy);
    }
}

static unsigned char *load_raw_image (const char *filename,
                                      int32_t *x,
                                      int32_t *y,
                                      int32_t *comp)
{
    unsigned char *ramdisk_data;
    unsigned char *image_data = 0;
    int32_t len;

    ramdisk_data = ramdisk_load(filename, &len);
    if (!ramdisk_data) {
        ERR("could not read ramdisk for file, %s", filename);
    }

    if (strstr(filename, ".tga")) {
        image_data = stbi_tga_load_from_memory(ramdisk_data,
                                               len, x, y, comp, 0);
    } else if (strstr(filename, ".jpg")) {
        image_data = stbi_jpeg_load_from_memory(ramdisk_data,
                                                len, x, y, comp, 0);
    } else if (strstr(filename, ".bmp")) {
        image_data = stbi_bmp_load_from_memory(ramdisk_data,
                                               len, x, y, comp, 0);
    } else if (strstr(filename, ".png")) {
        image_data = stbi_png_load_from_memory(ramdisk_data,
                                               len, x, y, comp, 0);
    } else {
        ERR("unknown suffix for image, %s", filename);
    }

    if (!image_data) {
        ERR("could not read memory for file, %s", filename);
    }

    DBG("Load %s, %ux%u", filename, *x, *y);

    myfree(ramdisk_data);

    return (image_data);
}

static void free_raw_image (unsigned char *image_data)
{
    stbi_image_free(image_data);
}

static SDL_Surface *load_image (const char *filename)
{
    uint32_t rmask, gmask, bmask, amask;
    unsigned char *image_data;
    SDL_Surface *rv;
    int32_t x, y, comp;

    image_data = load_raw_image(filename, &x, &y, &comp);
    if (!image_data) {
        ERR("could not read memory for file, %s", filename);
    }

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif

    if (comp == 4) {
        rv = SDL_CreateRGBSurface(0, x, y, 32, rmask, gmask, bmask, amask);
        newptr(rv, "SDL_CreateRGBSurface");
    } else if (comp == 3) {
        rv = SDL_CreateRGBSurface(0, x, y, 24, rmask, gmask, bmask, 0);
        newptr(rv, "SDL_CreateRGBSurface");
    } else {
        free_raw_image(image_data);
        return (0);
    }

    memcpy(rv->pixels, image_data, comp * x * y);

    free_raw_image(image_data);

    return (rv);
}

/*
 * Load a texture
 */
texp tex_load (const char *file, const char *name)
{
    texp t = tex_find(name);

    if (t) {
        return (t);
    }

    if (!file) {
        if (!name) {
            ERR("no file for tex");
        } else {
            ERR("no file for tex loading %s", name);
        }
    }

    SDL_Surface *surface = 0;
    surface = load_image(file);

    if (!surface) {
        ERR("could not make surface from file, %s", file);
    }

    t = tex_from_surface(surface, file, name);

    return (t);
}

/*
 * Load a texture which has regular tiles with single pixel gaps between
 * each tile
 */
texp tex_load_tiled (const char *file,
                     const char *name,
                     uint32_t tile_width,
                     uint32_t tile_height)
{
    texp t = tex_find(name);

    if (t) {
        return (t);
    }

    if (!file) {
        if (!name) {
            ERR("no file for tex");
        } else {
            ERR("no file for tex loading %s", name);
        }
    }

    SDL_Surface *surface = 0;

    surface = load_image(file);

    if (!surface) {
        ERR("could not make surface from file, %s", file);
    }

#ifdef TILES_WITH_PIXEL_BOUNDARIES_BETWEEN_TILES
    t = tex_from_tiled_surface(surface, tile_width, tile_height, file, name);
#else
    t = tex_from_surface(surface, file, name);
#endif

    t->tile_width = tile_width;
    t->tile_height = tile_height;

    t->tiles_width = tex_get_width(t) / tile_width;
    t->tiles_height = tex_get_height(t) / tile_height;

    return (t);
}

/*
 * Load a texture which has regular tiles with single pixel gaps between
 * each tile
 */
texp tex_load_tiled_black_and_white (const char *file,
                                     const char *name,
                                     uint32_t tile_width,
                                     uint32_t tile_height)
{
    texp t = tex_find(name);

    if (t) {
        return (t);
    }

    if (!file) {
        if (!name) {
            ERR("no file for tex");
        } else {
            ERR("no file for tex loading %s", name);
        }
    }

    SDL_Surface *surface = 0;

    surface = load_image(file);

    if (!surface) {
        ERR("could not make surface from file, %s", file);
    }

    t = tex_black_and_white(surface, tile_width, tile_height, file, name);

    t->tile_width = tile_width;
    t->tile_height = tile_height;

    t->tiles_width = tex_get_width(t) / tile_width;
    t->tiles_height = tex_get_height(t) / tile_height;

    return (t);
}

/*
 * Find an existing tex.
 */
texp tex_find (const char *file)
{
    tex target;
    tex *result;

    if (!file) {
        ERR("no filename given for tex find");
    }

    memset(&target, 0, sizeof(target));
    target.tree.key = (char*) file;

    result = (typeof(result)) tree_find(textures, &target.tree.node);
    if (!result) {
        return (0);
    }

    verify(result->surface);

    return (result);
}

/*
 * Creae a texture from a surface
 */
texp tex_from_surface (SDL_Surface *surface,
                       const char *file,
                       const char *name)
{
    tex *t;

    if (!surface) {
        ERR("could not make surface from file, %s", file);
    }

    /*
     * Check that the sdl_surface's width is a power of 2
     */
    if ((surface->w & (surface->w - 1)) != 0) {
        ERR("%s has a width %u that is not a power of 2", file, surface->w);
    }

    /*
     * Also check if the height is a power of 2
     */
    if ((surface->h & (surface->h - 1)) != 0) {
        ERR("%s has a height %u that is not a power of 2", file, surface->h);
    }

    DBG("Texture: %s, %dx%d", file, surface->w, surface->h);

    /*
     * Get the number of channels in the SDL surface
     */
    int32_t channels = surface->format->BytesPerPixel;
    int32_t textureFormat = 0;

    if (channels == 4) {
        /*
         * Contains alpha channel
         */
        if (surface->format->Rmask == 0x000000ff) {
            textureFormat = GL_RGBA;
        } else {
            textureFormat = GL_BGRA;
        }
    } else if (channels == 3) {
        /*
         * Contains no alpha channel
         */
        if (surface->format->Rmask == 0x000000ff) {
            textureFormat = GL_RGB;
        } else {
#ifdef GL_BGR
            textureFormat = GL_BGR;
#else
            ERR("%s Need support for GL_BGR", file);
#endif
        }
    } else {
        ERR("%s is not truecolor, need %d bytes per pixel", file,
            channels);
    }

    /*
     * Create the tex
     */
    GLuint gl_surface_binding = 0;

    if (!HEADLESS) {
        glGenTextures(1, &gl_surface_binding);
    }

    /*
     * Typical tex generation using data from the bitmap
     */
    if (!HEADLESS) {
        glBindTexture(GL_TEXTURE_2D, gl_surface_binding);
    }

    /*
     * Generate the tex
     */
    if (!HEADLESS) {
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA,
            surface->w,
            surface->h,
            0,
            textureFormat,
            GL_UNSIGNED_BYTE,
            surface->pixels
        );

        /*
         * linear filtering. Nearest is meant to be quicker but I didn't see
         * that in reality.
         */
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    if (!textures) {
        textures = tree_alloc(TREE_KEY_STRING, "TREE ROOT: tex");
    }

    t = (typeof(t)) myzalloc(sizeof(*t), "TREE NODE: tex");
    t->tree.key = dupstr(name, "TREE KEY: tex");

    if (!tree_insert(textures, &t->tree.node)) {
        ERR("tex insert name [%s] failed", name);
    }

    t->width = surface->w;
    t->height = surface->h;
    t->gl_surface_binding = gl_surface_binding;
    t->surface = surface;

    return (t);
}

/*
 * Creae a texture from a tiled surface
 */
texp tex_from_tiled_surface (SDL_Surface *in,
                             uint32_t tile_width,
                             uint32_t tile_height,
                             const char *file,
                             const char *name)
{
    tex *t;

    if (!in) {
        ERR("could not make surface from file, %s", file);
    }

    uint32_t rmask, gmask, bmask, amask;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif

    uint32_t iwidth  = in->w;
    uint32_t iheight = in->h;
    /*
     * Subtract space for the single pixel padding and make a surface to
     * copy pixels to.
     */
    uint32_t owidth  = (in->w / (tile_width  + 1)) * tile_width;
    uint32_t oheight = (in->h / (tile_height + 1)) * tile_height;

    owidth  = nextpoweroftwo(owidth);
    oheight = nextpoweroftwo(oheight);

    int32_t ix;
    int32_t iy;
    uint32_t ox;
    uint32_t oy;

    SDL_Surface *out = SDL_CreateRGBSurface(0, owidth, oheight, 32,
                                            rmask, gmask, bmask, amask);
    newptr(out, "SDL_CreateRGBSurface");

    /*
     * Omit every grid pixel between tiles.
     */
    ox = 0;

    for (ix = 0; ix < (int32_t) iwidth; ix++) {

        oy = 0;

        if (ix && !((ix+1) % (tile_width+1))) {
            continue;
        }

        for (iy = 0; iy < (int32_t) iheight; iy++) {

            if (iy&& !((iy+1) % (tile_height+1))) {
                continue;
            }

            color c;

            c = getPixel(in, ix, iy);

            putPixel(out, ox, oy, c);

            oy++;
        }

        ox++;
    }

#ifdef DEBUG_SURFACE1
    SDL_LockSurface(out);
    stbi_write_tga("neil.tga", out->w, out->h, STBI_rgb_alpha, out->pixels);
    SDL_UnlockSurface(out);
#endif

#ifdef GENERATE_TILES_FROM_TEMPLATE
    /*
     * The first 8 columns of tiles (bricks and the like) we merge with the
     * remaining columns of tiles (edges) to make a combination of the two.
     */
    uint32_t tile_block = (tile_width * 8);
    uint32_t x, y;

    for (x = 0; x < (uint32_t) out->w; x++) {
        for (y = 0; y < (uint32_t) out->h; y++) {
            color c, d;

            if (x > tile_width) {
                d = getPixel(out, x, y);

                if (d.a == 0) {
                    continue;
                }

                if (d.r || d.g || d.b) {
                    continue;
                }

                uint32_t tx = x % tile_block;

                c = getPixel(out, tx, y);

                putPixel(out, x, y, c);
            }
        }
    }

    /*
     * Tile 0 is always transparent black.
     */
    for (x = 0; x < tile_width; x++) {
        for (y = 0; y < tile_height * 2; y++) {
            color c = BLACK;
            c.a = 0;
            putPixel(out, x, y, c);
        }
    }
#endif

#ifdef DEBUG_SURFACE2
    SDL_LockSurface(out);
    stbi_write_tga("neil.tga", out->w, out->h, STBI_rgb_alpha, out->pixels);
    SDL_UnlockSurface(out);
#endif

    SDL_FreeSurface(in);
    oldptr(in);

    t = tex_from_surface(out, file, name);

    return (t);
}

static texp tex_black_and_white (SDL_Surface *in,
                                 uint32_t tile_width,
                                 uint32_t tile_height,
                                 const char *file,
                                 const char *name)
{
    tex *t;

    if (!in) {
        ERR("could not make surface from file, %s", file);
    }

    uint32_t rmask, gmask, bmask, amask;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif

    uint32_t iwidth  = in->w;
    uint32_t iheight = in->h;
    /*
     * Subtract space for the single pixel padding and make a surface to
     * copy pixels to.
     */
    uint32_t owidth  = iwidth;
    uint32_t oheight = iheight;

    int32_t ix;
    int32_t iy;
    uint32_t ox;
    uint32_t oy;

    SDL_Surface *out = SDL_CreateRGBSurface(0, owidth, oheight, 32,
                                            rmask, gmask, bmask, amask);
    newptr(out, "SDL_CreateRGBSurface");

    /*
     * Omit every grid pixel between tiles.
     */
    ox = 0;

    for (ix = 0; ix < (int32_t) iwidth; ix++) {

        oy = 0;

        for (iy = 0; iy < (int32_t) iheight; iy++) {

            color c;

            c = getPixel(in, ix, iy);
            
            uint8_t avg = (c.r + c.g + c.b) / 3;
            c.r = avg;
            c.g = avg;
            c.b = avg;

            putPixel(out, ox, oy, c);

            oy++;
        }

        ox++;
    }

    SDL_FreeSurface(in);
    oldptr(in);

    t = tex_from_surface(out, file, name);

    return (t);
}

int32_t tex_get_gl_binding (tex *tex)
{
    return (tex->gl_surface_binding);
}

uint32_t tex_get_width (tex *tex)
{
    if (!tex) {
        ERR("no texture");
    }

    return (tex->width);
}

uint32_t tex_get_height (tex *tex)
{
    if (!tex) {
        ERR("no texture");
    }

    return (tex->height);
}

uint32_t tex_get_tile_width (tex *tex)
{
    if (!tex) {
        ERR("no texture");
    }

    return (tex->tile_width);
}

uint32_t tex_get_tile_height (tex *tex)
{
    if (!tex) {
        ERR("no texture");
    }

    return (tex->tile_height);
}

uint32_t tex_get_tiles_width (tex *tex)
{
    if (!tex) {
        ERR("no texture");
    }

    return (tex->tiles_width);
}

uint32_t tex_get_tiles_height (tex *tex)
{
    if (!tex) {
        ERR("no texture");
    }

    return (tex->tiles_height);
}

SDL_Surface *tex_get_surface (tex *tex)
{
    return (tex->surface);
}

/*
 * Blits a whole tex.
 */
void tex_blit (tex *tex, point at)
{
    static point tl, br;

    tl.x = at.x - tex->width/2;
    br.y = at.y - tex->height/2;
    br.x = at.x + tex->width/2;
    tl.y = at.y + tex->height/2;

    blit(tex->gl_surface_binding,
         0.0f, 1.0f, 1.0f, 0.0f, tl.x, tl.y, br.x, br.y);
}

texp string2tex (const char **s)
{
    static char tmp[MAXSTR];
    static const char *eo_tmp = tmp + MAXSTR;
    const char *c = *s;
    char *t = tmp;

    while (t < eo_tmp) {
        if ((*c == '\0') || (*c == '$')) {
            break;
        }

        *t++ = *c++;
    }

    if (c == eo_tmp) {
        return (0);
    }

    *t++ = '\0';
    *s += (t - tmp);

    tex find;
    tex *target;

    memset(&find, 0, sizeof(find));
    find.tree.key = tmp;

    target = (typeof(target)) tree_find(textures, &find.tree.node);
    if (!target) {
        ERR("unknown tex [%s]", tmp);
    }

    return (target);
}
