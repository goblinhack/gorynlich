/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "slre.h"

#include "main.h"
#include "command.h"
#include "tree.h"
#include "string.h"

/*
 * Simple wid_console expanding code, takes a comand input and expands it as
 * much as possible to installed commands.
 *
 * Ambigious command : []
 *  Possible match   : set god-mode off
 *  Possible match   : set god-mode on
 *  Possible match   : set debug off
 *  Possible match   : set debug on
 *   Expand to       :
 *
 * Ambigious command : [s]
 *  Possible match   : set god-mode off
 *  Possible match   : set god-mode on
 *  Possible match   : set debug off
 *  Possible match   : set debug on
 *   Expand to       : set
 *
 * Ambigious command : [se]
 *  Possible match   : set god-mode off
 *  Possible match   : set god-mode on
 *  Possible match   : set debug off
 *  Possible match   : set debug on
 *   Expand to       : set
 *
 * Ambigious command : [set]
 *  Possible match   : set god-mode off
 *  Possible match   : set god-mode on
 *  Possible match   : set debug off
 *  Possible match   : set debug on
 *   Expand to       : set
 *
 * Ambigious command : [set d]
 *  Possible match   : set debug off
 *  Possible match   : set debug on
 *   Expand to       : set debug
 *
 * Ambigious command : [set g]
 *  Possible match   : set god-mode off
 *  Possible match   : set god-mode on
 *   Expand to       : set god-mode
 *
 * Ambigious command : [s g ]
 *  Possible match   : set god-mode off
 *  Possible match   : set god-mode on
 *   Expand to       : set god-mode
 *
 * Ambigious command : [s g o]
 *  Possible match   : set god-mode off
 *  Possible match   : set god-mode on
 *   Expand to       : set god-mode o
 *
 * Single match      : [s g of]
 *  Completes to     : set god-mode off
 *
 * Ambigious command : [s d]
 *  Possible match   : set debug off
 *  Possible match   : set debug on
 *   Expand to       : set debug
 *
 * Single match      : [s d on]
 *  Completes to     : set debug on
 *
 * Single match      : [s d of]
 *  Completes to     : set debug off
 *
 * Single match      : [set debug on]
 *  Completes to     : set debug on
 */

typedef struct command_t_ {
    tree_key_string tree;
    tokens_t tokens;
    tokens_t readable_tokens;
    command_fn_t callback;
} command_t;

static boolean command_inited;
static tree_root *commands;

void command_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (command_inited) {
        command_inited = false;

        tree_destroy(&commands, (tree_destroy_func)0);
    }
}

boolean command_init (void)
{
    command_inited = true;

    return (true);
}

void command_add (command_fn_t callback,
                  const char *input,
                  const char *readable)
{
    if (!commands) {
        commands = tree_alloc(TREE_KEY_STRING, "TREE ROOT: commands");
    }

    command_t *command = (typeof(command))
                    myzalloc(sizeof(*command), "TREE NODE: command");
    command->tree.key = dupstr(input, "TREE KEY: command");
    command->callback = callback;

    /*
     * Convert the command into tokens for matching.
     */
    tokens_tostring(input, &command->tokens);

    if (readable[0] == '\0') {
        tokens_tostring(input, &command->readable_tokens);
    } else {
        tokens_tostring(readable, &command->readable_tokens);
    }

    if (!tree_insert(commands, &command->tree.node)) {
        DIE("insert of command %s fail", input);
    }
}

