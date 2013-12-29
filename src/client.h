/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 *
 * A useful hack to help debug stale/free/bad pointers.
 */

extern boolean client_init(void);
extern void client_fini(void);
extern void client_poll(void);
