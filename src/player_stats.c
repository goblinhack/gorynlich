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
#include "music.h"

static int player_stats_generate_single_random (void) 
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

int player_stats_get_modifier (int value) 
{
    static const int modifiers[] = {
        [0] =               -5,
        [1] =               -5,
        [2] =               -4,
        [3] =               -4,
        [4] =               -3,
        [5] =               -3,
        [6] =               -2,
        [7] =               -2,
        [8] =               -1,
        [9] =               0,
        [10] =              0,
        [11] =              0,
        [12] =              +1,
        [13] =              +1,
        [14] =              +2,
        [15] =              +2,
        [16] =              +3,
        [17] =              +3,
        [18] =              +4,
        [19] =              +5,
        [20] =              +5,
        [21] =              +5,
        [22] =              +6,
        [23] =              +7,
        [24] =              +7,
        [25] =              +8,
        [26] =              +8,
        [27] =              +9,
        [28] =              +9,
        [29] =              +10,
        [30] =              +10,
        [31] =              +10,
        [32] =              +11,
        [33] =              +12,
        [34] =              +12,
        [35] =              +12,
        [36] =              +13,
        [37] =              +13,
        [38] =              +14,
        [39] =              +14,
        [40] =              +15,
        [41] =              +15,
        [42] =              +16,
        [43] =              +16,
        [44] =              +17,
    };

    if (value < 0) {
        return (0);
    }

    if (value >= ARRAY_SIZE(modifiers)) {
        value = ARRAY_SIZE(modifiers);
    }

    return (modifiers[value]);
}


void player_stats_generate_random (player_stats_t *player_stats) 
{
    player_stats->melee = player_stats_generate_single_random();
    player_stats->ranged = player_stats_generate_single_random();
    player_stats->speed = player_stats_generate_single_random();
    player_stats->vision = player_stats_generate_single_random();
    player_stats->healing = player_stats_generate_single_random();
}
