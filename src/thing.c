/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <SDL.h>

#include "main.h"
#include "tree.h"
#include "thing_private.h"
#include "thing.h"
#include "thing_template.h"
#include "thing_timer.h"
#include "wid.h"
#include "marshal.h"
#include "level.h"
#include "time.h"
#include "string.h"
#include "wid_textbox.h"
#include "wid_game_map.h"
#include "color.h"
#include "config.h"
#include "sound.h"
#include "map.h"

tree_root *things;
static uint32_t thing_id;
static boolean thing_init_done;
static void thing_destroy_internal(thingp t, const char *why);
static void thing_destroy_internal2(thingp t);

boolean thing_init (void)
{
    thing_init_done = true;

    return (true);
}

void thing_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (thing_init_done) {
        thing_init_done = false;

        tree_destroy(&things, (tree_destroy_func)thing_destroy_internal2);
    }
}

/*
 * Create a new thing.
 */
thingp thing_new (levelp level, const char *name)
{
    thingp t;
    thing_templatep thing_template;

    thing_template = thing_template_find(name);
    if (!thing_template) {
        DIE("thing [%s] has no template", name);
    }

    if (!things) {
        things = tree_alloc(TREE_KEY_INTEGER, "TREE ROOT: thing");
    }

    t = (typeof(t)) myzalloc(sizeof(*t), "TREE NODE: thing");
    t->tree.key = ++thing_id;
    t->thing_template = thing_template;

    if (!tree_insert(things, &t->tree.node)) {
        DIE("thing insert name [%s] failed", name);
    }

    if (level) {
        thing_set_level(t, level);
    }

    t->logname = dynprintf("%s[%p]", thing_shortname(t), t);

    THING_LOG(t, "created");

    return (t);
}

void thing_restarted (thingp t, levelp level)
{
    verify(t);

    thing_set_level(t, level);

    t->current_tile = 0;

    memset(t->visited, 0, sizeof(t->visited));

    if (!thing_is_dead(t) || !thing_is_buried(t)) {
        return;
    }

    if (t->dead_reason) {
        myfree(t->dead_reason);
        t->dead_reason = 0;
    }

    thing_set_is_dead(t, false);
    thing_set_is_buried(t, false);

    /*
     * Reset last map position.
     */
    t->last_x = 0;
    t->last_y = 0;

    /*
     * Record this thing opened the exit.
     */
    thing_set_opened_exit(t, false);
    thing_set_got_to_exit_first(t, false);
}

static void thing_destroy_internal2 (thingp t)
{
    thing_destroy_internal(t, "end of game");
}

static void thing_destroy_internal (thingp t, const char *why)
{
    thingp owner;

    THING_LOG(t, "destroy (%s)", why);
 
    /*
     * Stop all timers.
     */
    thing_timers_destroy(t);

    owner = t->item_owner;
    if (owner) {
        t->item_owner = 0;
        thing_item_drop(owner, t, true /* destroy */,
                        "destroyed whilst carried");
    }

    if (t->dead_reason) {
        myfree(t->dead_reason);
        t->dead_reason = 0;
    }

    thing_item_drop_all(t);

    if (t->wid) {
        thing_set_wid(t, 0);
    }

    if (t->logname) {
        myfree(t->logname);
        t->logname = 0;
    }
}

void thing_destroy (thingp t, const char *why)
{
    verify(t);

    if (!tree_remove(things, &t->tree.node)) {
        DIE("thing template destroy name [%s] failed", thing_name(t));
    }

    thing_destroy_internal(t, why);

    if (t == player) {
        player = 0;
    }

    myfree(t);
}

void thing_bury (thingp t)
{
    verify(t);

    if (!thing_is_dead(t)) {
        DIE("can't bury %s if not dead!", thing_logname(t));
        return;
    }

    if (thing_is_buried(t)) {
        return;
    }

    thing_set_is_buried(t, true);

    if (t == player) {
        if (t->lives) {
            THING_LOG(t, "buried and resurrected");
            level_restart(t->level);
        } else {
            THING_LOG(t, "buried, dust and bones");
            level_destroy(t->level);
        }

        sound_play_dead();
    } else {
        THING_LOG(t, "buried");
    }
}

