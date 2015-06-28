/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
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
#include "thing.h"
#include "client.h"
#include "socket_util.h"

void thing_stats_dump (const thing_statsp s)
{
    LOG("  stats (version %d, %s):",
        s->client_version, s->on_server ? "on server" : "on client");

    LOG("    %-20s %4d %-20s %4d", 
        "hp", s->hp, 
        "max-hp", s->max_hp);

    LOG("    %-20s %4d %-20s %4d", 
        "magic", s->magic, 
        "max-magic", s->max_magic);

    LOG("    %-20s %4u %-20s %4u", 
        "xp", s->xp, 
        "cash", s->cash);

    LOG("    %-20s %4u %-20s %4u", 
        "spending", s->spending_points, 
        "vision", s->vision);

    LOG("    %-20s %4u %-20s %4u", 
        "attack_melee", s->attack_melee, 
        "attack_ranged", s->attack_ranged);

    LOG("    %-20s %4u %-20s %4u", 
        "attack_magical", s->attack_magical, 
        "defense", s->defense);

    LOG("    %-20s %4u %-20s %4u", 
        "speed", s->speed, 
        "healing", s->healing);

    LOG("    %-20s %4u %-20s %4u", 
        "action bar", s->action_bar_index, 
        "-", 0);

    {
        LOG("  inventory:");

        char *item_str = 0;
        int i = 0;

        while (i < THING_INVENTORY_MAX) {
            item_t a = s->inventory[i++];

            char *tmp = item2str(a);
            if (tmp) {
                char *tmp2 = dynprintf("[%d] %s", i, tmp);
                swap(tmp, tmp2);
                myfree(tmp2);

                if (item_str) {
                    LOG("    %-30s %-30s", item_str, tmp);
                    myfree(item_str);
                    myfree(tmp);
                    item_str = 0;
                } else {
                    item_str = tmp;
                }
            }
        }

        if (item_str) {
            LOG("    %-30s", item_str);
            myfree(item_str);
        }
    }

    {
        LOG("  action bar:");

        int i;
        for (i = 0; i < THING_ACTION_BAR_MAX; i+=2) {
            item_t a = s->action_bar[i];
            item_t b = s->action_bar[i + 1];

            char *ia = item2str(a);
            char *ib = item2str(b);

            LOG("    (%d) %-30s (%d) %-30s",
                i, ia ? ia : "nothing", 
                i + 1, ib ? ib : "nothing");

            if (ia) {
                myfree(ia);
            }

            if (ib) {
                myfree(ib);
            }
        }
    }

    {
        LOG("  worn:");

        {
            item_t a = s->worn[THING_WORN_ARMOR];
            item_t b = s->worn[THING_WORN_HELMET];

            char *ia = item2str(a);
            char *ib = item2str(b);

            LOG("    %-10s: %-30s %-10s: %-30s", 
                "armour", ia ? ia : "-", 
                "helmet", ib ? ib : "-");

            if (ia) {
                myfree(ia);
            }

            if (ib) {
                myfree(ib);
            }
        }

        {
            item_t a = s->worn[THING_WORN_ARM_LEFT];
            item_t b = s->worn[THING_WORN_ARM_RIGHT];

            char *ia = item2str(a);
            char *ib = item2str(b);

            LOG("    %-10s: %-30s %-10s: %-30s", 
                "arm left", ia ? ia : "-", 
                "arm right", ib ? ib : "-");

            if (ia) {
                myfree(ia);
            }

            if (ib) {
                myfree(ib);
            }
        }

        {
            item_t a = s->worn[THING_WORN_BOOTS];
            char *ia = item2str(a);

            LOG("    %-10s: %-30s",
                "boots", ia ? ia : "-");

            if (ia) {
                myfree(ia);
            }
        }
    }
}

/*
 * Dump what changed between two stats
 */
