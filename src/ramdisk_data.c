#include <stdint.h>
#include "ramdisk.h"

extern unsigned char *data__player_session_start_ asm("data__player_session_start_");
extern unsigned char *data__player_session_end_ asm("data__player_session_end_");
static const unsigned char *const data__player_session_start =
    (const unsigned char *const) (char*)&data__player_session_start_;
static const unsigned char *const data__player_session_end   =
    (const unsigned char *const) (char*)&data__player_session_end_;

extern unsigned char *data_gfx_dragon_png_start_ asm("data_gfx_dragon_png_start_");
extern unsigned char *data_gfx_dragon_png_end_ asm("data_gfx_dragon_png_end_");
static const unsigned char *const data_gfx_dragon_png_start =
    (const unsigned char *const) (char*)&data_gfx_dragon_png_start_;
static const unsigned char *const data_gfx_dragon_png_end   =
    (const unsigned char *const) (char*)&data_gfx_dragon_png_end_;

extern unsigned char *data_gfx_gorynlich_png_start_ asm("data_gfx_gorynlich_png_start_");
extern unsigned char *data_gfx_gorynlich_png_end_ asm("data_gfx_gorynlich_png_end_");
static const unsigned char *const data_gfx_gorynlich_png_start =
    (const unsigned char *const) (char*)&data_gfx_gorynlich_png_start_;
static const unsigned char *const data_gfx_gorynlich_png_end   =
    (const unsigned char *const) (char*)&data_gfx_gorynlich_png_end_;

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

extern unsigned char *data_gfx_rat_png_start_ asm("data_gfx_rat_png_start_");
extern unsigned char *data_gfx_rat_png_end_ asm("data_gfx_rat_png_end_");
static const unsigned char *const data_gfx_rat_png_start =
    (const unsigned char *const) (char*)&data_gfx_rat_png_start_;
static const unsigned char *const data_gfx_rat_png_end   =
    (const unsigned char *const) (char*)&data_gfx_rat_png_end_;

extern unsigned char *data_gfx_shell_png_start_ asm("data_gfx_shell_png_start_");
extern unsigned char *data_gfx_shell_png_end_ asm("data_gfx_shell_png_end_");
static const unsigned char *const data_gfx_shell_png_start =
    (const unsigned char *const) (char*)&data_gfx_shell_png_start_;
static const unsigned char *const data_gfx_shell_png_end   =
    (const unsigned char *const) (char*)&data_gfx_shell_png_end_;

extern unsigned char *data_gfx_sprites_small_png_start_ asm("data_gfx_sprites_small_png_start_");
extern unsigned char *data_gfx_sprites_small_png_end_ asm("data_gfx_sprites_small_png_end_");
static const unsigned char *const data_gfx_sprites_small_png_start =
    (const unsigned char *const) (char*)&data_gfx_sprites_small_png_start_;
static const unsigned char *const data_gfx_sprites_small_png_end   =
    (const unsigned char *const) (char*)&data_gfx_sprites_small_png_end_;

extern unsigned char *data_gfx_sprites_small_xcf_start_ asm("data_gfx_sprites_small_xcf_start_");
extern unsigned char *data_gfx_sprites_small_xcf_end_ asm("data_gfx_sprites_small_xcf_end_");
static const unsigned char *const data_gfx_sprites_small_xcf_start =
    (const unsigned char *const) (char*)&data_gfx_sprites_small_xcf_start_;
static const unsigned char *const data_gfx_sprites_small_xcf_end   =
    (const unsigned char *const) (char*)&data_gfx_sprites_small_xcf_end_;

extern unsigned char *data_gfx_stars_png_start_ asm("data_gfx_stars_png_start_");
extern unsigned char *data_gfx_stars_png_end_ asm("data_gfx_stars_png_end_");
static const unsigned char *const data_gfx_stars_png_start =
    (const unsigned char *const) (char*)&data_gfx_stars_png_start_;
static const unsigned char *const data_gfx_stars_png_end   =
    (const unsigned char *const) (char*)&data_gfx_stars_png_end_;

extern unsigned char *data_gfx_title_png_start_ asm("data_gfx_title_png_start_");
extern unsigned char *data_gfx_title_png_end_ asm("data_gfx_title_png_end_");
static const unsigned char *const data_gfx_title_png_start =
    (const unsigned char *const) (char*)&data_gfx_title_png_start_;
static const unsigned char *const data_gfx_title_png_end   =
    (const unsigned char *const) (char*)&data_gfx_title_png_end_;

extern unsigned char *data_map_jigsaw_map_start_ asm("data_map_jigsaw_map_start_");
extern unsigned char *data_map_jigsaw_map_end_ asm("data_map_jigsaw_map_end_");
static const unsigned char *const data_map_jigsaw_map_start =
    (const unsigned char *const) (char*)&data_map_jigsaw_map_start_;
static const unsigned char *const data_map_jigsaw_map_end   =
    (const unsigned char *const) (char*)&data_map_jigsaw_map_end_;

extern unsigned char *data_map_map_light_data_start_ asm("data_map_map_light_data_start_");
extern unsigned char *data_map_map_light_data_end_ asm("data_map_map_light_data_end_");
static const unsigned char *const data_map_map_light_data_start =
    (const unsigned char *const) (char*)&data_map_map_light_data_start_;
static const unsigned char *const data_map_map_light_data_end   =
    (const unsigned char *const) (char*)&data_map_map_light_data_end_;

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

extern unsigned char *data_things__amulet1_session_start_ asm("data_things__amulet1_session_start_");
extern unsigned char *data_things__amulet1_session_end_ asm("data_things__amulet1_session_end_");
static const unsigned char *const data_things__amulet1_session_start =
    (const unsigned char *const) (char*)&data_things__amulet1_session_start_;
static const unsigned char *const data_things__amulet1_session_end   =
    (const unsigned char *const) (char*)&data_things__amulet1_session_end_;

extern unsigned char *data_things__chest1_session_start_ asm("data_things__chest1_session_start_");
extern unsigned char *data_things__chest1_session_end_ asm("data_things__chest1_session_end_");
static const unsigned char *const data_things__chest1_session_start =
    (const unsigned char *const) (char*)&data_things__chest1_session_start_;
static const unsigned char *const data_things__chest1_session_end   =
    (const unsigned char *const) (char*)&data_things__chest1_session_end_;

extern unsigned char *data_things__door_session_start_ asm("data_things__door_session_start_");
extern unsigned char *data_things__door_session_end_ asm("data_things__door_session_end_");
static const unsigned char *const data_things__door_session_start =
    (const unsigned char *const) (char*)&data_things__door_session_start_;
static const unsigned char *const data_things__door_session_end   =
    (const unsigned char *const) (char*)&data_things__door_session_end_;

extern unsigned char *data_things__floor_session_start_ asm("data_things__floor_session_start_");
extern unsigned char *data_things__floor_session_end_ asm("data_things__floor_session_end_");
static const unsigned char *const data_things__floor_session_start =
    (const unsigned char *const) (char*)&data_things__floor_session_start_;
static const unsigned char *const data_things__floor_session_end   =
    (const unsigned char *const) (char*)&data_things__floor_session_end_;

extern unsigned char *data_things__floor1_session_start_ asm("data_things__floor1_session_start_");
extern unsigned char *data_things__floor1_session_end_ asm("data_things__floor1_session_end_");
static const unsigned char *const data_things__floor1_session_start =
    (const unsigned char *const) (char*)&data_things__floor1_session_start_;
