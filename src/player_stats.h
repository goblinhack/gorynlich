/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

typedef struct player_stats_ {
    uint8_t spending_points;
    uint8_t melee;
    uint8_t ranged;
    uint8_t defense;
    uint8_t speed;
    uint8_t vision;
    uint8_t healing;
} player_stats_t;

int player_stats_get_modifier(int value);
void player_stats_generate_random(player_stats_t *player_stats);
