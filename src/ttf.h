/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file.
 */

#include "enum.h"

boolean ttf_init(void);
void ttf_fini(void);
fontp ttf_new(const char *name, int32_t pointSize, int32_t style);
void ttf_free(fontp);
void ttf_text_size(fontp, const char *text,
                   uint32_t *w, uint32_t *h,
                   enum_fmt *fmt,
                   double scaling,
                   double advance,
                   boolean fixed_width);
void ttf_putc(fontp, int32_t c, int32_t x, int32_t y, double scaling);
void ttf_puts(fontp, const char *text, int32_t x, int32_t y, double scaling,
              double advance,
              boolean fixed_width);
void ttf_puts_no_fmt(fontp, const char *text, int32_t x, int32_t y, double scaling,
              double advance,
              boolean fixed_width);
void ttf2tga(void);
fontp ttf_read_tga(char *name, int32_t pointsize);
fontp ttf_write_tga(char *name, int32_t pointsize);

#define TTF_CURSOR_CHAR 127
