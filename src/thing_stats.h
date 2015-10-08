/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
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

typedef struct {
    int8_t x;
    int8_t y;
} __attribute__ ((packed)) level_pos_t;

typedef struct item_t_ {
    uint16_t id;

    uint8_t quantity:5;
    uint8_t quality:3;

    uint8_t enchanted:3;
    uint8_t cursed:1;
    uint8_t unused:4;
} __attribute__ ((packed)) item_t;

int item_push(itemp dst, item_t src);
int item_pop(itemp dst, itemp popped);
int item_pop_all(itemp dst, itemp popped);

typedef struct thing_stats_ {
    uint16_t thing_id;

    /*
     * Used by the client to know if the server has accepted a change.
     * The client can therefore avoid updates of older versions.
     */
    int16_t client_version;

    int16_t hp;
    int16_t max_hp;
    int16_t magic;
    int16_t max_magic;

    int32_t xp;
    int32_t cash;

    uint8_t spending_points;
    uint8_t attack_melee;
    uint8_t attack_ranged;
    uint8_t attack_magical;
    uint8_t defense;
    uint8_t speed;
    uint8_t vision;
    uint8_t toughness;

    level_pos_t level_pos;

    /*
     * What we are carrying and where.
     */
    uint8_t action_bar_index;;

    /*
     * If carrying a shield, how powerful is it?
     */
    uint16_t shield;

    /*
     * Where this stats structure is used
     */
    uint8_t on_server:1;

    item_t inventory[THING_INVENTORY_MAX];
    item_t action_bar[THING_ACTION_BAR_MAX];
    item_t worn[THING_WORN_MAX];

    char pname[SMALL_STRING_LEN_MAX+1];
    char pclass[SMALL_STRING_LEN_MAX+1];

} __attribute__ ((packed)) thing_stats;

int thing_stats_val_to_modifier(int value);
void thing_stats_get_random(thing_statsp ,
                            int new_random_name_and_class);
void thing_stats_init(thing_statsp );
tpp thing_stats_to_tp(thing_statsp );

itemp thing_stats_has_item(thing_statsp thing_stats,
                            int32_t id,
                            int32_t *index);

itemp thing_stats_has_inventory_item(thing_statsp thing_stats,
                                        int32_t item,
                                        int32_t *index);

itemp thing_stats_has_action_bar_item(thing_statsp thing_stats,
                                       int32_t item,
                                       int32_t *index);

itemp thing_stats_has_worn_item(thing_statsp thing_stats,
                                   int32_t item,
                                   int32_t *index);

int thing_stats_item_add(thingp t,
                          thing_statsp thing_stats,
                          item_t item);

int thing_stats_item_remove(thingp t,
                             thing_statsp thing_stats,
                             const tpp it);

int thing_stats_item_degrade(thingp t,
                              thing_statsp thing_stats,
                              const tpp it);

int thing_stats_item_polymorph(thing_statsp thing_stats,
                                const int32_t from,
                                const int32_t to);

void player_inventory_sort(thing_statsp thing_stats);

void thing_stats_client_modified(thing_statsp player_stats);
void thing_stats_dump(const thing_statsp s);
int thing_stats_diff(const thing_statsp old, const thing_statsp new_stats);
int thing_stats_merge(thing_statsp merged_stats, 
                      thing_statsp current_stats, 
                      thing_statsp new_stats);;

