/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

widp wid_tooltip(const char *text, float x, float y, fontp font);
widp wid_tooltip_simple(const char *text);
widp wid_tooltip_transient(const char *text, uint32_t delay);
widp wid_tooltip_large_transient(const char *text, uint32_t delay);
