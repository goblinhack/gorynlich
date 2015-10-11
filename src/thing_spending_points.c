/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */


#include "color.h"
#include "thing.h"

void thing_modify_spending_points (thingp t, int val)
{
    if (!thing_is_player(t)) {
        return;
    }

    thing_stats_modify_spending_points(t, val);

    if (val > 0) {
        MSG_SERVER_SHOUT_AT(INFO, t, 0, 0, 
                            "%%%%fg=green$+%d spending points", val);

        MSG_SERVER_SHOUT_AT(INFO, t,  0, 0, 
                            "%%%%fg=gray$Press s to spend points");
    }
}

void thing_stats_check_for_changes (thingp t)
{
    if (!thing_is_player(t)) {
        return;
    }

    int ranged_modifier = 
        thing_stats_val_to_modifier(thing_stats_get_attack_ranged(t));

    switch (ranged_modifier) {
    case 2:
        if (!thing_has_ability_reverse_shot(t)) {
            t->has_ability_reverse_shot = true;
            MSG_SERVER_SHOUT_AT(POPUP, t, 0, 0,
                                "%%%%font=%s$%%%%fg=%s$reverse shot ability unlocked",
                                "vlarge", "green");
        }
        break;
    case 3:
        if (!thing_has_ability_double_shot(t)) {
            t->has_ability_double_shot = true;
            MSG_SERVER_SHOUT_AT(POPUP, t, 0, 0,
                                "%%%%font=%s$%%%%fg=%s$double shot ability unlocked",
                                "vlarge", "green");
        }
        break;
    case 4:
        if (!thing_has_ability_triple_shot(t)) {
            t->has_ability_triple_shot = true;
            MSG_SERVER_SHOUT_AT(POPUP, t, 0, 0,
                                "%%%%font=%s$%%%%fg=%s$triple shot ability unlocked",
                                "vlarge", "green");
        }
        break;
    case 5:
        if (!thing_has_ability_burst_shot(t)) {
            t->has_ability_burst_shot = true;
            MSG_SERVER_SHOUT_AT(POPUP, t, 0, 0,
                                "%%%%font=%s$%%%%fg=%s$burst shot ability unlocked",
                                "vlarge", "green");
        }
        break;
    }

    int melee_modifier = 
        thing_stats_val_to_modifier(thing_stats_get_attack_melee(t));

    switch (melee_modifier) {
    case 2:
        if (!thing_has_ability_reverse_swing(t)) {
            t->has_ability_reverse_swing = true;
            MSG_SERVER_SHOUT_AT(POPUP, t, 0, 0,
                                "%%%%font=%s$%%%%fg=%s$reverse swing ability unlocked",
                                "vlarge", "green");
        }
        break;
    case 3:
        if (!thing_has_ability_double_speed_swing(t)) {
            t->has_ability_double_speed_swing = true;
            MSG_SERVER_SHOUT_AT(POPUP, t, 0, 0,
                                "%%%%font=%s$%%%%fg=%s$double speed swing ability unlocked",
                                "vlarge", "green");
        }
        break;
    case 4:
        if (!thing_has_ability_triple_speed_swing(t)) {
            t->has_ability_triple_speed_swing = true;
            MSG_SERVER_SHOUT_AT(POPUP, t, 0, 0,
                                "%%%%font=%s$%%%%fg=%s$triple speed swing unlocked",
                                "vlarge", "green");
        }
        break;
    case 5:
        if (!thing_has_ability_double_damage_swing(t)) {
            t->has_ability_double_damage_swing = true;
            MSG_SERVER_SHOUT_AT(POPUP, t, 0, 0,
                                "%%%%font=%s$%%%%fg=%s$double damage swing unlocked",
                                "vlarge", "green");
        }
        break;
    }

    int health_modifier = 
        thing_stats_val_to_modifier(thing_stats_get_toughness(t));

    switch (health_modifier) {
    case 2:
        if (!thing_has_ability_rage(t)) {
            t->has_ability_rage = true;
            MSG_SERVER_SHOUT_AT(POPUP, t, 0, 0,
                                "%%%%font=%s$%%%%fg=%s$rage when quarter health unlocked",
                                "vlarge", "green");
        }
        break;
    case 3:
        if (!thing_has_ability_perma_rage(t)) {
            t->has_ability_perma_rage = true;
            MSG_SERVER_SHOUT_AT(POPUP, t, 0, 0,
                                "%%%%font=%s$%%%%fg=%s$permanent rage ability unlocked",
                                "vlarge", "green");
        }
        break;
    }
}
