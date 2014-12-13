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
#include "string_util.h"
#include "name.h"
#include "thing_template.h"
#include "wid_player_info.h"
#include "math_util.h"

int item_push (itemp dst, item_t src)
{
    if (!dst->id) {
        /*
         * No item at the destination. Just copy.
         */
        memcpy(dst, &src, sizeof(item_t));
        return (true);
    }

    if (!dst->quantity) {
        DIE("bug, dst should have quantity");
    }

    if (dst->id != src.id) {
        /*
         * Cannot push onto different item.
         */
        return (false);
    }

    if (dst->quantity + src.quantity >= THING_ITEM_CARRY_MAX) {
        /*
         * Cannot push this many.
         */
        return (false);
    }

    tpp tp = id_to_tp(dst->id);

    if (!tp_is_stackable(tp)) {
        /*
         * Cannot stack this item.
         */
        return (false);
    }

    if ((dst->quality != src.quality)     &&
        (dst->enchanted != src.enchanted) &&
        (dst->cursed != src.cursed)) {
        /*
         * Cannot stack different qualities of item.
         */
        return (false);
    }

    dst->quantity += src.quantity;

    /*
     * Copy attributes.
     */
    dst->quality = src.quality;
    dst->enchanted = src.enchanted;
    dst->cursed = src.cursed;

    return (true);
}

int item_pop (itemp dst, itemp popped)
{
    if (!dst->id) {
        return (false);
    }

    if (!dst->quantity) {
        DIE("bug, cannot pop, no quantity");
    }

    dst->quantity--;

    if (popped) {
        memcpy(popped, dst, sizeof(item_t));
        popped->quantity = 1;
    }

    if (!dst->quantity) {
        memset(dst, 0, sizeof(item_t));
    }

    return (true);
}

static int item_enchant_randomly (void)
{
    int r = myrand() % 1000;

    if (r <= 1) {
        return (5);
    }

    if (r <= 2) {
        return (4);
    }

    if (r <= 5) {
        return (3);
    }

    if (r <= 10) {
        return (2);
    }

    if (r <= 20) {
        return (1);
    }

    return (0);
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

    return (possible[myrand() % ARRAY_SIZE(possible)]);
}

int thing_stats_val_to_modifier (int value) 
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

void player_inventory_sort (thing_stats *player_stats)
{
    if (!player_stats) {
        DIE("no player stats");
    }

    static const uint32_t NCLASSES = 
                    THING_INVENTORY_CLASSES;
    static const uint32_t N_PER_CLASS = 
                    THING_INVENTORY_MAX / THING_INVENTORY_CLASSES;

    item_t inv_new[THING_INVENTORY_MAX];
    item_t inv_old[THING_INVENTORY_MAX];
    uint32_t count_per_class[THING_INVENTORY_CLASSES];

    memcpy(inv_old, &player_stats->inventory, sizeof(inv_old));
    memset(inv_new, 0, sizeof(inv_new));
    memset(count_per_class, 0, sizeof(count_per_class));

    uint32_t i;

    for (i = 0; i < THING_INVENTORY_MAX; i++) {
        uint32_t id;

        id = inv_old[i].id;
        if (!id) {
            continue;
        }

        uint32_t base = THING_INVENTORY_MISC_BASE;

        tpp tp = id_to_tp(id);

        if (tp_is_magical(tp)) {
            base = THING_INVENTORY_MAGICAL_BASE;
        } else if (tp_is_weapon(tp)) {
            base = THING_INVENTORY_WEAPON_BASE;
        } else if (tp_is_spell(tp)) {
            base = THING_INVENTORY_MAGICAL_BASE;
        } else if (tp_is_food(tp)) {
            base = THING_INVENTORY_FOOD_BASE;
        }

        if (count_per_class[base] >= N_PER_CLASS) {
            base = THING_INVENTORY_MISC_BASE;

            int which;

            for (which = 0; which < NCLASSES; which++) {
                if (count_per_class[which] < N_PER_CLASS) { 
                    base = N_PER_CLASS * which;
                    break;
                }
            }
        }

        int which = base / N_PER_CLASS;

        if (count_per_class[which] >= N_PER_CLASS) {
            /*
             * Should never happen.
             */
            ERR("could not fit item for sorting");
            continue;
        }

        uint32_t new_i = base + count_per_class[which];
        if (new_i >= THING_INVENTORY_MAX) {
            ERR("overflow in item sorting");
            continue;
        }

        inv_new[new_i] = inv_old[i];
        count_per_class[which]++;
    }

    memcpy(&player_stats->inventory, inv_new, sizeof(inv_new));
}

