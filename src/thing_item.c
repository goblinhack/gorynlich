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
                           thing_templatep tmp,
                           uint8_t auto_collect)
{
    uint32_t item;
    uint32_t quantity;

    if (thing_is_player(t)) {
        t->needs_tx_player_update = true;
    }

    item = thing_template_to_id(tmp);
    quantity = 1;

    if (thing_template_get_quantity(tmp)) {
        quantity += thing_template_get_quantity(tmp) - 1;
    }

    /*
     * Bit of a hack to make the collected item appear as an another.
     */
    if ((item == THING_KEYS2) || (item == THING_KEYS3)) {
        item = THING_KEY;
    }

    if (item == THING_TORCHES) {
        item = THING_TORCH;
    }

    if (item == THING_WATER_POISON) {
        item = THING_WATER;
        /*
         * Convert any existing items.
         */
        player_stats_item_polymorph(&t->stats,
                                    THING_WATER,
                                    THING_WATER_POISON);
    }

    /*
     * If treasure, just add it to the score. Don't carry it.
     */
    if (thing_template_is_treasure(tmp)) {
        /*
         * Bonus for collecting?
         */
        thing_set_score(t, thing_score(t) +
                        thing_template_get_bonus_score_on_collect(tmp) *
                        quantity);
        return;
    }

    /*
     * Can it fit in the backpack?
     */
    item_t i = {0};

    i.quality = it->quality ? it->quality : THING_ITEM_QUALITY_MAX;
    i.quantity = quantity;
    i.cursed = thing_template_is_cursed(tmp);

    if (!player_stats_item_add(t, &t->stats, tmp, i)) {
        THING_SHOUT_AT(t, INFO, "You could not collect %s",
                       thing_template_short_name(tmp));
        return;
    }

    /*
     * Bonus for collecting?
     */
    thing_set_score(t, thing_score(t) +
                    thing_template_get_bonus_score_on_collect(tmp) *
                    quantity);

    if (!auto_collect) {
        if (thing_is_player(t)) {
            THING_SHOUT_AT(t, INFO, "%s added", 
                           thing_template_short_name(tmp));
        }
    }

    /*
     * Auto use a weapon if carrying none.
     */
    if (thing_template_is_weapon(tmp)) {
        if (!t->weapon) {
            THING_LOG(t, "auto wield %s", thing_template_short_name(tmp));

            thing_wield(t, tmp);
        }
    }

    if (it) {
        it->is_collected = true;
    }
}

void thing_item_collect (thingp t, thingp it, thing_templatep tmp)
{
    thing_collect(t, it, tmp, false);
}

void thing_auto_collect (thingp t, thingp it, thing_templatep tmp)
{
    thing_collect(t, it, tmp, true);
}

void thing_used (thingp t, thing_templatep tmp)
{
    uint32_t id;
    item_t *item;

    id = thing_template_to_id(tmp);
    item = player_stats_has_item(&t->stats, id, 0);
    if (!item) {
        ERR("Tried to use item which is %s not carried", 
            thing_template_short_name(tmp));
        return;
    }

    if (thing_is_player(t)) {
        t->needs_tx_player_update = true;
    }

    /*
     * Switch of weapons.
     */
    if (thing_template_is_weapon(tmp)) {
        thing_wield(t, tmp);
        return;
    }

    /*
     * HP modifications.
     */
    int bonus_hp = thing_template_get_bonus_hp_on_use(tmp);
    if (item->cursed) {
        THING_SHOUT_AT(t, WARNING, "Cursed %s zaps %d health",
                       thing_template_short_name(tmp), bonus_hp);
        bonus_hp = -bonus_hp;
    }
    t->stats.hp += bonus_hp;

    /*
     * Need to allow magic items to override this.
     */
    if (thing_get_stats_hp(t) > thing_get_stats_max_hp(t)) {
        if (thing_template_is_magical(tmp)) {
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
    int bonus_id = thing_template_get_bonus_id_on_use(tmp);
    if (item->cursed) {
        THING_SHOUT_AT(t, WARNING, "Cursed %s zaps %d of ID",
                       thing_template_short_name(tmp), bonus_hp);

        bonus_id = -bonus_id;
    }
    t->stats.id += bonus_id;

    /*
     * Need to allow magic items to override this.
     */
    if (thing_get_stats_id(t) > thing_get_stats_max_id(t)) {
        if (thing_template_is_magical(tmp)) {
            /*
             * Allow temorary over max.
             */
        } else {
            t->stats.id = thing_get_stats_max_id(t);
        }
    }

    THING_LOG(t, "used %s", thing_template_short_name(tmp));

    /*
     * Remove from the inventory and other places.
     */
    player_stats_item_remove(t, &t->stats, tmp);

    /*
     * Check HP did not go too low.
     */
    if (thing_get_stats_hp(t) < 0) {
        const char *name = thing_template_short_name(tmp);

        thing_dead(t, 0, "%s", name);
        return;
    }
}

void thing_degrade (thingp t, thing_templatep tmp)
{
    uint32_t id;
    item_t *item;

    id = thing_template_to_id(tmp);
    item = player_stats_has_item(&t->stats, id, 0);
    if (!item) {
        ERR("Tried to degrade item which is %s not carried", 
            thing_template_short_name(tmp));
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
    player_stats_item_remove(t, &t->stats, tmp);

    if (thing_template_is_weapon(tmp)) {
        thing_unwield(t);

        THING_SHOUT_AT(t, WARNING, "Your weapon crumbles to dust");

        thing_wield_next_weapon(t);
    }
}

void thing_drop (thingp t, thing_templatep tmp)
{
    uint8_t auto_wield = false;
    uint32_t id;
    item_t *item;

    id = thing_template_to_id(tmp);
    item = player_stats_has_item(&t->stats, id, 0);
    if (!item) {
        ERR("tried to drop %s not carried", thing_template_short_name(tmp));
        return;
    }

    /*
     * Dropping the current weapon.
     */
    if (tmp == t->weapon) {
        thing_unwield(t);
        auto_wield = true;
    }

    THING_LOG(t, "drop %s", thing_template_short_name(tmp));

    /*
     * Remove from the inventory and other places.
     */
    player_stats_item_remove(t, &t->stats, tmp);

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

item_t *thing_is_carrying_specific_item (thingp t, uint32_t id)
{
    item_t *item;

    item = player_stats_has_item(&t->stats, id, 0);
    if (!item) {
        return (0);
    }

    return (item);
}

thing_templatep thing_is_carrying_thing (thingp t, thing_template_is fn)
{
    uint32_t i;

    for (i = 0; i < THING_MAX; i++) {
        thing_templatep tp = id_to_thing_template(i);

        if (!(*fn)(tp)) {
            continue;
        }

        item_t *item = thing_is_carrying_specific_item(t, i);
        if (item) {
            return (tp);
        }
    }

    return (0);
}

uint32_t thing_is_carrying_thing_count (thingp t, thing_template_is fn)
{
    uint32_t count = 0;
    uint32_t i;

    for (i = 0; i < THING_MAX; i++) {
        thing_templatep tp = id_to_thing_template(i);

        if (!(*fn)(tp)) {
            continue;
        }

        item_t *item = thing_is_carrying_specific_item(t, i);
        if (item) {
            count += item->quantity;
        }
    }

    return (count);
}
