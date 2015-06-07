/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <SDL_mixer.h>
#include <stdlib.h>

#include "main.h"
#include "music.h"
#include "ramdisk.h"
#include "tree.h"
#include "config.h"

typedef struct music_ {
    tree_key_string tree;
    Mix_Music *music;
    unsigned char *data;
    int32_t len;
} music;

tree_root *all_music;

uint8_t music_init_done;

uint8_t music_init (void)
{
    return (true);
}

static void music_destroy (music *m)
{
    Mix_FreeMusic(m->music);
    myfree(m->data);
}

void music_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (music_init_done) {
        music_init_done = false;

        if (all_music) {
            tree_destroy(&all_music, (tree_destroy_func)music_destroy);
        }
    }

    Mix_CloseAudio();
}

musicp music_load (const char *file, const char *name_alias)
{
    if (name_alias) {
        musicp m = music_find(name_alias);
        if (m) {
            return (m);
        }
    }

    if (!file) {
        if (!name_alias) {
            ERR("no file for music");
        } else {
            ERR("no file for music loading %s", name_alias);
        }
    }

    if (!all_music) {
        all_music = tree_alloc(TREE_KEY_STRING, "TREE ROOT: music");
    }

    musicp m = (typeof(m)) myzalloc(sizeof(*m), "TREE NODE: music");
    m->tree.key = dupstr(name_alias, "TREE KEY: music");

    if (!tree_insert(all_music, &m->tree.node)) {
        ERR("music insert name_alias [%s] failed", name_alias);
    }

    m->data = ramdisk_load(file, &m->len);
    if (!m->data) {
        ERR("cannot load music %s", file);
    }

    SDL_RWops *rw;

    rw = SDL_RWFromMem(m->data, m->len);
    if (!rw) {
        ERR("cannot make RW music %s", file);
    }

#if SDL_MAJOR_VERSION == 1 && SDL_MINOR_VERSION == 2 /* { */
    m->music = Mix_LoadMUS_RW(rw);
#else
    m->music = Mix_LoadMUS_RW(rw, false);
#endif /* } */
    if (!m->music) {
        MSG_BOX("Mix_LoadMUS_RW fail %s: %s %s", file, Mix_GetError(),
            SDL_GetError());
    }

    DBG("Load %s", file);

    return (m);
}

/*
 * Find an existing pice of music.
 */
musicp music_find (const char *name_alias)
{
    music target;
    music *result;

    if (!name_alias) {
        ERR("no name_alias given for music find");
    }

    memset(&target, 0, sizeof(target));
    target.tree.key = (char*) name_alias;

    result = (typeof(result)) tree_find(all_music, &target.tree.node);
    if (!result) {
        return (0);
    }

    return (result);
}

void music_update_volume (void)
{
    if (HEADLESS) {
        return;
    }

    Mix_VolumeMusic((int)
                    (float) global_config.music_volume *
                    ((float) MIX_MAX_VOLUME / (float) SOUND_MAX));
}

void music_play (const char *file,
                 const char *alias,
                 uint32_t rate)
{
    int audio_format = MIX_DEFAULT_FORMAT;
    int audio_channels = 2;
    int audio_buffers = 1024;

    if (HEADLESS) {
        return;
    }

    Mix_CloseAudio();

    if (Mix_OpenAudio(rate,
                      audio_format,
                      audio_channels,
                      audio_buffers) != 0) {

        MSG_BOX("Mix_OpenAudio fail: %s %s",
		Mix_GetError(), SDL_GetError());
    }

    music_init_done = true;

    musicp music = music_load(file, alias);

    music_update_volume();

    if (Mix_PlayMusic(music->music, 2) == -1) {
        WARN("cannot play %s: %s", music->tree.key, Mix_GetError());
    }
}

void music_play_game (void)
{
    static const char *music[] = {
        "data/music/charlotte.ogg",
        "data/music/DST_Alters.ogg",
        "data/music/DST_Arch_Delerium.ogg",
        "data/music/DST_Legends.ogg",
        "data/music/DST_LeDanse.ogg",
        "data/music/DST_ExInfernus.ogg",
        "data/music/DST_DoomCity.ogg",
        "data/music/DST_Arches.ogg",
        "data/music/DST_BattleLands.ogg",
        "data/music/DST_ClockTower.ogg",
        "data/music/DST_BoneTower.ogg",
        "data/music/DST_BlackRiver.ogg",
        "data/music/DST_DarkestKnight.ogg",
        "data/music/charlotte_slow.ogg",
    };

    int x = myrand() % ARRAY_SIZE(music);

    music_play(music[x], music[x], 22050);
}

void music_play_intro (void)
{
    music_play("data/music/charlotte8bit.ogg", "intro", 22050 );
}

void music_play_dead (void)
{
    music_play("data/music/DST_AncientCurse_II.ogg", "dead", 22050 / 2);
}

void music_play_game_over (void)
{
    music_play("data/music/DST_SuperWin.ogg", "game over", 22050 / 2);
}

void music_halt (void)
{
}
