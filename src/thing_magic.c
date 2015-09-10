/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */


#include "main.h"
#include "thing.h"
#include "wid_game_map_server.h"
#include "wid_game_map_client.h"
#include "time_util.h"

/*
 * Build up magic to be fired later.
 */
void thing_server_magic_powerup (thingp t)
{
    int magic = thing_stats_get_magic(t);

    if (magic > 1) {
        magic = 1;
    }

    if (magic == 0) {
        return;
    }

    t->magic_powerup += magic;

    thing_stats_modify_magic(t, -magic);

    t->timestamp_magic_powerup = time_get_time_ms();

    const char *magic_anim = tp_magic_anim(t->tp);

    tpp what = tp_find(magic_anim);
    if (!what) {
        THING_ERR(t, "cannot use magic effect %s", magic_anim);
        return;
    }

    thing_wield_magic(t, what);
}

/*
 * If enough time has passed since the last press of the m key on the client, 
 * fire!
 */
void thing_server_magic_fire (thingp t)
{
    if (!t->magic_powerup) {
        return;
    }

    if (!time_have_x_tenths_passed_since(2, t->timestamp_magic_powerup)) {
        return;
    }

    /*
     * Cannot magic until we're on a level.
     */
    if (!t->wid) {
        THING_LOG(t, "cannot magic yet, not on the level");
        return;
    }

    /*
     * Use the currently wielded weapon.
     */
    tpp weapon = thing_weapon(t);
    if (!weapon) {
        THING_LOG(t, "has no weapon, cannot magic");
        return;
    }

    /*
     * Does it potentially break each use?
     */
    thing_weapon_check_for_damage_on_firing(t, weapon);

    double dx, dy;

    /*
     * Any smaller than this and diagonal shots collide with adjoining walls.
     */
    double dist_from_player = 0.7;

    /*
     * Try current direction.
     */
    dx = 0.0;
    dy = 0.0;

    if (thing_is_dir_down(t)) {
        dy = dist_from_player;
    }

    if (thing_is_dir_up(t)) {
        dy = -dist_from_player;
    }

    if (thing_is_dir_right(t)) {
        dx = dist_from_player;
    }

    if (thing_is_dir_left(t)) {
        dx = -dist_from_player;
    }

    if (thing_is_dir_tl(t)) {
        dx = -dist_from_player;
        dy = -dist_from_player;
    }

    if (thing_is_dir_tr(t)) {
        dx = dist_from_player;
        dy = -dist_from_player;
    }

    if (thing_is_dir_bl(t)) {
        dx = -dist_from_player;
        dy = dist_from_player;
    }

    if (thing_is_dir_br(t)) {
        dx = dist_from_player;
        dy = dist_from_player;
    }

    /*
     * magic from the player position plus the initial delta so it looks like 
     * it comes from outside of the body.
     */
    double x = t->x;
    double y = t->y;

    x += dx;
    y += dy;

    tpp projectile = tp_find("magic1");
    if (!projectile) {
        /*
         * Might be a sword.
         */
        thing_swing(t);
        return;
    }

    widp w = wid_game_map_server_replace_tile(
                                    wid_game_map_server_grid_container,
                                    x,
                                    y,
                                    0, /* thing */
                                    projectile,
                                    0, /* tpp data */
                                    0, /* item */
                                    0 /* stats */);

    thingp p = wid_get_thing(w);

    /*
     * Make sure we keep track of who magicd so we can award scores.
     */
    thing_set_owner(p, t);

    double scale = 0.02 * (double) t->magic_powerup;
    p->scale = scale;

    p->damage = t->magic_powerup;
    p->stats.hp = t->magic_powerup;

    t->magic_powerup = 0;

    /*
     * Round up say -0.7 to -1.0
     */
    dx *= 10.0;
    dy *= 10.0;
    dx /= (dist_from_player * 10.0);
    dy /= (dist_from_player * 10.0);

    p->dx = dx;
    p->dy = dy;
    p->dir = t->dir;

    /*
     * Check for immediate collision with a wall
     */
    thing_handle_collisions(wid_game_map_server_grid_container, p);
    if (thing_is_dead_or_dying(p)) {
        return;
    }

    double fnexthop_x = p->x + p->dx;
    double fnexthop_y = p->y + p->dy;

    thing_server_move(p,
                      fnexthop_x,
                      fnexthop_y,
                      fnexthop_y < p->y,
                      fnexthop_y > p->y,
                      fnexthop_x < p->x,
                      fnexthop_x > p->x,
                      false, /* magic */
                      false  /* magic */);

    thing_unwield_magic(t);
}

void thing_set_magic_anim_id (thingp t, uint32_t magic_anim_id)
{
    thingp magic_anim;

    if (!magic_anim_id) {
        thing_set_magic_anim(t, 0);
        return;
    }

    if (t->on_server) {
        magic_anim = thing_server_find(magic_anim_id);
    } else {
        magic_anim = thing_client_find(magic_anim_id);
    }

    thing_set_magic_anim(t, magic_anim);
}

