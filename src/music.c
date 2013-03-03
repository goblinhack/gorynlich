/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <SDL_mixer.h>

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

boolean music_init_done;

boolean music_init (void)
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
            DIE("no file for music");
        } else {
            DIE("no file for music loading %s", name_alias);
        }
    }

    if (!all_music) {
        all_music = tree_alloc(TREE_KEY_STRING, "TREE ROOT: music");
    }

    musicp m = (typeof(m)) myzalloc(sizeof(*m), "TREE NODE: music");
    m->tree.key = dupstr(name_alias, "TREE KEY: music");

    if (!tree_insert(all_music, &m->tree.node)) {
        DIE("music insert name_alias [%s] failed", name_alias);
    }

    m->data = ramdisk_load(file, &m->len);
    if (!m->data) {
        DIE("cannot load music %s", file);
    }

    SDL_RWops *rw = SDL_RWFromMem(m->data, m->len);
    if (!rw) {
        DIE("cannot make RW music %s", file);
    }

    m->music = Mix_LoadMUS_RW(rw);
    if (!m->music) {
        DIE("cannot make music %s: %s %s", file, Mix_GetError(),
            SDL_GetError());
    }

    LOG("Load  %s", file);

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
        DIE("no name_alias given for music find");
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

    Mix_CloseAudio();

    if (Mix_OpenAudio(rate,
                      audio_format,
                      audio_channels,
                      audio_buffers) != 0) {

        ERR("unable to initialize SDL Mix audio: %s %s",
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
    music_play("data/music/music.wav", "game", 22050);
}

void music_play_intro (void)
{
    music_play("data/music/intro.wav", "intro", 16000);
}

void music_play_hiscore (void)
{
    music_play("data/music/hiscore.wav", "hiscore", 22050);
}

void music_halt (void)
{
}
