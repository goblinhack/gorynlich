/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "tex.h"
#include "color.h"
#include "wid_intro2.h"
#include "wid_intro3.h"
#include "wid_popup.h"
#include "wid_game_map.h"
#include "wid_server_join.h"
#include "wid_server_create.h"
#include "sdl.h"
#include "level.h"
#include "timer.h"
#include "tree.h"
#include "time.h"
#include "thing_template.h"
#include "client.h"

static widp wid_intro2;
static widp wid_intro2_background;
static widp wid_intro_player_container;

static void wid_intro2_play_selected(void);

static boolean wid_intro2_init_done;
static void wid_intro2_create(void);

static int intro_effect_delay = 200;

boolean wid_intro2_init (void)
{
    if (!wid_intro2_init_done) {
    }

    wid_intro2_init_done = true;

    return (true);
}

void wid_intro2_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (wid_intro2_init_done) {
        wid_intro2_init_done = false;

        if (wid_intro2) {
            wid_destroy(&wid_intro2);
            wid_destroy_in(wid_intro2_background, wid_hide_delay * 2);
        }
    }
}

static boolean wid_intro2_is_hidden;
static boolean wid_intro2_is_visible;

void wid_intro2_hide (void)
{
    if (wid_intro2_is_hidden) {
        return;
    }

    wid_intro2_is_hidden = true;
    wid_intro2_is_visible = false;

    if (!wid_intro2) {
        DIE("no wid intro");
    }

    wid_fade_out(wid_intro2_background, intro_effect_delay);

    wid_hide(wid_intro2, 0);
    wid_raise(wid_intro2);
    wid_update(wid_intro2);
}

void wid_intro2_visible (void)
{
    if (wid_intro2_is_visible) {
        return;
    }

    wid_intro2_create();

    wid_intro2_is_visible = true;
    wid_intro2_is_hidden = false;

    if (!wid_intro2) {
        DIE("no wid intro");
    }

    if (game_over) {
        game_over = false;
        return;
    }

    wid_visible(wid_intro2, 0);
    wid_raise(wid_intro2);
    wid_update(wid_intro2);

    wid_fade_in(wid_intro2_background, intro_effect_delay);
}

static boolean wid_intro2_key_event (widp w, const SDL_KEYSYM *key)
{
    switch ((int)key->sym) {
        case ' ':
        case SDLK_RETURN:
            wid_intro2_play_selected();
            return (true);

        default:
            return (true);
    }

    return (false);
}

static void wid_intro2_play_selected_cb (void *context)
{
    wid_intro3_visible();
}

static void wid_intro2_play_selected (void)
{
    action_timer_create(
            &wid_timers,
            (action_timer_callback)wid_intro2_play_selected_cb,
            0, /* context */
            "start game",
            intro_effect_delay,
            0 /* jitter */);

    wid_intro2_hide();
}

static boolean wid_intro2_play_mouse_event (widp w, int32_t x, int32_t y,
                                           uint32_t button)
{
    wid_intro2_play_selected();

    return (true);
}

static boolean wid_intro2_play_key_event (widp w, const SDL_KEYSYM *key)
{
    switch (key->sym) {
        case ' ':
        case SDLK_RETURN:
            wid_intro2_play_selected();
            return (true);

        default:
            break;
    }

    return (false);
}

static void wid_intro2_bg_create (void)
{
    widp wid;

    if (wid_intro2_background) {
        return;
    }

    {
        wid = wid_intro2_background = wid_new_window("bg");

        float f;

        f = (1024.0 / 680.0);

        fpoint tl = { 0.0, 0.0 };
        fpoint br = { 1.0, f };

        wid_set_tl_br_pct(wid, tl, br);

        wid_set_tex(wid, 0, "title2");

        wid_lower(wid);

        color c;
        c = WHITE;
        wid_set_mode(wid, WID_MODE_NORMAL);
        wid_set_color(wid, WID_COLOR_TL, c);
        wid_set_color(wid, WID_COLOR_BR, c);
        wid_set_color(wid, WID_COLOR_BG, c);

        wid_update(wid);
    }
}

static thing_templatep first_thing_template;
static const int32_t TILES_ACROSS = 2;
static const int32_t TILES_DOWN = 3;

