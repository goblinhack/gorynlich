/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */

#include <string.h>

#include "glapi.h"
#include "math_util.h"

#define GL_MAX_BUFFER_SIZE 1024

GLsizei gl_state;

GLfloat xy[4*GL_MAX_BUFFER_SIZE];
GLfloat uv[4*GL_MAX_BUFFER_SIZE];
uint8_t col[4*GL_MAX_BUFFER_SIZE];

GLfloat *xyp = xy;
GLfloat *uvp = uv;
uint8_t *colp = col;

#define TexCoord2f(x, y)                        \
    *uvp++ = x;                                 \
    *uvp++ = y;

#define Vertex2f(x, y)                          \
    *xyp++ = x;                                 \
    *xyp++ = y;

#define Color4u(color)                          \
    *colp++ = color.r;                          \
    *colp++ = color.g;                          \
    *colp++ = color.b;                          \
    *colp++ = color.a;                          \

static void Begin (GLsizei gl_enum)
{
    gl_state = gl_enum;
}

static inline void End (void)
{
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(2, GL_FLOAT, 0, xy);
    glTexCoordPointer(2, GL_FLOAT, 0, uv);
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, col);

    glDrawArrays(gl_state, 0, (int32_t)((xyp - xy) / 2));

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    xyp = xy;
    uvp = uv;
    colp = col;
}

static inline void vertex (fpoint tl, fpoint br, float x, float y,
                           fsize tex_tl, fsize tex_br, fsize tex_uv)
{
    float tx = (x - tl.x) / tex_uv.width;
    tx *= tex_br.width - tex_tl.width;

    float ty = (y - tl.y) / tex_uv.height;
    ty *= tex_br.height - tex_tl.height;

    Color4u(gl_color_current());
    TexCoord2f(tx, ty);
    Vertex2f(x, y);
}

static void quaduv (fpoint tl, fpoint br,
                    fpoint a, fpoint b, fpoint c, fpoint d,
                    fsize tex_tl, fsize tex_br, fsize uv)
{
    Begin(GL_TRIANGLE_STRIP);

    vertex(tl, br, a.x, a.y, tex_tl, tex_br, uv);
    vertex(tl, br, b.x, b.y, tex_tl, tex_br, uv);
    vertex(tl, br, d.x, d.y, tex_tl, tex_br, uv);
    vertex(tl, br, c.x, c.y, tex_tl, tex_br, uv);

    End();
}

/*
 * Draw a shaded square
 */
void gl_list_square (fpoint tl, fpoint br,
                     texp tex, fsize tex_tl, fsize tex_br, fsize uv,
                     color hi, color med, color lo)
{
    glEnable(GL_TEXTURE_2D);

    /*
     * Draw the central flat square
     */
    glcolor(med);

    Begin(GL_TRIANGLE_STRIP);

    if (!memcmp(&hi, &med, sizeof(color)) &&
        !memcmp(&hi, &lo, sizeof(color))) {
        vertex(tl, br, tl.x, tl.y, tex_tl, tex_br, uv);
        vertex(tl, br, br.x, tl.y, tex_tl, tex_br, uv);
        vertex(tl, br, tl.x, br.y, tex_tl, tex_br, uv);
        vertex(tl, br, br.x, br.y, tex_tl, tex_br, uv);
    } else {
        glcolor(hi);
        vertex(tl, br, tl.x, tl.y, tex_tl, tex_br, uv);
        glcolor(med);
        vertex(tl, br, br.x, tl.y, tex_tl, tex_br, uv);
        vertex(tl, br, tl.x, br.y, tex_tl, tex_br, uv);
        glcolor(lo);
        vertex(tl, br, br.x, br.y, tex_tl, tex_br, uv);
    }

    End();
}

/*
 * Draw a square with 3D relief
 */
