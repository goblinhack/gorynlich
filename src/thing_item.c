/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */

#include "main.h"
#include "thing.h"
#include "string_ext.h"

char *item2str (const item_t i)
{
    char *tmp;
    int32_t size;
    int32_t used;

    tmp = 0;
    size = 0;
    used = 0;

    if (!i.id) {
        return (0);
    }

    snprintf_realloc(&tmp, &size, &used, "%s",
                     tp_short_name(id_to_tp(i.id)));

    if (i.quantity > 1) {
        snprintf_realloc(&tmp, &size, &used, " x %u",
                        i.quantity);
    }

    if (i.quality < THING_ITEM_QUALITY_MAX) {
        snprintf_realloc(&tmp, &size, &used, " %2.0fpct",
                         (100.0 / (float)THING_ITEM_QUALITY_MAX) *
                         (float) i.quality);
    }

    if (i.cursed) {
        snprintf_realloc(&tmp, &size, &used, " (cursed)");
    }

    if (i.cursed) {
        snprintf_realloc(&tmp, &size, &used, " (+%d)",
                         i.enchanted);
    }

    return (tmp);
}

static void thing_collect (thingp t, 
                           thingp it,
                           item_t i,
                           tpp tp,
                           uint8_t auto_collect)
{
    uint32_t id;

    id = tp_to_id(tp);
    if (!i.id) {
        ERR("trying to collect item %s with no id", tp_name(tp));
    }

    if (id == THING_WATER_POISON) {
        /*
         * Convert any existing items.
         */
        thing_stats_item_polymorph(&t->stats,
                                   THING_WATER,
                                   THING_WATER_POISON);
    }

    /*
     * Keys are carried as individual keys.
     */
    const char *carried_as = tp_carried_as(tp);
    if (carried_as) {
        tpp what = tp_find(carried_as);

        if (!what) {
            ERR("could now find %s to carry item as for %s",
                tp_name(what), thing_logname(t));
        }

        int count;
        int quantity = tp_get_quantity(tp);

        if (quantity > 1) {
            THING_LOG(t, "collect %s as %u %ss", 
                      tp_short_name(tp), quantity, tp_short_name(what));
        } else {
            THING_LOG(t, "collect %s as %s", 
                      tp_short_name(tp), tp_short_name(what));
        }

        i.quantity = 1;
        i.id = tp_to_id(what);

        /*
         * Take this line out and shopkeepers will kill on key collection 8)
         */
        it->is_collected = true;

        for (count = 0; count < quantity; count++) {
            thing_collect(t, 
                          0,
                          i, 
                          what,
                          true /* auto_collect */);
        }

        if (thing_is_player(t)) {
            const char *sound = tp_sound_on_collect(tp);
            if (sound) {
                MSG_SERVER_SHOUT_AT(SOUND, t, t->x, t->y, "%s", sound);
            }

            MSG_SERVER_SHOUT_AT(INFO, t, 0, 0, "%s added", tp_short_name(tp));
        }
        return;
    }

    THING_LOG(t, "collect %s", tp_short_name(tp));

    /*
     * Can it fit in the backpack?
     */
    if (thing_is_player(t)) {
        /*
         * If treasure, just add it to the score. Don't carry it.
         */
        int32_t val = tp_get_bonus_cash_on_collect(tp) * i.quantity;

        if (tp_is_treasure(tp)) {
            /*
             * Bonus for collecting?
             */
            thing_stats_modify_cash(t, val);

            if (val) {
                MSG_SERVER_SHOUT_AT(OVER_THING, t, 0, 0,
                                    "%%%%font=%s$%%%%fg=%s$%d$", 
                                    "large", "gold", val);

                return;
            }
        }

        if (!thing_stats_item_add(t, &t->stats, i)) {
            MSG_SERVER_SHOUT_AT(INFO, t, 0, 0,
                                "You could not collect %s",
                                tp_short_name(tp));
            return;
        }

        player_inventory_sort(&t->stats);

        /*
         * Bonus for collecting?
         */
        thing_stats_modify_cash(t, val);

        if (val) {
            MSG_SERVER_SHOUT_AT(OVER_THING, t, 0, 0,
                                "%%%%font=%s$%%%%fg=%s$%d$", 
                                "large", "gold", val);
        }
    }

    if (!auto_collect) {
        if (thing_is_player(t)) {
            MSG_SERVER_SHOUT_AT(INFO, t, 0, 0,
                                "%s added", tp_short_name(tp));
        }
    }

    /*
     * Auto use a weapon if carrying none.
     */
    if (tp_is_weapon(tp)) {
        THING_LOG(t, "auto wield %s", tp_short_name(tp));

        thing_wield(t, tp);
    }

    if (thing_is_player(t)) {
        const char *sound = tp_sound_on_collect(tp);
        if (sound) {
            MSG_SERVER_SHOUT_AT(SOUND_GLOBAL, t, t->x, t->y, "%s", sound);
        }
    }

    if (it) {
        it->is_collected = true;
    }
}

