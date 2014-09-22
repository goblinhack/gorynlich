#include <stdint.h>
#include "ramdisk.h"

extern unsigned char *data_gfx_anim_eyes_png_start_ asm("data_gfx_anim_eyes_png_start_");
extern unsigned char *data_gfx_anim_eyes_png_end_ asm("data_gfx_anim_eyes_png_end_");
static const unsigned char *const data_gfx_anim_eyes_png_start =
    (const unsigned char *const) (char*)&data_gfx_anim_eyes_png_start_;
static const unsigned char *const data_gfx_anim_eyes_png_end   =
    (const unsigned char *const) (char*)&data_gfx_anim_eyes_png_end_;

extern unsigned char *data_gfx_anim_man_png_start_ asm("data_gfx_anim_man_png_start_");
extern unsigned char *data_gfx_anim_man_png_end_ asm("data_gfx_anim_man_png_end_");
static const unsigned char *const data_gfx_anim_man_png_start =
    (const unsigned char *const) (char*)&data_gfx_anim_man_png_start_;
static const unsigned char *const data_gfx_anim_man_png_end   =
    (const unsigned char *const) (char*)&data_gfx_anim_man_png_end_;

extern unsigned char *data_gfx_anim_treasure_chest_png_start_ asm("data_gfx_anim_treasure_chest_png_start_");
extern unsigned char *data_gfx_anim_treasure_chest_png_end_ asm("data_gfx_anim_treasure_chest_png_end_");
static const unsigned char *const data_gfx_anim_treasure_chest_png_start =
    (const unsigned char *const) (char*)&data_gfx_anim_treasure_chest_png_start_;
static const unsigned char *const data_gfx_anim_treasure_chest_png_end   =
    (const unsigned char *const) (char*)&data_gfx_anim_treasure_chest_png_end_;

extern unsigned char *data_gfx_cloudkill_png_start_ asm("data_gfx_cloudkill_png_start_");
extern unsigned char *data_gfx_cloudkill_png_end_ asm("data_gfx_cloudkill_png_end_");
static const unsigned char *const data_gfx_cloudkill_png_start =
    (const unsigned char *const) (char*)&data_gfx_cloudkill_png_start_;
static const unsigned char *const data_gfx_cloudkill_png_end   =
    (const unsigned char *const) (char*)&data_gfx_cloudkill_png_end_;

extern unsigned char *data_gfx_dragon_png_start_ asm("data_gfx_dragon_png_start_");
extern unsigned char *data_gfx_dragon_png_end_ asm("data_gfx_dragon_png_end_");
static const unsigned char *const data_gfx_dragon_png_start =
    (const unsigned char *const) (char*)&data_gfx_dragon_png_start_;
static const unsigned char *const data_gfx_dragon_png_end   =
    (const unsigned char *const) (char*)&data_gfx_dragon_png_end_;

extern unsigned char *data_gfx_explosion_png_start_ asm("data_gfx_explosion_png_start_");
extern unsigned char *data_gfx_explosion_png_end_ asm("data_gfx_explosion_png_end_");
static const unsigned char *const data_gfx_explosion_png_start =
    (const unsigned char *const) (char*)&data_gfx_explosion_png_start_;
static const unsigned char *const data_gfx_explosion_png_end   =
    (const unsigned char *const) (char*)&data_gfx_explosion_png_end_;

extern unsigned char *data_gfx_giant_rock_png_start_ asm("data_gfx_giant_rock_png_start_");
extern unsigned char *data_gfx_giant_rock_png_end_ asm("data_gfx_giant_rock_png_end_");
static const unsigned char *const data_gfx_giant_rock_png_start =
    (const unsigned char *const) (char*)&data_gfx_giant_rock_png_start_;
static const unsigned char *const data_gfx_giant_rock_png_end   =
    (const unsigned char *const) (char*)&data_gfx_giant_rock_png_end_;

extern unsigned char *data_gfx_gorynlich_png_start_ asm("data_gfx_gorynlich_png_start_");
extern unsigned char *data_gfx_gorynlich_png_end_ asm("data_gfx_gorynlich_png_end_");
static const unsigned char *const data_gfx_gorynlich_png_start =
    (const unsigned char *const) (char*)&data_gfx_gorynlich_png_start_;
static const unsigned char *const data_gfx_gorynlich_png_end   =
    (const unsigned char *const) (char*)&data_gfx_gorynlich_png_end_;

extern unsigned char *data_gfx_gothic_wide_png_start_ asm("data_gfx_gothic_wide_png_start_");
extern unsigned char *data_gfx_gothic_wide_png_end_ asm("data_gfx_gothic_wide_png_end_");
static const unsigned char *const data_gfx_gothic_wide_png_start =
    (const unsigned char *const) (char*)&data_gfx_gothic_wide_png_start_;
static const unsigned char *const data_gfx_gothic_wide_png_end   =
    (const unsigned char *const) (char*)&data_gfx_gothic_wide_png_end_;

extern unsigned char *data_gfx_gravestone_png_start_ asm("data_gfx_gravestone_png_start_");
extern unsigned char *data_gfx_gravestone_png_end_ asm("data_gfx_gravestone_png_end_");
static const unsigned char *const data_gfx_gravestone_png_start =
    (const unsigned char *const) (char*)&data_gfx_gravestone_png_start_;
static const unsigned char *const data_gfx_gravestone_png_end   =
    (const unsigned char *const) (char*)&data_gfx_gravestone_png_end_;

extern unsigned char *data_gfx_gravestone2_png_start_ asm("data_gfx_gravestone2_png_start_");
extern unsigned char *data_gfx_gravestone2_png_end_ asm("data_gfx_gravestone2_png_end_");
static const unsigned char *const data_gfx_gravestone2_png_start =
    (const unsigned char *const) (char*)&data_gfx_gravestone2_png_start_;
static const unsigned char *const data_gfx_gravestone2_png_end   =
    (const unsigned char *const) (char*)&data_gfx_gravestone2_png_end_;

extern unsigned char *data_gfx_help1_png_start_ asm("data_gfx_help1_png_start_");
extern unsigned char *data_gfx_help1_png_end_ asm("data_gfx_help1_png_end_");
static const unsigned char *const data_gfx_help1_png_start =
    (const unsigned char *const) (char*)&data_gfx_help1_png_start_;
static const unsigned char *const data_gfx_help1_png_end   =
    (const unsigned char *const) (char*)&data_gfx_help1_png_end_;

extern unsigned char *data_gfx_help2_png_start_ asm("data_gfx_help2_png_start_");
extern unsigned char *data_gfx_help2_png_end_ asm("data_gfx_help2_png_end_");
static const unsigned char *const data_gfx_help2_png_start =
    (const unsigned char *const) (char*)&data_gfx_help2_png_start_;
static const unsigned char *const data_gfx_help2_png_end   =
    (const unsigned char *const) (char*)&data_gfx_help2_png_end_;

extern unsigned char *data_gfx_help3_png_start_ asm("data_gfx_help3_png_start_");
extern unsigned char *data_gfx_help3_png_end_ asm("data_gfx_help3_png_end_");
static const unsigned char *const data_gfx_help3_png_start =
    (const unsigned char *const) (char*)&data_gfx_help3_png_start_;
static const unsigned char *const data_gfx_help3_png_end   =
    (const unsigned char *const) (char*)&data_gfx_help3_png_end_;

extern unsigned char *data_gfx_main_title_png_start_ asm("data_gfx_main_title_png_start_");
extern unsigned char *data_gfx_main_title_png_end_ asm("data_gfx_main_title_png_end_");
static const unsigned char *const data_gfx_main_title_png_start =
    (const unsigned char *const) (char*)&data_gfx_main_title_png_start_;
static const unsigned char *const data_gfx_main_title_png_end   =
    (const unsigned char *const) (char*)&data_gfx_main_title_png_end_;

extern unsigned char *data_gfx_poison_png_start_ asm("data_gfx_poison_png_start_");
extern unsigned char *data_gfx_poison_png_end_ asm("data_gfx_poison_png_end_");
static const unsigned char *const data_gfx_poison_png_start =
    (const unsigned char *const) (char*)&data_gfx_poison_png_start_;
static const unsigned char *const data_gfx_poison_png_end   =
    (const unsigned char *const) (char*)&data_gfx_poison_png_end_;

extern unsigned char *data_gfx_rat_png_start_ asm("data_gfx_rat_png_start_");
extern unsigned char *data_gfx_rat_png_end_ asm("data_gfx_rat_png_end_");
static const unsigned char *const data_gfx_rat_png_start =
    (const unsigned char *const) (char*)&data_gfx_rat_png_start_;
static const unsigned char *const data_gfx_rat_png_end   =
    (const unsigned char *const) (char*)&data_gfx_rat_png_end_;

extern unsigned char *data_gfx_button_red_png_start_ asm("data_gfx_button_red_png_start_");
extern unsigned char *data_gfx_button_red_png_end_ asm("data_gfx_button_red_png_end_");
static const unsigned char *const data_gfx_button_red_png_start =
    (const unsigned char *const) (char*)&data_gfx_button_red_png_start_;
static const unsigned char *const data_gfx_button_red_png_end   =
    (const unsigned char *const) (char*)&data_gfx_button_red_png_end_;

extern unsigned char *data_gfx_sprites_small_png_start_ asm("data_gfx_sprites_small_png_start_");
extern unsigned char *data_gfx_sprites_small_png_end_ asm("data_gfx_sprites_small_png_end_");
static const unsigned char *const data_gfx_sprites_small_png_start =
    (const unsigned char *const) (char*)&data_gfx_sprites_small_png_start_;
static const unsigned char *const data_gfx_sprites_small_png_end   =
    (const unsigned char *const) (char*)&data_gfx_sprites_small_png_end_;

extern unsigned char *data_gfx_title_png_start_ asm("data_gfx_title_png_start_");
extern unsigned char *data_gfx_title_png_end_ asm("data_gfx_title_png_end_");
static const unsigned char *const data_gfx_title_png_start =
    (const unsigned char *const) (char*)&data_gfx_title_png_start_;
static const unsigned char *const data_gfx_title_png_end   =
    (const unsigned char *const) (char*)&data_gfx_title_png_end_;

extern unsigned char *data_gfx_title2_png_start_ asm("data_gfx_title2_png_start_");
extern unsigned char *data_gfx_title2_png_end_ asm("data_gfx_title2_png_end_");
static const unsigned char *const data_gfx_title2_png_start =
    (const unsigned char *const) (char*)&data_gfx_title2_png_start_;
static const unsigned char *const data_gfx_title2_png_end   =
    (const unsigned char *const) (char*)&data_gfx_title2_png_end_;

extern unsigned char *data_gfx_title3_png_start_ asm("data_gfx_title3_png_start_");
extern unsigned char *data_gfx_title3_png_end_ asm("data_gfx_title3_png_end_");
static const unsigned char *const data_gfx_title3_png_start =
    (const unsigned char *const) (char*)&data_gfx_title3_png_start_;
static const unsigned char *const data_gfx_title3_png_end   =
    (const unsigned char *const) (char*)&data_gfx_title3_png_end_;

extern unsigned char *data_gfx_title_small_png_start_ asm("data_gfx_title_small_png_start_");
extern unsigned char *data_gfx_title_small_png_end_ asm("data_gfx_title_small_png_end_");
static const unsigned char *const data_gfx_title_small_png_start =
    (const unsigned char *const) (char*)&data_gfx_title_small_png_start_;
static const unsigned char *const data_gfx_title_small_png_end   =
    (const unsigned char *const) (char*)&data_gfx_title_small_png_end_;

extern unsigned char *data_gfx_wall_floor_png_start_ asm("data_gfx_wall_floor_png_start_");
extern unsigned char *data_gfx_wall_floor_png_end_ asm("data_gfx_wall_floor_png_end_");
static const unsigned char *const data_gfx_wall_floor_png_start =
    (const unsigned char *const) (char*)&data_gfx_wall_floor_png_start_;
static const unsigned char *const data_gfx_wall_floor_png_end   =
    (const unsigned char *const) (char*)&data_gfx_wall_floor_png_end_;

extern unsigned char *data_gfx_window_png_start_ asm("data_gfx_window_png_start_");
extern unsigned char *data_gfx_window_png_end_ asm("data_gfx_window_png_end_");
static const unsigned char *const data_gfx_window_png_start =
    (const unsigned char *const) (char*)&data_gfx_window_png_start_;
static const unsigned char *const data_gfx_window_png_end   =
    (const unsigned char *const) (char*)&data_gfx_window_png_end_;

extern unsigned char *data_gfx_window2_png_start_ asm("data_gfx_window2_png_start_");
extern unsigned char *data_gfx_window2_png_end_ asm("data_gfx_window2_png_end_");
static const unsigned char *const data_gfx_window2_png_start =
    (const unsigned char *const) (char*)&data_gfx_window2_png_start_;
static const unsigned char *const data_gfx_window2_png_end   =
    (const unsigned char *const) (char*)&data_gfx_window2_png_end_;

extern unsigned char *data_levels_1_start_ asm("data_levels_1_start_");
extern unsigned char *data_levels_1_end_ asm("data_levels_1_end_");
static const unsigned char *const data_levels_1_start =
    (const unsigned char *const) (char*)&data_levels_1_start_;
static const unsigned char *const data_levels_1_end   =
    (const unsigned char *const) (char*)&data_levels_1_end_;

extern unsigned char *data_levels_2_start_ asm("data_levels_2_start_");
extern unsigned char *data_levels_2_end_ asm("data_levels_2_end_");
static const unsigned char *const data_levels_2_start =
    (const unsigned char *const) (char*)&data_levels_2_start_;
static const unsigned char *const data_levels_2_end   =
    (const unsigned char *const) (char*)&data_levels_2_end_;

extern unsigned char *data_map_jigsaw_map_start_ asm("data_map_jigsaw_map_start_");
extern unsigned char *data_map_jigsaw_map_end_ asm("data_map_jigsaw_map_end_");
static const unsigned char *const data_map_jigsaw_map_start =
    (const unsigned char *const) (char*)&data_map_jigsaw_map_start_;
static const unsigned char *const data_map_jigsaw_map_end   =
    (const unsigned char *const) (char*)&data_map_jigsaw_map_end_;

extern unsigned char *data_music_charlotte_mp3_start_ asm("data_music_charlotte_mp3_start_");
extern unsigned char *data_music_charlotte_mp3_end_ asm("data_music_charlotte_mp3_end_");
static const unsigned char *const data_music_charlotte_mp3_start =
    (const unsigned char *const) (char*)&data_music_charlotte_mp3_start_;
static const unsigned char *const data_music_charlotte_mp3_end   =
    (const unsigned char *const) (char*)&data_music_charlotte_mp3_end_;

extern unsigned char *data_music_charlotte8bit_mp3_start_ asm("data_music_charlotte8bit_mp3_start_");
extern unsigned char *data_music_charlotte8bit_mp3_end_ asm("data_music_charlotte8bit_mp3_end_");
static const unsigned char *const data_music_charlotte8bit_mp3_start =
    (const unsigned char *const) (char*)&data_music_charlotte8bit_mp3_start_;
static const unsigned char *const data_music_charlotte8bit_mp3_end   =
    (const unsigned char *const) (char*)&data_music_charlotte8bit_mp3_end_;

extern unsigned char *data_music_hiscore_wav_start_ asm("data_music_hiscore_wav_start_");
extern unsigned char *data_music_hiscore_wav_end_ asm("data_music_hiscore_wav_end_");
static const unsigned char *const data_music_hiscore_wav_start =
    (const unsigned char *const) (char*)&data_music_hiscore_wav_start_;
static const unsigned char *const data_music_hiscore_wav_end   =
    (const unsigned char *const) (char*)&data_music_hiscore_wav_end_;

extern unsigned char *data_music_intro_wav_start_ asm("data_music_intro_wav_start_");
extern unsigned char *data_music_intro_wav_end_ asm("data_music_intro_wav_end_");
static const unsigned char *const data_music_intro_wav_start =
    (const unsigned char *const) (char*)&data_music_intro_wav_start_;
static const unsigned char *const data_music_intro_wav_end   =
    (const unsigned char *const) (char*)&data_music_intro_wav_end_;

extern unsigned char *data_music_music_wav_start_ asm("data_music_music_wav_start_");
extern unsigned char *data_music_music_wav_end_ asm("data_music_music_wav_end_");
static const unsigned char *const data_music_music_wav_start =
    (const unsigned char *const) (char*)&data_music_music_wav_start_;
static const unsigned char *const data_music_music_wav_end   =
    (const unsigned char *const) (char*)&data_music_music_wav_end_;

extern unsigned char *data_sound_chomp_wav_start_ asm("data_sound_chomp_wav_start_");
extern unsigned char *data_sound_chomp_wav_end_ asm("data_sound_chomp_wav_end_");
static const unsigned char *const data_sound_chomp_wav_start =
    (const unsigned char *const) (char*)&data_sound_chomp_wav_start_;
static const unsigned char *const data_sound_chomp_wav_end   =
    (const unsigned char *const) (char*)&data_sound_chomp_wav_end_;

extern unsigned char *data_sound_chomp2_wav_start_ asm("data_sound_chomp2_wav_start_");
extern unsigned char *data_sound_chomp2_wav_end_ asm("data_sound_chomp2_wav_end_");
static const unsigned char *const data_sound_chomp2_wav_start =
    (const unsigned char *const) (char*)&data_sound_chomp2_wav_start_;
static const unsigned char *const data_sound_chomp2_wav_end   =
    (const unsigned char *const) (char*)&data_sound_chomp2_wav_end_;

extern unsigned char *data_sound_click_wav_start_ asm("data_sound_click_wav_start_");
extern unsigned char *data_sound_click_wav_end_ asm("data_sound_click_wav_end_");
static const unsigned char *const data_sound_click_wav_start =
    (const unsigned char *const) (char*)&data_sound_click_wav_start_;
static const unsigned char *const data_sound_click_wav_end   =
    (const unsigned char *const) (char*)&data_sound_click_wav_end_;

extern unsigned char *data_sound_doorbell_wav_start_ asm("data_sound_doorbell_wav_start_");
extern unsigned char *data_sound_doorbell_wav_end_ asm("data_sound_doorbell_wav_end_");
static const unsigned char *const data_sound_doorbell_wav_start =
    (const unsigned char *const) (char*)&data_sound_doorbell_wav_start_;
