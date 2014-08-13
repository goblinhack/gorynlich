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
static float *gl_array_buf;
static float *gl_array_buf_end;

/*
 * Where we are currently up to in writing to these buffers.
 */
GLfloat *bufp;
GLfloat *bufp_end;
int buf_tex;

void blit_init (void)
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
    gl_array_size_required = 16 * 1024 * 1024;

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

void blit_flush_triangles (void)
{
    if (gl_array_buf == bufp) {
        return;
    }

    /*
     * Display all the tiles selected above in one blast.
     */
    glEnableClientState(GL_VERTEX_ARRAY);

    static uint32_t number_bytes_per_vertice =
                    sizeof(GLfloat) * NUMBER_DIMENSIONS_PER_COORD;

    uint32_t nvertices = ((char*)bufp - (char*)gl_array_buf) /
                    number_bytes_per_vertice;

    glBindTexture(GL_TEXTURE_2D, 0);

    glVertexPointer(
        NUMBER_DIMENSIONS_PER_COORD, // (x,y)
        GL_FLOAT,
        0,
        ((char*)gl_array_buf));

    glDrawArrays(GL_TRIANGLES, 0, nvertices);

    glDisableClientState(GL_VERTEX_ARRAY);

    blit_init();
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

void gl_blitline (float left, float top, float right, float bottom)
{
    GLfloat xy[2*2];
    GLfloat *xyp = xy;

    Vertex2f(left, top);
    Vertex2f(right, bottom);

    glEnableClientState(GL_VERTEX_ARRAY);

    glVertexPointer(2, GL_FLOAT, 0, xy);
    glDrawArrays(GL_LINES, 0, 2);

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