int thing_stats_diff (const thing_statsp old_stats, 
                      const thing_statsp new_stats)
{
    const char indent[] = "  ";
    int changed = 0;

    if (old_stats->on_server != new_stats->on_server) {
        changed = 1;
    }

    if (old_stats->client_version != new_stats->client_version) {
        LOG("%sVersion changed from %d to %d", indent, 
            old_stats->client_version, new_stats->client_version);
        changed = 1;
    }

    if (old_stats->hp != new_stats->hp) {
        LOG("%sHp changed from %d to %d", indent, 
            old_stats->hp, new_stats->hp);
        changed = 1;
    }

    if (old_stats->max_hp != new_stats->max_hp) {
        LOG("%smax_hp changed from %d to %d", indent, 
            old_stats->max_hp, new_stats->max_hp);
        changed = 1;
    }

    if (old_stats->shield != new_stats->shield) {
        LOG("%sshield changed from %d to %d", indent, 
            old_stats->shield, new_stats->shield);
        changed = 1;
    }

    if (old_stats->magic != new_stats->magic) {
        LOG("%sMagic changed from %d to %d", indent, 
            old_stats->magic, new_stats->magic);
        changed = 1;
    }

    if (old_stats->max_magic != new_stats->max_magic) {
        LOG("%smax_magic changed from %d to %d", indent, 
            old_stats->max_magic, new_stats->max_magic);
        changed = 1;
    }

    if (old_stats->xp != new_stats->xp) {
        LOG("%sXp changed from %d to %d", indent, 
            old_stats->xp, new_stats->xp);
        changed = 1;
    }

    if (old_stats->cash != new_stats->cash) {
        LOG("%sCash changed from %d to %d", indent, 
            old_stats->cash, new_stats->cash);
        changed = 1;
    }

    if (old_stats->spending_points != new_stats->spending_points) {
        LOG("%sSpending_points changed from %d to %d", indent, 
            old_stats->spending_points, new_stats->spending_points);
        changed = 1;
    }

    if (old_stats->vision != new_stats->vision) {
        LOG("%sVision changed from %d to %d", indent, 
            old_stats->vision, new_stats->vision);
        changed = 1;
    }

    if (old_stats->attack_melee != new_stats->attack_melee) {
        LOG("%sAttack_melee changed from %d to %d", indent, 
            old_stats->attack_melee, new_stats->attack_melee);
        changed = 1;
    }

    if (old_stats->attack_ranged != new_stats->attack_ranged) {
        LOG("%sAttack_ranged changed from %d to %d", indent, 
            old_stats->attack_ranged, new_stats->attack_ranged);
        changed = 1;
    }

    if (old_stats->attack_magical != new_stats->attack_magical) {
        LOG("%sAttack_magical changed from %d to %d", indent, 
            old_stats->attack_magical, new_stats->attack_magical);
        changed = 1;
    }

    if (old_stats->defense != new_stats->defense) {
        LOG("%sDefense changed from %d to %d", indent, 
            old_stats->defense, new_stats->defense);
        changed = 1;
    }

    if (old_stats->speed != new_stats->speed) {
        LOG("%sSpeed changed from %d to %d", indent, 
            old_stats->speed, new_stats->speed);
        changed = 1;
    }

    if (old_stats->vision != new_stats->vision) {
        LOG("%sVision changed from %d to %d", indent, 
            old_stats->vision, new_stats->vision);
        changed = 1;
    }

    if (old_stats->healing != new_stats->healing) {
        LOG("%sHealing changed from %d to %d", indent, 
            old_stats->healing, new_stats->healing);
        changed = 1;
    }

    if ((old_stats->level_pos.x != new_stats->level_pos.x) ||
        (old_stats->level_pos.y != new_stats->level_pos.y)) {
        LOG("%sVersion changed from %d.%d to %d.%d", indent, 
            old_stats->level_pos.x, 
            old_stats->level_pos.y, 
            new_stats->level_pos.x,
            new_stats->level_pos.y);
        changed = 1;
    }

    if (old_stats->action_bar_index != new_stats->action_bar_index) {
        LOG("%sAction index   changed from %d to %d", indent, 
            old_stats->action_bar_index, new_stats->action_bar_index);
        changed = 1;
    }

    {
        int i = 0;

        for (i = 0; i < THING_INVENTORY_MAX; i++) {
            item_t a = old_stats->inventory[i];
            item_t b = new_stats->inventory[i];

            if (memcmp(&a, &b, sizeof(a))) {
                changed = 1;

                char *ia = item2str(a);
                char *ib = item2str(b);

                if (ia && ib) {
                    LOG("%sItem changed, %s to %s", indent, ia, ib);
                    myfree(ia);
                    myfree(ib);
                } else if (ia) {
                    LOG("%sItem removed, %s", indent, ia);
                    myfree(ia);
                } else if (ib) {
                    LOG("%sItem added, %s", indent, ib);
                    myfree(ib);
                }
            }
        }
    }

    {
        int i;
        for (i = 0; i < THING_ACTION_BAR_MAX; i++) {
            item_t a = old_stats->action_bar[i];
            item_t b = new_stats->action_bar[i];

            if (memcmp(&a, &b, sizeof(a))) {
                changed = 1;

                char *ia = item2str(a);
                char *ib = item2str(b);

                if (ia && ib) {
                    LOG("%sAction item %d changed, %s to %s", indent, 
                        i, ia, ib);
                    myfree(ia);
                    myfree(ib);
                } else if (ia) {
                    LOG("%sAction item %d removed, %s", indent, i, ia);
                    myfree(ia);
                } else if (ib) {
                    LOG("%sAction item %d added, %s", indent, i, ib);
                    myfree(ib);
                }
            }
        }
    }

    {
        int i;
        for (i = 0; i < THING_WORN_MAX; i++) {
            item_t a = old_stats->worn[i];
            item_t b = new_stats->worn[i];

            if (memcmp(&a, &b, sizeof(a))) {
                changed = 1;

                char *ia = item2str(a);
                char *ib = item2str(b);

                if (ia && ib) {
                    LOG("%sWorn item %d changed, %s to %s", indent, i, ia, ib);
                    myfree(ia);
                    myfree(ib);
                } else if (ia) {
                    LOG("%sWorn item %d removed, %s", indent, i, ia);
                    myfree(ia);
                } else if (ib) {
                    LOG("%sWorn item %d added, %s", indent, i, ib);
                    myfree(ib);
                }
            }
        }
    }

    if (!changed) {
        if (memcmp(old_stats, new_stats, sizeof(*old_stats))) {
            hex_dump_log(old_stats, 0, sizeof(*old_stats));
            hex_dump_log(new_stats, 0, sizeof(*new_stats));
            ERR("stats differ bug is unable to debug the difference; fixme");
        }
    }

    return (changed);
}