itemp thing_stats_has_item (thing_stats *player_stats,
                             uint32_t id,
                             uint32_t *index)
{
    itemp i;
    
    i = thing_stats_has_action_bar_item(player_stats, id, index);
    if (i) {
        return (i);
    }

    i = thing_stats_has_worn_item(player_stats, id, index);
    if (i) {
        return (i);
    }

    i = thing_stats_has_inventory_item(player_stats, id, index);
    if (i) {
        return (i);
    }

    return (0);
}

itemp thing_stats_has_inventory_item (thing_stats *player_stats,
                                       uint32_t id,
                                       uint32_t *index)
{
    uint32_t i;

    for (i = 0; i < THING_INVENTORY_MAX; i++) {
        if (player_stats->inventory[i].id == id) {
            if (index) {
                *index = i;
            }

            return (&player_stats->inventory[i]);
        }
    }

    return (0);
}

itemp thing_stats_has_action_bar_item (thing_stats *player_stats,
                                        uint32_t id,
                                        uint32_t *index)
{
    uint32_t i;

    for (i = 0; i < THING_ACTION_BAR_MAX; i++) {
        if (player_stats->action_bar[i].id == id) {
            if (index) {
                *index = i;
            }

            return (&player_stats->action_bar[i]);
        }
    }

    return (0);
}

itemp thing_stats_has_worn_item (thing_stats *player_stats,
                                  uint32_t id,
                                  uint32_t *index)
{
    uint32_t i;

    for (i = 0; i < THING_WORN_MAX; i++) {
        if (player_stats->worn[i].id == id) {
            if (index) {
                *index = i;
            }

            return (&player_stats->worn[i]);
        }
    }

    return (0);
}

int thing_stats_item_add (thingp t,
                           thing_stats *player_stats,
                           item_t item)
{
    tpp it = id_to_tp(item.id);
    itemp oitem;
    uint32_t i;

    if (!item.quantity) {
        DIE("Bad quantity for item add %s", tp_short_name(it));
    }

    if (!item.id) {
        DIE("Bad ID for item add %s", tp_short_name(it));
    }
 
    /*
     * If the item is already on the action bar, try and push onto it.
     */
    oitem = thing_stats_has_action_bar_item(player_stats, item.id, 0);
    if (oitem) {
        if (item_push(oitem, item)) {
            return (true);
        }
    }

    /*
     * If the item is already on the inventor, try and push onto it.
     */
    oitem = thing_stats_has_inventory_item(player_stats, item.id, 0);
    if (oitem) {
        if (item_push(oitem, item)) {
            return (true);
        }
    }

    /*
     * If there is space on the action bar, add it.
     */
    if (tp_is_valid_for_action_bar(it)) {
        for (i = 0; i < THING_ACTION_BAR_MAX; i++) {
            oitem = &player_stats->action_bar[i];
            if (item_push(oitem, item)) {
                return (true);
            }
        }
    }

    /*
     * Can it be worn?
     */
    if (tp_is_armor(it)) {
        oitem = &player_stats->worn[THING_WORN_ARMOR];
        if (item_push(oitem, item)) {
            return (true);
        }
    }

    if (tp_is_helmet(it)) {
        oitem = &player_stats->worn[THING_WORN_HELMET];
        if (item_push(oitem, item)) {
            return (true);
        }
    }

    if (tp_is_boots(it)) {
        oitem = &player_stats->worn[THING_WORN_BOOTS];
        if (item_push(oitem, item)) {
            return (true);
        }
    }

    if (tp_is_hand_item(it)) {
        oitem = &player_stats->worn[THING_WORN_ARM_LEFT];
        if (item_push(oitem, item)) {
            return (true);
        }

        oitem = &player_stats->worn[THING_WORN_ARM_RIGHT];
        if (item_push(oitem, item)) {
            return (true);
        }
    }

    /*
     * Else just find a free slot in the inventory.
     */
    for (i = 0; i < THING_INVENTORY_MAX; i++) {
        oitem = &player_stats->inventory[i];
        if (item_push(oitem, item)) {
            return (true);
        }
    }

    if (t) {
        THING_SHOUT_AT(t, INFO, "Carrying too many items");
    }

    return (false);
}

