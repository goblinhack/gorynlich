/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>
#include <math.h>

#include "main.h"
#include "thing.h"
#include "thing_timer.h"
#include "wid.h"
#include "wid_game_map_client.h"
#include "wid_game_map_server.h"
#include "map.h"
#include "level.h"
#include "time.h"
#include "string.h"
#include "wid_textbox.h"
#include "color.h"
#include "sound.h"
#include "socket.h"
#include "client.h"
#include "timer.h"
#include "math.h"

void thing_collect (thingp t, thingp it, thing_templatep tmp)
{
    uint32_t item;
    uint32_t quantity;

    if (thing_is_player(t)) {
        t->needs_tx_player_update = true;
    }

    item = thing_template_to_id(tmp);
    quantity = 1;

    /*
     * Collect bundles of keys as the key item.
     */
    if (thing_template_is_key2(tmp)) {
        item = THING_KEY;
        quantity = 2;
    }

    if (thing_template_is_key3(tmp)) {
        item = THING_KEY;
        quantity = 3;
    }

    if (quantity > 1) {
        THING_LOG(t, "collects %u %s", quantity,
                  thing_template_short_name(tmp));
    } else {
        THING_LOG(t, "collects %s", thing_template_short_name(tmp));
    }

    /*
     * Bonus for collecting?
     */
    thing_set_score(t, thing_score(t) +
                thing_template_get_bonus_score_on_collect(tmp) * quantity);

    /*
     * If treasure, just add it to the score. Don't carry it.
     */
    if (thing_template_is_treasure(tmp)) {
        return;
    }

    /*
     * Collecting poisoned water poisons other water being carried.
     */
    if (item == THING_WATER_POISON) {
        t->carrying[item] += t->carrying[THING_WATER];
        t->carrying[THING_WATER] = 0;
    }

    if (thing_is_player(t)) {
        THING_SHOUT_AT(t, INFO, "%s added", thing_template_short_name(tmp));
    }

    t->carrying[item] += quantity;

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

void thing_used (thingp t, thing_templatep tmp)
{
    uint32_t item;

    item = thing_template_to_id(tmp);
    if (!t->carrying[item]) {
        ERR("tried to use %s not carried", thing_template_short_name(tmp));
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

    t->health += thing_template_get_bonus_health_on_use(tmp);

    if (t->health < 0) {
        const char *name = thing_template_short_name(tmp);

        thing_dead(t, 0, "%s", name);
    }

    THING_LOG(t, "used %s", thing_template_short_name(tmp));

    t->carrying[item]--;
}

void thing_item_destroyed (thingp t, thing_templatep tmp)
{
    uint32_t item;

    item = thing_template_to_id(tmp);
    if (!t->carrying[item]) {
        ERR("tried to item destroy %s not carried", 
            thing_template_short_name(tmp));
        return;
    }

    t->carrying[item]--;

    THING_LOG(t, "item destroyed %s", thing_template_short_name(tmp));

    if (thing_template_is_weapon(tmp)) {
        thing_unwield(t);

        THING_SHOUT_AT(t, WARNING, "Your weapon crumbles to dust");

        thing_wield_next_weapon(t);
    }

    if (thing_is_player(t)) {
        t->needs_tx_player_update = true;
    }
}

void thing_drop (thingp t, thing_templatep tmp)
{
    uint32_t item;
    uint8_t auto_wield = false;

    item = thing_template_to_id(tmp);
    if (!t->carrying[item]) {
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

    t->carrying[item]--;

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

uint8_t thing_is_carrying (thingp t, uint32_t item)
{
    if (!t->carrying[item]) {
        return (false);
    }

    return (true);
}
