/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info.
 */

#pragma once

#include "enum.h"

uint8_t ttf_init(void);
void ttf_fini(void);
fontp ttf_new(const char *name, int32_t pointSize, int32_t style);
void ttf_free(fontp);
void ttf_text_size(fontp *, const char *text,
                   double *w, 
                   double *h,
                   enum_fmt *fmt,
                   double scaling,
                   double advance,
                   uint8_t fixed_width);
void ttf_putc(fontp, int32_t c, double x, double y, double scaling);
void ttf_puts(fontp, const char *text, double x, double y, double scaling,
              double advance,
              uint8_t fixed_width);
void ttf_puts_no_fmt(fontp, const char *text, double x, double y, double scaling,
              double advance,
              uint8_t fixed_width);
void ttf2tga(void);
fontp ttf_read_tga(char *name, int32_t pointsize);
fontp ttf_write_tga(char *name, int32_t pointsize);

#define TTF_CURSOR_CHAR 127
