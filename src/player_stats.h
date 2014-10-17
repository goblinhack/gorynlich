/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#pragma once

#define THING_INVENTORY_MAX     100
#define THING_ACTION_BAR_MAX    10
#define THING_WORN_MAX          5

enum {
    THING_WORN_ARMOR,
    THING_WORN_HELMET,
    THING_WORN_BOOTS,
    THING_WORN_ARM_LEFT,
    THING_WORN_ARM_RIGHT,
};

#define THING_ITEM_CARRY_MAX    15
#define THING_ITEM_QUALITY_MAX  7

typedef struct item_t_ {
    uint8_t quantity:4;
    uint8_t quality:3;
    uint8_t cursed:1;
} item_t;

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

    /*
     * How many and of what we are carrying.
     */
    item_t carrying[THING_MAX];

    /*
     * These contain thing template IDs that refer back to the carrying
     * array. Things are either in the inventory or the action bar.
     */
    uint8_t inventory[THING_INVENTORY_MAX];
    uint8_t action_bar[THING_ACTION_BAR_MAX];
    uint8_t worn[THING_ACTION_BAR_MAX];
} __attribute__ ((packed)) player_stats_t;

int player_stats_get_modifier(int value);
void player_stats_generate_random(player_stats_t *);
void player_stats_init(player_stats_t *);
thing_templatep player_stats_to_thing_template(player_stats_t *);
