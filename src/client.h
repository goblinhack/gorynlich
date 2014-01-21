/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 *
 * A useful hack to help debug stale/free/bad pointers.
 */

extern boolean client_init(void);
extern void client_fini(void);
extern void client_tick(void);
boolean client_socket_join(char *host, char *port, uint16_t portno);
boolean client_socket_leave(void);

extern socketp client_joined_server;