static int32_t command_matches (const char *input,
                                char *output,
                                boolean show_ambiguous,
                                boolean show_complete,
                                boolean execute_command,
                                void *context)
{
    char cand_expand_to[MAXSTR];
    command_t *matched_command;
    char completes_to[MAXSTR];
    char expands_to[MAXSTR];
    tokens_t input_tokens;
    char match[MAXSTR];
    int32_t longest_match;
    int32_t common_len;
    command_t *command;
    int32_t matches;
    int32_t cnt;
    int32_t t;

    matched_command = 0;
    longest_match = -1;
    matches = 0;

    /*
     * Convert the input into tokens for matching.
     */
    tokens_tostring(input, &input_tokens);

    /*
     * Find the command(s) with the most number of matching tokens.
     */
    TREE_WALK(commands, command) {

        for (t = 0; t < (int32_t)min(command->tokens.cnt,
                                     input_tokens.cnt); t++) {

            cnt = strncmp(command->tokens.args[t],
                          input_tokens.args[t],
                          strlen(input_tokens.args[t]));

            if (slre_match(&command->tokens.regexp[t],
                           input_tokens.args[t],
                           strlen(input_tokens.args[t]),
                           0 /* captures */)) {
                /*
                 * Success
                 */
                cnt = 0;
            }

            if (cnt) {
                t = -1;
                break;
            }
        }

        longest_match = max(t, longest_match);
    }

    if (longest_match == -1) {
        return (0);
    }

    /*
     * Repeat and optionally dump other possibilities if the command is
     * not complete.
     */
    { TREE_WALK(commands, command) {

        for (t = 0; t < (int32_t)min(command->tokens.cnt,
                                     input_tokens.cnt); t++) {

            cnt = strncmp(command->tokens.args[t],
                          input_tokens.args[t],
                          strlen(input_tokens.args[t]));

            if (slre_match(&command->tokens.regexp[t],
                           input_tokens.args[t],
                           strlen(input_tokens.args[t]),
                           0 /* captures */)) {
                /*
                 * Success
                 */
                cnt = 0;
            }

            if (cnt) {
                break;
            }
        }

// tokens_print_to(&command->readable_tokens, match, sizeof(match));
        if (t == longest_match) {
            matches++;
// CON("  MATCH    \"%s\" [%d] longest %d", match,t,longest_match);

            matched_command = command;

            if (show_complete) {
                completes_to[0] = '\0';

                for (t = 0; t < longest_match; t++) {
                    strlcat(completes_to, command->tokens.args[t],
                            sizeof(completes_to));
                    strlcat(completes_to, " ", sizeof(completes_to));
                }

                if (output) {
                    strlcpy(output, completes_to, MAXSTR);
                }
            }

            tokens_print_to(&command->readable_tokens, match, sizeof(match));

            if (show_ambiguous) {
                CON("  \"%s\"", match);
            }
        } else {
// CON("  NO MATCH \"%s\" [%d] longest %d", match,t,longest_match);
        }
    } }

    /*
     * Repeat and complete the command to any full matches.
     */
    {
        expands_to[0] = '\0';

        {
            TREE_WALK(commands, command) {

                for (t = 0; t < (int32_t)min(command->tokens.cnt,
                                            input_tokens.cnt);
                    t++) {

                    cnt = strncmp(command->tokens.args[t],
                                input_tokens.args[t],
                                strlen(input_tokens.args[t]));

                    if (slre_match(&command->tokens.regexp[t],
                                input_tokens.args[t],
                                strlen(input_tokens.args[t]),
                                0 /* captures */)) {
                        /*
                         * Success
                         */
                        cnt = 0;
                    }

                    if (cnt) {
                        break;
                    }
                }

                if (t == longest_match) {
                    cand_expand_to[0] = '\0';

                    for (t = 0; t < longest_match; t++) {
                        if (strisregexp(command->tokens.args[t])) {
                            strlcat(cand_expand_to, input_tokens.args[t],
                                    sizeof(cand_expand_to));
                            strlcat(cand_expand_to, " ", sizeof(cand_expand_to));
                            continue;
                        }

                        strlcat(cand_expand_to, command->tokens.args[t],
                                sizeof(cand_expand_to));
                        strlcat(cand_expand_to, " ", sizeof(cand_expand_to));
                    }


                    if (expands_to[0] != '\0') {
                        common_len = strcommon(expands_to, cand_expand_to);
                        expands_to[common_len] = '\0';
                    } else {
                        strlcpy(expands_to, cand_expand_to,
                                sizeof(expands_to));
                    }
                }
            }

            /*
             * Expands to:
             */
            if (output) {
                strlcpy(output, expands_to, MAXSTR);
            }
        }
    }

    if (execute_command && matched_command && (matches == 1)) {
        (*matched_command->callback)(&input_tokens, context);
    }

    return (matches);
}

boolean command_handle (const char *input,
                        char *expandedtext,
                        boolean show_ambiguous,
                        boolean show_complete,
                        boolean execute_command,
                        void *context)
{
    int32_t matches;

    if (expandedtext) {
        *expandedtext = '\0';
    }

    /*
     * Check for ambiguous commands.
     */
    matches = command_matches(input, expandedtext, false, false,
                              execute_command, context);
    if (matches == 0) {
        CON("> %%%%fg=red$Unknown command: \"%s\"%%%%fg=reset$", input);
        return (false);
    }

    if (matches > 1) {
        if (show_ambiguous) {
            CON("> %%%%fg=red$Incomplete command, "
                "\"%s\"%%%%fg=reset$. Try:", input);
        }

        command_matches(input, expandedtext, show_ambiguous, show_complete,
                        execute_command, context);

        if (!show_ambiguous) {
            if (expandedtext) {
                if (!strcmp(input, expandedtext)) {
                    CON("> %%%%fg=red$Incomplete command, "
                        "\"%s\"%%%%fg=reset$. Try:", input);

                    command_matches(input, expandedtext, true, show_complete,
                                    execute_command, context);
                }
            } else {
                command_matches(input, expandedtext, true, show_complete,
                                execute_command, context);
            }
        }

        return (false);
    }

    if (!execute_command && (matches == 1)) {
        CON("> %%%%fg=red$Incomplete command, "
            "\"%s\"%%%%fg=reset$. Try:", input);

        command_matches(input, expandedtext, true, show_complete,
                        execute_command, context);
    }

    return (true);
}