static void thing_dead_ (thingp t, thingp killer, char *reason)
{
    verify(t);

    if (t->is_dead) {
        return;
    }

    thing_set_is_dead(t, true);

    /*
     * Bounty for the killer?
     */
    if (killer) {
        uint32_t score = thing_template_get_score_on_death(
                                                thing_get_template(t));

        if (score) {
            thing_inc_score_pump(killer, score);
        }
    }

    /*
     * Any last bonuses still being pumped?
     */
    uint32_t score_pump = thing_score_pump(t);

    if (score_pump) {
        thing_set_score(t, thing_score(t) + score_pump);
        thing_set_score_pump(t, 0);
    }

    /*
     * Stop bouncing or swaying.
     */
    if (t->wid) {
        if (thing_template_is_effect_pulse(t->thing_template)) {
            wid_scaling_to_pct_in(t->wid, 1.0, 1.0, 0, 0);
        }

        if (thing_template_is_effect_sway(t->thing_template)) {
            wid_rotate_to_pct_in(t->wid, 0, 0, 0, 0);
        }
    }

    /*
     * Stop moving.
     */
    thing_stop(t);

    /*
     * Why did I die!? 8(
     */
    if (t->dead_reason) {
        myfree(t->dead_reason);
        t->dead_reason = 0;
    }
    
    if (reason) {
        t->dead_reason = reason;
    }

    if (t->lives) {
        t->lives--;
    }

    /*
     * Get rid of end of life items. You can't take it with you when you go!
     * Unless you have a spare life.
     */
    thing_item_drop_end_of_life(t);

    if (!thing_is_left_as_corpse_on_death(t)) {
        /*
         * Pop from the level.
         */
        thing_set_wid(t, 0);
    }

    if (t->lives) {
        THING_LOG(t, "dead (%s) lives (%u) remaining", reason, t->lives);
    } else {
        THING_LOG(t, "dead (%s)", reason);
    }
}

void thing_dead (thingp t, thingp killer, const char *reason, ...)
{
    verify(t);

    va_list args;

    verify(t);

    if (god_mode) {
        if (thing_is_player(t)) {
            return;
        }
    }

    if (t->is_dead) {
        return;
    }

    if (reason) {
        va_start(args, reason);
        thing_dead_(t, killer, dynvprintf(reason, args));
        va_end(args);
    } else {
        thing_dead_(t, killer, 0);
    }
}

void thing_reached_exit (thingp t)
{
    levelp level;

    verify(t);

    level = thing_level(t);
    verify(level);

    /*
     * First to the exit?
     */
    if (!level_exit_reached_when_open(level)) {
        level_set_exit_reached_when_open(level, true);

        thing_set_got_to_exit_first(t, true);

        THING_LOG(t, "reached exit first");

        thing_inc_score_pump(t, ONESEC);
    } else {
        THING_LOG(t, "reached exit");

        thing_inc_score_pump(t, 100);
    }

    /*
     * Pop from the level.
     */
    thing_set_wid(t, 0);

    /*
     * Drop powerups.
     */
    thing_item_drop_end_of_level(t);

    /*
     * And all possessions.
     */
    thing_item_pop_all(t);

    if (t == player) {
        thing_set_level_no(t, thing_level_no(t) + 1);

        level_completed(t->level);

        sound_play_doorbell();
    }

    sound_play_level_end();
}

void things_level_start (levelp level)
{
    thingp t;

    TREE_WALK(things, t) {
        verify(t);

        if (t->wid) {
            verify(t->wid);
        }

        if (t->level) {
            continue;
        }

        thing_set_level(t, level);
    }
}

void things_level_destroyed (levelp level)
{
    thingp t;

    TREE_WALK(things, t) {
        verify(t);

        if (t->wid) {
            verify(t->wid);
        }

        THING_LOG(t, "level destroyed");

        /*
         * Let the owner destroy it.
         */
        if (thing_item_owner(t)) {
            thing_set_level(t, 0);
            continue;
        }

        thing_destroy(t, "level destroyed");
    }
}

void things_level_restarted (levelp level)
{
    thingp t;

    TREE_WALK(things, t) {
        verify(t);

        if (t->wid) {
            verify(t->wid);
        }

        if (thing_template_is_player(t->thing_template)) {
            THING_LOG(t, "level restarted, keep player");
            thing_set_level(t, 0);
            continue;
        }

        /*
         * Let the owner destroy it.
         */
        if (thing_item_owner(t)) {
            thing_set_level(t, 0);
            continue;
        }

        thing_destroy(t, "level restarted");
    }
}