static const unsigned char *const data_things__floor1_session_end   =
    (const unsigned char *const) (char*)&data_things__floor1_session_end_;

extern unsigned char *data_things__floor2_session_start_ asm("data_things__floor2_session_start_");
extern unsigned char *data_things__floor2_session_end_ asm("data_things__floor2_session_end_");
static const unsigned char *const data_things__floor2_session_start =
    (const unsigned char *const) (char*)&data_things__floor2_session_start_;
static const unsigned char *const data_things__floor2_session_end   =
    (const unsigned char *const) (char*)&data_things__floor2_session_end_;

extern unsigned char *data_things__floor3_session_start_ asm("data_things__floor3_session_start_");
extern unsigned char *data_things__floor3_session_end_ asm("data_things__floor3_session_end_");
static const unsigned char *const data_things__floor3_session_start =
    (const unsigned char *const) (char*)&data_things__floor3_session_start_;
static const unsigned char *const data_things__floor3_session_end   =
    (const unsigned char *const) (char*)&data_things__floor3_session_end_;

extern unsigned char *data_things__floor4_session_start_ asm("data_things__floor4_session_start_");
extern unsigned char *data_things__floor4_session_end_ asm("data_things__floor4_session_end_");
static const unsigned char *const data_things__floor4_session_start =
    (const unsigned char *const) (char*)&data_things__floor4_session_start_;
static const unsigned char *const data_things__floor4_session_end   =
    (const unsigned char *const) (char*)&data_things__floor4_session_end_;

extern unsigned char *data_things__floor5_session_start_ asm("data_things__floor5_session_start_");
extern unsigned char *data_things__floor5_session_end_ asm("data_things__floor5_session_end_");
static const unsigned char *const data_things__floor5_session_start =
    (const unsigned char *const) (char*)&data_things__floor5_session_start_;
static const unsigned char *const data_things__floor5_session_end   =
    (const unsigned char *const) (char*)&data_things__floor5_session_end_;

extern unsigned char *data_things__floor6_session_start_ asm("data_things__floor6_session_start_");
extern unsigned char *data_things__floor6_session_end_ asm("data_things__floor6_session_end_");
static const unsigned char *const data_things__floor6_session_start =
    (const unsigned char *const) (char*)&data_things__floor6_session_start_;
static const unsigned char *const data_things__floor6_session_end   =
    (const unsigned char *const) (char*)&data_things__floor6_session_end_;

extern unsigned char *data_things__food_session_start_ asm("data_things__food_session_start_");
extern unsigned char *data_things__food_session_end_ asm("data_things__food_session_end_");
static const unsigned char *const data_things__food_session_start =
    (const unsigned char *const) (char*)&data_things__food_session_start_;
static const unsigned char *const data_things__food_session_end   =
    (const unsigned char *const) (char*)&data_things__food_session_end_;

extern unsigned char *data_things__food2_session_start_ asm("data_things__food2_session_start_");
extern unsigned char *data_things__food2_session_end_ asm("data_things__food2_session_end_");
static const unsigned char *const data_things__food2_session_start =
    (const unsigned char *const) (char*)&data_things__food2_session_start_;
static const unsigned char *const data_things__food2_session_end   =
    (const unsigned char *const) (char*)&data_things__food2_session_end_;

extern unsigned char *data_things__gem1_session_start_ asm("data_things__gem1_session_start_");
extern unsigned char *data_things__gem1_session_end_ asm("data_things__gem1_session_end_");
static const unsigned char *const data_things__gem1_session_start =
    (const unsigned char *const) (char*)&data_things__gem1_session_start_;
static const unsigned char *const data_things__gem1_session_end   =
    (const unsigned char *const) (char*)&data_things__gem1_session_end_;

extern unsigned char *data_things__gem7_session_start_ asm("data_things__gem7_session_start_");
extern unsigned char *data_things__gem7_session_end_ asm("data_things__gem7_session_end_");
static const unsigned char *const data_things__gem7_session_start =
    (const unsigned char *const) (char*)&data_things__gem7_session_start_;
static const unsigned char *const data_things__gem7_session_end   =
    (const unsigned char *const) (char*)&data_things__gem7_session_end_;

extern unsigned char *data_things__generator1_session_start_ asm("data_things__generator1_session_start_");
extern unsigned char *data_things__generator1_session_end_ asm("data_things__generator1_session_end_");
static const unsigned char *const data_things__generator1_session_start =
    (const unsigned char *const) (char*)&data_things__generator1_session_start_;
static const unsigned char *const data_things__generator1_session_end   =
    (const unsigned char *const) (char*)&data_things__generator1_session_end_;

extern unsigned char *data_things__generator7_session_start_ asm("data_things__generator7_session_start_");
extern unsigned char *data_things__generator7_session_end_ asm("data_things__generator7_session_end_");
static const unsigned char *const data_things__generator7_session_start =
    (const unsigned char *const) (char*)&data_things__generator7_session_start_;
static const unsigned char *const data_things__generator7_session_end   =
    (const unsigned char *const) (char*)&data_things__generator7_session_end_;

extern unsigned char *data_things__keys1_session_start_ asm("data_things__keys1_session_start_");
extern unsigned char *data_things__keys1_session_end_ asm("data_things__keys1_session_end_");
static const unsigned char *const data_things__keys1_session_start =
    (const unsigned char *const) (char*)&data_things__keys1_session_start_;
static const unsigned char *const data_things__keys1_session_end   =
    (const unsigned char *const) (char*)&data_things__keys1_session_end_;

extern unsigned char *data_things__keys2_session_start_ asm("data_things__keys2_session_start_");
extern unsigned char *data_things__keys2_session_end_ asm("data_things__keys2_session_end_");
static const unsigned char *const data_things__keys2_session_start =
    (const unsigned char *const) (char*)&data_things__keys2_session_start_;
static const unsigned char *const data_things__keys2_session_end   =
    (const unsigned char *const) (char*)&data_things__keys2_session_end_;

extern unsigned char *data_things__keys3_session_start_ asm("data_things__keys3_session_start_");
extern unsigned char *data_things__keys3_session_end_ asm("data_things__keys3_session_end_");
static const unsigned char *const data_things__keys3_session_start =
    (const unsigned char *const) (char*)&data_things__keys3_session_start_;
static const unsigned char *const data_things__keys3_session_end   =
    (const unsigned char *const) (char*)&data_things__keys3_session_end_;

extern unsigned char *data_things__mask1_session_start_ asm("data_things__mask1_session_start_");
extern unsigned char *data_things__mask1_session_end_ asm("data_things__mask1_session_end_");
static const unsigned char *const data_things__mask1_session_start =
    (const unsigned char *const) (char*)&data_things__mask1_session_start_;
static const unsigned char *const data_things__mask1_session_end   =
    (const unsigned char *const) (char*)&data_things__mask1_session_end_;

extern unsigned char *data_things__mask3_session_start_ asm("data_things__mask3_session_start_");
extern unsigned char *data_things__mask3_session_end_ asm("data_things__mask3_session_end_");
static const unsigned char *const data_things__mask3_session_start =
    (const unsigned char *const) (char*)&data_things__mask3_session_start_;
static const unsigned char *const data_things__mask3_session_end   =
    (const unsigned char *const) (char*)&data_things__mask3_session_end_;