/*
 * Dump what changed between two stats
 */
int thing_stats_merge (thing_statsp merged_stats, thing_statsp current_stats, thing_statsp new_stats)
{
    int changed = false;

    if (current_stats->hp != new_stats->hp) {
        merged_stats->hp = new_stats->hp;
        changed = true;
    }

    if (current_stats->magic != new_stats->magic) {
        merged_stats->magic = new_stats->magic;
        changed = true;
    }

    if (current_stats->xp != new_stats->xp) {
        merged_stats->xp = new_stats->xp;
        changed = true;
    }

    if (current_stats->cash != new_stats->cash) {
        merged_stats->cash = new_stats->cash;
        changed = true;
    }

    if (current_stats->client_version != new_stats->client_version) {
        merged_stats->client_version = new_stats->client_version;
        changed = true;
    }

    if (current_stats->spending_points != new_stats->spending_points) {
        merged_stats->spending_points = new_stats->spending_points;
        changed = true;
    }

    if (current_stats->vision != new_stats->vision) {
        merged_stats->vision = new_stats->vision;
        changed = true;
    }

    if (current_stats->attack_melee != new_stats->attack_melee) {
        merged_stats->attack_melee = new_stats->attack_melee;
        changed = true;
    }

    if (current_stats->attack_ranged != new_stats->attack_ranged) {
        merged_stats->attack_ranged = new_stats->attack_ranged;
        changed = true;
    }

    if (current_stats->defense != new_stats->defense) {
        merged_stats->defense = new_stats->defense;
        changed = true;
    }

    if (current_stats->attack_magical != new_stats->attack_magical) {
        merged_stats->attack_magical = new_stats->attack_magical;
        changed = true;
    }

    if (current_stats->action_bar_index != new_stats->action_bar_index) {
        merged_stats->action_bar_index = new_stats->action_bar_index;
        changed = true;
    }

    {
        int i = 0;

        for (i = 0; i < THING_INVENTORY_MAX; i++) {
            item_t a = current_stats->inventory[i];
            item_t b = new_stats->inventory[i];
            item_t *c = &merged_stats->inventory[i];

            if (memcmp(&a, &b, sizeof(a))) {
                memcpy(c, &b, sizeof(b));
                changed = true;
            }
        }
    }

    {
        int i;
        for (i = 0; i < THING_ACTION_BAR_MAX; i++) {
            item_t a = current_stats->action_bar[i];
            item_t b = new_stats->action_bar[i];
            item_t *c = &merged_stats->action_bar[i];

            if (memcmp(&a, &b, sizeof(a))) {
                memcpy(c, &b, sizeof(b));
                changed = true;
            }
        }
    }

    {
        int i;
        for (i = 0; i < THING_WORN_MAX; i++) {
            item_t a = current_stats->worn[i];
            item_t b = new_stats->worn[i];
            item_t *c = &merged_stats->worn[i];

            if (memcmp(&a, &b, sizeof(a))) {
                memcpy(c, &b, sizeof(b));
                changed = true;
            }
        }
    }

    return (changed);
}

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
        ERR("bug, dst should have quantity");
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
        ERR("bug, cannot pop, no quantity");
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

int item_pop_all (itemp dst, itemp popped)
{
    if (!dst->id) {
        return (false);
    }

    if (!dst->quantity) {
        ERR("bug, cannot pop, no quantity");
    }

    memcpy(popped, dst, sizeof(item_t));
    memset(dst, 0, sizeof(item_t));

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
        modifiers[14] =              +1;
        modifiers[15] =              +1;
        modifiers[16] =              +2;
        modifiers[17] =              +2;
        modifiers[18] =              +2;
        modifiers[19] =              +2;
        modifiers[20] =              +2;
        modifiers[21] =              +3;
        modifiers[22] =              +3;
        modifiers[23] =              +3;
        modifiers[24] =              +4;
        modifiers[25] =              +4;
        modifiers[26] =              +4;
        modifiers[27] =              +5;
        modifiers[28] =              +5;
        modifiers[29] =              +5;
        modifiers[30] =              +6;
        modifiers[31] =              +6;
        modifiers[32] =              +6;
        modifiers[33] =              +7;
        modifiers[34] =              +7;
        modifiers[35] =              +7;
        modifiers[36] =              +8;
        modifiers[37] =              +8;
        modifiers[38] =              +8;
        modifiers[39] =              +9;
        modifiers[40] =              +9;
        modifiers[41] =              +9;
        modifiers[42] =              +10;
        modifiers[43] =              +10;
        modifiers[44] =              +10;
    }

    if (value < 0) {
        return (0);
    }

    if (value >= (int) ARRAY_SIZE(modifiers)) {
        value = (int) ARRAY_SIZE(modifiers);
    }

    return (modifiers[value]);
}