void gl_list_square_bevelled (fpoint tl, fpoint br,
                              texp tex, fsize tex_tl, fsize tex_br,
                              fsize uv, color hi, color med, color lo,
                              fsize bevel)
{
    fpoint a = {tl.x, tl.y};
    fpoint b = {br.x, tl.y};
    fpoint c = {br.x, br.y};
    fpoint d = {tl.x, br.y};
    fpoint a1 = {tl.x + bevel.width, tl.y + bevel.height};
    fpoint b1 = {br.x - bevel.width, tl.y + bevel.height};
    fpoint c1 = {br.x - bevel.width, br.y - bevel.height};
    fpoint d1 = {tl.x + bevel.width, br.y - bevel.height};

    glEnable(GL_TEXTURE_2D);

    /*
     * Draw the central flat square
     */
    Begin(GL_TRIANGLE_FAN);

    glcolor(hi);
    vertex(tl, br, a.x, a.y, tex_tl, tex_br, uv);
    glcolor(med);
    vertex(tl, br, b.x, b.y, tex_tl, tex_br, uv);
    glcolor(lo);
    vertex(tl, br, c.x, c.y, tex_tl, tex_br, uv);
    glcolor(med);
    vertex(tl, br, d.x, d.y, tex_tl, tex_br, uv);

    End();

    /*
     * Now draw the bevelled border
     */
    glcolor(lo);
    quaduv(tl, br, c1, c, d, d1, tex_tl, tex_br, uv);
    glcolor(hi);
    quaduv(tl, br, a, a1, d1, d, tex_tl, tex_br, uv);

    glcolor(hi);
    quaduv(tl, br, a, b, b1, a1, tex_tl, tex_br, uv);
    glcolor(lo);
    quaduv(tl, br, b, c, c1, b1, tex_tl, tex_br, uv);
}

/*
 * Draw a square with 3D relief
 */
void gl_list_square_bevelled_plain (fpoint tl, fpoint br,
                                    texp tex, fsize tex_tl, fsize tex_br,
                                    fsize uv, color hi, color med, color lo,
                                    fsize bevel)
{
    fpoint a = {tl.x, tl.y};
    fpoint b = {br.x, tl.y};
    fpoint c = {br.x, br.y};
    fpoint d = {tl.x, br.y};
    fpoint a1 = {tl.x + bevel.width, tl.y + bevel.height};
    fpoint b1 = {br.x - bevel.width, tl.y + bevel.height};
    fpoint c1 = {br.x - bevel.width, br.y - bevel.height};
    fpoint d1 = {tl.x + bevel.width, br.y - bevel.height};

    glEnable(GL_TEXTURE_2D);

    /*
     * Draw the central flat square
     */
    Begin(GL_TRIANGLE_FAN);

    glcolor(med);
    vertex(tl, br, a.x, a.y, tex_tl, tex_br, uv);
    vertex(tl, br, b.x, b.y, tex_tl, tex_br, uv);
    vertex(tl, br, c.x, c.y, tex_tl, tex_br, uv);
    vertex(tl, br, d.x, d.y, tex_tl, tex_br, uv);

    End();

    /*
     * Now draw the bevelled border
     */
    glcolor(lo);
    quaduv(tl, br, c1, c, d, d1, tex_tl, tex_br, uv);
    glcolor(hi);
    quaduv(tl, br, a, a1, d1, d, tex_tl, tex_br, uv);

    glcolor(hi);
    quaduv(tl, br, a, b, b1, a1, tex_tl, tex_br, uv);
    glcolor(lo);
    quaduv(tl, br, b, c, c1, b1, tex_tl, tex_br, uv);
}

/*
 * Draw a rounded square
 */
