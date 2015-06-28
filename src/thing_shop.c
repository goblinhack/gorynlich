/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>
#include <math.h>

#include "main.h"
#include "thing.h"
#include "wid_game_map_server.h"
#include "map.h"
#include "level.h"
#include "thing_shop.h"
#include "time_util.h"
#include "socket_util.h"

void shop_enter (thingp t, thingp floor)
{
    if (!thing_is_player(t)) {
        return;
    }

    if (t->timestamp_last_shop_enter &&
        !time_have_x_secs_passed_since(60, t->timestamp_last_shop_enter)) {
        return;
    }

    t->timestamp_last_shop_enter = time_get_time_ms();

    thingp shopkeeper = thing_owner(floor);
    if (!shopkeeper) {
        MSG_SERVER_SHOUT_AT_PLAYER(POPUP, t, "No shopkeeper to be seen");
        return;
    }

    if (thing_is_dead(shopkeeper)) {
        /*
         * You killed the keeper earlier?
         */
        MSG_SERVER_SHOUT_AT_PLAYER(POPUP, t, "A bad thing happened here...");
        return;
        
    } else if (thing_is_angry(shopkeeper)) {
        /*
         * Reentering the crime scene?
         */
        MSG_SERVER_SHOUT_AT_PLAYER(POPUP, t, "%%%%fg=red$%s", "Get out of my shop!");

    } else {
        /*
         * Welcome message.
         */
        static const char *messages[] = {
            "Welcome noble aventure person to my shop!",
            "Welcome brave blood covered person!",
            "Welcome to my most humble shop!",
            "Welcome to my most humble bazaar!",
            "Welcome noble one to my shop",
            "Welcome brave and fearless one to my shop",
            "Please look around and remember to pay!",
            "Sale on today. All items must be sold!",
        };

        MSG_SERVER_SHOUT_AT_PLAYER(SOUND, t, "shotgun_reload");
        MSG_SERVER_SHOUT_AT_PLAYER(POPUP, t, "%%%%fg=green$%s", messages[myrand() % ARRAY_SIZE(messages)]);
    }

    THING_LOG(t, "entered shop");

    t->in_shop_owned_by_thing_id = shopkeeper->thing_id;
}

void shop_leave (thingp t, thingp floor)
{
    THING_LOG(t, "leave shop");

    if (!thing_is_player(t)) {
        return;
    }

    /*
     * Nothing bought?
     */
    if (!t->in_shop_owned_by_thing_id) {
        return;
    }

    if (t->money_owed == 0) {
        THING_LOG(t, "nothing owed to shopkeeper");
        return;
    }

    /*
     * Dead keeper?
     */
    thingp shopkeeper = thing_server_find(t->in_shop_owned_by_thing_id);
    if (!shopkeeper) {
        t->money_owed = 0;
        t->in_shop_owned_by_thing_id = 0;
        MSG_SERVER_SHOUT_AT_PLAYER(POPUP, t, "Odd... No one around to pay");
        return;
    }

    if (t->money_owed < 0) {
        /*
         * Shopkeeper owes us.
         */
        if (thing_is_dead(shopkeeper)) {
            MSG_SERVER_SHOUT_AT_PLAYER(POPUP, t, "No one lives to pay you");
        } else if (thing_is_angry(shopkeeper)) {
            MSG_SERVER_SHOUT_AT_PLAYER(POPUP, t, "%%%%fg=red$%s", "I refuse to pay a thief!");
        } else {
            MSG_SERVER_SHOUT_AT_PLAYER(POPUP, t, "%%%%fg=green$%s", "Good doing business with you");

            thing_stats_modify_cash(t, -t->money_owed);
        }

        t->money_owed = 0;
        return;
    }

    /*
     * Stealing?
     */
    thing_set_is_angry(shopkeeper, true);
    shop_steal_message(t);

    t->money_owed = 0;
    t->in_shop_owned_by_thing_id = 0;
}

