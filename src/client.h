/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */

#pragma once

extern uint8_t client_init(void);
extern void client_fini(void);
extern void client_tick(void);
uint8_t client_socket_join(const char *host, const char *port, uint16_t portno,
                           uint8_t quiet);
uint8_t client_socket_close(char *host, char *port);
uint8_t client_socket_leave(void);
uint8_t client_socket_set_name(const char *name);
uint8_t client_socket_set_pclass(const char *name);
uint8_t client_socket_shout(char *shout);
msg_player_statep client_get_player(void);

extern gsocketp client_joined_server;
extern uint32_t client_player_died;

/*
 * client_hooks.c
 */
void client_level_starting(void);
void client_level_ending(void);
struct msg_server_hiscores_;
void client_player_fully_dead(struct msg_server_hiscores_ *);