void thing_stop (thingp t)
{
    verify(t);

    if (t->wid) {
        verify(t->wid);
    }

    if (t->wid) {
        wid_move_stop(t->wid);
    }
}

void thing_resume (thingp t)
{
    verify(t);

    if (t->wid) {
        verify(t->wid);
    }

    if (t->wid) {
        wid_move_resume(t->wid);
    }
}

void things_stop (levelp level)
{
    thingp t;

    TREE_WALK(things, t) {
        verify(t);

        thing_stop(t);
    }
}

void things_stop_all_except (levelp level, thingp o)
{
    thingp t;

    TREE_WALK(things, t) {
        verify(t);

        if (t == o) {
            continue;
        }

        thing_stop(t);
    }
}

void things_marshal (marshal_p out)
{
    tree_root *tree;
    thingp t;

    tree = things;

    TREE_WALK(tree, t) {
        verify(t);

        if (t->wid) {
            verify(t->wid);
        }

        marshal_thing(out, t);

        {
            marshal_p tmp;
            tmp = marshal(0); /* stdout */
            marshal_thing(tmp, t);
            marshal_fini(tmp);
        }
    }
}

void demarshal_thing (demarshal_p ctx, thingp t)
{
    if (!ctx) {
        ERR("no thing to demarshal");
    }

    char *name;

    GET_OPT_DEF_NAMED_STRING(ctx, "name", name, "<no name>");

    myfree(name);
}

void marshal_thing (marshal_p ctx, thingp t)
{
    if (t->tree.key) {
        PUT_NAMED_STRING(ctx,  "name", thing_name(t));
    }
}

boolean thing_test (int32_t argc, char *argv[])
{
    marshal_p out;

    out = marshal("things_test.data");

    thing_templates_marshal(out);

    marshal_fini(out);

    return (0);
}

thing_templatep thing_get_template (thingp t)
{
    verify(t);

    return (t->thing_template);
}

const char *thing_logname (thingp t)
{
    verify(t);

    return (t->logname);
}

const char *thing_shortname (thingp t)
{
    verify(t);

    return (thing_template_shortname(t->thing_template));
}

const char *thing_dead_reason (thingp t)
{
    verify(t);

    return (t->dead_reason);
}

uint8_t thing_z_depth (thingp t)
{
    verify(t);

    return (thing_template_get_z_depth(t->thing_template));
}

uint8_t thing_z_order (thingp t)
{
    verify(t);

    return (thing_template_get_z_order(t->thing_template));
}

uint32_t thing_level_no (thingp t)
{
    verify(t);

    return (t->level_no);
}

void thing_set_level_no (thingp t, uint32_t level)
{
    verify(t);

    t->level_no = level;
}

levelp thing_level (thingp t)
{
    verify(t);

    return (t->level);
}

void thing_set_level (thingp t, levelp level)
{
    verify(t);

    t->level = level;
}

uint32_t thing_lives (thingp t)
{
    verify(t);

    return (t->lives);
}

void thing_set_lives (thingp t, uint32_t lives)
{
    verify(t);

    t->lives = lives;
}

uint32_t thing_score (thingp t)
{
    verify(t);

    return (t->score);
}

void thing_set_score (thingp t, uint32_t score)
{
    verify(t);

    t->score = score;
}

uint32_t thing_score_pump (thingp t)
{
    verify(t);

    return (t->score_pump);
}

widp thing_message (thingp t, const char *message)
{
    verify(t);

    if (!thing_is_player(t) && !thing_is_esnail(t) && !thing_is_car(t)) {
        return (0);
    }
    
    if (!t->wid) {
        return (0);
    }

    widp w = wid_textbox(wid_game_map_window,
                         &wid_score, message, 0.5, 0.03, med_font);

    wid_set_no_shape(w);
    wid_set_color(wid_score, WID_COLOR_TEXT, YELLOW);

    /*
     * Move the text over the thing.
     */
    float mx, my;
    wid_get_mxy(t->wid, &mx, &my);

    uint32_t lifespan = ONESEC * 3;

    /*
     * Fade out.
     */
    wid_fade_out(w, lifespan);

    /*
     * Self destroy.
     */
    wid_destroy_in(w, lifespan / 2);

    /*
     * Float up from the thing.
     */
    float floater = global_config.video_gl_height / TILES_SCREEN_HEIGHT;

    wid_move_to_abs_centered(w, mx, my - floater / 2.0);
    wid_move_to_abs_centered_in(w, mx, my - floater * 1.5, lifespan);

    /*
     * And stay in front.
     */
    wid_set_z_depth(w, 100);

    return (wid_score);
}

