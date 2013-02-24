/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

typedef struct tokens_t_ {
    struct slre regexp[MAXSTR];
    char *args[MAXSTR];
    char data[MAXSTR];
    uint32_t cnt;
} tokens_t;

tokens_t *tokens_tostring(const char *input, tokens_t *);
void tokens_print_to(tokens_t *, char *output, int32_t output_size);
void tokens_print(tokens_t *);
void tokens_test(void);
