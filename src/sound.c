/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */

#include <SDL_mixer.h>
#include <stdlib.h>

#include "main.h"
#include "sound.h"
#include "ramdisk.h"
#include "tree.h"
#include "config.h"
#include "music.h"
#include "math_util.h"
#include "thing.h"
#include "vision.h"

typedef struct sound_ {
    tree_key_string tree;
    Mix_Chunk *sound;
    unsigned char *data;
    int32_t len;
    double volume;
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

static soundp sound_load (double volume, const char *filename, const char *name_alias)
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

    m->volume = volume;

    SDL_RWops *rw = SDL_RWFromMem(m->data, m->len);
    if (!rw) {
        ERR("cannot make RW sound %s", filename);
        return (0);
    }

    m->sound = Mix_LoadWAV_RW(rw, 0 /* freesrc */);
    if (!m->sound) {
        ERR("cannot make sound %s: %s %s", filename, Mix_GetError(),
            SDL_GetError());
        SDL_ClearError();
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

void sound_play_at (const char *name_alias, double x, double y)
{
    if (HEADLESS) {
        return;
    }

    if (!music_init_done) {
        return;
    }

    soundp sound = sound_load(0.5, 0, name_alias);
    if (!sound) {
        LOG("cannot load sound %s: %s", name_alias, Mix_GetError());

        return;
    }

    double volume = (float) global_config.sound_volume * sound->volume *
              ((float) MIX_MAX_VOLUME / (float) SOUND_MAX);

    if (player) {
        int sx, sy;

        real_to_map(x, y, &sx, &sy);

        int distance = dmap_distance_to_player(sx, sy);
        if (distance == -1) {
            return;
        }

        /*
         * Cheap effect, make the light fade away with distance.
         */
        double scale = (256.0 - (((double)distance) * 4.0)) / 256.0;
        if (scale <= 0.1) {
            return;
        }

        volume *= scale;

        if (distance > 1) {
            if (!can_see(client_level, player->x, player->y, sx, sy)) {
                volume /= 2.0;
            }
        }

        if (volume < 1.0) {
            return;
        }

        LOG("Client: play: %s vol %f dist %d",name_alias, volume, distance);
    }

    if (Mix_PlayChannel(-1, sound->sound, 0) == -1) {
        LOG("Client: cannot play sound %s: %s", sound->tree.key, Mix_GetError());

        return;
    }

    Mix_VolumeChunk(sound->sound, volume);
}

/*
 * Like the above but no checks for line of sight and always make some sound 
 * even if quiet
 */
void sound_play_global_at (const char *name_alias, double x, double y)
{
    if (HEADLESS) {
        return;
    }

    if (!music_init_done) {
        return;
    }

    soundp sound = sound_load(0.5, 0, name_alias);
    if (!sound) {
        LOG("cannot load sound %s: %s", name_alias, Mix_GetError());

        return;
    }

    double volume = (float) global_config.sound_volume * sound->volume *
              ((float) MIX_MAX_VOLUME / (float) SOUND_MAX);

    if (player) {
        double distance = DISTANCE(player->x, player->y, x, y) / 8.0;

        if (distance > 1.0) {
            volume /= distance;
        }

        int visible = true;

        if (volume < 1.0) {
            volume = 1.0;
        }

        LOG("Client: play: global %s vol %f dist %f can_see %d",name_alias, volume, distance, visible);
    }

    if (Mix_PlayChannel(-1, sound->sound, 0) == -1) {
        /*
         * Try harder for global sounds as they are important.
         */
        Mix_HaltChannel(0);

        if (Mix_PlayChannel(-1, sound->sound, 0) == -1) {
            ERR("cannot play sound %s: %s", sound->tree.key, Mix_GetError());
            return;
        }
    }

    Mix_VolumeChunk(sound->sound, volume);
}

void sound_play (const char *name_alias)
{
    if (HEADLESS) {
        return;
    }

    if (!music_init_done) {
        return;
    }

    LOG("Client: play: %s", name_alias);

    soundp sound = sound_load(0.5, 0, name_alias);
    if (!sound) {
        LOG("cannot load sound %s: %s", name_alias, Mix_GetError());

        return;
    }

    if (Mix_PlayChannel(-1, sound->sound, 0) == -1) {
        LOG("cannot play %s: %s", sound->tree.key, Mix_GetError());

        return;
    }

    double volume = (float) global_config.sound_volume * sound->volume *
              ((float) MIX_MAX_VOLUME / (float) SOUND_MAX);


    LOG("Client: play: %s vol %f",name_alias,volume);

    Mix_VolumeChunk(sound->sound, volume);
}

void sound_play_n (const char *name_alias, int32_t n)
{
    if (HEADLESS) {
        return;
    }

    if (!music_init_done) {
        return;
    }

    soundp sound = sound_load(0.5, 0, name_alias);

    if (Mix_PlayChannel(-1, sound->sound, n) == -1) {
        LOG("Client: cannot play %s: %s", sound->tree.key, Mix_GetError());

        return;
    }

    Mix_VolumeChunk(sound->sound,
                    (float) global_config.sound_volume * sound->volume *
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

void sound_play_level_end (void)
{
    sound_play("level_end");
}

void sound_play_slime (void)
{
    sound_play("slime");
}

void sound_load_all (void)
{
    sound_load(0.5, "data/sound/click.wav", "click");
    sound_load(0.5, "data/sound/powerup.wav", "powerup");
    sound_load(0.7, "data/sound/teleport.wav", "teleport");
    sound_load(0.7, "data/sound/188712__oceanictrancer__happy_effect.wav", "level_end");
    sound_load(0.7, "data/sound/explosion.wav", "explosion");
    sound_load(0.2, "data/sound/slime.wav", "slime");
    sound_load(0.5, "data/sound/swords_collide_sound_explorer_2015600826.wav", "sword");
    sound_load(0.1, "data/sound/cash_register2.wav", "cash_register");
    sound_load(0.5, "data/sound/shotgun_reload_by_ra_the_sun_god.wav", "shotgun_reload");
    sound_load(0.5, "data/sound/shotgun_by_ra_the_sun_god.wav", "shotgun");
    sound_load(0.2, "data/sound/60009__qubodup__swosh_22.ogg", "swoosh");
    sound_load(0.2, "data/sound/flame_Arrow_SoundBible.com_618067908.wav", "fireball1");
    sound_load(0.2, "data/sound/105016__julien_matthey__jm_fx_fireball_01.wav", "fireball2");
    sound_load(0.2, "data/sound/39016__wildweasel__dsfirxpl.wav", "fireball3");
    sound_load(0.2, "data/sound/219566__qubodup__poison_spell_magic.ogg", "poison");
    sound_load(0.2, "data/sound/207562__jwsoundfoley__open_24oz_soda_bottle_outdoors_long_fizz_explode.ogg", "acid");
    sound_load(0.5, "data/sound/coin_roll.wav", "payment");
    sound_load(0.5, "data/sound/treasure.wav", "treasure");
    sound_load(1.0, "data/sound/85568__joelaudio__dragon_roar.wav", "dragon");
    sound_load(1.0, "data/sound/212764__qubodup__lion_roar.ogg", "orc");
    sound_load(1.0, "data/sound/Red_Alert_FX_001.wav", "thief");
    sound_load(1.0, "data/sound/boom.wav", "boom");
    sound_load(0.5, "data/sound/effect.wav", "effect");
    sound_load(0.3, "data/sound/zombie_SoundBible.com_1966938763.wav", "zombie");
    sound_load(0.3, "data/sound/Bite_SoundBible.com_2056759375.wav", "bite");
    sound_load(0.1, "data/sound/bite_SoundBible.com_1625781385.wav", "urgh");
    sound_load(0.4, "data/sound/zombie_in_painSoundBible.com_134322253.wav", "urgh2");
    sound_load(0.7, "data/sound/demon_die.wav", "urgh3");
    sound_load(0.5, "data/sound/82388__robinhood76__01308_man_hit_9.wav", "player_hit");
    sound_load(0.5, "data/sound/188363__darkalicenite__grunt.wav", "female_player_hit");
    sound_load(0.8, "data/sound/45137__dj_chronos__dark_church_bell.wav", "player_death");
    sound_load(0.5, "data/sound/205563__everheat__arrow.wav", "arrow");
    sound_load(0.2, "data/sound/49676__ejfortin__energy_short_sword_5.wav", "energy1");
    sound_load(0.2, "data/sound/146733__fins__energy.wav", "energy2");
    sound_load(0.3, "data/sound/148975__adam_n__squelch_1.wav", "squelch");
    sound_load(0.5, "data/sound/Object_Drop_001.wav", "drop");
    sound_load(0.5, "data/sound/Door_Latch_002.wav", "door");
    sound_load(0.5, "data/sound/Electric_Zap.wav", "shield");
    sound_load(1.0, "data/sound/207809__exsomniel__dark_choir_01.wav", "jesus");
    sound_load(1.0, "data/sound/65195__uair01__chamber_choir_chord_o.wav", "choir");
    sound_load(0.5, "data/sound/238855__limetoe__collected_item.wav", "potion");
    sound_load(0.5, "data/sound/317827__blockfighter298__weapon_pickup.wav", "pickup");
    sound_load(0.5, "data/sound/170243__beatsbycasper__car_horn.wav", "beep");
    sound_load(0.5, "data/sound/244745__reitanna__egg_crack19.wav", "egg");
    sound_load(0.2, "data/sound/saw.wav", "saw");
}
