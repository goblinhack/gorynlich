/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */

#pragma once

widp wid_button(const char *text, float x, float y, fontp font);
widp wid_button_simple(const char *text);
widp wid_button_transient(const char *text, uint32_t delay);
widp wid_button_large_transient(const char *text, uint32_t delay);
widp wid_button_large(const char *text);
widp wid_fullscreen_text(const char *text);
