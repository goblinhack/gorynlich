/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

typedef boolean(*command_fn_t)(tokenp, void *context);

#include "token.h"

void command_fini(void);
boolean command_init(void);
void command_add(command_fn_t callback,
                 const char *input,
                 const char *readable_input);
boolean command_handle(const char *input, char *expandedtext,
                       boolean show_ambiguous,
                       boolean show_complete,
                       boolean execute_command,
                       void *context);
void commands_test(void);

#include "linenoise.h"
void completion(const char *input, linenoiseCompletions *lc);
void linenoise_tick(void);
