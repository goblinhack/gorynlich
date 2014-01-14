/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

boolean wid_server_init(void);
void wid_server_fini(void);
void wid_server_hide(void);
void wid_server_visible(void);
void server_add(const char *name, uint32_t score);
boolean server_save(void);
boolean server_load(void);
widp server_try_to_add(uint32_t score_in);