void shop_collect_message (thingp t, thingp item)
{
    if (!thing_is_player(t)) {
        return;
    }

    static const char *messages[] = {
        "A most wise and noble purchase.",
        "Only used by one careful old wizard.",
        "Even my mother would use that one!",
        "Even my pet rabbit would use that one!",
        "Even my pet mososaur would use that one!",
        "Every home needs one of those!",
        "Every dungoneer needs at least one of those!",
        "Every dungoneer needs at least three of those!",
        "A bargain there!",
        "I'm going out of business with these bargains!",
        "I'm malfunctioning with these bargains!",
        "I'm as crazy as a banana with these prices!",
        "I'm as crazy as a plesiosaur with these prices!",
        "Fifty percent off that item!",
        "Must be sold today! Moving to a new dungeon!",
        "A most high quality item!",
        "A most polished piece!",
        "Why that fine piece, a veritale steal!",
        "Guaranteed results!",
        "You'll never need to return that!",
        "Sure to have a satisfying result!",
        "Sure to have an interesting result!",
        "Surely you should consider more?",
        "I'm insane at these prices!",
        "I'm going mad at these prices!",
        "I'm suffering the heat at these prices!",
        "I'm giving that away!",
        "I'm giving this to charity at these prices!",
        "Why am I selling at these prices?",
        "I insult myself at these prices!",
        "I insult my mother at these prices!",
        "I insult all mothers at these prices!",
    };

    thingp shopkeeper = thing_owner(item);
    if (!shopkeeper) {
        MSG_SERVER_SHOUT_AT_PLAYER(POPUP, t, "Odd... no one owns this. Free item...");
        return;
    }

    thing_set_owner(item, t);

    if (t->timestamp_last_shop_enter &&
        time_have_x_secs_passed_since(10, t->timestamp_last_shop_enter)) {

        if (thing_is_dead(shopkeeper)) {
            MSG_SERVER_SHOUT_AT_PLAYER(POPUP, t, "No one to pay");

            return;
        } else if (thing_is_angry(shopkeeper)) {
            /*
             * Already a thief and stealing again?!
             */
            MSG_SERVER_SHOUT_AT_PLAYER(POPUP, t, "%%%%fg=red$More thievery!");
        } else {
            MSG_SERVER_SHOUT_AT_PLAYER(POPUP, t, "%s", messages[myrand() % ARRAY_SIZE(messages)]);

            MSG_SERVER_SHOUT_AT_PLAYER(WARNING, t, "%%%%fg=red$Press P to pay");
        }
    }

    t->money_owed += tp_get_cost(item->tp);
    t->in_shop_owned_by_thing_id = shopkeeper->thing_id;

    THING_LOG(t, "owes %d$ to id %d", t->money_owed, t->in_shop_owned_by_thing_id);
}

void shop_deposit_message (thingp t, thingp item)
{
    if (!thing_is_player(t)) {
        return;
    }

    static const char *messages[] = {
        "Thankyou, an interesting piece",
        "Hmm, a bit damaged, but I'll take it",
        "Thank-you sir",
        "Thank-you ma'am",
        "Where do you find this junk?",
        "I'm insane to buy this from you",
        "Did you find that in the street?",
        "This isn't a charity shop!",
        "I'll take it, but no more please!",
        "What a piece of cheap junk",
    };

    thingp shopkeeper = thing_server_find(t->in_shop_owned_by_thing_id);
    if (!shopkeeper) {
        MSG_SERVER_SHOUT_AT_PLAYER(POPUP, t, "No one seems to be running the shop");
        return;
    }

    if (thing_is_dead(shopkeeper)) {
        MSG_SERVER_SHOUT_AT_PLAYER(POPUP, t, "No one to accept these goods");
        return;
    } else if (thing_is_angry(shopkeeper)) {
        MSG_SERVER_SHOUT_AT_PLAYER(WARNING, t, "%%%%fg=red$I'm not taking your stolen goods!");
        return;
    }

    if (!tp_get_cost(item->tp)) {
        MSG_SERVER_SHOUT_AT_PLAYER(POPUP, t, "I'm not taking that junk");
        return;
    }

    t->money_owed -= tp_get_cost(item->tp);

    thing_set_owner(item, shopkeeper);

    MSG_SERVER_SHOUT_AT_PLAYER(POPUP, t,
                               "%s", messages[myrand() % ARRAY_SIZE(messages)]);
}

