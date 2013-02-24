/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file.
 */

boolean music_init(void);
void music_fini(void);
musicp music_load(const char *file, const char *name);
musicp music_find(const char *name);
void music_play(const char *file, const char *alias, uint32_t rate);
void music_play_intro(void);
void music_play_game(void);
void music_play_hiscore(void);
void music_halt(void);
void music_update_volume(void);

extern boolean music_init_done;