static void wid_intro2_buttons_tick (widp wid)
{
    int tick = time_get_time_milli() / 100;
    int which = tick % 8;

    thing_templatep t = wid_get_thing_template(wid);
    const char *tn = thing_template_shortname(t);

    char tilename[20];

    switch (which) {
    case 0: snprintf(tilename, sizeof(tilename) - 1, "%s-right", tn); break;
    case 1: snprintf(tilename, sizeof(tilename) - 1, "%s-br", tn); break;
    case 2: snprintf(tilename, sizeof(tilename) - 1, "%s-down", tn); break;
    case 3: snprintf(tilename, sizeof(tilename) - 1, "%s-bl", tn); break;
    case 4: snprintf(tilename, sizeof(tilename) - 1, "%s-left", tn); break;
    case 5: snprintf(tilename, sizeof(tilename) - 1, "%s-tl", tn); break;
    case 6: snprintf(tilename, sizeof(tilename) - 1, "%s-up", tn); break;
    case 7: snprintf(tilename, sizeof(tilename) - 1, "%s-tr", tn); break;
    }

    wid_set_tilename(wid, tilename);
}

static boolean wid_intro2_buttons_add_tiles (const tree_node *node, void *arg)
{
    static int32_t x;
    static int32_t y;
    thing_templatep thing_template;
    widp child;

    thing_template = (typeof(thing_template)) 
            (((char*) node) - STRUCT_OFFSET(struct thing_template_, tree2));

    if (!thing_template_is_player(thing_template)) {
        return (true);
    }

    if (!first_thing_template) {
        first_thing_template = thing_template;
    }

    float w = 0.36;
    float h = 0.32;

    fpoint tl = {
        (w * (float)(x)),
        (h * (float)(y))
    };

    fpoint br = {
        (w * (float)(x+1)),
        (h * (float)(y+1))
    };

    br.x -= 0.04;
    br.y -= 0.04;

    child = wid_new_square_button(wid_intro_player_container,
                                  "possible players");

    wid_set_color(child, WID_COLOR_BG, BLACK);
    wid_set_color(child, WID_COLOR_TEXT, WHITE);
    wid_set_color(child, WID_COLOR_BR, PINK);
    wid_set_color(child, WID_COLOR_BG, BLACK);

    if (!strcmp(thing_template_shortname(thing_template), 
                global_config.pclass)) {
        wid_set_color(child, WID_COLOR_TL, RED);
    }

    wid_set_mode(child, WID_MODE_OVER);
    wid_set_color(child, WID_COLOR_BG, STEELBLUE);

    wid_set_mode(child, WID_MODE_NORMAL);

    wid_set_thing_template(child, thing_template);
    wid_set_tooltip(child, thing_template_get_tooltip(thing_template));
    wid_set_tl_br_pct(child, tl, br);
    wid_set_on_tick(child, wid_intro2_buttons_tick);

    x++;

    if (x >= TILES_ACROSS) {
        x = 0;
        y++;
    }

    return (true);
}

/*
 * Key down etc...
 */
static boolean wid_intro2_name_receive_input (widp w, const SDL_KEYSYM *key)
{
    switch (key->sym) {
        case SDLK_RETURN: {
            /*
             * Change name.
             */
            wid_set_show_cursor(w, false);

            char *name = (char*) wid_get_text(w);

            client_socket_set_name(name);
            break;
        }

        default:
            break;
    }

    /*
     * Feed to the general input handler
     */
    return (wid_receive_input(w, key));
}

