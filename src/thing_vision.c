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
#include "thing.h"
#include "client.h"
#include "socket_util.h"

double thing_stats_get_total_vision (thingp t, double vision)
{
    double modifier = thing_stats_val_to_modifier(thing_stats_get_vision(t));
    double final_vision = ceil(vision + (vision * (modifier / 10.0)));

    /*
     * Allow dark vision once modifiers are high enough.
     */
    if (modifier > 0) {
        if (final_vision <= modifier / 2.0) {
            final_vision = modifier / 2.0;
        }
    }

    if (final_vision < 0) {
        final_vision = 0;
    }

    return (final_vision);
}
