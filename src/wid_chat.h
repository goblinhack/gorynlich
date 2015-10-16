/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info.
 */

#pragma once

void wid_chat_fini(void);
uint8_t wid_chat_init(void);
void wid_chat_log(const char *s);
void wid_chat_hello(void);
void wid_chat_hide(void);
void wid_chat_visible(void);

/*
 * Global widgets.
 */
extern widp wid_chat_input_line;

extern widp wid_chat_window;
