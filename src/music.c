/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */

#include <SDL_mixer.h>
#include <SDL.h>
#include <stdlib.h>

#include "main.h"
#include "music.h"
#include "ramdisk.h"
#include "tree.h"
#include "config.h"
#include "sound.h"

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
        SDL_ClearError();
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
    int audio_buffers = 4096;

    if (!music_init_done) {
        if (Mix_OpenAudio(rate,
                          audio_format,
                          audio_channels,
                          audio_buffers) != 0) {

            MSG_BOX("Mix_OpenAudio fail: %s %s",
                    Mix_GetError(), SDL_GetError());
            SDL_ClearError();
        }

        music_init_done = true;
    }

    musicp music = music_load(file, alias);

    music_update_volume();

    static int sound_loaded;
    if (!sound_loaded) {
        sound_loaded = true;
        sound_load_all();
    }

    if (HEADLESS) {
        return;
    }

    if (Mix_FadeInMusicPos(music->music, -1, 2000, 0) == -1) {
//    if (Mix_PlayMusic(music->music, -1) == -1) {
        WARN("cannot play music %s: %s", music->tree.key, Mix_GetError());
    }
}

void music_play_game (level_pos_t level_pos)
{
    static const char *music[] = {
        "data/music/charlotte.ogg",
        "data/music/charlotte_slow.ogg",
        "data/music/DST_Alters.ogg",
        "data/music/DST_Arch_Delerium.ogg",
        "data/music/DST_Arches.ogg",
        "data/music/DST_Aronara.ogg",
        "data/music/DST_BattleLands.ogg",
        "data/music/DST_Beneathere.ogg",
        "data/music/DST_BlackRiver.ogg",
        "data/music/DST_BloodAndIron.ogg",
        "data/music/DST_BoneTower.ogg",
        "data/music/DST_ClockTower.ogg",
        "data/music/DST_DarkDance.ogg",
        "data/music/DST_Darkening.ogg",
        "data/music/DST_DarkestKnight.ogg",
        "data/music/DST_ExInfernus.ogg",
        "data/music/DST_GhostHouse.ogg",
        "data/music/DST_H2O.ogg",
        "data/music/DST_LeDanse.ogg",
        "data/music/DST_Legends.ogg",
        "data/music/DST_MechaSelect.ogg",
        "data/music/DST_OldCavern.ogg",
        "data/music/DST_Ornament.ogg",
        "data/music/DST_OverlandTheme.ogg",
        "data/music/DST_ReflectingPool.ogg",
        "data/music/DST_Sarcophage.ogg",
        "data/music/DST_SnowMode.ogg",
        "data/music/DST_StarryBook.ogg",
        "data/music/DST_SuperWin.ogg",
        "data/music/DST_TwoHeaded.ogg",
        "data/music/DST_DoomCity.ogg",
    };

    /*
     * So we get the same music for each player on the same level.
     */
    int r = (level_pos.x + (level_pos.y * 10)) * 100;
    int x = r % ARRAY_SIZE(music);

    music_play(music[x], music[x], 22050);
}

void music_play_death (void)
{
    music_play("data/music/DST_TheHauntedChapel.ogg", "death", 22050 );
}

void music_play_demo (void)
{
    music_play("data/music/DST_BattleLands.ogg", "battle", 22050 );
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
    music_play("data/music/DST_BattleLands.ogg", "game over", 22050 );
}

void music_halt (void)
{
    Mix_FadeOutMusic(1500);
}
