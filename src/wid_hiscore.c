/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "color.h"
#include "wid_hiscore.h"
#include "string.h"
#include "wid_text_input.h"
#include "marshal.h"
#include "wid_popup.h"

static const char *hiscore_dir_and_file = "gorynlich-hiscore.txt";

static widp wid_hiscore;
static widp wid_hiscore_container;
static uint8_t wid_hiscore_init_done;

static void wid_hiscore_create(void);
static void wid_hiscore_destroy(void);

tree_rootp hiscores;

void hiscore_add (const char *player_name, 
                  const char *death_reason, 
                  uint32_t score)
{
    static uint32_t tiebreak;
    hiscore *h;

    if (!hiscores) {
        hiscores = tree_alloc(TREE_KEY_TWO_INTEGER, "TREE ROOT: hiscore");
    }

    h = (typeof(h)) myzalloc(sizeof(*h), "TREE NODE: hiscore");

    h->player_name = dupstr(player_name, "player name");
    h->death_reason = dupstr(death_reason, "death reason");

    h->tree.key2 = score;
    h->tree.key3 = tiebreak++;

    while (!tree_insert(hiscores, &h->tree.node)) {
        h->tree.key3 = tiebreak++;
    }
}

uint8_t wid_hiscore_init (void)
{
    if (!wid_hiscore_init_done) {
        hiscore_load();
    }

    wid_hiscore_init_done = true;

    return (true);
}

static void hiscore_destroy (hiscore *node)
{
    myfree(node->player_name);
    myfree(node->death_reason);
}

void wid_hiscore_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (wid_hiscore_init_done) {
        wid_hiscore_init_done = false;

        wid_hiscore_destroy();

        tree_destroy(&hiscores, (tree_destroy_func)hiscore_destroy);
    }
}

void wid_hiscore_hide (void)
{
    wid_hiscore_destroy();
}

void wid_hiscore_visible (void)
{
    wid_hiscore_create();
}

static uint8_t wid_hiscore_mouse_event (widp w, int32_t x, int32_t y,
                                        uint32_t button)
{
    wid_hiscore_hide();

    return (true);
}

static uint8_t wid_hiscore_key_event (widp w, const SDL_KEYSYM *key)
{
    switch (key->sym) {
        case 'q':
        case SDLK_ESCAPE:
            wid_hiscore_hide();
            return (true);

        default:
            break;
    }

    return (false);
}

static uint8_t wid_hiscore_receive_mouse_motion (
                    widp w,
                    int32_t x, int32_t y,
                    int32_t relx, int32_t rely,
                    int32_t wheelx, int32_t wheely)
{
    if (wheelx || wheely) {
        /*
         * Allow scrolling.
         */
        return (false);
    }

    /*
     * Block moving the window.
     */
    return (true);
}

