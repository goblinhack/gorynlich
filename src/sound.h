/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info.
 */

#pragma once

uint8_t sound_init(void);
void sound_fini(void);
soundp sound_find(const char *name);
void sound_play(const char *name);
void sound_play_global_at(const char *name, double x, double y);
void sound_play_at(const char *name, double x, double y);
void sound_play_n(const char *name, int32_t n);
void sound_play_dead(void);
void sound_play_slime(void);
void sound_play_level_end(void);
void sound_play_click(void);
void sound_load_all(void);