extern unsigned char *data_things__potion1_session_start_ asm("data_things__potion1_session_start_");
extern unsigned char *data_things__potion1_session_end_ asm("data_things__potion1_session_end_");
static const unsigned char *const data_things__potion1_session_start =
    (const unsigned char *const) (char*)&data_things__potion1_session_start_;
static const unsigned char *const data_things__potion1_session_end   =
    (const unsigned char *const) (char*)&data_things__potion1_session_end_;

extern unsigned char *data_things__potion5_session_start_ asm("data_things__potion5_session_start_");
extern unsigned char *data_things__potion5_session_end_ asm("data_things__potion5_session_end_");
static const unsigned char *const data_things__potion5_session_start =
    (const unsigned char *const) (char*)&data_things__potion5_session_start_;
static const unsigned char *const data_things__potion5_session_end   =
    (const unsigned char *const) (char*)&data_things__potion5_session_end_;

extern unsigned char *data_things__rock_0_session_start_ asm("data_things__rock_0_session_start_");
extern unsigned char *data_things__rock_0_session_end_ asm("data_things__rock_0_session_end_");
static const unsigned char *const data_things__rock_0_session_start =
    (const unsigned char *const) (char*)&data_things__rock_0_session_start_;
static const unsigned char *const data_things__rock_0_session_end   =
    (const unsigned char *const) (char*)&data_things__rock_0_session_end_;

extern unsigned char *data_things__rock_1_session_start_ asm("data_things__rock_1_session_start_");
extern unsigned char *data_things__rock_1_session_end_ asm("data_things__rock_1_session_end_");
static const unsigned char *const data_things__rock_1_session_start =
    (const unsigned char *const) (char*)&data_things__rock_1_session_start_;
static const unsigned char *const data_things__rock_1_session_end   =
    (const unsigned char *const) (char*)&data_things__rock_1_session_end_;

extern unsigned char *data_things__treasure_session_start_ asm("data_things__treasure_session_start_");
extern unsigned char *data_things__treasure_session_end_ asm("data_things__treasure_session_end_");
static const unsigned char *const data_things__treasure_session_start =
    (const unsigned char *const) (char*)&data_things__treasure_session_start_;
static const unsigned char *const data_things__treasure_session_end   =
    (const unsigned char *const) (char*)&data_things__treasure_session_end_;

extern unsigned char *data_things__wall_session_start_ asm("data_things__wall_session_start_");
extern unsigned char *data_things__wall_session_end_ asm("data_things__wall_session_end_");
static const unsigned char *const data_things__wall_session_start =
    (const unsigned char *const) (char*)&data_things__wall_session_start_;
static const unsigned char *const data_things__wall_session_end   =
    (const unsigned char *const) (char*)&data_things__wall_session_end_;

extern unsigned char *data_things__wall2_session_start_ asm("data_things__wall2_session_start_");
extern unsigned char *data_things__wall2_session_end_ asm("data_things__wall2_session_end_");
static const unsigned char *const data_things__wall2_session_start =
    (const unsigned char *const) (char*)&data_things__wall2_session_start_;
static const unsigned char *const data_things__wall2_session_end   =
    (const unsigned char *const) (char*)&data_things__wall2_session_end_;

extern unsigned char *data_things__water1_session_start_ asm("data_things__water1_session_start_");
extern unsigned char *data_things__water1_session_end_ asm("data_things__water1_session_end_");
static const unsigned char *const data_things__water1_session_start =
    (const unsigned char *const) (char*)&data_things__water1_session_start_;
static const unsigned char *const data_things__water1_session_end   =
    (const unsigned char *const) (char*)&data_things__water1_session_end_;

extern unsigned char *data_things_amulet1_start_ asm("data_things_amulet1_start_");
extern unsigned char *data_things_amulet1_end_ asm("data_things_amulet1_end_");
static const unsigned char *const data_things_amulet1_start =
    (const unsigned char *const) (char*)&data_things_amulet1_start_;
static const unsigned char *const data_things_amulet1_end   =
    (const unsigned char *const) (char*)&data_things_amulet1_end_;

extern unsigned char *data_things_bomb_start_ asm("data_things_bomb_start_");
extern unsigned char *data_things_bomb_end_ asm("data_things_bomb_end_");
static const unsigned char *const data_things_bomb_start =
    (const unsigned char *const) (char*)&data_things_bomb_start_;
static const unsigned char *const data_things_bomb_end   =
    (const unsigned char *const) (char*)&data_things_bomb_end_;

extern unsigned char *data_things_chest1_start_ asm("data_things_chest1_start_");
extern unsigned char *data_things_chest1_end_ asm("data_things_chest1_end_");
static const unsigned char *const data_things_chest1_start =
    (const unsigned char *const) (char*)&data_things_chest1_start_;
static const unsigned char *const data_things_chest1_end   =
    (const unsigned char *const) (char*)&data_things_chest1_end_;

extern unsigned char *data_things_coins1_start_ asm("data_things_coins1_start_");
extern unsigned char *data_things_coins1_end_ asm("data_things_coins1_end_");
static const unsigned char *const data_things_coins1_start =
    (const unsigned char *const) (char*)&data_things_coins1_start_;
static const unsigned char *const data_things_coins1_end   =
    (const unsigned char *const) (char*)&data_things_coins1_end_;

extern unsigned char *data_things_door_start_ asm("data_things_door_start_");
extern unsigned char *data_things_door_end_ asm("data_things_door_end_");
static const unsigned char *const data_things_door_start =
    (const unsigned char *const) (char*)&data_things_door_start_;
static const unsigned char *const data_things_door_end   =
    (const unsigned char *const) (char*)&data_things_door_end_;

extern unsigned char *data_things_exit_start_ asm("data_things_exit_start_");
extern unsigned char *data_things_exit_end_ asm("data_things_exit_end_");
static const unsigned char *const data_things_exit_start =
    (const unsigned char *const) (char*)&data_things_exit_start_;
static const unsigned char *const data_things_exit_end   =
    (const unsigned char *const) (char*)&data_things_exit_end_;

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

extern unsigned char *data_things_explosion_start_ asm("data_things_explosion_start_");
extern unsigned char *data_things_explosion_end_ asm("data_things_explosion_end_");
static const unsigned char *const data_things_explosion_start =
    (const unsigned char *const) (char*)&data_things_explosion_start_;
static const unsigned char *const data_things_explosion_end   =
    (const unsigned char *const) (char*)&data_things_explosion_end_;

extern unsigned char *data_things_floor_start_ asm("data_things_floor_start_");
extern unsigned char *data_things_floor_end_ asm("data_things_floor_end_");
static const unsigned char *const data_things_floor_start =
    (const unsigned char *const) (char*)&data_things_floor_start_;
static const unsigned char *const data_things_floor_end   =
    (const unsigned char *const) (char*)&data_things_floor_end_;

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

extern unsigned char *data_things_food_start_ asm("data_things_food_start_");
extern unsigned char *data_things_food_end_ asm("data_things_food_end_");
static const unsigned char *const data_things_food_start =
    (const unsigned char *const) (char*)&data_things_food_start_;
static const unsigned char *const data_things_food_end   =
    (const unsigned char *const) (char*)&data_things_food_end_;

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

extern unsigned char *data_things_generator2_start_ asm("data_things_generator2_start_");
extern unsigned char *data_things_generator2_end_ asm("data_things_generator2_end_");
static const unsigned char *const data_things_generator2_start =
    (const unsigned char *const) (char*)&data_things_generator2_start_;
static const unsigned char *const data_things_generator2_end   =
    (const unsigned char *const) (char*)&data_things_generator2_end_;

