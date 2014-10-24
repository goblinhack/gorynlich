/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "color.h"
#include "wid_editor.h"
#include "wid_editor_buttons.h"
#include "thing.h"

widp wid_editor_buttons_window;
widp wid_editor_buttons_container1;
widp wid_editor_buttons_container2;
widp wid_editor_buttons_container3;
widp wid_editor_wid_draw;
widp wid_editor_wid_line;
widp wid_editor_wid_fill;
widp wid_editor_wid_eraser;
widp wid_editor_wid_undo;
widp wid_editor_wid_redo;

static widp wid_editor_buttons_vert_scroll;

static float wid_tile_height = 0.07;

static float wid_button_width = 1.0f / 4.0f;
static float wid_button_height = 1.0f / 3.0f;

static const int32_t ICONS_ACROSS = 4;
static const int32_t TILES_ACROSS = 6;
static const int32_t TILES_DOWN = 10;

/*
 * Mouse up etc...
 */
static uint8_t wid_editor_buttons_receive_mouse_down (widp w,
                                                    int32_t x,
                                                    int32_t y,
                                                    uint32_t button)
{
    return (false);
}

/*
 * Mouse up etc...
 */
static uint8_t
wid_editor_buttons_tile_receive_mouse_down (widp w,
                                          int32_t x,
                                          int32_t y,
                                          uint32_t button)
{
    widp focus;
    thing_templatep tp;

    focus = wid_get_focus(wid_editor_buttons_container2);
    if (!focus) {
        return (false);
    }

    tp = wid_get_thing_template(w);
    if (!tp) {
        return (false);
    }

    wid_set_thing_template(focus, tp);

    return (true);
}

/*
 * Mouse up etc...
 */
static uint8_t
wid_editor_buttons_recent_tile_receive_mouse_down (widp w,
                                                 int32_t x,
                                                 int32_t y,
                                                 uint32_t button)
{
    wid_set_focus(w);

    return (false);
}

/*
 * Mouse up etc...
 */
static uint8_t
wid_editor_buttons_icon_back_receive_mouse_down (widp w,
                                               int32_t x,
                                               int32_t y,
                                               uint32_t button)
{
    wid_editor_hide();

    return (true);
}

/*
 * Mouse up etc...
 */
static uint8_t
wid_editor_buttons_icon_save_receive_mouse_down (widp w,
                                               int32_t x,
                                               int32_t y,
                                               uint32_t button)
{
    wid_editor_save();

    return (true);
}

/*
 * Mouse up etc...
 */
static uint8_t
wid_editor_buttons_icon_title_receive_mouse_down (widp w,
                                               int32_t x,
                                               int32_t y,
                                               uint32_t button)
{
    wid_editor_title();

    return (true);
}

/*
 * Mouse up etc...
 */
static uint8_t
wid_editor_buttons_icon_load_receive_mouse_down (widp w,
                                               int32_t x,
                                               int32_t y,
                                               uint32_t button)
{
    wid_editor_load();

    return (true);
}

/*
 * Mouse up etc...
 */
static uint8_t
wid_editor_buttons_icon_draw_receive_mouse_down (widp w,
                                               int32_t x,
                                               int32_t y,
                                               uint32_t button)
{
    wid_editor_draw();

    return (true);
}

/*
 * Mouse up etc...
 */
static uint8_t
wid_editor_buttons_icon_line_receive_mouse_down (widp w,
                                               int32_t x,
                                               int32_t y,
                                               uint32_t button)
{
    wid_editor_line();

    return (true);
}

/*
 * Mouse up etc...
 */
static uint8_t
wid_editor_buttons_icon_fill_receive_mouse_down (widp w,
                                               int32_t x,
                                               int32_t y,
                                               uint32_t button)
{
    wid_editor_fill();

    return (true);
}

/*
 * Mouse up etc...
 */
static uint8_t
wid_editor_buttons_icon_clear_receive_mouse_down (widp w,
                                                int32_t x,
                                                int32_t y,
                                                uint32_t button)
{
    wid_editor_clear();

    return (true);
}

/*
 * Mouse up etc...
 */
static uint8_t
wid_editor_buttons_icon_erase_receive_mouse_down (widp w,
                                                int32_t x,
                                                int32_t y,
                                                uint32_t button)
{
    wid_editor_erase();

    return (true);
}