static item_t thing_tp_to_item (tpp tp)
{
    item_t item = {0};

    item.id = tp_to_id(tp);
    item.quantity = 1;
    item.quality = THING_ITEM_QUALITY_MAX;

    return (item);
}

void thing_item_collect (thingp t, thingp it, tpp tp)
{
    if (!it) {
        thing_collect(t, it, thing_tp_to_item(tp), tp, false);
    } else {
        thing_collect(t, it, it->item, tp, false);
    }
}

void thing_auto_collect (thingp t, thingp it, tpp tp)
{
    if (!it) {
        thing_collect(t, it, thing_tp_to_item(tp), tp, true);
    } else {
        thing_collect(t, it, it->item, tp, true);
    }
}

void thing_used (thingp t, tpp tp)
{
    uint32_t id;
    itemp item;

    id = tp_to_id(tp);
    if (!id) {
        ERR("No such item %s", tp_short_name(tp));
        return;
    }

    item = thing_has_item(t, id);
    if (!item) {
        ERR("Tried to use an item (%s) which is not carried", 
            tp_short_name(tp));
        return;
    }

    /*
     * Switch of weapons.
     */
    if (tp_is_weapon(tp)) {
        thing_wield(t, tp);
        return;
    }

    /*
     * HP modifications.
     */
    int bonus_hp = tp_get_bonus_hp_on_use(tp);
    if (item->cursed) {
        MSG_SERVER_SHOUT_AT(WARNING, t, 0, 0,
                            "Cursed %s zaps %d health",
                            tp_short_name(tp), bonus_hp);
        bonus_hp = -bonus_hp;
    }

    if (tp_is_food(tp)) {
        MSG_SERVER_SHOUT_AT(SOUND, t, t->x, t->y, "bite");
    }

    if (id == THING_SPAM) {
        thing_stats_modify_speed(t, -1);
    }

    thing_stats_modify_hp(t, bonus_hp);

    /*
     * Need to allow magic items to override this.
     */
#if 0
    /*
     * I think it's nicer to go over max hp and then fall back to
     * the maximum slowly
     */
    if (thing_stats_get_hp(t) > thing_stats_get_max_hp(t)) {
        if (tp_is_magical(tp)) {
            /*
             * Allow temporary over max.
             */
        } else {
            thing_stats_set_hp(t, thing_stats_get_max_hp(t));
        }
    }
#endif

    if (thing_stats_get_hp(t) <= 0) {
        if (tp_is_life_saving(tp)) {
            thing_stats_set_hp(t, thing_stats_get_max_hp(t) / 2);

            t->is_dying = false;

            MSG_SERVER_SHOUT_AT(WARNING, t, 0, 0,
                                "%s crumbles to dust, yet you live",
                                tp_short_name(tp));
        }
    } else {
        /*
         * Can this happen?
         */
        if (tp_is_life_saving(tp)) {
            MSG_SERVER_SHOUT_AT(WARNING, t, 0, 0,
                                "%s crumbles to dust, sadly with no effect",
                                tp_short_name(tp));
        }
    }

    /*
     * id modifications.
     */
    int bonus_magic = tp_get_bonus_magic_on_use(tp);
    if (item->cursed) {
        MSG_SERVER_SHOUT_AT(WARNING, t, 0, 0,
                            "Cursed %s zaps %d of ID",
                            tp_short_name(tp), bonus_hp);

        bonus_magic = -bonus_magic;
    }

    thing_stats_modify_magic(t, bonus_magic);

    /*
     * Need to allow magic items to override this.
     */
    if (thing_stats_get_magic(t) > thing_stats_get_max_magic(t)) {
        if (tp_is_magical(tp)) {
            /*
             * Allow temporary over max.
             */
        } else {
            thing_stats_set_magic(t, thing_stats_get_max_magic(t));
        }
    }

    THING_LOG(t, "used %s", tp_short_name(tp));

    /*
     * Remove from the inventory and other places.
     */
    thing_stats_item_remove(t, &t->stats, tp);
    player_inventory_sort(&t->stats);

    /*
     * Did the item push us into death's icy grip?
     */
    if (thing_stats_get_hp(t) < 0) {
        thing_stats_set_hp(t, 0);

        const char *name = tp_short_name(tp);

        thing_dying(t, 0, "%s", name);
        return;
    }
}