void thing_inc_score_pump (thingp t, uint32_t delta)
{
    verify(t);

    t->score_pump += delta;

    char *tmp = dynprintf("%6u", delta);
    thing_message(t, tmp);
    myfree(tmp);
}

void thing_set_score_pump (thingp t, uint32_t score_pump)
{
    verify(t);

    t->score_pump = score_pump;
}

tree_rootp thing_tile_tiles (thingp t)
{
    verify(t);

    return (thing_template_get_tiles(t->thing_template));
}

tree_rootp thing_tile_tiles2 (thingp t)
{
    verify(t);

    return (thing_template_get_tiles2(t->thing_template));
}

widp thing_wid (thingp t)
{
    verify(t);

    return (t->wid);
}

thingp thing_item_owner (thingp t)
{
    verify(t);

    return (t->item_owner);
}

void thing_set_item_owner (thingp t, thingp owner)
{
    verify(t);

    if (owner) {
        verify(owner);
    }

    t->item_owner = owner;

    if (owner) {
        THING_LOG(t, "set owner %s", thing_logname(owner));
    }
}

void thing_set_wid (thingp t, widp w)
{
    verify(t);

    if (w) {
        verify(w);
    } else {
        if (t->wid) {
            verify(t->wid);
            wid_destroy(&t->wid);

        }
    }

    t->wid = w;

    if (w) {
        THING_DBG(t, "set wid %p/%s", w, wid_logname(w));
    }
}

void thing_inc_powerup_spam_count (thingp t, boolean val)
{
    verify(t);

    t->powerup_spam_count += val;
}

void thing_dec_powerup_spam_count (thingp t, boolean val)
{
    verify(t);

    if (!t->powerup_spam_count) {
        DIE("powerup decreemnt");
    }

    t->powerup_spam_count -= val;
}

boolean thing_has_powerup_spam_count (thingp t)
{
    verify(t);

    return (t->powerup_spam_count);
}

void thing_inc_powerup2_count (thingp t, boolean val)
{
    verify(t);

    t->powerup2_count += val;
}

void thing_dec_powerup2_count (thingp t, boolean val)
{
    verify(t);

    if (!t->powerup2_count) {
        DIE("powerup decreemnt");
    }

    t->powerup2_count -= val;
}

boolean thing_has_powerup2_count (thingp t)
{
    verify(t);

    return (t->powerup2_count);
}

void thing_inc_powerup3_count (thingp t, boolean val)
{
    verify(t);

    t->powerup3_count += val;
}

void thing_dec_powerup3_count (thingp t, boolean val)
{
    verify(t);

    if (!t->powerup3_count) {
        DIE("powerup decreemnt");
    }

    t->powerup3_count -= val;
}

boolean thing_has_powerup3_count (thingp t)
{
    verify(t);

    return (t->powerup3_count);
}

void thing_inc_powerup4_count (thingp t, boolean val)
{
    verify(t);

    t->powerup4_count += val;
}

void thing_dec_powerup4_count (thingp t, boolean val)
{
    verify(t);

    if (!t->powerup4_count) {
        DIE("powerup decreemnt");
    }

    t->powerup4_count -= val;
}

boolean thing_has_powerup4_count (thingp t)
{
    verify(t);

    return (t->powerup4_count);
}

void thing_inc_powerup5_count (thingp t, boolean val)
{
    verify(t);

    t->powerup5_count += val;
}

void thing_dec_powerup5_count (thingp t, boolean val)
{
    verify(t);

    if (!t->powerup5_count) {
        DIE("powerup decreemnt");
    }

    t->powerup5_count -= val;
}

boolean thing_has_powerup5_count (thingp t)
{
    verify(t);

    return (t->powerup5_count);
}

void thing_inc_powerup6_count (thingp t, boolean val)
{
    verify(t);

    t->powerup6_count += val;
}

void thing_dec_powerup6_count (thingp t, boolean val)
{
    verify(t);

    if (!t->powerup6_count) {
        DIE("powerup decreemnt");
    }

    t->powerup6_count -= val;
}

boolean thing_has_powerup6_count (thingp t)
{
    verify(t);

    return (t->powerup6_count);
}