/*
 * Mouse up etc...
 */
static uint8_t
wid_editor_buttons_icon_undo_receive_mouse_down (widp w,
                                                int32_t x,
                                                int32_t y,
                                                uint32_t button)
{
    wid_editor_undo();

    return (true);
}

/*
 * Mouse up etc...
 */
static uint8_t
wid_editor_buttons_icon_redo_receive_mouse_down (widp w,
                                                int32_t x,
                                                int32_t y,
                                                uint32_t button)
{
    wid_editor_redo();

    return (true);
}

static thing_templatep first_thing_template;

static uint8_t wid_editor_buttons_add_tiles (const tree_node *node, void *arg)
{
    static int32_t x;
    static int32_t y;
    thing_templatep tp;
    widp child;

    tp = (typeof(tp)) 
            (((char*) node) - STRUCT_OFFSET(struct thing_template_, tree2));

    if (thing_template_is_hidden_from_editor(tp)) {
        return (true);
    }

    if (!first_thing_template) {
        first_thing_template = tp;
    }

    float w = 1.0 / TILES_ACROSS;
    float h = 1.0 / TILES_DOWN;

    fpoint tl = {
        (w * (float)(x)),
        (h * (float)(y))
    };

    fpoint br = {
        (w * (float)(x+1)),
        (h * (float)(y+1))
    };

    child = wid_new_square_button(wid_editor_buttons_container1,
                                  "palette tile");

    wid_set_color(child, WID_COLOR_BG, BLACK);
    wid_set_color(child, WID_COLOR_TEXT, WHITE);
    wid_set_color(child, WID_COLOR_BR, PINK);
    wid_set_color(child, WID_COLOR_BG, BLACK);

    wid_set_mode(child, WID_MODE_OVER);
    wid_set_color(child, WID_COLOR_BG, STEELBLUE);

    wid_set_mode(child, WID_MODE_NORMAL);

    wid_set_thing_template(child, tp);
    wid_set_tooltip(child, thing_template_get_tooltip(tp));
    wid_set_tl_br_pct(child, tl, br);
    wid_set_on_mouse_down(child, wid_editor_buttons_tile_receive_mouse_down);

    x++;

    if (x >= TILES_ACROSS) {
        x = 0;
        y++;
    }

    return (true);
}

/*
 * Create the wid_editor_buttons
 */