static const unsigned char *const data_sound_doorbell_wav_end   =
    (const unsigned char *const) (char*)&data_sound_doorbell_wav_end_;

extern unsigned char *data_sound_explosion_wav_start_ asm("data_sound_explosion_wav_start_");
extern unsigned char *data_sound_explosion_wav_end_ asm("data_sound_explosion_wav_end_");
static const unsigned char *const data_sound_explosion_wav_start =
    (const unsigned char *const) (char*)&data_sound_explosion_wav_start_;
static const unsigned char *const data_sound_explosion_wav_end   =
    (const unsigned char *const) (char*)&data_sound_explosion_wav_end_;

extern unsigned char *data_sound_letter_wav_start_ asm("data_sound_letter_wav_start_");
extern unsigned char *data_sound_letter_wav_end_ asm("data_sound_letter_wav_end_");
static const unsigned char *const data_sound_letter_wav_start =
    (const unsigned char *const) (char*)&data_sound_letter_wav_start_;
static const unsigned char *const data_sound_letter_wav_end   =
    (const unsigned char *const) (char*)&data_sound_letter_wav_end_;

extern unsigned char *data_sound_level_end_wav_start_ asm("data_sound_level_end_wav_start_");
extern unsigned char *data_sound_level_end_wav_end_ asm("data_sound_level_end_wav_end_");
static const unsigned char *const data_sound_level_end_wav_start =
    (const unsigned char *const) (char*)&data_sound_level_end_wav_start_;
static const unsigned char *const data_sound_level_end_wav_end   =
    (const unsigned char *const) (char*)&data_sound_level_end_wav_end_;

extern unsigned char *data_sound_paper_wav_start_ asm("data_sound_paper_wav_start_");
extern unsigned char *data_sound_paper_wav_end_ asm("data_sound_paper_wav_end_");
static const unsigned char *const data_sound_paper_wav_start =
    (const unsigned char *const) (char*)&data_sound_paper_wav_start_;
static const unsigned char *const data_sound_paper_wav_end   =
    (const unsigned char *const) (char*)&data_sound_paper_wav_end_;

extern unsigned char *data_sound_powerup_wav_start_ asm("data_sound_powerup_wav_start_");
extern unsigned char *data_sound_powerup_wav_end_ asm("data_sound_powerup_wav_end_");
static const unsigned char *const data_sound_powerup_wav_start =
    (const unsigned char *const) (char*)&data_sound_powerup_wav_start_;
static const unsigned char *const data_sound_powerup_wav_end   =
    (const unsigned char *const) (char*)&data_sound_powerup_wav_end_;

extern unsigned char *data_sound_ratdeath_wav_start_ asm("data_sound_ratdeath_wav_start_");
extern unsigned char *data_sound_ratdeath_wav_end_ asm("data_sound_ratdeath_wav_end_");
static const unsigned char *const data_sound_ratdeath_wav_start =
    (const unsigned char *const) (char*)&data_sound_ratdeath_wav_start_;
static const unsigned char *const data_sound_ratdeath_wav_end   =
    (const unsigned char *const) (char*)&data_sound_ratdeath_wav_end_;

extern unsigned char *data_sound_rocket_wav_start_ asm("data_sound_rocket_wav_start_");
extern unsigned char *data_sound_rocket_wav_end_ asm("data_sound_rocket_wav_end_");
static const unsigned char *const data_sound_rocket_wav_start =
    (const unsigned char *const) (char*)&data_sound_rocket_wav_start_;
static const unsigned char *const data_sound_rocket_wav_end   =
    (const unsigned char *const) (char*)&data_sound_rocket_wav_end_;

extern unsigned char *data_sound_slime_wav_start_ asm("data_sound_slime_wav_start_");
extern unsigned char *data_sound_slime_wav_end_ asm("data_sound_slime_wav_end_");
static const unsigned char *const data_sound_slime_wav_start =
    (const unsigned char *const) (char*)&data_sound_slime_wav_start_;
static const unsigned char *const data_sound_slime_wav_end   =
    (const unsigned char *const) (char*)&data_sound_slime_wav_end_;

extern unsigned char *data_sound_spam_wav_start_ asm("data_sound_spam_wav_start_");
extern unsigned char *data_sound_spam_wav_end_ asm("data_sound_spam_wav_end_");
static const unsigned char *const data_sound_spam_wav_start =
    (const unsigned char *const) (char*)&data_sound_spam_wav_start_;
static const unsigned char *const data_sound_spam_wav_end   =
    (const unsigned char *const) (char*)&data_sound_spam_wav_end_;

extern unsigned char *data_sound_thief_wav_start_ asm("data_sound_thief_wav_start_");
extern unsigned char *data_sound_thief_wav_end_ asm("data_sound_thief_wav_end_");
static const unsigned char *const data_sound_thief_wav_start =
    (const unsigned char *const) (char*)&data_sound_thief_wav_start_;
static const unsigned char *const data_sound_thief_wav_end   =
    (const unsigned char *const) (char*)&data_sound_thief_wav_end_;

extern unsigned char *data_things__anim_eyes_session_start_ asm("data_things__anim_eyes_session_start_");
extern unsigned char *data_things__anim_eyes_session_end_ asm("data_things__anim_eyes_session_end_");
static const unsigned char *const data_things__anim_eyes_session_start =
    (const unsigned char *const) (char*)&data_things__anim_eyes_session_start_;
static const unsigned char *const data_things__anim_eyes_session_end   =
    (const unsigned char *const) (char*)&data_things__anim_eyes_session_end_;

extern unsigned char *data_things__anim_man_session_start_ asm("data_things__anim_man_session_start_");
extern unsigned char *data_things__anim_man_session_end_ asm("data_things__anim_man_session_end_");
static const unsigned char *const data_things__anim_man_session_start =
    (const unsigned char *const) (char*)&data_things__anim_man_session_start_;
static const unsigned char *const data_things__anim_man_session_end   =
    (const unsigned char *const) (char*)&data_things__anim_man_session_end_;

extern unsigned char *data_things__anim_treasure_chest_session_start_ asm("data_things__anim_treasure_chest_session_start_");
extern unsigned char *data_things__anim_treasure_chest_session_end_ asm("data_things__anim_treasure_chest_session_end_");
static const unsigned char *const data_things__anim_treasure_chest_session_start =
    (const unsigned char *const) (char*)&data_things__anim_treasure_chest_session_start_;
static const unsigned char *const data_things__anim_treasure_chest_session_end   =
    (const unsigned char *const) (char*)&data_things__anim_treasure_chest_session_end_;

extern unsigned char *data_things__blood1_session_start_ asm("data_things__blood1_session_start_");
extern unsigned char *data_things__blood1_session_end_ asm("data_things__blood1_session_end_");
static const unsigned char *const data_things__blood1_session_start =
    (const unsigned char *const) (char*)&data_things__blood1_session_start_;
static const unsigned char *const data_things__blood1_session_end   =
    (const unsigned char *const) (char*)&data_things__blood1_session_end_;

extern unsigned char *data_things__blood2_session_start_ asm("data_things__blood2_session_start_");
extern unsigned char *data_things__blood2_session_end_ asm("data_things__blood2_session_end_");
static const unsigned char *const data_things__blood2_session_start =
    (const unsigned char *const) (char*)&data_things__blood2_session_start_;
static const unsigned char *const data_things__blood2_session_end   =
    (const unsigned char *const) (char*)&data_things__blood2_session_end_;

extern unsigned char *data_things__brazier_session_start_ asm("data_things__brazier_session_start_");
extern unsigned char *data_things__brazier_session_end_ asm("data_things__brazier_session_end_");
static const unsigned char *const data_things__brazier_session_start =
    (const unsigned char *const) (char*)&data_things__brazier_session_start_;
static const unsigned char *const data_things__brazier_session_end   =
    (const unsigned char *const) (char*)&data_things__brazier_session_end_;

extern unsigned char *data_things__death_session_start_ asm("data_things__death_session_start_");
extern unsigned char *data_things__death_session_end_ asm("data_things__death_session_end_");
static const unsigned char *const data_things__death_session_start =
    (const unsigned char *const) (char*)&data_things__death_session_start_;
static const unsigned char *const data_things__death_session_end   =
    (const unsigned char *const) (char*)&data_things__death_session_end_;

extern unsigned char *data_things__door1_session_start_ asm("data_things__door1_session_start_");
extern unsigned char *data_things__door1_session_end_ asm("data_things__door1_session_end_");
static const unsigned char *const data_things__door1_session_start =
    (const unsigned char *const) (char*)&data_things__door1_session_start_;
static const unsigned char *const data_things__door1_session_end   =
    (const unsigned char *const) (char*)&data_things__door1_session_end_;

extern unsigned char *data_things__elf_session_start_ asm("data_things__elf_session_start_");
extern unsigned char *data_things__elf_session_end_ asm("data_things__elf_session_end_");
static const unsigned char *const data_things__elf_session_start =
    (const unsigned char *const) (char*)&data_things__elf_session_start_;
static const unsigned char *const data_things__elf_session_end   =
    (const unsigned char *const) (char*)&data_things__elf_session_end_;

extern unsigned char *data_things__exit7_session_start_ asm("data_things__exit7_session_start_");
extern unsigned char *data_things__exit7_session_end_ asm("data_things__exit7_session_end_");
static const unsigned char *const data_things__exit7_session_start =
    (const unsigned char *const) (char*)&data_things__exit7_session_start_;
static const unsigned char *const data_things__exit7_session_end   =
    (const unsigned char *const) (char*)&data_things__exit7_session_end_;

extern unsigned char *data_things__floor1_session_start_ asm("data_things__floor1_session_start_");
extern unsigned char *data_things__floor1_session_end_ asm("data_things__floor1_session_end_");
static const unsigned char *const data_things__floor1_session_start =
    (const unsigned char *const) (char*)&data_things__floor1_session_start_;
static const unsigned char *const data_things__floor1_session_end   =
    (const unsigned char *const) (char*)&data_things__floor1_session_end_;

extern unsigned char *data_things__key_session_start_ asm("data_things__key_session_start_");
extern unsigned char *data_things__key_session_end_ asm("data_things__key_session_end_");
static const unsigned char *const data_things__key_session_start =
    (const unsigned char *const) (char*)&data_things__key_session_start_;
static const unsigned char *const data_things__key_session_end   =
    (const unsigned char *const) (char*)&data_things__key_session_end_;

extern unsigned char *data_things__keys_session_start_ asm("data_things__keys_session_start_");
extern unsigned char *data_things__keys_session_end_ asm("data_things__keys_session_end_");
static const unsigned char *const data_things__keys_session_start =
    (const unsigned char *const) (char*)&data_things__keys_session_start_;
static const unsigned char *const data_things__keys_session_end   =
    (const unsigned char *const) (char*)&data_things__keys_session_end_;

extern unsigned char *data_things__plant_session_start_ asm("data_things__plant_session_start_");
extern unsigned char *data_things__plant_session_end_ asm("data_things__plant_session_end_");
static const unsigned char *const data_things__plant_session_start =
    (const unsigned char *const) (char*)&data_things__plant_session_start_;
static const unsigned char *const data_things__plant_session_end   =
    (const unsigned char *const) (char*)&data_things__plant_session_end_;

extern unsigned char *data_things__spam_session_start_ asm("data_things__spam_session_start_");
extern unsigned char *data_things__spam_session_end_ asm("data_things__spam_session_end_");
static const unsigned char *const data_things__spam_session_start =
    (const unsigned char *const) (char*)&data_things__spam_session_start_;
static const unsigned char *const data_things__spam_session_end   =
    (const unsigned char *const) (char*)&data_things__spam_session_end_;

extern unsigned char *data_things__sparks1_session_start_ asm("data_things__sparks1_session_start_");
extern unsigned char *data_things__sparks1_session_end_ asm("data_things__sparks1_session_end_");
static const unsigned char *const data_things__sparks1_session_start =
    (const unsigned char *const) (char*)&data_things__sparks1_session_start_;
static const unsigned char *const data_things__sparks1_session_end   =
    (const unsigned char *const) (char*)&data_things__sparks1_session_end_;

extern unsigned char *data_things__sparks2_session_start_ asm("data_things__sparks2_session_start_");
extern unsigned char *data_things__sparks2_session_end_ asm("data_things__sparks2_session_end_");
static const unsigned char *const data_things__sparks2_session_start =
    (const unsigned char *const) (char*)&data_things__sparks2_session_start_;
static const unsigned char *const data_things__sparks2_session_end   =
    (const unsigned char *const) (char*)&data_things__sparks2_session_end_;

extern unsigned char *data_things__torch_session_start_ asm("data_things__torch_session_start_");
extern unsigned char *data_things__torch_session_end_ asm("data_things__torch_session_end_");
static const unsigned char *const data_things__torch_session_start =
    (const unsigned char *const) (char*)&data_things__torch_session_start_;
static const unsigned char *const data_things__torch_session_end   =
    (const unsigned char *const) (char*)&data_things__torch_session_end_;

extern unsigned char *data_things__torches_session_start_ asm("data_things__torches_session_start_");
extern unsigned char *data_things__torches_session_end_ asm("data_things__torches_session_end_");
static const unsigned char *const data_things__torches_session_start =
    (const unsigned char *const) (char*)&data_things__torches_session_start_;
static const unsigned char *const data_things__torches_session_end   =
    (const unsigned char *const) (char*)&data_things__torches_session_end_;

extern unsigned char *data_things__wand_f_session_start_ asm("data_things__wand_f_session_start_");
extern unsigned char *data_things__wand_f_session_end_ asm("data_things__wand_f_session_end_");
static const unsigned char *const data_things__wand_f_session_start =
    (const unsigned char *const) (char*)&data_things__wand_f_session_start_;
static const unsigned char *const data_things__wand_f_session_end   =
    (const unsigned char *const) (char*)&data_things__wand_f_session_end_;

extern unsigned char *data_things__wand_fire_session_start_ asm("data_things__wand_fire_session_start_");
extern unsigned char *data_things__wand_fire_session_end_ asm("data_things__wand_fire_session_end_");
static const unsigned char *const data_things__wand_fire_session_start =
    (const unsigned char *const) (char*)&data_things__wand_fire_session_start_;
static const unsigned char *const data_things__wand_fire_session_end   =
    (const unsigned char *const) (char*)&data_things__wand_fire_session_end_;

extern unsigned char *data_things__warrior_session_start_ asm("data_things__warrior_session_start_");
extern unsigned char *data_things__warrior_session_end_ asm("data_things__warrior_session_end_");
static const unsigned char *const data_things__warrior_session_start =
    (const unsigned char *const) (char*)&data_things__warrior_session_start_;
static const unsigned char *const data_things__warrior_session_end   =
    (const unsigned char *const) (char*)&data_things__warrior_session_end_;

extern unsigned char *data_things__wizard_session_start_ asm("data_things__wizard_session_start_");
extern unsigned char *data_things__wizard_session_end_ asm("data_things__wizard_session_end_");
static const unsigned char *const data_things__wizard_session_start =
    (const unsigned char *const) (char*)&data_things__wizard_session_start_;
static const unsigned char *const data_things__wizard_session_end   =
    (const unsigned char *const) (char*)&data_things__wizard_session_end_;

extern unsigned char *data_things_amulet1_start_ asm("data_things_amulet1_start_");
extern unsigned char *data_things_amulet1_end_ asm("data_things_amulet1_end_");
static const unsigned char *const data_things_amulet1_start =
    (const unsigned char *const) (char*)&data_things_amulet1_start_;
static const unsigned char *const data_things_amulet1_end   =
    (const unsigned char *const) (char*)&data_things_amulet1_end_;

extern unsigned char *data_things_anim_eyes_start_ asm("data_things_anim_eyes_start_");
extern unsigned char *data_things_anim_eyes_end_ asm("data_things_anim_eyes_end_");
static const unsigned char *const data_things_anim_eyes_start =
    (const unsigned char *const) (char*)&data_things_anim_eyes_start_;
static const unsigned char *const data_things_anim_eyes_end   =
    (const unsigned char *const) (char*)&data_things_anim_eyes_end_;

extern unsigned char *data_things_anim_man_start_ asm("data_things_anim_man_start_");
extern unsigned char *data_things_anim_man_end_ asm("data_things_anim_man_end_");
static const unsigned char *const data_things_anim_man_start =
    (const unsigned char *const) (char*)&data_things_anim_man_start_;
static const unsigned char *const data_things_anim_man_end   =
    (const unsigned char *const) (char*)&data_things_anim_man_end_;

extern unsigned char *data_things_anim_treasure_chest_start_ asm("data_things_anim_treasure_chest_start_");
extern unsigned char *data_things_anim_treasure_chest_end_ asm("data_things_anim_treasure_chest_end_");
static const unsigned char *const data_things_anim_treasure_chest_start =
    (const unsigned char *const) (char*)&data_things_anim_treasure_chest_start_;
static const unsigned char *const data_things_anim_treasure_chest_end   =
    (const unsigned char *const) (char*)&data_things_anim_treasure_chest_end_;

extern unsigned char *data_things_apple_start_ asm("data_things_apple_start_");
extern unsigned char *data_things_apple_end_ asm("data_things_apple_end_");
static const unsigned char *const data_things_apple_start =
    (const unsigned char *const) (char*)&data_things_apple_start_;
static const unsigned char *const data_things_apple_end   =
    (const unsigned char *const) (char*)&data_things_apple_end_;

extern unsigned char *data_things_apple_poison_start_ asm("data_things_apple_poison_start_");
extern unsigned char *data_things_apple_poison_end_ asm("data_things_apple_poison_end_");
static const unsigned char *const data_things_apple_poison_start =
    (const unsigned char *const) (char*)&data_things_apple_poison_start_;
static const unsigned char *const data_things_apple_poison_end   =
    (const unsigned char *const) (char*)&data_things_apple_poison_end_;

extern unsigned char *data_things_arrow_start_ asm("data_things_arrow_start_");
extern unsigned char *data_things_arrow_end_ asm("data_things_arrow_end_");
static const unsigned char *const data_things_arrow_start =
    (const unsigned char *const) (char*)&data_things_arrow_start_;
static const unsigned char *const data_things_arrow_end   =
    (const unsigned char *const) (char*)&data_things_arrow_end_;

extern unsigned char *data_things_axe1_start_ asm("data_things_axe1_start_");
extern unsigned char *data_things_axe1_end_ asm("data_things_axe1_end_");
static const unsigned char *const data_things_axe1_start =
    (const unsigned char *const) (char*)&data_things_axe1_start_;
