/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "color.h"
#include "wid_popup.h"
#include "wid_player_stats.h"
#include "string.h"
#include "name.h"
#include "thing_template.h"
#include "wid_player_info.h"
#include "math.h"

static int player_stats_generate_spending_points (void) 
{
    /*
     * A slightly generous bell curve.
     */
    int possible[] = {
        8,
        7,  7,
        6,  6,
        5,  5,  5, 
        4,  4,  4,  4,  4,
        3,  3,  3,  3,  3,  3,  3,  3,
        2,  2,  2,  2,  2,  2,
        1,  1,  1,  1,
    };

    return (possible[rand() % ARRAY_SIZE(possible)]);
}

int player_stats_get_modifier (int value) 
{
    static int modifiers[45];
    static int init;

    if (!init) {
        init = 1;

        modifiers[0] =               -5;
        modifiers[1] =               -5;
        modifiers[2] =               -4;
        modifiers[3] =               -4;
        modifiers[4] =               -3;
        modifiers[5] =               -3;
        modifiers[6] =               -2;
        modifiers[7] =               -2;
        modifiers[8] =               -1;
        modifiers[9] =               0;
        modifiers[10] =              0;
        modifiers[11] =              0;
        modifiers[12] =              +1;
        modifiers[13] =              +1;
        modifiers[14] =              +2;
        modifiers[15] =              +2;
        modifiers[16] =              +3;
        modifiers[17] =              +3;
        modifiers[18] =              +4;
        modifiers[19] =              +5;
        modifiers[20] =              +5;
        modifiers[21] =              +5;
        modifiers[22] =              +6;
        modifiers[23] =              +7;
        modifiers[24] =              +7;
        modifiers[25] =              +8;
        modifiers[26] =              +8;
        modifiers[27] =              +9;
        modifiers[28] =              +9;
        modifiers[29] =              +10;
        modifiers[30] =              +10;
        modifiers[31] =              +10;
        modifiers[32] =              +11;
        modifiers[33] =              +12;
        modifiers[34] =              +12;
        modifiers[35] =              +12;
        modifiers[36] =              +13;
        modifiers[37] =              +13;
        modifiers[38] =              +14;
        modifiers[39] =              +14;
        modifiers[40] =              +15;
        modifiers[41] =              +15;
        modifiers[42] =              +16;
        modifiers[43] =              +16;
        modifiers[44] =              +17;
    }

    if (value < 0) {
        return (0);
    }

    if (value >= (int) ARRAY_SIZE(modifiers)) {
        value = (int) ARRAY_SIZE(modifiers);
    }

    return (modifiers[value]);
}

static int player_stats_item_add (player_stats_t *player_stats,
                                  const thing_templatep t) 
{
    const int id = thing_template_to_id(t);

    if (player_stats->carrying[id].quantity == THING_ITEM_CARRY_MAX) {
        MSG_BOX("Trying to carry too many of %s", 
                thing_template_short_name(t));
        return (false);
    }

    /*
     * Add to the inventory.
     */
    int i;
    for (i = 0; i < THING_INVENTORY_MAX; i++) {
        if (!player_stats->inventory[i]) {
            player_stats->inventory[i] = id;
            break;
        }
    }

    if (i == THING_INVENTORY_MAX) {
        MSG_BOX("Trying to carry too many items to add %s",
                thing_template_short_name(t));
        return (false);
    }

    player_stats->carrying[id].quantity++;

    /*
     * Allow quality to carry onto the top item. Only once we pop off the
     * stack do we reset quality. Cursed we never pop.
     */

    /*
     * If there is space on the action bar, add it.
     */
    if (thing_template_is_valid_for_action_bar(t)) {
        for (i = 0; i < THING_ACTION_BAR_MAX; i++) {
            if (!player_stats->action_bar[i]) {
                player_stats->action_bar[i] = id;
                break;
            }
        }
    }

    return (true);
}

