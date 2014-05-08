/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 *
 * A useful hack to help debug stale/free/bad pointers.
 */

#pragma once

extern boolean client_init(void);
extern void client_fini(void);
extern void client_tick(void);
boolean client_socket_join(char *host, char *port, uint16_t portno,
                           boolean quiet);
boolean client_socket_leave(void);
boolean client_socket_set_name(const char *name);
boolean client_socket_set_pclass(const char *name);
msg_player_statep client_get_player(int n);

extern socketp client_joined_server;