void shop_pay_for_items (thingp t)
{
    if (!thing_is_player(t)) {
        return;
    }

    static const char *messages[] = {
        "Blessings upon you! I eat tonight!",
        "Blessings upon you! My children eat tonight!",
        "Blessings upon you! My dog eats tonight!",
        "Blessings upon you! My pet dragon eats tonight!",
        "Blessings upon you! My pet slime mold eats tonight!",
        "Blessings upon you! My pet goblin eats tonight!",
        "A worthy purchase!",
        "May the sun shine on your armour!",
        "A thousand thank-yous!",
        "A thousand salutations!",
        "May your camel always be watered!",
        "May your goblin always be watered!",
        "May your dragon breath always be firey!",
        "May your helmet always be shiny!",
        "May your pet goblin always be ferocious!",
        "May goodly knights accompany you!",
        "May you meet no slime!",
        "Hurry back!",
        "Hurry back! It's lonely here!",
        "Hurry back! It's lonely here! In the dark..",
        "Come again fine customer!",
        "Come again noble sire!",
        "Refer me to your friends!",
    };

    thingp shopkeeper = thing_server_find(t->in_shop_owned_by_thing_id);
    if (!shopkeeper) {
        MSG_SERVER_SHOUT_AT_PLAYER(POPUP, t, "There is no one to pay");
        return;
    }

    if (thing_is_dead(shopkeeper)) {
        MSG_SERVER_SHOUT_AT_PLAYER(POPUP, t, "He cannot take payment now...");
        return;
    }

    if (thing_is_angry(shopkeeper)) {
        MSG_SERVER_SHOUT_AT_PLAYER(POPUP, t,
                                   "%%%%fg=red$I'll never take your money!");
        return;
    }

    if (t->money_owed > thing_stats_get_cash(t)) {
        /*
         * Urk!
         */
        MSG_SERVER_SHOUT_AT_PLAYER(POPUP, t,
                                   "%%%%fg=red$Not enough money. You owe %d$", 
                                   t->money_owed);
        return;
    }

    if (t->timestamp_last_shop_enter &&
        time_have_x_secs_passed_since(10, t->timestamp_last_shop_enter)) {

        MSG_SERVER_SHOUT_AT_PLAYER(POPUP, t,
                                   "%s", messages[myrand() % ARRAY_SIZE(messages)]);
    }

    thing_stats_modify_cash(t, -t->money_owed);

    t->money_owed = 0;

    MSG_SERVER_SHOUT_AT_PLAYER(SOUND, t, "payment");
}

