/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <SDL.h>
#include "glapi.h"
#include "stb_image.h"

#include "main.h"
#include "math.h"
#include "gl.h"
#include "pixel.h"
#include "color.h"
#include "bits.h"
#include "tex.h"
#include "tile.h"
#include "ttf.h"
#include "ramdisk.h"
#include "time.h"
#include "string_ext.h"
#include "string.h"

#define TTF_FIXED_WIDTH_CHAR '0'
#define TTF_GLYPH_MIN ' '
#define TTF_GLYPH_MAX 126
#define TTF_TABSTOP 80

/*
 * Enable this to generate the font bitmaps.
 */
#define nTTF2TGA

#ifdef ENABLE_GENERATE_TTF
#include "SDL_ttf.h"
#endif

typedef struct {
    uint32_t width;
    uint32_t height;
    int32_t minx;
    int32_t maxx;
    int32_t miny;
    int32_t maxy;
    int32_t advance;
    float texMinX;
    float texMaxX;
    float texMinY;
    float texMaxY;
} glyph;

typedef struct {
    SDL_Surface *image;
    uint32_t tex;
} glyphtex;

typedef struct font {
    glyph glyphs[TTF_GLYPH_MAX+1];
    glyphtex tex[TTF_GLYPH_MAX+1];
    SDL_Color foreground;
    SDL_Color background;
} font;

#ifdef ENABLE_GENERATE_TTF
static void ttf_create_tex_from_char(TTF_Font *ttf, const char *name,
                                     font *f, uint8_t c);
#endif

static boolean ttf_init_done;

boolean ttf_init (void)
{
#ifdef ENABLE_GENERATE_TTF
    if (TTF_Init() < 0) {
        ERR("cannot init SDL_ttf");
        return (false);
    }
#endif
    ttf_init_done = true;

    return (true);
}

void ttf_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (ttf_init_done) {
        ttf_init_done = false;
#ifdef ENABLE_GENERATE_TTF
        TTF_Quit();
#endif
    }
}

#ifdef ENABLE_GENERATE_TTF
/*
 * Load a new font and create textures for each glyph
 */
font *ttf_new (const char *name, int32_t pointSize, int32_t style)
{
    TTF_Font *ttf;
    uint8_t c;
    font *f;

    f = (fontp)myzalloc(sizeof(*f), __FUNCTION__);

    DBG("Load TTF: %s", name);

    ttf = TTF_OpenFont(name, pointSize);
    if (!ttf) {
        DIE("cannot open font file %s", name);
    }

    f->foreground.r = 255;
    f->foreground.g = 255;
    f->foreground.b = 255;
    f->background.r = 0;
    f->background.g = 0;
    f->background.b = 0;

    TTF_SetFontStyle(ttf, style);

    if (pointSize > 40) {
        TTF_SetFontOutline(ttf, 1.5);
    } else {
        TTF_SetFontOutline(ttf, 0.5);
    }

    for (c = TTF_GLYPH_MIN; c < TTF_GLYPH_MAX; c++) {
        ttf_create_tex_from_char(ttf, name, f, c);
    }

    TTF_CloseFont(ttf);

    return (f);
}
#endif

void ttf_free (font *f)
{
    if (!f) {
        return;
    }

    myfree(f);
}

/*
 * Return a SDL rectangle with the size of the font
 */
void ttf_text_size (font *f, const char *text,
                    uint32_t *w, uint32_t *h,
                    enum_fmt *fmt,
                    double scaling,
                    double advance,
                    boolean fixed_width)
{
    boolean found_format_string = false;
    int32_t c;
    texp tex;
    tilep tile;
    int32_t x_start = 0;
    int32_t x;
    enum_fmt _fmt;

    x = 0;
    *w = 0;
    *h = f->glyphs[(uint32_t)TTF_FIXED_WIDTH_CHAR].height * scaling;

    while ((c = *text++) != '\0') {
	if (!found_format_string) {
	    if (c == '%') {
		found_format_string = true;
		continue;
	    }
	} else if (found_format_string) {
	    if (c == '%') {
		if (!strncmp(text, "fg=", 3)) {
		    text += 3;
                    (void) string2color(&text);
		    found_format_string = false;
		    continue;
		} else if (!strncmp(text, "fmt=", 4)) {
		    text += 4;

                    _fmt = string2fmt(&text);
                    if (fmt) {
                        *fmt = _fmt;
                    }

		    found_format_string = false;
		    continue;
		} else if (!strncmp(text, "tex=", 4)) {
		    text += 4;
                    tex = string2tex(&text);
		    found_format_string = false;
                    *w += tex_get_width(tex) * scaling * advance;
		    continue;
		} else if (!strncmp(text, "tile=", 5)) {
		    text += 5;
                    tile = string2tile(&text);
		    found_format_string = false;
                    *w += tile_get_width(tile) * scaling * advance;
		    continue;
		}
	    }
	}

	found_format_string = false;

        if (c == '\t') {
            x = ((((x-x_start) / TTF_TABSTOP) + 1) * TTF_TABSTOP);
            x = x + x_start;
        } else {
            if (fixed_width) {
                x += f->glyphs[TTF_FIXED_WIDTH_CHAR].width * scaling * advance;
            } else {
                if (c == TTF_CURSOR_CHAR) {
                    x += f->glyphs[TTF_FIXED_WIDTH_CHAR].width * scaling * advance;
                } else {
                    x += f->glyphs[c].width * scaling * advance;
                }
            }
        }
    }

    *w = x;
}

