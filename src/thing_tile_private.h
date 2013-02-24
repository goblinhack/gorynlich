/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file.
 */

/*
 * Single animation tile.
 */
typedef struct thing_tile_ {
    tree_key_int tree;

    char *tile;
    char *command;

    /*
     * Delay in ms between frames.
     */
    uint32_t delay;

    uint8_t is_join_block:1;
    uint8_t is_join_horiz:1;
    uint8_t is_join_vert:1;
    uint8_t is_join_node:1;
    uint8_t is_join_left:1;
    uint8_t is_join_right:1;
    uint8_t is_join_top:1;
    uint8_t is_join_bot:1;
    uint8_t is_join_tl:1;
    uint8_t is_join_tr:1;
    uint8_t is_join_bl:1;
    uint8_t is_join_br:1;
    uint8_t is_join_t:1;
    uint8_t is_join_t90:1;
    uint8_t is_join_t180:1;
    uint8_t is_join_t270:1;
    uint8_t is_join_x:1;
    uint8_t is_join_tl2:1;
    uint8_t is_join_tr2:1;
    uint8_t is_join_bl2:1;
    uint8_t is_join_br2:1;
    uint8_t is_join_t_1:1;
    uint8_t is_join_t_2:1;
    uint8_t is_join_t_3:1;
    uint8_t is_join_t90_1:1;
    uint8_t is_join_t90_2:1;
    uint8_t is_join_t90_3:1;
    uint8_t is_join_t180_1:1;
    uint8_t is_join_t180_2:1;
    uint8_t is_join_t180_3:1;
    uint8_t is_join_t270_1:1;
    uint8_t is_join_t270_2:1;
    uint8_t is_join_t270_3:1;
    uint8_t is_yyy1:1;
    uint8_t is_yyy2:1;
    uint8_t is_yyy3:1;
    uint8_t is_yyy4:1;
    uint8_t is_yyy5:1;
    uint8_t is_yyy6:1;
    uint8_t is_yyy7:1;
    uint8_t is_yyy8:1;
    uint8_t is_yyy9:1;
    uint8_t is_yyy10:1;
    uint8_t is_yyy11:1;
    uint8_t is_yyy12:1;
    uint8_t is_yyy13:1;
    uint8_t is_yyy14:1;
    uint8_t is_yyy15:1;
    uint8_t is_yyy16:1;
    uint8_t is_yyy17:1;
    uint8_t is_open:1;
    uint8_t is_dead:1;
    uint8_t is_end_of_anim:1;

} thing_tile;
