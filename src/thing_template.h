/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file.
 */

boolean thing_template_init(void);
void thing_template_fini(void);
thing_templatep thing_template_load(const char *file);
thing_templatep thing_template_find(const char *name);
void demarshal_thing_template(demarshal_p ctx, thing_templatep);
void marshal_thing_template(marshal_p ctx, thing_templatep);
void thing_templates_marshal(marshal_p out);
boolean thing_test(int32_t argc, char *argv[]);
thing_templatep string2thing_template(const char **s);

const char *thing_template_name(thing_templatep);
const char *thing_template_shortname(thing_templatep);
const char *thing_template_get_tooltip(thing_templatep);
uint8_t thing_template_get_z_depth(thing_templatep);
uint8_t thing_template_get_z_order(thing_templatep);
uint32_t thing_template_get_speed(thing_templatep);
uint32_t thing_template_get_lifespan(thing_templatep);
uint32_t thing_template_get_score_on_death(thing_templatep);
uint32_t thing_template_get_score_on_collect(thing_templatep);
uint32_t thing_template_get_ppp1(thing_templatep);
uint32_t thing_template_get_ppp2(thing_templatep);
uint32_t thing_template_get_ppp3(thing_templatep);
uint32_t thing_template_get_ppp4(thing_templatep);
uint32_t thing_template_get_ppp5(thing_templatep);
uint32_t thing_template_get_ppp6(thing_templatep);
uint32_t thing_template_get_ppp7(thing_templatep);
uint32_t thing_template_get_ppp8(thing_templatep);
uint32_t thing_template_get_ppp9(thing_templatep);
uint32_t thing_template_get_ppp10(thing_templatep);
uint32_t thing_template_get_ppp11(thing_templatep);
uint32_t thing_template_get_ppp12(thing_templatep);
uint32_t thing_template_get_ppp13(thing_templatep);
uint32_t thing_template_get_ppp14(thing_templatep);
uint32_t thing_template_get_ppp15(thing_templatep);
uint32_t thing_template_get_ppp16(thing_templatep);
uint32_t thing_template_get_ppp17(thing_templatep);
uint32_t thing_template_get_ppp18(thing_templatep);
uint32_t thing_template_get_ppp19(thing_templatep);
uint32_t thing_template_get_ppp20(thing_templatep);

boolean thing_template_is_player(thing_templatep);
boolean thing_template_is_monst(thing_templatep);
boolean thing_template_is_snail(thing_templatep);
boolean thing_template_is_wall(thing_templatep);
boolean thing_template_is_food(thing_templatep);
boolean thing_template_is_floor(thing_templatep);
boolean thing_template_is_exit(thing_templatep);
boolean thing_template_is_star_yellow(thing_templatep);
boolean thing_template_is_rock(thing_templatep);
boolean thing_template_is_water(thing_templatep);
boolean thing_template_is_lava(thing_templatep);
boolean thing_template_is_xxx4(thing_templatep);
boolean thing_template_is_xxx5(thing_templatep);
boolean thing_template_is_xxx6(thing_templatep);
boolean thing_template_is_xxx7(thing_templatep);
boolean thing_template_is_xxx8(thing_templatep);
boolean thing_template_is_star(thing_templatep);
boolean thing_template_is_powerup_spam(thing_templatep);
boolean thing_template_is_rock_0(thing_templatep);
boolean thing_template_is_car(thing_templatep);
boolean thing_template_is_star_green(thing_templatep);
boolean thing_template_is_star_cyan(thing_templatep);
boolean thing_template_is_star_black(thing_templatep);
boolean thing_template_is_star_purple(thing_templatep);
boolean thing_template_is_explosion(thing_templatep);
boolean thing_template_is_spikes(thing_templatep);
boolean thing_template_is_star_red(thing_templatep);
boolean thing_template_is_star_pink(thing_templatep);
boolean thing_template_is_star_blue(thing_templatep);
boolean thing_template_is_seedpod(thing_templatep);
boolean thing_template_is_bomb(thing_templatep);
boolean thing_template_is_spam(thing_templatep);
boolean thing_template_is_road(thing_templatep);
boolean thing_template_is_pipe(thing_templatep);
boolean thing_template_is_item_removed_at_level_end(thing_templatep);
boolean thing_template_is_scarable(thing_templatep);
boolean thing_template_is_shrunk_when_carried(thing_templatep);
boolean thing_template_is_hidden_from_editor(thing_templatep);
boolean thing_template_is_animated(thing_templatep);
boolean thing_template_is_follows_owner(thing_templatep);
boolean thing_template_is_powerup_rocket(thing_templatep);
boolean thing_template_is_left_as_corpse_on_death(thing_templatep);
boolean thing_template_is_item_perma(thing_templatep);
boolean thing_template_is_esnail(thing_templatep);
boolean thing_template_is_item_hidden(thing_templatep);
boolean thing_template_is_bonus_letter(thing_templatep);
boolean thing_template_is_thing(thing_templatep);
boolean thing_template_is_plant(thing_templatep);
boolean thing_template_is_letter(thing_templatep);
boolean thing_template_is_joinable(thing_templatep);
boolean thing_template_is_effect_sway(thing_templatep);
boolean thing_template_is_effect_pulse(thing_templatep);
boolean thing_template_is_effect_rotate_4way(thing_templatep);
boolean thing_template_is_effect_rotate_2way(thing_templatep);