void player_inventory_sort (thing_statsp player_stats)
{
    if (!player_stats) {
        ERR("no player stats");
    }

    static const int32_t NCLASSES = 
                    THING_INVENTORY_CLASSES;
    static const int32_t N_PER_CLASS = 
                    THING_INVENTORY_MAX / THING_INVENTORY_CLASSES;

    item_t inv_new[THING_INVENTORY_MAX];
    item_t inv_old[THING_INVENTORY_MAX];
    int32_t count_per_class[THING_INVENTORY_CLASSES];

    memcpy(inv_old, &player_stats->inventory, sizeof(inv_old));
    memset(inv_new, 0, sizeof(inv_new));
    memset(count_per_class, 0, sizeof(count_per_class));

    int32_t i;

    /*
     * For each item in the inventory, move it to a sorted slot based on its 
     * type.
     */
    for (i = 0; i < THING_INVENTORY_MAX; i++) {
        int32_t id;

        id = inv_old[i].id;
        if (!id) {
            continue;
        }

        /*
         * Before we look for a sorting slot for this item, see if one of its 
         * kind has already been sorted. This way we can merge together 
         * multiple torches into one bundle.
         */
        int merged = 0;
        int32_t j;

        for (j = 0; j < THING_INVENTORY_MAX; j++) {
            int32_t id2;

            /*
             * Look for similar items. They might not be identical i.e. one 
             * enchanted and the other not. Those we can't merge.
             */
            id2 = inv_new[j].id;
            if (id != id2) {
                continue;
            }

            itemp dst = &inv_new[j];
            item_t src = inv_old[i];

            if (item_push(dst, src)) {
                /*
                 * Woo hoo, we merged this item onto an existing item.
                 */
                merged = 1;
                break;
            }

            /*
             * Can't merge this item. Maybe it contains too many of its kind.
             */
        }

        if (merged) {
            continue;
        }

        /*
         * Ok, this item is not in the new sorted list yet, look for somewhere 
         * (a sorted slot) to fit it in.
         */
        int32_t base = THING_INVENTORY_MISC_BASE;

        tpp tp = id_to_tp(id);

        /*
         * Find in which sorting slot the item should move.
         */
        if (tp_is_magical(tp)) {
            base = THING_INVENTORY_MAGICAL_BASE;
        } else if (tp_is_weapon(tp)) {
            base = THING_INVENTORY_WEAPON_BASE;
        } else if (tp_is_spell(tp) || tp_is_potion(tp)) {
            base = THING_INVENTORY_MAGICAL_BASE;
        } else if (tp_is_food(tp)) {
            base = THING_INVENTORY_FOOD_BASE;
        }

        /*
         * If that destination sort slot is full, try any slot to fit the item 
         * in.
         */
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

        /*
         * Add this new unique item into the sorted section.
         */
        int32_t new_i = base + count_per_class[which];
        if (new_i >= THING_INVENTORY_MAX) {
            ERR("overflow in item sorting");
            continue;
        }

        inv_new[new_i] = inv_old[i];
        count_per_class[which]++;
    }

    memcpy(&player_stats->inventory, inv_new, sizeof(inv_new));
}

itemp thing_stats_has_item (thing_statsp player_stats,
                             int32_t id,
                             int32_t *index)
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