void gl_list_rounded_square (fpoint tl, fpoint br,
                             texp tex, fsize tex_tl, fsize tex_br, fsize uv,
                             color hi, color med, color lo,
                             fsize radius, int32_t sides)
{
    radius.width = min(radius.width, (br.x - tl.x)/3);
    radius.height = min(radius.height, (br.y - tl.y)/3);

    glcolor(med);

    glEnable(GL_TEXTURE_2D);

    fpoint a = {tl.x, tl.y + radius.height};
    fpoint b = {br.x, tl.y + radius.height};
    fpoint c = {br.x, br.y - radius.height};
    fpoint d = {tl.x, br.y - radius.height};

    fpoint a1 = {tl.x + radius.width, tl.y};
    fpoint b1 = {br.x - radius.width, tl.y};
    fpoint c1 = {br.x - radius.width, br.y};
    fpoint d1 = {tl.x + radius.width, br.y};

    /*
     * Draw the central flat square
     */
    quaduv(tl, br, a1, b1, c1, d1, tex_tl, tex_br, uv);

    /*
     * Now side squares
     */
    quaduv(tl, br, a, b, c, d, tex_tl, tex_br, uv);

    float step = RAD_360 / (float)sides;

    /*
     * Now draw the rounded filled edges, smaller, plain
     */

    /*
     * Top right
     */
    Begin(GL_TRIANGLE_FAN);

    float x = br.x - radius.width;
    float y = br.y - radius.height;
    float j = 0.0f;

    vertex(tl, br, x, y, tex_tl, tex_br, uv);

    int32_t i;
    for (i = 0; i <= sides/4; i++) {

        vertex(tl, br,
            x + radius.width*sin(j),
            y + radius.height*cos(j),
            tex_tl, tex_br, uv);

        j += step;
    }

    End();

    /*
     * Bottom right
     */
    Begin(GL_TRIANGLE_FAN);

    x = br.x - radius.width;
    y = tl.y + radius.height;
    j = RAD_90;

    vertex(tl, br, x, y, tex_tl, tex_br, uv);

    for (i = 0; i <= sides/4; i++) {

        vertex(tl, br,
            x + radius.width*sin(j),
            y + radius.height*cos(j),
            tex_tl, tex_br, uv);

        j += step;
    }

    End();

    /*
     * Bottom left
     */
    Begin(GL_TRIANGLE_FAN);

    x = tl.x + radius.width;
    y = tl.y + radius.height;
    j = RAD_180;

    vertex(tl, br, x, y, tex_tl, tex_br, uv);

    for (i = 0; i <= sides/4; i++) {

        vertex(tl, br,
            x + radius.width*sin(j),
            y + radius.height*cos(j),
            tex_tl, tex_br, uv);

        j += step;
    }

    End();

    /*
     * Bottom right
     */
    Begin(GL_TRIANGLE_FAN);

    x = tl.x + radius.width;
    y = br.y - radius.height;
    j = RAD_270;

    vertex(tl, br, x, y, tex_tl, tex_br, uv);

    for (i = 0; i <= sides/4; i++) {

        vertex(tl, br,
            x + radius.width*sin(j),
            y + radius.height*cos(j),
            tex_tl, tex_br, uv);

        j += step;
    }

    End();
}

/*
 * Draw a rounded square with 3D relief
 */