int16_t thing_template_path_cost_is_exit(thing_templatep);
int16_t thing_template_path_cost_is_floor(thing_templatep);
int16_t thing_template_path_cost_is_food(thing_templatep);
int16_t thing_template_path_cost_is_letter(thing_templatep);
int16_t thing_template_path_cost_is_monst(thing_templatep);
int16_t thing_template_path_cost_is_plant(thing_templatep);
int16_t thing_template_path_cost_is_player(thing_templatep);
int16_t thing_template_path_cost_is_snail(thing_templatep);
int16_t thing_template_path_cost_is_star_yellow(thing_templatep);
int16_t thing_template_path_cost_is_rock(thing_templatep);
int16_t thing_template_path_cost_is_water(thing_templatep);
int16_t thing_template_path_cost_is_lava(thing_templatep);
int16_t thing_template_path_cost_is_xxx4(thing_templatep);
int16_t thing_template_path_cost_is_xxx5(thing_templatep);
int16_t thing_template_path_cost_is_xxx6(thing_templatep);
int16_t thing_template_path_cost_is_xxx7(thing_templatep);
int16_t thing_template_path_cost_is_xxx8(thing_templatep);
int16_t thing_template_path_cost_is_star(thing_templatep);
int16_t thing_template_path_cost_is_powerup_spam(thing_templatep);
int16_t thing_template_path_cost_is_rock_0(thing_templatep);
int16_t thing_template_path_cost_is_car(thing_templatep);
int16_t thing_template_path_cost_is_star_green(thing_templatep);
int16_t thing_template_path_cost_is_star_cyan(thing_templatep);
int16_t thing_template_path_cost_is_star_black(thing_templatep);
int16_t thing_template_path_cost_is_star_purple(thing_templatep);
int16_t thing_template_path_cost_is_explosion(thing_templatep);
int16_t thing_template_path_cost_is_spikes(thing_templatep);
int16_t thing_template_path_cost_is_star_red(thing_templatep);
int16_t thing_template_path_cost_is_star_pink(thing_templatep);
int16_t thing_template_path_cost_is_star_blue(thing_templatep);
int16_t thing_template_path_cost_is_seedpod(thing_templatep);
int16_t thing_template_path_cost_is_bomb(thing_templatep);
int16_t thing_template_path_cost_is_spam(thing_templatep);
int16_t thing_template_path_cost_is_road(thing_templatep);
int16_t thing_template_path_cost_is_pipe(thing_templatep);
int16_t thing_template_path_cost_is_item_removed_at_level_end(thing_templatep);
int16_t thing_template_path_cost_is_scarable(thing_templatep);
int16_t thing_template_path_cost_is_shrunk_when_carried(thing_templatep);
int16_t thing_template_path_cost_is_hidden_from_editor(thing_templatep);
int16_t thing_template_path_cost_is_animated(thing_templatep);
int16_t thing_template_path_cost_is_follows_owner(thing_templatep);
int16_t thing_template_path_cost_is_powerup_rocket(thing_templatep);
int16_t thing_template_path_cost_is_left_as_corpse_on_death(thing_templatep);
int16_t thing_template_path_cost_is_item_perma(thing_templatep);
int16_t thing_template_path_cost_is_esnail(thing_templatep);
int16_t thing_template_path_cost_is_item_hidden(thing_templatep);
int16_t thing_template_path_cost_is_bonus_letter(thing_templatep);
int16_t thing_template_path_cost_is_thing(thing_templatep);
int16_t thing_template_path_cost_is_joinable(thing_templatep);
int16_t thing_template_path_cost_is_wall(thing_templatep);
int16_t thing_template_path_cost_is_effect_sway(thing_templatep);
int16_t thing_template_path_cost_is_effect_pulse(thing_templatep);
int16_t thing_template_path_cost_is_effect_rotate_4way(thing_templatep);
int16_t thing_template_path_cost_is_effect_rotate_2way(thing_templatep);

