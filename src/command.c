/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */

#include <unistd.h>
#include <fcntl.h>
#include "slre.h"
#include <SDL.h>

#include "main.h"
#include "command.h"
#include "string_util.h"
#include "term.h"
#include "wid.h"

/*
 * Simple console expanding code, takes a comand input and expands it as
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
    tokens_t input_tokens;
    command_fn_t callback;
} command_t;

static uint8_t command_inited;
static tree_root *commands;

void command_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (command_inited) {
        command_inited = false;

        tree_destroy(&commands, (tree_destroy_func)0);
    }
}

uint8_t command_init (void)
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

    tokens_tostring(input, &command->input_tokens);
    tokens_tostring(readable, &command->readable_tokens);

    if (!tree_insert(commands, &command->tree.node)) {
        ERR("insert of command %s fail", input);
    }
}

static int32_t command_matches (const char *input,
                                char *output,
                                uint8_t show_ambiguous,
                                uint8_t show_complete,
                                uint8_t execute_command,
                                void *context)
{
    char cand_expand_to[MAXSTR];
    command_t *matched_command;
    char completes_to[MAXSTR];
    char expands_to[MAXSTR];
    tokens_t input_tokens;
    char match[MAXSTR];
    char match2[MAXSTR];
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
                           (int) strlen(input_tokens.args[t]),
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
                           (int) strlen(input_tokens.args[t]),
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

            tokens_print_to(&command->input_tokens, match, sizeof(match));

            tokens_print_to(&command->readable_tokens, match2, sizeof(match2));

            if (show_ambiguous) {
                CON("  %-40s -- %s", match, match2);
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
                                (int) strlen(input_tokens.args[t]),
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

uint8_t command_handle (const char *input,
                        char *expandedtext,
                        uint8_t show_ambiguous,
                        uint8_t show_complete,
                        uint8_t execute_command,
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
                if (!strcasecmp(input, expandedtext)) {
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

#ifndef _WIN32
static char wid_text[MAXSTR];

static void console_set_text (const char *s)
{
    strcpy(wid_text, s);
}

static const char *console_get_text (void)
{
    return (wid_text);
}

static int cursor_x;

static void console_clear (void)
{
    int x;

    term_goto(0, TERM_HEIGHT - 1);

    for (x = 0; x < TERM_WIDTH; x++) {
        term_putc(' ');
    }
}

static void console_refresh (void)
{
    if (!HEADLESS && !enable_console) {
        return;
    }

    static char tmp[MAXSTR];
    static char cursor_char[2] = { '_', '\0' };

    strlcpy(tmp, wid_text, cursor_x + 1);
    strlcat(tmp, cursor_char, sizeof(tmp));
    strlcat(tmp, wid_text + cursor_x, sizeof(tmp));

    console_clear();

    term_goto(0, TERM_HEIGHT - 1);
    term_putf("gorynlich> ");
    term_putf(tmp);
    term_putf(" ");
    term_refresh();
}
#endif

void console_tick (void)
{
    if (!HEADLESS && !enable_console) {
        return;
    }

#ifndef _WIN32
    char seq[2];
    char seq2[2];
    char c;

    console_refresh();

    /*
     * Read nonblocking to get chars.
     */
    int fd = STDIN_FILENO;
    int flags = fcntl(fd, F_GETFL, 0);
    size_t nread;
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    nread = read(fd,&c,1);
    fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
    if (nread <= 0) {
        return;
    }

    char beforecursor[MAXSTR];
    char updatedtext[MAXSTR];
    char aftercursor[MAXSTR];
    char entered[MAXSTR];
    char newchar[2];
    int origlen;
    int cnt;

    newchar[0] = '\0';
    newchar[0] = c;

    origlen = (uint32_t)strlen(wid_text);

    strlcpy(beforecursor, wid_text, cursor_x + 1);
    strlcpy(aftercursor, wid_text + cursor_x, sizeof(aftercursor));

    switch (c) {
        case '':
            if (!history_walk) {
                history_walk = HISTORY_MAX - 1;
            } else {
                history_walk--;
            }

            console_set_text(history[history_walk]);
            cursor_x = (uint32_t)strlen(console_get_text());
            break;

        case '':
            history_walk++;
            if (history_walk >= HISTORY_MAX) {
                history_walk = 0;
            }

            console_set_text(history[history_walk]);
            cursor_x = (uint32_t)strlen(console_get_text());
            break;

        case '':
            cursor_x = 0;
            break;

        case '':
            cursor_x = origlen;
            break;

        case '':
        case '':
            if (cursor_x > 0) {
                strlcpy(updatedtext, beforecursor, cursor_x);
                strlcat(updatedtext, aftercursor, sizeof(updatedtext));

                cursor_x--;

                console_set_text(updatedtext);
            }
            break;

        case '\t':
            updatedtext[0] = '\0';
            command_handle(console_get_text(), updatedtext,
                            false /* show ambiguous */,
                            true /* show complete */,
                            false /* execute command */,
                            0 /* context */);

            if (updatedtext[0]) {
                console_set_text(updatedtext);
                cursor_x = (uint32_t)strlen(updatedtext);;
            }
            return;

        case '\n':
        case '\r':
            if (origlen) {
                strlcpy(entered, console_get_text(), sizeof(entered));

                if (!command_handle(entered, updatedtext,
                                true /* show ambiguous */,
                                false /* show complete */,
                                true /* execute command */,
                                0 /* context */)) {
                    return;
                }

                if (updatedtext[0]) {
                    console_set_text(updatedtext);
                    cursor_x = (uint32_t)strlen(updatedtext);;
                }

                strlcpy(history[history_at], updatedtext,
                        sizeof(history[history_at]));

                history_at++;
                if (history_at >= HISTORY_MAX) {
                    history_at = 0;
                }
                history_walk = history_at;

                console_set_text("");
                cursor_x = 0;
            } else {
                CON(" ");
            }
            return;

        case 27:    /* escape sequence */

            if (read(fd,seq,2) == -1) break;

            if (seq[0] == 91 && seq[1] == 68) {
                if (cursor_x > 0) {
                    cursor_x--;
                }
            } else if (seq[0] == 91 && seq[1] == 67) {
                if (cursor_x < origlen) {
                    cursor_x++;
                }
            } else if (seq[0] == 91 && (seq[1] == 65 || seq[1] == 66)) {
                /* Up and Down arrows */
                if (seq[1] == 65) {
                    cnt = 0;
                    while (cnt < HISTORY_MAX) {
                        cnt++;
                        if (!history_walk) {
                            history_walk = HISTORY_MAX - 1;
                        } else {
                            history_walk--;
                        }

                        console_set_text(history[history_walk]);
                        if (!history[history_walk][0]) {
                            continue;
                        }

                        cursor_x = (uint32_t)strlen(console_get_text());
                        break;
                    }
                    break;
                } else {
                    cnt = 0;
                    while (cnt < HISTORY_MAX) {
                        cnt++;

                        history_walk++;
                        if (history_walk >= HISTORY_MAX) {
                            history_walk = 0;
                        }

                        console_set_text(history[history_walk]);
                        if (!history[history_walk][0]) {
                            continue;
                        }

                        cursor_x = (uint32_t)strlen(console_get_text());
                        break;
                    }
                    break;
                }

            } else if (seq[0] == 91 && seq[1] > 48 && seq[1] < 55) {
                /* extended escape, read additional two bytes. */
                if (read(fd,seq2,2) == -1) break;

                if (seq[1] == 51 && seq2[0] == 126) {
                    /* Delete key. */
                    if (cursor_x > 0) {
                        strlcpy(updatedtext, beforecursor, cursor_x);
                        strlcat(updatedtext, aftercursor, sizeof(updatedtext));

                        cursor_x--;

                        console_set_text(updatedtext);
                    }
                    break;
                }
            }
            break;

        case '?':
            updatedtext[0] = '\0';
            command_handle(console_get_text(), updatedtext,
                            true /* show ambiguous */,
                            false /* show complete */,
                            false /* execute command */,
                            0 /* context */);

            if (updatedtext[0]) {
                console_set_text(updatedtext);
                cursor_x = (uint32_t)strlen(updatedtext);;
            }
            return;

        default: {
            if (origlen >= (typeof(origlen)) sizeof(updatedtext) - 1) {
                break;
            }

            newchar[1] = '\0';
            newchar[0] = c;
            if (!newchar[0]) {
                break;
            }

            strlcpy(updatedtext, beforecursor, cursor_x + 1);
            strlcat(updatedtext, newchar, sizeof(updatedtext));
            strlcat(updatedtext, aftercursor, sizeof(updatedtext));

            cursor_x++;

            console_set_text(updatedtext);
        }
    }
#endif
}