void thing_set_magic_anim (thingp t, thingp magic_anim)
{
    if (magic_anim) {
        verify(magic_anim);
    }

    thingp old_magic_anim = thing_magic_anim(t);

    if (old_magic_anim) {
        if (old_magic_anim == magic_anim) {
            return;
        }

        if (magic_anim) {
            THING_LOG(t, "magic carry changed, %s->%s",
                      thing_logname(old_magic_anim),
                      thing_logname(magic_anim));
        } else {
            THING_LOG(t, "remove magic carry animation, %s",
                      thing_logname(old_magic_anim));
        }
    } else {
        if (magic_anim) {
            THING_LOG(t, "magic carry anim now, %s",
                      thing_logname(magic_anim));
        }
    }

    if (magic_anim) {
        t->magic_anim_thing_id = magic_anim->thing_id;

        /*
         * Scale up weapons so they look the same size as the carryer.
         */
        double scale = tp_get_scale(t->tp);
        if (scale != 1.0) {
            wid_scaling_blit_to_pct_in(magic_anim->wid, scale, scale, 500, 9999999);
        }
    } else {
        t->magic_anim_thing_id = 0;
    }
}

thingp thing_magic_anim (thingp t)
{
    thingp magic_anim = 0;

    if (t->on_server) {
        if (t->magic_anim_thing_id) {
            magic_anim = thing_server_id(t->magic_anim_thing_id);
        }
    } else {
        if (t->magic_anim_thing_id) {
            magic_anim = thing_client_id(t->magic_anim_thing_id);
        }
    }

    return (magic_anim);
}

void thing_magic_sheath (thingp t)
{
    tpp magic = thing_magic(t);
    if (!magic) {
        return;
    }

    THING_LOG(t, "sheathing magic %s", tp_short_name(magic));

    /*
     * If this magic has its own thing id for animations then destroy that.
     */
    thingp magic_anim = thing_magic_anim(t);
    if (magic_anim) {
        THING_LOG(t, "unwield, carry anim magic %s", 
                  thing_logname(magic_anim));
        thing_dead(magic_anim, 0, "owner magic");
        thing_set_magic_anim(t, 0);
    }

    t->magic_anim = 0;

    if (t->on_server) {
        thing_update(t);
    }
}

void thing_unwield_magic (thingp t)
{
    tpp magic = thing_magic(t);
    if (!magic) {
        return;
    }

    THING_LOG(t, "unwielding weapon magic %s", tp_short_name(magic));

    thing_magic_sheath(t);
}

void thing_wield_magic (thingp t, tpp magic)
{
    thing_unwield_magic(t);

    if (thing_is_player(t)) {
        THING_LOG(t, "unwield weapon magic %s", tp_short_name(magic));
    }

    /*
     * Find out what to use as the sheild.
     */
    tpp what = 0;
    const char *as = tp_magic_anim(magic);
    if (!as) {
        as = tp_magic_anim(t->tp);
        if (!as) {
            what = magic;
        } else {
            what = tp_find(as);
            if (!what) {
                THING_ERR(t, "Could not find %s to wield", as);
                return;
            }
        }
    } else {
        what = tp_find(as);
        if (!what) {
            THING_ERR(t, "Could not find %s to wield", as);
            return;
        }
    }

    if (!what) {
        THING_ERR(t, "Could not use magic");
        return;
    }

    t->magic_anim = magic;

    widp magic_anim_wid;

    if (t->on_server) {
        magic_anim_wid = wid_game_map_server_replace_tile(
                                wid_game_map_server_grid_container,
                                t->x,
                                t->y,
                                0, /* thing */
                                what,
                                0, /* tpp data */
                                0 /* item */,
                                0 /* stats */);
    } else {
        magic_anim_wid = wid_game_map_client_replace_tile(
                                wid_game_map_client_grid_container,
                                t->x, t->y, 0, what);
    }

    /*
     * Save the thing id so the client wid can keep track of the magic.
     */
    thingp child = wid_get_thing(magic_anim_wid);
    thing_set_magic_anim(t, child);

    child->dir = t->dir;

    child->scale = ((double)t->magic_powerup) * 0.02;

    /*
     * Attach to the thing.
     */
    thing_set_owner(child, t);

    if (child->on_server) {
        thing_update(child);
    } else {
        thing_client_wid_update(child, child->x, child->y, 
                                false /* smooth */,
                                false /* is new */);
    }
}

widp thing_get_magic_anim_wid (thingp t)
{
    thingp magic = thing_magic_anim(t);
    if (!magic) {
        return (0);
    }

    return (thing_wid(magic));
}

void thing_magic_tick (thingp t)
{
    /*
     * Auto heal
     */
    if (thing_stats_get_magic(t) < thing_stats_get_max_magic(t)) {
        int delta = thing_stats_val_to_modifier(thing_stats_get_toughness(t));
        if (delta > 0) {
            thing_stats_modify_magic(t, delta);
        }
    }

    /*
     * If over max magic, return to max.
     */
    int delta = thing_stats_get_magic(t) - thing_stats_get_max_magic(t);
    if (delta > 0) {
        thing_stats_modify_magic(t, -1);
    }
}
