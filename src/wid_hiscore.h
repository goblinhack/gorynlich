/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

boolean wid_hiscore_init(void);
void wid_hiscore_fini(void);
void wid_hiscore_hide(void);
void wid_hiscore_visible(void);
void hiscore_add(const char *name, uint32_t score);
boolean hiscore_save(void);
boolean hiscore_load(void);
widp hiscore_try_to_add(uint32_t score_in);