/*
 * Blit the font to the screen
 */
void ttf_putc (font *f, int32_t c, int32_t x, int32_t y, double scaling)
{
    double texMinX = f->glyphs[c].texMinX;
    double texMaxX = f->glyphs[c].texMaxX;
    double texMinY = f->glyphs[c].texMinY;
    double texMaxY = f->glyphs[c].texMaxY;

    if (c == TTF_CURSOR_CHAR) {
        static boolean first = true;
        static uint32_t last;

        c = TTF_FIXED_WIDTH_CHAR;

        GLfloat left = (GLfloat)(x);
        GLfloat right = (GLfloat)(x + f->glyphs[c].width * scaling);
        GLfloat top = (GLfloat)(y);
        GLfloat bottom = (GLfloat)(y + f->glyphs[c].height * (scaling));

        glBindTexture(GL_TEXTURE_2D, 0);

        if (first) {
            first = false;
            last = time_get_time_cached();
        }

        glcolor_save();

        if (time_have_x_tenths_passed_since(10, last)) {
            glcolor(CONSOLE_CURSOR_COLOR);
            last = time_get_time_cached();
        } else if (time_have_x_tenths_passed_since(5, last)) {
            glcolor(CONSOLE_CURSOR_COLOR);
        } else {
            glcolor(BLACK);

            gl_blitsquare(left, top, right, bottom);

            left += 1;
            right -= 1;
            top += 1;
            bottom -= 1;

            gl_blitsquare(left, top, right, bottom);

            glcolor_restore();
            return;
        }

        gl_blitquad(left, top, right, bottom);

        glcolor(BLACK);

        left += 1;
        right -= 1;
        top += 1;
        bottom -= 1;

        gl_blitquad(left, top, right, bottom);

        glcolor_restore();
        return;
    }

    GLfloat left = (GLfloat)(x);
    GLfloat right = (GLfloat)(x + f->glyphs[c].width * scaling);
    GLfloat top = (GLfloat)(y);
    GLfloat bottom = (GLfloat)(y + f->glyphs[c].height * (scaling));

    glBindTexture(GL_TEXTURE_2D, f->tex[c].tex);
    blit(texMinX, texMinY, texMaxX, texMaxY, left, top, right, bottom);
}

/*
 * Blit the font to the screen
 */
