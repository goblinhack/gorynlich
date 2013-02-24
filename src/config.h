/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file.
 */

void config_fini(void);
boolean config_init(void);
boolean config_save(void);
boolean config_load(void);

struct config {
    int32_t video_pix_width;
    int32_t video_pix_height;
    int32_t video_gl_width;
    int32_t video_gl_height;
    float xscale;
    float yscale;
    int32_t sound_volume;
    int32_t music_volume;
};

extern struct config global_config;

#define SOUND_MIN 0
#define SOUND_MAX 3