extern unsigned char *data_things_generator3_start_ asm("data_things_generator3_start_");
extern unsigned char *data_things_generator3_end_ asm("data_things_generator3_end_");
static const unsigned char *const data_things_generator3_start =
    (const unsigned char *const) (char*)&data_things_generator3_start_;
static const unsigned char *const data_things_generator3_end   =
    (const unsigned char *const) (char*)&data_things_generator3_end_;

extern unsigned char *data_things_generator4_start_ asm("data_things_generator4_start_");
extern unsigned char *data_things_generator4_end_ asm("data_things_generator4_end_");
static const unsigned char *const data_things_generator4_start =
    (const unsigned char *const) (char*)&data_things_generator4_start_;
static const unsigned char *const data_things_generator4_end   =
    (const unsigned char *const) (char*)&data_things_generator4_end_;

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

extern unsigned char *data_things_keys1_start_ asm("data_things_keys1_start_");
extern unsigned char *data_things_keys1_end_ asm("data_things_keys1_end_");
static const unsigned char *const data_things_keys1_start =
    (const unsigned char *const) (char*)&data_things_keys1_start_;
static const unsigned char *const data_things_keys1_end   =
    (const unsigned char *const) (char*)&data_things_keys1_end_;

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

extern unsigned char *data_things_mask1_start_ asm("data_things_mask1_start_");
extern unsigned char *data_things_mask1_end_ asm("data_things_mask1_end_");
static const unsigned char *const data_things_mask1_start =
    (const unsigned char *const) (char*)&data_things_mask1_start_;
static const unsigned char *const data_things_mask1_end   =
    (const unsigned char *const) (char*)&data_things_mask1_end_;

extern unsigned char *data_things_mask2_start_ asm("data_things_mask2_start_");
extern unsigned char *data_things_mask2_end_ asm("data_things_mask2_end_");
static const unsigned char *const data_things_mask2_start =
    (const unsigned char *const) (char*)&data_things_mask2_start_;
static const unsigned char *const data_things_mask2_end   =
    (const unsigned char *const) (char*)&data_things_mask2_end_;

extern unsigned char *data_things_mask3_start_ asm("data_things_mask3_start_");
extern unsigned char *data_things_mask3_end_ asm("data_things_mask3_end_");
static const unsigned char *const data_things_mask3_start =
    (const unsigned char *const) (char*)&data_things_mask3_start_;
static const unsigned char *const data_things_mask3_end   =
    (const unsigned char *const) (char*)&data_things_mask3_end_;

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

extern unsigned char *data_things_plant_start_ asm("data_things_plant_start_");
extern unsigned char *data_things_plant_end_ asm("data_things_plant_end_");
static const unsigned char *const data_things_plant_start =
    (const unsigned char *const) (char*)&data_things_plant_start_;
static const unsigned char *const data_things_plant_end   =
    (const unsigned char *const) (char*)&data_things_plant_end_;

extern unsigned char *data_things_player_start_ asm("data_things_player_start_");
extern unsigned char *data_things_player_end_ asm("data_things_player_end_");
static const unsigned char *const data_things_player_start =
    (const unsigned char *const) (char*)&data_things_player_start_;
static const unsigned char *const data_things_player_end   =
    (const unsigned char *const) (char*)&data_things_player_end_;

extern unsigned char *data_things_potion1_start_ asm("data_things_potion1_start_");
extern unsigned char *data_things_potion1_end_ asm("data_things_potion1_end_");
static const unsigned char *const data_things_potion1_start =
    (const unsigned char *const) (char*)&data_things_potion1_start_;
static const unsigned char *const data_things_potion1_end   =
    (const unsigned char *const) (char*)&data_things_potion1_end_;

extern unsigned char *data_things_potion2_start_ asm("data_things_potion2_start_");
extern unsigned char *data_things_potion2_end_ asm("data_things_potion2_end_");
static const unsigned char *const data_things_potion2_start =
    (const unsigned char *const) (char*)&data_things_potion2_start_;
static const unsigned char *const data_things_potion2_end   =
    (const unsigned char *const) (char*)&data_things_potion2_end_;

extern unsigned char *data_things_potion3_start_ asm("data_things_potion3_start_");
extern unsigned char *data_things_potion3_end_ asm("data_things_potion3_end_");
static const unsigned char *const data_things_potion3_start =
    (const unsigned char *const) (char*)&data_things_potion3_start_;
static const unsigned char *const data_things_potion3_end   =
    (const unsigned char *const) (char*)&data_things_potion3_end_;

extern unsigned char *data_things_potion4_start_ asm("data_things_potion4_start_");
extern unsigned char *data_things_potion4_end_ asm("data_things_potion4_end_");
static const unsigned char *const data_things_potion4_start =
    (const unsigned char *const) (char*)&data_things_potion4_start_;
static const unsigned char *const data_things_potion4_end   =
    (const unsigned char *const) (char*)&data_things_potion4_end_;

extern unsigned char *data_things_potion5_start_ asm("data_things_potion5_start_");
extern unsigned char *data_things_potion5_end_ asm("data_things_potion5_end_");
static const unsigned char *const data_things_potion5_start =
    (const unsigned char *const) (char*)&data_things_potion5_start_;
static const unsigned char *const data_things_potion5_end   =
    (const unsigned char *const) (char*)&data_things_potion5_end_;

extern unsigned char *data_things_seedpod_start_ asm("data_things_seedpod_start_");
extern unsigned char *data_things_seedpod_end_ asm("data_things_seedpod_end_");
static const unsigned char *const data_things_seedpod_start =
    (const unsigned char *const) (char*)&data_things_seedpod_start_;
static const unsigned char *const data_things_seedpod_end   =
    (const unsigned char *const) (char*)&data_things_seedpod_end_;

extern unsigned char *data_things_spam_start_ asm("data_things_spam_start_");
extern unsigned char *data_things_spam_end_ asm("data_things_spam_end_");
static const unsigned char *const data_things_spam_start =
    (const unsigned char *const) (char*)&data_things_spam_start_;
static const unsigned char *const data_things_spam_end   =
    (const unsigned char *const) (char*)&data_things_spam_end_;

extern unsigned char *data_things_wall_start_ asm("data_things_wall_start_");
extern unsigned char *data_things_wall_end_ asm("data_things_wall_end_");
static const unsigned char *const data_things_wall_start =
    (const unsigned char *const) (char*)&data_things_wall_start_;
static const unsigned char *const data_things_wall_end   =
    (const unsigned char *const) (char*)&data_things_wall_end_;

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

extern unsigned char *data_things_water1_start_ asm("data_things_water1_start_");
extern unsigned char *data_things_water1_end_ asm("data_things_water1_end_");
static const unsigned char *const data_things_water1_start =
    (const unsigned char *const) (char*)&data_things_water1_start_;
static const unsigned char *const data_things_water1_end   =
    (const unsigned char *const) (char*)&data_things_water1_end_;

extern unsigned char *data_things_water2_start_ asm("data_things_water2_start_");
extern unsigned char *data_things_water2_end_ asm("data_things_water2_end_");
static const unsigned char *const data_things_water2_start =
    (const unsigned char *const) (char*)&data_things_water2_start_;
static const unsigned char *const data_things_water2_end   =
    (const unsigned char *const) (char*)&data_things_water2_end_;