static void ttf_puts_internal (font *f, const char *text,
                               int32_t x, int32_t y,
                               double scaling, double advance,
                               boolean include_formatting,
                               boolean draw_cursor,
                               boolean fixed_width)
{
    color saved_color = gl_color_current();
    boolean color_retore_needed = false;
    boolean found_format_string = false;
    int32_t c;
    texp tex;
    tilep tile;
    color fg;
    int32_t x_start = x;

    while ((c = *text++) != '\0') {
	if (!found_format_string) {
	    if (c == '%') {
		found_format_string = true;
		continue;
	    }
	} else if (found_format_string) {
	    if (c == '%') {
		if (!strncmp(text, "fg=", 3)) {
		    text += 3;

                    fg = string2color(&text);
                    fg.a = saved_color.a;

                    if (!color_retore_needed) {
                        color_retore_needed = true;
                        if (include_formatting) {
                            glcolor_save();
                        }
                    }

                    if (include_formatting) {
                        glcolor(fg);
                    }

		    found_format_string = false;
		    continue;

		} else if (!strncmp(text, "fmt=", 4)) {
		    text += 4;

                    (void)string2fmt(&text);

		    found_format_string = false;
		    continue;

		} else if (!strncmp(text, "tex=", 4)) {
		    text += 4;
                    tex = string2tex(&text);

                    /*
                     * Move forward half the texture size so it is centered.
                     */
                    x += (tex_get_width(tex) * scaling * advance)/2;

                    point at;
                    at.x = x;
                    at.y = y + (tex_get_height(tex) * scaling * advance)/2;
                    tex_blit(tex, at);

                    /*
                     * Now the other half.
                     */
                    x += (tex_get_width(tex) * scaling * advance)/2;

		    found_format_string = false;
		    continue;

		} else if (!strncmp(text, "tile=", 5)) {
		    text += 5;
                    tile = string2tile(&text);

                    /*
                     * Move forward half the tile size so it is centered.
                     */
                    x += (tile_get_width(tile) * scaling * advance)/2;

                    point at;
                    at.x = x;
                    at.y = y + (tile_get_height(tile) * scaling * advance)/2;
                    tile_blit(tile, 0, at);

                    /*
                     * Now the other half.
                     */
                    x += (tile_get_width(tile) * scaling * advance)/2;

		    found_format_string = false;
		    continue;
		}
	    }
	}

	found_format_string = false;

        if (c == '\t') {
            x = ((((x-x_start) / TTF_TABSTOP) + 1) * TTF_TABSTOP);
            x = x + x_start;
        } else {
            double maxc = f->glyphs[TTF_FIXED_WIDTH_CHAR].width;
            double thisc = f->glyphs[c].width;
            double pad = ((maxc - thisc) * scaling * advance) / 2.0;

            if (fixed_width) {
                x += pad;
            }

            if (c == TTF_CURSOR_CHAR) {
                if (draw_cursor) {
                    ttf_putc(f, c, x, y, scaling);
                }
            } else {
                ttf_putc(f, c, x, y, scaling);
            }

            if (fixed_width) {
                x += thisc;
                x += pad;
            } else {
                if (c == TTF_CURSOR_CHAR) {
                    x += f->glyphs[TTF_FIXED_WIDTH_CHAR].width * scaling * advance;
                } else {
                    x += f->glyphs[c].width * scaling * advance;
                }
            }
        }
    }

    if (color_retore_needed) {
        glcolor(saved_color);
    }
}

void ttf_puts (font *f, const char *text, int32_t x, int32_t y,
               double scaling, double advance,
               boolean fixed_width)
{
    ttf_puts_internal(f, text, x, y, scaling, advance,
                      true, /* include formatting */
                      true, /* include cursor */
                      fixed_width);
}

void ttf_puts_no_fmt (font *f, const char *text, int32_t x, int32_t y,
                      double scaling, double advance,
                      boolean fixed_width)
{
    ttf_puts_internal(f, text, x, y, scaling, advance,
                      false, /* include formatting */
                      false, /* include cursor */
                      fixed_width);
}

//
// Create a tex from a surface. Set the alpha according to the color key.
// Pixels that match the color key get an alpha of zero while all other pixels
// get an alpha of one. We use black for the color key.
//
#ifdef ENABLE_GENERATE_TTF
static void
ttf_set_color_key (SDL_Surface *glyph_surface,
                   GLfloat *texcoord,
                   uint8_t ckr,
                   uint8_t ckg,
                   uint8_t ckb,
                   uint32_t *width,
                   uint32_t *height)
{
    SDL_Surface *tmp;
    uint32_t colorkey;

    /*
     * Use the surface width and height expanded to powers of 2
     */
    *width = glyph_surface->w;
    *height = glyph_surface->h;

    texcoord[0] = 0; // Min X
    texcoord[1] = 0; // Min Y
    texcoord[2] =
        (GLfloat)(((double)glyph_surface->w) / ((double)*width));  // Max X
    texcoord[3] =
        (GLfloat)(((double)glyph_surface->h) / ((double)*height)); // Max Y

    tmp = SDL_CreateRGBSurface(glyph_surface->flags,
                               *width, *height, 32,
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
                               0xFF000000,
                               0x00FF0000,
                               0x0000FF00,
                               0x000000FF
#else
                               0x000000FF,
                               0x0000FF00,
                               0x00FF0000,
                               0xFF000000
#endif
                            );

    if (!tmp) {
        ERR("Failed to make RGB surface size %d %d: %s",
            *width, *height, SDL_GetError());
        return;
    }

    newptr(tmp, "SDL_CreateRGBSurface");

    /*
     * Set up so that colorkey pixels become transparent
     */
    colorkey = SDL_MapRGBA(tmp->format, 0, 0, 0, 0);
    SDL_FillRect(tmp, 0, colorkey);

    colorkey = SDL_MapRGBA(glyph_surface->format, ckr, ckg, ckb, 0);
#if (SDL_MAJOR_VERSION == 2)
    SDL_SetColorKey(glyph_surface, SDL_TRUE, colorkey);
#else
    SDL_SetColorKey(glyph_surface, SDL_SRCCOLORKEY, colorkey);
#endif

    SDL_FreeSurface(tmp);
    oldptr(tmp);
}