int thing_stats_item_remove (thingp t,
                             thing_stats *player_stats,
                             const tpp it)
{
    const int id = tp_to_id(it);

    itemp item = thing_stats_has_item(player_stats, id, 0);
    if (!item) {
        if (t) {
            THING_SHOUT_AT(t, INFO, "Not carrying the %s",
                           tp_short_name(it));
        }
        return (false);
    }

    if (!item_pop(item, 0)) {
        return (false);
    }

    return (true);
}

/*
 * Change all items the player is carrying of "from" into "to". e.g. change
 * all water to poisoned water.
 */
int thing_stats_item_polymorph (thing_stats *player_stats,
                                 const uint32_t from,
                                 const uint32_t to)
{
    itemp from_item = thing_stats_has_item(player_stats, from, 0);

    /*
     * If not carrying, nothing to change.
     */
    if (!from_item) {
        return (false);
    }

    from_item->id = to;

    return (true);
}

static void thing_stats_get_random_items (thing_stats *player_stats) 
{
    int count = gaussrand(2, 1) + 1;

    while (count--) {
        tpp t = 0;

        for (;;) {
            t = id_to_tp(myrand() % THING_MAX);

            if (!tp_is_carryable(t)) {
                continue;
            }

            if (!tp_is_given_randomly_at_start(t)) {
                continue;
            }

            uint32_t chance = myrand() % 10000;

            if (tp_get_d10000_chance_of_appearing(t) < chance) {
                continue;
            }

            break;
        }

        int quality = (myrand() % (THING_ITEM_QUALITY_MAX - 1)) + 1;

        item_t i = { 0 };
        i.id = tp_to_id(t);
        i.quantity = 1;
        i.quality = quality;

        const tpp item_tp = id_to_tp(i.id);

        if (tp_can_be_enchanted(item_tp)) {
            i.enchanted = item_enchant_randomly();
        }

        if (i.enchanted) {
            /*
             * Lucky.
             */
            LOG("  Auto provision %s, quality %u, +%u", 
                tp_short_name(t), quality, i.enchanted);
        } else {
            LOG("  Auto provision %s, quality %u", tp_short_name(t), quality);
        }

        thing_stats_item_add(0 /* thing */, player_stats, i);
    }
}

static void player_stats_generate_fixed_items (thing_stats *player_stats) 
{
    const tpp tp = thing_stats_to_tp(player_stats);

    /*
     * Start with items defined for this base class.
     */
    uint32_t i;

    for (i = 0; i < THING_MAX; i++) {

        if (!tp->base_items[i].quantity) {
            continue;
        }

        tpp tp_item;

        tp_item = id_to_tp(i);

        /*
         * Only top quality items to start.
         */
        item_t item = {0};

        item.id = i;
        item.quantity = tp_item->item.quantity;
        item.quality = THING_ITEM_QUALITY_MAX;
        item.enchanted = item_enchant_randomly();

        const char *carried_as = tp_carried_as(tp_item);
        if (carried_as) {
            tpp what = tp_find(carried_as);
            if (!what) {
                DIE("could now find %s to auto carry item as", 
                    tp_name(what));
            }

            item.id = tp_to_id(what);
        }

        thing_stats_item_add(0 /* thing */, player_stats, item);
    }
}