static void wid_intro2_create (void)
{
    if (wid_intro2) {
        return;
    }

    wid_intro2 = wid_new_window("intro buttons");

    wid_set_no_shape(wid_intro2);

    fpoint tl = {0.0f, 0.0f};
    fpoint br = {1.0f, 1.0f};
    wid_set_tl_br_pct(wid_intro2, tl, br);
    wid_set_on_key_down(wid_intro2, wid_intro2_key_event);

    color col = BLACK;
    col.a = 0;
    glcolor(col);

    wid_set_mode(wid_intro2, WID_MODE_NORMAL);
    wid_set_color(wid_intro2, WID_COLOR_TL, col);
    wid_set_color(wid_intro2, WID_COLOR_BR, col);
    wid_set_color(wid_intro2, WID_COLOR_BG, col);

    {
        widp child;

        child = wid_new_square_button(wid_intro2, "play");
        wid_set_font(child, med_font);
        wid_set_no_shape(child);

        fpoint tl = {0.2f, 0.70f};
        fpoint br = {0.8f, 0.90f};

        wid_set_tl_br_pct(child, tl, br);
        wid_set_text(child, "Choose player");
        wid_fade_in_out(child, 1000, 1000, false /* fade out first */);

        wid_set_color(child, WID_COLOR_TEXT, WHITE);
        color c = WHITE;
        c.a = 200;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_OVER);
        c.a = 200;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_FOCUS);
        c.a = 100;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_on_mouse_down(child, wid_intro2_play_mouse_event);
        wid_set_on_key_down(child, wid_intro2_play_key_event);
        wid_set_text_outline(child, true);
    }

    {
        wid_intro_player_container = wid_new_window("players");

        fpoint tl = {0.75f, 0.43f};
        fpoint br = {1.0, 1.0f};
        wid_set_tl_br_pct(wid_intro_player_container, tl, br);

        color col = BLACK;
        col.a = 0;
        glcolor(col);

        wid_set_mode(wid_intro_player_container, WID_MODE_NORMAL);
        wid_set_color(wid_intro_player_container, WID_COLOR_TL, col);
        wid_set_color(wid_intro_player_container, WID_COLOR_BR, col);
        wid_set_color(wid_intro_player_container, WID_COLOR_BG, col);

        tree_walk(thing_templates_create_order,
                  wid_intro2_buttons_add_tiles, 0 /* arg */);
    }

    {
        fpoint tl = {0.4, 0.45};
        fpoint br = {0.5, 0.5};

        widp w = wid_new_container(wid_intro2, "wid intro name container");

        wid_set_tl_br_pct(w, tl, br);

        wid_set_text(w, "name");
        wid_set_font(w, small_font);
        wid_set_no_shape(w);

        wid_set_color(w, WID_COLOR_BG, BLACK);
        wid_set_color(w, WID_COLOR_TL, STEELBLUE);
        wid_set_color(w, WID_COLOR_BR, STEELBLUE);
        wid_set_text_outline(w, true);
    }

    {
        fpoint tl = {0.5, 0.45};
        fpoint br = {0.7, 0.5};

        widp w = wid_new_container(wid_intro2, "wid intro name value");

        wid_set_tl_br_pct(w, tl, br);

        wid_set_text(w, global_config.name);
        wid_set_font(w, small_font);

        wid_set_color(w, WID_COLOR_BG, BLACK);
        wid_set_color(w, WID_COLOR_TL, STEELBLUE);
        wid_set_color(w, WID_COLOR_BR, STEELBLUE);
        wid_set_square(w);
        wid_set_bevelled(w, true);
        wid_set_bevel(w, 2);
        wid_set_text_outline(w, true);

        wid_set_show_cursor(w, true);
        wid_set_on_key_down(w, wid_intro2_name_receive_input);
    }

    {
        fpoint tl = {0.4, 0.51};
        fpoint br = {0.5, 0.56};

        widp w = wid_new_container(wid_intro2, "wid intro pclass container");

        wid_set_tl_br_pct(w, tl, br);

        wid_set_text(w, "class");
        wid_set_font(w, small_font);
        wid_set_no_shape(w);

        wid_set_color(w, WID_COLOR_BG, BLACK);
        wid_set_color(w, WID_COLOR_TL, STEELBLUE);
        wid_set_color(w, WID_COLOR_BR, STEELBLUE);
        wid_set_text_outline(w, true);
    }

    {
        fpoint tl = {0.5, 0.51};
        fpoint br = {0.7, 0.56};

        widp w = wid_new_container(wid_intro2, "wid intro pclass value");

        wid_set_tl_br_pct(w, tl, br);

        wid_set_text(w, global_config.pclass);
        wid_set_font(w, small_font);
        wid_set_no_shape(w);

        wid_set_color(w, WID_COLOR_BG, BLACK);
        wid_set_color(w, WID_COLOR_TL, STEELBLUE);
        wid_set_color(w, WID_COLOR_BR, STEELBLUE);
        wid_set_text_outline(w, true);
    }

    wid_update(wid_intro_player_container);

    wid_intro2_bg_create();
    wid_update(wid_intro2);

    wid_move_to_pct_centered(wid_intro2, 0.5f, 0.5f);
}