/*
 * Given a single character, make it into an opengl tex
 */
static void
ttf_create_tex_from_char (TTF_Font *ttf, const char *name, font *f, uint8_t c)
{
    static char text[2];
    int32_t e;

    /*
     * Load the glyph info
     */
    e = TTF_GlyphMetrics(ttf, c,
                         &f->glyphs[c].minx,
                         &f->glyphs[c].maxx,
                         &f->glyphs[c].miny,
                         &f->glyphs[c].maxy,
                         &f->glyphs[c].advance);
    if (e != 0) {
        ERR("error loading font glyph %u %s", c, name);
        return;
    }

    text[0] = c;
    text[1] = '\0';

    SDL_Surface *g0 =
            TTF_RenderText_Shaded(ttf, text, f->foreground, f->background);
    if (!g0) {
        ERR("error rendering font glyph %u %s", c, name);
        return;
    }

    SDL_Surface *g1 = 
            TTF_RenderText_Shaded(ttf, text, f->foreground, f->background);
    if (!g1) {
        ERR("error getting display format for font glyph %u %s", c, name);
        return;
    }

    SDL_FreeSurface(g0);

    GLfloat texcoord [4];

    f->tex[c].image = g1;
    f->tex[c].tex = 0;

    ttf_set_color_key(g1, texcoord, 0, 0, 0,
                      &f->glyphs[c].width, &f->glyphs[c].height);

    f->glyphs[c].texMinX = texcoord[0];
    f->glyphs[c].texMinY = texcoord[1];
    f->glyphs[c].texMaxX = texcoord[2];
    f->glyphs[c].texMaxY = texcoord[3];
}
#endif

font *
ttf_read_tga (char *name, int32_t pointsize)
{
    char filename[MAXSTR];
    uint32_t c;
    texp tex;
    font *f;

    f = (typeof(f)) myzalloc(sizeof(*f), "TTF font");
    if (!f) {
        DIE("could not alloc font %s", name);
    }

    /*
     * Read the glyph data.
     */
    snprintf(filename, sizeof(filename), "%s_pointsize%u.data",
             name, pointsize);

    const unsigned char *glyph_data = ramdisk_load(filename, 0);
    if (!glyph_data) {
        DIE("could not load font %s data", filename);
    }

    memcpy(f->glyphs, glyph_data, sizeof(f->glyphs));

    snprintf(filename, sizeof(filename), "%s_pointsize%u.tga",
             name, pointsize);

    tex = tex_load(filename,
                   filename /* to make unique for same point size */);
    if (!tex) {
        DIE("could not load font %s tex", filename);
    }

    for (c = TTF_GLYPH_MIN; c < TTF_GLYPH_MAX; c++) {
        f->tex[c].tex = tex_get_gl_binding(tex);
        f->tex[c].image = tex_get_surface(tex);
    }

    return (f);
}

