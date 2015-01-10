/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include "main.h"
#include "thing.h"
#include "thing_stats.h"

static void thing_collect (thingp t, 
                           thingp it,
                           item_t i,
                           tpp tp,
                           uint8_t auto_collect)
{
    uint32_t id;

    if (thing_is_player(t)) {
        t->needs_tx_player_update = true;
    }

    id = tp_to_id(tp);

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
            DIE("could now find %s to carry item as for %s",
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

        for (count = 0; count < quantity; count++) {
            thing_collect(t, 
                          0,
                          i, 
                          what,
                          true /* auto_collect */);
        }

        if (thing_is_player(t)) {
            MSG_SERVER_SHOUT_AT_PLAYER(INFO, t, "%s added", tp_short_name(tp));
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
        if (tp_is_treasure(tp)) {
            /*
             * Bonus for collecting?
             */
            t->stats.cash += tp_get_bonus_cash_on_collect(tp) *
                            i.quantity;
            return;
        }

        if (!thing_stats_item_add(t, &t->stats, i)) {
            MSG_SERVER_SHOUT_AT_PLAYER(INFO, t,
                                       "You could not collect %s",
                                       tp_short_name(tp));
            return;
        }

        /*
         * Bonus for collecting?
         */
        t->stats.cash += tp_get_bonus_cash_on_collect(tp) *
                        i.quantity;
    }

    if (!auto_collect) {
        if (thing_is_player(t)) {
            MSG_SERVER_SHOUT_AT_PLAYER(INFO, t,
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
        ERR("Tried to use item which is %s not carried", 
            tp_short_name(tp));
        return;
    }

    if (thing_is_player(t)) {
        t->needs_tx_player_update = true;
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
        MSG_SERVER_SHOUT_AT_PLAYER(WARNING, t,
                                   "Cursed %s zaps %d health",
                                   tp_short_name(tp), bonus_hp);
        bonus_hp = -bonus_hp;
    }
    t->stats.hp += bonus_hp;

    /*
     * Need to allow magic items to override this.
     */
    if (thing_get_stats_hp(t) > thing_get_stats_max_hp(t)) {
        if (tp_is_magical(tp)) {
            /*
             * Allow temorary over max.
             */
        } else {
            t->stats.hp = thing_get_stats_max_hp(t);
        }
    }

    /*
     * id modifications.
     */
    int bonus_magic = tp_get_bonus_magic_on_use(tp);
    if (item->cursed) {
        MSG_SERVER_SHOUT_AT_PLAYER(WARNING, t,
                                   "Cursed %s zaps %d of ID",
                                   tp_short_name(tp), bonus_hp);

        bonus_magic = -bonus_magic;
    }
    t->stats.magic += bonus_magic;

    /*
     * Need to allow magic items to override this.
     */
    if (thing_get_stats_magic(t) > thing_get_stats_max_magic(t)) {
        if (tp_is_magical(tp)) {
            /*
             * Allow temorary over max.
             */
        } else {
            t->stats.magic = thing_get_stats_max_magic(t);
        }
    }

    THING_LOG(t, "used %s", tp_short_name(tp));

    /*
     * Remove from the inventory and other places.
     */
    thing_stats_item_remove(t, &t->stats, tp);

    /*
     * Check HP did not go too low.
     */
    if (thing_get_stats_hp(t) < 0) {
        t->stats.hp = 0;

        const char *name = tp_short_name(tp);

        thing_dead(t, 0, "%s", name);
        return;
    }
}

void thing_wear_out (thingp t, tpp tp)
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
        ERR("Tried to use item which is %s not carried", 
            tp_short_name(tp));
        return;
    }

    if (thing_is_player(t)) {
        t->needs_tx_player_update = true;
    }

    if (item->quality) {
        item->quality--;
        return;
    }

    /*
     * Remove from the inventory and other places.
     */
    thing_stats_item_remove(t, &t->stats, tp);

    if (tp_is_weapon(tp)) {
        thing_unwield(t);

        MSG_SERVER_SHOUT_AT_PLAYER(WARNING, t,
                                   "Your weapon crumbles to dust");

        thing_wield_next_weapon(t);
    }
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

    /*
     * Wield the next weapon we have.
     */
    if (auto_wield) {
        thing_wield_next_weapon(t);
    }

    if (thing_is_player(t)) {
        t->needs_tx_player_update = true;
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

    for (i = 0; i < THING_MAX; i++) {
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

    for (i = 0; i < THING_MAX; i++) {
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
