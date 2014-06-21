/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#pragma once

#include "tree.h"

uint8_t wid_hiscore_init(void);
void wid_hiscore_fini(void);
void wid_hiscore_hide(void);
void wid_hiscore_visible(void);
void hiscore_add(const char *name, uint32_t score);
uint8_t hiscore_save(void);
uint8_t hiscore_load(void);
widp hiscore_try_to_add(uint32_t score_in);

typedef struct hiscore_ {
    tree_key_two_int tree;
    char *name;
} hiscore;

extern tree_rootp hiscores;
