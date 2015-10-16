/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */

#pragma once

widp wid_textbox(widp parent,
                 widp *textbox,
                 const char *text, float x, float y, fontp font);

widp wid_textbox_fixed_width(widp parent,
                 widp *textbox,
                 const char *text, float x, float y, fontp font);

widp wid_textbox_simple(widp parent,
                        widp *textbox,
                        const char *text, float x, float y);