static const unsigned char *const data_things_axe1_end   =
    (const unsigned char *const) (char*)&data_things_axe1_end_;

extern unsigned char *data_things_axe1_anim_start_ asm("data_things_axe1_anim_start_");
extern unsigned char *data_things_axe1_anim_end_ asm("data_things_axe1_anim_end_");
static const unsigned char *const data_things_axe1_anim_start =
    (const unsigned char *const) (char*)&data_things_axe1_anim_start_;
static const unsigned char *const data_things_axe1_anim_end   =
    (const unsigned char *const) (char*)&data_things_axe1_anim_end_;

extern unsigned char *data_things_axe2_start_ asm("data_things_axe2_start_");
extern unsigned char *data_things_axe2_end_ asm("data_things_axe2_end_");
static const unsigned char *const data_things_axe2_start =
    (const unsigned char *const) (char*)&data_things_axe2_start_;
static const unsigned char *const data_things_axe2_end   =
    (const unsigned char *const) (char*)&data_things_axe2_end_;

extern unsigned char *data_things_axe2_anim_start_ asm("data_things_axe2_anim_start_");
extern unsigned char *data_things_axe2_anim_end_ asm("data_things_axe2_anim_end_");
static const unsigned char *const data_things_axe2_anim_start =
    (const unsigned char *const) (char*)&data_things_axe2_anim_start_;
static const unsigned char *const data_things_axe2_anim_end   =
    (const unsigned char *const) (char*)&data_things_axe2_anim_end_;

extern unsigned char *data_things_banana_start_ asm("data_things_banana_start_");
extern unsigned char *data_things_banana_end_ asm("data_things_banana_end_");
static const unsigned char *const data_things_banana_start =
    (const unsigned char *const) (char*)&data_things_banana_start_;
static const unsigned char *const data_things_banana_end   =
    (const unsigned char *const) (char*)&data_things_banana_end_;

extern unsigned char *data_things_blood1_start_ asm("data_things_blood1_start_");
extern unsigned char *data_things_blood1_end_ asm("data_things_blood1_end_");
static const unsigned char *const data_things_blood1_start =
    (const unsigned char *const) (char*)&data_things_blood1_start_;
static const unsigned char *const data_things_blood1_end   =
    (const unsigned char *const) (char*)&data_things_blood1_end_;

extern unsigned char *data_things_blood2_start_ asm("data_things_blood2_start_");
extern unsigned char *data_things_blood2_end_ asm("data_things_blood2_end_");
static const unsigned char *const data_things_blood2_start =
    (const unsigned char *const) (char*)&data_things_blood2_start_;
static const unsigned char *const data_things_blood2_end   =
    (const unsigned char *const) (char*)&data_things_blood2_end_;

extern unsigned char *data_things_bonepile1_start_ asm("data_things_bonepile1_start_");
extern unsigned char *data_things_bonepile1_end_ asm("data_things_bonepile1_end_");
static const unsigned char *const data_things_bonepile1_start =
    (const unsigned char *const) (char*)&data_things_bonepile1_start_;
static const unsigned char *const data_things_bonepile1_end   =
    (const unsigned char *const) (char*)&data_things_bonepile1_end_;

extern unsigned char *data_things_bonepile2_start_ asm("data_things_bonepile2_start_");
extern unsigned char *data_things_bonepile2_end_ asm("data_things_bonepile2_end_");
static const unsigned char *const data_things_bonepile2_start =
    (const unsigned char *const) (char*)&data_things_bonepile2_start_;
static const unsigned char *const data_things_bonepile2_end   =
    (const unsigned char *const) (char*)&data_things_bonepile2_end_;

extern unsigned char *data_things_bonepile3_start_ asm("data_things_bonepile3_start_");
extern unsigned char *data_things_bonepile3_end_ asm("data_things_bonepile3_end_");
static const unsigned char *const data_things_bonepile3_start =
    (const unsigned char *const) (char*)&data_things_bonepile3_start_;
static const unsigned char *const data_things_bonepile3_end   =
    (const unsigned char *const) (char*)&data_things_bonepile3_end_;

extern unsigned char *data_things_bow1_start_ asm("data_things_bow1_start_");
extern unsigned char *data_things_bow1_end_ asm("data_things_bow1_end_");
static const unsigned char *const data_things_bow1_start =
    (const unsigned char *const) (char*)&data_things_bow1_start_;
static const unsigned char *const data_things_bow1_end   =
    (const unsigned char *const) (char*)&data_things_bow1_end_;

extern unsigned char *data_things_bow1_anim_start_ asm("data_things_bow1_anim_start_");
extern unsigned char *data_things_bow1_anim_end_ asm("data_things_bow1_anim_end_");
static const unsigned char *const data_things_bow1_anim_start =
    (const unsigned char *const) (char*)&data_things_bow1_anim_start_;
static const unsigned char *const data_things_bow1_anim_end   =
    (const unsigned char *const) (char*)&data_things_bow1_anim_end_;

extern unsigned char *data_things_bow2_start_ asm("data_things_bow2_start_");
extern unsigned char *data_things_bow2_end_ asm("data_things_bow2_end_");
static const unsigned char *const data_things_bow2_start =
    (const unsigned char *const) (char*)&data_things_bow2_start_;
static const unsigned char *const data_things_bow2_end   =
    (const unsigned char *const) (char*)&data_things_bow2_end_;

extern unsigned char *data_things_bow3_start_ asm("data_things_bow3_start_");
extern unsigned char *data_things_bow3_end_ asm("data_things_bow3_end_");
static const unsigned char *const data_things_bow3_start =
    (const unsigned char *const) (char*)&data_things_bow3_start_;
static const unsigned char *const data_things_bow3_end   =
    (const unsigned char *const) (char*)&data_things_bow3_end_;

extern unsigned char *data_things_bow4_start_ asm("data_things_bow4_start_");
extern unsigned char *data_things_bow4_end_ asm("data_things_bow4_end_");
static const unsigned char *const data_things_bow4_start =
    (const unsigned char *const) (char*)&data_things_bow4_start_;
static const unsigned char *const data_things_bow4_end   =
    (const unsigned char *const) (char*)&data_things_bow4_end_;

extern unsigned char *data_things_brazier_start_ asm("data_things_brazier_start_");
extern unsigned char *data_things_brazier_end_ asm("data_things_brazier_end_");
static const unsigned char *const data_things_brazier_start =
    (const unsigned char *const) (char*)&data_things_brazier_start_;
static const unsigned char *const data_things_brazier_end   =
    (const unsigned char *const) (char*)&data_things_brazier_end_;

extern unsigned char *data_things_bread_start_ asm("data_things_bread_start_");
extern unsigned char *data_things_bread_end_ asm("data_things_bread_end_");
static const unsigned char *const data_things_bread_start =
    (const unsigned char *const) (char*)&data_things_bread_start_;
static const unsigned char *const data_things_bread_end   =
    (const unsigned char *const) (char*)&data_things_bread_end_;

extern unsigned char *data_things_cheese_start_ asm("data_things_cheese_start_");
extern unsigned char *data_things_cheese_end_ asm("data_things_cheese_end_");
static const unsigned char *const data_things_cheese_start =
    (const unsigned char *const) (char*)&data_things_cheese_start_;
static const unsigned char *const data_things_cheese_end   =
    (const unsigned char *const) (char*)&data_things_cheese_end_;

extern unsigned char *data_things_chest1_start_ asm("data_things_chest1_start_");
extern unsigned char *data_things_chest1_end_ asm("data_things_chest1_end_");
static const unsigned char *const data_things_chest1_start =
    (const unsigned char *const) (char*)&data_things_chest1_start_;
static const unsigned char *const data_things_chest1_end   =
    (const unsigned char *const) (char*)&data_things_chest1_end_;

extern unsigned char *data_things_cloudkill1_start_ asm("data_things_cloudkill1_start_");
extern unsigned char *data_things_cloudkill1_end_ asm("data_things_cloudkill1_end_");
static const unsigned char *const data_things_cloudkill1_start =
    (const unsigned char *const) (char*)&data_things_cloudkill1_start_;
static const unsigned char *const data_things_cloudkill1_end   =
    (const unsigned char *const) (char*)&data_things_cloudkill1_end_;

extern unsigned char *data_things_cloudkill2_start_ asm("data_things_cloudkill2_start_");
extern unsigned char *data_things_cloudkill2_end_ asm("data_things_cloudkill2_end_");
static const unsigned char *const data_things_cloudkill2_start =
    (const unsigned char *const) (char*)&data_things_cloudkill2_start_;
static const unsigned char *const data_things_cloudkill2_end   =
    (const unsigned char *const) (char*)&data_things_cloudkill2_end_;

extern unsigned char *data_things_cloudkill3_start_ asm("data_things_cloudkill3_start_");
extern unsigned char *data_things_cloudkill3_end_ asm("data_things_cloudkill3_end_");
static const unsigned char *const data_things_cloudkill3_start =
    (const unsigned char *const) (char*)&data_things_cloudkill3_start_;
static const unsigned char *const data_things_cloudkill3_end   =
    (const unsigned char *const) (char*)&data_things_cloudkill3_end_;

extern unsigned char *data_things_cloudkill4_start_ asm("data_things_cloudkill4_start_");
extern unsigned char *data_things_cloudkill4_end_ asm("data_things_cloudkill4_end_");
static const unsigned char *const data_things_cloudkill4_start =
    (const unsigned char *const) (char*)&data_things_cloudkill4_start_;
static const unsigned char *const data_things_cloudkill4_end   =
    (const unsigned char *const) (char*)&data_things_cloudkill4_end_;

extern unsigned char *data_things_coins1_start_ asm("data_things_coins1_start_");
extern unsigned char *data_things_coins1_end_ asm("data_things_coins1_end_");
static const unsigned char *const data_things_coins1_start =
    (const unsigned char *const) (char*)&data_things_coins1_start_;
static const unsigned char *const data_things_coins1_end   =
    (const unsigned char *const) (char*)&data_things_coins1_end_;

extern unsigned char *data_things_death_start_ asm("data_things_death_start_");
extern unsigned char *data_things_death_end_ asm("data_things_death_end_");
static const unsigned char *const data_things_death_start =
    (const unsigned char *const) (char*)&data_things_death_start_;
static const unsigned char *const data_things_death_end   =
    (const unsigned char *const) (char*)&data_things_death_end_;

extern unsigned char *data_things_door1_start_ asm("data_things_door1_start_");
extern unsigned char *data_things_door1_end_ asm("data_things_door1_end_");
static const unsigned char *const data_things_door1_start =
    (const unsigned char *const) (char*)&data_things_door1_start_;
static const unsigned char *const data_things_door1_end   =
    (const unsigned char *const) (char*)&data_things_door1_end_;

extern unsigned char *data_things_door2_start_ asm("data_things_door2_start_");
extern unsigned char *data_things_door2_end_ asm("data_things_door2_end_");
static const unsigned char *const data_things_door2_start =
    (const unsigned char *const) (char*)&data_things_door2_start_;
static const unsigned char *const data_things_door2_end   =
    (const unsigned char *const) (char*)&data_things_door2_end_;

extern unsigned char *data_things_dwarf_start_ asm("data_things_dwarf_start_");
extern unsigned char *data_things_dwarf_end_ asm("data_things_dwarf_end_");
static const unsigned char *const data_things_dwarf_start =
    (const unsigned char *const) (char*)&data_things_dwarf_start_;
static const unsigned char *const data_things_dwarf_end   =
    (const unsigned char *const) (char*)&data_things_dwarf_end_;

extern unsigned char *data_things_elf_start_ asm("data_things_elf_start_");
extern unsigned char *data_things_elf_end_ asm("data_things_elf_end_");
static const unsigned char *const data_things_elf_start =
    (const unsigned char *const) (char*)&data_things_elf_start_;
static const unsigned char *const data_things_elf_end   =
    (const unsigned char *const) (char*)&data_things_elf_end_;

extern unsigned char *data_things_exit1_start_ asm("data_things_exit1_start_");
extern unsigned char *data_things_exit1_end_ asm("data_things_exit1_end_");
static const unsigned char *const data_things_exit1_start =
    (const unsigned char *const) (char*)&data_things_exit1_start_;
static const unsigned char *const data_things_exit1_end   =
    (const unsigned char *const) (char*)&data_things_exit1_end_;

extern unsigned char *data_things_exit2_start_ asm("data_things_exit2_start_");
extern unsigned char *data_things_exit2_end_ asm("data_things_exit2_end_");
static const unsigned char *const data_things_exit2_start =
    (const unsigned char *const) (char*)&data_things_exit2_start_;
static const unsigned char *const data_things_exit2_end   =
    (const unsigned char *const) (char*)&data_things_exit2_end_;

extern unsigned char *data_things_exit3_start_ asm("data_things_exit3_start_");
extern unsigned char *data_things_exit3_end_ asm("data_things_exit3_end_");
static const unsigned char *const data_things_exit3_start =
    (const unsigned char *const) (char*)&data_things_exit3_start_;
static const unsigned char *const data_things_exit3_end   =
    (const unsigned char *const) (char*)&data_things_exit3_end_;

extern unsigned char *data_things_exit4_start_ asm("data_things_exit4_start_");
extern unsigned char *data_things_exit4_end_ asm("data_things_exit4_end_");
static const unsigned char *const data_things_exit4_start =
    (const unsigned char *const) (char*)&data_things_exit4_start_;
static const unsigned char *const data_things_exit4_end   =
    (const unsigned char *const) (char*)&data_things_exit4_end_;

extern unsigned char *data_things_exit5_start_ asm("data_things_exit5_start_");
extern unsigned char *data_things_exit5_end_ asm("data_things_exit5_end_");
static const unsigned char *const data_things_exit5_start =
    (const unsigned char *const) (char*)&data_things_exit5_start_;
static const unsigned char *const data_things_exit5_end   =
    (const unsigned char *const) (char*)&data_things_exit5_end_;

extern unsigned char *data_things_exit6_start_ asm("data_things_exit6_start_");
extern unsigned char *data_things_exit6_end_ asm("data_things_exit6_end_");
static const unsigned char *const data_things_exit6_start =
    (const unsigned char *const) (char*)&data_things_exit6_start_;
static const unsigned char *const data_things_exit6_end   =
    (const unsigned char *const) (char*)&data_things_exit6_end_;

extern unsigned char *data_things_exit7_start_ asm("data_things_exit7_start_");
extern unsigned char *data_things_exit7_end_ asm("data_things_exit7_end_");
static const unsigned char *const data_things_exit7_start =
    (const unsigned char *const) (char*)&data_things_exit7_start_;
static const unsigned char *const data_things_exit7_end   =
    (const unsigned char *const) (char*)&data_things_exit7_end_;

extern unsigned char *data_things_explosion1_start_ asm("data_things_explosion1_start_");
extern unsigned char *data_things_explosion1_end_ asm("data_things_explosion1_end_");
static const unsigned char *const data_things_explosion1_start =
    (const unsigned char *const) (char*)&data_things_explosion1_start_;
static const unsigned char *const data_things_explosion1_end   =
    (const unsigned char *const) (char*)&data_things_explosion1_end_;

extern unsigned char *data_things_explosion2_start_ asm("data_things_explosion2_start_");
extern unsigned char *data_things_explosion2_end_ asm("data_things_explosion2_end_");
static const unsigned char *const data_things_explosion2_start =
    (const unsigned char *const) (char*)&data_things_explosion2_start_;
static const unsigned char *const data_things_explosion2_end   =
    (const unsigned char *const) (char*)&data_things_explosion2_end_;

extern unsigned char *data_things_explosion3_start_ asm("data_things_explosion3_start_");
extern unsigned char *data_things_explosion3_end_ asm("data_things_explosion3_end_");
static const unsigned char *const data_things_explosion3_start =
    (const unsigned char *const) (char*)&data_things_explosion3_start_;
static const unsigned char *const data_things_explosion3_end   =
    (const unsigned char *const) (char*)&data_things_explosion3_end_;

extern unsigned char *data_things_explosion4_start_ asm("data_things_explosion4_start_");
extern unsigned char *data_things_explosion4_end_ asm("data_things_explosion4_end_");
static const unsigned char *const data_things_explosion4_start =
    (const unsigned char *const) (char*)&data_things_explosion4_start_;
static const unsigned char *const data_things_explosion4_end   =
    (const unsigned char *const) (char*)&data_things_explosion4_end_;

extern unsigned char *data_things_fireball_start_ asm("data_things_fireball_start_");
extern unsigned char *data_things_fireball_end_ asm("data_things_fireball_end_");
static const unsigned char *const data_things_fireball_start =
    (const unsigned char *const) (char*)&data_things_fireball_start_;
static const unsigned char *const data_things_fireball_end   =
    (const unsigned char *const) (char*)&data_things_fireball_end_;

extern unsigned char *data_things_floor1_start_ asm("data_things_floor1_start_");
extern unsigned char *data_things_floor1_end_ asm("data_things_floor1_end_");
static const unsigned char *const data_things_floor1_start =
    (const unsigned char *const) (char*)&data_things_floor1_start_;
static const unsigned char *const data_things_floor1_end   =
    (const unsigned char *const) (char*)&data_things_floor1_end_;

extern unsigned char *data_things_floor2_start_ asm("data_things_floor2_start_");
extern unsigned char *data_things_floor2_end_ asm("data_things_floor2_end_");
static const unsigned char *const data_things_floor2_start =
    (const unsigned char *const) (char*)&data_things_floor2_start_;
static const unsigned char *const data_things_floor2_end   =
    (const unsigned char *const) (char*)&data_things_floor2_end_;

extern unsigned char *data_things_floor3_start_ asm("data_things_floor3_start_");
extern unsigned char *data_things_floor3_end_ asm("data_things_floor3_end_");
static const unsigned char *const data_things_floor3_start =
    (const unsigned char *const) (char*)&data_things_floor3_start_;
static const unsigned char *const data_things_floor3_end   =
    (const unsigned char *const) (char*)&data_things_floor3_end_;

extern unsigned char *data_things_floor4_start_ asm("data_things_floor4_start_");
extern unsigned char *data_things_floor4_end_ asm("data_things_floor4_end_");
static const unsigned char *const data_things_floor4_start =
    (const unsigned char *const) (char*)&data_things_floor4_start_;
static const unsigned char *const data_things_floor4_end   =
    (const unsigned char *const) (char*)&data_things_floor4_end_;

