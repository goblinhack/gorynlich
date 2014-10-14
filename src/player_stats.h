/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

typedef struct player_stats_ {
    char pname[SMALL_STRING_LEN_MAX];
    char pclass[SMALL_STRING_LEN_MAX];
    int16_t hp;
    int16_t max_hp;
    int16_t id;
    int16_t max_id;
    uint16_t experience;
    uint8_t spending_points;
    uint8_t attack_melee;
    uint8_t attack_ranged;
    uint8_t attack_magical;
    uint8_t defense;
    uint8_t speed;
    uint8_t vision;
    uint8_t healing;
} player_stats_t;

int player_stats_get_modifier(int value);
void player_stats_generate_random(player_stats_t *);
void player_stats_init(player_stats_t *);
thing_templatep player_stats_to_thing_template(player_stats_t *);
