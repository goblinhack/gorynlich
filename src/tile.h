/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file.
 */

boolean tile_init(void);
void tile_fini(void);
void tile_load(const char *file, uint32_t width, uint32_t height,
               uint32_t nargs, ...);
void tile_load_arr(const char *file, uint32_t width, uint32_t height,
                   uint32_t nargs, const char *arr[]);
tilep tile_find(const char *name);
tilep tile_from_surface(SDL_Surface *surface,
                      const char *optional_file,
                      const char *name);
void tile_blit(tilep tile, char *name, point at);
void tile_blit_at(tilep tile, char *name, point tl, point br);
void tile_blit_fat(tilep tile, char *name, fpoint tl, fpoint br);
void tile_blit_mask_fat(tilep tile, char *name, fpoint tl, fpoint br);
void blit_quad(point tl, point br);
int32_t tile_get_gl_binding(tilep);
int32_t tile_get_width(tilep);
int32_t tile_get_height(tilep);
const char *tile_name(tilep);
texp tile_get_tex(tilep);
tilep string2tile(const char **s);
void tile_get_coords(tilep, float *x1, float *y1, float *x2, float *y2);
