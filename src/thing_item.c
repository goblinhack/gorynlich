/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include "main.h"
#include "thing.h"
#include "player_stats.h"

static void thing_collect (thingp t, 
                           thingp it, 
                           thing_templatep tp,
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
        player_stats_item_polymorph(&t->stats,
                                    THING_WATER,
                                    THING_WATER_POISON);
    }

    /*
     * Keys are carried as individual keys.
     */
    const char *carried_as = tp_carried_as(tp);
    if (carried_as) {
        thing_templatep what = tp_find(carried_as);
        if (!what) {
            DIE("could now find %s to carry item as for %s",
                tp_name(what), thing_logname(t));
        }

        id = tp_to_id(what);
    }

    /*
     * Can it fit in the backpack?
     */
    if (thing_is_player(t) && it) {
        /*
         * If treasure, just add it to the score. Don't carry it.
         */
        if (tp_is_treasure(tp)) {
            /*
            * Bonus for collecting?
            */
            thing_set_score(t, thing_score(t) +
                            tp_get_bonus_score_on_collect(tp) *
                            it->item.quantity);
            return;
        }

        if (!player_stats_item_add(t, &t->stats, it->item)) {
            THING_SHOUT_AT(t, INFO, "You could not collect %s",
                        tp_short_name(tp));
            return;
        }

        /*
         * Bonus for collecting?
         */
        thing_set_score(t, thing_score(t) +
                        tp_get_bonus_score_on_collect(tp) *
                        it->item.quantity);
    }

    if (!auto_collect) {
        if (thing_is_player(t)) {
            THING_SHOUT_AT(t, INFO, "%s added", tp_short_name(tp));
        }
    }

    /*
     * Auto use a weapon if carrying none.
     */
    if (tp_is_weapon(tp)) {
        if (!t->weapon) {
            THING_LOG(t, "auto wield %s", tp_short_name(tp));

            thing_wield(t, tp);
        }
    }

    if (it) {
        it->is_collected = true;
    }
}

void thing_item_collect (thingp t, thingp it, thing_templatep tp)
{
    thing_collect(t, it, tp, false);
}

void thing_auto_collect (thingp t, thingp it, thing_templatep tp)
{
    thing_collect(t, it, tp, true);
}

void thing_used (thingp t, thing_templatep tp, itemp item)
{
    uint32_t id;

    if (!item) {
        id = tp_to_id(tp);
        item = player_stats_has_item(&t->stats, id, 0);
        if (!item) {
            ERR("Tried to use item which is %s not carried", 
                tp_short_name(tp));
            return;
        }
    } else {
        id = item->id;
        if (!id) {
            DIE("no item used");
        }
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
        THING_SHOUT_AT(t, WARNING, "Cursed %s zaps %d health",
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
    int bonus_id = tp_get_bonus_id_on_use(tp);
    if (item->cursed) {
        THING_SHOUT_AT(t, WARNING, "Cursed %s zaps %d of ID",
                       tp_short_name(tp), bonus_hp);

        bonus_id = -bonus_id;
    }
    t->stats.id += bonus_id;

    /*
     * Need to allow magic items to override this.
     */
    if (thing_get_stats_id(t) > thing_get_stats_max_id(t)) {
        if (tp_is_magical(tp)) {
            /*
             * Allow temorary over max.
             */
        } else {
            t->stats.id = thing_get_stats_max_id(t);
        }
    }

    THING_LOG(t, "used %s", tp_short_name(tp));

    /*
     * Remove from the inventory and other places.
     */
    player_stats_item_remove(t, &t->stats, tp);

    /*
     * Check HP did not go too low.
     */
    if (thing_get_stats_hp(t) < 0) {
        const char *name = tp_short_name(tp);

        thing_dead(t, 0, "%s", name);
        return;
    }
}

void thing_degrade (thingp t, thing_templatep tp, itemp item)
{
    uint32_t id;

    if (!item) {
        id = tp_to_id(tp);
        item = player_stats_has_item(&t->stats, id, 0);
        if (!item) {
            ERR("Tried to use item which is %s not carried", 
                tp_short_name(tp));
            return;
        }
    } else {
        id = item->id;
        if (!id) {
            DIE("no item used");
        }
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
    player_stats_item_remove(t, &t->stats, tp);

    if (tp_is_weapon(tp)) {
        thing_unwield(t);

        THING_SHOUT_AT(t, WARNING, "Your weapon crumbles to dust");

        thing_wield_next_weapon(t);
    }
}

void thing_drop (thingp t, thing_templatep tp, itemp item)
{
    uint8_t auto_wield = false;
    uint32_t id;

    if (!item) {
        id = tp_to_id(tp);
        item = player_stats_has_item(&t->stats, id, 0);
        if (!item) {
            ERR("Tried to use item which is %s not carried", 
                tp_short_name(tp));
            return;
        }
    } else {
        id = item->id;
        if (!id) {
            DIE("no item used");
        }
    }

    /*
     * Dropping the current weapon.
     */
    if (tp == t->weapon) {
        thing_unwield(t);
        auto_wield = true;
    }

    THING_LOG(t, "drop %s", tp_short_name(tp));

    /*
     * Remove from the inventory and other places.
     */
    player_stats_item_remove(t, &t->stats, tp);

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

    item = player_stats_has_item(&t->stats, id, 0);
    if (!item) {
        return (0);
    }

    return (item);
}

thing_templatep thing_is_carrying_thing (thingp t, tp_is fn)
{
    uint32_t i;

    for (i = 0; i < THING_MAX; i++) {
        thing_templatep tp = id_to_tp(i);

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
        thing_templatep tp = id_to_tp(i);

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
