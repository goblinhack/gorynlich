/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */

#pragma once

typedef uint8_t(*command_fn_t)(tokenp, void *context);

#include "token.h"

void command_fini(void);
uint8_t command_init(void);
void command_add(command_fn_t callback,
                 const char *input,
                 const char *readable_input);
uint8_t command_handle(const char *input, char *expandedtext,
                       uint8_t show_ambiguous,
                       uint8_t show_complete,
                       uint8_t execute_command,
                       void *context);
void commands_test(void);

void console_tick(void);
