/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "glapi.h"

#include "main.h"
#include "config.h"
#include "gl.h"

void gl_enter_2d_mode (void)
{
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
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}

void blit (float texMinX,
           float texMinY,
           float texMaxX,
           float texMaxY,
           float left,
           float top,
           float right,
           float bottom)
{
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
    global_config.video_gl_width = DEFAULT_VIDEO_WIDTH;
    global_config.video_gl_height = DEFAULT_VIDEO_HEIGHT;

    global_config.xscale = (float)global_config.video_gl_width / (float)width;
    global_config.yscale = (float)global_config.video_gl_height / (float)height;
}
