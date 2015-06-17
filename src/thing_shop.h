/*
 * Copyright (C) 2011 Neil McGill
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
#include "time_util.h"

void shop_enter_message(thingp t);
void shop_collect_message(thingp t);
void shop_purchase_message(thingp t);
void shop_steal_message(thingp t);
void shop_welcom_message(thingp t);
