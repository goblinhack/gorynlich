/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */

uint8_t wid_dead_init(void);
void wid_dead_fini(void);
void wid_dead_hide(void);
void wid_dead_visible(const char *name, 
                      const char *reason,
                      uint8_t rejoin_allowed);