extern unsigned char *data_ttf_large_ttf_pointsize50_data_start_ asm("data_ttf_large_ttf_pointsize50_data_start_");
extern unsigned char *data_ttf_large_ttf_pointsize50_data_end_ asm("data_ttf_large_ttf_pointsize50_data_end_");
static const unsigned char *const data_ttf_large_ttf_pointsize50_data_start =
    (const unsigned char *const) (char*)&data_ttf_large_ttf_pointsize50_data_start_;
static const unsigned char *const data_ttf_large_ttf_pointsize50_data_end   =
    (const unsigned char *const) (char*)&data_ttf_large_ttf_pointsize50_data_end_;

extern unsigned char *data_ttf_large_ttf_pointsize50_tga_start_ asm("data_ttf_large_ttf_pointsize50_tga_start_");
extern unsigned char *data_ttf_large_ttf_pointsize50_tga_end_ asm("data_ttf_large_ttf_pointsize50_tga_end_");
static const unsigned char *const data_ttf_large_ttf_pointsize50_tga_start =
    (const unsigned char *const) (char*)&data_ttf_large_ttf_pointsize50_tga_start_;
static const unsigned char *const data_ttf_large_ttf_pointsize50_tga_end   =
    (const unsigned char *const) (char*)&data_ttf_large_ttf_pointsize50_tga_end_;

extern unsigned char *data_ttf_LICENSE_txt_start_ asm("data_ttf_LICENSE_txt_start_");
extern unsigned char *data_ttf_LICENSE_txt_end_ asm("data_ttf_LICENSE_txt_end_");
static const unsigned char *const data_ttf_LICENSE_txt_start =
    (const unsigned char *const) (char*)&data_ttf_LICENSE_txt_start_;
static const unsigned char *const data_ttf_LICENSE_txt_end   =
    (const unsigned char *const) (char*)&data_ttf_LICENSE_txt_end_;

extern unsigned char *data_ttf_med_ttf_pointsize40_data_start_ asm("data_ttf_med_ttf_pointsize40_data_start_");
extern unsigned char *data_ttf_med_ttf_pointsize40_data_end_ asm("data_ttf_med_ttf_pointsize40_data_end_");
static const unsigned char *const data_ttf_med_ttf_pointsize40_data_start =
    (const unsigned char *const) (char*)&data_ttf_med_ttf_pointsize40_data_start_;
static const unsigned char *const data_ttf_med_ttf_pointsize40_data_end   =
    (const unsigned char *const) (char*)&data_ttf_med_ttf_pointsize40_data_end_;

extern unsigned char *data_ttf_med_ttf_pointsize40_tga_start_ asm("data_ttf_med_ttf_pointsize40_tga_start_");
extern unsigned char *data_ttf_med_ttf_pointsize40_tga_end_ asm("data_ttf_med_ttf_pointsize40_tga_end_");
static const unsigned char *const data_ttf_med_ttf_pointsize40_tga_start =
    (const unsigned char *const) (char*)&data_ttf_med_ttf_pointsize40_tga_start_;
static const unsigned char *const data_ttf_med_ttf_pointsize40_tga_end   =
    (const unsigned char *const) (char*)&data_ttf_med_ttf_pointsize40_tga_end_;

extern unsigned char *data_ttf_Readme_txt_start_ asm("data_ttf_Readme_txt_start_");
extern unsigned char *data_ttf_Readme_txt_end_ asm("data_ttf_Readme_txt_end_");
static const unsigned char *const data_ttf_Readme_txt_start =
    (const unsigned char *const) (char*)&data_ttf_Readme_txt_start_;
static const unsigned char *const data_ttf_Readme_txt_end   =
    (const unsigned char *const) (char*)&data_ttf_Readme_txt_end_;

extern unsigned char *data_ttf_small_ttf_pointsize20_data_start_ asm("data_ttf_small_ttf_pointsize20_data_start_");
extern unsigned char *data_ttf_small_ttf_pointsize20_data_end_ asm("data_ttf_small_ttf_pointsize20_data_end_");
static const unsigned char *const data_ttf_small_ttf_pointsize20_data_start =
    (const unsigned char *const) (char*)&data_ttf_small_ttf_pointsize20_data_start_;
static const unsigned char *const data_ttf_small_ttf_pointsize20_data_end   =
    (const unsigned char *const) (char*)&data_ttf_small_ttf_pointsize20_data_end_;

extern unsigned char *data_ttf_small_ttf_pointsize20_tga_start_ asm("data_ttf_small_ttf_pointsize20_tga_start_");
extern unsigned char *data_ttf_small_ttf_pointsize20_tga_end_ asm("data_ttf_small_ttf_pointsize20_tga_end_");
static const unsigned char *const data_ttf_small_ttf_pointsize20_tga_start =
    (const unsigned char *const) (char*)&data_ttf_small_ttf_pointsize20_tga_start_;
static const unsigned char *const data_ttf_small_ttf_pointsize20_tga_end   =
    (const unsigned char *const) (char*)&data_ttf_small_ttf_pointsize20_tga_end_;

ramdisk_t ramdisk_data[] = {
{
    /* filename */ "data/.player.session",
},
{
    /* filename */ "data/gfx/dragon.png",
},
{
    /* filename */ "data/gfx/gorynlich.png",
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
    /* filename */ "data/gfx/rat.png",
},
{
    /* filename */ "data/gfx/shell.png",
},
{
    /* filename */ "data/gfx/sprites_small.png",
},
{
    /* filename */ "data/gfx/sprites_small.xcf",
},
{
    /* filename */ "data/gfx/stars.png",
},
{
    /* filename */ "data/gfx/title.png",
},
{
    /* filename */ "data/map/jigsaw.map",
},
{
    /* filename */ "data/map/map_light.data",
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
    /* filename */ "data/things/.amulet1.session",
},
{
    /* filename */ "data/things/.chest1.session",
},
{
    /* filename */ "data/things/.door.session",
},
{
    /* filename */ "data/things/.floor.session",
},
{
    /* filename */ "data/things/.floor1.session",
},
{
    /* filename */ "data/things/.floor2.session",
},
{
    /* filename */ "data/things/.floor3.session",
},
{
    /* filename */ "data/things/.floor4.session",
},
{
    /* filename */ "data/things/.floor5.session",
},
{
    /* filename */ "data/things/.floor6.session",
},
{
    /* filename */ "data/things/.food.session",
},
{
    /* filename */ "data/things/.food2.session",
},
{
    /* filename */ "data/things/.gem1.session",
},
{
    /* filename */ "data/things/.gem7.session",
},
{
    /* filename */ "data/things/.generator1.session",
},
{
    /* filename */ "data/things/.generator7.session",
},
{
    /* filename */ "data/things/.keys1.session",
},
{
    /* filename */ "data/things/.keys2.session",
},
{
    /* filename */ "data/things/.keys3.session",
},
{
    /* filename */ "data/things/.mask1.session",
},
{
    /* filename */ "data/things/.mask3.session",
},
{
    /* filename */ "data/things/.potion1.session",
},
{
    /* filename */ "data/things/.potion5.session",
},
{
    /* filename */ "data/things/.rock_0.session",
},
{
    /* filename */ "data/things/.rock_1.session",
},
{
    /* filename */ "data/things/.treasure.session",
},
{
    /* filename */ "data/things/.wall.session",
},
{
    /* filename */ "data/things/.wall2.session",
},
{
    /* filename */ "data/things/.water1.session",
},
{
    /* filename */ "data/things/amulet1",
},
{
    /* filename */ "data/things/bomb",
},
{
    /* filename */ "data/things/chest1",
},
{
    /* filename */ "data/things/coins1",
},
{
    /* filename */ "data/things/door",
},
{
    /* filename */ "data/things/exit",
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
    /* filename */ "data/things/explosion",
},
{
    /* filename */ "data/things/floor",
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
    /* filename */ "data/things/food",
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
    /* filename */ "data/things/generator2",
},
{
    /* filename */ "data/things/generator3",
},
{
    /* filename */ "data/things/generator4",
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
    /* filename */ "data/things/keys1",
},
{
    /* filename */ "data/things/keys2",
},
{
    /* filename */ "data/things/keys3",
},
{
    /* filename */ "data/things/mask1",
},
{
    /* filename */ "data/things/mask2",
},
{
    /* filename */ "data/things/mask3",
},
{
    /* filename */ "data/things/noentry",
},
{
    /* filename */ "data/things/pipe",
},
{
    /* filename */ "data/things/plant",
},
{
    /* filename */ "data/things/player",
},
{
    /* filename */ "data/things/potion1",
},
{
    /* filename */ "data/things/potion2",
},
{
    /* filename */ "data/things/potion3",
},
{
    /* filename */ "data/things/potion4",
},
{
    /* filename */ "data/things/potion5",
},
{
    /* filename */ "data/things/seedpod",
},
{
    /* filename */ "data/things/spam",
},
{
    /* filename */ "data/things/wall",
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
    /* filename */ "data/things/water1",
},
{
    /* filename */ "data/things/water2",
},
{
    /* filename */ "data/ttf/large.ttf_pointsize50.data",
},
{
    /* filename */ "data/ttf/large.ttf_pointsize50.tga",
},
{
    /* filename */ "data/ttf/LICENSE.txt",
},
{
    /* filename */ "data/ttf/med.ttf_pointsize40.data",
},
{
    /* filename */ "data/ttf/med.ttf_pointsize40.tga",
},
{
    /* filename */ "data/ttf/Readme.txt",
},
{
    /* filename */ "data/ttf/small.ttf_pointsize20.data",
},
{
    /* filename */ "data/ttf/small.ttf_pointsize20.tga",
},
    {0},
};

