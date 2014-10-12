/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include "main.h"
#include "ttf.h"
#include "font.h"
#include "string.h"

static int32_t font_inited;

fontp vsmall_font;
fontp small_font;
fontp med_font;
fontp large_font;
fontp vlarge_font;

void font_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (font_inited) {
        font_inited = false;

        if (vsmall_font) {
            ttf_free(vsmall_font);
            vsmall_font = 0;
        }

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

        if (vlarge_font) {
            ttf_free(vlarge_font);
            vlarge_font = 0;
        }
    }
}

uint8_t font_init (void)
{
    int32_t vsmall_font_size = VSMALL_FONT_SIZE;
    int32_t small_font_size = SMALL_FONT_SIZE;
    int32_t med_font_size = MED_FONT_SIZE;
    int32_t vlarge_font_size = VLARGE_FONT_SIZE;
    int32_t large_font_size = LARGE_FONT_SIZE;
    int32_t delta;

    font_inited = true;

    delta = 0;
    if (global_config.video_pix_width <= 640) {
        delta = -7;
    } else if (global_config.video_pix_width <= 800) {
        delta = -5;
    } else if (global_config.video_pix_width < 1400) {
    } else {
        delta = +2;
    }

    vsmall_font_size += delta;
    small_font_size  += delta;
    med_font_size    += delta;
    vlarge_font_size += delta;
    large_font_size  += delta;

#ifdef ENABLE_GENERATE_TTF
    /*
     * Generate bitmaps from TTF.
     */
    char *tmp;
    tmp = strprepend(mybasename(VSMALL_FONT, __FUNCTION__), TTF_PATH);
    vsmall_font  = ttf_write_tga((char*) tmp, VSMALL_FONT_SIZE);
    myfree(tmp);

    tmp = strprepend(mybasename(SMALL_FONT, __FUNCTION__), TTF_PATH);
    small_font  = ttf_write_tga((char*) tmp, SMALL_FONT_SIZE);
    myfree(tmp);

    tmp = strprepend(mybasename(MED_FONT, __FUNCTION__), TTF_PATH);
    med_font    = ttf_write_tga((char*) tmp, MED_FONT_SIZE);
    myfree(tmp);

    tmp = strprepend(mybasename(LARGE_FONT, __FUNCTION__), TTF_PATH);
    large_font  = ttf_write_tga((char*) tmp, LARGE_FONT_SIZE);
    myfree(tmp);

    tmp = strprepend(mybasename(VLARGE_FONT, __FUNCTION__), TTF_PATH);
    vlarge_font  = ttf_write_tga((char*) tmp, VLARGE_FONT_SIZE);
    myfree(tmp);
#endif

    vsmall_font = ttf_read_tga((char*)VSMALL_FONT, VSMALL_FONT_SIZE);
    small_font  = ttf_read_tga((char*)SMALL_FONT, SMALL_FONT_SIZE);
    med_font    = ttf_read_tga((char*)MED_FONT, MED_FONT_SIZE);
    large_font  = ttf_read_tga((char*)LARGE_FONT, LARGE_FONT_SIZE);
    vlarge_font  = ttf_read_tga((char*)VLARGE_FONT, VLARGE_FONT_SIZE);

    return (true);
}
