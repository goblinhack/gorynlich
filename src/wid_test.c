/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */

void wid_test_wid_create(void);

static widp wid_test_window;
static widp wid_test_container;
static widp wid_test_vert_scroll;
static widp wid_test_horiz_scroll;

/*
 * Create the wid_test
 */
void wid_test_wid_create (void)
{
    {
        fpoint tl = {0.2f, 0.2f};
        fpoint br = {0.8f, 0.8f};
        color c;

        wid_test_window = wid_new_square_window("test");

        c = WHITE;
        c.a = 230;

        wid_set_mode(wid_test_window, WID_MODE_NORMAL);
        wid_set_color(wid_test_window, WID_COLOR_TL, c);
        wid_set_color(wid_test_window, WID_COLOR_BG, c);
        wid_set_color(wid_test_window, WID_COLOR_BR, c);
        wid_set_color(wid_test_window, WID_COLOR_TEXT, WHITE);

        wid_set_name(wid_test_window, "wid_test");

        wid_set_text_bot(wid_test_window, true);
        wid_set_text_lhs(wid_test_window, true);
        wid_set_tl_br_pct(wid_test_window, tl, br);

        fsize sz = {0.0f, 0.0f};
        wid_set_tex_tl(wid_test_window, sz);

        fsize sz2 = {1.32f, 1.0f};
        wid_set_tex_br(wid_test_window, sz2);
    }

    {
        fpoint tl = {0.0f, 0.0f};
        fpoint br = {1.0f, 1.0f};

        wid_test_container = wid_new_container(wid_test_window, "");

        wid_set_tl_br_pct(wid_test_container, tl, br);
        wid_set_text_lhs(wid_test_container, true);
        wid_set_text_bot(wid_test_container, true);
        color c;
        c = STEELBLUE;
        c.a = 50;

        wid_set_mode(wid_test_container, WID_MODE_NORMAL);
        wid_set_color(wid_test_container, WID_COLOR_TL, c);
        wid_set_color(wid_test_container, WID_COLOR_BG, c);
        wid_set_color(wid_test_container, WID_COLOR_BR, c);
        wid_set_color(wid_test_container, WID_COLOR_TEXT, WHITE);
    }

    {
        fpoint tl = {0.0f, 0.0f};
        fpoint br = {0.6f, 0.6f};
        color c;

        widp child = wid_new_rounded_button(wid_test_container, "button 1");
        wid_set_tl_br_pct(child, tl, br);

        c = STEELBLUE;
        c.a = 50;

        wid_set_mode(child, WID_MODE_NORMAL);
        wid_set_color(child, WID_COLOR_TL, c);
        wid_set_color(child, WID_COLOR_BG, c);
        wid_set_color(child, WID_COLOR_BR, c);
        wid_set_color(child, WID_COLOR_TEXT, WHITE);
    }

    {
        fpoint tl = {0.8f, 0.8f};
        fpoint br = {1.2f, 1.2f};

        widp child = wid_new_rounded_button(wid_test_container, "button 2");
        wid_set_tl_br_pct(child, tl, br);
        wid_set_text(child, "long name");
    }

    wid_test_vert_scroll =
        wid_new_vert_scroll_bar(wid_test_window, wid_test_container);
    wid_test_horiz_scroll =
        wid_new_horiz_scroll_bar(wid_test_window, wid_test_container);

    wid_move_to_bottom(wid_test_vert_scroll);
    wid_move_to_right(wid_test_horiz_scroll);

    wid_visible(wid_get_parent(wid_test_vert_scroll), 0);
    wid_visible(wid_test_vert_scroll, 0);
    wid_raise(wid_get_parent(wid_test_vert_scroll));

    wid_visible(wid_get_parent(wid_test_horiz_scroll), 0);
    wid_visible(wid_test_horiz_scroll, 0);
    wid_raise(wid_get_parent(wid_test_horiz_scroll));

    wid_raise(wid_test_vert_scroll);
    wid_raise(wid_test_horiz_scroll);

    wid_update(wid_test_window);
}