void gl_list_rounded_square_bevelled (fpoint tl, fpoint br,
                                      texp tex, fsize tex_tl, fsize tex_br,
                                      fsize uv, color hi, color med, color lo,
                                      fsize bevel, fsize radius, int32_t sides)
{
    radius.width = min(radius.width, (br.x - tl.x)/3);
    radius.height = min(radius.height, (br.y - tl.y)/3);

    //
    //
    //
    //        x-----A-----------------------B----x
    //        |\               2                /|
    //        | \                              / |
    //        |  x - A1------------------B1 - x  |
    //        |  |                            |  |
    //        |  |                            |  |
    //        |  |             1              |  |
    //        |  |                            |  |
    //        |  |                            |  |
    //        |  |                            |  |
    //        |  |                            |  |
    //        |  x - C1------------------D1 - x  |
    //        | /              3               \ |
    //        |/                                \|
    //        x-----C-----------------------D----x
    //
    //
    //
    glEnable(GL_TEXTURE_2D);

    {
    fpoint a = {tl.x + bevel.width + radius.width, tl.y};
    fpoint b = {br.x - bevel.width - radius.width, tl.y};
    fpoint c = {br.x - bevel.width - radius.width, br.y};
    fpoint d = {tl.x + bevel.width + radius.width, br.y};
    fpoint a1 = {tl.x + bevel.width + radius.width, tl.y + bevel.height};
    fpoint b1 = {br.x - bevel.width - radius.width, tl.y + bevel.height};
    fpoint c1 = {br.x - bevel.width - radius.width, br.y - bevel.height};
    fpoint d1 = {tl.x + bevel.width + radius.width, br.y - bevel.height};

    /*
     * Draw the central flat square (1);
     */
    glcolor(med);
    quaduv(tl, br, a1, b1, c1, d1, tex_tl, tex_br, uv);

    /*
     * Now draw the bevelled border (2);
     */
    glcolor(lo);
    quaduv(tl, br, c1, c, d, d1, tex_tl, tex_br, uv);
    quaduv(tl, br, a, a1, d1, d, tex_tl, tex_br, uv);

    /*
     * (3);
     */
    glcolor(hi);
    quaduv(tl, br, a, b, b1, a1, tex_tl, tex_br, uv);
    quaduv(tl, br, b, c, c1, b1, tex_tl, tex_br, uv);
    }

    //
    //
    //
    //        x----------------------------------x
    //        |\                                /|
    //        | \                              / |
    //        |  x----------------------------x  |
    //        |  |                            |  |
    //        A  A1                          B1  B
    //        |  |                            |  |
    //        | 3|1            1             2|4 |
    //        |  |                            |  |
    //        C  C1                          D1  D
    //        |  |                            |  |
    //        |  x----------------------------x  |
    //        | /                              \ |
    //        |/                                \|
    //        x----------------------------------x
    //
    //
    //
    fpoint a = {tl.x, tl.y + bevel.height + radius.height};
    fpoint b = {br.x, tl.y + bevel.height + radius.height};
    fpoint c = {br.x, br.y - bevel.height - radius.height};
    fpoint d = {tl.x, br.y - bevel.height - radius.height};

    fpoint a1 = {tl.x + bevel.width, tl.y + bevel.height + radius.height};
    fpoint b1 = {br.x - bevel.width, tl.y + bevel.height + radius.height};
    fpoint c1 = {br.x - bevel.width, br.y - bevel.height - radius.height};
    fpoint d1 = {tl.x + bevel.width, br.y - bevel.height - radius.height};

    fpoint a2 = {tl.x + bevel.width + radius.width, tl.y + bevel.height + radius.height};
    fpoint b2 = {br.x - bevel.width - radius.width, tl.y + bevel.height + radius.height};
    fpoint c2 = {br.x - bevel.width - radius.width, br.y - bevel.height - radius.height};
    fpoint d2 = {tl.x + bevel.width + radius.width, br.y - bevel.height - radius.height};

    /*
     * Draw the central flat squares (1);
     */
    glcolor(med);
    quaduv(tl, br, a1, a2, d2, d1, tex_tl, tex_br, uv);
    quaduv(tl, br, b2, b1, c1, c2, tex_tl, tex_br, uv);

    /*
     * Now draw the bevelled border
     */

    /*
     * (3);
     */
    glcolor(hi);
    quaduv(tl, br, a, a1, d1, d, tex_tl, tex_br, uv);

    /*
     * (4);
     */
    glcolor(lo);
    quaduv(tl, br, b, c, c1, b1, tex_tl, tex_br, uv);

    float step = RAD_360 / sides;

    /*
     * Now draw the rounded filled edges, large first, with highlights
     */
    radius.width += bevel.width;
    radius.height += bevel.height;

    /*
     * Top right
     */
    Begin(GL_TRIANGLE_STRIP);

    float x = br.x - radius.width;
    float y = br.y - radius.height;
    float j = 0.0f;

    glcolor(lo);
    vertex(tl, br, x, y, tex_tl, tex_br, uv);

    int32_t i;
    for (i = 0; i <= sides/4; i++) {

        vertex(tl, br,
            x + (radius.width-bevel.width)*sin(j),
            y + (radius.height-bevel.height)*cos(j),
            tex_tl, tex_br, uv);
        vertex(tl, br,
            x + radius.width*sin(j),
            y + radius.height*cos(j),
            tex_tl, tex_br, uv);

        j += step;

        if (i == sides/8) {
            glcolor(lo);
        }
    }

    End();

    /*
     * Bottom right
     */
    Begin(GL_TRIANGLE_STRIP);

    x = br.x - radius.width;
    y = tl.y + radius.height;
    j = RAD_90;

    glcolor(lo);
    vertex(tl, br, x, y, tex_tl, tex_br, uv);

    for (i = 0; i <= sides/4; i++) {

        vertex(tl, br,
            x + (radius.width-bevel.width)*sin(j),
            y + (radius.height-bevel.height)*cos(j),
            tex_tl, tex_br, uv);

        vertex(tl, br,
            x + radius.width*sin(j),
            y + radius.height*cos(j),
            tex_tl, tex_br, uv);

        j += step;

        if (i == sides/8) {
            glcolor(hi);
        }
    }

    End();

    /*
     * Bottom left
     */
    Begin(GL_TRIANGLE_STRIP);

    x = tl.x + radius.width;
    y = tl.y + radius.height;
    j = RAD_180;

    glcolor(hi);
    vertex(tl, br, x, y, tex_tl, tex_br, uv);

    for (i = 0; i <= sides/4; i++) {

        vertex(tl, br,
            x + (radius.width-bevel.width)*sin(j),
            y + (radius.height-bevel.height)*cos(j),
            tex_tl, tex_br, uv);

        vertex(tl, br,
            x + radius.width*sin(j),
            y + radius.height*cos(j),
            tex_tl, tex_br, uv);

        j += step;
    }

    End();

    /*
     * Top right
     */
    Begin(GL_TRIANGLE_STRIP);

    x = tl.x + radius.width;
    y = br.y - radius.height;
    j = RAD_270;

    vertex(tl, br, x, y, tex_tl, tex_br, uv);

    glcolor(hi);

    for (i = 0; i <= sides/4; i++) {

        vertex(tl, br,
            x + (radius.width-bevel.width)*sin(j),
            y + (radius.height-bevel.height)*cos(j),
            tex_tl, tex_br, uv);

        vertex(tl, br,
            x + radius.width*sin(j),
            y + radius.height*cos(j),
            tex_tl, tex_br, uv);

        if (i == sides/8) {
            glcolor(lo);
        }

        j += step;
    }

    End();

    glcolor(med);

    /*
     * Now draw the rounded filled edges, smaller, plain
     */
    radius.width -= bevel.width;
    radius.height -= bevel.height;

    /*
     * Top right
     */
    Begin(GL_TRIANGLE_FAN);

    x = br.x - radius.width - bevel.width;
    y = br.y - radius.height - bevel.height;
    j = 0;

    vertex(tl, br, x, y, tex_tl, tex_br, uv);

    for (i = 0; i <= sides/4; i++) {

        vertex(tl, br,
            x + radius.width*sin(j),
            y + radius.height*cos(j),
            tex_tl, tex_br, uv);

        j += step;
    }

    End();

    /*
     * Bottom right
     */
    Begin(GL_TRIANGLE_FAN);

    x = br.x - radius.width - bevel.width;
    y = tl.y + radius.height + bevel.height;
    j = RAD_90;

    vertex(tl, br, x, y, tex_tl, tex_br, uv);

    for (i = 0; i <= sides/4; i++) {

        vertex(tl, br,
            x + radius.width*sin(j),
            y + radius.height*cos(j),
            tex_tl, tex_br, uv);

        j += step;
    }

    End();

    /*
     * Bottom left
     */
    Begin(GL_TRIANGLE_FAN);

    x = tl.x + radius.width + bevel.width;
    y = tl.y + radius.height + bevel.height;
    j = RAD_180;

    vertex(tl, br, x, y, tex_tl, tex_br, uv);

    for (i = 0; i <= sides/4; i++) {

        vertex(tl, br,
            x + radius.width*sin(j),
            y + radius.height*cos(j),
            tex_tl, tex_br, uv);

        j += step;
    }

    End();

    /*
     * Bottom right
     */
    Begin(GL_TRIANGLE_FAN);

    x = tl.x + radius.width + bevel.width;
    y = br.y - radius.height - bevel.height;
    j = RAD_270;

    vertex(tl, br, x, y, tex_tl, tex_br, uv);

    for (i = 0; i <= sides/4; i++) {

        vertex(tl, br,
            x + radius.width*sin(j),
            y + radius.height*cos(j),
            tex_tl, tex_br, uv);

        j += step;
    }

    End();
}