extern unsigned char *data_things_floor5_start_ asm("data_things_floor5_start_");
extern unsigned char *data_things_floor5_end_ asm("data_things_floor5_end_");
static const unsigned char *const data_things_floor5_start =
    (const unsigned char *const) (char*)&data_things_floor5_start_;
static const unsigned char *const data_things_floor5_end   =
    (const unsigned char *const) (char*)&data_things_floor5_end_;

extern unsigned char *data_things_floor6_start_ asm("data_things_floor6_start_");
extern unsigned char *data_things_floor6_end_ asm("data_things_floor6_end_");
static const unsigned char *const data_things_floor6_start =
    (const unsigned char *const) (char*)&data_things_floor6_start_;
static const unsigned char *const data_things_floor6_end   =
    (const unsigned char *const) (char*)&data_things_floor6_end_;

extern unsigned char *data_things_gem1_start_ asm("data_things_gem1_start_");
extern unsigned char *data_things_gem1_end_ asm("data_things_gem1_end_");
static const unsigned char *const data_things_gem1_start =
    (const unsigned char *const) (char*)&data_things_gem1_start_;
static const unsigned char *const data_things_gem1_end   =
    (const unsigned char *const) (char*)&data_things_gem1_end_;

extern unsigned char *data_things_gem2_start_ asm("data_things_gem2_start_");
extern unsigned char *data_things_gem2_end_ asm("data_things_gem2_end_");
static const unsigned char *const data_things_gem2_start =
    (const unsigned char *const) (char*)&data_things_gem2_start_;
static const unsigned char *const data_things_gem2_end   =
    (const unsigned char *const) (char*)&data_things_gem2_end_;

extern unsigned char *data_things_gem3_start_ asm("data_things_gem3_start_");
extern unsigned char *data_things_gem3_end_ asm("data_things_gem3_end_");
static const unsigned char *const data_things_gem3_start =
    (const unsigned char *const) (char*)&data_things_gem3_start_;
static const unsigned char *const data_things_gem3_end   =
    (const unsigned char *const) (char*)&data_things_gem3_end_;

extern unsigned char *data_things_gem4_start_ asm("data_things_gem4_start_");
extern unsigned char *data_things_gem4_end_ asm("data_things_gem4_end_");
static const unsigned char *const data_things_gem4_start =
    (const unsigned char *const) (char*)&data_things_gem4_start_;
static const unsigned char *const data_things_gem4_end   =
    (const unsigned char *const) (char*)&data_things_gem4_end_;

extern unsigned char *data_things_gem5_start_ asm("data_things_gem5_start_");
extern unsigned char *data_things_gem5_end_ asm("data_things_gem5_end_");
static const unsigned char *const data_things_gem5_start =
    (const unsigned char *const) (char*)&data_things_gem5_start_;
static const unsigned char *const data_things_gem5_end   =
    (const unsigned char *const) (char*)&data_things_gem5_end_;

extern unsigned char *data_things_gem6_start_ asm("data_things_gem6_start_");
extern unsigned char *data_things_gem6_end_ asm("data_things_gem6_end_");
static const unsigned char *const data_things_gem6_start =
    (const unsigned char *const) (char*)&data_things_gem6_start_;
static const unsigned char *const data_things_gem6_end   =
    (const unsigned char *const) (char*)&data_things_gem6_end_;

extern unsigned char *data_things_gem7_start_ asm("data_things_gem7_start_");
extern unsigned char *data_things_gem7_end_ asm("data_things_gem7_end_");
static const unsigned char *const data_things_gem7_start =
    (const unsigned char *const) (char*)&data_things_gem7_start_;
static const unsigned char *const data_things_gem7_end   =
    (const unsigned char *const) (char*)&data_things_gem7_end_;

extern unsigned char *data_things_generator1_start_ asm("data_things_generator1_start_");
extern unsigned char *data_things_generator1_end_ asm("data_things_generator1_end_");
static const unsigned char *const data_things_generator1_start =
    (const unsigned char *const) (char*)&data_things_generator1_start_;
static const unsigned char *const data_things_generator1_end   =
    (const unsigned char *const) (char*)&data_things_generator1_end_;

extern unsigned char *data_things_generator5_start_ asm("data_things_generator5_start_");
extern unsigned char *data_things_generator5_end_ asm("data_things_generator5_end_");
static const unsigned char *const data_things_generator5_start =
    (const unsigned char *const) (char*)&data_things_generator5_start_;
static const unsigned char *const data_things_generator5_end   =
    (const unsigned char *const) (char*)&data_things_generator5_end_;

extern unsigned char *data_things_generator6_start_ asm("data_things_generator6_start_");
extern unsigned char *data_things_generator6_end_ asm("data_things_generator6_end_");
static const unsigned char *const data_things_generator6_start =
    (const unsigned char *const) (char*)&data_things_generator6_start_;
static const unsigned char *const data_things_generator6_end   =
    (const unsigned char *const) (char*)&data_things_generator6_end_;

extern unsigned char *data_things_generator7_start_ asm("data_things_generator7_start_");
extern unsigned char *data_things_generator7_end_ asm("data_things_generator7_end_");
static const unsigned char *const data_things_generator7_start =
    (const unsigned char *const) (char*)&data_things_generator7_start_;
static const unsigned char *const data_things_generator7_end   =
    (const unsigned char *const) (char*)&data_things_generator7_end_;

extern unsigned char *data_things_ghost1_start_ asm("data_things_ghost1_start_");
extern unsigned char *data_things_ghost1_end_ asm("data_things_ghost1_end_");
static const unsigned char *const data_things_ghost1_start =
    (const unsigned char *const) (char*)&data_things_ghost1_start_;
static const unsigned char *const data_things_ghost1_end   =
    (const unsigned char *const) (char*)&data_things_ghost1_end_;

extern unsigned char *data_things_ghost2_start_ asm("data_things_ghost2_start_");
extern unsigned char *data_things_ghost2_end_ asm("data_things_ghost2_end_");
static const unsigned char *const data_things_ghost2_start =
    (const unsigned char *const) (char*)&data_things_ghost2_start_;
static const unsigned char *const data_things_ghost2_end   =
    (const unsigned char *const) (char*)&data_things_ghost2_end_;

extern unsigned char *data_things_ghost3_start_ asm("data_things_ghost3_start_");
extern unsigned char *data_things_ghost3_end_ asm("data_things_ghost3_end_");
static const unsigned char *const data_things_ghost3_start =
    (const unsigned char *const) (char*)&data_things_ghost3_start_;
static const unsigned char *const data_things_ghost3_end   =
    (const unsigned char *const) (char*)&data_things_ghost3_end_;

extern unsigned char *data_things_hit_miss_start_ asm("data_things_hit_miss_start_");
extern unsigned char *data_things_hit_miss_end_ asm("data_things_hit_miss_end_");
static const unsigned char *const data_things_hit_miss_start =
    (const unsigned char *const) (char*)&data_things_hit_miss_start_;
static const unsigned char *const data_things_hit_miss_end   =
    (const unsigned char *const) (char*)&data_things_hit_miss_end_;

extern unsigned char *data_things_hit_success_start_ asm("data_things_hit_success_start_");
extern unsigned char *data_things_hit_success_end_ asm("data_things_hit_success_end_");
static const unsigned char *const data_things_hit_success_start =
    (const unsigned char *const) (char*)&data_things_hit_success_start_;
static const unsigned char *const data_things_hit_success_end   =
    (const unsigned char *const) (char*)&data_things_hit_success_end_;

extern unsigned char *data_things_key_start_ asm("data_things_key_start_");
extern unsigned char *data_things_key_end_ asm("data_things_key_end_");
static const unsigned char *const data_things_key_start =
    (const unsigned char *const) (char*)&data_things_key_start_;
static const unsigned char *const data_things_key_end   =
    (const unsigned char *const) (char*)&data_things_key_end_;

extern unsigned char *data_things_keys2_start_ asm("data_things_keys2_start_");
extern unsigned char *data_things_keys2_end_ asm("data_things_keys2_end_");
static const unsigned char *const data_things_keys2_start =
    (const unsigned char *const) (char*)&data_things_keys2_start_;
static const unsigned char *const data_things_keys2_end   =
    (const unsigned char *const) (char*)&data_things_keys2_end_;

extern unsigned char *data_things_keys3_start_ asm("data_things_keys3_start_");
extern unsigned char *data_things_keys3_end_ asm("data_things_keys3_end_");
static const unsigned char *const data_things_keys3_start =
    (const unsigned char *const) (char*)&data_things_keys3_start_;
static const unsigned char *const data_things_keys3_end   =
    (const unsigned char *const) (char*)&data_things_keys3_end_;

extern unsigned char *data_things_noentry_start_ asm("data_things_noentry_start_");
extern unsigned char *data_things_noentry_end_ asm("data_things_noentry_end_");
static const unsigned char *const data_things_noentry_start =
    (const unsigned char *const) (char*)&data_things_noentry_start_;
static const unsigned char *const data_things_noentry_end   =
    (const unsigned char *const) (char*)&data_things_noentry_end_;

extern unsigned char *data_things_pipe_start_ asm("data_things_pipe_start_");
extern unsigned char *data_things_pipe_end_ asm("data_things_pipe_end_");
static const unsigned char *const data_things_pipe_start =
    (const unsigned char *const) (char*)&data_things_pipe_start_;
static const unsigned char *const data_things_pipe_end   =
    (const unsigned char *const) (char*)&data_things_pipe_end_;

extern unsigned char *data_things_poison1_start_ asm("data_things_poison1_start_");
extern unsigned char *data_things_poison1_end_ asm("data_things_poison1_end_");
static const unsigned char *const data_things_poison1_start =
    (const unsigned char *const) (char*)&data_things_poison1_start_;
static const unsigned char *const data_things_poison1_end   =
    (const unsigned char *const) (char*)&data_things_poison1_end_;

extern unsigned char *data_things_poison2_start_ asm("data_things_poison2_start_");
extern unsigned char *data_things_poison2_end_ asm("data_things_poison2_end_");
static const unsigned char *const data_things_poison2_start =
    (const unsigned char *const) (char*)&data_things_poison2_start_;
static const unsigned char *const data_things_poison2_end   =
    (const unsigned char *const) (char*)&data_things_poison2_end_;

extern unsigned char *data_things_poison3_start_ asm("data_things_poison3_start_");
extern unsigned char *data_things_poison3_end_ asm("data_things_poison3_end_");
static const unsigned char *const data_things_poison3_start =
    (const unsigned char *const) (char*)&data_things_poison3_start_;
static const unsigned char *const data_things_poison3_end   =
    (const unsigned char *const) (char*)&data_things_poison3_end_;

extern unsigned char *data_things_poison4_start_ asm("data_things_poison4_start_");
extern unsigned char *data_things_poison4_end_ asm("data_things_poison4_end_");
static const unsigned char *const data_things_poison4_start =
    (const unsigned char *const) (char*)&data_things_poison4_start_;
static const unsigned char *const data_things_poison4_end   =
    (const unsigned char *const) (char*)&data_things_poison4_end_;

extern unsigned char *data_things_potion_cloudkill_start_ asm("data_things_potion_cloudkill_start_");
extern unsigned char *data_things_potion_cloudkill_end_ asm("data_things_potion_cloudkill_end_");
static const unsigned char *const data_things_potion_cloudkill_start =
    (const unsigned char *const) (char*)&data_things_potion_cloudkill_start_;
static const unsigned char *const data_things_potion_cloudkill_end   =
    (const unsigned char *const) (char*)&data_things_potion_cloudkill_end_;

extern unsigned char *data_things_potion_fire_start_ asm("data_things_potion_fire_start_");
extern unsigned char *data_things_potion_fire_end_ asm("data_things_potion_fire_end_");
static const unsigned char *const data_things_potion_fire_start =
    (const unsigned char *const) (char*)&data_things_potion_fire_start_;
static const unsigned char *const data_things_potion_fire_end   =
    (const unsigned char *const) (char*)&data_things_potion_fire_end_;

extern unsigned char *data_things_potion_life_start_ asm("data_things_potion_life_start_");
extern unsigned char *data_things_potion_life_end_ asm("data_things_potion_life_end_");
static const unsigned char *const data_things_potion_life_start =
    (const unsigned char *const) (char*)&data_things_potion_life_start_;
static const unsigned char *const data_things_potion_life_end   =
    (const unsigned char *const) (char*)&data_things_potion_life_end_;

extern unsigned char *data_things_potion_monsticide_start_ asm("data_things_potion_monsticide_start_");
extern unsigned char *data_things_potion_monsticide_end_ asm("data_things_potion_monsticide_end_");
static const unsigned char *const data_things_potion_monsticide_start =
    (const unsigned char *const) (char*)&data_things_potion_monsticide_start_;
static const unsigned char *const data_things_potion_monsticide_end   =
    (const unsigned char *const) (char*)&data_things_potion_monsticide_end_;

extern unsigned char *data_things_potion_shield_start_ asm("data_things_potion_shield_start_");
extern unsigned char *data_things_potion_shield_end_ asm("data_things_potion_shield_end_");
static const unsigned char *const data_things_potion_shield_start =
    (const unsigned char *const) (char*)&data_things_potion_shield_start_;
static const unsigned char *const data_things_potion_shield_end   =
    (const unsigned char *const) (char*)&data_things_potion_shield_end_;

extern unsigned char *data_things_ring1_start_ asm("data_things_ring1_start_");
extern unsigned char *data_things_ring1_end_ asm("data_things_ring1_end_");
static const unsigned char *const data_things_ring1_start =
    (const unsigned char *const) (char*)&data_things_ring1_start_;
static const unsigned char *const data_things_ring1_end   =
    (const unsigned char *const) (char*)&data_things_ring1_end_;

extern unsigned char *data_things_ring2_start_ asm("data_things_ring2_start_");
extern unsigned char *data_things_ring2_end_ asm("data_things_ring2_end_");
static const unsigned char *const data_things_ring2_start =
    (const unsigned char *const) (char*)&data_things_ring2_start_;
static const unsigned char *const data_things_ring2_end   =
    (const unsigned char *const) (char*)&data_things_ring2_end_;

extern unsigned char *data_things_ring3_start_ asm("data_things_ring3_start_");
extern unsigned char *data_things_ring3_end_ asm("data_things_ring3_end_");
static const unsigned char *const data_things_ring3_start =
    (const unsigned char *const) (char*)&data_things_ring3_start_;
static const unsigned char *const data_things_ring3_end   =
    (const unsigned char *const) (char*)&data_things_ring3_end_;

extern unsigned char *data_things_rock1_start_ asm("data_things_rock1_start_");
extern unsigned char *data_things_rock1_end_ asm("data_things_rock1_end_");
static const unsigned char *const data_things_rock1_start =
    (const unsigned char *const) (char*)&data_things_rock1_start_;
static const unsigned char *const data_things_rock1_end   =
    (const unsigned char *const) (char*)&data_things_rock1_end_;

extern unsigned char *data_things_scythe1_start_ asm("data_things_scythe1_start_");
extern unsigned char *data_things_scythe1_end_ asm("data_things_scythe1_end_");
static const unsigned char *const data_things_scythe1_start =
    (const unsigned char *const) (char*)&data_things_scythe1_start_;
static const unsigned char *const data_things_scythe1_end   =
    (const unsigned char *const) (char*)&data_things_scythe1_end_;

extern unsigned char *data_things_scythe1_anim_start_ asm("data_things_scythe1_anim_start_");
extern unsigned char *data_things_scythe1_anim_end_ asm("data_things_scythe1_anim_end_");
static const unsigned char *const data_things_scythe1_anim_start =
    (const unsigned char *const) (char*)&data_things_scythe1_anim_start_;
static const unsigned char *const data_things_scythe1_anim_end   =
    (const unsigned char *const) (char*)&data_things_scythe1_anim_end_;

extern unsigned char *data_things_spam_start_ asm("data_things_spam_start_");
extern unsigned char *data_things_spam_end_ asm("data_things_spam_end_");
static const unsigned char *const data_things_spam_start =
    (const unsigned char *const) (char*)&data_things_spam_start_;
static const unsigned char *const data_things_spam_end   =
    (const unsigned char *const) (char*)&data_things_spam_end_;

extern unsigned char *data_things_sword1_start_ asm("data_things_sword1_start_");
extern unsigned char *data_things_sword1_end_ asm("data_things_sword1_end_");
static const unsigned char *const data_things_sword1_start =
    (const unsigned char *const) (char*)&data_things_sword1_start_;
static const unsigned char *const data_things_sword1_end   =
    (const unsigned char *const) (char*)&data_things_sword1_end_;

extern unsigned char *data_things_sword1_anim_start_ asm("data_things_sword1_anim_start_");
extern unsigned char *data_things_sword1_anim_end_ asm("data_things_sword1_anim_end_");
static const unsigned char *const data_things_sword1_anim_start =
    (const unsigned char *const) (char*)&data_things_sword1_anim_start_;
static const unsigned char *const data_things_sword1_anim_end   =
    (const unsigned char *const) (char*)&data_things_sword1_anim_end_;

extern unsigned char *data_things_sword2_start_ asm("data_things_sword2_start_");
extern unsigned char *data_things_sword2_end_ asm("data_things_sword2_end_");
static const unsigned char *const data_things_sword2_start =
    (const unsigned char *const) (char*)&data_things_sword2_start_;
static const unsigned char *const data_things_sword2_end   =
    (const unsigned char *const) (char*)&data_things_sword2_end_;

extern unsigned char *data_things_sword2_anim_start_ asm("data_things_sword2_anim_start_");
extern unsigned char *data_things_sword2_anim_end_ asm("data_things_sword2_anim_end_");
static const unsigned char *const data_things_sword2_anim_start =
    (const unsigned char *const) (char*)&data_things_sword2_anim_start_;
static const unsigned char *const data_things_sword2_anim_end   =
    (const unsigned char *const) (char*)&data_things_sword2_anim_end_;

extern unsigned char *data_things_sword3_start_ asm("data_things_sword3_start_");
extern unsigned char *data_things_sword3_end_ asm("data_things_sword3_end_");
static const unsigned char *const data_things_sword3_start =
    (const unsigned char *const) (char*)&data_things_sword3_start_;
static const unsigned char *const data_things_sword3_end   =
    (const unsigned char *const) (char*)&data_things_sword3_end_;

extern unsigned char *data_things_sword3_anim_start_ asm("data_things_sword3_anim_start_");
extern unsigned char *data_things_sword3_anim_end_ asm("data_things_sword3_anim_end_");
static const unsigned char *const data_things_sword3_anim_start =
    (const unsigned char *const) (char*)&data_things_sword3_anim_start_;
