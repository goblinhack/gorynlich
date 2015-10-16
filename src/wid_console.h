/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info.
 */

#pragma once

void wid_console_fini(void);
uint8_t wid_console_init(void);
void wid_console_log(const char *s);
void wid_console_hello(void);

/*
 * Global widgets.
 */
extern widp wid_console_input_line;

extern widp wid_console_window;
