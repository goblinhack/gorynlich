/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file.
 */

boolean tex_init(void);
void tex_fini(void);
texp tex_load(const char *file, const char *name);
texp tex_load_tiled(const char *file, const char *name, 
                    uint32_t x,
                    uint32_t y);
texp tex_find(const char *name);
texp tex_from_surface(SDL_Surface *surface,
                      const char *optional_file,
                      const char *name);
texp tex_from_tiled_surface(SDL_Surface *surface,
                            uint32_t x,
                            uint32_t y,
                            const char *optional_file,
                            const char *name);
void tex_blit(texp tex, point at);
void blit_quad(point tl, point br);
int32_t tex_get_gl_binding(texp);
uint32_t tex_get_width(texp);
uint32_t tex_get_height(texp);
SDL_Surface *tex_get_surface(texp);
texp string2tex(const char **s);