void thing_inc_powerup7_count (thingp t, boolean val)
{
    verify(t);

    t->powerup7_count += val;
}

void thing_dec_powerup7_count (thingp t, boolean val)
{
    verify(t);

    if (!t->powerup7_count) {
        DIE("powerup decreemnt");
    }

    t->powerup7_count -= val;
}

boolean thing_has_powerup7_count (thingp t)
{
    verify(t);

    return (t->powerup7_count);
}

void thing_inc_powerup8_count (thingp t, boolean val)
{
    verify(t);

    t->powerup8_count += val;
}

void thing_dec_powerup8_count (thingp t, boolean val)
{
    verify(t);

    if (!t->powerup8_count) {
        DIE("powerup decreemnt");
    }

    t->powerup8_count -= val;
}

boolean thing_has_powerup8_count (thingp t)
{
    verify(t);

    return (t->powerup8_count);
}

void thing_inc_powerup_rocket_count (thingp t, boolean val)
{
    verify(t);

    t->powerup_rocket_count += val;

    THING_LOG(t, "powerup inc, rocket now %d", t->powerup_rocket_count);

    sound_play_rocket();
}

void thing_dec_powerup_rocket_count (thingp t, boolean val)
{
    verify(t);

    if (!t->powerup_rocket_count) {
        DIE("%s, powerup dec, rocket now %d", t->logname,
            t->powerup_rocket_count);
    }

    t->powerup_rocket_count -= val;

    THING_LOG(t, "powerup dec, rocket now %d", t->powerup_rocket_count);
}

boolean thing_has_powerup_rocket_count (thingp t)
{
    verify(t);

    return (t->powerup_rocket_count);
}

void thing_set_is_qqq1 (thingp t, boolean val)
{
    verify(t);

    t->is_qqq1 = val;
}

boolean thing_is_qqq1 (thingp t)
{
    verify(t);

    return (t->is_qqq1);
}

void thing_set_is_qqq2 (thingp t, boolean val)
{
    verify(t);

    t->is_qqq2 = val;
}

boolean thing_is_qqq2 (thingp t)
{
    verify(t);

    return (t->is_qqq2);
}

void thing_set_is_qqq3 (thingp t, boolean val)
{
    verify(t);

    t->is_qqq3 = val;
}

boolean thing_is_qqq3 (thingp t)
{
    verify(t);

    return (t->is_qqq3);
}

void thing_set_is_qqq4 (thingp t, boolean val)
{
    verify(t);

    t->is_qqq4 = val;
}

boolean thing_is_qqq4 (thingp t)
{
    verify(t);

    return (t->is_qqq4);
}

void thing_set_is_qqq5 (thingp t, boolean val)
{
    verify(t);

    t->is_qqq5 = val;
}

boolean thing_is_qqq5 (thingp t)
{
    verify(t);

    return (t->is_qqq5);
}

void thing_set_is_qqq6 (thingp t, boolean val)
{
    verify(t);

    t->is_qqq6 = val;
}

boolean thing_is_qqq6 (thingp t)
{
    verify(t);

    return (t->is_qqq6);
}

void thing_set_is_qqq7 (thingp t, boolean val)
{
    verify(t);

    t->is_qqq7 = val;
}

boolean thing_is_qqq7 (thingp t)
{
    verify(t);

    return (t->is_qqq7);
}

void thing_set_got_to_exit_first (thingp t, boolean val)
{
    verify(t);

    t->got_to_exit_first = val;
}

boolean thing_got_to_exit_first (thingp t)
{
    verify(t);

    return (t->got_to_exit_first);
}

void thing_set_redo_maze_search (thingp t, boolean val)
{
    verify(t);

    t->redo_maze_search = val;
}

boolean thing_redo_maze_search (thingp t)
{
    verify(t);

    return (t->redo_maze_search);
}

void thing_set_is_follows_owner (thingp t, boolean val)
{
    verify(t);

    t->is_follows_owner = val;
}

void thing_set_is_dir_down (thingp t, boolean val)
{
    verify(t);

    t->is_dir_down = val;
}

boolean thing_is_dir_down (thingp t)
{
    verify(t);

    return (t->is_dir_down);
}

void thing_set_is_dir_up (thingp t, boolean val)
{
    verify(t);

    t->is_dir_up = val;
}

boolean thing_is_dir_up (thingp t)
{
    verify(t);

    return (t->is_dir_up);
}