static const unsigned char *const data_things_sword3_anim_end   =
    (const unsigned char *const) (char*)&data_things_sword3_anim_end_;

extern unsigned char *data_things_torch_start_ asm("data_things_torch_start_");
extern unsigned char *data_things_torch_end_ asm("data_things_torch_end_");
static const unsigned char *const data_things_torch_start =
    (const unsigned char *const) (char*)&data_things_torch_start_;
static const unsigned char *const data_things_torch_end   =
    (const unsigned char *const) (char*)&data_things_torch_end_;

extern unsigned char *data_things_torches_start_ asm("data_things_torches_start_");
extern unsigned char *data_things_torches_end_ asm("data_things_torches_end_");
static const unsigned char *const data_things_torches_start =
    (const unsigned char *const) (char*)&data_things_torches_start_;
static const unsigned char *const data_things_torches_end   =
    (const unsigned char *const) (char*)&data_things_torches_end_;

extern unsigned char *data_things_valkyrie_start_ asm("data_things_valkyrie_start_");
extern unsigned char *data_things_valkyrie_end_ asm("data_things_valkyrie_end_");
static const unsigned char *const data_things_valkyrie_start =
    (const unsigned char *const) (char*)&data_things_valkyrie_start_;
static const unsigned char *const data_things_valkyrie_end   =
    (const unsigned char *const) (char*)&data_things_valkyrie_end_;

extern unsigned char *data_things_wall1_start_ asm("data_things_wall1_start_");
extern unsigned char *data_things_wall1_end_ asm("data_things_wall1_end_");
static const unsigned char *const data_things_wall1_start =
    (const unsigned char *const) (char*)&data_things_wall1_start_;
static const unsigned char *const data_things_wall1_end   =
    (const unsigned char *const) (char*)&data_things_wall1_end_;

extern unsigned char *data_things_wall2_start_ asm("data_things_wall2_start_");
extern unsigned char *data_things_wall2_end_ asm("data_things_wall2_end_");
static const unsigned char *const data_things_wall2_start =
    (const unsigned char *const) (char*)&data_things_wall2_start_;
static const unsigned char *const data_things_wall2_end   =
    (const unsigned char *const) (char*)&data_things_wall2_end_;

extern unsigned char *data_things_wall3_start_ asm("data_things_wall3_start_");
extern unsigned char *data_things_wall3_end_ asm("data_things_wall3_end_");
static const unsigned char *const data_things_wall3_start =
    (const unsigned char *const) (char*)&data_things_wall3_start_;
static const unsigned char *const data_things_wall3_end   =
    (const unsigned char *const) (char*)&data_things_wall3_end_;

extern unsigned char *data_things_wall4_start_ asm("data_things_wall4_start_");
extern unsigned char *data_things_wall4_end_ asm("data_things_wall4_end_");
static const unsigned char *const data_things_wall4_start =
    (const unsigned char *const) (char*)&data_things_wall4_start_;
static const unsigned char *const data_things_wall4_end   =
    (const unsigned char *const) (char*)&data_things_wall4_end_;

extern unsigned char *data_things_wall5_start_ asm("data_things_wall5_start_");
extern unsigned char *data_things_wall5_end_ asm("data_things_wall5_end_");
static const unsigned char *const data_things_wall5_start =
    (const unsigned char *const) (char*)&data_things_wall5_start_;
static const unsigned char *const data_things_wall5_end   =
    (const unsigned char *const) (char*)&data_things_wall5_end_;

extern unsigned char *data_things_wall6_start_ asm("data_things_wall6_start_");
extern unsigned char *data_things_wall6_end_ asm("data_things_wall6_end_");
static const unsigned char *const data_things_wall6_start =
    (const unsigned char *const) (char*)&data_things_wall6_start_;
static const unsigned char *const data_things_wall6_end   =
    (const unsigned char *const) (char*)&data_things_wall6_end_;

extern unsigned char *data_things_wand1_anim_start_ asm("data_things_wand1_anim_start_");
extern unsigned char *data_things_wand1_anim_end_ asm("data_things_wand1_anim_end_");
static const unsigned char *const data_things_wand1_anim_start =
    (const unsigned char *const) (char*)&data_things_wand1_anim_start_;
static const unsigned char *const data_things_wand1_anim_end   =
    (const unsigned char *const) (char*)&data_things_wand1_anim_end_;

extern unsigned char *data_things_wand_fire_start_ asm("data_things_wand_fire_start_");
extern unsigned char *data_things_wand_fire_end_ asm("data_things_wand_fire_end_");
static const unsigned char *const data_things_wand_fire_start =
    (const unsigned char *const) (char*)&data_things_wand_fire_start_;
static const unsigned char *const data_things_wand_fire_end   =
    (const unsigned char *const) (char*)&data_things_wand_fire_end_;

extern unsigned char *data_things_warrior_start_ asm("data_things_warrior_start_");
extern unsigned char *data_things_warrior_end_ asm("data_things_warrior_end_");
static const unsigned char *const data_things_warrior_start =
    (const unsigned char *const) (char*)&data_things_warrior_start_;
static const unsigned char *const data_things_warrior_end   =
    (const unsigned char *const) (char*)&data_things_warrior_end_;

extern unsigned char *data_things_water_start_ asm("data_things_water_start_");
extern unsigned char *data_things_water_end_ asm("data_things_water_end_");
static const unsigned char *const data_things_water_start =
    (const unsigned char *const) (char*)&data_things_water_start_;
static const unsigned char *const data_things_water_end   =
    (const unsigned char *const) (char*)&data_things_water_end_;

extern unsigned char *data_things_water_poison_start_ asm("data_things_water_poison_start_");
extern unsigned char *data_things_water_poison_end_ asm("data_things_water_poison_end_");
static const unsigned char *const data_things_water_poison_start =
    (const unsigned char *const) (char*)&data_things_water_poison_start_;
static const unsigned char *const data_things_water_poison_end   =
    (const unsigned char *const) (char*)&data_things_water_poison_end_;

extern unsigned char *data_things_weapon_swing1_start_ asm("data_things_weapon_swing1_start_");
extern unsigned char *data_things_weapon_swing1_end_ asm("data_things_weapon_swing1_end_");
static const unsigned char *const data_things_weapon_swing1_start =
    (const unsigned char *const) (char*)&data_things_weapon_swing1_start_;
static const unsigned char *const data_things_weapon_swing1_end   =
    (const unsigned char *const) (char*)&data_things_weapon_swing1_end_;

extern unsigned char *data_things_wizard_start_ asm("data_things_wizard_start_");
extern unsigned char *data_things_wizard_end_ asm("data_things_wizard_end_");
static const unsigned char *const data_things_wizard_start =
    (const unsigned char *const) (char*)&data_things_wizard_start_;
static const unsigned char *const data_things_wizard_end   =
    (const unsigned char *const) (char*)&data_things_wizard_end_;

