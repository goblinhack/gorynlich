/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */

#pragma once

widp wid_popup(const char *text, const char *title,
               float x, float y,
               fontp title_font,
               fontp body_font,
               fontp button_font,
               int32_t args, ...);

widp wid_popup_simple(const char *text);
widp wid_popup_ok(const char *text);
widp wid_popup_error(const char *text);