void ramdisk_init (void)
{
    ramdisk_data[0].data = data__player_session_start;
    ramdisk_data[0].len = data__player_session_end - data__player_session_start;
    ramdisk_data[1].data = data_gfx_dragon_png_start;
    ramdisk_data[1].len = data_gfx_dragon_png_end - data_gfx_dragon_png_start;
    ramdisk_data[2].data = data_gfx_gorynlich_png_start;
    ramdisk_data[2].len = data_gfx_gorynlich_png_end - data_gfx_gorynlich_png_start;
    ramdisk_data[3].data = data_gfx_help1_png_start;
    ramdisk_data[3].len = data_gfx_help1_png_end - data_gfx_help1_png_start;
    ramdisk_data[4].data = data_gfx_help2_png_start;
    ramdisk_data[4].len = data_gfx_help2_png_end - data_gfx_help2_png_start;
    ramdisk_data[5].data = data_gfx_help3_png_start;
    ramdisk_data[5].len = data_gfx_help3_png_end - data_gfx_help3_png_start;
    ramdisk_data[6].data = data_gfx_rat_png_start;
    ramdisk_data[6].len = data_gfx_rat_png_end - data_gfx_rat_png_start;
    ramdisk_data[7].data = data_gfx_shell_png_start;
    ramdisk_data[7].len = data_gfx_shell_png_end - data_gfx_shell_png_start;
    ramdisk_data[8].data = data_gfx_sprites_small_png_start;
    ramdisk_data[8].len = data_gfx_sprites_small_png_end - data_gfx_sprites_small_png_start;
    ramdisk_data[9].data = data_gfx_sprites_small_xcf_start;
    ramdisk_data[9].len = data_gfx_sprites_small_xcf_end - data_gfx_sprites_small_xcf_start;
    ramdisk_data[10].data = data_gfx_stars_png_start;
    ramdisk_data[10].len = data_gfx_stars_png_end - data_gfx_stars_png_start;
    ramdisk_data[11].data = data_gfx_title_png_start;
    ramdisk_data[11].len = data_gfx_title_png_end - data_gfx_title_png_start;
    ramdisk_data[12].data = data_map_jigsaw_map_start;
    ramdisk_data[12].len = data_map_jigsaw_map_end - data_map_jigsaw_map_start;
    ramdisk_data[13].data = data_map_map_light_data_start;
    ramdisk_data[13].len = data_map_map_light_data_end - data_map_map_light_data_start;
    ramdisk_data[14].data = data_music_hiscore_wav_start;
    ramdisk_data[14].len = data_music_hiscore_wav_end - data_music_hiscore_wav_start;
    ramdisk_data[15].data = data_music_intro_wav_start;
    ramdisk_data[15].len = data_music_intro_wav_end - data_music_intro_wav_start;
    ramdisk_data[16].data = data_music_music_wav_start;
    ramdisk_data[16].len = data_music_music_wav_end - data_music_music_wav_start;
    ramdisk_data[17].data = data_sound_chomp_wav_start;
    ramdisk_data[17].len = data_sound_chomp_wav_end - data_sound_chomp_wav_start;
    ramdisk_data[18].data = data_sound_chomp2_wav_start;
    ramdisk_data[18].len = data_sound_chomp2_wav_end - data_sound_chomp2_wav_start;
    ramdisk_data[19].data = data_sound_click_wav_start;
    ramdisk_data[19].len = data_sound_click_wav_end - data_sound_click_wav_start;
    ramdisk_data[20].data = data_sound_doorbell_wav_start;
    ramdisk_data[20].len = data_sound_doorbell_wav_end - data_sound_doorbell_wav_start;
    ramdisk_data[21].data = data_sound_explosion_wav_start;
    ramdisk_data[21].len = data_sound_explosion_wav_end - data_sound_explosion_wav_start;
    ramdisk_data[22].data = data_sound_letter_wav_start;
    ramdisk_data[22].len = data_sound_letter_wav_end - data_sound_letter_wav_start;
    ramdisk_data[23].data = data_sound_level_end_wav_start;
    ramdisk_data[23].len = data_sound_level_end_wav_end - data_sound_level_end_wav_start;
    ramdisk_data[24].data = data_sound_paper_wav_start;
    ramdisk_data[24].len = data_sound_paper_wav_end - data_sound_paper_wav_start;
    ramdisk_data[25].data = data_sound_powerup_wav_start;
    ramdisk_data[25].len = data_sound_powerup_wav_end - data_sound_powerup_wav_start;
    ramdisk_data[26].data = data_sound_ratdeath_wav_start;
    ramdisk_data[26].len = data_sound_ratdeath_wav_end - data_sound_ratdeath_wav_start;
    ramdisk_data[27].data = data_sound_rocket_wav_start;
    ramdisk_data[27].len = data_sound_rocket_wav_end - data_sound_rocket_wav_start;
    ramdisk_data[28].data = data_sound_slime_wav_start;
    ramdisk_data[28].len = data_sound_slime_wav_end - data_sound_slime_wav_start;
    ramdisk_data[29].data = data_sound_spam_wav_start;
    ramdisk_data[29].len = data_sound_spam_wav_end - data_sound_spam_wav_start;
    ramdisk_data[30].data = data_sound_thief_wav_start;
    ramdisk_data[30].len = data_sound_thief_wav_end - data_sound_thief_wav_start;
    ramdisk_data[31].data = data_things__amulet1_session_start;
    ramdisk_data[31].len = data_things__amulet1_session_end - data_things__amulet1_session_start;
    ramdisk_data[32].data = data_things__chest1_session_start;
    ramdisk_data[32].len = data_things__chest1_session_end - data_things__chest1_session_start;
    ramdisk_data[33].data = data_things__door_session_start;
    ramdisk_data[33].len = data_things__door_session_end - data_things__door_session_start;
    ramdisk_data[34].data = data_things__floor_session_start;
    ramdisk_data[34].len = data_things__floor_session_end - data_things__floor_session_start;
    ramdisk_data[35].data = data_things__floor1_session_start;
    ramdisk_data[35].len = data_things__floor1_session_end - data_things__floor1_session_start;
    ramdisk_data[36].data = data_things__floor2_session_start;
    ramdisk_data[36].len = data_things__floor2_session_end - data_things__floor2_session_start;
    ramdisk_data[37].data = data_things__floor3_session_start;
    ramdisk_data[37].len = data_things__floor3_session_end - data_things__floor3_session_start;
    ramdisk_data[38].data = data_things__floor4_session_start;
    ramdisk_data[38].len = data_things__floor4_session_end - data_things__floor4_session_start;
    ramdisk_data[39].data = data_things__floor5_session_start;
    ramdisk_data[39].len = data_things__floor5_session_end - data_things__floor5_session_start;
    ramdisk_data[40].data = data_things__floor6_session_start;
    ramdisk_data[40].len = data_things__floor6_session_end - data_things__floor6_session_start;
    ramdisk_data[41].data = data_things__food_session_start;
    ramdisk_data[41].len = data_things__food_session_end - data_things__food_session_start;
    ramdisk_data[42].data = data_things__food2_session_start;
    ramdisk_data[42].len = data_things__food2_session_end - data_things__food2_session_start;
    ramdisk_data[43].data = data_things__gem1_session_start;
    ramdisk_data[43].len = data_things__gem1_session_end - data_things__gem1_session_start;
    ramdisk_data[44].data = data_things__gem7_session_start;
    ramdisk_data[44].len = data_things__gem7_session_end - data_things__gem7_session_start;
    ramdisk_data[45].data = data_things__generator1_session_start;
    ramdisk_data[45].len = data_things__generator1_session_end - data_things__generator1_session_start;
    ramdisk_data[46].data = data_things__generator7_session_start;
    ramdisk_data[46].len = data_things__generator7_session_end - data_things__generator7_session_start;
    ramdisk_data[47].data = data_things__keys1_session_start;
    ramdisk_data[47].len = data_things__keys1_session_end - data_things__keys1_session_start;
    ramdisk_data[48].data = data_things__keys2_session_start;
    ramdisk_data[48].len = data_things__keys2_session_end - data_things__keys2_session_start;
    ramdisk_data[49].data = data_things__keys3_session_start;
    ramdisk_data[49].len = data_things__keys3_session_end - data_things__keys3_session_start;
    ramdisk_data[50].data = data_things__mask1_session_start;
    ramdisk_data[50].len = data_things__mask1_session_end - data_things__mask1_session_start;
    ramdisk_data[51].data = data_things__mask3_session_start;
    ramdisk_data[51].len = data_things__mask3_session_end - data_things__mask3_session_start;
    ramdisk_data[52].data = data_things__potion1_session_start;
    ramdisk_data[52].len = data_things__potion1_session_end - data_things__potion1_session_start;
    ramdisk_data[53].data = data_things__potion5_session_start;
    ramdisk_data[53].len = data_things__potion5_session_end - data_things__potion5_session_start;
    ramdisk_data[54].data = data_things__rock_0_session_start;
    ramdisk_data[54].len = data_things__rock_0_session_end - data_things__rock_0_session_start;
    ramdisk_data[55].data = data_things__rock_1_session_start;
    ramdisk_data[55].len = data_things__rock_1_session_end - data_things__rock_1_session_start;
    ramdisk_data[56].data = data_things__treasure_session_start;
    ramdisk_data[56].len = data_things__treasure_session_end - data_things__treasure_session_start;
    ramdisk_data[57].data = data_things__wall_session_start;
    ramdisk_data[57].len = data_things__wall_session_end - data_things__wall_session_start;
    ramdisk_data[58].data = data_things__wall2_session_start;
    ramdisk_data[58].len = data_things__wall2_session_end - data_things__wall2_session_start;
    ramdisk_data[59].data = data_things__water1_session_start;
    ramdisk_data[59].len = data_things__water1_session_end - data_things__water1_session_start;
    ramdisk_data[60].data = data_things_amulet1_start;
    ramdisk_data[60].len = data_things_amulet1_end - data_things_amulet1_start;
    ramdisk_data[61].data = data_things_bomb_start;
    ramdisk_data[61].len = data_things_bomb_end - data_things_bomb_start;
    ramdisk_data[62].data = data_things_chest1_start;
    ramdisk_data[62].len = data_things_chest1_end - data_things_chest1_start;
    ramdisk_data[63].data = data_things_coins1_start;
    ramdisk_data[63].len = data_things_coins1_end - data_things_coins1_start;
    ramdisk_data[64].data = data_things_door_start;
    ramdisk_data[64].len = data_things_door_end - data_things_door_start;
    ramdisk_data[65].data = data_things_exit_start;
    ramdisk_data[65].len = data_things_exit_end - data_things_exit_start;
    ramdisk_data[66].data = data_things_exit1_start;
    ramdisk_data[66].len = data_things_exit1_end - data_things_exit1_start;
    ramdisk_data[67].data = data_things_exit2_start;
    ramdisk_data[67].len = data_things_exit2_end - data_things_exit2_start;
    ramdisk_data[68].data = data_things_exit3_start;
    ramdisk_data[68].len = data_things_exit3_end - data_things_exit3_start;
    ramdisk_data[69].data = data_things_exit4_start;
    ramdisk_data[69].len = data_things_exit4_end - data_things_exit4_start;
    ramdisk_data[70].data = data_things_exit5_start;
    ramdisk_data[70].len = data_things_exit5_end - data_things_exit5_start;
    ramdisk_data[71].data = data_things_exit6_start;
    ramdisk_data[71].len = data_things_exit6_end - data_things_exit6_start;
    ramdisk_data[72].data = data_things_exit7_start;
    ramdisk_data[72].len = data_things_exit7_end - data_things_exit7_start;
    ramdisk_data[73].data = data_things_explosion_start;
    ramdisk_data[73].len = data_things_explosion_end - data_things_explosion_start;
    ramdisk_data[74].data = data_things_floor_start;
    ramdisk_data[74].len = data_things_floor_end - data_things_floor_start;
    ramdisk_data[75].data = data_things_floor2_start;
    ramdisk_data[75].len = data_things_floor2_end - data_things_floor2_start;
    ramdisk_data[76].data = data_things_floor3_start;
    ramdisk_data[76].len = data_things_floor3_end - data_things_floor3_start;
    ramdisk_data[77].data = data_things_floor4_start;
    ramdisk_data[77].len = data_things_floor4_end - data_things_floor4_start;
    ramdisk_data[78].data = data_things_floor5_start;
    ramdisk_data[78].len = data_things_floor5_end - data_things_floor5_start;
    ramdisk_data[79].data = data_things_floor6_start;
    ramdisk_data[79].len = data_things_floor6_end - data_things_floor6_start;
    ramdisk_data[80].data = data_things_food_start;
    ramdisk_data[80].len = data_things_food_end - data_things_food_start;
    ramdisk_data[81].data = data_things_gem1_start;
    ramdisk_data[81].len = data_things_gem1_end - data_things_gem1_start;
    ramdisk_data[82].data = data_things_gem2_start;
    ramdisk_data[82].len = data_things_gem2_end - data_things_gem2_start;
    ramdisk_data[83].data = data_things_gem3_start;
    ramdisk_data[83].len = data_things_gem3_end - data_things_gem3_start;
    ramdisk_data[84].data = data_things_gem4_start;
    ramdisk_data[84].len = data_things_gem4_end - data_things_gem4_start;
    ramdisk_data[85].data = data_things_gem5_start;
    ramdisk_data[85].len = data_things_gem5_end - data_things_gem5_start;
    ramdisk_data[86].data = data_things_gem6_start;
    ramdisk_data[86].len = data_things_gem6_end - data_things_gem6_start;
    ramdisk_data[87].data = data_things_gem7_start;
    ramdisk_data[87].len = data_things_gem7_end - data_things_gem7_start;
    ramdisk_data[88].data = data_things_generator1_start;
    ramdisk_data[88].len = data_things_generator1_end - data_things_generator1_start;
    ramdisk_data[89].data = data_things_generator2_start;
    ramdisk_data[89].len = data_things_generator2_end - data_things_generator2_start;
    ramdisk_data[90].data = data_things_generator3_start;
    ramdisk_data[90].len = data_things_generator3_end - data_things_generator3_start;
    ramdisk_data[91].data = data_things_generator4_start;
    ramdisk_data[91].len = data_things_generator4_end - data_things_generator4_start;
    ramdisk_data[92].data = data_things_generator5_start;
    ramdisk_data[92].len = data_things_generator5_end - data_things_generator5_start;
    ramdisk_data[93].data = data_things_generator6_start;
    ramdisk_data[93].len = data_things_generator6_end - data_things_generator6_start;
    ramdisk_data[94].data = data_things_generator7_start;
    ramdisk_data[94].len = data_things_generator7_end - data_things_generator7_start;
    ramdisk_data[95].data = data_things_keys1_start;
    ramdisk_data[95].len = data_things_keys1_end - data_things_keys1_start;
    ramdisk_data[96].data = data_things_keys2_start;
    ramdisk_data[96].len = data_things_keys2_end - data_things_keys2_start;
    ramdisk_data[97].data = data_things_keys3_start;
    ramdisk_data[97].len = data_things_keys3_end - data_things_keys3_start;
    ramdisk_data[98].data = data_things_mask1_start;
    ramdisk_data[98].len = data_things_mask1_end - data_things_mask1_start;
    ramdisk_data[99].data = data_things_mask2_start;
    ramdisk_data[99].len = data_things_mask2_end - data_things_mask2_start;
    ramdisk_data[100].data = data_things_mask3_start;
    ramdisk_data[100].len = data_things_mask3_end - data_things_mask3_start;
    ramdisk_data[101].data = data_things_noentry_start;
    ramdisk_data[101].len = data_things_noentry_end - data_things_noentry_start;
    ramdisk_data[102].data = data_things_pipe_start;
    ramdisk_data[102].len = data_things_pipe_end - data_things_pipe_start;
    ramdisk_data[103].data = data_things_plant_start;
    ramdisk_data[103].len = data_things_plant_end - data_things_plant_start;
    ramdisk_data[104].data = data_things_player_start;
    ramdisk_data[104].len = data_things_player_end - data_things_player_start;
    ramdisk_data[105].data = data_things_potion1_start;
    ramdisk_data[105].len = data_things_potion1_end - data_things_potion1_start;
    ramdisk_data[106].data = data_things_potion2_start;
    ramdisk_data[106].len = data_things_potion2_end - data_things_potion2_start;
    ramdisk_data[107].data = data_things_potion3_start;
    ramdisk_data[107].len = data_things_potion3_end - data_things_potion3_start;
    ramdisk_data[108].data = data_things_potion4_start;
    ramdisk_data[108].len = data_things_potion4_end - data_things_potion4_start;
    ramdisk_data[109].data = data_things_potion5_start;
    ramdisk_data[109].len = data_things_potion5_end - data_things_potion5_start;
    ramdisk_data[110].data = data_things_seedpod_start;
    ramdisk_data[110].len = data_things_seedpod_end - data_things_seedpod_start;
    ramdisk_data[111].data = data_things_spam_start;
    ramdisk_data[111].len = data_things_spam_end - data_things_spam_start;
    ramdisk_data[112].data = data_things_wall_start;
    ramdisk_data[112].len = data_things_wall_end - data_things_wall_start;
    ramdisk_data[113].data = data_things_wall2_start;
    ramdisk_data[113].len = data_things_wall2_end - data_things_wall2_start;
    ramdisk_data[114].data = data_things_wall3_start;
    ramdisk_data[114].len = data_things_wall3_end - data_things_wall3_start;
    ramdisk_data[115].data = data_things_wall4_start;
    ramdisk_data[115].len = data_things_wall4_end - data_things_wall4_start;
    ramdisk_data[116].data = data_things_wall5_start;
    ramdisk_data[116].len = data_things_wall5_end - data_things_wall5_start;
    ramdisk_data[117].data = data_things_water1_start;
    ramdisk_data[117].len = data_things_water1_end - data_things_water1_start;
    ramdisk_data[118].data = data_things_water2_start;
    ramdisk_data[118].len = data_things_water2_end - data_things_water2_start;
    ramdisk_data[119].data = data_ttf_large_ttf_pointsize50_data_start;
    ramdisk_data[119].len = data_ttf_large_ttf_pointsize50_data_end - data_ttf_large_ttf_pointsize50_data_start;
    ramdisk_data[120].data = data_ttf_large_ttf_pointsize50_tga_start;
    ramdisk_data[120].len = data_ttf_large_ttf_pointsize50_tga_end - data_ttf_large_ttf_pointsize50_tga_start;
    ramdisk_data[121].data = data_ttf_LICENSE_txt_start;
    ramdisk_data[121].len = data_ttf_LICENSE_txt_end - data_ttf_LICENSE_txt_start;
    ramdisk_data[122].data = data_ttf_med_ttf_pointsize40_data_start;
    ramdisk_data[122].len = data_ttf_med_ttf_pointsize40_data_end - data_ttf_med_ttf_pointsize40_data_start;
    ramdisk_data[123].data = data_ttf_med_ttf_pointsize40_tga_start;
    ramdisk_data[123].len = data_ttf_med_ttf_pointsize40_tga_end - data_ttf_med_ttf_pointsize40_tga_start;
    ramdisk_data[124].data = data_ttf_Readme_txt_start;
    ramdisk_data[124].len = data_ttf_Readme_txt_end - data_ttf_Readme_txt_start;
    ramdisk_data[125].data = data_ttf_small_ttf_pointsize20_data_start;
    ramdisk_data[125].len = data_ttf_small_ttf_pointsize20_data_end - data_ttf_small_ttf_pointsize20_data_start;
    ramdisk_data[126].data = data_ttf_small_ttf_pointsize20_tga_start;
    ramdisk_data[126].len = data_ttf_small_ttf_pointsize20_tga_end - data_ttf_small_ttf_pointsize20_tga_start;
}