itemp thing_stats_has_inventory_item (thing_statsp player_stats,
                                       int32_t id,
                                       int32_t *index)
{
    int32_t i;

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

itemp thing_stats_has_action_bar_item (thing_statsp player_stats,
                                      int32_t id,
                                      int32_t *index)
{
    int32_t i;

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

itemp thing_stats_has_worn_item (thing_statsp player_stats,
                                  int32_t id,
                                  int32_t *index)
{
    int32_t i;

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
                          thing_statsp player_stats,
                          item_t item)
{
    tpp it = id_to_tp(item.id);
    itemp oitem;
    int32_t i;

    if (!item.quantity) {
        ERR("Bad quantity for item add %s", tp_short_name(it));
    }

    if (!item.id) {
        ERR("Bad ID for item add %s", tp_short_name(it));
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
     * All items go on the ation bar first if there is space.
     */
    for (i = 0; i < THING_ACTION_BAR_MAX; i++) {
        oitem = &player_stats->action_bar[i];
        if (item_push(oitem, item)) {
            return (true);
        }
    }

    /*
     * If the item is already on the inventory, try and push onto it.
     */
    oitem = thing_stats_has_inventory_item(player_stats, item.id, 0);
    if (oitem) {
        if (item_push(oitem, item)) {
            return (true);
        }
    }

    /*
     * Can it be worn?
     */
    if (tp_is_armour(it)) {
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
        MSG_SERVER_SHOUT_AT_PLAYER(INFO, t, "Carrying too many items");
    }

    return (false);
}

int thing_stats_item_remove (thingp t,
                             thing_statsp player_stats,
                             const tpp it)
{
    const int id = tp_to_id(it);

    itemp item = thing_stats_has_item(player_stats, id, 0);
    if (!item) {
        if (t) {
            MSG_SERVER_SHOUT_AT_PLAYER(INFO, t, "Not carrying the %s",
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
int thing_stats_item_polymorph (thing_statsp player_stats,
                                 const int32_t from,
                                 const int32_t to)
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

static void thing_stats_get_random_items (thing_statsp player_stats) 
{
    int count = gaussrand(2, 1) + 1;

    while (count-- > 0) {
        tpp t = 0;

        for (;;) {
            t = id_to_tp(myrand() % THING_MAX);

            if (!tp_is_carryable(t)) {
                continue;
            }

            if (!tp_is_given_randomly_at_start(t)) {
                continue;
            }

            int32_t chance = myrand() % 10000;

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

static void player_stats_generate_fixed_items (thing_statsp player_stats) 
{
    const tpp tp = thing_stats_to_tp(player_stats);

    /*
     * Start with items defined for this base class.
     */
    int32_t i;

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
                ERR("could now find %s to auto carry item as", 
                    tp_name(what));
            }

            item.id = tp_to_id(what);
        }

        thing_stats_item_add(0 /* thing */, player_stats, item);
    }
}

void thing_stats_get_random (thing_statsp player_stats,
                             int new_random_name_and_class) 
{
    LOG("Generate random character");

    if (!player_stats->pclass[0] || new_random_name_and_class) {
        strncpy(player_stats->pclass, pclass_random(),
                sizeof(player_stats->pclass) - 1);
    }

    const tpp tp = thing_stats_to_tp(player_stats);

    thing_stats_init(player_stats);

    /*
     * If the player ever set a name manually then never override it on 
     * rerolls
     */
    if (!wid_player_info_set_name) {
        /*
         * If no name of a force of a new name, make one up
         */
        if (!player_stats->pname[0] || new_random_name_and_class) {
            strncpy(player_stats->pname, name_random(player_stats->pclass),
                    sizeof(player_stats->pname) - 1);
        }
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
     * We usually call this on the client
     */
    player_stats->on_server = 0;

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

    player_inventory_sort(player_stats);

    LOG("Done generating random character");
}

void thing_stats_init (thing_statsp player_stats) 
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
thing_stats_to_tp (thing_statsp player_stats)
{
    return (tp_find_short_name(player_stats->pclass));
}

/*
 * If the stats have changed with those on the socket, update the server.
 */
void thing_stats_client_modified (thing_statsp player_stats) 
{
    if (!client_joined_server) {
        return;
    }

    if (socket_set_player_stats(client_joined_server, player_stats)) {
        /*
         * Something changed.
         */
        socket_tx_client_status(client_joined_server);
    }
}

int32_t thing_stats_get_cash (thingp t)
{
    int32_t val;

    verify(t);

    val = t->stats.cash;
    return (val);
}

int32_t thing_stats_get_client_version (thingp t)
{
    int32_t val;

    verify(t);

    val = t->stats.client_version;
    return (val);
}

int32_t thing_stats_get_on_server (thingp t)
{
    int32_t val;

    verify(t);

    val = t->stats.on_server;
    return (val);
}

int32_t thing_stats_get_hp (thingp t)
{
    int32_t val;

    verify(t);

    val = t->stats.hp;
    return (val);
}

int32_t thing_stats_get_action_bar_index (thingp t)
{
    int32_t val;

    verify(t);

    val = t->stats.action_bar_index;
    return (val);
}

int32_t thing_stats_get_max_hp (thingp t)
{
    int32_t val;

    verify(t);

    val = t->stats.max_hp;
    if (val) {
        return (val);
    }

    val = tp_get_stats_max_hp(t->tp);
    return (val);
}

int32_t thing_stats_get_magic (thingp t)
{
    int32_t val;

    verify(t);

    val = t->stats.magic;
    return (val);
}

int32_t thing_stats_get_max_magic (thingp t)
{
    int32_t val;

    verify(t);

    val = t->stats.max_magic;
    if (val) {
        return (val);
    }

    val = tp_get_stats_max_magic(t->tp);
    return (val);
}

int32_t thing_stats_get_xp (thingp t)
{
    int32_t val;

    verify(t);

    val = t->stats.xp;
    return (val);
}

int32_t thing_stats_get_attack_melee (thingp t)
{
    int32_t val;

    verify(t);

    val = t->stats.attack_melee;
    if (val) {
        return (val);
    }

    val = tp_get_stats_attack_melee(t->tp);
    return (val);
}

int32_t thing_stats_get_attack_ranged (thingp t)
{
    int32_t val;

    verify(t);

    val = t->stats.attack_ranged;
    if (val) {
        return (val);
    }

    val = tp_get_stats_attack_ranged(t->tp);
    return (val);
}

int32_t thing_stats_get_attack_magical (thingp t)
{
    int32_t val;

    verify(t);

    val = t->stats.attack_magical;
    if (val) {
        return (val);
    }

    val = tp_get_stats_attack_magical(t->tp);
    return (val);
}

int32_t thing_stats_get_speed (thingp t)
{
    int32_t val;

    verify(t);

    val = t->stats.speed;
    if (val) {
        return (val);
    }

    val = tp_get_stats_speed(t->tp);
    return (val);
}

int32_t thing_stats_get_vision (thingp t)
{
    int32_t val;

    verify(t);

    val = t->stats.vision;
    if (val) {
        return (val);
    }

    val = tp_get_stats_vision(t->tp);
    return (val);
}

int32_t thing_stats_get_healing (thingp t)
{
    int32_t val;

    verify(t);

    val = t->stats.healing;
    if (val) {
        return (val);
    }

    val = tp_get_stats_healing(t->tp);
    return (val);
}

int32_t thing_stats_get_defense (thingp t)
{
    int32_t val;

    verify(t);

    val = t->stats.defense;
    if (val) {
        return (val);
    }

    val = tp_get_stats_defense(t->tp);
    return (val);
}

static void thing_stats_verify (thingp t)
{
    verify(t);

    if (t->on_server != t->stats.on_server) {
        ERR("%s thing stats location do not match thing", thing_logname(t));
    }
}

void thing_stats_set_cash (thingp t, int32_t val)
{
    thing_stats_verify(t);

    stats_set_cash(&t->stats, val);
}

void thing_stats_set_client_version (thingp t, int32_t val)
{
    thing_stats_verify(t);

    stats_set_client_version(&t->stats, val);
}

void thing_stats_set_on_server (thingp t, int32_t val)
{
    verify(t);

    stats_set_on_server(&t->stats, val);
}

void thing_stats_set_action_bar_index (thingp t, int32_t val)
{
    thing_stats_verify(t);

    stats_set_action_bar_index(&t->stats, val);
}

void thing_stats_set_hp (thingp t, int32_t val)
{
    thing_stats_verify(t);

    stats_set_hp(&t->stats, val);
}

void thing_stats_set_max_hp (thingp t, int32_t val)
{
    thing_stats_verify(t);

    stats_set_max_hp(&t->stats, val);
}

void thing_stats_set_magic (thingp t, int32_t val)
{
    thing_stats_verify(t);

    stats_set_magic(&t->stats, val);
}

void thing_stats_set_max_magic (thingp t, int32_t val)
{
    thing_stats_verify(t);

    stats_set_max_magic(&t->stats, val);
}

void thing_stats_set_xp (thingp t, int32_t val)
{
    thing_stats_verify(t);

    stats_set_xp(&t->stats, val);
}

void thing_stats_set_attack_melee (thingp t, int32_t val)
{
    thing_stats_verify(t);

    stats_set_attack_melee(&t->stats, val);
}

void thing_stats_set_attack_ranged (thingp t, int32_t val)
{
    thing_stats_verify(t);

    stats_set_attack_ranged(&t->stats, val);
}

void thing_stats_set_attack_magical (thingp t, int32_t val)
{
    thing_stats_verify(t);

    stats_set_attack_magical(&t->stats, val);
}

void thing_stats_set_speed (thingp t, int32_t val)
{
    thing_stats_verify(t);

    stats_set_speed(&t->stats, val);
}

void thing_stats_set_spending_points (thingp t, int32_t val)
{
    thing_stats_verify(t);

    stats_set_spending_points(&t->stats, val);
}

void thing_stats_set_vision (thingp t, int32_t val)
{
    thing_stats_verify(t);

    stats_set_vision(&t->stats, val);
}

void thing_stats_set_healing (thingp t, int32_t val)
{
    thing_stats_verify(t);

    stats_set_healing(&t->stats, val);
}

void thing_stats_set_defense (thingp t, int32_t val)
{
    thing_stats_verify(t);

    stats_set_defense(&t->stats, val);
}

void thing_stats_modify_cash (thingp t, int32_t val)
{
    thing_stats_verify(t);

    if (val > 0) {
        MSG_SERVER_SHOUT_AT_PLAYER(SOUND, t, "treasure");
    }

    stats_modify_cash(&t->stats, val);
}

void thing_stats_modify_client_version (thingp t, int32_t val)
{
    thing_stats_verify(t);

    stats_modify_client_version(&t->stats, val);
}

void thing_stats_modify_hp (thingp t, int32_t val)
{
    thing_stats_verify(t);

    stats_modify_hp(&t->stats, val);
}

void thing_stats_modify_action_bar_index (thingp t, int32_t val)
{
    thing_stats_verify(t);

    stats_modify_action_bar_index(&t->stats, val);
}

void thing_stats_modify_max_hp (thingp t, int32_t val)
{
    thing_stats_verify(t);

    if (val > 0) {
        MSG_SERVER_SHOUT_AT_PLAYER(SOUND, t, "effect");
    }

    stats_modify_max_hp(&t->stats, val);
}

void thing_stats_modify_magic (thingp t, int32_t val)
{
    thing_stats_verify(t);

    stats_modify_magic(&t->stats, val);
}

void thing_stats_modify_max_magic (thingp t, int32_t val)
{
    thing_stats_verify(t);

    if (val > 0) {
        MSG_SERVER_SHOUT_AT_PLAYER(SOUND, t, "effect");
    }

    stats_modify_max_magic(&t->stats, val);
}

void thing_stats_modify_xp (thingp t, int32_t val)
{
    thing_stats_verify(t);

    stats_modify_xp(&t->stats, val);
}

void thing_stats_modify_attack_melee (thingp t, int32_t val)
{
    thing_stats_verify(t);

    if (val > 0) {
        MSG_SERVER_SHOUT_AT_PLAYER(SOUND, t, "effect");
    }

    stats_modify_attack_melee(&t->stats, val);
}

void thing_stats_modify_attack_ranged (thingp t, int32_t val)
{
    thing_stats_verify(t);

    if (val > 0) {
        MSG_SERVER_SHOUT_AT_PLAYER(SOUND, t, "effect");
    }

    stats_modify_attack_ranged(&t->stats, val);
}

void thing_stats_modify_attack_magical (thingp t, int32_t val)
{
    thing_stats_verify(t);

    if (val > 0) {
        MSG_SERVER_SHOUT_AT_PLAYER(SOUND, t, "effect");
    }

    stats_modify_attack_magical(&t->stats, val);
}

void thing_stats_modify_speed (thingp t, int32_t val)
{
    thing_stats_verify(t);

    if (val > 0) {
        MSG_SERVER_SHOUT_AT_PLAYER(SOUND, t, "effect");
    }

    stats_modify_speed(&t->stats, val);
}

void thing_stats_modify_spending_points (thingp t, int32_t val)
{
    thing_stats_verify(t);

    if (val > 0) {
        MSG_SERVER_SHOUT_AT_PLAYER(SOUND, t, "effect");
    }

    stats_modify_spending_points(&t->stats, val);
}

void thing_stats_modify_vision (thingp t, int32_t val)
{
    thing_stats_verify(t);

    if (val > 0) {
        MSG_SERVER_SHOUT_AT_PLAYER(SOUND, t, "effect");
    }

    stats_modify_vision(&t->stats, val);
}

void thing_stats_modify_healing (thingp t, int32_t val)
{
    thing_stats_verify(t);

    if (val > 0) {
        MSG_SERVER_SHOUT_AT_PLAYER(SOUND, t, "effect");
    }

    stats_modify_healing(&t->stats, val);
}

void thing_stats_modify_defense (thingp t, int32_t val)
{
    thing_stats_verify(t);

    if (val > 0) {
        MSG_SERVER_SHOUT_AT_PLAYER(SOUND, t, "effect");
    }

    stats_modify_defense(&t->stats, val);
}

int32_t stats_get_cash (thing_statsp stats)
{
    int32_t val;

    val = stats->cash;
    return (val);
}

int32_t stats_get_client_version (thing_statsp stats)
{
    int32_t val;

    val = stats->client_version;
    return (val);
}

int32_t stats_get_on_server (thing_statsp stats)
{
    int32_t val;

    val = stats->on_server;
    return (val);
}

int32_t stats_get_hp (thing_statsp stats)
{
    int32_t val;

    val = stats->hp;
    return (val);
}

int32_t stats_get_action_bar_index (thing_statsp stats)
{
    int32_t val;

    val = stats->action_bar_index;
    return (val);
}

int32_t stats_get_max_hp (thing_statsp stats)
{
    int32_t val;

    val = stats->max_hp;
    return (val);
}

int32_t stats_get_magic (thing_statsp stats)
{
    int32_t val;

    val = stats->magic;
    return (val);
}

int32_t stats_get_max_magic (thing_statsp stats)
{
    int32_t val;

    val = stats->max_magic;
    return (val);
}

int32_t stats_get_xp (thing_statsp stats)
{
    int32_t val;

    val = stats->xp;
    return (val);
}

int32_t stats_get_attack_melee (thing_statsp stats)
{
    int32_t val;

    val = stats->attack_melee;
    return (val);
}

int32_t stats_get_attack_ranged (thing_statsp stats)
{
    int32_t val;

    val = stats->attack_ranged;
    return (val);
}

int32_t stats_get_attack_magical (thing_statsp stats)
{
    int32_t val;

    val = stats->attack_magical;
    return (val);
}

int32_t stats_get_speed (thing_statsp stats)
{
    int32_t val;

    val = stats->speed;
    return (val);
}

int32_t stats_get_spending_points (thing_statsp stats)
{
    int32_t val;

    val = stats->spending_points;
    return (val);
}

int32_t stats_get_vision (thing_statsp stats)
{
    int32_t val;

    val = stats->vision;
    return (val);
}

int32_t stats_get_healing (thing_statsp stats)
{
    int32_t val;

    val = stats->healing;
    return (val);
}

int32_t stats_get_defense (thing_statsp stats)
{
    int32_t val;

    val = stats->defense;
    return (val);
}

void stats_bump_version (thing_statsp stats)
{
    if (!stats->on_server) {
        stats->client_version++;
    }
}

void stats_set_cash (thing_statsp stats, int32_t val)
{
    if (stats->cash == val) {
        return;
    }

    stats->cash = val;
    stats_bump_version(stats);
}

void stats_set_client_version (thing_statsp stats, int32_t val)
{
    stats->client_version = val;
}

void stats_set_on_server (thing_statsp stats, int32_t val)
{
    stats->on_server = val ? 1 : 0;
}

void stats_set_action_bar_index (thing_statsp stats, int32_t val)
{
    if (stats->action_bar_index == val) {
        return;
    }

    stats->action_bar_index = val;
    stats_bump_version(stats);
}

void stats_set_hp (thing_statsp stats, int32_t val)
{
    if (stats->hp == val) {
        return;
    }

    stats->hp = val;
    stats_bump_version(stats);
}

void stats_set_max_hp (thing_statsp stats, int32_t val)
{
    if (stats->max_hp == val) {
        return;
    }

    stats->max_hp = val;
    stats_bump_version(stats);
}

void stats_set_magic (thing_statsp stats, int32_t val)
{
    if (stats->magic == val) {
        return;
    }

    stats->magic = val;
    stats_bump_version(stats);
}

void stats_set_max_magic (thing_statsp stats, int32_t val)
{
    if (stats->max_magic == val) {
        return;
    }

    stats->max_magic = val;
    stats_bump_version(stats);
}

void stats_set_xp (thing_statsp stats, int32_t val)
{
    if (stats->xp == val) {
        return;
    }

    stats->xp = val;
    stats_bump_version(stats);
}

void stats_set_attack_melee (thing_statsp stats, int32_t val)
{
    if (stats->attack_melee == val) {
        return;
    }

    stats->attack_melee = val;
    stats_bump_version(stats);
}

void stats_set_attack_ranged (thing_statsp stats, int32_t val)
{
    if (stats->attack_ranged == val) {
        return;
    }

    stats->attack_ranged = val;
    stats_bump_version(stats);
}

void stats_set_attack_magical (thing_statsp stats, int32_t val)
{
    if (stats->attack_magical == val) {
        return;
    }

    stats->attack_magical = val;
    stats_bump_version(stats);
}

void stats_set_speed (thing_statsp stats, int32_t val)
{
    if (stats->speed == val) {
        return;
    }

    stats->speed = val;
    stats_bump_version(stats);
}

void stats_set_spending_points (thing_statsp stats, int32_t val)
{
    if (stats->spending_points == val) {
        return;
    }

    stats->spending_points = val;
    stats_bump_version(stats);
}

void stats_set_vision (thing_statsp stats, int32_t val)
{
    if (stats->vision == val) {
        return;
    }

    stats->vision = val;
    stats_bump_version(stats);
}

void stats_set_healing (thing_statsp stats, int32_t val)
{
    if (stats->healing == val) {
        return;
    }

    stats->healing = val;
    stats_bump_version(stats);
}

void stats_set_defense (thing_statsp stats, int32_t val)
{
    if (stats->defense == val) {
        return;
    }

    stats->defense = val;
    stats_bump_version(stats);
}

void stats_modify_cash (thing_statsp stats, int32_t val)
{
    stats->cash += val;
    stats_bump_version(stats);
}

void stats_modify_client_version (thing_statsp stats, int32_t val)
{
    stats->client_version += val;
}

void stats_modify_hp (thing_statsp stats, int32_t val)
{
    stats->hp += val;

    if (stats->hp <= THING_MIN_HEALTH) {
        stats->hp = THING_MIN_HEALTH;
    }
    stats_bump_version(stats);
}

void stats_modify_action_bar_index (thing_statsp stats, int32_t val)
{
    stats->action_bar_index += val;
    stats_bump_version(stats);
}

void stats_modify_max_hp (thing_statsp stats, int32_t val)
{
    stats->max_hp += val;
    stats_bump_version(stats);
}

void stats_modify_magic (thing_statsp stats, int32_t val)
{
    stats->magic += val;
    stats_bump_version(stats);
}

void stats_modify_max_magic (thing_statsp stats, int32_t val)
{
    stats->max_magic += val;
    stats_bump_version(stats);
}

void stats_modify_xp (thing_statsp stats, int32_t val)
{
    stats->xp += val;
    stats_bump_version(stats);
}

void stats_modify_attack_melee (thing_statsp stats, int32_t val)
{
    stats->attack_melee += val;
    stats_bump_version(stats);
}

void stats_modify_attack_ranged (thing_statsp stats, int32_t val)
{
    stats->attack_ranged += val;
    stats_bump_version(stats);
}

void stats_modify_attack_magical (thing_statsp stats, int32_t val)
{
    stats->attack_magical += val;
    stats_bump_version(stats);
}

void stats_modify_speed (thing_statsp stats, int32_t val)
{
    stats->speed += val;
    stats_bump_version(stats);
}

void stats_modify_spending_points (thing_statsp stats, int32_t val)
{
    stats->spending_points += val;
    stats_bump_version(stats);
}

void stats_modify_vision (thing_statsp stats, int32_t val)
{
    stats->vision += val;
    stats_bump_version(stats);
}

void stats_modify_healing (thing_statsp stats, int32_t val)
{
    stats->healing += val;
    stats_bump_version(stats);
}

void stats_modify_defense (thing_statsp stats, int32_t val)
{
    stats->defense += val;
    stats_bump_version(stats);
}
