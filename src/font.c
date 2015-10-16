/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */

#include "main.h"
#include "ttf.h"
#include "font.h"
#include "string_util.h"

static int32_t font_inited;

fontp fixed_font;
fontp vsmall_font;
fontp small_font;
fontp med_font;
fontp large_font;
fontp vlarge_font;
fontp vvlarge_font;

void font_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (font_inited) {
        font_inited = false;

        if (fixed_font) {
            ttf_free(fixed_font);
            fixed_font = 0;
        }

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

        if (vvlarge_font) {
            ttf_free(vvlarge_font);
            vvlarge_font = 0;
        }
    }
}

uint8_t font_init (void)
{
    int32_t fixed_font_size = FIXED_FONT_SIZE;
    int32_t vsmall_font_size = VSMALL_FONT_SIZE;
    int32_t small_font_size = SMALL_FONT_SIZE;
    int32_t med_font_size = MED_FONT_SIZE;
    int32_t vlarge_font_size = VLARGE_FONT_SIZE;
    int32_t vvlarge_font_size = VVLARGE_FONT_SIZE;
    int32_t large_font_size = LARGE_FONT_SIZE;

    font_inited = true;

    double scale = (double)global_config.video_pix_width / 1000.0;

    fixed_font_size  = ((double)fixed_font_size  ) * scale;
    vsmall_font_size = ((double)vsmall_font_size ) * scale;
    small_font_size  = ((double)small_font_size  ) * scale;
    med_font_size    = ((double)med_font_size    ) * scale;
    large_font_size  = ((double)large_font_size  ) * scale;
    vlarge_font_size = ((double)vlarge_font_size ) * scale;
    vvlarge_font_size = ((double)vvlarge_font_size ) * scale;

    if (fixed_font_size > MAX_FONT_SIZE - 1) {
        fixed_font_size = MAX_FONT_SIZE - 1;
    }
    if (vsmall_font_size > MAX_FONT_SIZE - 1) {
        vsmall_font_size = MAX_FONT_SIZE - 1;
    }
    if (small_font_size > MAX_FONT_SIZE - 1) {
        small_font_size = MAX_FONT_SIZE - 1;
    }
    if (med_font_size > MAX_FONT_SIZE - 1) {
        med_font_size = MAX_FONT_SIZE - 1;
    }
    if (large_font_size > MAX_FONT_SIZE - 1) {
        large_font_size = MAX_FONT_SIZE - 1;
    }
    if (vlarge_font_size > MAX_FONT_SIZE - 1) {
        vlarge_font_size = MAX_FONT_SIZE - 1;
    }
    if (vvlarge_font_size > MAX_FONT_SIZE - 1) {
        vvlarge_font_size = MAX_FONT_SIZE - 1;
    }

#ifdef ENABLE_GENERATE_TTF
    /*
     * Generate bitmaps from TTF.
     */
    char *tmp;

    int32_t i;

    for (i = 2; i < MAX_FONT_SIZE; i+=2) {
        tmp = strprepend(mybasename(FIXED_FONT, __FUNCTION__), TTF_PATH);
        ttf_write_tga((char*) tmp, i);
        myfree(tmp);

        tmp = strprepend(mybasename(VSMALL_FONT, __FUNCTION__), TTF_PATH);
        ttf_write_tga((char*) tmp, i);
        myfree(tmp);

        tmp = strprepend(mybasename(SMALL_FONT, __FUNCTION__), TTF_PATH);
        ttf_write_tga((char*) tmp, i);
        myfree(tmp);

        tmp = strprepend(mybasename(MED_FONT, __FUNCTION__), TTF_PATH);
        ttf_write_tga((char*) tmp, i);
        myfree(tmp);

        tmp = strprepend(mybasename(LARGE_FONT, __FUNCTION__), TTF_PATH);
        ttf_write_tga((char*) tmp, i);
        myfree(tmp);

        tmp = strprepend(mybasename(VLARGE_FONT, __FUNCTION__), TTF_PATH);
        ttf_write_tga((char*) tmp, i);
        myfree(tmp);

        tmp = strprepend(mybasename(VVLARGE_FONT, __FUNCTION__), TTF_PATH);
        ttf_write_tga((char*) tmp, i);
        myfree(tmp);
    }
#endif

    fixed_font = ttf_read_tga((char*)FIXED_FONT, fixed_font_size & ~1);
    vsmall_font = ttf_read_tga((char*)VSMALL_FONT, vsmall_font_size & ~1);
    small_font  = ttf_read_tga((char*)SMALL_FONT, small_font_size & ~1);
    med_font    = ttf_read_tga((char*)MED_FONT, med_font_size & ~1);
    large_font  = ttf_read_tga((char*)LARGE_FONT, large_font_size & ~1);
    vlarge_font = ttf_read_tga((char*)VLARGE_FONT, vlarge_font_size & ~1);
    vvlarge_font = ttf_read_tga((char*)VVLARGE_FONT, vvlarge_font_size & ~1);

    return (true);
}
