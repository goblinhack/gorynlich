/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <SDL_mixer.h>
#include <stdlib.h>

#include "main.h"
#include "sound.h"
#include "ramdisk.h"
#include "tree.h"
#include "config.h"
#include "music.h"

typedef struct sound_ {
    tree_key_string tree;
    Mix_Chunk *sound;
    unsigned char *data;
    int32_t len;
} sound;

tree_root *all_sound;

static uint8_t sound_init_done;

uint8_t sound_init (void)
{
    sound_init_done = true;

    return (true);
}

static void sound_destroy (sound *m)
{
    Mix_FreeChunk(m->sound);
    myfree(m->data);
}

void sound_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (sound_init_done) {
        sound_init_done = false;

        if (all_sound) {
            tree_destroy(&all_sound, (tree_destroy_func)sound_destroy);
        }
    }
}

soundp sound_load (const char *filename, const char *name_alias)
{
    if (!music_init_done) {
        DIE("need music init")
        return (0);
    }

    if (name_alias) {
        soundp m = sound_find(name_alias);
        if (m) {
            return (m);
        }
    }

    if (!filename) {
        if (!name_alias) {
            ERR("no filename for sound");
            return (0);
        } else {
            ERR("no filename for loading sound %s", name_alias);
            return (0);
        }
    }

    if (!all_sound) {
        all_sound = tree_alloc(TREE_KEY_STRING, "TREE ROOT: sound");
    }

    soundp m = (typeof(m)) myzalloc(sizeof(*m), "TREE NODE: sound");
    m->tree.key = dupstr(name_alias, "TREE KEY: sound");

    if (!tree_insert(all_sound, &m->tree.node)) {
        DIE("sound insert name_alias [%s] failed", name_alias);
    }

    m->data = ramdisk_load(filename, &m->len);
    if (!m->data) {
        ERR("cannot load sound %s from ramdisk", filename);
        return (0);
    }

    SDL_RWops *rw = SDL_RWFromMem(m->data, m->len);
    if (!rw) {
        ERR("cannot make RW sound %s", filename);
        return (0);
    }

    m->sound = Mix_LoadWAV_RW(rw, 0 /* freesrc */);
    if (!m->sound) {
        ERR("cannot make sound %s: %s %s", filename, Mix_GetError(),
            SDL_GetError());
        return (0);
    }

    DBG("Load %s", filename);

    return (m);
}

/*
 * Find an existing pice of sound.
 */
soundp sound_find (const char *name_alias)
{
    sound target;
    sound *result;

    if (!name_alias) {
        ERR("no name_alias given for sound find");
    }

    memset(&target, 0, sizeof(target));
    target.tree.key = (char*) name_alias;

    result = (typeof(result)) tree_find(all_sound, &target.tree.node);
    if (!result) {
        return (0);
    }

    return (result);
}

void sound_play (const char *name_alias)
{
    if (HEADLESS) {
        return;
    }

    if (!music_init_done) {
        return;
    }

    soundp sound = sound_load(0, name_alias);
    if (!sound) {
        LOG("cannot load sound %s: %s", name_alias, Mix_GetError());

        return;
    }

    if (Mix_PlayChannel(-1, sound->sound, 0) == -1) {
        LOG("cannot play %s: %s", sound->tree.key, Mix_GetError());

        return;
    }

    Mix_VolumeChunk(sound->sound,
                    (float) global_config.sound_volume *
                    ((float) MIX_MAX_VOLUME / (float) SOUND_MAX));
}

void sound_play_n (const char *name_alias, int32_t n)
{
    if (HEADLESS) {
        return;
    }

    if (!music_init_done) {
        return;
    }

    soundp sound = sound_load(0, name_alias);

    if (Mix_PlayChannel(-1, sound->sound, n) == -1) {
        LOG("cannot play %s: %s", sound->tree.key, Mix_GetError());

        return;
    }

    Mix_VolumeChunk(sound->sound,
                    (float) global_config.sound_volume *
                    ((float) MIX_MAX_VOLUME / (float) SOUND_MAX));
}

void sound_play_dead (void)
{
    sound_play("dead");
}

void sound_play_click (void)
{
    sound_play("click");
}

void sound_play_chomp (void)
{
    sound_play("chomp");
}

void sound_play_chomp2 (void)
{
    sound_play("chomp2");
}

void sound_play_powerup (void)
{
    sound_play("powerup");
}

void sound_play_spam (void)
{
    sound_play("spam");
}

void sound_play_letter (void)
{
    sound_play("letter");
}

void sound_play_rocket (void)
{
    sound_play_n("rocket", 1);
}

void sound_play_level_end (void)
{
    sound_play("level_end");
}

void sound_play_doorbell (void)
{
    sound_play("doorbell");
}

void sound_play_paper (void)
{
    sound_play("paper");
}

void sound_play_thief (void)
{
    sound_play("thief");
}

void sound_play_explosion (void)
{
    sound_play("explosion");
}

void sound_play_slime (void)
{
    sound_play("slime");
}

void sound_load_all (void)
{
    sound_load("data/sound/ratdeath.wav", "dead");
    sound_load("data/sound/click.wav", "click");
    sound_load("data/sound/chomp.wav", "chomp");
    sound_load("data/sound/chomp2.wav", "chomp2");
    sound_load("data/sound/powerup.wav", "powerup");
    sound_load("data/sound/spam.wav", "spam");
    sound_load("data/sound/letter.wav", "letter");
    sound_load("data/sound/rocket.wav", "rocket");
    sound_load("data/sound/level_end.wav", "level_end");
    sound_load("data/sound/doorbell.wav", "doorbell");
    sound_load("data/sound/paper.wav", "paper");
    sound_load("data/sound/thief.wav", "thief");
    sound_load("data/sound/explosion.wav", "explosion");
    sound_load("data/sound/slime.wav", "slime");
    sound_load("data/sound/swords_collide_sound_explorer_2015600826.wav", "sword");
    sound_load("data/sound/cash_register2.wav", "cash_register");
    sound_load("data/sound/shotgun_reload_by_ra_the_sun_god.wav", "shotgun_reload");
    sound_load("data/sound/shotgun_by_ra_the_sun_god.wav", "shotgun");
    sound_load("data/sound/swoosh_3_SoundBible.com_1573211927.wav", "swoosh");
}