void thing_stats_get_random (thing_stats *player_stats) 
{
    LOG("Generate random character");

    strncpy(player_stats->pclass, pclass_random(),
            sizeof(player_stats->pclass) - 1);

    const tpp tp = thing_stats_to_tp(player_stats);

    thing_stats_init(player_stats);

    if (!wid_player_info_set_name) {
        strncpy(player_stats->pname, name_random(player_stats->pclass),
                sizeof(player_stats->pname) - 1);
    }

    player_stats->spending_points = player_stats_generate_spending_points();

    /*
     * attack_melee
     */
    player_stats->attack_melee = tp_get_stats_attack_melee(tp);
    player_stats->attack_melee = gaussrand(player_stats->attack_melee, 2);

    /*
     * attack_ranged
     */
    player_stats->attack_ranged = tp_get_stats_attack_ranged(tp);
    player_stats->attack_ranged = gaussrand(player_stats->attack_ranged, 2);

    /*
     * attack_magical
     */
    player_stats->attack_magical = tp_get_stats_attack_magical(tp);
    player_stats->attack_magical = gaussrand(player_stats->attack_magical, 2);


    /*
     * defense
     */
    player_stats->defense = tp_get_stats_defense(tp);
    player_stats->defense = gaussrand(player_stats->defense, 2);

    /*
     * speed
     */
    player_stats->speed = tp_get_stats_speed(tp);
    player_stats->speed = gaussrand(player_stats->speed, 2);

    /*
     * vision
     */
    player_stats->vision = tp_get_stats_vision(tp);
    player_stats->vision = gaussrand(player_stats->vision, 2);

    /*
     * healing
     */
    player_stats->healing = tp_get_stats_healing(tp);
    player_stats->healing = gaussrand(player_stats->healing, 2);

    /*
     * cash
     */
    player_stats->cash = tp_get_stats_cash(tp);

    /*
     * hp
     */
    player_stats->hp = tp_get_stats_max_hp(tp);
    player_stats->max_hp = 
        player_stats->hp = gaussrand(player_stats->hp,
                                     player_stats->hp / 10);

    /*
     * id
     */
    player_stats->magic = tp_get_stats_max_magic(tp);
    player_stats->max_magic = 
        player_stats->magic = gaussrand(player_stats->magic,
                                     player_stats->magic / 10);

    LOG(" %20s %s", "Name", player_stats->pname);
    LOG(" %20s %s", "Class", player_stats->pclass);
    LOG(" %20s %d", "Points", player_stats->spending_points);
    LOG(" %20s %d", "Hp", player_stats->hp);
    LOG(" %20s %d", "Cash", player_stats->cash);
    LOG(" %20s %d", "Max Hp", player_stats->max_hp);
    LOG(" %20s %d", "Magic", player_stats->magic);
    LOG(" %20s %d", "Max Magic", player_stats->max_magic);
    LOG(" %20s %d", "Experience", player_stats->xp);
    LOG(" %20s %d", "Spending Points", player_stats->spending_points);
    LOG(" %20s %d", "Attack Melee", player_stats->attack_melee);
    LOG(" %20s %d", "Attack Ranged", player_stats->attack_ranged);
    LOG(" %20s %d", "Attack Magical", player_stats->attack_magical);
    LOG(" %20s %d", "Defense", player_stats->defense);
    LOG(" %20s %d", "Speed", player_stats->speed);
    LOG(" %20s %d", "Vision", player_stats->vision);
    LOG(" %20s %d", "Healing", player_stats->healing);

    /*
     * Be generous and give some items at startup.
     */
    player_stats_generate_fixed_items(player_stats);

    /*
     * Be generous and give some items at startup.
     */
    thing_stats_get_random_items(player_stats);
}

void thing_stats_init (thing_stats *player_stats) 
{
    memset(player_stats->inventory, 0, sizeof(player_stats->inventory));
    memset(player_stats->action_bar, 0, sizeof(player_stats->action_bar));
    memset(player_stats->worn, 0, sizeof(player_stats->worn));

    /*
     * Do not memset carrying as that removes base class items.
     */

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

tpp
thing_stats_to_tp (thing_stats *player_stats)
{
    return (tp_find_short_name(player_stats->pclass));
}
