/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */


#include "main.h"
#include "marshal.h"
#include "config.h"
#include "string.h"

struct config global_config;

static const char *config_dir_and_file = "gorynlich-config.txt";
static int32_t config_inited;

void config_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (config_inited) {
        config_inited = false;
    }
}

boolean config_init (void)
{
    config_inited = true;

    return (true);
}

static boolean demarshal_config (demarshal_p ctx, struct config *p)
{
    boolean rc;

    rc = true;

    rc = rc && GET_OPT_NAMED_INT32(ctx, "width", p->video_pix_width);
    rc = rc && GET_OPT_NAMED_INT32(ctx, "height", p->video_pix_height);
    rc = rc && GET_OPT_NAMED_INT32(ctx, "sound_volume", p->sound_volume);
    rc = rc && GET_OPT_NAMED_INT32(ctx, "music_volume", p->music_volume);

    return (rc);
}

static void marshal_config (marshal_p ctx, struct config *p)
{
    PUT_NAMED_INT32(ctx, "width", p->video_pix_width);
    PUT_NAMED_INT32(ctx, "height", p->video_pix_height);
    PUT_NAMED_INT32(ctx, "sound_volume", p->sound_volume);
    PUT_NAMED_INT32(ctx, "music_volume", p->music_volume);
}

boolean config_save (void)
{
    char *file = dynprintf("%s", config_dir_and_file);
    marshal_p ctx;

    CON("Saving config to: %s", file);

    ctx = marshal(file);
    if (!ctx) {
        ERR("Failed to save: %s", file);
        myfree(file);
        return (false);
    }

    marshal_config(ctx, &global_config);

    if (marshal_fini(ctx) < 0) {
        ERR("Failed to finalize: %s", file);
        myfree(file);
        return (false);
    }

    myfree(file);

    return (true);
}

boolean config_load (void)
{
    char *file = dynprintf("%s", config_dir_and_file);
    demarshal_p ctx;

    /*
     * Default settings.
     */
    global_config.video_pix_width = 0;
    global_config.video_pix_height = 0;
    global_config.sound_volume = SOUND_MAX;
    global_config.music_volume = SOUND_MAX;

    if (!(ctx = demarshal(file))) {
        ERR("Failed to read config: %s", file);

        myfree(file);
        return (true);
    }

    if (!demarshal_config(ctx, &global_config)) {
        ERR("Failed to parse: %s", file);
    }

    myfree(file);
    demarshal_fini(ctx);

    return (true);
}
