/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include "main.h"
#include "marshal.h"
#include "string.h"
#include "name.h"

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

uint8_t config_init (void)
{
    config_inited = true;

    return (true);
}

static uint8_t demarshal_config (demarshal_p ctx, struct config *p)
{
    uint8_t rc;

    rc = true;

    rc = rc && GET_OPT_NAMED_INT32(ctx, "width", p->video_pix_width);
    rc = rc && GET_OPT_NAMED_INT32(ctx, "height", p->video_pix_height);
    rc = rc && GET_OPT_NAMED_INT32(ctx, "sound_volume", p->sound_volume);
    rc = rc && GET_OPT_NAMED_INT32(ctx, "music_volume", p->music_volume);

    char *tmp = 0;
    GET_OPT_NAMED_STRING(ctx, "name", tmp);
    if (tmp) {
        strncpy(p->player_stats.name, tmp, sizeof(p->player_stats.name) - 1);
        myfree(tmp);
        tmp = 0;
    }

    tmp = 0;
    GET_OPT_NAMED_STRING(ctx, "class", tmp);
    if (tmp) {
        strncpy(p->player_stats.pclass, tmp, 
                sizeof(p->player_stats.pclass) - 1);
        myfree(tmp);
        tmp = 0;
    }

    tmp = 0;
    GET_OPT_NAMED_STRING(ctx, "server_name", tmp);
    if (tmp) {
        strncpy(p->server_name, tmp, sizeof(p->server_name) - 1);
        myfree(tmp);
    }

    rc = rc && GET_OPT_NAMED_UINT8(ctx, "server_max_players", 
                                   p->server_max_players);

    rc = rc && GET_OPT_NAMED_UINT16(ctx, "server_port", p->server_port);

    if (!p->server_max_players) {
        p->server_max_players = 4;
    }

    if (!p->server_port) {
        p->server_port = SERVER_DEFAULT_PORT;
    }

    return (rc);
}

static void marshal_config (marshal_p ctx, struct config *p)
{
    PUT_NAMED_INT32(ctx, "width", p->video_pix_width);
    PUT_NAMED_INT32(ctx, "height", p->video_pix_height);
    PUT_NAMED_INT32(ctx, "sound_volume", p->sound_volume);
    PUT_NAMED_INT32(ctx, "music_volume", p->music_volume);

    if (p->player_stats.name[0]) {
        PUT_NAMED_STRING(ctx, "name", p->player_stats.name);
    }

    if (p->player_stats.pclass[0]) {
        PUT_NAMED_STRING(ctx, "class", p->player_stats.pclass);
    }

    if (p->server_name[0]) {
        PUT_NAMED_STRING(ctx, "server_name", p->server_name);
    }

    PUT_NAMED_UINT8(ctx, "server_max_players", p->server_max_players);

    PUT_NAMED_UINT16(ctx, "server_port", p->server_port);
}

uint8_t config_save (void)
{
    char *file = dynprintf("%s", config_dir_and_file);
    marshal_p ctx;

    CON("Saving config to: %s", file);

    ctx = marshal(file);
    if (!ctx) {
        WARN("Failed to save: %s", file);
        myfree(file);
        return (false);
    }

    marshal_config(ctx, &global_config);

    if (marshal_fini(ctx) < 0) {
        WARN("Failed to finalize: %s", file);
        myfree(file);
        return (false);
    }

    myfree(file);

    return (true);
}

uint8_t config_load (void)
{
    char *file = dynprintf("%s", config_dir_and_file);
    demarshal_p ctx;

    /*
     * Default settings.
     */
    global_config.video_pix_width = 0;
    global_config.video_pix_height = 0;
    global_config.sound_volume = SOUND_MAX;
    global_config.music_volume = SOUND_MED;

    if (!global_config.player_stats.pclass[0]) {
        strncpy(global_config.player_stats.pclass, "warrior", 
                sizeof(global_config.player_stats.pclass) - 1);
    }

    if (!global_config.player_stats.name[0]) {
        strncpy(global_config.player_stats.name, 
                name_random(global_config.player_stats.pclass),
                sizeof(global_config.player_stats.name) - 1);
    }

    if (!global_config.server_name[0]) {
        strncpy(global_config.server_name, "unnamed-server", 
                sizeof(global_config.server_name) - 1);
    }

    global_config.server_max_players = 4;
    global_config.server_port = SERVER_DEFAULT_PORT;

    if (!(ctx = demarshal(file))) {
        myfree(file);
        return (true);
    }

    if (!demarshal_config(ctx, &global_config)) {
        MSG_BOX("Failed to parse: %s", file);
    }

    myfree(file);
    demarshal_fini(ctx);

    return (true);
}