#ifdef ENABLE_GENERATE_TTF
font *
ttf_write_tga (char *name, int32_t pointsize)
{
    uint32_t rmask, gmask, bmask, amask;
    uint32_t glyph_per_row;
    char filename[200];
    SDL_Surface *dst;
    uint32_t height;
    uint32_t width;
    int16_t maxx;
    int16_t maxy[TTF_GLYPH_MAX];
    uint32_t c;
    uint32_t x;
    uint32_t y;
    uint32_t h;
    font *f;

    /*
     * x glyphs horizontally and y vertically.
     */
    glyph_per_row = 16;

    f = ttf_new(name, pointsize, TTF_STYLE_NORMAL);
    if (!f) {
        DIE("could not create font %s", name);
    }

    maxx = 0;
    memset(maxy, 0, sizeof(maxy));

    /*
     * Find the largest font glyph pointsize.
     */
    x = 0;
    y = 0;
    height = 0;

    for (c = TTF_GLYPH_MIN; c < TTF_GLYPH_MAX; c++) {

        if (f->tex[c].image) {
            maxx = max(maxx, f->tex[c].image->w);
            maxy[y] = max(maxy[y], f->tex[c].image->h);
        }

        if (++x >= glyph_per_row) {
            x = 0;
            height += maxy[y];
            y++;
        }
    }

    if (!maxx) {
        DIE("no glyphs in font %s", name);
    }

    width = glyph_per_row * maxx;

    if (MULTIPLE_BITS(width)) {
        width = nextpoweroftwo(width);
    }

    height += 40;

    if (MULTIPLE_BITS(height)) {
        height = nextpoweroftwo(height);
    }

    /*
     * Make a large surface for all glyphs.
     */
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

    dst = SDL_CreateRGBSurface(0, width, height, 32,
                               rmask, gmask, bmask, amask);
    if (!dst) {
        DIE("no surface created for size %dx%d font %s", width, height, name);
    }

    newptr(dst, "SDL_CreateRGBSurface");

    /*
     * Blit each glyph to the large surface.
     */
    x = 0;
    y = 0;
    h = 0;

    for (c = TTF_GLYPH_MIN; c < TTF_GLYPH_MAX; c++) {

        if (f->tex[c].image) {
            SDL_Rect dstrect = { maxx * x, h, maxx, maxy[y] };

            SDL_BlitSurface(f->tex[c].image, 0, dst, &dstrect);
        }

        if (++x >= glyph_per_row) {
            x = 0;
            h += maxy[y];
            y++;
        }
    }

    /*
     * Convert the black border smoothing that ttf adds into alpha.
     */
    {
        int32_t x;
        int32_t y;

        for (x = 0; x < dst->w; x++) {
            for (y = 0; y < dst->h; y++) {

                color c;

                c = getPixel(dst, x, y);

                if ((c.a == 255) &&
                    (c.r == 255) &&
                    (c.g == 255) &&
                    (c.b == 255)) {
                    /*
                     * Do nothing.
                     */
                } else if ((c.a == 0) &&
                    (c.r == 0) &&
                    (c.g == 0) &&
                    (c.b == 0)) {
                    /*
                     * Do nothing.
                     */
                } else {
                    /*
                     * Convery gray to white with alpha.
                     */
                    c.a = (c.r + c.g + c.b) / 3;
                    c.r = 255;
                    c.g = 255;
                    c.b = 255;
                }

                putPixel(dst, x, y, c);
            }
        }
    }

    snprintf(filename, sizeof(filename), "%s_pointsize%u.tga",
             name, pointsize);

    SDL_LockSurface(dst);
    stbi_write_tga(filename, dst->w, dst->h, STBI_rgb_alpha, dst->pixels);
    SDL_UnlockSurface(dst);

    texp tex;
    tex = tex_from_surface(dst, filename, filename);
    if (!tex) {
        DIE("could not convert %s to tex", filename);
    }

    /*
     * Work our the tex co-ords for each glyph in the large tex.
     */
    x = 0;
    y = 0;
    h = 0;

    for (c = TTF_GLYPH_MIN; c < TTF_GLYPH_MAX; c++) {

        f->tex[c].image = dst;
        f->tex[c].tex = tex_get_gl_binding(tex);

        f->glyphs[c].texMinX =
                        (double)(x * maxx) /
                        (double)dst->w;
        f->glyphs[c].texMaxX =
                        (double)((x * maxx) + f->glyphs[c].width) /
                        (double)dst->w;
        f->glyphs[c].texMinY =
                        (double)(h) /
                        (double)dst->h;
        f->glyphs[c].texMaxY =
                        (double)(h + f->glyphs[c].height) /
                        (double)dst->h;

        if (++x >= glyph_per_row) {
            x = 0;
            h += maxy[y];
            y++;
        }
    }

    /*
     * Save the glyph data.
     */
    snprintf(filename, sizeof(filename), "%s_pointsize%u.data",
             name, pointsize);

    FILE *out = fopen(filename, "w");

    fwrite(f->glyphs, sizeof(f->glyphs), 1, out);

    fclose(out);

    printf("wrote %s\n",filename);
    return (f);
}
#endif