void shop_steal_message (thingp t)
{
    if (!thing_is_player(t)) {
        return;
    }

    static const char *messages[] = {
        "You steal from me, I break your legs!",
        "You steal from me, I break your antennae!",
        "You steal from me, I break your tentacles!",
        "You steal from me, I break your probiscus!",
        "You steal from me, I break your horns!",
        "You steal from me, I break your long nose!",
        "You steal from me, I break your arms!",
        "You steal from me, I break your pinkies!",
        "You steal from me, I break your wand!",
        "You steal from me, I break your sword!",
        "Come back you scoundrel!",
        "Thief! Thief!",
        "Thief! Scoundrel!",
        "Thief! Not again... <sob>!",
        "Thief! Thief! I say a thief!",
        "Thief! I spit on your shoes!",
        "Thief! I spit on your eldeberry bush!",
        "Thief! I spit in your general direction!",
        "Thief! A pox on you!",
        "Thief! May a pigeon eat your shoes!",
        "Thief! May a hippogriff sit on you!",
        "Thief! May a goblin eat your shoes!",
        "Thief! May a troll drop on your head!",
        "Thief! May you find fourteen spiders in your lunch!",
        "Thief! May a stoat crawl up your pants!",
        "Thief! May a hulk find you attractive!",
        "Thief! May you find maggots in your food!",
        "Thief! May you find dragons in your food!",
        "Thief! Come back so I may thank you!",
        "Thief! You are banned! banned I say!",
        "Thief! And you seemed so honest!",
        "Thief! And your mother was a goblin!",
        "Thief! And your mother was a half goblin!",
        "Thief! And your father was an orc!",
        "Thief! May a camel drool on you!",
        "Thief! May your drown in your own drool!",
    };

    thingp shopkeeper = thing_server_find(t->in_shop_owned_by_thing_id);
    if (!shopkeeper) {
        MSG_SERVER_SHOUT_AT_PLAYER(POPUP, t, "%%%%fg=red$%s", "You steal from children?");
        return;
    }

    if (thing_is_dead(shopkeeper)) {
        MSG_SERVER_SHOUT_AT_PLAYER(POPUP, t, "%%%%fg=red$%s", "You steal from the dead...");
        return;
    }

    MSG_SERVER_SHOUT_AT_PLAYER(POPUP, t,
                               "%%%%fg=red$%s", messages[myrand() % ARRAY_SIZE(messages)]);

    MSG_SERVER_SHOUT_AT_PLAYER(SOUND, t, "thief");
}

void shop_break_message (thingp t, thingp shopkeeper)
{
    if (!thing_is_player(t)) {
        return;
    }

    static const char *messages[] = {
        "You break my stuff, I break your legs!",
        "You break my stuff, I break your antennae!",
        "You break my stuff, I break your tentacles!",
        "You break my stuff, I break your probiscus!",
        "You break my stuff, I break your horns!",
        "You break my stuff, I break your long nose!",
        "You break my stuff, I break your arms!",
        "You break my stuff, I break your pinkies!",
        "You break my stuff, I break your wand!",
        "You break my stuff, I break your sword!",
    };

    if (thing_is_dead(shopkeeper)) {
        return;
    }

    thing_set_is_angry(shopkeeper, true);

    MSG_SERVER_SHOUT_AT_PLAYER(POPUP, t,
                               "%%%%fg=red$%s", messages[myrand() % ARRAY_SIZE(messages)]);
}

void shop_whodunnit_break_message (thingp t, thingp shopkeeper)
{
    if (!thing_is_player(t)) {
        return;
    }

    if (thing_is_dead(shopkeeper)) {
        return;
    }

    static const char *messages[] = {
        "Who broke my stuff! I'll get em",
        "Who broke my stuff! Was it you?",
    };

    MSG_SERVER_SHOUT_AT_PLAYER(POPUP, t,
                               "%%%%fg=orange$%s", messages[myrand() % ARRAY_SIZE(messages)]);
}

static thingp all_shop_floor_tiles[MAP_WIDTH][MAP_HEIGHT];
static thingp all_shopkeeper_tiles[MAP_WIDTH][MAP_HEIGHT];

static void shop_get_all_shop_floor_things (void)
{
    memset(all_shop_floor_tiles, 0, sizeof(all_shop_floor_tiles));

    thing_map *map = &thing_server_map;

    int x;
    int y;

    for (x = 0; x < MAP_WIDTH; x++) {
        for (y = 0; y < MAP_HEIGHT; y++) {

            thing_map_cell *cell = &map->cells[x][y];

            uint32_t i;
            for (i = 0; i < cell->count; i++) {
                thingp it;
                
                it = thing_server_id(cell->id[i]);

                if (thing_is_shop_floor(it)) {
                    all_shop_floor_tiles[x][y] = it;
                }
            }
        }
    }
}

