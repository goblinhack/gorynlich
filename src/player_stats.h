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

#define THING_ITEM_CARRY_MAX    32
#define THING_ITEM_QUALITY_MAX  7

typedef struct item_t_ {
    uint8_t id;

    uint8_t quantity:5;
    uint8_t quality:3;

    uint8_t enchanted:3;
    uint8_t cursed:1;
    uint8_t pad:4;
} __attribute__ ((packed)) item_t;

int item_push(item_t *dst, item_t src);
int item_pop(item_t *dst, item_t *popped);

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
     * What we are carrying and where.
     */
    item_t inventory[THING_INVENTORY_MAX];
    item_t action_bar[THING_ACTION_BAR_MAX];
    item_t worn[THING_WORN_MAX];
} __attribute__ ((packed)) player_stats_t;

int player_stats_get_modifier(int value);
void player_stats_generate_random(player_stats_t *);
void player_stats_init(player_stats_t *);
thing_templatep player_stats_to_thing_template(player_stats_t *);

item_t *player_stats_has_item(player_stats_t *player_stats,
                              uint32_t id,
                              uint32_t *index);

item_t *player_stats_has_inventory_item(player_stats_t *player_stats,
                                        uint32_t item,
                                        uint32_t *index);

item_t *player_stats_has_action_bar_item(player_stats_t *player_stats,
                                         uint32_t item,
                                         uint32_t *index);

item_t *player_stats_has_worn_item(player_stats_t *player_stats,
                                   uint32_t item,
                                   uint32_t *index);

int player_stats_item_add(thingp t,
                          player_stats_t *player_stats,
                          item_t item);

int player_stats_item_remove(thingp t,
                             player_stats_t *player_stats,
                             const thing_templatep it);

int player_stats_item_degrade(thingp t,
                              player_stats_t *player_stats,
                              const thing_templatep it);

int player_stats_item_polymorph(player_stats_t *player_stats,
                                const uint32_t from,
                                const uint32_t to);