int32_t thing_stats_get_client_version(thingp t);
int32_t thing_stats_get_on_server(thingp t);
int32_t thing_stats_get_cash(thingp t);
int32_t thing_stats_get_hp(thingp t);
int32_t thing_stats_get_action_bar_index(thingp t);
int32_t thing_stats_get_max_hp(thingp t);
int32_t thing_stats_get_magic(thingp t);
int32_t thing_stats_get_max_magic(thingp t);
int32_t thing_stats_get_xp(thingp t);
int32_t thing_stats_get_attack_melee(thingp t);
int32_t thing_stats_get_attack_ranged(thingp t);
int32_t thing_stats_get_attack_magical(thingp t);
int32_t thing_stats_get_speed(thingp t);
int32_t thing_stats_get_spending_points(thingp t);
int32_t thing_stats_get_vision(thingp t);
int32_t thing_stats_get_toughness(thingp t);
int32_t thing_stats_get_defense(thingp t);
void thing_stats_set_client_version(thingp t, int32_t val);
void thing_stats_set_on_server(thingp t, int32_t val);
void thing_stats_set_cash(thingp t, int32_t val);
void thing_stats_set_hp(thingp t, int32_t val);
void thing_stats_set_action_bar_index(thingp t, int32_t val);
void thing_stats_set_max_hp(thingp t, int32_t val);
void thing_stats_set_magic(thingp t, int32_t val);
void thing_stats_set_max_magic(thingp t, int32_t val);
void thing_stats_set_xp(thingp t, int32_t val);
void thing_stats_set_attack_melee(thingp t, int32_t val);
void thing_stats_set_attack_ranged(thingp t, int32_t val);
void thing_stats_set_attack_magical(thingp t, int32_t val);
void thing_stats_set_speed(thingp t, int32_t val);
void thing_stats_set_spending_points(thingp t, int32_t val);
void thing_stats_set_vision(thingp t, int32_t val);
void thing_stats_set_toughness(thingp t, int32_t val);
void thing_stats_set_defense(thingp t, int32_t val);
void thing_stats_modify_client_version(thingp t, int32_t val);
void thing_stats_modify_cash(thingp t, int32_t val);
void thing_stats_modify_hp(thingp t, int32_t val);
void thing_stats_modify_action_bar_index(thingp t, int32_t val);
void thing_stats_modify_max_hp(thingp t, int32_t val);
void thing_stats_modify_magic(thingp t, int32_t val);
void thing_stats_modify_max_magic(thingp t, int32_t val);
void thing_stats_modify_xp(thingp t, int32_t val);
void thing_stats_modify_attack_melee(thingp t, int32_t val);
void thing_stats_modify_attack_ranged(thingp t, int32_t val);
void thing_stats_modify_attack_magical(thingp t, int32_t val);
void thing_stats_modify_speed(thingp t, int32_t val);
void thing_stats_modify_spending_points(thingp t, int32_t val);
void thing_stats_modify_vision(thingp t, int32_t val);
void thing_stats_modify_toughness(thingp t, int32_t val);
void thing_stats_modify_defense(thingp t, int32_t val);

int32_t stats_get_client_version(thing_statsp);
int32_t stats_get_on_server(thing_statsp);
int32_t stats_get_cash(thing_statsp);
int32_t stats_get_hp(thing_statsp);
int32_t stats_get_action_bar_index(thing_statsp);
int32_t stats_get_max_hp(thing_statsp);
int32_t stats_get_magic(thing_statsp);
int32_t stats_get_max_magic(thing_statsp);
int32_t stats_get_xp(thing_statsp);
int32_t stats_get_attack_melee(thing_statsp);
int32_t stats_get_attack_ranged(thing_statsp);
int32_t stats_get_attack_magical(thing_statsp);
int32_t stats_get_speed(thing_statsp);
int32_t stats_get_spending_points(thing_statsp);
int32_t stats_get_vision(thing_statsp);
int32_t stats_get_toughness(thing_statsp);
int32_t stats_get_defense(thing_statsp);
void stats_set_client_version(thing_statsp, int32_t val);
void stats_set_on_server(thing_statsp, int32_t val);
void stats_set_cash(thing_statsp, int32_t val);
void stats_set_hp(thing_statsp, int32_t val);
void stats_set_action_bar_index(thing_statsp, int32_t val);
void stats_set_max_hp(thing_statsp, int32_t val);
void stats_set_magic(thing_statsp, int32_t val);
void stats_set_max_magic(thing_statsp, int32_t val);
void stats_set_xp(thing_statsp, int32_t val);
void stats_set_attack_melee(thing_statsp, int32_t val);
void stats_set_attack_ranged(thing_statsp, int32_t val);
void stats_set_attack_magical(thing_statsp, int32_t val);
void stats_set_speed(thing_statsp, int32_t val);
void stats_set_spending_points(thing_statsp, int32_t val);
void stats_set_vision(thing_statsp, int32_t val);
void stats_set_toughness(thing_statsp, int32_t val);
void stats_set_defense(thing_statsp, int32_t val);
void stats_modify_client_version(thing_statsp, int32_t val);
void stats_modify_cash(thing_statsp, int32_t val);
void stats_modify_hp(thing_statsp, int32_t val);
void stats_modify_action_bar_index(thing_statsp, int32_t val);
void stats_modify_max_hp(thing_statsp, int32_t val);
void stats_modify_magic(thing_statsp, int32_t val);
void stats_modify_max_magic(thing_statsp, int32_t val);
void stats_modify_xp(thing_statsp, int32_t val);
void stats_modify_attack_melee(thing_statsp, int32_t val);
void stats_modify_attack_ranged(thing_statsp, int32_t val);
void stats_modify_attack_magical(thing_statsp, int32_t val);
void stats_modify_speed(thing_statsp, int32_t val);
void stats_modify_spending_points(thing_statsp, int32_t val);
void stats_modify_vision(thing_statsp, int32_t val);
void stats_modify_toughness(thing_statsp, int32_t val);
void stats_modify_defense(thing_statsp, int32_t val);
void stats_bump_version(thing_statsp stats);