static void shop_get_all_shopkeeper_things (void)
{
    memset(all_shopkeeper_tiles, 0, sizeof(all_shopkeeper_tiles));

    thing_map *map = &thing_server_map;

    int x;
    int y;

    for (x = 0; x < MAP_WIDTH; x++) {
        for (y = 0; y < MAP_HEIGHT; y++) {

            if (!all_shop_floor_tiles[x][y]) {
                continue;
            }

            thing_map_cell *cell = &map->cells[x][y];

            uint32_t i;
            for (i = 0; i < cell->count; i++) {
                thingp it;
                
                it = thing_server_id(cell->id[i]);

                if (thing_is_shopkeeper(it)) {
                    all_shopkeeper_tiles[x][y] = it;
                }
            }
        }
    }
}

static void shop_flood_own_things (thingp shopkeeper, int x, int y)
{
    if ((x < 0) || (y < 0) ||
        (x >= MAP_WIDTH) ||
        (y >= MAP_HEIGHT)) {
        return;
    }

    thingp floor = all_shop_floor_tiles[x][y];
    if (!floor) {
        return;
    }

    all_shop_floor_tiles[x][y] = 0;

    thing_map *map = &thing_server_map;
    thing_map_cell *cell = &map->cells[x][y];

    uint32_t i;
    for (i = 0; i < cell->count; i++) {
        thingp it;
        
        it = thing_server_id(cell->id[i]);

        if (thing_is_treasure(it)) {
            thing_set_owner(it, shopkeeper);
        }

        if (thing_is_shop_floor(it)) {
            thing_set_owner(it, shopkeeper);
        }
    }

    shop_flood_own_things(shopkeeper, x + 1, y);
    shop_flood_own_things(shopkeeper, x - 1, y);
    shop_flood_own_things(shopkeeper, x, y + 1);
    shop_flood_own_things(shopkeeper, x, y - 1);
}

static void shop_own_all_shopkeeper_things (void)
{
    int x;
    int y;

    for (x = 0; x < MAP_WIDTH; x++) {
        for (y = 0; y < MAP_HEIGHT; y++) {
            thingp shopkeeper = all_shopkeeper_tiles[x][y];
            if (!shopkeeper) {
                continue;
            }

            shop_flood_own_things(shopkeeper, x, y);
        }
    }
}

void shop_fixup (void)
{
    shop_get_all_shop_floor_things();
    shop_get_all_shopkeeper_things();
    shop_own_all_shopkeeper_things();
}

void thing_shop_item_tick (thingp t)
{
    /*
     * Treasure that has an owner is in a shop.
     */
    if (thing_owner(t)) {
        MSG_SERVER_SHOUT_OVER_THING(POPUP, t,
                                    "%%%%font=%s$%%%%fg=%s$%d$", 
                                    "medium", "gold", tp_get_cost(t->tp));
    }
}

/*
 * Is a thing close to or inside a shop?
 */
int shop_inside (thingp t)
{
    thing_map *map = &thing_server_map;

    int x;
    int y;
    int dx;
    int dy;

    for (dx = -2; dx <= 2; dx++) {
        for (dy = -2; dy <= 2; dy++) {

            x = t->x + dx;
            y = t->y + dy;

            thing_map_cell *cell = &map->cells[x][y];

            uint32_t i;
            for (i = 0; i < cell->count; i++) {
                thingp it;
                
                it = thing_server_id(cell->id[i]);

                if (thing_is_shop_floor(it)) {
                    return (true);
                }
            }
        }
    }

    return (false);
}

void shop_on_level (void)
{
    socket_tx_server_shout_at_all_players(SOUND, "cash_register");
    socket_tx_server_shout_at_all_players(INFO, "You hear the chime of a cash register");
}
