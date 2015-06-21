/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file.
 */

#pragma once

uint8_t sound_init(void);
void sound_fini(void);
soundp sound_load(const char *file, const char *name);
soundp sound_find(const char *name);
void sound_play(const char *name);
void sound_play_at(const char *name, double x, double y);
void sound_play_n(const char *name, int32_t n);
void sound_play_dead(void);
void sound_play_click(void);
void sound_play_chomp(void);
void sound_play_chomp2(void);
void sound_play_powerup(void);
void sound_play_spam(void);
void sound_play_letter(void);
void sound_play_rocket(void);
void sound_play_level_end(void);
void sound_play_doorbell(void);
void sound_play_paper(void);
void sound_play_thief(void);
void sound_play_explosion(void);
void sound_play_slime(void);
void sound_load_all(void);