tree_rootp thing_template_get_tiles(thing_templatep);
tree_rootp thing_template_get_tiles2(thing_templatep);

extern tree_rootp thing_templates;
extern tree_rootp thing_templates_create_order;

enum {
    IS_JOIN_BLOCK,
    IS_JOIN_HORIZ,
    IS_JOIN_VERT,
    IS_JOIN_NODE,
    IS_JOIN_LEFT,
    IS_JOIN_RIGHT,
    IS_JOIN_TOP,
    IS_JOIN_BOT,
    IS_JOIN_TL,
    IS_JOIN_TR,
    IS_JOIN_BL,
    IS_JOIN_BR,
    IS_JOIN_T,
    IS_JOIN_T90,
    IS_JOIN_T180,
    IS_JOIN_T270,
    IS_JOIN_X,
    IS_JOIN_TL2,
    IS_JOIN_TR2,
    IS_JOIN_BL2,
    IS_JOIN_BR2,
    IS_JOIN_T_1,
    IS_JOIN_T_2,
    IS_JOIN_T_3,
    IS_JOIN_T90_1,
    IS_JOIN_T90_2,
    IS_JOIN_T90_3,
    IS_JOIN_T180_1,
    IS_JOIN_T180_2,
    IS_JOIN_T180_3,
    IS_JOIN_T270_1,
    IS_JOIN_T270_2,
    IS_JOIN_T270_3,
    IS_JOIN_X1,
    IS_JOIN_X1_270,
    IS_JOIN_X1_180,
    IS_JOIN_X1_90,
    IS_JOIN_X2,
    IS_JOIN_X2_270,
    IS_JOIN_X2_180,
    IS_JOIN_X2_90,
    IS_JOIN_X3,
    IS_JOIN_X3_180,
    IS_JOIN_X4,
    IS_JOIN_X4_270,
    IS_JOIN_X4_180,
    IS_JOIN_X4_90,
    IS_JOIN_MAX,
};

/*
 * Convert a join type into an index
 */
uint8_t thing_template_join_type_to_index(
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
            boolean is_join_t270_3,
            boolean is_join_x1,
            boolean is_join_x1_270,
            boolean is_join_x1_180,
            boolean is_join_x1_90,
            boolean is_join_x2,
            boolean is_join_x2_270,
            boolean is_join_x2_180,
            boolean is_join_x2_90,
            boolean is_join_x3,
            boolean is_join_x3_180,
            boolean is_join_x4,
            boolean is_join_x4_270,
            boolean is_join_x4_180,
            boolean is_join_x4_90);