static void wid_hiscore_create (void)
{
    if (wid_hiscore) {
        return;
    }

    widp w = wid_hiscore = wid_new_rounded_window("wid settings");

    fpoint tl = {0.05, 0.1};
    fpoint br = {0.95, 0.9};

    wid_set_tl_br_pct(w, tl, br);
    wid_set_font(w, med_font);

    wid_set_color(w, WID_COLOR_TEXT, WHITE);

    color c = BLACK;
    c.a = 200;
    wid_set_color(w, WID_COLOR_BG, c);

    c = STEELBLUE;
    c.a = 200;
    wid_set_color(w, WID_COLOR_TL, c);
    wid_set_color(w, WID_COLOR_BR, c);
    wid_set_bevel(w, 4);
    wid_set_on_mouse_up(w, wid_hiscore_mouse_event);
    wid_set_on_key_down(w, wid_hiscore_key_event);

    wid_set_on_mouse_motion(w, wid_hiscore_receive_mouse_motion);

    {
        widp w = wid_hiscore_container =
            wid_new_container(wid_hiscore, "wid settings container");

        fpoint tl = {0.0, 0.0};
        fpoint br = {1.0, 1.0};

        wid_set_tl_br_pct(w, tl, br);
    }

    {
        fpoint tl = {0.0, 0.0};
        fpoint br = {1.0, 0.1};

        w = wid_new_container(wid_hiscore_container, "wid hiscrore title");

        wid_set_tl_br_pct(w, tl, br);

        wid_set_text(w, "High Scores");
        wid_set_font(w, large_font);
        wid_set_color(w, WID_COLOR_TEXT, STEELBLUE);

        wid_set_text_outline(w, true);
    }

    {
        uint32_t i = 0;
        hiscore *h;

        TREE_WALK_REVERSE(hiscores, h) {
            widp w = wid_new_square_button(wid_hiscore_container,
                                           "hiscore name");

            fpoint tl = {0.05, 0.1};
            fpoint br = {0.51, 0.2};

            float height = 0.08;

            if (i < 1) {
                wid_set_color(w, WID_COLOR_TEXT, YELLOW);
            } else if (i < 4) {
                wid_set_color(w, WID_COLOR_TEXT, GREEN);
            } else {
                wid_set_color(w, WID_COLOR_TEXT, SKYBLUE);
            }

            br.y += (float)i * height;
            tl.y += (float)i * height;

            wid_set_tl_br_pct(w, tl, br);

            wid_set_text(w, h->player_name);

            color c = BLACK;

            c.a = 100;
            wid_set_mode(w, WID_MODE_NORMAL);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_OVER);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_NORMAL);

            wid_set_bevel(w,0);
            wid_set_no_shape(w);
            wid_set_text_outline(w, true);
            wid_set_font(w, large_font);
            wid_set_text_lhs(w, true);

            i++;
        }
    }

    {
        uint32_t i = 0;
        hiscore *h;

        TREE_WALK_REVERSE(hiscores, h) {
            widp w = wid_new_square_button(wid_hiscore_container,
                                           "hiscore killed by");

            fpoint tl = {0.3, 0.1};
            fpoint br = {0.8, 0.2};

            float height = 0.08;

            if (i < 1) {
                wid_set_color(w, WID_COLOR_TEXT, YELLOW);
            } else if (i < 4) {
                wid_set_color(w, WID_COLOR_TEXT, GREEN);
            } else {
                wid_set_color(w, WID_COLOR_TEXT, SKYBLUE);
            }

            br.y += (float)i * height;
            tl.y += (float)i * height;

            wid_set_tl_br_pct(w, tl, br);

            if (h->death_reason && h->death_reason[0]) {
                char *tmp = dynprintf("killed by %s", h->death_reason);
                wid_set_text(w, tmp);
                myfree(tmp);
            }

            color c = BLACK;

            c.a = 100;
            wid_set_mode(w, WID_MODE_NORMAL);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_OVER);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_NORMAL);

            wid_set_bevel(w,0);
            wid_set_no_shape(w);
            wid_set_text_outline(w, true);
            wid_set_font(w, vsmall_font);
            wid_set_text_lhs(w, true);

            i++;
        }
    }

    {
        uint32_t i = 0;
        hiscore *h;

        TREE_WALK_REVERSE(hiscores, h) {
            widp w = wid_new_square_button(wid_hiscore_container,
                                           "hiscore value");

            fpoint tl = {0.52, 0.1};
            fpoint br = {0.95, 0.2};

            float height = 0.08;

            if (i < 1) {
                wid_set_color(w, WID_COLOR_TEXT, YELLOW);
            } else if (i < 4) {
                wid_set_color(w, WID_COLOR_TEXT, GREEN);
            } else {
                wid_set_color(w, WID_COLOR_TEXT, SKYBLUE);
            }

            br.y += (float)i * height;
            tl.y += (float)i * height;

            wid_set_tl_br_pct(w, tl, br);

            char *tmp = dynprintf("%08d", h->tree.key2);
            wid_set_text(w, tmp);
            myfree(tmp);

            color c = BLUE;

            c.a = 100;
            wid_set_mode(w, WID_MODE_NORMAL);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_OVER);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_NORMAL);

            wid_set_bevel(w,0);
            wid_set_no_shape(w);
            wid_set_text_outline(w, true);
            wid_set_text_fixed_width(w, true);
            wid_set_font(w, large_font);

            i++;
        }
    }

    wid_raise(wid_hiscore);

    wid_update(wid_hiscore);
}

