/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file.
 */

/*
 * http://en.wikipedia.org/wiki/Display_resolution
 *
 * screen sizes @ 2012
 *
 *                                  Steam   Web
 * FHD      16:9    1920    1080    25.04   5.09
 * HD      ~16:9    1366    768     17.19   19.14
 * SXGA     5:4     1280    1024    10.66   7.49
 * WSXGA+   16:10   1680    1050    10.26   3.66
 * WXGA+    16:10   1440    900     7.60    6.61
 * HD+      16:9    1600    900     6.82    3.82
 * XGA      4:3     1024    768     5.53    18.69
 * WXGA     16:10   1280    800     4.25    12.97
 * WUXGA    16:10   1920    1200    3.65    1.11
 * HD      ~16:9    1360    768     2.36    2.28
 * WXGA     16:9    1280    720     1.51    1.54
 * SXGA     4:3     1280    960     0.72    0.72
 * WQHD     16:9    2560    1440    0.72    0.36
 * XGA+     4:3     1152    864     0.87    1.55
 * UXGA     4:3     1600    1200    0.53    n/a
 * WSVGA    17:10   1024    600     0.31    2.25
 * WQXGA    16:10   2560    1600    0.19    n/a
 * SXGA+    4:3     1400    1050    0.18    n/a
 * SVGA     4:3     800     600     0.17    1.03
 * QWXGA    16:9    2048    1152    0.13    n/a
 * VGA      4:3     640     480     0.02    n/a
 * WXGA     5:3     1280    768     n/a     1.54
 *          3:4     768     1024    n/a     1.93
 *          16:9    1093    614     n/a     0.63
 */
#define IPHONE_VIDEO_WIDTH               (320)
#define IPHONE_VIDEO_HEIGHT              (480)
#define MEDIUM_VIDEO_WIDTH               (576)
#define MEDIUM_VIDEO_HEIGHT              (768)
#define IPAD_VIDEO_WIDTH                 (768)
#define IPAD_VIDEO_HEIGHT                (1024)

/*
 * gl.c
 */
void gl_enter_2d_mode(void);
void gl_leave_2d_mode(void);

void blit(float tx0, float ty0, float tx1, float ty1,
          float tlx, float tly, float brx, float bry);
void gl_blitquad(float tlx, float tly, float brx, float bry);
void gl_blitsquare(float tlx, float tly, float brx, float bry);

/*
 * glshapes.c
 */
void gl_list_square(fpoint tl, fpoint br,
                    texp, fsize tex_tl, fsize tex_br, fsize uv,
                    color hi, color med, color lo);

void gl_list_square_bevelled(fpoint tl, fpoint br,
                             texp, fsize tex_tl, fsize tex_br, fsize uv,
                             color hi, color med, color lo,
                             fsize bevel);

void gl_list_square_bevelled_plain(fpoint tl, fpoint br,
                                   texp, fsize tex_tl, fsize tex_br, fsize uv,
                                   color hi, color med, color lo,
                                   fsize bevel);

void gl_list_rounded_square(fpoint tl, fpoint br,
                            texp, fsize tex_tl, fsize tex_br, fsize uv,
                            color hi, color med, color lo,
                            fsize radius, int32_t sides);

void gl_list_rounded_square_bevelled(fpoint tl, fpoint br,
                                     texp, fsize tex_tl, fsize tex_br, fsize uv,
                                     color hi, color med, color lo,
                                     fsize bevel, fsize radius, int32_t sides);

void gl_list_square_outline_bevelled_plain(fpoint tl, fpoint br,
                                           texp tex,
                                           fsize tex_tl, fsize tex_br,
                                           fsize uv,
                                           color hi, color med, color lo,
                                           fsize bevel);

void gl_ortho_set(int32_t width, int32_t height);

/*
 * gl_push_texcoord
 *
 * Push elements onto the array buffer.
 */
static inline void 
gl_push_texcoord (float **p, float x, float y)
{
    *(*p)++ = x;
    *(*p)++ = y;
}

/*
 * gl_push_vertex
 *
 * Push elements onto the array buffer.
 */
static inline void 
gl_push_vertex (float **p, float x, float y)
{
    *(*p)++ = x;
    *(*p)++ = y;
}

/*
 * gl_push_rgba
 *
 * Push elements onto the array buffer.
 */
static inline void 
gl_push_rgba (float **p, float r, float g, float b, float a)
{
    *(*p)++ = r;
    *(*p)++ = g;
    *(*p)++ = b;
    *(*p)++ = a;
}
