/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file.
 */

typedef struct thing_template_ {
    tree_key_string tree;
    tree_key_int tree2;

    /*
     * Internal description of the thing.
     */
    char *shortname;

    /*
     * End user description of the thing.
     */
    char *tooltip;

    /*
     * In relation to other widgets, where are we.
     */
    uint8_t z_depth;
    uint8_t z_order;

    /*
     * Animation tiles.
     */
    tree_rootp tiles;
    tree_rootp tiles2;

    /*
     * Speed in milliseconds it takes to move one tile.
     */
    uint32_t speed;

    /*
     * Lifespan in milliseconds.
     */
    uint32_t lifespan;

    /*
     * Various bounties.
     */
    uint32_t score_on_death;
    uint32_t score_on_collect;

    uint32_t ppp1;
    uint32_t ppp2;
    uint32_t ppp3;
    uint32_t ppp4;
    uint32_t ppp5;
    uint32_t ppp6;
    uint32_t ppp7;
    uint32_t ppp8;
    uint32_t ppp9;
    uint32_t ppp10;
    uint32_t ppp11;
    uint32_t ppp12;
    uint32_t ppp13;
    uint32_t ppp14;
    uint32_t ppp15;
    uint32_t ppp16;
    uint32_t ppp17;
    uint32_t ppp18;
    uint32_t ppp19;
    uint32_t ppp20;

    uint8_t is_exit:1;
    uint8_t is_floor:1;
    uint8_t is_food:1;
    uint8_t is_letter:1;
    uint8_t is_monst:1;
    uint8_t is_plant:1;
    uint8_t is_player:1;
    uint8_t is_snail:1;
    uint8_t is_star_yellow:1;
    uint8_t is_rock:1;
    uint8_t is_xxx2:1;
    uint8_t is_xxx3:1;
    uint8_t is_xxx4:1;
    uint8_t is_xxx5:1;
    uint8_t is_xxx6:1;
    uint8_t is_xxx7:1;
    uint8_t is_xxx8:1;
    uint8_t is_star:1;
    uint8_t is_powerup_spam:1;
    uint8_t is_rock_0:1;
    uint8_t is_car:1;
    uint8_t is_star_green:1;
    uint8_t is_star_cyan:1;
    uint8_t is_star_black:1;
    uint8_t is_star_purple:1;
    uint8_t is_explosion:1;
    uint8_t is_spikes:1;
    uint8_t is_star_pink:1;
    uint8_t is_star_red:1;
    uint8_t is_star_blue:1;
    uint8_t is_seedpod:1;
    uint8_t is_bomb:1;
    uint8_t is_spam:1;
    uint8_t is_road:1;
    uint8_t is_pipe:1;
    uint8_t is_item_removed_at_level_end:1;
    uint8_t is_scarable:1;
    uint8_t is_shrunk_when_carried:1;
    uint8_t is_hidden_from_editor:1;
    uint8_t is_animated:1;
    uint8_t is_follows_owner:1;
    uint8_t is_powerup_rocket:1;
    uint8_t is_left_as_corpse_on_death:1;
    uint8_t is_item_perma:1;
    uint8_t is_esnail:1;
    uint8_t is_item_hidden:1;
    uint8_t is_bonus_letter:1;
    uint8_t is_thing:1;
    uint8_t is_joinable:1;
    uint8_t is_wall:1;
    uint8_t is_effect_sway:1;
    uint8_t is_effect_pulse:1;
    uint8_t is_effect_rotate_4way:1;
    uint8_t is_effect_rotate_2way:1;

} thing_template;
