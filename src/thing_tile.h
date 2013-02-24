/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file.
 */

void demarshal_thing_tiles(demarshal_p, thing_templatep);
void marshal_thing_tiles(marshal_p, thing_templatep);
void demarshal_thing_tiles2(demarshal_p, thing_templatep);
void marshal_thing_tiles2(marshal_p, thing_templatep);
void thing_tile_free(thing_tilep);

const char *thing_tile_name(thing_tilep);
uint32_t thing_tile_delay(thing_tilep);
const char *thing_tile_command(thing_tilep);
boolean thing_tile_is_join_block(thing_tilep);
boolean thing_tile_is_join_horiz(thing_tilep);
boolean thing_tile_is_join_vert(thing_tilep);
boolean thing_tile_is_join_node(thing_tilep);
boolean thing_tile_is_join_left(thing_tilep);
boolean thing_tile_is_join_right(thing_tilep);
boolean thing_tile_is_join_top(thing_tilep);
boolean thing_tile_is_join_bot(thing_tilep);
boolean thing_tile_is_join_tl(thing_tilep);
boolean thing_tile_is_join_tr(thing_tilep);
boolean thing_tile_is_join_bl(thing_tilep);
boolean thing_tile_is_join_br(thing_tilep);
boolean thing_tile_is_join_t(thing_tilep);
boolean thing_tile_is_join_t90(thing_tilep);
boolean thing_tile_is_join_t180(thing_tilep);
boolean thing_tile_is_join_t270(thing_tilep);
boolean thing_tile_is_join_x(thing_tilep);
boolean thing_tile_is_join_tl2(thing_tilep);
boolean thing_tile_is_join_tr2(thing_tilep);
boolean thing_tile_is_join_bl2(thing_tilep);
boolean thing_tile_is_join_br2(thing_tilep);
boolean thing_tile_is_join_t_1(thing_tilep);
boolean thing_tile_is_join_t_2(thing_tilep);
boolean thing_tile_is_join_t_3(thing_tilep);
boolean thing_tile_is_join_t90_1(thing_tilep);
boolean thing_tile_is_join_t90_2(thing_tilep);
boolean thing_tile_is_join_t90_3(thing_tilep);
boolean thing_tile_is_join_t180_1(thing_tilep);
boolean thing_tile_is_join_t180_2(thing_tilep);
boolean thing_tile_is_join_t180_3(thing_tilep);
boolean thing_tile_is_join_t270_1(thing_tilep);
boolean thing_tile_is_join_t270_2(thing_tilep);
boolean thing_tile_is_join_t270_3(thing_tilep);
boolean thing_tile_is_yyy1(thing_tilep);
boolean thing_tile_is_yyy2(thing_tilep);
boolean thing_tile_is_yyy3(thing_tilep);
boolean thing_tile_is_yyy4(thing_tilep);
boolean thing_tile_is_yyy5(thing_tilep);
boolean thing_tile_is_yyy6(thing_tilep);
boolean thing_tile_is_yyy7(thing_tilep);
boolean thing_tile_is_yyy8(thing_tilep);
boolean thing_tile_is_yyy9(thing_tilep);
boolean thing_tile_is_yyy10(thing_tilep);
boolean thing_tile_is_yyy11(thing_tilep);
boolean thing_tile_is_yyy12(thing_tilep);
boolean thing_tile_is_yyy13(thing_tilep);
boolean thing_tile_is_yyy14(thing_tilep);
boolean thing_tile_is_yyy15(thing_tilep);
boolean thing_tile_is_yyy16(thing_tilep);
boolean thing_tile_is_yyy17(thing_tilep);
boolean thing_tile_is_open(thing_tilep);
boolean thing_tile_is_dead(thing_tilep);
boolean thing_tile_is_end_of_anim(thing_tilep);

thing_tilep thing_tile_find(thing_templatep,
            boolean is_join_block,
            boolean is_join_horiz,
            boolean is_join_vert,
            boolean is_join_node,
            boolean is_join_left,
            boolean is_join_right,
            boolean is_join_top,
            boolean is_join_bot,
            boolean is_join_tl,
            boolean is_join_tr,
            boolean is_join_bl,
            boolean is_join_br,
            boolean is_join_t,
            boolean is_join_t90,
            boolean is_join_t180,
            boolean is_join_t270,
            boolean is_join_x,
            boolean is_join_tl2,
            boolean is_join_tr2,
            boolean is_join_bl2,
            boolean is_join_br2,
            boolean is_join_t_1,
            boolean is_join_t_2,
            boolean is_join_t_3,
            boolean is_join_t90_1,
            boolean is_join_t90_2,
            boolean is_join_t90_3,
            boolean is_join_t180_1,
            boolean is_join_t180_2,
            boolean is_join_t180_3,
            boolean is_join_t270_1,
            boolean is_join_t270_2,
            boolean is_join_t270_3);

thing_tilep thing_tile_first(tree_rootp root);
thing_tilep thing_tile_next(tree_rootp root, thing_tilep in);

