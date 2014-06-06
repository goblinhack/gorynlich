/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <stdlib.h>

#include "main.h"
#include "gl.h"
#include "color.h"

void gl_enter_2d_mode (void)
{
    if (HEADLESS) {
        return;
    }

    /*
     * Enable Texture Mapping
     */
    glEnable(GL_TEXTURE_2D);

    /*
     * Enable alpha blending for sprites
     */
    glAlphaFunc(GL_GREATER, 0.1f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /*
     * Setup our viewport
     */
    glViewport(0, 0, global_config.video_pix_width,
               global_config.video_pix_height);

    /*
     * Change to the projection matrix and set our viewing volume.
     */
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();

    /*
     * Reset the view
     */
    glLoadIdentity();

    /*
     * 2D projection
     */
    glOrthof(0, 
            global_config.video_gl_width, global_config.video_gl_height,
            0, -1200.0, 1200.0);

    /*
     * Make sure we're changing the model view and not the projection
     */
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    /*
     * Reset the view
     */
    glLoadIdentity();
}

void
gl_leave_2d_mode (void)
{
    if (HEADLESS) {
        return;
    }

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}

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

#define NUMBER_BYTES_PER_ARRAY_ELEM (NUMBER_BYTES_PER_VERTICE * NUMBER_COORDS_PER_VERTEX)

/*
 * Two arrays, xy and uv.
 */
static const uint32_t NUMBER_ARRAY_ELEM_ARRAYS = 2;

/*
 * gl_push
 */
static inline void
gl_push (float **p,
         float *p_end,
         uint8_t first,
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

    if (!first) {
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

static float *gl_array_buf;
static float *gl_array_buf_end;

/*
 * Where we are currently up to in writing to these buffers.
 */
static GLfloat *bufp;
static GLfloat *bufp_end;
static int buf_tex;

static void blit_init (void)
{
    buf_tex = 0;

    if (gl_array_buf) {
        bufp = gl_array_buf;
        return;
    }

    /*
     * Our array size requirements.
     */
    uint64_t gl_array_size_required;

    /*
     * If the screen size has changed or this is the first run, allocate our
     * buffer if our size requirements have changed.
     */
    gl_array_size_required = 1 * 1024 * 1024;

    gl_array_buf = myzalloc(gl_array_size_required, "GL xy buffer");
    gl_array_buf_end =
            (typeof(gl_array_buf_end))
                ((char *)gl_array_buf) + gl_array_size_required;

    bufp = gl_array_buf;
    bufp_end = gl_array_buf_end;
}

void blit_fini (void)
{
    if (gl_array_buf) {
        myfree(gl_array_buf);
        gl_array_buf = 0;
    }
}

void blit_flush (void)
{
    if (gl_array_buf == bufp) {
        return;
    }

    /*
     * Display all the tiles selected above in one blast.
     */
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    static uint32_t nvertices;

    nvertices = ((char*)bufp - (char*)gl_array_buf) /
                    NUMBER_BYTES_PER_VERTICE;

    glBindTexture(GL_TEXTURE_2D, buf_tex);

    glTexCoordPointer(
        NUMBER_DIMENSIONS_PER_COORD, // (u,v)
        GL_FLOAT,
        NUMBER_BYTES_PER_VERTICE,
        gl_array_buf);

    glVertexPointer(
        NUMBER_DIMENSIONS_PER_COORD, // (x,y)
        GL_FLOAT,
        NUMBER_BYTES_PER_VERTICE,
        ((char*)gl_array_buf) +
            sizeof(GLfloat) *        // skip (x,y)
            NUMBER_DIMENSIONS_PER_COORD);

    glColorPointer(
        NUMBER_COMPONENTS_PER_COLOR, // (r,g,b,a)
        GL_FLOAT,
        NUMBER_BYTES_PER_VERTICE,
        ((char*)gl_array_buf) +
            sizeof(GLfloat) *        // skip (x,y)
            NUMBER_DIMENSIONS_PER_COORD +
            sizeof(GLfloat) *        // skip (u,v)
            NUMBER_DIMENSIONS_PER_COORD);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, nvertices);

    glBindTexture(GL_TEXTURE_2D, 0);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    blit_init();
}

void blit (int tex,
           float texMinX,
           float texMinY,
           float texMaxX,
           float texMaxY,
           float left,
           float top,
           float right,
           float bottom)
{
#ifdef ENABLE_GL_BULK_DRAW_ARRAYS
    uint8_t first;

    if (!buf_tex) {
        blit_init();
        first = true;
    } else if (buf_tex != tex) {
        blit_flush();
        first = true;
    } else {
        first = false;
    }

    buf_tex = tex;

    color c = gl_color_current();

    float r = ((float)c.r) / 255.0;
    float g = ((float)c.g) / 255.0;
    float b = ((float)c.b) / 255.0;
    float a = ((float)c.a) / 255.0;

    float r1 = r;
    float g1 = g;
    float b1 = b;
    float a1 = a;

    float r2 = r;
    float g2 = g;
    float b2 = b;
    float a2 = a;

    float r3 = r;
    float g3 = g;
    float b3 = b;
    float a3 = a;

    float r4 = r;
    float g4 = g;
    float b4 = b;
    float a4 = a;

    gl_push(&bufp,
            bufp_end,
            first,
            texMinX,
            texMinY,
            texMaxX,
            texMaxY,
            left,
            top,
            right,
            bottom,
            r1, g1, b1, a1,
            r2, g2, b2, a2,
            r3, g3, b3, a3,
            r4, g4, b4, a4);
#else
    glBindTexture(GL_TEXTURE_2D, tex);

    GLfloat xy[4*2];
    GLfloat uv[4*2];
    GLfloat *xyp = xy;
    GLfloat *uvp = uv;

#define TexCoord2f(x, y) *uvp++ = x; *uvp++ = y;
#define Vertex2f(x, y) *xyp++ = x; *xyp++ = y;

    TexCoord2f(texMinX, texMinY);
    TexCoord2f(texMaxX, texMinY);
    TexCoord2f(texMinX, texMaxY);
    TexCoord2f(texMaxX, texMaxY);

    Vertex2f(left, top);
    Vertex2f(right, top);
    Vertex2f(left, bottom);
    Vertex2f(right, bottom);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(2, GL_FLOAT, 0, xy);
    glTexCoordPointer(2, GL_FLOAT, 0, uv);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
#endif
}

void gl_blitquad (float left, float top, float right, float bottom)
{
    GLfloat xy[4*2];
    GLfloat *xyp = xy;

    Vertex2f(left, top);
    Vertex2f(right, top);
    Vertex2f(left, bottom);
    Vertex2f(right, bottom);

    glEnableClientState(GL_VERTEX_ARRAY);

    glVertexPointer(2, GL_FLOAT, 0, xy);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisableClientState(GL_VERTEX_ARRAY);
}

void gl_blitsquare (float left, float top, float right, float bottom)
{
    GLfloat xy[4*2];
    GLfloat *xyp = xy;

    Vertex2f(left, top);
    Vertex2f(right, top);
    Vertex2f(right, bottom);
    Vertex2f(left, bottom);

    glEnableClientState(GL_VERTEX_ARRAY);

    glVertexPointer(2, GL_FLOAT, 0, xy);
    glDrawArrays(GL_LINE_LOOP, 0, 4);

    glDisableClientState(GL_VERTEX_ARRAY);
}

void gl_ortho_set (int32_t width, int32_t height)
{
#ifdef SCALE_GRAPHICS
    global_config.video_gl_width = DEFAULT_VIDEO_WIDTH;
    global_config.video_gl_height = DEFAULT_VIDEO_HEIGHT;
#else
    global_config.video_gl_width = width;
    global_config.video_gl_height = height;
#endif

    global_config.xscale = (float)global_config.video_gl_width / (float)width;
    global_config.yscale = (float)global_config.video_gl_height / (float)height;
}