int thing_wear_out (thingp t, tpp tp)
{
    uint32_t id;
    itemp item;

    id = tp_to_id(tp);
    if (!id) {
        ERR("No such item %s", tp_short_name(tp));
        return (false);
    }

    item = thing_has_item(t, id);
    if (!item) {
        ERR("Tried to wear out an item (%s) which is not carried", 
            tp_short_name(tp));
        return (false);
    }

    if (item->quality) {
        item->quality--;
        return (false);
    }

    /*
     * Remove from the inventory and other places.
     */
    thing_stats_item_remove(t, &t->stats, tp);
    player_inventory_sort(&t->stats);

    if (tp_is_weapon(tp)) {
        thing_weapon_worn_out(t, tp);
    }

    return (true);
}

void thing_drop (thingp t, tpp tp)
{
    uint8_t auto_wield = false;
    uint32_t id;
    itemp item;

    id = tp_to_id(tp);
    if (!id) {
        ERR("No such item %s", tp_short_name(tp));
        return;
    }

    item = thing_has_item(t, id);
    if (!item) {
        ERR("Tried to drop item which is %s not carried", 
            tp_short_name(tp));
        return;
    }

    /*
     * Dropping the current weapon.
     */
    if (tp == thing_weapon(t)) {
        thing_unwield(t);
        auto_wield = true;
    }

    THING_LOG(t, "drop %s", tp_short_name(tp));

    /*
     * Remove from the inventory and other places.
     */
    thing_stats_item_remove(t, &t->stats, tp);
    player_inventory_sort(&t->stats);

    /*
     * Wield the next weapon we have.
     */
    if (auto_wield) {
        thing_wield_next_weapon(t);
    }
}

itemp thing_is_carrying_specific_item (thingp t, uint32_t id)
{
    itemp item;

    item = thing_stats_has_item(&t->stats, id, 0);
    if (!item) {
        return (0);
    }

    return (item);
}

tpp thing_is_carrying_thing (thingp t, tp_is fn)
{
    uint32_t i;

    for (i = 0; i < THING_MAX_ID; i++) {
        tpp tp = id_to_tp(i);

        if (!(*fn)(tp)) {
            continue;
        }

        itemp item = thing_is_carrying_specific_item(t, i);
        if (item) {
            return (tp);
        }
    }

    return (0);
}

uint32_t thing_is_carrying_thing_count (thingp t, tp_is fn)
{
    uint32_t count = 0;
    uint32_t i;

    for (i = 0; i < THING_MAX_ID; i++) {
        tpp tp = id_to_tp(i);

        if (!(*fn)(tp)) {
            continue;
        }

        itemp item = thing_is_carrying_specific_item(t, i);
        if (item) {
            count += item->quantity;
        }
    }

    return (count);
}
