/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include "glapi.h"

static void gl_init_fbo(void);

void gl_enter_2d_mode (void)
{
    if (HEADLESS) {
        return;
    }

#ifdef _WIN32
    if (glewInit() != GLEW_OK) {
	SDL_MSG_BOX("GLEW init fail");
	return;
    }
#endif

    /*
     * Enable Texture Mapping
     */
    glEnable(GL_TEXTURE_2D);

    /*
     * Enable alpha blending for sprites
     */
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
    glOrtho(0, 
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

    gl_init_fbo();

    glLineWidth(2.0);
    glEnable(GL_LINE_SMOOTH);
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

static void gl_init_fbo_ (
    GLuint *render_buf_id,
    GLuint *fbo_id,
    GLuint *fbo_tex_id)
{
    GLuint tex_width = global_config.video_pix_width;
    GLuint tex_height = global_config.video_pix_height;

    glGenTextures(1, fbo_tex_id);
    glBindTexture(GL_TEXTURE_2D, *fbo_tex_id);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 
                 tex_width, tex_height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    /*
     * Create a render buffer object.
     */
    glGenRenderbuffersEXT(1, render_buf_id);
    glBindRenderbuffer(GL_RENDERBUFFER, *render_buf_id);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT,
                          tex_width, tex_height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    /*
     * Create a frame buffer object.
     */
    glGenFramebuffers(1, fbo_id);
    glBindFramebuffer(GL_FRAMEBUFFER, *fbo_id);

    /*
     * Attach the texture to FBO color attachment point
     */
    glFramebufferTexture2D(GL_FRAMEBUFFER,        // 1. fbo target: GL_FRAMEBUFFER 
                           GL_COLOR_ATTACHMENT0,  // 2. attachment point
                           GL_TEXTURE_2D,         // 3. tex target: GL_TEXTURE_2D
                           *fbo_tex_id,           // 4. tex ID
                           0);                    // 5. mipmap level: 0(base)

    /*
     * Attach the renderbuffer to depth attachment point
     */
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,      // 1. fbo target: GL_FRAMEBUFFER
                              GL_DEPTH_ATTACHMENT, // 2. attachment point
                              GL_RENDERBUFFER,     // 3. rbo target: GL_RENDERBUFFER
                              *render_buf_id);     // 4. rbo ID

    /*
     * Check FBO status
     */
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        ERR("Failed to create framebuffer");
    }

    // switch back to window-system-provided framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint render_buf_id1;
GLuint fbo_id1;
GLuint fbo_tex_id1;

static void gl_init_fbo (void)
{
    gl_init_fbo_(&render_buf_id1, &fbo_id1, &fbo_tex_id1);
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

    gl_array_buf = (typeof(gl_array_buf))
                    myzalloc(gl_array_size_required, "GL xy buffer");

    /*
     * Make the end a bit smaller so we have plenty of headroom.
     */
    gl_array_buf_end =
            (typeof(gl_array_buf_end))
                ((char *)gl_array_buf) + 
                ((gl_array_size_required * 2) / 3);

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

void blit_flush_colored_triangles (void)
{
    if (gl_array_buf == bufp) {
        return;
    }

    /*
     * Display all the tiles selected above in one blast.
     */
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    static uint32_t nvertices;

    static const GLsizei stride = 
                        sizeof(GLfloat) *
                        NUMBER_DIMENSIONS_PER_COORD +
                        sizeof(GLfloat) *
                        NUMBER_COMPONENTS_PER_COLOR;

    nvertices = ((char*)bufp - (char*)gl_array_buf) / stride;

    glVertexPointer(
        NUMBER_DIMENSIONS_PER_COORD, // (x,y)
        GL_FLOAT,
        stride,
        gl_array_buf);

    glColorPointer(
        NUMBER_COMPONENTS_PER_COLOR, // (r,g,b,a)
        GL_FLOAT,
        stride,
        ((char*)gl_array_buf) +
            sizeof(GLfloat) *        // skip (x,y)
            NUMBER_DIMENSIONS_PER_COORD);

    glDrawArrays(GL_TRIANGLES, 0, nvertices);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    blit_init();
}

void blit_flush_triangle_fan (void)
{
    if (gl_array_buf == bufp) {
        return;
    }

    /*
     * Display all the tiles selected above in one blast.
     */
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    static uint32_t nvertices;

    static const GLsizei stride = 
                        sizeof(GLfloat) *
                        NUMBER_DIMENSIONS_PER_COORD +
                        sizeof(GLfloat) *
                        NUMBER_COMPONENTS_PER_COLOR;

    nvertices = ((char*)bufp - (char*)gl_array_buf) / stride;

    glVertexPointer(
        NUMBER_DIMENSIONS_PER_COORD, // (x,y)
        GL_FLOAT,
        stride,
        gl_array_buf);

    glColorPointer(
        NUMBER_COMPONENTS_PER_COLOR, // (r,g,b,a)
        GL_FLOAT,
        stride,
        ((char*)gl_array_buf) +
            sizeof(GLfloat) *        // skip (x,y)
            NUMBER_DIMENSIONS_PER_COORD);

    glDrawArrays(GL_TRIANGLE_FAN, 0, nvertices);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    blit_init();
}

void blit_flush_triangle_strip (void)
{
    if (gl_array_buf == bufp) {
        return;
    }

    /*
     * Display all the tiles selected above in one blast.
     */
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    static uint32_t nvertices;

    static const GLsizei stride = 
                        sizeof(GLfloat) *
                        NUMBER_DIMENSIONS_PER_COORD +
                        sizeof(GLfloat) *
                        NUMBER_COMPONENTS_PER_COLOR;

    nvertices = ((char*)bufp - (char*)gl_array_buf) / stride;

    glVertexPointer(
        NUMBER_DIMENSIONS_PER_COORD, // (x,y)
        GL_FLOAT,
        stride,
        gl_array_buf);

    glColorPointer(
        NUMBER_COMPONENTS_PER_COLOR, // (r,g,b,a)
        GL_FLOAT,
        stride,
        ((char*)gl_array_buf) +
            sizeof(GLfloat) *        // skip (x,y)
            NUMBER_DIMENSIONS_PER_COORD);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, nvertices);

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
        0, // stride
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
