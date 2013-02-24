/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file.
 */

void wid_console_fini(void);
boolean wid_console_init(void);
void wid_console_log(const char *s);
void wid_console_hello(void);

/*
 * Global widgets.
 */
extern widp wid_console_input_line;

extern widp wid_console_window;