void thing_set_is_dir_left (thingp t, boolean val)
{
    verify(t);

    t->is_dir_left = val;
}

boolean thing_is_dir_left (thingp t)
{
    verify(t);

    return (t->is_dir_left);
}

void thing_set_is_dir_right (thingp t, boolean val)
{
    verify(t);

    t->is_dir_right = val;
}

boolean thing_is_dir_right (thingp t)
{
    verify(t);

    return (t->is_dir_right);
}

void thing_set_opened_exit (thingp t, boolean val)
{
    THING_LOG(t, "opened exit");

    verify(t);

    t->opened_exit = val;
}

boolean thing_opened_exit (thingp t)
{
    verify(t);

    return (t->opened_exit);
}

void thing_set_is_open (thingp t, boolean val)
{
    verify(t);

    t->timestamp_change_to_next_frame = time_get_time_cached();

    t->is_open = val;
}

boolean thing_is_open (thingp t)
{
    verify(t);

    return (t->is_open);
}

void thing_set_is_buried (thingp t, boolean val)
{
    verify(t);

    t->is_buried = val;
}

boolean thing_is_buried (thingp t)
{
    verify(t);

    return (t->is_buried);
}

void thing_set_is_dead (thingp t, boolean val)
{
    verify(t);

    t->is_dead = val;
}

boolean thing_is_dead (thingp t)
{
    verify(t);

    return (t->is_dead);
}

const char * thing_name (thingp t)
{
    verify(t);

    return (thing_template_name(t->thing_template));
}

const char * thing_tooltip (thingp t)
{
    verify(t);

    return (thing_template_get_tooltip(t->thing_template));
}

uint32_t thing_speed (thingp t)
{
    verify(t);

    return (thing_template_get_speed(t->thing_template));
}

tree_rootp thing_tiles (thingp t)
{
    verify(t);

    return (thing_template_get_tiles(t->thing_template));
}

tree_rootp thing_tiles2 (thingp t)
{
    verify(t);

    return (thing_template_get_tiles2(t->thing_template));
}

int32_t thing_grid_x (thingp t)
{
    verify(t);

    return (t->grid_x);
}

int32_t thing_grid_y (thingp t)
{
    verify(t);

    return (t->grid_y);
}

boolean thing_is_exit (thingp t)
{
    verify(t);

    return (thing_template_is_exit(thing_get_template(t)));
}

boolean thing_is_floor (thingp t)
{
    verify(t);

    return (thing_template_is_floor(thing_get_template(t)));
}

boolean thing_is_food (thingp t)
{
    verify(t);

    return (thing_template_is_food(thing_get_template(t)));
}

boolean thing_is_ladder (thingp t)
{
    verify(t);

    return (thing_template_is_ladder(thing_get_template(t)));
}

boolean thing_is_monst (thingp t)
{
    verify(t);

    return (thing_template_is_monst(thing_get_template(t)));
}

boolean thing_is_plant (thingp t)
{
    verify(t);

    return (thing_template_is_plant(thing_get_template(t)));
}

boolean thing_is_player (thingp t)
{
    verify(t);

    return (thing_template_is_player(thing_get_template(t)));
}

boolean thing_is_snail (thingp t)
{
    verify(t);

    return (thing_template_is_snail(thing_get_template(t)));
}

boolean thing_is_gem_0 (thingp t)
{
    verify(t);

    return (thing_template_is_gem_0(thing_get_template(t)));
}

boolean thing_is_rock (thingp t)
{
    verify(t);

    return (thing_template_is_rock(thing_get_template(t)));
}

boolean thing_is_water (thingp t)
{
    verify(t);

    return (thing_template_is_water(thing_get_template(t)));
}

boolean thing_is_lava (thingp t)
{
    verify(t);

    return (thing_template_is_lava(thing_get_template(t)));
}

boolean thing_is_gem_6 (thingp t)
{
    verify(t);

    return (thing_template_is_gem_6(thing_get_template(t)));
}

boolean thing_is_gem_7 (thingp t)
{
    verify(t);

    return (thing_template_is_gem_7(thing_get_template(t)));
}

boolean thing_is_boulder (thingp t)
{
    verify(t);

    return (thing_template_is_boulder(thing_get_template(t)));
}

boolean thing_is_xxx7 (thingp t)
{
    verify(t);

    return (thing_template_is_xxx7(thing_get_template(t)));
}

