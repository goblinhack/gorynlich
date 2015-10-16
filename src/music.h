/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info.
 */

#pragma once

uint8_t music_init(void);
void music_fini(void);
musicp music_load(const char *file, const char *name);
musicp music_find(const char *name);
void music_play(const char *file, const char *alias, uint32_t rate);
void music_play_demo(void);
void music_play_death(void);
void music_play_intro(void);
void music_play_game(level_pos_t);
void music_play_dead(void);
void music_play_game_over(void);
void music_halt(void);
void music_update_volume(void);

extern uint8_t music_init_done;
