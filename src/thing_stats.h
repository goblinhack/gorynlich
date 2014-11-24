/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#pragma once

#define THING_INVENTORY_MAX             100

#define THING_INVENTORY_FOOD_BASE       0
#define THING_INVENTORY_WEAPON_BASE     20
#define THING_INVENTORY_MAGICAL_BASE    40
#define THING_INVENTORY_SPELL_BASE      60
#define THING_INVENTORY_MISC_BASE       80
#define THING_INVENTORY_CLASSES         5

#define THING_ACTION_BAR_MAX            10
#define THING_WORN_MAX                  5

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

int item_push(itemp dst, item_t src);
int item_pop(itemp dst, itemp popped);

typedef struct thing_stats_ {
    uint16_t thing_id;
    uint16_t weapon_carry_anim_id_latest;
    uint16_t weapon_swing_anim_id_latest;

    int16_t hp;
    int16_t max_hp;
    int16_t magic;
    int16_t max_magic;

    uint32_t xp;
    uint32_t cash;

    uint8_t spending_points;
    uint8_t attack_melee;
    uint8_t attack_ranged;
    uint8_t attack_magical;
    uint8_t defense;
    uint8_t speed;
    uint8_t vision;
    uint8_t healing;
    uint8_t weapon;

    /*
     * What we are carrying and where.
     */
    item_t inventory[THING_INVENTORY_MAX];
    item_t action_bar[THING_ACTION_BAR_MAX];
    item_t worn[THING_WORN_MAX];

    char pname[SMALL_STRING_LEN_MAX+1];
    char pclass[SMALL_STRING_LEN_MAX+1];

} __attribute__ ((packed)) thing_stats;

int thing_stats_val_to_modifier(int value);
void thing_stats_get_random(thing_stats *);
void thing_stats_init(thing_stats *);
tpp thing_stats_to_tp(thing_stats *);

itemp thing_stats_has_item(thing_stats *thing_stats,
                            uint32_t id,
                            uint32_t *index);

itemp thing_stats_has_inventory_item(thing_stats *thing_stats,
                                        uint32_t item,
                                        uint32_t *index);

itemp thing_stats_has_action_bar_item(thing_stats *thing_stats,
                                       uint32_t item,
                                       uint32_t *index);

itemp thing_stats_has_worn_item(thing_stats *thing_stats,
                                   uint32_t item,
                                   uint32_t *index);

int thing_stats_item_add(thingp t,
                          thing_stats *thing_stats,
                          item_t item);

int thing_stats_item_remove(thingp t,
                             thing_stats *thing_stats,
                             const tpp it);

int thing_stats_item_degrade(thingp t,
                              thing_stats *thing_stats,
                              const tpp it);

int thing_stats_item_polymorph(thing_stats *thing_stats,
                                const uint32_t from,
                                const uint32_t to);

void player_inventory_sort(thing_stats *thing_stats);