void wid_hiscore_destroy (void)
{
    wid_destroy(&wid_hiscore);
}

static uint8_t demarshal_hiscore (demarshal_p ctx, hiscore *p)
{
    uint8_t rc;

    rc = true;

    rc = rc && GET_OPT_NAMED_STRING(ctx, "name", p->player_name);
    rc = rc && GET_OPT_NAMED_STRING(ctx, "death_reason", p->death_reason);
    rc = rc && GET_OPT_NAMED_INT32(ctx, "score", p->tree.key2);
    rc = rc && GET_OPT_NAMED_INT32(ctx, "tiebreak", p->tree.key3);

    return (rc);
}

static void marshal_hiscore (marshal_p ctx, hiscore *p)
{
    PUT_NAMED_STRING(ctx, "name", p->player_name);
    PUT_NAMED_STRING(ctx, "death_reason", p->death_reason);
    PUT_NAMED_INT32(ctx, "score", p->tree.key2);
    PUT_NAMED_INT32(ctx, "tiebreak", p->tree.key3);
}

uint8_t hiscore_save (void)
{
    char *file = dynprintf("%s", hiscore_dir_and_file);
    marshal_p ctx;

    ctx = marshal(file);
    if (!ctx) {
        MSGERR("Failed to save: %s", file);
        myfree(file);
        return (false);
    }

    uint32_t count = 0;
    hiscore *h;

    TREE_WALK_REVERSE(hiscores, h) {
        marshal_hiscore(ctx, h);

        if (count++ >= MAX_HISCORES - 1) {
            break;
        }
    }

    if (marshal_fini(ctx) < 0) {
        ERR("Failed to finalize: %s", file);
        myfree(file);

        return (false);
    }

    myfree(file);

    return (true);
}

uint8_t hiscore_load (void)
{
    char *file = dynprintf("%s", hiscore_dir_and_file);
    demarshal_p ctx;

    if (hiscores) {
        tree_destroy(&hiscores, 0);
    }

    DBG("Load %s", file);

    uint32_t count = 0;
    hiscore h;

    if ((ctx = demarshal(file))) {
        while (demarshal_hiscore(ctx, &h)) {
            hiscore_add(h.player_name, h.death_reason, h.tree.key2);
            myfree(h.player_name);
            myfree(h.death_reason);

            if (count++ > MAX_HISCORES) {
                break;
            }
        }

        demarshal_fini(ctx);
    }

    while (count++ < MAX_HISCORES - 1) {
        hiscore_add("No one", "", 0);
    }

    myfree(file);

    return (true);
}

static widp wid_hiscore_name_popup;
static uint32_t score;

void hiscore_try_to_add (const char *player_name,
                         const char *death_reason,
                         uint32_t score_in)
{
    uint32_t count = 0;

    score = score_in;
    hiscore *h;

    if (!score_in) {
        return;
    }

    TREE_WALK_REVERSE(hiscores, h) {

        if (count >= MAX_HISCORES) {
            return;
        }

        if (score_in > (uint32_t) h->tree.key2) {
            break;
        }

        count++;
    }
    const char *which[] = {
        "First",
        "Second",
        "Third",
        "Fourth",
        "Fifth",
        "Sixth",
        "Seventh",
        "Eighth",
        "Nineth",
        "Tenth",
    };

    if (count >= ARRAY_SIZE(which)) {
        return;
    }

    char *place_str = dynprintf("New Hi Score, %s place!", which[count]);

    LOG("%s", place_str);

    hiscore_add(player_name, death_reason, score);

    myfree(place_str);
}
