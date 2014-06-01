/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#pragma once

widp wid_notify(const char *text, uint32_t level);
void wid_notify_tick(void);

enum {
    NORMAL,
    WARNING,
    SEVERE
};