/*
 * Draw a square outline 3D relief
 */
void
gl_list_square_outline_bevelled_plain (fpoint tl, fpoint br,
                                       texp tex, fsize tex_tl, fsize tex_br,
                                       fsize uv, color hi, color med, color lo,
                                       fsize bevel)
{
    //
    //
    //        x-----A-----------------------B----x
    //        |\               2                /|
    //        | \                              / |
    //        |  x - A1------------------B1 - x  |
    //        |  |                            |  |
    //        |  |                            |  |
    //        |  |             1              |  |
    //        |  |                            |  |
    //        |  |                            |  |
    //        |  |                            |  |
    //        |  |                            |  |
    //        |  x - C1------------------D1 - x  |
    //        | /              3               \ |
    //        |/                                \|
    //        x-----C-----------------------D----x
    //
    //

    fpoint a = {tl.x, tl.y};
    fpoint b = {br.x, tl.y};
    fpoint c = {br.x, br.y};
    fpoint d = {tl.x, br.y};
    fpoint a1 = {tl.x + bevel.width, tl.y + bevel.height};
    fpoint b1 = {br.x - bevel.width, tl.y + bevel.height};
    fpoint c1 = {br.x - bevel.width, br.y - bevel.height};
    fpoint d1 = {tl.x + bevel.width, br.y - bevel.height};

    glEnable(GL_TEXTURE_2D);

    /*
     * Draw the central flat square
     */

    /*
     * Now draw the bevelled border
     */
    glcolor(lo);
    quaduv(tl, br, c1, c, d, d1, tex_tl, tex_br, uv);
    glcolor(hi);
    quaduv(tl, br, a, a1, d1, d, tex_tl, tex_br, uv);

    glcolor(hi);
    quaduv(tl, br, a, b, b1, a1, tex_tl, tex_br, uv);
    glcolor(lo);
    quaduv(tl, br, b, c, c1, b1, tex_tl, tex_br, uv);
}
