/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file.
 */

boolean tex_init(void);
void tex_fini(void);
texp tex_load(const char *file, const char *name, boolean mask_needed);
texp tex_find(const char *name);
texp tex_from_surface(SDL_Surface *surface,
                      SDL_Surface *mask,
                      const char *optional_file,
                      const char *name);
void tex_blit(texp tex, point at);
void mask_blit(texp tex, point at);
void blit_quad(point tl, point br);
int32_t tex_get_gl_binding(texp);
int32_t tex_get_gl_mask_binding(texp);
uint32_t tex_get_width(texp);
uint32_t tex_get_height(texp);
SDL_Surface *tex_get_surface(texp);
SDL_Surface *tex_get_mask(texp);
texp string2tex(const char **s);
