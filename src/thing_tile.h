/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file.
 */

void demarshal_thing_tiles(demarshal_p, tpp);
void marshal_thing_tiles(marshal_p, tpp);
void thing_tile_free(thing_tilep);

const char *thing_tile_name(thing_tilep);
uint32_t thing_tile_delay(thing_tilep);
uint32_t thing_tile_move(thing_tilep);
const char *thing_tile_command(thing_tilep);
uint8_t thing_tile_is_moving(thing_tilep);
uint8_t thing_tile_begin_jump(thing_tilep);
uint8_t thing_tile_is_jumping(thing_tilep);
uint8_t thing_tile_is_end_of_jump(thing_tilep);
uint8_t thing_tile_is_join_block(thing_tilep);
uint8_t thing_tile_is_join_horiz(thing_tilep);
uint8_t thing_tile_is_join_vert(thing_tilep);
uint8_t thing_tile_is_join_node(thing_tilep);
uint8_t thing_tile_is_join_left(thing_tilep);
uint8_t thing_tile_is_join_right(thing_tilep);
uint8_t thing_tile_is_join_top(thing_tilep);
uint8_t thing_tile_is_join_bot(thing_tilep);
uint8_t thing_tile_is_join_tl(thing_tilep);
uint8_t thing_tile_is_join_tr(thing_tilep);
uint8_t thing_tile_is_join_bl(thing_tilep);
uint8_t thing_tile_is_join_br(thing_tilep);
uint8_t thing_tile_is_join_t(thing_tilep);
uint8_t thing_tile_is_join_t90(thing_tilep);
uint8_t thing_tile_is_join_t180(thing_tilep);
uint8_t thing_tile_is_join_t270(thing_tilep);
uint8_t thing_tile_is_join_x(thing_tilep);
uint8_t thing_tile_is_join_tl2(thing_tilep);
uint8_t thing_tile_is_join_tr2(thing_tilep);
uint8_t thing_tile_is_join_bl2(thing_tilep);
uint8_t thing_tile_is_join_br2(thing_tilep);
uint8_t thing_tile_is_join_t_1(thing_tilep);
uint8_t thing_tile_is_join_t_2(thing_tilep);
uint8_t thing_tile_is_join_t_3(thing_tilep);
uint8_t thing_tile_is_join_t90_1(thing_tilep);
uint8_t thing_tile_is_join_t90_2(thing_tilep);
uint8_t thing_tile_is_join_t90_3(thing_tilep);
uint8_t thing_tile_is_join_t180_1(thing_tilep);
uint8_t thing_tile_is_join_t180_2(thing_tilep);
uint8_t thing_tile_is_join_t180_3(thing_tilep);
uint8_t thing_tile_is_join_t270_1(thing_tilep);
uint8_t thing_tile_is_join_t270_2(thing_tilep);
uint8_t thing_tile_is_join_t270_3(thing_tilep);
uint8_t thing_tile_is_join_x(thing_tilep);
uint8_t thing_tile_is_join_x1(thing_tilep);
uint8_t thing_tile_is_join_x1_270(thing_tilep);
uint8_t thing_tile_is_join_x1_180(thing_tilep);
uint8_t thing_tile_is_join_x1_90(thing_tilep);
uint8_t thing_tile_is_join_x2(thing_tilep);
uint8_t thing_tile_is_join_x2_270(thing_tilep);
uint8_t thing_tile_is_join_x2_180(thing_tilep);
uint8_t thing_tile_is_join_x2_90(thing_tilep);
uint8_t thing_tile_is_join_x3(thing_tilep);
uint8_t thing_tile_is_join_x3_180(thing_tilep);
uint8_t thing_tile_is_join_x4(thing_tilep);
uint8_t thing_tile_is_join_x4_270(thing_tilep);
uint8_t thing_tile_is_join_x4_180(thing_tilep);
uint8_t thing_tile_is_join_x4_90(thing_tilep);
uint8_t thing_tile_is_join_horiz2(thing_tilep);
uint8_t thing_tile_is_join_vert2(thing_tilep);
uint8_t thing_tile_is_dir_up(thing_tilep);
uint8_t thing_tile_is_dir_down(thing_tilep);
uint8_t thing_tile_is_dir_left(thing_tilep);
uint8_t thing_tile_is_dir_right(thing_tilep);
uint8_t thing_tile_is_dir_tl(thing_tilep);
uint8_t thing_tile_is_dir_bl(thing_tilep);
uint8_t thing_tile_is_dir_tr(thing_tilep);
uint8_t thing_tile_is_dir_br(thing_tilep);
uint8_t thing_tile_is_yyy5(thing_tilep);
uint8_t thing_tile_is_yyy6(thing_tilep);
uint8_t thing_tile_is_yyy7(thing_tilep);
uint8_t thing_tile_is_yyy8(thing_tilep);
uint8_t thing_tile_is_yyy9(thing_tilep);
uint8_t thing_tile_is_yyy10(thing_tilep);
uint8_t thing_tile_is_yyy11(thing_tilep);
uint8_t thing_tile_is_yyy12(thing_tilep);
uint8_t thing_tile_is_yyy13(thing_tilep);
uint8_t thing_tile_is_yyy14(thing_tilep);
uint8_t thing_tile_is_yyy15(thing_tilep);
uint8_t thing_tile_is_yyy16(thing_tilep);
uint8_t thing_tile_is_sleeping(thing_tilep);
uint8_t thing_tile_is_open(thing_tilep);
uint8_t thing_tile_is_dead(thing_tilep);
uint8_t thing_tile_is_dying(thing_tilep);
uint8_t thing_tile_is_end_of_anim(thing_tilep);
uint8_t thing_tile_is_dead_on_end_of_anim(thing_tilep);
uint8_t thing_tile_is_dead_on_end_of_anim_on_server_only(thing_tilep);

thing_tilep thing_tile_find(tpp, uint32_t index,
                            tilep *tile);
thing_tilep thing_tile_first(tree_rootp root);
thing_tilep thing_tile_next(tree_rootp root, thing_tilep in);
thing_tilep thing_tile_random(tree_rootp root);