extern unsigned char *data_ttf_emulogic_gif_start_ asm("data_ttf_emulogic_gif_start_");
extern unsigned char *data_ttf_emulogic_gif_end_ asm("data_ttf_emulogic_gif_end_");
static const unsigned char *const data_ttf_emulogic_gif_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_gif_start_;
static const unsigned char *const data_ttf_emulogic_gif_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_gif_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize10_data_start_ asm("data_ttf_emulogic_ttf_pointsize10_data_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize10_data_end_ asm("data_ttf_emulogic_ttf_pointsize10_data_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize10_data_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize10_data_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize10_data_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize10_data_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize10_tga_start_ asm("data_ttf_emulogic_ttf_pointsize10_tga_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize10_tga_end_ asm("data_ttf_emulogic_ttf_pointsize10_tga_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize10_tga_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize10_tga_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize10_tga_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize10_tga_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize11_data_start_ asm("data_ttf_emulogic_ttf_pointsize11_data_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize11_data_end_ asm("data_ttf_emulogic_ttf_pointsize11_data_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize11_data_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize11_data_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize11_data_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize11_data_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize11_tga_start_ asm("data_ttf_emulogic_ttf_pointsize11_tga_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize11_tga_end_ asm("data_ttf_emulogic_ttf_pointsize11_tga_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize11_tga_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize11_tga_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize11_tga_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize11_tga_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize12_data_start_ asm("data_ttf_emulogic_ttf_pointsize12_data_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize12_data_end_ asm("data_ttf_emulogic_ttf_pointsize12_data_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize12_data_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize12_data_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize12_data_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize12_data_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize12_tga_start_ asm("data_ttf_emulogic_ttf_pointsize12_tga_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize12_tga_end_ asm("data_ttf_emulogic_ttf_pointsize12_tga_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize12_tga_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize12_tga_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize12_tga_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize12_tga_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize14_data_start_ asm("data_ttf_emulogic_ttf_pointsize14_data_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize14_data_end_ asm("data_ttf_emulogic_ttf_pointsize14_data_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize14_data_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize14_data_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize14_data_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize14_data_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize14_tga_start_ asm("data_ttf_emulogic_ttf_pointsize14_tga_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize14_tga_end_ asm("data_ttf_emulogic_ttf_pointsize14_tga_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize14_tga_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize14_tga_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize14_tga_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize14_tga_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize15_data_start_ asm("data_ttf_emulogic_ttf_pointsize15_data_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize15_data_end_ asm("data_ttf_emulogic_ttf_pointsize15_data_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize15_data_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize15_data_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize15_data_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize15_data_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize15_tga_start_ asm("data_ttf_emulogic_ttf_pointsize15_tga_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize15_tga_end_ asm("data_ttf_emulogic_ttf_pointsize15_tga_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize15_tga_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize15_tga_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize15_tga_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize15_tga_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize16_data_start_ asm("data_ttf_emulogic_ttf_pointsize16_data_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize16_data_end_ asm("data_ttf_emulogic_ttf_pointsize16_data_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize16_data_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize16_data_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize16_data_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize16_data_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize16_tga_start_ asm("data_ttf_emulogic_ttf_pointsize16_tga_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize16_tga_end_ asm("data_ttf_emulogic_ttf_pointsize16_tga_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize16_tga_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize16_tga_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize16_tga_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize16_tga_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize17_data_start_ asm("data_ttf_emulogic_ttf_pointsize17_data_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize17_data_end_ asm("data_ttf_emulogic_ttf_pointsize17_data_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize17_data_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize17_data_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize17_data_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize17_data_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize17_tga_start_ asm("data_ttf_emulogic_ttf_pointsize17_tga_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize17_tga_end_ asm("data_ttf_emulogic_ttf_pointsize17_tga_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize17_tga_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize17_tga_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize17_tga_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize17_tga_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize18_data_start_ asm("data_ttf_emulogic_ttf_pointsize18_data_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize18_data_end_ asm("data_ttf_emulogic_ttf_pointsize18_data_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize18_data_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize18_data_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize18_data_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize18_data_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize18_tga_start_ asm("data_ttf_emulogic_ttf_pointsize18_tga_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize18_tga_end_ asm("data_ttf_emulogic_ttf_pointsize18_tga_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize18_tga_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize18_tga_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize18_tga_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize18_tga_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize19_data_start_ asm("data_ttf_emulogic_ttf_pointsize19_data_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize19_data_end_ asm("data_ttf_emulogic_ttf_pointsize19_data_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize19_data_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize19_data_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize19_data_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize19_data_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize19_tga_start_ asm("data_ttf_emulogic_ttf_pointsize19_tga_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize19_tga_end_ asm("data_ttf_emulogic_ttf_pointsize19_tga_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize19_tga_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize19_tga_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize19_tga_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize19_tga_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize20_data_start_ asm("data_ttf_emulogic_ttf_pointsize20_data_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize20_data_end_ asm("data_ttf_emulogic_ttf_pointsize20_data_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize20_data_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize20_data_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize20_data_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize20_data_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize20_tga_start_ asm("data_ttf_emulogic_ttf_pointsize20_tga_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize20_tga_end_ asm("data_ttf_emulogic_ttf_pointsize20_tga_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize20_tga_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize20_tga_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize20_tga_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize20_tga_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize21_data_start_ asm("data_ttf_emulogic_ttf_pointsize21_data_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize21_data_end_ asm("data_ttf_emulogic_ttf_pointsize21_data_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize21_data_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize21_data_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize21_data_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize21_data_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize21_tga_start_ asm("data_ttf_emulogic_ttf_pointsize21_tga_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize21_tga_end_ asm("data_ttf_emulogic_ttf_pointsize21_tga_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize21_tga_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize21_tga_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize21_tga_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize21_tga_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize22_data_start_ asm("data_ttf_emulogic_ttf_pointsize22_data_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize22_data_end_ asm("data_ttf_emulogic_ttf_pointsize22_data_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize22_data_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize22_data_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize22_data_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize22_data_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize22_tga_start_ asm("data_ttf_emulogic_ttf_pointsize22_tga_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize22_tga_end_ asm("data_ttf_emulogic_ttf_pointsize22_tga_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize22_tga_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize22_tga_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize22_tga_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize22_tga_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize23_data_start_ asm("data_ttf_emulogic_ttf_pointsize23_data_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize23_data_end_ asm("data_ttf_emulogic_ttf_pointsize23_data_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize23_data_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize23_data_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize23_data_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize23_data_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize23_tga_start_ asm("data_ttf_emulogic_ttf_pointsize23_tga_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize23_tga_end_ asm("data_ttf_emulogic_ttf_pointsize23_tga_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize23_tga_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize23_tga_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize23_tga_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize23_tga_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize24_data_start_ asm("data_ttf_emulogic_ttf_pointsize24_data_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize24_data_end_ asm("data_ttf_emulogic_ttf_pointsize24_data_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize24_data_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize24_data_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize24_data_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize24_data_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize24_tga_start_ asm("data_ttf_emulogic_ttf_pointsize24_tga_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize24_tga_end_ asm("data_ttf_emulogic_ttf_pointsize24_tga_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize24_tga_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize24_tga_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize24_tga_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize24_tga_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize25_data_start_ asm("data_ttf_emulogic_ttf_pointsize25_data_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize25_data_end_ asm("data_ttf_emulogic_ttf_pointsize25_data_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize25_data_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize25_data_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize25_data_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize25_data_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize25_tga_start_ asm("data_ttf_emulogic_ttf_pointsize25_tga_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize25_tga_end_ asm("data_ttf_emulogic_ttf_pointsize25_tga_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize25_tga_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize25_tga_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize25_tga_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize25_tga_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize26_data_start_ asm("data_ttf_emulogic_ttf_pointsize26_data_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize26_data_end_ asm("data_ttf_emulogic_ttf_pointsize26_data_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize26_data_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize26_data_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize26_data_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize26_data_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize26_tga_start_ asm("data_ttf_emulogic_ttf_pointsize26_tga_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize26_tga_end_ asm("data_ttf_emulogic_ttf_pointsize26_tga_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize26_tga_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize26_tga_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize26_tga_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize26_tga_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize28_data_start_ asm("data_ttf_emulogic_ttf_pointsize28_data_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize28_data_end_ asm("data_ttf_emulogic_ttf_pointsize28_data_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize28_data_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize28_data_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize28_data_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize28_data_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize28_tga_start_ asm("data_ttf_emulogic_ttf_pointsize28_tga_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize28_tga_end_ asm("data_ttf_emulogic_ttf_pointsize28_tga_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize28_tga_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize28_tga_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize28_tga_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize28_tga_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize30_data_start_ asm("data_ttf_emulogic_ttf_pointsize30_data_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize30_data_end_ asm("data_ttf_emulogic_ttf_pointsize30_data_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize30_data_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize30_data_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize30_data_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize30_data_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize30_tga_start_ asm("data_ttf_emulogic_ttf_pointsize30_tga_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize30_tga_end_ asm("data_ttf_emulogic_ttf_pointsize30_tga_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize30_tga_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize30_tga_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize30_tga_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize30_tga_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize32_data_start_ asm("data_ttf_emulogic_ttf_pointsize32_data_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize32_data_end_ asm("data_ttf_emulogic_ttf_pointsize32_data_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize32_data_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize32_data_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize32_data_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize32_data_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize32_tga_start_ asm("data_ttf_emulogic_ttf_pointsize32_tga_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize32_tga_end_ asm("data_ttf_emulogic_ttf_pointsize32_tga_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize32_tga_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize32_tga_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize32_tga_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize32_tga_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize36_data_start_ asm("data_ttf_emulogic_ttf_pointsize36_data_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize36_data_end_ asm("data_ttf_emulogic_ttf_pointsize36_data_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize36_data_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize36_data_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize36_data_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize36_data_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize36_tga_start_ asm("data_ttf_emulogic_ttf_pointsize36_tga_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize36_tga_end_ asm("data_ttf_emulogic_ttf_pointsize36_tga_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize36_tga_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize36_tga_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize36_tga_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize36_tga_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize6_data_start_ asm("data_ttf_emulogic_ttf_pointsize6_data_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize6_data_end_ asm("data_ttf_emulogic_ttf_pointsize6_data_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize6_data_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize6_data_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize6_data_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize6_data_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize6_tga_start_ asm("data_ttf_emulogic_ttf_pointsize6_tga_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize6_tga_end_ asm("data_ttf_emulogic_ttf_pointsize6_tga_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize6_tga_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize6_tga_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize6_tga_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize6_tga_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize7_data_start_ asm("data_ttf_emulogic_ttf_pointsize7_data_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize7_data_end_ asm("data_ttf_emulogic_ttf_pointsize7_data_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize7_data_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize7_data_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize7_data_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize7_data_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize7_tga_start_ asm("data_ttf_emulogic_ttf_pointsize7_tga_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize7_tga_end_ asm("data_ttf_emulogic_ttf_pointsize7_tga_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize7_tga_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize7_tga_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize7_tga_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize7_tga_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize8_data_start_ asm("data_ttf_emulogic_ttf_pointsize8_data_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize8_data_end_ asm("data_ttf_emulogic_ttf_pointsize8_data_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize8_data_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize8_data_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize8_data_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize8_data_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize8_tga_start_ asm("data_ttf_emulogic_ttf_pointsize8_tga_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize8_tga_end_ asm("data_ttf_emulogic_ttf_pointsize8_tga_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize8_tga_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize8_tga_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize8_tga_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize8_tga_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize9_data_start_ asm("data_ttf_emulogic_ttf_pointsize9_data_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize9_data_end_ asm("data_ttf_emulogic_ttf_pointsize9_data_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize9_data_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize9_data_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize9_data_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize9_data_end_;

extern unsigned char *data_ttf_emulogic_ttf_pointsize9_tga_start_ asm("data_ttf_emulogic_ttf_pointsize9_tga_start_");
extern unsigned char *data_ttf_emulogic_ttf_pointsize9_tga_end_ asm("data_ttf_emulogic_ttf_pointsize9_tga_end_");
static const unsigned char *const data_ttf_emulogic_ttf_pointsize9_tga_start =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize9_tga_start_;
static const unsigned char *const data_ttf_emulogic_ttf_pointsize9_tga_end   =
    (const unsigned char *const) (char*)&data_ttf_emulogic_ttf_pointsize9_tga_end_;

extern unsigned char *data_ttf_LICENSE_txt_start_ asm("data_ttf_LICENSE_txt_start_");
extern unsigned char *data_ttf_LICENSE_txt_end_ asm("data_ttf_LICENSE_txt_end_");
static const unsigned char *const data_ttf_LICENSE_txt_start =
    (const unsigned char *const) (char*)&data_ttf_LICENSE_txt_start_;
static const unsigned char *const data_ttf_LICENSE_txt_end   =
    (const unsigned char *const) (char*)&data_ttf_LICENSE_txt_end_;

extern unsigned char *data_ttf_Readme_txt_start_ asm("data_ttf_Readme_txt_start_");
extern unsigned char *data_ttf_Readme_txt_end_ asm("data_ttf_Readme_txt_end_");
static const unsigned char *const data_ttf_Readme_txt_start =
    (const unsigned char *const) (char*)&data_ttf_Readme_txt_start_;
static const unsigned char *const data_ttf_Readme_txt_end   =
    (const unsigned char *const) (char*)&data_ttf_Readme_txt_end_;

ramdisk_t ramdisk_data[] = {
{
    /* filename */ "data/gfx/anim_eyes.png",
},
{
    /* filename */ "data/gfx/anim_man.png",
},
{
    /* filename */ "data/gfx/anim_treasure_chest.png",
},
{
    /* filename */ "data/gfx/cloudkill.png",
},
{
    /* filename */ "data/gfx/dragon.png",
},
{
    /* filename */ "data/gfx/explosion.png",
},
{
    /* filename */ "data/gfx/giant_rock.png",
},
{
    /* filename */ "data/gfx/gorynlich.png",
},
{
    /* filename */ "data/gfx/gothic_wide.png",
},
{
    /* filename */ "data/gfx/gravestone.png",
},
{
    /* filename */ "data/gfx/gravestone2.png",
},
{
    /* filename */ "data/gfx/help1.png",
},
{
    /* filename */ "data/gfx/help2.png",
},
{
    /* filename */ "data/gfx/help3.png",
},
{
    /* filename */ "data/gfx/main_title.png",
},
{
    /* filename */ "data/gfx/poison.png",
},
{
    /* filename */ "data/gfx/rat.png",
},
{
    /* filename */ "data/gfx/button_red.png",
},
{
    /* filename */ "data/gfx/sprites_small.png",
},
{
    /* filename */ "data/gfx/title.png",
},
{
    /* filename */ "data/gfx/title2.png",
},
{
    /* filename */ "data/gfx/title3.png",
},
{
    /* filename */ "data/gfx/title_small.png",
},
{
    /* filename */ "data/gfx/wall_floor.png",
},
{
    /* filename */ "data/gfx/window.png",
},
{
    /* filename */ "data/gfx/window2.png",
},
{
    /* filename */ "data/levels/1",
},
{
    /* filename */ "data/levels/2",
},
{
    /* filename */ "data/map/jigsaw.map",
},
{
    /* filename */ "data/music/charlotte.mp3",
},
{
    /* filename */ "data/music/charlotte8bit.mp3",
},
{
    /* filename */ "data/music/hiscore.wav",
},
{
    /* filename */ "data/music/intro.wav",
},
{
    /* filename */ "data/music/music.wav",
},
{
    /* filename */ "data/sound/chomp.wav",
},
{
    /* filename */ "data/sound/chomp2.wav",
},
{
    /* filename */ "data/sound/click.wav",
},
{
    /* filename */ "data/sound/doorbell.wav",
},
{
    /* filename */ "data/sound/explosion.wav",
},
{
    /* filename */ "data/sound/letter.wav",
},
{
    /* filename */ "data/sound/level_end.wav",
},
{
    /* filename */ "data/sound/paper.wav",
},
{
    /* filename */ "data/sound/powerup.wav",
},
{
    /* filename */ "data/sound/ratdeath.wav",
},
{
    /* filename */ "data/sound/rocket.wav",
},
{
    /* filename */ "data/sound/slime.wav",
},
{
    /* filename */ "data/sound/spam.wav",
},
{
    /* filename */ "data/sound/thief.wav",
},
{
    /* filename */ "data/things/.anim_eyes.session",
},
{
    /* filename */ "data/things/.anim_man.session",
},
{
    /* filename */ "data/things/.anim_treasure_chest.session",
},
{
    /* filename */ "data/things/.blood1.session",
},
{
    /* filename */ "data/things/.blood2.session",
},
{
    /* filename */ "data/things/.brazier.session",
},
{
    /* filename */ "data/things/.death.session",
},
{
    /* filename */ "data/things/.door1.session",
},
{
    /* filename */ "data/things/.elf.session",
},
{
    /* filename */ "data/things/.exit7.session",
},
{
    /* filename */ "data/things/.floor1.session",
},
{
    /* filename */ "data/things/.key.session",
},
{
    /* filename */ "data/things/.keys.session",
},
{
    /* filename */ "data/things/.plant.session",
},
{
    /* filename */ "data/things/.spam.session",
},
{
    /* filename */ "data/things/.sparks1.session",
},
{
    /* filename */ "data/things/.sparks2.session",
},
{
    /* filename */ "data/things/.torch.session",
},
{
    /* filename */ "data/things/.torches.session",
},
{
    /* filename */ "data/things/.wand_f.session",
},
{
    /* filename */ "data/things/.wand_fire.session",
},
{
    /* filename */ "data/things/.warrior.session",
},
{
    /* filename */ "data/things/.wizard.session",
},
{
    /* filename */ "data/things/amulet1",
},
{
    /* filename */ "data/things/anim_eyes",
},
{
    /* filename */ "data/things/anim_man",
},
{
    /* filename */ "data/things/anim_treasure_chest",
},
{
    /* filename */ "data/things/apple",
},
{
    /* filename */ "data/things/apple_poison",
},
{
    /* filename */ "data/things/arrow",
},
{
    /* filename */ "data/things/axe1",
},
{
    /* filename */ "data/things/axe1_anim",
},
{
    /* filename */ "data/things/axe2",
},
{
    /* filename */ "data/things/axe2_anim",
},
{
    /* filename */ "data/things/banana",
},
{
    /* filename */ "data/things/blood1",
},
{
    /* filename */ "data/things/blood2",
},
{
    /* filename */ "data/things/bonepile1",
},
{
    /* filename */ "data/things/bonepile2",
},
{
    /* filename */ "data/things/bonepile3",
},
{
    /* filename */ "data/things/bow1",
},
{
    /* filename */ "data/things/bow1_anim",
},
{
    /* filename */ "data/things/bow2",
},
{
    /* filename */ "data/things/bow3",
},
{
    /* filename */ "data/things/bow4",
},
{
    /* filename */ "data/things/brazier",
},
{
    /* filename */ "data/things/bread",
},
{
    /* filename */ "data/things/cheese",
},
{
    /* filename */ "data/things/chest1",
},
{
    /* filename */ "data/things/cloudkill1",
},
{
    /* filename */ "data/things/cloudkill2",
},
{
    /* filename */ "data/things/cloudkill3",
},
{
    /* filename */ "data/things/cloudkill4",
},
{
    /* filename */ "data/things/coins1",
},
{
    /* filename */ "data/things/death",
},
{
    /* filename */ "data/things/door1",
},
{
    /* filename */ "data/things/door2",
},
{
    /* filename */ "data/things/dwarf",
},
{
    /* filename */ "data/things/elf",
},
{
    /* filename */ "data/things/exit1",
},
{
    /* filename */ "data/things/exit2",
},
{
    /* filename */ "data/things/exit3",
},
{
    /* filename */ "data/things/exit4",
},
{
    /* filename */ "data/things/exit5",
},
{
    /* filename */ "data/things/exit6",
},
{
    /* filename */ "data/things/exit7",
},
{
    /* filename */ "data/things/explosion1",
},
{
    /* filename */ "data/things/explosion2",
},
{
    /* filename */ "data/things/explosion3",
},
{
    /* filename */ "data/things/explosion4",
},
{
    /* filename */ "data/things/fireball",
},
{
    /* filename */ "data/things/floor1",
},
{
    /* filename */ "data/things/floor2",
},
{
    /* filename */ "data/things/floor3",
},
{
    /* filename */ "data/things/floor4",
},
{
    /* filename */ "data/things/floor5",
},
{
    /* filename */ "data/things/floor6",
},
{
    /* filename */ "data/things/gem1",
},
{
    /* filename */ "data/things/gem2",
},
{
    /* filename */ "data/things/gem3",
},
{
    /* filename */ "data/things/gem4",
},
{
    /* filename */ "data/things/gem5",
},
{
    /* filename */ "data/things/gem6",
},
{
    /* filename */ "data/things/gem7",
},
{
    /* filename */ "data/things/generator1",
},
{
    /* filename */ "data/things/generator5",
},
{
    /* filename */ "data/things/generator6",
},
{
    /* filename */ "data/things/generator7",
},
{
    /* filename */ "data/things/ghost1",
},
{
    /* filename */ "data/things/ghost2",
},
{
    /* filename */ "data/things/ghost3",
},
{
    /* filename */ "data/things/hit_miss",
},
{
    /* filename */ "data/things/hit_success",
},
{
    /* filename */ "data/things/key",
},
{
    /* filename */ "data/things/keys2",
},
{
    /* filename */ "data/things/keys3",
},
{
    /* filename */ "data/things/noentry",
},
{
    /* filename */ "data/things/pipe",
},
{
    /* filename */ "data/things/poison1",
},
{
    /* filename */ "data/things/poison2",
},
{
    /* filename */ "data/things/poison3",
},
{
    /* filename */ "data/things/poison4",
},
{
    /* filename */ "data/things/potion_cloudkill",
},
{
    /* filename */ "data/things/potion_fire",
},
{
    /* filename */ "data/things/potion_life",
},
{
    /* filename */ "data/things/potion_monsticide",
},
{
    /* filename */ "data/things/potion_shield",
},
{
    /* filename */ "data/things/ring1",
},
{
    /* filename */ "data/things/ring2",
},
{
    /* filename */ "data/things/ring3",
},
{
    /* filename */ "data/things/rock1",
},
{
    /* filename */ "data/things/scythe1",
},
{
    /* filename */ "data/things/scythe1_anim",
},
{
    /* filename */ "data/things/spam",
},
{
    /* filename */ "data/things/sword1",
},
{
    /* filename */ "data/things/sword1_anim",
},
{
    /* filename */ "data/things/sword2",
},
{
    /* filename */ "data/things/sword2_anim",
},
{
    /* filename */ "data/things/sword3",
},
{
    /* filename */ "data/things/sword3_anim",
},
{
    /* filename */ "data/things/torch",
},
{
    /* filename */ "data/things/torches",
},
{
    /* filename */ "data/things/valkyrie",
},
{
    /* filename */ "data/things/wall1",
},
{
    /* filename */ "data/things/wall2",
},
{
    /* filename */ "data/things/wall3",
},
{
    /* filename */ "data/things/wall4",
},
{
    /* filename */ "data/things/wall5",
},
{
    /* filename */ "data/things/wall6",
},
{
    /* filename */ "data/things/wand1_anim",
},
{
    /* filename */ "data/things/wand_fire",
},
{
    /* filename */ "data/things/warrior",
},
{
    /* filename */ "data/things/water",
},
{
    /* filename */ "data/things/water_poison",
},
{
    /* filename */ "data/things/weapon_swing1",
},
{
    /* filename */ "data/things/wizard",
},
{
    /* filename */ "data/ttf/emulogic.gif",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize10.data",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize10.tga",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize11.data",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize11.tga",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize12.data",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize12.tga",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize14.data",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize14.tga",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize15.data",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize15.tga",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize16.data",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize16.tga",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize17.data",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize17.tga",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize18.data",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize18.tga",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize19.data",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize19.tga",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize20.data",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize20.tga",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize21.data",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize21.tga",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize22.data",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize22.tga",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize23.data",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize23.tga",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize24.data",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize24.tga",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize25.data",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize25.tga",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize26.data",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize26.tga",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize28.data",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize28.tga",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize30.data",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize30.tga",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize32.data",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize32.tga",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize36.data",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize36.tga",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize6.data",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize6.tga",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize7.data",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize7.tga",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize8.data",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize8.tga",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize9.data",
},
{
    /* filename */ "data/ttf/emulogic.ttf_pointsize9.tga",
},
{
    /* filename */ "data/ttf/LICENSE.txt",
},
{
    /* filename */ "data/ttf/Readme.txt",
},
    {0},
};

void ramdisk_init (void)
{
    ramdisk_data[0].data = data_gfx_anim_eyes_png_start;
    ramdisk_data[0].len = data_gfx_anim_eyes_png_end - data_gfx_anim_eyes_png_start;
    ramdisk_data[1].data = data_gfx_anim_man_png_start;
    ramdisk_data[1].len = data_gfx_anim_man_png_end - data_gfx_anim_man_png_start;
    ramdisk_data[2].data = data_gfx_anim_treasure_chest_png_start;
    ramdisk_data[2].len = data_gfx_anim_treasure_chest_png_end - data_gfx_anim_treasure_chest_png_start;
    ramdisk_data[3].data = data_gfx_cloudkill_png_start;
    ramdisk_data[3].len = data_gfx_cloudkill_png_end - data_gfx_cloudkill_png_start;
    ramdisk_data[4].data = data_gfx_dragon_png_start;
    ramdisk_data[4].len = data_gfx_dragon_png_end - data_gfx_dragon_png_start;
    ramdisk_data[5].data = data_gfx_explosion_png_start;
    ramdisk_data[5].len = data_gfx_explosion_png_end - data_gfx_explosion_png_start;
    ramdisk_data[6].data = data_gfx_giant_rock_png_start;
    ramdisk_data[6].len = data_gfx_giant_rock_png_end - data_gfx_giant_rock_png_start;
    ramdisk_data[7].data = data_gfx_gorynlich_png_start;
    ramdisk_data[7].len = data_gfx_gorynlich_png_end - data_gfx_gorynlich_png_start;
    ramdisk_data[8].data = data_gfx_gothic_wide_png_start;
    ramdisk_data[8].len = data_gfx_gothic_wide_png_end - data_gfx_gothic_wide_png_start;
    ramdisk_data[9].data = data_gfx_gravestone_png_start;
    ramdisk_data[9].len = data_gfx_gravestone_png_end - data_gfx_gravestone_png_start;
    ramdisk_data[10].data = data_gfx_gravestone2_png_start;
    ramdisk_data[10].len = data_gfx_gravestone2_png_end - data_gfx_gravestone2_png_start;
    ramdisk_data[11].data = data_gfx_help1_png_start;
    ramdisk_data[11].len = data_gfx_help1_png_end - data_gfx_help1_png_start;
    ramdisk_data[12].data = data_gfx_help2_png_start;
    ramdisk_data[12].len = data_gfx_help2_png_end - data_gfx_help2_png_start;
    ramdisk_data[13].data = data_gfx_help3_png_start;
    ramdisk_data[13].len = data_gfx_help3_png_end - data_gfx_help3_png_start;
    ramdisk_data[14].data = data_gfx_main_title_png_start;
    ramdisk_data[14].len = data_gfx_main_title_png_end - data_gfx_main_title_png_start;
    ramdisk_data[15].data = data_gfx_poison_png_start;
    ramdisk_data[15].len = data_gfx_poison_png_end - data_gfx_poison_png_start;
    ramdisk_data[16].data = data_gfx_rat_png_start;
    ramdisk_data[16].len = data_gfx_rat_png_end - data_gfx_rat_png_start;
    ramdisk_data[17].data = data_gfx_button_red_png_start;
    ramdisk_data[17].len = data_gfx_button_red_png_end - data_gfx_button_red_png_start;
    ramdisk_data[18].data = data_gfx_sprites_small_png_start;
    ramdisk_data[18].len = data_gfx_sprites_small_png_end - data_gfx_sprites_small_png_start;
    ramdisk_data[19].data = data_gfx_title_png_start;
    ramdisk_data[19].len = data_gfx_title_png_end - data_gfx_title_png_start;
    ramdisk_data[20].data = data_gfx_title2_png_start;
    ramdisk_data[20].len = data_gfx_title2_png_end - data_gfx_title2_png_start;
    ramdisk_data[21].data = data_gfx_title3_png_start;
    ramdisk_data[21].len = data_gfx_title3_png_end - data_gfx_title3_png_start;
    ramdisk_data[22].data = data_gfx_title_small_png_start;
    ramdisk_data[22].len = data_gfx_title_small_png_end - data_gfx_title_small_png_start;
    ramdisk_data[23].data = data_gfx_wall_floor_png_start;
    ramdisk_data[23].len = data_gfx_wall_floor_png_end - data_gfx_wall_floor_png_start;
    ramdisk_data[24].data = data_gfx_window_png_start;
    ramdisk_data[24].len = data_gfx_window_png_end - data_gfx_window_png_start;
    ramdisk_data[25].data = data_gfx_window2_png_start;
    ramdisk_data[25].len = data_gfx_window2_png_end - data_gfx_window2_png_start;
    ramdisk_data[26].data = data_levels_1_start;
    ramdisk_data[26].len = data_levels_1_end - data_levels_1_start;
    ramdisk_data[27].data = data_levels_2_start;
    ramdisk_data[27].len = data_levels_2_end - data_levels_2_start;
    ramdisk_data[28].data = data_map_jigsaw_map_start;
    ramdisk_data[28].len = data_map_jigsaw_map_end - data_map_jigsaw_map_start;
    ramdisk_data[29].data = data_music_charlotte_mp3_start;
    ramdisk_data[29].len = data_music_charlotte_mp3_end - data_music_charlotte_mp3_start;
    ramdisk_data[30].data = data_music_charlotte8bit_mp3_start;
    ramdisk_data[30].len = data_music_charlotte8bit_mp3_end - data_music_charlotte8bit_mp3_start;
    ramdisk_data[31].data = data_music_hiscore_wav_start;
    ramdisk_data[31].len = data_music_hiscore_wav_end - data_music_hiscore_wav_start;
    ramdisk_data[32].data = data_music_intro_wav_start;
    ramdisk_data[32].len = data_music_intro_wav_end - data_music_intro_wav_start;
    ramdisk_data[33].data = data_music_music_wav_start;
    ramdisk_data[33].len = data_music_music_wav_end - data_music_music_wav_start;
    ramdisk_data[34].data = data_sound_chomp_wav_start;
    ramdisk_data[34].len = data_sound_chomp_wav_end - data_sound_chomp_wav_start;
    ramdisk_data[35].data = data_sound_chomp2_wav_start;
    ramdisk_data[35].len = data_sound_chomp2_wav_end - data_sound_chomp2_wav_start;
    ramdisk_data[36].data = data_sound_click_wav_start;
    ramdisk_data[36].len = data_sound_click_wav_end - data_sound_click_wav_start;
    ramdisk_data[37].data = data_sound_doorbell_wav_start;
    ramdisk_data[37].len = data_sound_doorbell_wav_end - data_sound_doorbell_wav_start;
    ramdisk_data[38].data = data_sound_explosion_wav_start;
    ramdisk_data[38].len = data_sound_explosion_wav_end - data_sound_explosion_wav_start;
    ramdisk_data[39].data = data_sound_letter_wav_start;
    ramdisk_data[39].len = data_sound_letter_wav_end - data_sound_letter_wav_start;
    ramdisk_data[40].data = data_sound_level_end_wav_start;
    ramdisk_data[40].len = data_sound_level_end_wav_end - data_sound_level_end_wav_start;
    ramdisk_data[41].data = data_sound_paper_wav_start;
    ramdisk_data[41].len = data_sound_paper_wav_end - data_sound_paper_wav_start;
    ramdisk_data[42].data = data_sound_powerup_wav_start;
    ramdisk_data[42].len = data_sound_powerup_wav_end - data_sound_powerup_wav_start;
    ramdisk_data[43].data = data_sound_ratdeath_wav_start;
    ramdisk_data[43].len = data_sound_ratdeath_wav_end - data_sound_ratdeath_wav_start;
    ramdisk_data[44].data = data_sound_rocket_wav_start;
    ramdisk_data[44].len = data_sound_rocket_wav_end - data_sound_rocket_wav_start;
    ramdisk_data[45].data = data_sound_slime_wav_start;
    ramdisk_data[45].len = data_sound_slime_wav_end - data_sound_slime_wav_start;
    ramdisk_data[46].data = data_sound_spam_wav_start;
    ramdisk_data[46].len = data_sound_spam_wav_end - data_sound_spam_wav_start;
    ramdisk_data[47].data = data_sound_thief_wav_start;
    ramdisk_data[47].len = data_sound_thief_wav_end - data_sound_thief_wav_start;
    ramdisk_data[48].data = data_things__anim_eyes_session_start;
    ramdisk_data[48].len = data_things__anim_eyes_session_end - data_things__anim_eyes_session_start;
    ramdisk_data[49].data = data_things__anim_man_session_start;
    ramdisk_data[49].len = data_things__anim_man_session_end - data_things__anim_man_session_start;
    ramdisk_data[50].data = data_things__anim_treasure_chest_session_start;
    ramdisk_data[50].len = data_things__anim_treasure_chest_session_end - data_things__anim_treasure_chest_session_start;
    ramdisk_data[51].data = data_things__blood1_session_start;
    ramdisk_data[51].len = data_things__blood1_session_end - data_things__blood1_session_start;
    ramdisk_data[52].data = data_things__blood2_session_start;
    ramdisk_data[52].len = data_things__blood2_session_end - data_things__blood2_session_start;
    ramdisk_data[53].data = data_things__brazier_session_start;
    ramdisk_data[53].len = data_things__brazier_session_end - data_things__brazier_session_start;
    ramdisk_data[54].data = data_things__death_session_start;
    ramdisk_data[54].len = data_things__death_session_end - data_things__death_session_start;
    ramdisk_data[55].data = data_things__door1_session_start;
    ramdisk_data[55].len = data_things__door1_session_end - data_things__door1_session_start;
    ramdisk_data[56].data = data_things__elf_session_start;
    ramdisk_data[56].len = data_things__elf_session_end - data_things__elf_session_start;
    ramdisk_data[57].data = data_things__exit7_session_start;
    ramdisk_data[57].len = data_things__exit7_session_end - data_things__exit7_session_start;
    ramdisk_data[58].data = data_things__floor1_session_start;
    ramdisk_data[58].len = data_things__floor1_session_end - data_things__floor1_session_start;
    ramdisk_data[59].data = data_things__key_session_start;
    ramdisk_data[59].len = data_things__key_session_end - data_things__key_session_start;
    ramdisk_data[60].data = data_things__keys_session_start;
    ramdisk_data[60].len = data_things__keys_session_end - data_things__keys_session_start;
    ramdisk_data[61].data = data_things__plant_session_start;
    ramdisk_data[61].len = data_things__plant_session_end - data_things__plant_session_start;
    ramdisk_data[62].data = data_things__spam_session_start;
    ramdisk_data[62].len = data_things__spam_session_end - data_things__spam_session_start;
    ramdisk_data[63].data = data_things__sparks1_session_start;
    ramdisk_data[63].len = data_things__sparks1_session_end - data_things__sparks1_session_start;
    ramdisk_data[64].data = data_things__sparks2_session_start;
    ramdisk_data[64].len = data_things__sparks2_session_end - data_things__sparks2_session_start;
    ramdisk_data[65].data = data_things__torch_session_start;
    ramdisk_data[65].len = data_things__torch_session_end - data_things__torch_session_start;
    ramdisk_data[66].data = data_things__torches_session_start;
    ramdisk_data[66].len = data_things__torches_session_end - data_things__torches_session_start;
    ramdisk_data[67].data = data_things__wand_f_session_start;
    ramdisk_data[67].len = data_things__wand_f_session_end - data_things__wand_f_session_start;
    ramdisk_data[68].data = data_things__wand_fire_session_start;
    ramdisk_data[68].len = data_things__wand_fire_session_end - data_things__wand_fire_session_start;
    ramdisk_data[69].data = data_things__warrior_session_start;
    ramdisk_data[69].len = data_things__warrior_session_end - data_things__warrior_session_start;
    ramdisk_data[70].data = data_things__wizard_session_start;
    ramdisk_data[70].len = data_things__wizard_session_end - data_things__wizard_session_start;
    ramdisk_data[71].data = data_things_amulet1_start;
    ramdisk_data[71].len = data_things_amulet1_end - data_things_amulet1_start;
    ramdisk_data[72].data = data_things_anim_eyes_start;
    ramdisk_data[72].len = data_things_anim_eyes_end - data_things_anim_eyes_start;
    ramdisk_data[73].data = data_things_anim_man_start;
    ramdisk_data[73].len = data_things_anim_man_end - data_things_anim_man_start;
    ramdisk_data[74].data = data_things_anim_treasure_chest_start;
    ramdisk_data[74].len = data_things_anim_treasure_chest_end - data_things_anim_treasure_chest_start;
    ramdisk_data[75].data = data_things_apple_start;
    ramdisk_data[75].len = data_things_apple_end - data_things_apple_start;
    ramdisk_data[76].data = data_things_apple_poison_start;
    ramdisk_data[76].len = data_things_apple_poison_end - data_things_apple_poison_start;
    ramdisk_data[77].data = data_things_arrow_start;
    ramdisk_data[77].len = data_things_arrow_end - data_things_arrow_start;
    ramdisk_data[78].data = data_things_axe1_start;
    ramdisk_data[78].len = data_things_axe1_end - data_things_axe1_start;
    ramdisk_data[79].data = data_things_axe1_anim_start;
    ramdisk_data[79].len = data_things_axe1_anim_end - data_things_axe1_anim_start;
    ramdisk_data[80].data = data_things_axe2_start;
    ramdisk_data[80].len = data_things_axe2_end - data_things_axe2_start;
    ramdisk_data[81].data = data_things_axe2_anim_start;
    ramdisk_data[81].len = data_things_axe2_anim_end - data_things_axe2_anim_start;
    ramdisk_data[82].data = data_things_banana_start;
    ramdisk_data[82].len = data_things_banana_end - data_things_banana_start;
    ramdisk_data[83].data = data_things_blood1_start;
    ramdisk_data[83].len = data_things_blood1_end - data_things_blood1_start;
    ramdisk_data[84].data = data_things_blood2_start;
    ramdisk_data[84].len = data_things_blood2_end - data_things_blood2_start;
    ramdisk_data[85].data = data_things_bonepile1_start;
    ramdisk_data[85].len = data_things_bonepile1_end - data_things_bonepile1_start;
    ramdisk_data[86].data = data_things_bonepile2_start;
    ramdisk_data[86].len = data_things_bonepile2_end - data_things_bonepile2_start;
    ramdisk_data[87].data = data_things_bonepile3_start;
    ramdisk_data[87].len = data_things_bonepile3_end - data_things_bonepile3_start;
    ramdisk_data[88].data = data_things_bow1_start;
    ramdisk_data[88].len = data_things_bow1_end - data_things_bow1_start;
    ramdisk_data[89].data = data_things_bow1_anim_start;
    ramdisk_data[89].len = data_things_bow1_anim_end - data_things_bow1_anim_start;
    ramdisk_data[90].data = data_things_bow2_start;
    ramdisk_data[90].len = data_things_bow2_end - data_things_bow2_start;
    ramdisk_data[91].data = data_things_bow3_start;
    ramdisk_data[91].len = data_things_bow3_end - data_things_bow3_start;
    ramdisk_data[92].data = data_things_bow4_start;
    ramdisk_data[92].len = data_things_bow4_end - data_things_bow4_start;
    ramdisk_data[93].data = data_things_brazier_start;
    ramdisk_data[93].len = data_things_brazier_end - data_things_brazier_start;
    ramdisk_data[94].data = data_things_bread_start;
    ramdisk_data[94].len = data_things_bread_end - data_things_bread_start;
    ramdisk_data[95].data = data_things_cheese_start;
    ramdisk_data[95].len = data_things_cheese_end - data_things_cheese_start;
    ramdisk_data[96].data = data_things_chest1_start;
    ramdisk_data[96].len = data_things_chest1_end - data_things_chest1_start;
    ramdisk_data[97].data = data_things_cloudkill1_start;
    ramdisk_data[97].len = data_things_cloudkill1_end - data_things_cloudkill1_start;
    ramdisk_data[98].data = data_things_cloudkill2_start;
    ramdisk_data[98].len = data_things_cloudkill2_end - data_things_cloudkill2_start;
    ramdisk_data[99].data = data_things_cloudkill3_start;
    ramdisk_data[99].len = data_things_cloudkill3_end - data_things_cloudkill3_start;
    ramdisk_data[100].data = data_things_cloudkill4_start;
    ramdisk_data[100].len = data_things_cloudkill4_end - data_things_cloudkill4_start;
    ramdisk_data[101].data = data_things_coins1_start;
    ramdisk_data[101].len = data_things_coins1_end - data_things_coins1_start;
    ramdisk_data[102].data = data_things_death_start;
    ramdisk_data[102].len = data_things_death_end - data_things_death_start;
    ramdisk_data[103].data = data_things_door1_start;
    ramdisk_data[103].len = data_things_door1_end - data_things_door1_start;
    ramdisk_data[104].data = data_things_door2_start;
    ramdisk_data[104].len = data_things_door2_end - data_things_door2_start;
    ramdisk_data[105].data = data_things_dwarf_start;
    ramdisk_data[105].len = data_things_dwarf_end - data_things_dwarf_start;
    ramdisk_data[106].data = data_things_elf_start;
    ramdisk_data[106].len = data_things_elf_end - data_things_elf_start;
    ramdisk_data[107].data = data_things_exit1_start;
    ramdisk_data[107].len = data_things_exit1_end - data_things_exit1_start;
    ramdisk_data[108].data = data_things_exit2_start;
    ramdisk_data[108].len = data_things_exit2_end - data_things_exit2_start;
    ramdisk_data[109].data = data_things_exit3_start;
    ramdisk_data[109].len = data_things_exit3_end - data_things_exit3_start;
    ramdisk_data[110].data = data_things_exit4_start;
    ramdisk_data[110].len = data_things_exit4_end - data_things_exit4_start;
    ramdisk_data[111].data = data_things_exit5_start;
    ramdisk_data[111].len = data_things_exit5_end - data_things_exit5_start;
    ramdisk_data[112].data = data_things_exit6_start;
    ramdisk_data[112].len = data_things_exit6_end - data_things_exit6_start;
    ramdisk_data[113].data = data_things_exit7_start;
    ramdisk_data[113].len = data_things_exit7_end - data_things_exit7_start;
    ramdisk_data[114].data = data_things_explosion1_start;
    ramdisk_data[114].len = data_things_explosion1_end - data_things_explosion1_start;
    ramdisk_data[115].data = data_things_explosion2_start;
    ramdisk_data[115].len = data_things_explosion2_end - data_things_explosion2_start;
    ramdisk_data[116].data = data_things_explosion3_start;
    ramdisk_data[116].len = data_things_explosion3_end - data_things_explosion3_start;
    ramdisk_data[117].data = data_things_explosion4_start;
    ramdisk_data[117].len = data_things_explosion4_end - data_things_explosion4_start;
    ramdisk_data[118].data = data_things_fireball_start;
    ramdisk_data[118].len = data_things_fireball_end - data_things_fireball_start;
    ramdisk_data[119].data = data_things_floor1_start;
    ramdisk_data[119].len = data_things_floor1_end - data_things_floor1_start;
    ramdisk_data[120].data = data_things_floor2_start;
    ramdisk_data[120].len = data_things_floor2_end - data_things_floor2_start;
    ramdisk_data[121].data = data_things_floor3_start;
    ramdisk_data[121].len = data_things_floor3_end - data_things_floor3_start;
    ramdisk_data[122].data = data_things_floor4_start;
    ramdisk_data[122].len = data_things_floor4_end - data_things_floor4_start;
    ramdisk_data[123].data = data_things_floor5_start;
    ramdisk_data[123].len = data_things_floor5_end - data_things_floor5_start;
    ramdisk_data[124].data = data_things_floor6_start;
    ramdisk_data[124].len = data_things_floor6_end - data_things_floor6_start;
    ramdisk_data[125].data = data_things_gem1_start;
    ramdisk_data[125].len = data_things_gem1_end - data_things_gem1_start;
    ramdisk_data[126].data = data_things_gem2_start;
    ramdisk_data[126].len = data_things_gem2_end - data_things_gem2_start;
    ramdisk_data[127].data = data_things_gem3_start;
    ramdisk_data[127].len = data_things_gem3_end - data_things_gem3_start;
    ramdisk_data[128].data = data_things_gem4_start;
    ramdisk_data[128].len = data_things_gem4_end - data_things_gem4_start;
    ramdisk_data[129].data = data_things_gem5_start;
    ramdisk_data[129].len = data_things_gem5_end - data_things_gem5_start;
    ramdisk_data[130].data = data_things_gem6_start;
    ramdisk_data[130].len = data_things_gem6_end - data_things_gem6_start;
    ramdisk_data[131].data = data_things_gem7_start;
    ramdisk_data[131].len = data_things_gem7_end - data_things_gem7_start;
    ramdisk_data[132].data = data_things_generator1_start;
    ramdisk_data[132].len = data_things_generator1_end - data_things_generator1_start;
    ramdisk_data[133].data = data_things_generator5_start;
    ramdisk_data[133].len = data_things_generator5_end - data_things_generator5_start;
    ramdisk_data[134].data = data_things_generator6_start;
    ramdisk_data[134].len = data_things_generator6_end - data_things_generator6_start;
    ramdisk_data[135].data = data_things_generator7_start;
    ramdisk_data[135].len = data_things_generator7_end - data_things_generator7_start;
    ramdisk_data[136].data = data_things_ghost1_start;
    ramdisk_data[136].len = data_things_ghost1_end - data_things_ghost1_start;
    ramdisk_data[137].data = data_things_ghost2_start;
    ramdisk_data[137].len = data_things_ghost2_end - data_things_ghost2_start;
    ramdisk_data[138].data = data_things_ghost3_start;
    ramdisk_data[138].len = data_things_ghost3_end - data_things_ghost3_start;
    ramdisk_data[139].data = data_things_hit_miss_start;
    ramdisk_data[139].len = data_things_hit_miss_end - data_things_hit_miss_start;
    ramdisk_data[140].data = data_things_hit_success_start;
    ramdisk_data[140].len = data_things_hit_success_end - data_things_hit_success_start;
    ramdisk_data[141].data = data_things_key_start;
    ramdisk_data[141].len = data_things_key_end - data_things_key_start;
    ramdisk_data[142].data = data_things_keys2_start;
    ramdisk_data[142].len = data_things_keys2_end - data_things_keys2_start;
    ramdisk_data[143].data = data_things_keys3_start;
    ramdisk_data[143].len = data_things_keys3_end - data_things_keys3_start;
    ramdisk_data[144].data = data_things_noentry_start;
    ramdisk_data[144].len = data_things_noentry_end - data_things_noentry_start;
    ramdisk_data[145].data = data_things_pipe_start;
    ramdisk_data[145].len = data_things_pipe_end - data_things_pipe_start;
    ramdisk_data[146].data = data_things_poison1_start;
    ramdisk_data[146].len = data_things_poison1_end - data_things_poison1_start;
    ramdisk_data[147].data = data_things_poison2_start;
    ramdisk_data[147].len = data_things_poison2_end - data_things_poison2_start;
    ramdisk_data[148].data = data_things_poison3_start;
    ramdisk_data[148].len = data_things_poison3_end - data_things_poison3_start;
    ramdisk_data[149].data = data_things_poison4_start;
    ramdisk_data[149].len = data_things_poison4_end - data_things_poison4_start;
    ramdisk_data[150].data = data_things_potion_cloudkill_start;
    ramdisk_data[150].len = data_things_potion_cloudkill_end - data_things_potion_cloudkill_start;
    ramdisk_data[151].data = data_things_potion_fire_start;
    ramdisk_data[151].len = data_things_potion_fire_end - data_things_potion_fire_start;
    ramdisk_data[152].data = data_things_potion_life_start;
    ramdisk_data[152].len = data_things_potion_life_end - data_things_potion_life_start;
    ramdisk_data[153].data = data_things_potion_monsticide_start;
    ramdisk_data[153].len = data_things_potion_monsticide_end - data_things_potion_monsticide_start;
    ramdisk_data[154].data = data_things_potion_shield_start;
    ramdisk_data[154].len = data_things_potion_shield_end - data_things_potion_shield_start;
    ramdisk_data[155].data = data_things_ring1_start;
    ramdisk_data[155].len = data_things_ring1_end - data_things_ring1_start;
    ramdisk_data[156].data = data_things_ring2_start;
    ramdisk_data[156].len = data_things_ring2_end - data_things_ring2_start;
    ramdisk_data[157].data = data_things_ring3_start;
    ramdisk_data[157].len = data_things_ring3_end - data_things_ring3_start;
    ramdisk_data[158].data = data_things_rock1_start;
    ramdisk_data[158].len = data_things_rock1_end - data_things_rock1_start;
    ramdisk_data[159].data = data_things_scythe1_start;
    ramdisk_data[159].len = data_things_scythe1_end - data_things_scythe1_start;
    ramdisk_data[160].data = data_things_scythe1_anim_start;
    ramdisk_data[160].len = data_things_scythe1_anim_end - data_things_scythe1_anim_start;
    ramdisk_data[161].data = data_things_spam_start;
    ramdisk_data[161].len = data_things_spam_end - data_things_spam_start;
    ramdisk_data[162].data = data_things_sword1_start;
    ramdisk_data[162].len = data_things_sword1_end - data_things_sword1_start;
    ramdisk_data[163].data = data_things_sword1_anim_start;
    ramdisk_data[163].len = data_things_sword1_anim_end - data_things_sword1_anim_start;
    ramdisk_data[164].data = data_things_sword2_start;
    ramdisk_data[164].len = data_things_sword2_end - data_things_sword2_start;
    ramdisk_data[165].data = data_things_sword2_anim_start;
    ramdisk_data[165].len = data_things_sword2_anim_end - data_things_sword2_anim_start;
    ramdisk_data[166].data = data_things_sword3_start;
    ramdisk_data[166].len = data_things_sword3_end - data_things_sword3_start;
    ramdisk_data[167].data = data_things_sword3_anim_start;
    ramdisk_data[167].len = data_things_sword3_anim_end - data_things_sword3_anim_start;
    ramdisk_data[168].data = data_things_torch_start;
    ramdisk_data[168].len = data_things_torch_end - data_things_torch_start;
    ramdisk_data[169].data = data_things_torches_start;
    ramdisk_data[169].len = data_things_torches_end - data_things_torches_start;
    ramdisk_data[170].data = data_things_valkyrie_start;
    ramdisk_data[170].len = data_things_valkyrie_end - data_things_valkyrie_start;
    ramdisk_data[171].data = data_things_wall1_start;
    ramdisk_data[171].len = data_things_wall1_end - data_things_wall1_start;
    ramdisk_data[172].data = data_things_wall2_start;
    ramdisk_data[172].len = data_things_wall2_end - data_things_wall2_start;
    ramdisk_data[173].data = data_things_wall3_start;
    ramdisk_data[173].len = data_things_wall3_end - data_things_wall3_start;
    ramdisk_data[174].data = data_things_wall4_start;
    ramdisk_data[174].len = data_things_wall4_end - data_things_wall4_start;
    ramdisk_data[175].data = data_things_wall5_start;
    ramdisk_data[175].len = data_things_wall5_end - data_things_wall5_start;
    ramdisk_data[176].data = data_things_wall6_start;
    ramdisk_data[176].len = data_things_wall6_end - data_things_wall6_start;
    ramdisk_data[177].data = data_things_wand1_anim_start;
    ramdisk_data[177].len = data_things_wand1_anim_end - data_things_wand1_anim_start;
    ramdisk_data[178].data = data_things_wand_fire_start;
    ramdisk_data[178].len = data_things_wand_fire_end - data_things_wand_fire_start;
    ramdisk_data[179].data = data_things_warrior_start;
    ramdisk_data[179].len = data_things_warrior_end - data_things_warrior_start;
    ramdisk_data[180].data = data_things_water_start;
    ramdisk_data[180].len = data_things_water_end - data_things_water_start;
    ramdisk_data[181].data = data_things_water_poison_start;
    ramdisk_data[181].len = data_things_water_poison_end - data_things_water_poison_start;
    ramdisk_data[182].data = data_things_weapon_swing1_start;
    ramdisk_data[182].len = data_things_weapon_swing1_end - data_things_weapon_swing1_start;
    ramdisk_data[183].data = data_things_wizard_start;
    ramdisk_data[183].len = data_things_wizard_end - data_things_wizard_start;
    ramdisk_data[184].data = data_ttf_emulogic_gif_start;
    ramdisk_data[184].len = data_ttf_emulogic_gif_end - data_ttf_emulogic_gif_start;
    ramdisk_data[185].data = data_ttf_emulogic_ttf_pointsize10_data_start;
    ramdisk_data[185].len = data_ttf_emulogic_ttf_pointsize10_data_end - data_ttf_emulogic_ttf_pointsize10_data_start;
    ramdisk_data[186].data = data_ttf_emulogic_ttf_pointsize10_tga_start;
    ramdisk_data[186].len = data_ttf_emulogic_ttf_pointsize10_tga_end - data_ttf_emulogic_ttf_pointsize10_tga_start;
    ramdisk_data[187].data = data_ttf_emulogic_ttf_pointsize11_data_start;
    ramdisk_data[187].len = data_ttf_emulogic_ttf_pointsize11_data_end - data_ttf_emulogic_ttf_pointsize11_data_start;
    ramdisk_data[188].data = data_ttf_emulogic_ttf_pointsize11_tga_start;
    ramdisk_data[188].len = data_ttf_emulogic_ttf_pointsize11_tga_end - data_ttf_emulogic_ttf_pointsize11_tga_start;
    ramdisk_data[189].data = data_ttf_emulogic_ttf_pointsize12_data_start;
    ramdisk_data[189].len = data_ttf_emulogic_ttf_pointsize12_data_end - data_ttf_emulogic_ttf_pointsize12_data_start;
    ramdisk_data[190].data = data_ttf_emulogic_ttf_pointsize12_tga_start;
    ramdisk_data[190].len = data_ttf_emulogic_ttf_pointsize12_tga_end - data_ttf_emulogic_ttf_pointsize12_tga_start;
    ramdisk_data[191].data = data_ttf_emulogic_ttf_pointsize14_data_start;
    ramdisk_data[191].len = data_ttf_emulogic_ttf_pointsize14_data_end - data_ttf_emulogic_ttf_pointsize14_data_start;
    ramdisk_data[192].data = data_ttf_emulogic_ttf_pointsize14_tga_start;
    ramdisk_data[192].len = data_ttf_emulogic_ttf_pointsize14_tga_end - data_ttf_emulogic_ttf_pointsize14_tga_start;
    ramdisk_data[193].data = data_ttf_emulogic_ttf_pointsize15_data_start;
    ramdisk_data[193].len = data_ttf_emulogic_ttf_pointsize15_data_end - data_ttf_emulogic_ttf_pointsize15_data_start;
    ramdisk_data[194].data = data_ttf_emulogic_ttf_pointsize15_tga_start;
    ramdisk_data[194].len = data_ttf_emulogic_ttf_pointsize15_tga_end - data_ttf_emulogic_ttf_pointsize15_tga_start;
    ramdisk_data[195].data = data_ttf_emulogic_ttf_pointsize16_data_start;
    ramdisk_data[195].len = data_ttf_emulogic_ttf_pointsize16_data_end - data_ttf_emulogic_ttf_pointsize16_data_start;
    ramdisk_data[196].data = data_ttf_emulogic_ttf_pointsize16_tga_start;
    ramdisk_data[196].len = data_ttf_emulogic_ttf_pointsize16_tga_end - data_ttf_emulogic_ttf_pointsize16_tga_start;
    ramdisk_data[197].data = data_ttf_emulogic_ttf_pointsize17_data_start;
    ramdisk_data[197].len = data_ttf_emulogic_ttf_pointsize17_data_end - data_ttf_emulogic_ttf_pointsize17_data_start;
    ramdisk_data[198].data = data_ttf_emulogic_ttf_pointsize17_tga_start;
    ramdisk_data[198].len = data_ttf_emulogic_ttf_pointsize17_tga_end - data_ttf_emulogic_ttf_pointsize17_tga_start;
    ramdisk_data[199].data = data_ttf_emulogic_ttf_pointsize18_data_start;
    ramdisk_data[199].len = data_ttf_emulogic_ttf_pointsize18_data_end - data_ttf_emulogic_ttf_pointsize18_data_start;
    ramdisk_data[200].data = data_ttf_emulogic_ttf_pointsize18_tga_start;
    ramdisk_data[200].len = data_ttf_emulogic_ttf_pointsize18_tga_end - data_ttf_emulogic_ttf_pointsize18_tga_start;
    ramdisk_data[201].data = data_ttf_emulogic_ttf_pointsize19_data_start;
    ramdisk_data[201].len = data_ttf_emulogic_ttf_pointsize19_data_end - data_ttf_emulogic_ttf_pointsize19_data_start;
    ramdisk_data[202].data = data_ttf_emulogic_ttf_pointsize19_tga_start;
    ramdisk_data[202].len = data_ttf_emulogic_ttf_pointsize19_tga_end - data_ttf_emulogic_ttf_pointsize19_tga_start;
    ramdisk_data[203].data = data_ttf_emulogic_ttf_pointsize20_data_start;
    ramdisk_data[203].len = data_ttf_emulogic_ttf_pointsize20_data_end - data_ttf_emulogic_ttf_pointsize20_data_start;
    ramdisk_data[204].data = data_ttf_emulogic_ttf_pointsize20_tga_start;
    ramdisk_data[204].len = data_ttf_emulogic_ttf_pointsize20_tga_end - data_ttf_emulogic_ttf_pointsize20_tga_start;
    ramdisk_data[205].data = data_ttf_emulogic_ttf_pointsize21_data_start;
    ramdisk_data[205].len = data_ttf_emulogic_ttf_pointsize21_data_end - data_ttf_emulogic_ttf_pointsize21_data_start;
    ramdisk_data[206].data = data_ttf_emulogic_ttf_pointsize21_tga_start;
    ramdisk_data[206].len = data_ttf_emulogic_ttf_pointsize21_tga_end - data_ttf_emulogic_ttf_pointsize21_tga_start;
    ramdisk_data[207].data = data_ttf_emulogic_ttf_pointsize22_data_start;
    ramdisk_data[207].len = data_ttf_emulogic_ttf_pointsize22_data_end - data_ttf_emulogic_ttf_pointsize22_data_start;
    ramdisk_data[208].data = data_ttf_emulogic_ttf_pointsize22_tga_start;
    ramdisk_data[208].len = data_ttf_emulogic_ttf_pointsize22_tga_end - data_ttf_emulogic_ttf_pointsize22_tga_start;
    ramdisk_data[209].data = data_ttf_emulogic_ttf_pointsize23_data_start;
    ramdisk_data[209].len = data_ttf_emulogic_ttf_pointsize23_data_end - data_ttf_emulogic_ttf_pointsize23_data_start;
    ramdisk_data[210].data = data_ttf_emulogic_ttf_pointsize23_tga_start;
    ramdisk_data[210].len = data_ttf_emulogic_ttf_pointsize23_tga_end - data_ttf_emulogic_ttf_pointsize23_tga_start;
    ramdisk_data[211].data = data_ttf_emulogic_ttf_pointsize24_data_start;
    ramdisk_data[211].len = data_ttf_emulogic_ttf_pointsize24_data_end - data_ttf_emulogic_ttf_pointsize24_data_start;
    ramdisk_data[212].data = data_ttf_emulogic_ttf_pointsize24_tga_start;
    ramdisk_data[212].len = data_ttf_emulogic_ttf_pointsize24_tga_end - data_ttf_emulogic_ttf_pointsize24_tga_start;
    ramdisk_data[213].data = data_ttf_emulogic_ttf_pointsize25_data_start;
    ramdisk_data[213].len = data_ttf_emulogic_ttf_pointsize25_data_end - data_ttf_emulogic_ttf_pointsize25_data_start;
    ramdisk_data[214].data = data_ttf_emulogic_ttf_pointsize25_tga_start;
    ramdisk_data[214].len = data_ttf_emulogic_ttf_pointsize25_tga_end - data_ttf_emulogic_ttf_pointsize25_tga_start;
    ramdisk_data[215].data = data_ttf_emulogic_ttf_pointsize26_data_start;
    ramdisk_data[215].len = data_ttf_emulogic_ttf_pointsize26_data_end - data_ttf_emulogic_ttf_pointsize26_data_start;
    ramdisk_data[216].data = data_ttf_emulogic_ttf_pointsize26_tga_start;
    ramdisk_data[216].len = data_ttf_emulogic_ttf_pointsize26_tga_end - data_ttf_emulogic_ttf_pointsize26_tga_start;
    ramdisk_data[217].data = data_ttf_emulogic_ttf_pointsize28_data_start;
    ramdisk_data[217].len = data_ttf_emulogic_ttf_pointsize28_data_end - data_ttf_emulogic_ttf_pointsize28_data_start;
    ramdisk_data[218].data = data_ttf_emulogic_ttf_pointsize28_tga_start;
    ramdisk_data[218].len = data_ttf_emulogic_ttf_pointsize28_tga_end - data_ttf_emulogic_ttf_pointsize28_tga_start;
    ramdisk_data[219].data = data_ttf_emulogic_ttf_pointsize30_data_start;
    ramdisk_data[219].len = data_ttf_emulogic_ttf_pointsize30_data_end - data_ttf_emulogic_ttf_pointsize30_data_start;
    ramdisk_data[220].data = data_ttf_emulogic_ttf_pointsize30_tga_start;
    ramdisk_data[220].len = data_ttf_emulogic_ttf_pointsize30_tga_end - data_ttf_emulogic_ttf_pointsize30_tga_start;
    ramdisk_data[221].data = data_ttf_emulogic_ttf_pointsize32_data_start;
    ramdisk_data[221].len = data_ttf_emulogic_ttf_pointsize32_data_end - data_ttf_emulogic_ttf_pointsize32_data_start;
    ramdisk_data[222].data = data_ttf_emulogic_ttf_pointsize32_tga_start;
    ramdisk_data[222].len = data_ttf_emulogic_ttf_pointsize32_tga_end - data_ttf_emulogic_ttf_pointsize32_tga_start;
    ramdisk_data[223].data = data_ttf_emulogic_ttf_pointsize36_data_start;
    ramdisk_data[223].len = data_ttf_emulogic_ttf_pointsize36_data_end - data_ttf_emulogic_ttf_pointsize36_data_start;
    ramdisk_data[224].data = data_ttf_emulogic_ttf_pointsize36_tga_start;
    ramdisk_data[224].len = data_ttf_emulogic_ttf_pointsize36_tga_end - data_ttf_emulogic_ttf_pointsize36_tga_start;
    ramdisk_data[225].data = data_ttf_emulogic_ttf_pointsize6_data_start;
    ramdisk_data[225].len = data_ttf_emulogic_ttf_pointsize6_data_end - data_ttf_emulogic_ttf_pointsize6_data_start;
    ramdisk_data[226].data = data_ttf_emulogic_ttf_pointsize6_tga_start;
    ramdisk_data[226].len = data_ttf_emulogic_ttf_pointsize6_tga_end - data_ttf_emulogic_ttf_pointsize6_tga_start;
    ramdisk_data[227].data = data_ttf_emulogic_ttf_pointsize7_data_start;
    ramdisk_data[227].len = data_ttf_emulogic_ttf_pointsize7_data_end - data_ttf_emulogic_ttf_pointsize7_data_start;
    ramdisk_data[228].data = data_ttf_emulogic_ttf_pointsize7_tga_start;
    ramdisk_data[228].len = data_ttf_emulogic_ttf_pointsize7_tga_end - data_ttf_emulogic_ttf_pointsize7_tga_start;
    ramdisk_data[229].data = data_ttf_emulogic_ttf_pointsize8_data_start;
    ramdisk_data[229].len = data_ttf_emulogic_ttf_pointsize8_data_end - data_ttf_emulogic_ttf_pointsize8_data_start;
    ramdisk_data[230].data = data_ttf_emulogic_ttf_pointsize8_tga_start;
    ramdisk_data[230].len = data_ttf_emulogic_ttf_pointsize8_tga_end - data_ttf_emulogic_ttf_pointsize8_tga_start;
    ramdisk_data[231].data = data_ttf_emulogic_ttf_pointsize9_data_start;
    ramdisk_data[231].len = data_ttf_emulogic_ttf_pointsize9_data_end - data_ttf_emulogic_ttf_pointsize9_data_start;
    ramdisk_data[232].data = data_ttf_emulogic_ttf_pointsize9_tga_start;
    ramdisk_data[232].len = data_ttf_emulogic_ttf_pointsize9_tga_end - data_ttf_emulogic_ttf_pointsize9_tga_start;
    ramdisk_data[233].data = data_ttf_LICENSE_txt_start;
    ramdisk_data[233].len = data_ttf_LICENSE_txt_end - data_ttf_LICENSE_txt_start;
    ramdisk_data[234].data = data_ttf_Readme_txt_start;
    ramdisk_data[234].len = data_ttf_Readme_txt_end - data_ttf_Readme_txt_start;
}
