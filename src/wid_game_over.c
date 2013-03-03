/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "color.h"
#include "wid_intro.h"
#include "sdl.h"
#include "thing_template.h"
#include "wid_popup.h"
#include "wid_game_over.h"

static widp wid_game_over;
static widp wid_game_over_credits;
static boolean wid_game_over_init_done;
static void wid_game_over_create(void);
static void wid_game_over_destroy(void);
static void wid_game_over_finished(widp wid);

boolean wid_game_over_init (void)
{
    if (!wid_game_over_init_done) {
    }

    wid_game_over_init_done = true;

    return (true);
}

void wid_game_over_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (wid_game_over_init_done) {
        wid_game_over_init_done = false;

        wid_game_over_destroy();
    }
}

void wid_game_over_hide (void)
{
    wid_game_over_destroy();
}

void wid_game_over_visible (void)
{
    wid_game_over_create();
}

static void wid_game_over_destroy (void)
{
    wid_destroy(&wid_game_over);
    wid_destroy(&wid_game_over_credits);
}

static void wid_game_over_finished (widp wid)
{
    if (sdl_is_exiting()) {
        return;
    }

    wid_game_over_hide();

    wid_intro_visible();
}

void wid_game_over_create (void)
{
    int32_t i;

    if (sdl_is_exiting()) {
        return;
    }

    widp w = wid_new_window("guide1");
    fpoint tl = { 0, 0 };
    fpoint br = { 1, 1 };

    wid_set_tl_br_pct(w, tl, br);

    wid_set_mode(w, WID_MODE_NORMAL);
    wid_set_color(w, WID_COLOR_TL, BLACK);
    wid_set_color(w, WID_COLOR_BR, BLACK);
    wid_set_color(w, WID_COLOR_BG, BLACK);

    wid_destroy_in(w, 110000);
    wid_set_on_destroy(w, wid_game_over_finished);
    wid_move_to_pct_centered(w, 0.5f, 0.5f);

    for (i = 0; i < 500; i++) {
        fpoint tl;
        fpoint br;

        tl.x = 0.01 * ((float)(rand() % 100));
        tl.y = 0.01 * ((float)(rand() % 100));

        br.x = tl.x + 0.1;
        br.y = tl.y + 0.08;

        float dy = (float)(rand() % 100) / 10.0;
        tl.y -= dy;
        br.y -= dy;

        widp child = wid_new_square_button(w, "star");

        wid_set_tl_br_pct(child, tl, br);
        wid_set_mode(child, WID_MODE_NORMAL);
        wid_set_color(child, WID_COLOR_TEXT, RED);
        wid_set_color(child, WID_COLOR_TL, BLACK);
        wid_set_color(child, WID_COLOR_BG, BLACK);
        wid_set_color(child, WID_COLOR_BR, BLACK);
        wid_set_no_shape(child);

        wid_rotate_to_pct_in(child,
                             0.0, rand() % 1000,
                             (rand() % 5000) + 1000, 999);

        thing_templatep thing_template;
        
        if ((rand() % 10) < 8) {
            thing_template = thing_template_find("data/things/star_yellow");
        } else {
            thing_template = thing_template_find("data/things/player");
        }

        wid_move_to_pct_in(child, tl.x, br.y, 10000);

        wid_set_thing_template(child, thing_template);
    }

    wid_game_over_credits = wid_popup(
          "Game Over!\n"
          "\n"
          "\n"
          "Congratulations!\n"
          "\n"
          "\n"
          "\n"
          "You have proven that\n"
          "in this modern age\n"
          "there is still\n"
          "a need for old\n"
          "fashioned\n"
          "mail delivery\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "You have proven\n"
          "yourself mighty among\n"
          "snails. Now go eat\n"
          "some lettuce.\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "Or send a letter\n"
          "to your mom or mum\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "This game is dedicated\n"
          "to my mum\n"
          "and yours\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "For Trish\n"
          "For Charlotte\n"
          "For Ben\n"
          "For SuperSnail\n"
          "For Stuart\n"
          "For Lovely Angie\n"
          "For Baby Jesus\n"
          "For Jason Isaacs\n"
          "For heck, even you\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "This game was written badly\n"
          "by Neil McGill\n"
          "\n"
          "\n"
          "Sorry\n"
          "\n"
          "\n"
          "Hope you liked it.\n"
          "\n"
          "\n"
          "I know the end is a bit naff\n"
          "\n"
          "Oh well\n"
          "\n"
          "Cant get the staff\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "Now go off and be nice to someone\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "Bye!\n"
          ,
          0,
          0.5, 0.5,                 /* x,y postition in percent */
          large_font,               /* title font */
          large_font,               /* body font */
          med_font,                 /* button font */
          0);

    wid_move_to_pct_centered(wid_game_over_credits, 0.5, 5.5);
    wid_move_to_pct_centered_in(wid_game_over_credits, 0.5, -2.5, 100000);

    wid_raise(w);
    wid_raise(wid_game_over_credits);

    wid_update(w);

}
