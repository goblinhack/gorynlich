/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <string.h>

#include "main.h"
#include "ttf.h"
#include "font.h"

static int32_t font_inited;

void font_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (font_inited) {
        font_inited = false;

        if (small_font) {
            ttf_free(small_font);
            small_font = 0;
        }

        if (med_font) {
            ttf_free(med_font);
            med_font = 0;
        }

        if (large_font) {
            ttf_free(large_font);
            large_font = 0;
        }
    }
}

boolean font_init (void)
{
    font_inited = true;

    small_font  = ttf_read_tga((char*)SMALL_FONT, SMALL_FONT_SIZE);
    med_font    = ttf_read_tga((char*)MED_FONT, MED_FONT_SIZE);
    large_font  = ttf_read_tga((char*)LARGE_FONT, LARGE_FONT_SIZE);

    return (true);
}