boolean thing_is_xxx8 (thingp t)
{
    verify(t);

    return (thing_template_is_xxx8(thing_get_template(t)));
}

boolean thing_is_star (thingp t)
{
    verify(t);

    return (thing_template_is_star(thing_get_template(t)));
}

boolean thing_is_powerup_spam (thingp t)
{
    verify(t);

    return (thing_template_is_powerup_spam(thing_get_template(t)));
}

boolean thing_is_rock_0 (thingp t)
{
    verify(t);

    return (thing_template_is_rock_0(thing_get_template(t)));
}

boolean thing_is_car (thingp t)
{
    verify(t);

    return (thing_template_is_car(thing_get_template(t)));
}

boolean thing_is_transparent (thingp t)
{
    verify(t);

    return (thing_template_is_transparent(thing_get_template(t)));
}

boolean thing_is_radiant (thingp t)
{
    verify(t);

    return (thing_template_is_radiant(thing_get_template(t)));
}

boolean thing_is_gem_1 (thingp t)
{
    verify(t);

    return (thing_template_is_gem_1(thing_get_template(t)));
}

boolean thing_is_gem_2 (thingp t)
{
    verify(t);

    return (thing_template_is_gem_2(thing_get_template(t)));
}

boolean thing_is_explosion (thingp t)
{
    verify(t);

    return (thing_template_is_explosion(thing_get_template(t)));
}

boolean thing_is_spikes (thingp t)
{
    verify(t);

    return (thing_template_is_spikes(thing_get_template(t)));
}

boolean thing_is_gem_4 (thingp t)
{
    verify(t);

    return (thing_template_is_gem_4(thing_get_template(t)));
}

boolean thing_is_gem_5 (thingp t)
{
    verify(t);

    return (thing_template_is_gem_5(thing_get_template(t)));
}

boolean thing_is_seedpod (thingp t)
{
    verify(t);

    return (thing_template_is_seedpod(thing_get_template(t)));
}

boolean thing_is_bomb (thingp t)
{
    verify(t);

    return (thing_template_is_bomb(thing_get_template(t)));
}

boolean thing_is_spam (thingp t)
{
    verify(t);

    return (thing_template_is_spam(thing_get_template(t)));
}

boolean thing_is_road (thingp t)
{
    verify(t);

    return (thing_template_is_road(thing_get_template(t)));
}

boolean thing_is_pipe (thingp t)
{
    verify(t);

    return (thing_template_is_pipe(thing_get_template(t)));
}

boolean thing_is_item_removed_at_level_end (thingp t)
{
    verify(t);

    return (thing_template_is_item_removed_at_level_end(thing_get_template(t)));
}

boolean thing_is_scarable (thingp t)
{
    verify(t);

    return (thing_template_is_scarable(thing_get_template(t)));
}

boolean thing_is_shrunk_when_carried (thingp t)
{
    verify(t);

    return (thing_template_is_shrunk_when_carried(thing_get_template(t)));
}

boolean thing_is_hidden_from_editor (thingp t)
{
    verify(t);

    return (thing_template_is_hidden_from_editor(thing_get_template(t)));
}

boolean thing_is_animated (thingp t)
{
    verify(t);

    return (thing_template_is_animated(thing_get_template(t)));
}

boolean thing_is_follows_owner (thingp t)
{
    verify(t);

    return (thing_template_is_follows_owner(thing_get_template(t)));
}

boolean thing_is_powerup_rocket (thingp t)
{
    verify(t);

    return (thing_template_is_powerup_rocket(thing_get_template(t)));
}

boolean thing_is_left_as_corpse_on_death (thingp t)
{
    verify(t);

    return (thing_template_is_left_as_corpse_on_death(thing_get_template(t)));
}

boolean thing_is_item_perma (thingp t)
{
    verify(t);

    return (thing_template_is_item_perma(thing_get_template(t)));
}

boolean thing_is_esnail (thingp t)
{
    verify(t);

    return (thing_template_is_esnail(thing_get_template(t)));
}

boolean thing_is_item_hidden (thingp t)
{
    verify(t);

    return (thing_template_is_item_hidden(thing_get_template(t)));
}

boolean thing_is_bonus_letter (thingp t)
{
    verify(t);

    return (thing_template_is_bonus_letter(thing_get_template(t)));
}

boolean thing_is_thing (thingp t)
{
    verify(t);

    return (thing_template_is_thing(thing_get_template(t)));
}