void wid_editor_buttons_wid_create (void)
{
    if (wid_editor_buttons_window) {
        return;
    }

    {
        fpoint tl = {0.0f, 0.0f};
        fpoint br = {0.32f, 1.0f};

        wid_editor_buttons_window =
                        wid_new_square_window("wid_editor_buttons");

        wid_set_mode(wid_editor_buttons_window, WID_MODE_NORMAL);

        wid_set_color(wid_editor_buttons_window, WID_COLOR_BG, BLACK);
        wid_set_color(wid_editor_buttons_window, WID_COLOR_TL, BLACK);
        wid_set_color(wid_editor_buttons_window, WID_COLOR_BR, BLACK);
        wid_set_movable(wid_editor_buttons_window, false);

        wid_set_text_advance(wid_editor_buttons_window, 0.9f);
        wid_set_text_scaling(wid_editor_buttons_window, 1.0f);
        wid_set_text_pos(wid_editor_buttons_window, true, 0.5f, 0.10f);
        wid_set_text_outline(wid_editor_buttons_window, true);

        wid_set_on_mouse_down(wid_editor_buttons_window,
                            wid_editor_buttons_receive_mouse_down);

        wid_set_text_bot(wid_editor_buttons_window, true);
        wid_set_text_lhs(wid_editor_buttons_window, true);
        wid_set_tl_br_pct(wid_editor_buttons_window, tl, br);

        fsize sz = {0.0f, 0.0f};
        wid_set_tex_tl(wid_editor_buttons_window, sz);

        fsize sz2 = {1.32f, 1.0f};
        wid_set_tex_br(wid_editor_buttons_window, sz2);
    }

    /*
     * Tiles.
     */
    {
        fpoint tl = {0.02f, 0.00f};
        fpoint br = {0.99f, wid_tile_height * TILES_DOWN};

        wid_editor_buttons_container1 =
                        wid_new_container(wid_editor_buttons_window,
                                          "wid editor button container1");
        wid_set_tl_br_pct(wid_editor_buttons_container1, tl, br);
    }

    /*
     * Recently used tiles.
     */
    {
        fpoint tl = {0.02f, wid_tile_height * TILES_DOWN};
        fpoint br = {0.99f, wid_tile_height * (TILES_DOWN + 1)};

        wid_editor_buttons_container2 =
                        wid_new_container(wid_editor_buttons_window,
                                          "wid editor button container2");
        wid_set_tl_br_pct(wid_editor_buttons_container2, tl, br);
    }

    /*
     * Icons.
     */
    {
        fpoint tl = {0.02f, wid_tile_height * (TILES_DOWN + 1)};
        fpoint br = {0.99f, 0.99f};

        wid_editor_buttons_container3 =
                        wid_new_container(wid_editor_buttons_window,
                                          "wid editor button container3");
        wid_set_tl_br_pct(wid_editor_buttons_container3, tl, br);
    }

    tree_walk(thing_templates_create_order,
              wid_editor_buttons_add_tiles, 0 /* arg */);

    {
        int32_t x;
        int32_t y;

        widp child;

        x = 0;
        y = 0;

        float w = 1.0 / TILES_ACROSS;
        float h = 1.0;

        int32_t i;

        for (i=0;i<TILES_ACROSS;i++) {
            fpoint tl = {
                (w * (float)(x)),
                (h * (float)(y))
            };

            fpoint br = {
                (w * (float)(x+1)),
                (h * (float)(y+1))
            };

            child = wid_new_square_button(wid_editor_buttons_container2,
                                          "palette recent tile");

            wid_set_color(child, WID_COLOR_BG, BLACK);
            wid_set_color(child, WID_COLOR_TEXT, WHITE);
            wid_set_tl_br_pct(child, tl, br);
            wid_set_focusable(child, i+1);
            wid_set_on_mouse_down(child,
                                wid_editor_buttons_recent_tile_receive_mouse_down);

            if (i == 0) {
                wid_set_thing_template(child, first_thing_template);
            }

            x++;
        }
    }

    {
        int32_t x;
        int32_t y;

        widp child;

        x = 0;
        y = 0;

        int32_t i;

        for (i=0; i<12; i++) {

            fpoint tl = {
                (wid_button_width * (float)(x)),
                (wid_button_height * (float)(y))
            };

            fpoint br = {
                (wid_button_width * (float)(x+1)),
                (wid_button_height * (float)(y+1))
            };

            child = wid_new_square_button(wid_editor_buttons_container3,
                                                 "palette icon");
            color c;

            switch (i) {
            case 0:
                wid_set_text(child, "Draw");
                wid_editor_wid_draw = child;

                wid_set_tooltip(child, "Draw single tiles");

                wid_set_on_mouse_down(
                                child,
                                wid_editor_buttons_icon_draw_receive_mouse_down);
                break;

            case 1:
                wid_set_text(child, "Fill");
                wid_editor_wid_fill = child;

                wid_set_tooltip(child, "Fill area with tiles");

                wid_set_on_mouse_down(
                            child,
                            wid_editor_buttons_icon_fill_receive_mouse_down);
                break;

            case 2:
                wid_set_text(child, "Clear");

                wid_set_tooltip(child, "Clear ALL tiles");

                wid_set_on_mouse_down(
                            child,
                            wid_editor_buttons_icon_clear_receive_mouse_down);
                break;

            case 3:
                wid_set_text(child, "Eraser");
                wid_editor_wid_eraser = child;

                wid_set_tooltip(child, "Erase selected tiles");

                wid_set_on_mouse_down(
                            child,
                            wid_editor_buttons_icon_erase_receive_mouse_down);
                break;

            case 4:
                wid_set_text(child, "Undo");
                wid_editor_wid_undo = child;

                wid_set_tooltip(child, "Undo last change");

                wid_set_on_mouse_down(
                            child,
                            wid_editor_buttons_icon_undo_receive_mouse_down);
                break;

            case 5:
                wid_set_text(child, "Redo");
                wid_editor_wid_redo = child;

                wid_set_tooltip(child, "Undo an undo");

                wid_set_on_mouse_down(
                            child,
                            wid_editor_buttons_icon_redo_receive_mouse_down);
                break;

            case 6:
                wid_set_text(child, "Line");
                wid_editor_wid_line = child;

                wid_set_tooltip(child, "Click on the start and then end of a line");

                wid_set_on_mouse_down(
                                child,
                                wid_editor_buttons_icon_line_receive_mouse_down);
                break;

            case 7:
                wid_set_text(child, "Title");

                wid_set_tooltip(child, "Name this level");

                wid_set_on_mouse_down(
                                child,
                                wid_editor_buttons_icon_title_receive_mouse_down);
                break;

            case 8:
                wid_set_text(child, "Save");

                wid_set_tooltip(child, "Save level");

                wid_set_on_mouse_down(
                                child,
                                wid_editor_buttons_icon_save_receive_mouse_down);
                break;

            case 9:
                wid_set_text(child, "Load");

                wid_set_tooltip(child, "Load level");

                wid_set_on_mouse_down(
                                child,
                                wid_editor_buttons_icon_load_receive_mouse_down);
                break;

            case 10:
                wid_set_text(child, "Back");

                wid_set_tooltip(child, "Back to main menu");

                wid_set_on_mouse_down(
                                child,
                                wid_editor_buttons_icon_back_receive_mouse_down);
                break;

            case 11:
                wid_set_text(child, "Help");

                wid_set_tooltip(child,
                  "%%fmt=left$%%fg=white$left,space\t\t\t%%fg=green$Place tile\n"
                  "%%fmt=left$%%fg=white$right,bksp\t\t\t%%fg=green$Erase tile\n"
                  "%%fmt=left$%%fg=white$d\t\t\t\t%%fg=green$Draw mode\n"
                  "%%fmt=left$%%fg=white$e\t\t\t\t%%fg=green$Eraser mode\n"
                  "%%fmt=left$%%fg=white$f\t\t\t\t%%fg=green$Fill mode\n"
                  "%%fmt=left$%%fg=white$n\t\t\t\t%%fg=green$Line mode\n"
                  "%%fmt=left$%%fg=white$u\t\t\t\t%%fg=green$Undo\n"
                  "%%fmt=left$%%fg=white$r\t\t\t\t%%fg=green$Redo\n"
                  "%%fmt=left$%%fg=white$c\t\t\t\t%%fg=green$Clear level\n"
                  "%%fmt=left$%%fg=white$s\t\t\t\t%%fg=green$Save level\n"
                  "%%fmt=left$%%fg=white$l\t\t\t\t%%fg=green$Load level\n"
                  "%%fmt=left$%%fg=white$b,ESC\t\t\t\t%%fg=green$Main menu");

                break;
            }

            wid_set_font(child, small_font);

            wid_set_mode(child, WID_MODE_NORMAL);
            wid_set_color(child, WID_COLOR_BG, BLACK);

            c.a = 250;
            wid_set_mode(child, WID_MODE_OVER);
            wid_set_color(child, WID_COLOR_BG, c);

            c = STEELBLUE;
            c.a = 250;
            wid_set_mode(child, WID_MODE_ACTIVE);
            wid_set_color(child, WID_COLOR_BG, c);

            wid_set_mode(child, WID_MODE_FOCUS);

            wid_set_mode(child, WID_MODE_NORMAL);

            wid_set_tl_br_pct(child, tl, br);

            x++;

            if (x >= ICONS_ACROSS) {
                x = 0;
                y++;
            }
        }
    }

    wid_editor_buttons_vert_scroll =
        wid_new_vert_scroll_bar(wid_editor_buttons_window,
                                wid_editor_buttons_container1);

    wid_visible(wid_get_parent(wid_editor_buttons_vert_scroll), 0);
    wid_visible(wid_editor_buttons_vert_scroll, 0);
    wid_raise(wid_get_parent(wid_editor_buttons_vert_scroll));

    wid_move_to_top(wid_editor_buttons_vert_scroll);
}

void wid_editor_buttons_wid_destroy (void)
{
    wid_destroy(&wid_editor_buttons_window);
}
