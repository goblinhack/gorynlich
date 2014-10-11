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

static int player_stats_generate_spending_stat (void) 
{
    /*
     * A slightly generous bell curve.
     */
    int possible[] = {
        18, 
        17, 
        16, 16, 
        15, 15, 15, 
        14, 14, 14, 14, 
        13, 13, 13, 13, 13, 
        12, 12, 12, 12, 12, 
        11, 11, 11, 11, 11, 11,
        10, 10, 10, 10, 10, 10,
        9,  9,  9,  9,  9, 
        8,  8,  8,  8,  8, 
        7,  7,  7,  7, 
        6,  6,  6,  6, 
        5,  5,  5, 
        4,  4,  
        3,  3,
        2,  
        1,  
    };

    return (possible[rand() % ARRAY_SIZE(possible)]);
}

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


void player_stats_generate_random (player_stats_t *player_stats) 
{
    memset(player_stats, 0, sizeof(*player_stats));

    player_stats->spending_points = player_stats_generate_spending_points();
    player_stats->attack_melee = player_stats_generate_spending_stat();
    player_stats->attack_ranged = player_stats_generate_spending_stat();
    player_stats->defense = player_stats_generate_spending_stat();
    player_stats->speed = player_stats_generate_spending_stat();
    player_stats->vision = player_stats_generate_spending_stat();
    player_stats->healing = player_stats_generate_spending_stat();

    player_stats_init(player_stats);

    strncpy(player_stats->pclass, pclass_random(),
            sizeof(player_stats->pclass) - 1);

    strncpy(player_stats->pname, name_random(player_stats->pclass),
            sizeof(player_stats->pname) - 1);

    thing_templatep thing_template = 
                    player_stats_to_thing_template(player_stats);

    player_stats->hp =
    player_stats->max_hp = thing_template_get_stats_max_hp(thing_template);
}

void player_stats_init (player_stats_t *player_stats) 
{
    if (!player_stats->attack_melee) {
        player_stats->attack_melee = 10;
    }
    if (!player_stats->attack_ranged) {
        player_stats->attack_ranged = 10;
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