boolean thing_is_joinable (thingp t)
{
    verify(t);

    return (thing_template_is_joinable(thing_get_template(t)));
}

boolean thing_is_wall (thingp t)
{
    verify(t);

    return (thing_template_is_wall(thing_get_template(t)));
}

boolean thing_is_effect_sway (thingp t)
{
    verify(t);

    return (thing_template_is_effect_sway(thing_get_template(t)));
}

boolean thing_is_effect_pulse (thingp t)
{
    verify(t);

    return (thing_template_is_effect_pulse(thing_get_template(t)));
}

boolean thing_is_effect_rotate_4way (thingp t)
{
    verify(t);

    return (thing_template_is_effect_rotate_4way(thing_get_template(t)));
}

boolean thing_is_effect_rotate_2way (thingp t)
{
    verify(t);

    return (thing_template_is_effect_rotate_2way(thing_get_template(t)));
}

tree_rootp *thing_carried_itemsp (thingp t)
{
    verify(t);

    return (&t->carried_items);
}

tree_rootp thing_carried_items (thingp t)
{
    verify(t);

    return (t->carried_items);
}

thing_tilep thing_current_tile (thingp t)
{
    verify(t);

    return (t->current_tile);
}

void thing_place (void *context)
{
    thing_place_context_t *place;

    place = (typeof(place)) context;

    wid_game_map_replace_tile(wid_game_map_grid_container,
                              place->x,
                              place->y,
                              0, /* give to player count */
                              place->thing_template);

    if (thing_template_is_explosion(place->thing_template)) {
        sound_play_explosion();
    }

    myfree(context);
}

void thing_teleport (thingp t, int32_t x, int32_t y)
{
    if (time_get_time_cached() - t->timestamp_teleport < 500) {
        return;
    }

    t->timestamp_teleport = time_get_time_cached();

    widp wid_next_floor = wid_grid_find_thing_template(
                                    wid_game_map_grid_container,
                                    x,
                                    y,
                                    thing_template_is_floor);
    if (!wid_next_floor) {
        DIE("no floor tile to hpp to");
    }

    float next_floor_x, next_floor_y;

    wid_get_mxy(wid_next_floor, &next_floor_x, &next_floor_y);

    widp w = thing_wid(t);

    wid_move_end(w);
    wid_move_to_abs_centered_in(w, next_floor_x, next_floor_y, 0);

    sound_play_level_end();
}

/*
 * thing_move
 */
boolean 
thing_move (map_t *map, thingp t, int32_t x, int32_t y)
{
    uint32_t tx;
    uint32_t ty;

    verify(t);

    tx = t->x / TILE_SCREEN_WIDTH;
    ty = t->y / TILE_SCREEN_WIDTH;

    if (map_out_of_bounds(tx, ty)) {
        THING_LOG(t, "place of thing failed");
        DIE("out of bounds on old location for map at %d,%d", x, y);
    }

    /*
     * Remove from the old location.
     */
    if (t->mprev) {
        t->mprev->mnext = t->mnext;
    }

    if (t->mnext) {
        t->mnext->mprev = t->mprev;
    }

    if (map->things[tx][ty] == t) {
        map->things[tx][ty] = t->mnext;
    }

    t->mprev = 0;
    t->mnext = 0;

    /*
     * Move to the new location.
     */
    t->x = x;
    t->y = y;

    tx = t->x / TILE_SCREEN_WIDTH;
    ty = t->y / TILE_SCREEN_WIDTH;

    if (map_out_of_bounds(tx, ty)) {
        THING_LOG(t, "place of thing failed");
        DIE("out of bounds on new location for map at %d,%d", x, y);
    }

    if (!map->things[tx][ty]) {
        map->things[tx][ty] = t;
    } else {
        t->mnext = map->things[tx][ty]->mnext;
        t->mprev = map->things[tx][ty];

        t->mnext->mprev = t;
        t->mprev->mnext = t;
    }

    return (true);
}

/*
 * thing_get_map_first
 */
thingp
thing_get_map_first (map_t *map, int32_t x, int32_t y)
{
    if (map_out_of_bounds(x, y)) {
        DIE("out of bounds on get map %d,%d", x, y);
    }

    return (map->things[x][y]);
}

/*
 * thing_get_map_next
 */
thingp
thing_get_map_next (map_t *map, thingp t)
{
    if (!t) {
        return (0);
    }

    verify(t);

    return (t->mnext);
}