static void player_stats_generate_random_items (player_stats_t *player_stats) 
{
    int count = gaussrand(2, 1) + 1;

    while (count--) {
        thing_templatep t = 0;

        for (;;) {
            t = id_to_thing_template(rand() % THING_MAX);

            if (!thing_template_is_carryable(t)) {
                continue;
            }

            int chance = rand() % 1000;
            if (thing_template_get_chance_of_appearing(t) > chance) {
                continue;
            }

CON("ADD %s ",thing_template_short_name(t));
            break;
        }

        player_stats_item_add(player_stats, t);
    }
}

void player_stats_generate_random (player_stats_t *player_stats) 
{
    strncpy(player_stats->pclass, pclass_random(),
            sizeof(player_stats->pclass) - 1);

    thing_templatep thing_template = 
                    player_stats_to_thing_template(player_stats);

    player_stats_init(player_stats);

    if (!wid_player_info_set_name) {
        strncpy(player_stats->pname, name_random(player_stats->pclass),
                sizeof(player_stats->pname) - 1);
    }

    player_stats->spending_points = player_stats_generate_spending_points();

    /*
     * attack_melee
     */
    player_stats->attack_melee = 
                    thing_template_get_stats_attack_melee(thing_template);
    player_stats->attack_melee = gaussrand(player_stats->attack_melee, 2);

    /*
     * attack_ranged
     */
    player_stats->attack_ranged = 
                    thing_template_get_stats_attack_ranged(thing_template);
    player_stats->attack_ranged = gaussrand(player_stats->attack_ranged, 2);

    /*
     * attack_magical
     */
    player_stats->attack_magical = 
                    thing_template_get_stats_attack_magical(thing_template);
    player_stats->attack_magical = gaussrand(player_stats->attack_magical, 2);


    /*
     * defense
     */
    player_stats->defense = 
                    thing_template_get_stats_defense(thing_template);
    player_stats->defense = gaussrand(player_stats->defense, 2);


    /*
     * speed
     */
    player_stats->speed = 
                    thing_template_get_stats_speed(thing_template);
    player_stats->speed = gaussrand(player_stats->speed, 2);


    /*
     * vision
     */
    player_stats->vision = 
                    thing_template_get_stats_vision(thing_template);
    player_stats->vision = gaussrand(player_stats->vision, 2);

    /*
     * healing
     */
    player_stats->healing = 
                    thing_template_get_stats_healing(thing_template);
    player_stats->healing = gaussrand(player_stats->healing, 2);

    /*
     * hp
     */
    player_stats->hp = thing_template_get_stats_max_hp(thing_template);
    player_stats->max_hp = 
        player_stats->hp = gaussrand(player_stats->hp, 
                                    player_stats->hp / 10);

    /*
     * id
     */
    player_stats->id = thing_template_get_stats_max_id(thing_template);
    player_stats->max_id = 
        player_stats->id = gaussrand(player_stats->id, 
                                    player_stats->id / 10);

    /*
     * Be generous and give some items at startup.
     */
    player_stats_generate_random_items(player_stats);
}

void player_stats_init (player_stats_t *player_stats) 
{
    memset(player_stats->inventory, 0, sizeof(player_stats->inventory));
    memset(player_stats->action_bar, 0, sizeof(player_stats->action_bar));
    memset(player_stats->carrying, 0, sizeof(player_stats->carrying));
    memset(player_stats->worn, 0, sizeof(player_stats->worn));

    if (!player_stats->attack_melee) {
        player_stats->attack_melee = 10;
    }
    if (!player_stats->attack_ranged) {
        player_stats->attack_ranged = 10;
    }
    if (!player_stats->attack_magical) {
        player_stats->attack_magical = 10;
    }
    if (!player_stats->defense) {
        player_stats->defense = 10;
    }
    if (!player_stats->speed) {
        player_stats->speed = 10;
    }
    if (!player_stats->vision) {
        player_stats->vision = 10;
    }
    if (!player_stats->healing) {
        player_stats->healing = 10;
    }
}

thing_templatep
player_stats_to_thing_template (player_stats_t *player_stats)
{
    return (thing_template_find_short_name(player_stats->pclass));
}
